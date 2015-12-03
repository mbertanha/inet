//
// Copyright (C) 2006 Andras Varga
// Copyright (C) 2001 Eric Wu and Steve Woon, Monash University, Melbourne, Australia
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

//
// Required for MACAddress declarations
//
cplusplus {{
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h" // for ~EtherType
#include "inet/common/BitVector.h"
}}

class noncobject MACAddress;
class noncobject BitVector;

enum EtherType;

namespace inet::ieee80211;

//
// 802.11 frame type constants (type+subtype), for the "type" field of
// Ieee80211FrameControl
//
enum Ieee80211FrameType
{
    // management:
    ST_ASSOCIATIONREQUEST = 0x00;
    ST_ASSOCIATIONRESPONSE = 0x01;
    ST_REASSOCIATIONREQUEST = 0x02;
    ST_REASSOCIATIONRESPONSE = 0x03;
    ST_PROBEREQUEST = 0x04;
    ST_PROBERESPONSE = 0x05;
    ST_BEACON = 0x08;
    ST_ATIM = 0x09;
    ST_DISASSOCIATION = 0x0a;
    ST_AUTHENTICATION = 0x0b;
    ST_DEAUTHENTICATION = 0x0c;
    ST_ACTION = 0x0d;
    ST_NOACKACTION = 0x0e;

    // control (CFEND/CFEND_CFACK omitted):
    ST_PSPOLL = 0x1a;
    ST_RTS = 0x1b;
    ST_CTS = 0x1c;
    ST_ACK = 0x1d;
    ST_BLOCKACK_REQ = 0x18;
    ST_BLOCKACK     = 0x19;

    // data (CFPOLL/CFACK subtypes omitted):
    ST_DATA = 0x20;
    ST_DATA_WITH_QOS = 0x28;
    //Feedback frame for multicast tramsmission
    ST_LBMS_REQUEST = 0x30;
    ST_LBMS_REPORT = 0x31;
}

//
// Ack policy values for the 802.11 QoS control field
//
enum AckPolicy
{
    NORMAL_ACK = 0;
    NO_ACK = 1;
    NO_EXPLICIT_ACK = 2;
    BLOCK_ACK = 3;
}

//
// The common part of 802.11 frames.
//
// NOTE:
// FCS value is not explicitly modeled, but it is included in the length.
// Frame control format fields not supported by this model are omitted:
// MoreFlag, PowerMgmt, MoreData, WEP, Order.
//
packet Ieee80211Frame
{
    byteLength = LENGTH_ACK / 8;
    short type @enum(Ieee80211FrameType); // type and subtype
    bool toDS;
    bool fromDS;
    bool retry;
    bool moreFragments;
    simtime_t duration = -1; // "duration" in the Duration/ID field (-1=no duration)
    short AID = -1;          // "id" (Association ID) in the Duration/ID field (-1=no ID)
    MACAddress receiverAddress; // aka address1
    simtime_t MACArrive;    // FIXME remove it, technical data, used inside of MAC module
}

//
// Format of a 802.11 frame with address1 present, like ACK and CTS
//
packet Ieee80211OneAddressFrame extends Ieee80211Frame
{
}

//
// Format of the 802.11 ACK frame
//
packet Ieee80211ACKFrame extends Ieee80211OneAddressFrame
{
    byteLength = LENGTH_ACK / 8;
    type = ST_ACK;
}

//
// Format of a 802.11 frame with address1 and address2 present
//
packet Ieee80211TwoAddressFrame extends Ieee80211OneAddressFrame
{
    byteLength = LENGTH_RTS / 8;
    MACAddress transmitterAddress; // aka address2
}

//
// Format of the 802.11 RTS frame
//
packet Ieee80211RTSFrame extends Ieee80211TwoAddressFrame
{
    byteLength = LENGTH_RTS / 8;
    type = ST_RTS;
}

//
// Format of the 802.11 CTS frame
//
packet Ieee80211CTSFrame extends Ieee80211OneAddressFrame
{
    byteLength = LENGTH_CTS / 8;
    type = ST_CTS;
}

//
// Common base class for 802.11 data and management frames
//
packet Ieee80211DataOrMgmtFrame extends Ieee80211TwoAddressFrame
{
    byteLength = LENGTH_MGMT / 8;
    MACAddress address3;
    short fragmentNumber;
    uint16 sequenceNumber;
}

//
// Format of the 802.11 data frame
//
packet Ieee80211DataFrame extends Ieee80211DataOrMgmtFrame
{
//  The presence of the Address 4 field is determined by the setting of the To DS and From DS subfields of
//  the Frame Control field (see below). The QoS Control field is present when the QoS subfield of the Subtype
//  field is set to 1 (8.3.2.1 Data frame format)
//  FIXME: byteLength is not a constant value (DATAFRAME_HEADER_MINLENGTH / 8 = 28).
//  For example, with Address 4 and without QoS the correct length is 34 bytes.
//  Without Address 4 and QoS the correct value is indeed 28.
//  With Address 4 and QoS the correct value is indeed 36.
    byteLength = DATAFRAME_HEADER_MINLENGTH / 8;
    type = ST_DATA;         // or ST_DATA_WITH_QOS
    MACAddress address4;    // optional field, exists when toDS and fromDS are true
    uint16 qos;             // TODO nobody uses this currently, only its parts like tid below... 
    int ackPolicy @enum(AckPolicy); //FIXME in real life this is part of the optional qos field above...
    uint8 tid; //FIXME in real life this is part of the optional qos field above...
    bool aMsduPresent = false; // 8.2.4.5.9 A-MSDU Present subfield FIXME in real life this is part of the optional qos field above...
}

//TODO this can be introduced when the SNAP header 
//packet Ieee80211QoSDataFrame extends Ieee80211DataFrame
//{
//    type = ST_DATA_WITH_QOS;
//    byteLength = (DATAFRAME_HEADER_MINLENGTH + QOSCONTROL_BITS) / 8;
//    int ackPolicy @enum(AckPolicy); //FIXME in real life this is part of the optional qos field above...
//    uint8 tid;
//    // TODO: other QoS control field parts
//}

//
// 802.11 data frame with the 8-byte SNAP header (AA AA 03, 00 00 00, <2-byte ~EtherType>)
//
packet Ieee80211DataFrameWithSNAP extends Ieee80211DataFrame
{
    byteLength = DATAFRAME_HEADER_MINLENGTH / 8 + SNAP_HEADER_BYTES;
    int etherType @enum(EtherType);
}

//
// Base class for 802.11 management frames (subclasses will add frame body contents)
//
packet Ieee80211ManagementFrame extends Ieee80211DataOrMgmtFrame
{
}

// 8.3.3.13 Action frame format -- mgmt frame with subtype action
packet Ieee80211ActionFrame extends Ieee80211DataOrMgmtFrame
{
    type = ST_ACTION;
    short category;
    // TODO: subclasses will add action subfield contents
    //short action @enum(WirelessNetworkManagementAction); // action TODO: remove it
}

packet Ieee80211MsduSubframe
{
    bitLength = LENGTH_A_MSDU_SUBFRAME_HEADER;
    MACAddress sa; // source addr.
    MACAddress da; // destination addr.
    int length;
}

packet Ieee80211AMsdu
{
    Ieee80211MsduSubframe subframes[];
}

//
// Table 8-203—ADDBA Request frame Action field format -- 736p.
//
packet Ieee80211AddbaRequest extends Ieee80211ActionFrame
{
    // byteLength TODO
    category = 3; // representing Block Ack
    int blockAckAction = 0; // representing ADDBA request
    int dialogToken = 1; // nonzero, arbitrary

    // Block Ack Parameter Set field
    bool aMsduSupported; // The A-MSDU Supported subfield determines whether an A-MSDU may be carried in a QoS data MPDU sent under this Block Ack agreement.
    bool blockAckPolicy; // The Block Ack Policy subfield is set to 1 for immediate Block Ack and 0 for delayed Block Ack.
    int tid; // The TID subfield contains the value of the TC or TS for which the BlockAck is being requested.
    int bufferSize; // The Buffer Size subfield indicates the number of buffers available for this particular TID

    // Block Ack Timeout Value field
    // The Block Ack Timeout Value field contains the duration, in TUs, after which the Block Ack setup is
    // terminated, if there are no frame exchanges (see 10.5.4) within this duration using this Block Ack
    //  agreement. A value of 0 disables the timeout.
    simtime_t blockAckTimeoutValue;

    // Block Ack Starting Sequence Control
    int _fragmentNumber; // FIXME: DataOrMgmtFreme has a fragmentNumber
    int startingSequenceNumber; // the sequence number of the first MSDU

}

packet Ieee80211AddbaResponse extends Ieee80211ActionFrame
{
    // byteLength TODO
    category = 3; // representing Block Ack
    int blockAckAction = 1; // representing ADDBA response
    int dialogToken = 1; // The Dialog Token field value is copied from the corresponding received ADDBA Request frame.
    int statusCode; // The Status Code field is used in a response management frame to indicate the success or failure of a requested operation.

    // Block Ack Parameter Set field
    bool aMsduSupported; // The A-MSDU Supported subfield determines whether an A-MSDU may be carried in a QoS data MPDU sent under this Block Ack agreement.
    bool blockAckPolicy; // The Block Ack Policy subfield is set to 1 for immediate Block Ack and 0 for delayed Block Ack.
    int tid; // The TID subfield contains the value of the TC or TS for which the BlockAck is being requested.
    int bufferSize; // The Buffer Size subfield indicates the number of buffers available for this particular TID

    // Block Ack Timeout Value field
    // The Block Ack Timeout Value field contains the duration, in TUs, after which the Block Ack setup is
    // terminated, if there are no frame exchanges (see 10.5.4) within this duration using this Block Ack
    //  agreement. A value of 0 disables the timeout.
    simtime_t blockAckTimeoutValue;
}

//
// 8.3.1.9 BlockAck frame format -- 410p.
//
packet Ieee80211BlockAck extends Ieee80211OneAddressFrame
{
    // byteLength TODO
    // The RA field of the BlockAck frame is the address of the recipient STA that requested the Block Ack.
    // The TA field is the address of the STA transmitting the BlockAck frame.
    type = ST_BLOCKACK;

    // BA Control
    bool blockAckPolicy; // Table 8-17—BA Ack Policy subfield: 0-Normal Acknowledgment, 1-No Acknowledgment.

    // Table 8-18—BlockAck frame variant encoding
    bool multiTid;
    bool compressedBitmap;
}

//
// 8.3.1.9.2 Basic BlockAck variant -- 411p.
//
packet Ieee80211BasicBlockAck extends Ieee80211BlockAck
{
    // byteLength TODO
    multiTid = 0;
    compressedBitmap = 0;

    // The BA Information field of the Basic BlockAck frame comprises the Block Ack Starting Sequence Control
    // subfield and the Block Ack Bitmap subfield, as shown in Figure 8-26.

    int fragmentNumber;
    int startingSequenceNumber;
    // The Block Ack Bitmap subfield is 128 octets in length and is used to indicate the received status of up to
    // 64 MSDUs. Bit position n of the Block Ack bitmap, if equal to 1, acknowledges receipt of an MPDU with an
    // MPDU sequence control value equal to (Block Ack Starting Sequence Control + n). Bit position n of the
    // Block Ack bitmap, if equal to 0, indicates that an MPDU with MPDU sequence control value equal to
    // (Block Ack Starting Sequence Control + n) has not been received. Each of the MPDU Sequence Control
    // field and Block Ack Starting Sequence Control subfield values are treated as a 16-bit unsigned integer. For
    // unused fragment numbers of an MSDU, the corresponding bits in the bitmap are set to 0.
    BitVector blockAckBitmap[64]; // 64 * 16 bitmatrix
}

//
// 8.3.1.9.3 Compressed BlockAck variant
//
packet Ieee80211CompressedBlockAck extends Ieee80211BlockAck
{
    // byteLength TODO
    multiTid = 0;
    compressedBitmap = 1;

    // The BA Information field of the Compressed BlockAck frame comprises the Block Ack Starting Sequence
    // Control subfield and the Block Ack Bitmap subfield, as shown in Figure 8-27.

    int fragmentNumber;
    int startingSequenceNumber;
    BitVector blockAckBitmap; // 64 bits
}

//
// 8.3.1.9.4 Multi-TID BlockAck variant
//
packet Ieee80211MultiTidBlockAck extends Ieee80211BlockAck
{
    // byteLength TODO
    multiTid = 1;
    compressedBitmap = 1;

    // TODO: unimplemented
}
