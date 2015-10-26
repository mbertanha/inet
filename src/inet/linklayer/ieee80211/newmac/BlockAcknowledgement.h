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

namespace inet {
namespace ieee80211 {

class Ieee80211DataOrMgmtFrame;

/**
 * Data structure for managing the sender side of a Block Acknowledgment session.
 */
//TODO: deal with fragmentation too? (use <seq,frag> instead of <seq>)
class INET_API BlockAcknowledgmentSendSessions
{
    public:
        class INET_API Session {
            private:
                int beginSequenceNumber;
                int windowSize;
                std::vector<Ieee80211DataOrMgmtFrame*> resendBuffer;
                simtime_t lastUseTime;
            public:
                Session();
                virtual ~Session();
                virtual void addFrameToSend(Ieee80211DataOrMgmtFrame *frame);
                virtual void framesAcknowledged(std::vector<int> ackedSequenceNumbers); // better, use a bitmap like the one in the BA frame
                virtual std::vector<Ieee80211DataOrMgmtFrame*> getFramesToRetransmit();
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

    public:
        BlockAcknowledgmentSendSessions();
        virtual ~BlockAcknowledgmentSendSessions();

        virtual void addSession(const MACAddress& responder, int tid, int startSequenceNumber, int windowSize);
        virtual void deleteSession(const MACAddress& responder, int tid);
        virtual Session *getSession(const MACAddress& responder, int tid);

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
                int windowSize;
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
        BlockAcknowledgmentReceiveSessions();
        virtual ~BlockAcknowledgmentReceiveSessions();

        virtual void addSession(const MACAddress& originator, int tid, int startSequenceNumber, int windowSize);
        virtual void deleteSession(const MACAddress& originator, int tid);
        virtual Session *getSession(const MACAddress& originator, int tid);

};

} // namespace ieee80211
} // namespace inet

#endif

