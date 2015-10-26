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

void BlockAcknowledgmentSendSessions::addSession(const MACAddress& responder, int tid, int startSequenceNumber, int windowSize)
{
    Session *session = new Session();
    sendSessions[Key(responder, tid)] = session;
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

BlockAcknowledgmentSendSessions::Session::Session()
{
    //TODO
}

BlockAcknowledgmentSendSessions::Session::~Session()
{
    for (auto frame : resendBuffer)
        delete frame;
}

void BlockAcknowledgmentSendSessions::Session::addFrameToSend(Ieee80211DataOrMgmtFrame *frame)
{
    lastUseTime = simTime();
    int seqNum = frame->getSequenceNumber();
    //TODO
}

void BlockAcknowledgmentSendSessions::Session::framesAcknowledged(std::vector<int> ackedSequenceNumbers)
{
    //TODO
}

std::vector<Ieee80211DataOrMgmtFrame*> BlockAcknowledgmentSendSessions::Session::getFramesToRetransmit()
{
    //TODO
}

//----

BlockAcknowledgmentReceiveSessions::BlockAcknowledgmentReceiveSessions()
{
}

BlockAcknowledgmentReceiveSessions::~BlockAcknowledgmentReceiveSessions()
{
    for (auto it : receiveSessions)
        delete it.second;
}

void BlockAcknowledgmentReceiveSessions::addSession(const MACAddress& originator, int tid, int startSequenceNumber, int windowSize)
{
    Session *session = new Session();
    receiveSessions[Key(originator, tid)] = session;
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
    //TODO
}

Ieee80211DataOrMgmtFrame *BlockAcknowledgmentReceiveSessions::Session::extractFrame()
{
    //TODO
}

std::vector<Ieee80211DataOrMgmtFrame*> BlockAcknowledgmentReceiveSessions::Session::extractAndFlushUntil(int startSequenceNumber)
{
    //TODO
}

} // namespace ieee80211
} // namespace inet

