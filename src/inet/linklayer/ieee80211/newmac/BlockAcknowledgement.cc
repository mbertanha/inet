//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#include "BlockAcknowledgement.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/common/BitVector.h"

namespace inet {
namespace ieee80211 {

BlockAcknowledgmentSendSessions::BlockAcknowledgmentSendSessions()
{

}

BlockAcknowledgmentSendSessions::~BlockAcknowledgmentSendSessions()
{
    for (auto it : sendSessions)
        delete it.second;
}

void BlockAcknowledgmentSendSessions::addSession(Ieee80211AddbaRequest *request, Ieee80211AddbaResponse *response)
{
    ASSERT(response->getTid() == request->getTid());
    ASSERT(response->getBlockAckPolicy() == request->getBlockAckPolicy());
    Session *session = new Session(request, response);
    MACAddress receiverAddr = request->getReceiverAddress();
    int tid = request->getTid();
    sendSessions[Key(receiverAddr, tid)] = session;
}

void BlockAcknowledgmentSendSessions::deleteSession(const MACAddress& responder, int tid)
{
    auto it = sendSessions.find(Key(responder, tid));
    ASSERT(it != sendSessions.end());
    delete it->second;
    sendSessions.erase(it);
}

BlockAcknowledgmentSendSessions::Session *BlockAcknowledgmentSendSessions::getSession(const MACAddress& responder, int tid)
{
    auto it = sendSessions.find(Key(responder, tid));
    return it == sendSessions.end() ? nullptr : it->second;
}

BlockAcknowledgmentSendSessions::Session::Session(Ieee80211AddbaRequest *request, Ieee80211AddbaResponse *response)
{
    // When a Block Ack agreement is established between two HT STAs, the originator may change the size of its
    // transmission window if the value in the Buffer Size field of the ADDBA Response frame is larger than the
    // value in the ADDBA Request frame.
    // If the value in the Buffer Size field of the ADDBA Response frame is smaller than the value in the ADDBA Request
    // frame, the originator shall change the size of its transmission window (WinSizeO) so that it is not greater than
    // the value in the Buffer Size field of the ADDBA Response frame and is not greater than the value 64.
    if (response->getBufferSize() > request->getBufferSize())
        txWindowSize = response->getBufferSize(); // TODO: may change..
    else if (response->getBufferSize() < request->getBufferSize())
        txWindowSize = response->getBufferSize();
    txWindowSize = std::min(64, txWindowSize);
    startingSequenceNumber = request->getStartingSequenceNumber();
    // Once the Block Ack exchange has been set up, data and ACK frames are transferred using the procedure
    // described in 9.21.3.
}

BlockAcknowledgmentSendSessions::Session::~Session()
{
    for (auto frame : transmittedFrames)
        delete frame;
}

void BlockAcknowledgmentSendSessions::Session::addToTransmittedFrames(Ieee80211DataOrMgmtFrame *frame)
{
    transmittedFrames.push_back(frame);
}

void BlockAcknowledgmentSendSessions::blockAckReceived(Ieee80211BlockAck *blockAck)
{
    int tid;
    if (blockAck->getMultiTid() == 0 && blockAck->getCompressedBitmap() == 0) // Note: fragments are supported
    {
        Ieee80211BasicBlockAck *basicBlockAck = check_and_cast<Ieee80211BasicBlockAck*>(blockAck);
        tid = basicBlockAck->getTidInfo();
        getSession(basicBlockAck->getTransmitterAddress(), tid)->collectFramesToRetransmit(basicBlockAck);
    }
    else if (blockAck->getMultiTid() == 0 && blockAck->getCompressedBitmap() == 1) // Note: fragments are not supported
    {
        Ieee80211CompressedBlockAck *compressedBlockAck = check_and_cast<Ieee80211CompressedBlockAck*>(blockAck);
        tid = compressedBlockAck->getTidInfo();
        getSession(compressedBlockAck->getTransmitterAddress(), tid)->collectFramesToRetransmit(compressedBlockAck);
    }
    else if (blockAck->getMultiTid() == 1 && blockAck->getCompressedBitmap() == 1)
        throw cRuntimeError("MultiTid BlockAck is unsupported.");
    else
        throw cRuntimeError("Unknown BlockAck variant");
}

void BlockAcknowledgmentSendSessions::Session::collectFramesToRetransmit(Ieee80211BasicBlockAck *basicBlockAck)
{
    for (auto frame : transmittedFrames)
    {
        int sequenceNumber = frame->getSequenceNumber();
        int fragmentNumber = frame->getFragmentNumber();

        BitVector ackedFragments = basicBlockAck->getBlockAckBitmap(sequenceNumber - startingSequenceNumber);
        if (ackedFragments.getBit(fragmentNumber) == 0)
            framesToRetransmit.push_back(frame);
    }
}

void BlockAcknowledgmentSendSessions::Session::collectFramesToRetransmit(Ieee80211CompressedBlockAck *basicBlockAck)
{
    BitVector ackedMsdus = basicBlockAck->getBlockAckBitmap();
    for (auto frame : transmittedFrames)
    {
        int sequenceNumber = frame->getSequenceNumber();
        if (ackedMsdus.getBit(sequenceNumber - startingSequenceNumber) == 0)
            framesToRetransmit.push_back(frame);
    }
}

//
// 10.5.3.3 Procedure at the recipient of the DELBA frame
//
void BlockAcknowledgmentSendSessions::delbaReceived(Ieee80211Delba* delba)
{

}

bool BlockAcknowledgmentSendSessions::addbaResponseReceived(Ieee80211AddbaRequest* request, Ieee80211AddbaResponse* response)
{
    // The A-MSDU Supported field indicates whether an A-MSDU may be sent under the particular Block Ack
    // agreement. The originator sets this field to 1 to indicate that it might transmit A-MSDUs with this TID. The
    // recipient sets this field to 1 to indicate that it is capable of receiving an A-MSDU with this TID.
    bool aMsduSupported = request->getAMsduSupported() && response->getAMsduSupported();
    // NOTE—The recipient is free to respond with any setting of the A-MSDU supported field. If the value in the ADDBA
    // Response frame is not acceptable to the originator, it can delete the Block Ack agreement and transmit data using normal
    // acknowledgment.
    if (aMsduSupported != request->getAMsduSupported())
        return false;
    addSession(request, response);
    return true;
}
std::vector<Ieee80211DataOrMgmtFrame*>& BlockAcknowledgmentSendSessions::getFramesToRetransmit(Ieee80211BlockAck* blockAck)
{
    MACAddress responder = blockAck->getTransmitterAddress();
    int tid;
    if (blockAck->getMultiTid() == 0 && blockAck->getCompressedBitmap() == 0) // Note: fragments are supported
    {
        Ieee80211BasicBlockAck *basicBlockAck = check_and_cast<Ieee80211BasicBlockAck*>(blockAck);
        tid = basicBlockAck->getTidInfo();
    }
    else if (blockAck->getMultiTid() == 0 && blockAck->getCompressedBitmap() == 1) // Note: fragments are not supported
    {
        Ieee80211CompressedBlockAck *compressedBlockAck = check_and_cast<Ieee80211CompressedBlockAck*>(blockAck);
        tid = compressedBlockAck->getTidInfo();
    }
    else if (blockAck->getMultiTid() == 1 && blockAck->getCompressedBitmap() == 1)
    {
        throw cRuntimeError("MultiTid BlockAck is unsupported.");
        // TODO:
        // int tids[] = multiTidBlockAck->getTidInfo();
        // for (tid : tids)
        //  ... getSession(responder, tid)->getFramesToRetransmit()
    }
    else
        throw cRuntimeError("Unknown BlockAck variant");
    return getSession(responder, tid)->getFramesToRetransmit();
}



//----

BlockAcknowledgmentReceiveSessions::BlockAcknowledgmentReceiveSessions(Ieee80211AddbaRequest *request, Ieee80211AddbaResponse *response)
{
}

BlockAcknowledgmentReceiveSessions::~BlockAcknowledgmentReceiveSessions()
{
    for (auto it : receiveSessions)
        delete it.second;
}

bool BlockAcknowledgmentReceiveSessions::addSession(Ieee80211AddbaRequest *request, Ieee80211AddbaResponse *response)
{

}

void BlockAcknowledgmentReceiveSessions::deleteSession(const MACAddress& originator, int tid)
{
    auto it = receiveSessions.find(Key(originator, tid));
    ASSERT(it != receiveSessions.end());
    delete it->second;
    receiveSessions.erase(it);
}

BlockAcknowledgmentReceiveSessions::Session *BlockAcknowledgmentReceiveSessions::getSession(const MACAddress& originator, int tid)
{
    auto it = receiveSessions.find(Key(originator, tid));
    return it == receiveSessions.end() ? nullptr : it->second;
}

BlockAcknowledgmentReceiveSessions::Session::Session()
{
    //TODO
}

BlockAcknowledgmentReceiveSessions::Session::~Session()
{
    for (auto frame : reorderBuffer)
        delete frame;
}

void BlockAcknowledgmentReceiveSessions::Session::addReceivedFrame(Ieee80211DataOrMgmtFrame *frame)
{

}

Ieee80211DataOrMgmtFrame *BlockAcknowledgmentReceiveSessions::Session::extractFrame()
{
}

std::vector<Ieee80211DataOrMgmtFrame*> BlockAcknowledgmentReceiveSessions::Session::extractAndFlushUntil(int startSequenceNumber)
{
}

} // namespace ieee80211
} // namespace inet

