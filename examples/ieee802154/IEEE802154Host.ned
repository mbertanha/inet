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

package inet.examples.ieee802154;

import inet.applications.generic.IPvXTrafGen;
import inet.node.inet.StandardHost;


module IEEE802154Host extends StandardHost
{
    parameters:
        numRadios = default(1);
        wlan[0].typename = default("Ieee802154NarrowbandNic");
        wlan[0].radio.transmitter.preambleDuration = 0s;

        trafficgen.protocol = 59;
        trafficgen.packetLength = 10B;

        @statistic[rcvdPkSrc](title="packets received per source"; source=rcvdPk; record="groupCount(sourceAddr)"; interpolationmode=none);

        @display("i=device/accesspoint;bgb=,-");
        forwarding = default(true);

    submodules:
        trafficgen: IPvXTrafGen {
            @display("p=149,287");
        }

    connections:
        networkLayer.transportOut++ --> trafficgen.ipIn;
        networkLayer.transportIn++ <-- trafficgen.ipOut;
}
