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

#ifndef __INET_BLOCKACKSUPPORT_H
#define __INET_BLOCKACKSUPPORT_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class Ieee80211DataOrMgmtFrame;

/**
 * Data structure for managing the sender side of a Block Acknowledgment session.
 */
class INET_API BlockAcknowledgmentSendSessions
{
    public:
        class INET_API Session {
            private:
                bool aMsduSupported;
                int startingSequenceNumber;
                int txWindowSize;
                std::vector<Ieee80211DataOrMgmtFrame*>  transmittedFrames;
                std::vector<Ieee80211DataOrMgmtFrame*> framesToRetransmit;
                // cMessage *inactivityTimer = nullptr; TODO: 10.5.3.3 Procedure at the recipient of the DELBA frame

            public:
                Session(Ieee80211AddbaRequest *request, Ieee80211AddbaResponse *response);
                virtual ~Session();
                virtual void collectFramesToRetransmit(Ieee80211BasicBlockAck *basicBlockAck);
                virtual void collectFramesToRetransmit(Ieee80211CompressedBlockAck *basicBlockAck);
                virtual void addToTransmittedFrames(Ieee80211DataOrMgmtFrame *frame);
                virtual std::vector<Ieee80211DataOrMgmtFrame*>& getFramesToRetransmit() { return framesToRetransmit; }
        };

    protected:
        struct Key {
            MACAddress address;
            uint8_t tid;
            Key(const MACAddress& address, uint8_t tid) : address(address), tid(tid) {}
            bool operator == (const Key& o) const { return address == o.address && tid == o.tid; }
            bool operator < (const Key& o) const { return address < o.address || (address == o.address && tid < o.tid); }
        };

        typedef std::map<Key,Session*> SendSessions;
        SendSessions sendSessions;

    protected:
        virtual void addSession(Ieee80211AddbaRequest *request, Ieee80211AddbaResponse *response);
        virtual void deleteSession(const MACAddress& responder, int tid);
        virtual Session *getSession(const MACAddress& responder, int tid);

    public:
        BlockAcknowledgmentSendSessions();
        virtual ~BlockAcknowledgmentSendSessions();

        virtual void blockAckReceived(Ieee80211BlockAck *blockAck);
        virtual void delbaReceived(Ieee80211Delba *delba);
        virtual bool addbaResponseReceived(Ieee80211AddbaRequest *request, Ieee80211AddbaResponse *response);
        virtual std::vector<Ieee80211DataOrMgmtFrame*>& getFramesToRetransmit(Ieee80211BlockAck *blockAck); // Note: insert them into the TX queue in reverse order.
};

/**
 * Data structure for managing the receiver side of a Block Acknowledgment session.
 */
class INET_API BlockAcknowledgmentReceiveSessions
{
    public:
        class Session {
            private:
                int beginSequenceNumber;
                int windowSize; // buffer size?
                std::vector<Ieee80211DataOrMgmtFrame*> reorderBuffer;
                simtime_t lastUseTime;

            public:
                Session();
                virtual ~Session();
                virtual void addReceivedFrame(Ieee80211DataOrMgmtFrame *frame);  // adds to reorderBuffer
                virtual Ieee80211DataOrMgmtFrame *extractFrame();  // returns nullptr if we arrived at a hole in the reorder buffer
                virtual std::vector<Ieee80211DataOrMgmtFrame*> extractAndFlushUntil(int startSequenceNumber); // ignore holes
        };

    protected:
        struct Key {
            MACAddress address;
            uint8_t tid;
            Key(const MACAddress& address, uint8_t tid) : address(address), tid(tid) {}
            bool operator == (const Key& o) const { return address == o.address && tid == o.tid; }
            bool operator < (const Key& o) const { return address < o.address || (address == o.address && tid < o.tid); }
        };

        typedef std::map<Key,Session*> ReceiveSessions;
        ReceiveSessions receiveSessions;

    public:
        BlockAcknowledgmentReceiveSessions(Ieee80211AddbaRequest *request, Ieee80211AddbaResponse *response);
        virtual ~BlockAcknowledgmentReceiveSessions();

        virtual bool addSession(Ieee80211AddbaRequest *request, Ieee80211AddbaResponse *response);
        virtual void deleteSession(const MACAddress& originator, int tid);
        virtual Session *getSession(const MACAddress& originator, int tid);

};

} // namespace ieee80211
} // namespace inet

#endif

