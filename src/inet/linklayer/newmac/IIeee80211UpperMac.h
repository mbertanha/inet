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

#ifndef __INET_IIEEE80211UPPERMAC_H
#define __INET_IIEEE80211UPPERMAC_H

#include "inet/common/INETDefs.h"
#include "IIeee80211MacTx.h"

namespace inet {
namespace ieee80211 {

class IIeee80211UpperMacContext;
class Ieee80211Frame;
class Ieee80211DataOrMgmtFrame;

class IIeee80211UpperMac
{
    public:
        virtual void setContext(IIeee80211UpperMacContext *context) = 0;
        virtual void upperFrameReceived(Ieee80211DataOrMgmtFrame *frame) = 0;
        virtual void lowerFrameReceived(Ieee80211Frame *frame) = 0;
        virtual void transmissionComplete(IIeee80211MacTx::ICallback *callback, int txIndex) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

