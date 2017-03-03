/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jaume Nin <jnin@cttc.cat>
 *         Nicola Baldo <nbaldo@cttc.cat>
 */


#include "ns3/epc-enb-application.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv4.h"
#include "ns3/inet-socket-address.h"
#include "ns3/uinteger.h"
#include "lte-enb-rrc.h"
#include "epc-gtpu-header.h"
#include "eps-bearer-tag.h"
#include "ns3/net-device.h"
#include "ns3/ipv4-flow-probe.h"
#include "ns3/flow-monitor.h"
#include "ns3/routerlayer.h"
#include "ns3/udp-header.h"
#include "ns3/seq-ts-header.h"
#include "ns3/tcp-header.h"
#include <list>
#include <map>
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/ipv4-flow-probe.h"
#include <fstream>
namespace ns3 {

class Ipv4FlowProbeTag : public Tag
{
public:
	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (TagBuffer buf) const;
	virtual void Deserialize (TagBuffer buf);
	virtual void Print (std::ostream &os) const;
	Ipv4FlowProbeTag ();
	/**
	 * \brief Consructor
	 * \param flowId the flow identifier
	 * \param packetId the packet identifier
	 * \param packetSize the packet size
	 */
	Ipv4FlowProbeTag (uint32_t flowId, uint32_t packetId, uint32_t packetSize);
	/**
	 * \brief Set the flow identifier
	 * \param flowId the flow identifier
	 */
	void SetFlowId (uint32_t flowId);
	/**
	 * \brief Set the packet identifier
	 * \param packetId the packet identifier
	 */
	void SetPacketId (uint32_t packetId);
	/**
	 * \brief Set the packet size
	 * \param packetSize the packet size
	 */
	void SetPacketSize (uint32_t packetSize);
	/**
	 * \brief Set the flow identifier
	 * \returns the flow identifier
	 */
	uint32_t GetFlowId (void) const;
	/**
	 * \brief Set the packet identifiertotal ul lte throughput 2.03036
	 *
	 * \returns the packet identifier
	 */
	uint32_t GetPacketId (void) const;
	/**
	 * \brief Get the packet size
	 * \returns the packet size
	 */
	uint32_t GetPacketSize (void) const;
private:
	uint32_t m_flowId;      //!< flow identifier
	uint32_t m_packetId;    //!< packet identifier
	uint32_t m_packetSize;  //!< packet size

};

TypeId
Ipv4FlowProbeTag::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::Ipv4FlowProbeTag")
    																																																																																																																																																																																.SetParent<Tag> ()
    																																																																																																																																																																																.AddConstructor<Ipv4FlowProbeTag> ()
    																																																																																																																																																																																;
	return tid;
}
TypeId
Ipv4FlowProbeTag::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}
uint32_t
Ipv4FlowProbeTag::GetSerializedSize (void) const
{
	return 4 + 4 + 4;
}
void
Ipv4FlowProbeTag::Serialize (TagBuffer buf) const
{
	buf.WriteU32 (m_flowId);
	buf.WriteU32 (m_packetId);
	buf.WriteU32 (m_packetSize);
}
void
Ipv4FlowProbeTag::Deserialize (TagBuffer buf)
{
	m_flowId = buf.ReadU32 ();
	m_packetId = buf.ReadU32 ();
	m_packetSize = buf.ReadU32 ();
}
void
Ipv4FlowProbeTag::Print (std::ostream &os) const
{
	os << "FlowId=" << m_flowId;
	os << "PacketId=" << m_packetId;
	os << "PacketSize=" << m_packetSize;
}
Ipv4FlowProbeTag::Ipv4FlowProbeTag ()
: Tag ()
{
}

Ipv4FlowProbeTag::Ipv4FlowProbeTag (uint32_t flowId, uint32_t packetId, uint32_t packetSize)
: Tag (), m_flowId (flowId), m_packetId (packetId), m_packetSize (packetSize)
{
}

void
Ipv4FlowProbeTag::SetFlowId (uint32_t id)
{
	m_flowId = id;
}
void
Ipv4FlowProbeTag::SetPacketId (uint32_t id)
{
	m_packetId = id;
}
void
Ipv4FlowProbeTag::SetPacketSize (uint32_t size)
{
	m_packetSize = size;
}
uint32_t
Ipv4FlowProbeTag::GetFlowId (void) const
{
	return m_flowId;
}
uint32_t
Ipv4FlowProbeTag::GetPacketId (void) const
{
	return m_packetId;
}
uint32_t
Ipv4FlowProbeTag::GetPacketSize (void) const
{
	return m_packetSize;
}
NS_LOG_COMPONENT_DEFINE ("EpcEnbApplication")
;uint64_t nmb=0;
//uint32_t pkt_cnt=0;
uint32_t packtno=0;
uint16_t temp_port=0;
uint32_t EpcEnbApplication::onlylte=1;
uint64_t EpcEnbApplication::holding_time=0;
std::map<Mac48Address,Ipv4Address> EpcEnbApplication::macipmap;
//std::map<Mac48Address,std::map<double,double> > EpcEnbApplication::snr_per;
std::map<Mac48Address,double> EpcEnbApplication::rssi;
uint32_t current_value=0,past_value=0;
uint64_t past=0;
uint32_t EpcEnbApplication::holded_packets=0;
int count=-1;
//THomas -- User defined class for collecting stats for TCP Window
RouterLayer router;
double lte_usedrbs=0, wifi_load;
struct time_info{
	uint32_t source_port;
	int64_t past;
	SequenceNumber32 last_seqno;
	uint32_t dest_port;
	SequenceNumber32 window_size;
}s[10];
struct pkt_info {
	uint16_t source_port;
	uint16_t dest_port;
	uint32_t seqno;
	uint32_t recvbytes;
	double pktsendtime;
};
struct udp_pktinfo {
	uint32_t fid;
	uint32_t pid;
	uint32_t recvbytes;
	double pktsendtime;
};
struct flowtable_info {
	uint32_t current_interface;
	double Qos_Enabled;
	double delay;
	double throughput;
	Ipv4Address ip;
	EpsBearer epsbearer;
	double appdatarate;
	uint32_t protocolno;
	double new_flow;
};
struct sinrperinfo {
	double lteval;
	double wifival;
};
std::list<pkt_info> pkt_list;
std::list<udp_pktinfo> udp_pktlist;
std::map<uint16_t, std::map<uint16_t,double> > lastpktrecv;
std::map<uint16_t, std::map<uint16_t,double> > firstpktsend;
std::map<uint16_t, std::map<uint16_t,double> > total_delay;
std::map<uint16_t, std::map<uint16_t,uint64_t> > RecvBytes;
std::map<uint32_t, double > udp_recvbytes;
std::map<uint32_t, double > udp_sendbytes;
std::map<uint32_t, double > udp_lastpktrecv;
std::map<uint32_t, double > udp_firstpktsend;
std::map<uint32_t, double > udp_totaldelay;
std::map<uint32_t, flowtable_info> flowtable;
std::map<Ipv4Address, sinrperinfo> sinr_table;
std::map<Ipv4Address, sinrperinfo> per_table;
std::map<uint32_t, Ipv4Address> rntiipmap;
std::map<Ipv4Address, EpsBearer> ipbearermap;
std::map<Ipv4Address, double> ipdatarate;
std::map<uint16_t, uint32_t>  Past_Ack_number_sent;
std::map<uint16_t,uint32_t> Last_Received_data_sequence_number;
int flag=0;
int mxpt=0;
int fair=0; 
bool tcp=false;
double uplinkthrpt_lte=0;
double wifitpt=0;
double EpcEnbApplication::uplinkthrpt_wifi=0;
double uplinkthrpt=0;
double EpcEnbApplication::uplinksendpkt=0;
uint32_t EpcEnbApplication::lte_fraction=5;
uint32_t EpcEnbApplication::wifi_fraction=5;
bool EpcEnbApplication::skip_ack=false;
bool EpcEnbApplication::DIDA=false;
bool EpcEnbApplication::boost_ack=false;
double ullastpktrecv=0;
uint32_t localcounter=1;
uint32_t estimated_window=0;
uint64_t local_timer=0;
uint32_t testing_counter;

std::ofstream total ("logs/exp3_4.txt");
std::ofstream flows ("logs/exp3flows_4.txt");

double duration=0.3;
uint32_t shiftedflowid=-1;
double phydatarate=15;
//bool flip=true;
/*void EpcEnbApplication::setobject(Ptr<EpcEnbApplication> epcenb) {
	RouterLayer::routerlayerepcenbapp_object=epcenb;
}*/

EpcEnbApplication::EpsFlowId_t::EpsFlowId_t ()
{
}

EpcEnbApplication::EpsFlowId_t::EpsFlowId_t (const uint16_t a, const uint8_t b)
: m_rnti (a),
  m_bid (b)
{
}

bool
operator == (const EpcEnbApplication::EpsFlowId_t &a, const EpcEnbApplication::EpsFlowId_t &b)
{
	return ( (a.m_rnti == b.m_rnti) && (a.m_bid == b.m_bid) );
}

bool
operator < (const EpcEnbApplication::EpsFlowId_t& a, const EpcEnbApplication::EpsFlowId_t& b)
{
	return ( (a.m_rnti < b.m_rnti) || ( (a.m_rnti == b.m_rnti) && (a.m_bid < b.m_bid) ) );
}


TypeId
EpcEnbApplication::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::EpcEnbApplication")
    																																																																																																																																																																																								.SetParent<Object> ();
	return tid;
}

void
EpcEnbApplication::DoDispose (void)
{
	NS_LOG_FUNCTION (this);
	m_lteSocket = 0;
	m_s1uSocket = 0;
	delete m_s1SapProvider;
	delete m_s1apSapEnb;
}


EpcEnbApplication::EpcEnbApplication (Ptr<Socket> lteSocket, Ptr<Socket> s1uSocket, Ipv4Address enbS1uAddress, Ipv4Address sgwS1uAddress, uint16_t cellId)
: m_lteSocket (lteSocket),
  m_s1uSocket (s1uSocket),
  m_enbS1uAddress (enbS1uAddress),
  m_sgwS1uAddress (sgwS1uAddress),
  m_gtpuUdpPort (2152), // fixed by the standard
  m_s1SapUser (0),
  m_s1apSapMme (0),
  m_cellId (cellId)
{
	NS_LOG_FUNCTION (this << lteSocket << s1uSocket << sgwS1uAddress);
	m_s1uSocket->SetRecvCallback (MakeCallback (&EpcEnbApplication::RecvFromS1uSocket, this));
	m_lteSocket->SetRecvCallback (MakeCallback (&EpcEnbApplication::RecvFromLteSocket, this));
	m_s1SapProvider = new MemberEpcEnbS1SapProvider<EpcEnbApplication> (this);
	m_s1apSapEnb = new MemberEpcS1apSapEnb<EpcEnbApplication> (this);
}


EpcEnbApplication::~EpcEnbApplication (void)
{
	NS_LOG_FUNCTION (this);
}

EpcEnbApplication::EpcEnbApplication (void)
{
	NS_LOG_FUNCTION (this);
}

void 
EpcEnbApplication::SetS1SapUser (EpcEnbS1SapUser * s)
{
	m_s1SapUser = s;
}


EpcEnbS1SapProvider* 
EpcEnbApplication::GetS1SapProvider ()
{
	return m_s1SapProvider;
}

void 
EpcEnbApplication::SetS1apSapMme (EpcS1apSapMme * s)
{
	m_s1apSapMme = s;
}


EpcS1apSapEnb* 
EpcEnbApplication::GetS1apSapEnb ()
{
	return m_s1apSapEnb;
}

void 
EpcEnbApplication::DoInitialUeMessage (uint64_t imsi, uint16_t rnti)
{
	NS_LOG_FUNCTION (this);
	// side effect: create entry if not exist
	m_imsiRntiMap[imsi] = rnti;
	m_s1apSapMme->InitialUeMessage (imsi, rnti, imsi, m_cellId);
}

void 
EpcEnbApplication::DoPathSwitchRequest (EpcEnbS1SapProvider::PathSwitchRequestParameters params)
{
	NS_LOG_FUNCTION (this);
	uint16_t enbUeS1Id = params.rnti;
	uint64_t mmeUeS1Id = params.mmeUeS1Id;
	uint64_t imsi = mmeUeS1Id;
	// side effect: create entry if not exist
	m_imsiRntiMap[imsi] = params.rnti;

	uint16_t gci = params.cellId;
	std::list<EpcS1apSapMme::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList;
	for (std::list<EpcEnbS1SapProvider::BearerToBeSwitched>::iterator bit = params.bearersToBeSwitched.begin ();
			bit != params.bearersToBeSwitched.end ();
			++bit)
	{
		EpsFlowId_t flowId;
		flowId.m_rnti = params.rnti;
		flowId.m_bid = bit->epsBearerId;
		uint32_t teid = bit->teid;

		EpsFlowId_t rbid (params.rnti, bit->epsBearerId);
		// side effect: create entries if not exist
		m_rbidTeidMap[params.rnti][bit->epsBearerId] = teid;
		m_teidRbidMap[teid] = rbid;

		EpcS1apSapMme::ErabSwitchedInDownlinkItem erab;
		erab.erabId = bit->epsBearerId;
		erab.enbTransportLayerAddress = m_enbS1uAddress;
		erab.enbTeid = bit->teid;

		erabToBeSwitchedInDownlinkList.push_back (erab);
	}
	m_s1apSapMme->PathSwitchRequest (enbUeS1Id, mmeUeS1Id, gci, erabToBeSwitchedInDownlinkList);
}

void 
EpcEnbApplication::DoUeContextRelease (uint16_t rnti)
{
	NS_LOG_FUNCTION (this << rnti);
	std::map<uint16_t, std::map<uint8_t, uint32_t> >::iterator rntiIt = m_rbidTeidMap.find (rnti);
	if (rntiIt != m_rbidTeidMap.end ())
	{
		for (std::map<uint8_t, uint32_t>::iterator bidIt = rntiIt->second.begin ();
				bidIt != rntiIt->second.end ();
				++bidIt)
		{
			uint32_t teid = bidIt->second;
			m_teidRbidMap.erase (teid);
		}
		m_rbidTeidMap.erase (rntiIt);
	}

}

void 
EpcEnbApplication::DoInitialContextSetupRequest (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<EpcS1apSapEnb::ErabToBeSetupItem> erabToBeSetupList)
{
	NS_LOG_FUNCTION (this);

	for (std::list<EpcS1apSapEnb::ErabToBeSetupItem>::iterator erabIt = erabToBeSetupList.begin ();
			erabIt != erabToBeSetupList.end ();
			++erabIt)
	{
		// request the RRC to setup a radio bearer

		uint64_t imsi = mmeUeS1Id;
		std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
		NS_ASSERT_MSG (imsiIt != m_imsiRntiMap.end (), "unknown IMSI");
		uint16_t rnti = imsiIt->second;

		struct EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params;
		params.rnti = rnti;
		params.bearer = erabIt->erabLevelQosParameters;
		params.bearerId = erabIt->erabId;
		params.gtpTeid = erabIt->sgwTeid;
		m_s1SapUser->DataRadioBearerSetupRequest (params);

		EpsFlowId_t rbid (rnti, erabIt->erabId);
		// side effect: create entries if not exist
		m_rbidTeidMap[rnti][erabIt->erabId] = params.gtpTeid;
		m_teidRbidMap[params.gtpTeid] = rbid;

	}
}

void 
EpcEnbApplication::DoPathSwitchRequestAcknowledge (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t gci, std::list<EpcS1apSapEnb::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList)
{
	NS_LOG_FUNCTION (this);
	////////std::cout<<"RecvFromLteSocket"<<std::endl;
	uint64_t imsi = mmeUeS1Id;
	std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
	NS_ASSERT_MSG (imsiIt != m_imsiRntiMap.end (), "unknown IMSI");
	uint16_t rnti = imsiIt->second;
	EpcEnbS1SapUser::PathSwitchRequestAcknowledgeParameters params;
	params.rnti = rnti;
	m_s1SapUser->PathSwitchRequestAcknowledge (params);
}

void 
EpcEnbApplication::RecvFromLteSocket (Ptr<Socket> socket)
{
	//std::cout << "-------------------------------- start recvfromltesocket--------------------------------" << std::endl;

	NS_LOG_FUNCTION (this);
	NS_ASSERT (socket == m_lteSocket);
	Ptr<Packet> packet = socket->Recv ();
	//////std::cout << "pkt size " <<packet->GetSize() <<std::endl;
	uplinkthrpt_lte+=packet->GetSize();
	//////std::cout<<"RecvFromLteSocket "<<std::endl;
	SocketAddressTag satag;
	packet->RemovePacketTag (satag);
	EpsBearerTag tag;
	bool found = packet->RemovePacketTag (tag);
	NS_ASSERT (found);
	uint16_t rnti = tag.GetRnti ();
	uint8_t bid = tag.GetBid ();
	//CRAN
	////////std::cout<<"Bearer ID: "<<(uint32_t)bid<<std::endl;
	//packet->Print(////std::cout);
	//////std::cout << std::endl;
	Ipv4Header ipv4Header;
	packet->RemoveHeader(ipv4Header);
	TcpHeader seqheader;
	packet->PeekHeader(seqheader);
	packet->AddHeader(ipv4Header);
	/*if(seqheader.GetAckNumber().GetValue()!=1) {
		for(std::list<pkt_info>::iterator it=pkt_list.begin();it!=pkt_list.end();) {
			//////std::cout << "src port " << seqheader.GetSourcePort() << "it dest porrt " <<it->dest_port  << std::endl;
			//////std::cout << seqheader.GetAckNumber().GetValue() << " " << it->seqno  << std::endl;
			if(seqheader.GetSourcePort()==it->dest_port && seqheader.GetDestinationPort()==it->source_port && (seqheader.GetAckNumber().GetValue()>=it->seqno || seqheader.GetAckNumber().GetValue()>=it->seqno+1)) {

				//////std::cout << "--------------------got Ack---------------------------------------------\n\n";
				total_delay[it->source_port][it->dest_port]+=Simulator::Now ().GetSeconds()-it->pktsendtime;//.GetMilliSeconds()-it->pktsendtime;
				lastpktrecv[it->source_port][it->dest_port]=Simulator::Now ().GetSeconds();//.GetMilliSeconds();
				if(seqheader.GetAckNumber().GetValue()==it->seqno || seqheader.GetAckNumber().GetValue()==it->seqno+1) {
					RecvBytes[it->source_port][it->dest_port]+=it->recvbytes+packet->GetSize();
				} else if(seqheader.GetAckNumber().GetValue()>it->seqno) {
					RecvBytes[it->source_port][it->dest_port]+=it->recvbytes;

				}
				//////std::cout << "src port " << seqheader.GetSourcePort() << "it dest porrt " <<seqheader.GetDestinationPort() << " stored seqno " <<  it->seqno << " pkt seqno " << seqheader.GetAckNumber().GetValue() << " recv bytes " << RecvBytes[it->source_port][it->dest_port] << std::endl;
				it=pkt_list.erase(it);
			} else {
				it++;
			}
		}
	}*/
	//////std::cout <<"UL src" << seqheader.GetSourcePort() << "dest " << seqheader.GetDestinationPort() <<" " << (uint32_t)ipv4Header.GetIdentification()<< std::endl;
	//===============Insering source port  in map====================
	////////std::cout << "==============================" << (uint32_t)seqheader.GetAckNumber().GetValue() << std::endl;
	//int i=0;
	/*int64_t present=Simulator::Now ().GetMilliSeconds();
	int flag_updated=0;
	for(i=0; i<10;i++){
		if(s[i].source_port==seqheader.GetSourcePort() && s[i].dest_port==seqheader.GetDestinationPort())
		{

			if((present-s[i].past)>=1 && seqheader.GetSequenceNumber()>=s[i].last_seqno)
			{
				////////std::cout<<"inside condition "<<seqheader.GetSourcePort()<<"," <<seqheader.GetDestinationPort()<<","<< seqheader.GetSequenceNumber()-s[i].last_seqno<<std::endl;
				s[i].past=  present;
				s[i].window_size=s[i].last_seqno;
				s[i].last_seqno=seqheader.GetSequenceNumber();
			}
			flag_updated=1;
			break;
		}
		if(s[i].source_port==0 && s[i].dest_port==0){
			//	New port number is found
			break;
		}
	}
	if(i<10&& flag_updated==0)
	{
		for(i=0; i<10;i++){
			if(s[i].source_port==0 && s[i].dest_port==0)
			{
				s[i].source_port=seqheader.GetSourcePort();
				s[i].past=0;
				s[i].dest_port=seqheader.GetDestinationPort();
				//s[i].last_seqno=seqheader.GetSequenceNumber();
				break;
			}

		}

	}*/
	//	for(i=0;i<8;i++) {
	//		////////std::cout << s[i].source_port <<","<<s[i].dest_port <<","<<s[i].last_seqno-s[i].window_size << std::endl;
	//	}
	//========================
	////////std::cout<<"Sequence Header ACK = "<<seqheader.GetAckNumber()<<std::endl;
	////////std::cout<<"SourcePort no"<<seqheader.GetSourcePort()<<", Destination port"<<seqheader.GetDestinationPort()<<std::endl;

	////////std::cout<<"UE address = "<<ipv4Header.GetSource()<<"    Remote host address = "<<ipv4Header.GetDestination();
	//BEARER_ID_UE_IP_MAP[bid]=ipv4Header.GetSource();
	//--------------
	NS_LOG_LOGIC ("received packet with RNTI=" << (uint32_t) rnti << ", BID=" << (uint32_t)  bid);
	std::map<uint16_t, std::map<uint8_t, uint32_t> >::iterator rntiIt = m_rbidTeidMap.find (rnti);
	if (rntiIt == m_rbidTeidMap.end ())
	{
		//std::cout<<"UE context not found, discarding packet"<<std::endl;
		NS_LOG_WARN ("UE context not found, discarding packet");
	}
	else
	{
		////std::cout<<"send to s1usocket"<<std::endl;
		std::map<uint8_t, uint32_t>::iterator bidIt = rntiIt->second.find (bid);
		NS_ASSERT (bidIt != rntiIt->second.end ());
		uint32_t teid = bidIt->second;
		SendToS1uSocket (packet, teid);
	}
	// //////std::cout<<"Context Release Rnti= "<<rnti<<std::endl;

}

void 
EpcEnbApplication::RecvFromS1uSocket (Ptr<Socket> socket)
{
	//////std::cout << "flag " << flag << std::endl;
	if(flag==0) {
		////std::cout<<"RecvFromS1uSocket";
		flag=1;
		resetfunc();
	}
	////

	NS_LOG_FUNCTION (this << socket);
	NS_ASSERT (socket == m_s1uSocket);
	Ptr<Packet> packet = socket->Recv ();
	GtpuHeader gtpu;
	packet->RemoveHeader (gtpu);
	uint32_t teid = gtpu.GetTeid ();

	std::map<uint32_t, EpsFlowId_t>::iterator it = m_teidRbidMap.find (teid);
	// uint32_t tmp_id=it->first;
	// EpsFlowId_t flow_id=it->second;

	// //////std::cout<<"ID = "<<tmp_id <<" Transaction ID = "<<(uint32_t)flow_id.m_bid<<std::endl;
	//  NS_ASSERT (it != m_teidRbidMap.end ());

	/// \internal
	/// Workaround for \bugid{231}
	SocketAddressTag tag;
	packet->RemovePacketTag (tag);
	// CRAN
	SendToLteSocket (packet, it->second.m_rnti, it->second.m_bid);
}
void EpcEnbApplication::udpstats(uint32_t fid,uint32_t pid, double recvbytes, double time) {

	for(std::list<udp_pktinfo>::iterator it=udp_pktlist.begin();it!=udp_pktlist.end();) {
		if(it->fid==fid && it->pid==pid) {
			//std::cout << it->fid  << " recvbytes " << recvbytes << std::endl;
			udp_recvbytes[fid]+=recvbytes;
			udp_lastpktrecv[fid]=time;
			udp_totaldelay[fid]=time-it->pktsendtime;
			it=udp_pktlist.erase(it);
			break;
		} else {
			it++;
		}
	}
	//////std::cout << "count " << count << std::endl;
}

void EpcEnbApplication::addusedrbs(double usedrbs) {
	lte_usedrbs+=usedrbs;
}

void EpcEnbApplication::updatewifiload(double load) {
	if(load>0)
		wifi_load=load;
	else
		wifi_load=0;
}
void EpcEnbApplication::addbearer(Ipv4Address ip, EpsBearer bearer,double interpacketinterval) {
	////std::cout << interpacketinterval << std::endl;
	ipbearermap[ip]=bearer;
	ipdatarate[ip]=interpacketinterval;
}
void EpcEnbApplication::updateltesinrper(std::string context, uint16_t cellid,uint16_t rnti, double rsrp, double avgsinr) {
	Ipv4Address ip=rntiipmap[rnti];
	//////std::cout << "ip " << ip << std::endl;
	if(sinr_table.find(ip)!=sinr_table.end()) {
		sinrperinfo s=sinr_table[ip];
		s.lteval=avgsinr;
		sinr_table[ip]=s;

	} else {
		sinrperinfo s;
		s.lteval=avgsinr;
		s.wifival=0;
		sinr_table[ip]=s;

	}
}
void EpcEnbApplication::updatewifisinrper(double sinr,double per, Mac48Address macaddr) {
	if(macipmap.find(macaddr)!=macipmap.end()) {

		Ipv4Address ip=macipmap[macaddr];
		if(sinr_table.find(ip)!=sinr_table.end()) {
			sinrperinfo s=sinr_table[ip];
			s.wifival=sinr;
			sinr_table[ip]=s;

		} else {
			sinrperinfo s;
			s.wifival=sinr;
			s.lteval=0;
			sinr_table[ip]=s;

		}
		if(per_table.find(ip)!=per_table.end()) {
			sinrperinfo s=per_table[ip];
			s.wifival=per;
			per_table[ip]=s;
		} else {
			sinrperinfo s;
			s.wifival=per;
			s.lteval=0;
			per_table[ip]=s;
		}

	}
}
void 
EpcEnbApplication::SendToLteSocket (Ptr<Packet> packet, uint16_t rnti, uint8_t bid)
{
	if(flag==0) {
		//////std::cout << "-------------------------------- start sendtoltesocket--------------------------------" << std::endl;
		flag=1;
		resetfunc();
	}
	Ipv4Header ipv4Header;
	packet->RemoveHeader(ipv4Header);
	Ipv4FlowProbeTag flowprobe;
	bool found;
	rntiipmap[rnti]=ipv4Header.GetDestination();
	found =packet->PeekPacketTag(flowprobe);
	if((uint32_t)ipv4Header.GetProtocol()==6)
		tcp=true;
	else
		tcp=false;
	TcpHeader seqheader;
	packet->PeekHeader(seqheader);
	uint32_t desport=seqheader.GetDestinationPort();
	//uint16_t pid=ipv4Header.GetIdentification();
	SequenceNumber32 ack= seqheader.GetAckNumber();
	packet->AddHeader(ipv4Header);
	NS_LOG_FUNCTION (this << packet << rnti << (uint16_t) bid << packet->GetSize ());
	EpsBearerTag tag (rnti, bid);
	packet->AddPacketTag (tag);
	BEARER_ID_UE_IP_MAP[rnti]=ipv4Header.GetSource();
	if(tcp==true) {

		std::map<uint16_t,uint32_t>::iterator received_seq= Last_Received_data_sequence_number.find(seqheader.GetSourcePort());

		if(!received_seq->first)
		{
			Last_Received_data_sequence_number[seqheader.GetSourcePort()]=seqheader.GetSequenceNumber().GetValue();
		}

		received_seq->second=seqheader.GetSequenceNumber().GetValue();
		if(ipv4Header.GetIdentification()!=0 && ipv4Header.GetIdentification()!=1 ) {
			pkt_info p;
			p.source_port=seqheader.GetSourcePort();
			p.dest_port=seqheader.GetDestinationPort();
			p.seqno=ipv4Header.GetPayloadSize()+seqheader.GetSequenceNumber().GetValue()-20;
			//////std::cout << "payload size " << ipv4Header.GetPayloadSize() << std::endl;
			p.pktsendtime=Simulator::Now ().GetSeconds();//.GetMilliSeconds();
			p.recvbytes=packet->GetSize();
			//	////std::cout << "sendtoltesocket " << p.source_port << std::endl;
			pkt_list.push_back(p);

			if(firstpktsend[p.source_port][p.dest_port]==0) {
				firstpktsend[p.source_port][p.dest_port]=p.pktsendtime;
			}
		}
	}  else {
		count++;
		uint32_t fid,pid;
		if(found) {

			fid=flowprobe.GetFlowId();
			pid=flowprobe.GetPacketId();
			//////std::cout << "fid " << fid << " pid " << pid << std::endl;
			udp_pktinfo p;
			p.fid=fid;
			p.recvbytes=packet->GetSize();
			p.pktsendtime=Simulator::Now ().GetSeconds();
			p.pid=pid;
			udp_pktlist.push_back(p);
			if(udp_firstpktsend[fid]==0) {
				udp_firstpktsend[fid]=Simulator::Now ().GetSeconds();
			}
			//udpstats( fid, pid, p.recvbytes, udp_firstpktsend[fid]);
			//std::cout << fid << " sendbytes " << p.recvbytes << std::endl;
		}

	}
	////std::cout << "outside "<< ipv4Header.GetSource() << std::endl;
	if(flowtable.find(flowprobe.GetFlowId())==flowtable.end() && ipv4Header.GetSource()!="2.0.0.2") {

		flowtable_info f;
		f.current_interface=1;
		EpsBearer eps=ipbearermap[ipv4Header.GetDestination()];
		////std::cout << ipv4Header.GetSource() << " " << ipv4Header.GetDestination() << " " << ack.GetValue() << " "<< seqheader.GetDestinationPort() << std::endl;
		/*if(ack.GetValue()!=1) {
			f.Qos_Enabled=-1;
			//std::cout << "inside "<< flowprobe.GetFlowId() << " "<< ipv4Header.GetSource() << " " << f.Qos_Enabled << std::endl;
		} else {*/
		if(eps.qci==EpsBearer::GBR_CONV_VOICE) {
			f.Qos_Enabled=1;
		} else {
			f.Qos_Enabled=0;
		}
		//}
		f.epsbearer=eps;
		f.appdatarate=ipdatarate[ipv4Header.GetDestination()];
		f.delay=0;
		f.throughput=0;
		f.ip=ipv4Header.GetDestination();
		f.new_flow=1;
		f.protocolno=(uint32_t)ipv4Header.GetProtocol();
		flowtable[flowprobe.GetFlowId()]=f;

	}
	if(ipv4Header.GetSource()!="2.0.0.2") {
		udp_sendbytes[flowprobe.GetFlowId()]+=packet->GetSize();
	}
	//---------------------
	flowtable_info flow=flowtable[flowprobe.GetFlowId()];

	flow.new_flow=0;
	if(EpcEnbApplication::onlylte==1) {
		flow.current_interface=1;
		m_lteSocket->Send (packet);
	}

	else if(EpcEnbApplication::onlylte==2 || EpcEnbApplication::onlylte==6) {

		wifitpt+=packet->GetSize();
		flow.current_interface=2;
		router.senddownlink(packet,2048);

	}
	else if(EpcEnbApplication::onlylte==5) {

		if(ack.GetValue()!=1) {
			std::cout<<"Down link through lte and ack.GetValue() is "<<ack.GetValue()<<std::endl;
			flow.current_interface=1;
			m_lteSocket->Send (packet);
		} else {
			std::cout<<"Down link through wifi and ack.GetValue() is "<<ack.GetValue()<<std::endl;
			flow.current_interface=2;
			router.senddownlink(packet,2048);
		}
	}
	else if(EpcEnbApplication::onlylte==3 ||  EpcEnbApplication::onlylte==7) {

		flow.current_interface=1;

		// IP header based split
		/*if(ipv4Header.GetIdentification()%10<8)//|| ipv4Header.GetIdentification()%10==8)
		{
			m_lteSocket->Send (packet);
		}
		else{
			//std::cout<<"Wi-Fi"<<std::endl;
			router.senddownlink(packet,2048);
		}*/
		/*
		 * Split for every flow equally
		 */
		//std::cout<<"Fraction of LTE and WiFi= " <<EpcEnbApplication::lte_fraction<<"  "<<EpcEnbApplication::wifi_fraction<<std::endl;
		std::map<uint16_t, uint32_t>::iterator it= split_value.find(desport);
		if(!it->first)
		{
			split_value[desport]=1;
		}
		if(it->second <EpcEnbApplication::lte_fraction)
		{
			it->second =it->second +1;
			m_lteSocket->Send (packet);
		}else{
			router.senddownlink(packet,2048);
			it->second =it->second +1;
		}
		it->second =(it->second)%(EpcEnbApplication::lte_fraction+EpcEnbApplication::wifi_fraction);
	}
	else if( EpcEnbApplication::onlylte==4){
		flow.current_interface=1;
		m_lteSocket->Send (packet);
	}

	flowtable[flowprobe.GetFlowId()]=flow;

}


uint32_t topsis(uint32_t gbr, uint32_t currentinterface,uint32_t rank) {
	////std::cout << "reached here" << std::endl;
	double sinrltesum=0,sinrwifisum=0,perwifisum=0;
	double table[flowtable.size()][5];//=new double[flowtable.size()][4];
	std::map<uint32_t, flowtable_info>::iterator it;
	int count=0;
	for(it=flowtable.begin();it!=flowtable.end();it++) {
		if(it->second.Qos_Enabled==gbr && it->second.current_interface==currentinterface) {
			if(it->first!=shiftedflowid) {

				sinrltesum +=sinr_table[it->second.ip].lteval;
				sinrwifisum +=sinr_table[it->second.ip].wifival;
				perwifisum +=per_table[it->second.ip].wifival;
				table[count][0]=it->first;
				table[count][1]=sinr_table[it->second.ip].lteval;
				table[count][2]=sinr_table[it->second.ip].wifival;
				table[count][3]=per_table[it->second.ip].wifival;
				//std::cout << table[count][0] << " " << table[count][1] << " " << table[count][2] << " " << table[count][3] << std::endl;
				count++;
			}
		}
	}
	if(count==0) {
		for(it=flowtable.begin();it!=flowtable.end();it++) {
			if(it->second.Qos_Enabled==-1 && it->second.current_interface==currentinterface) {
				sinrltesum +=sinr_table[it->second.ip].lteval;
				sinrwifisum +=sinr_table[it->second.ip].wifival;
				perwifisum +=per_table[it->second.ip].wifival;
				table[count][0]=it->first;
				table[count][1]=sinr_table[it->second.ip].lteval;
				table[count][2]=sinr_table[it->second.ip].wifival;
				table[count][3]=per_table[it->second.ip].wifival;
				count++;
			}
		}
	}
	////std::cout << "count " << count << std::endl;
	for(int i=0;i<count ;i++ ) {
		if(sinrltesum!=0)
			table[i][1]=table[i][1]/sinrltesum;
		if(sinrwifisum!=0)
			table[i][2]=table[i][2]/sinrwifisum;
		if(perwifisum!=0)
			table[i][3]=table[i][3]/perwifisum;
	}
	double w[3];// = new double[3];
	double tw[3];//= new double[3];
	double tb[3];//= new double[3];
	double wsum=0;
	for(int i=0;i<3;i++) {
		w[i]=0.3;
		wsum+=w[i];
		tw[i]=std::numeric_limits<double>::max();
		tb[i]=0;
	}
	for(int i=0;i<3;i++) {
		w[i]=w[i]/wsum;
	}
	for(int i=0;i<count ;i++ ) {
		table[i][1]=table[i][1]*w[0];
		if(currentinterface==1) {
			//std::cout << "min lte interface" << std::endl;
			if(table[i][1]<tb[0]) {
				tb[0]=table[i][1];
			}
			if(table[i][1]>tw[0]) {
				tw[0]=table[i][1];
			}
		} else {
			if(table[i][1]>tb[0]) {
				tb[0]=table[i][1];
			}
			if(table[i][1]<tw[0]) {
				tw[0]=table[i][1];
			}
		}
		if(currentinterface==2) {
			//std::cout << "min wifi interface" << std::endl;
			table[i][2]=table[i][2]*w[1];
			if(table[i][2]<tb[1]) {
				tb[1]=table[i][2];
			}
			if(table[i][2]>tw[1]) {
				tw[1]=table[i][2];
			}
			table[i][3]=table[i][3]*w[2];
			if(table[i][3]<tb[2]) {
				tb[2]=table[i][3];
			}
			if(table[i][3]>tw[2]) {
				tw[2]=table[i][3];
			}
		}
		else {
			table[i][2]=table[i][2]*w[1];
			if(table[i][2]>tb[1]) {
				tb[1]=table[i][2];
			}
			if(table[i][2]<tw[1]) {
				tw[1]=table[i][2];
			}
			table[i][3]=table[i][3]*w[2];
			if(table[i][3]>tb[2]) {
				tb[2]=table[i][3];
			}
			if(table[i][3]<tw[2]) {
				tw[2]=table[i][3];
			}
		}
	}
	for(int i=0;i<count;i++) {
		double dw=sqrt(pow(table[i][1]-tw[0],2)+pow(table[i][2]-tw[1],2)+pow(table[i][3]-tw[2],2));
		double di=sqrt(pow(table[i][1]-tb[0],2)+pow(table[i][2]-tb[1],2)+pow(table[i][3]-tb[2],2));
		if(dw+di!=0)
			table[i][4]=dw/(dw+di);
		else
			table[i][4]=0;
		////std::cout << table[i][4] << std::endl;
	}
	for(int i=0;i<count;i++) {
		int max=table[i][4];
		int index=i;
		for(int j=i+1;j<count;j++) {
			if(table[j][4]>max) {
				max=table[j][4];
				index=j;
			}
		}
		double* temp=table[i];
		table[i][0]=table[index][0];
		table[i][1]=table[index][1];
		table[i][2]=table[index][2];
		table[i][3]=table[index][3];
		table[i][4]=table[index][4];
		table[index][0]=temp[0];
		table[index][1]=temp[1];
		table[index][2]=temp[2];
		table[index][3]=temp[3];
		table[index][4]=temp[4];
	}
	if(count==0) {
		////std::cout << "count 0" << std::endl;
		return -1;
	}
	else if(rank<=(uint32_t)count) {
		////std::cout << "count " << count << " rank " << rank << std::endl;
		return table[rank-1][0];
	}
	else {
		////std::cout << "rank > count" << std::endl;
		return -1;
	}
}
int GBR_lte() {
	int lte=0;
	std::map<uint32_t, flowtable_info>::iterator it;
	for(it=flowtable.begin();it!=flowtable.end();it++) {
		if(it->second.current_interface==1 && it->second.Qos_Enabled==1) {
			if(lte==0)
				lte=1;
			else if(lte==2) {
				lte=3;
				break;
			}
		}
		if(it->second.current_interface==2 && it->second.Qos_Enabled==1) {
			if(lte==0)
				lte=2;
			else if(lte==1) {
				lte=3;
				break;
			}
		}
	}
	return lte;
}
int gbr_ensured(uint32_t current_interface) {
	int gbr_ensured=1;
	std::map<uint32_t, flowtable_info>::iterator it;
	for(it=flowtable.begin();it!=flowtable.end();it++) {
		if(it->first!=shiftedflowid) {
			if(it->second.current_interface==current_interface && it->second.Qos_Enabled==1 && (((double)it->second.epsbearer.gbrQosInfo.gbrDl/1000000))-(it->second.throughput*8/duration/1000/1000)>=0.2) {//((double)it->second.epsbearer.gbrQosInfo.gbrDl/10000000)) {
				//	std::cout << it->first << " " <<(double)it->second.epsbearer.gbrQosInfo.gbrDl/1000/1000 << " " << it->second.throughput*8/duration/1000/1000 << " " << (((double)it->second.epsbearer.gbrQosInfo.gbrDl/1000000))-(it->second.throughput*8/duration/1000/1000) << " " << (double)it->second.epsbearer.gbrQosInfo.gbrDl/10000000 << std::endl;
				////std::cout << it->first << " " <<it->second.epsbearer.gbrQosInfo.gbrDl << " " << it->second.throughput <<  std::endl;
				gbr_ensured=0;
				break;
			}
		}
	}
	return gbr_ensured;
}
double sum(uint32_t current_interface, uint32_t gbr) {
	double sum=0;
	int count=0;
	std::map<uint32_t, flowtable_info>::iterator it;
	for(it=flowtable.begin();it!=flowtable.end();it++) {
		if(gbr==3) {
			if(it->second.current_interface==current_interface ) {
				sum+=it->second.throughput;
			}
		} else {
			if(it->second.current_interface==current_interface && it->second.Qos_Enabled==gbr) {
				sum+=(double)it->second.epsbearer.gbrQosInfo.gbrDl/1000000-it->second.throughput*8/duration/1000/1000;
				count++;
			}
		}
	}
	//std::cout << sum*8/duration/1000/1000 << " "<< count <<std::endl;
	if(gbr==3)
		return sum*8/duration/1000/1000;
	else if(count!=0)
		return sum;
	else
		return 0;
}
double sum1(uint32_t current_interface) {
	double tsum=0;
	//int count=0;
	std::map<uint32_t, flowtable_info>::iterator it;
	for(it=flowtable.begin();it!=flowtable.end();it++) {

		if(it->second.current_interface==current_interface ) {
			tsum+=it->second.throughput;
		}

	}
	return tsum*8/duration/1000/1000;
}
bool checkNGBR(uint32_t currentinterface) {
	std::map<uint32_t, flowtable_info>::iterator it;
	/*for(it=flowtable.begin();it!=flowtable.end();it++) {
		std::cout << it->first << " " << it->second.Qos_Enabled << " " << it->second.current_interface << std::endl;
	}*/
	for(it=flowtable.begin();it!=flowtable.end();it++) {
		if(it->second.current_interface==currentinterface && it->second.Qos_Enabled==0) {
			std::cout << it->first << " " << it->second.Qos_Enabled << " " << it->second.current_interface << std::endl;
			return true;
		}
	}
	return false;
}
void maximizethrpt() {
	double lteload=lte_usedrbs/50;
	double wifiload=wifi_load*66;
	std::cout << "maximize throughput is called "<<mxpt++<< std::endl;
	////std::cout << "lteload " << lteload << " wifiload " << wifiload << std::endl;
	if(lteload-wifiload>4) {
		std::cout << "load of lte is greater than wifi" << std::endl;
		uint32_t ngbrflowid=topsis(0,1,1);
		if(flowtable[ngbrflowid].protocolno==6) {
			// move NGBR flow to wifi
			std::cout << "move NGBR "<< ngbrflowid << " " << sinr_table[flowtable[ngbrflowid].ip].lteval << " " << sinr_table[flowtable[ngbrflowid].ip].wifival << " flow to wifi" << std::endl;
			flowtable_info f=flowtable[ngbrflowid];
			f.current_interface=2;
			flowtable[ngbrflowid]=f;
		} else {
			int count=2;
			while(1) {
				if(ngbrflowid==(uint32_t)-1) {
					break;
				} else {
					double ND=flowtable[ngbrflowid].appdatarate;
					//check availability in wifi
					double load=wifi_load*66;
					if(load<66 && ND<=(66-load)*phydatarate/100) {
						// move NGBR flow to wifi
						//					std::cout << "move NGBR flow " << ngbrflowid << " " << sinr_table[flowtable[ngbrflowid].ip].lteval << " " << sinr_table[flowtable[ngbrflowid].ip].wifival<< " to wifi" << std::endl;
						flowtable_info f=flowtable[ngbrflowid];
						f.current_interface=2;
						flowtable[ngbrflowid]=f;
						break;
					} else {
						ngbrflowid=topsis(0,1,count);
						count++;
					}
				}
			}
		}
	} else if(wifiload-lteload>4){
		//std::cout << "load of wifi is greater than lte" << std::endl;
		// move NGBR flow to lte
		uint32_t ngbrflowid=topsis(0,2,1);
		if(ngbrflowid!=(uint32_t)-1) {
			if(ngbrflowid!=(uint32_t)-1) {
				//			std::cout << "move NGBR "<< ngbrflowid << " " << sinr_table[flowtable[ngbrflowid].ip].lteval << " " << sinr_table[flowtable[ngbrflowid].ip].wifival<< " flow to lte" << std::endl;
				flowtable_info f=flowtable[ngbrflowid];
				f.current_interface=1;
				flowtable[ngbrflowid]=f;
			}
		}
	}
}
void movefairly() {
	double tsum=0;
	double isum=0;
	std::cout << "move fairly is called "<<fair++<< std::endl;
	std::map<uint32_t, flowtable_info>::iterator it;
	int count=0;
	for(it=flowtable.begin();it!=flowtable.end();it++) {
		if(it->second.Qos_Enabled==1) {
			double val=it->second.throughput*8/duration/1000/1000/((double)it->second.epsbearer.gbrQosInfo.gbrDl/1000000);
			//std::cout << it->second.throughput*8/duration/1000/1000 << " " << (double)it->second.epsbearer.gbrQosInfo.gbrDl/1000000 << " " << val << std::endl;
			tsum+=val;
			isum+=(val*val);
			count++;
		}
	}
	double lteindex=(double)tsum*tsum*100/((double)count*isum) ;
	tsum=0;
	isum=0;
	/*double count1=0;
	double count2=0;*/
	for(it=flowtable.begin();it!=flowtable.end();it++) {
		if(it->second.Qos_Enabled==1 && it->second.current_interface==(uint32_t)1) {
			tsum+=((double)it->second.epsbearer.gbrQosInfo.gbrDl/1000000)-it->second.throughput*8/duration/1000/1000;
			//	count1++;

		} if(it->second.Qos_Enabled==1 && it->second.current_interface==(uint32_t)2) {
			isum+=((double)it->second.epsbearer.gbrQosInfo.gbrDl/1000000)-it->second.throughput*8/duration/1000/1000;
			//	count2++;
		}
	}
	//double wifiindex=(double)tsum*tsum*100/((double)count*isum) ;
	std::cout << lteindex << " " << (tsum)-(isum) << std::endl;
	if(count!=0) {
		if((tsum)-(isum)<1 && lteindex<95) {
			// check throughput of gbr in lte > wifi
			if(sum(1,1)>sum(2,1)) {
				// move one GBR flow to lte

				uint32_t gbrflowid=topsis(1,2,1);
				std::cout << "move one GBR flow to lte " << gbrflowid << std::endl;
				flowtable_info f=flowtable[gbrflowid];
				f.current_interface=1;
				flowtable[gbrflowid]=f;
			} /*else {
				uint32_t gbrflowid=topsis(1,1,1);
				flowtable_info f=flowtable[gbrflowid];
				f.current_interface=2;
				flowtable[gbrflowid]=f;
			}*/
		}
	}
}
int nflows(uint32_t current_interface) {
	int count=0;
	std::map<uint32_t, flowtable_info>::iterator it;
	for(it=flowtable.begin();it!=flowtable.end();it++) {
		if(it->second.current_interface==current_interface) {
			count++;
		}
	}
	return count;
}
void algo() {
	int gbr_lte=GBR_lte();
	// no GBR flows
	if(gbr_lte==0) {
		std::cout << "no gbr " << std::endl;
		//maximize the throughput
		maximizethrpt();
	}
	// GBR flows only in lte
	else if(gbr_lte==1){
		std::cout << "gbr are only in lte" << std::endl;
		int gbr_ensuredlte=gbr_ensured(1);
		int gbr_ensuredwifi=gbr_ensured(2);
		// gbr is ensured in lte;
		if(gbr_ensuredlte) {
			//maximize the throughput
			//std::cout << "maximize throughput" << std::endl;
			std::cout << "gbr is  ensured in lte" << std::endl;
			maximizethrpt();
		}
		// gbr is not ensured in lte
		else {
			std::cout << "gbr is not ensured in lte" << std::endl;
			uint32_t gbrflowid=topsis(1,1,1);
			double ND=flowtable[gbrflowid].appdatarate;
			double wifiload=wifi_load*66;
			std::cout  << " wififree " << (66-wifiload)*phydatarate/100 << " nd " << ND <<  std::endl;
			//wifi can accommodate
			if(wifiload<66 && ND<=(66-wifiload)*phydatarate/100 && gbr_ensuredwifi) {
				// move GBR flow to wifi
				std::cout << "move GBR flow " << gbrflowid << " "<< sinr_table[flowtable[gbrflowid].ip].lteval << " " << sinr_table[flowtable[gbrflowid].ip].wifival << "  to wifi" << std::endl;
				flowtable_info f=flowtable[gbrflowid];
				f.current_interface=2;
				flowtable[gbrflowid]=f;
			}
			// wifi cannot accommodate
			else {
				// check NGBR in wifi
				if(checkNGBR(2)) {
					// move NBGR flow to lte

					uint32_t ngbrflowid=topsis(0,2,1);
					flowtable_info f=flowtable[ngbrflowid];
					f.current_interface=1;
					flowtable[gbrflowid]=f;
					// move GBR flow to wifi
					f=flowtable[gbrflowid];
					f.current_interface=2;
					flowtable[gbrflowid]=f;
					std::cout << "move NGBR flow " << ngbrflowid << " "<< sinr_table[flowtable[ngbrflowid].ip].lteval << " " << sinr_table[flowtable[ngbrflowid].ip].wifival << "to lte" << std::endl;
					std::cout << "move GBR flow " << gbrflowid << " "<< sinr_table[flowtable[gbrflowid].ip].lteval << " " << sinr_table[flowtable[gbrflowid].ip].wifival << "to wifi" << std::endl;
				}
			}
		}

	}
	// GBR flows are in both lte and wifi
	else {
		std::cout << "GBR flows are in both lte and wifi" << std::endl;
		int gbrensured_lte=gbr_ensured(1);
		bool flag=true;
		// gbr is not ensured in lte
		shiftedflowid=-1;
		if(!gbrensured_lte) {
			flag=false;
			std::cout << "gbr is not ensured in lte" << std::endl;
			uint32_t gbrflowid=topsis(1,1,1);
			double ND=flowtable[gbrflowid].appdatarate;
			//wifi can accommodate
			double wifiload=wifi_load*100;
			std::cout  << "wifiload " <<  wifiload << " wififree " << (66-wifi_load*66)*phydatarate/100 << " nd " << ND <<  std::endl;
			//wifi can accommodate
			if(wifi_load*66<66 && ND<=(66-wifi_load*66)*phydatarate/100 && gbr_ensured(2)) {
				std::cout << "wifi can accommodate\n move GBR flow  " << gbrflowid << " "<< sinr_table[flowtable[gbrflowid].ip].lteval << " " << sinr_table[flowtable[gbrflowid].ip].wifival  << " to wifi" << std::endl;
				// move GBR flow to wifi
				shiftedflowid=gbrflowid;
				flowtable_info f=flowtable[gbrflowid];
				f.current_interface=2;
				flowtable[gbrflowid]=f;
			}
			// wifi cannot accommodate
			else {
				// check NGBR in wifi
				if(checkNGBR(2)) {
					std::cout << "wifi cannot accommodate\n move GBR flow to lte" << std::endl;
					// move NBGR flow to lte

					uint32_t ngbrflowid=topsis(0,2,1);
					shiftedflowid=ngbrflowid;
					flowtable_info f=flowtable[ngbrflowid];
					f.current_interface=1;
					flowtable[gbrflowid]=f;
					// move GBR flow to wifi
					f=flowtable[gbrflowid];
					f.current_interface=2;
					flowtable[gbrflowid]=f;
					std::cout << "move NGBR flow " << ngbrflowid << " "<< sinr_table[flowtable[ngbrflowid].ip].lteval << " " << sinr_table[flowtable[ngbrflowid].ip].wifival << "to lte" << std::endl;
					std::cout << "move GBR flow " << gbrflowid << " "<< sinr_table[flowtable[gbrflowid].ip].lteval << " " << sinr_table[flowtable[gbrflowid].ip].wifival << "to wifi" << std::endl;

				}
			}
		}
		int gbrensured_wifi=gbr_ensured(2);
		// gbr ensured in wifi
		if(gbrensured_wifi ) {
			//maximize throughput
			if(flag) {
				//			std::cout << "gbr ensured in wifi\n maximize throughput\n";
				maximizethrpt();
			}
		}
		// gbr not ensured in wifi
		else {
			//		std::cout << "gbr not ensured in wifi\n";
			double AVlte=sum(1,2);
			uint32_t gbrflowid=topsis(1,2,1);
			double NFD=flowtable[gbrflowid].appdatarate;
			//lte can accommodate GBR flow
			if(NFD<=AVlte+0.4) {
				//			std::cout << "lte can accommodate GBR flow\n move GBR " << gbrflowid << "  to lte \n";
				//move GBR to lte
				flowtable_info f=flowtable[gbrflowid];
				f.current_interface=1;
				flowtable[gbrflowid]=f;
			}
			// lte cannot accommodate GBR flow
			else {
				//std::cout << "lte cannot accommodate GBR flow" << std::endl;
				// check NGBR in wifi
				if(checkNGBR(2)) {
					//				std::cout << "lte cannot accommodate GBR flow\n move NGBR to lte \n";
					//move NGBR to lte
					uint32_t ngbrflowid=topsis(0,2,1);
					flowtable_info f=flowtable[ngbrflowid];
					f.current_interface=1;
					flowtable[gbrflowid]=f;
				}
				// no NGBR in wifi
				else {
					//				std::cout << sum(1,1) << " " << sum(2,1) << std::endl;
					// check throughput of gbr in lte > wifi
					if(sum(1,1)>sum(2,1)) {
						//					std::cout << "lte cannot accommodate GBR flow\n no NGBR in wifi\n move flows GBR flows to lte fairely\n ";
						// move flows GBR flows to lte fairely
						//std::cout << "lte cannot accommodate GBR flow\n no NGBR in wifi \n move flows GBR flows to lte fairely\n";
						movefairly();
					}
				}
			}
		}
	}
}

void algo_naive() {
	double lteload=lte_usedrbs/50;
	double wifiload=wifi_load*66;
	//	std::cout << "lteload " << lteload << " wifiload " << wifiload << std::endl;
	if(lteload-wifiload>20) {
		//std::cout << "load of lte is greater than wifi" << std::endl;
		std::map<uint32_t, flowtable_info>::iterator it;
		for(it=flowtable.begin();it!=flowtable.end();it++) {
			if(it->second.new_flow) {
				flowtable_info f=flowtable[it->first];
				f.current_interface=2;
				f.new_flow=0;
				flowtable[it->first]=f;
			}
		}
	} else {
		//std::cout << "load of wifi is greater than lte" << std::endl;
		// move NGBR flow to lte
		std::map<uint32_t, flowtable_info>::iterator it;
		for(it=flowtable.begin();it!=flowtable.end();it++) {
			flowtable_info f=flowtable[it->first];
			f.new_flow=0;
			flowtable[it->first]=f;
		}
	}
}
int no_gbrnotensured(uint32_t current_interface) {
	double count=0;
	std::map<uint32_t, flowtable_info>::iterator it;
	for(it=flowtable.begin();it!=flowtable.end();it++) {
		if(it->first!=shiftedflowid) {
			if(it->second.current_interface==current_interface && it->second.Qos_Enabled==1 && (((double)it->second.epsbearer.gbrQosInfo.gbrDl/1000000))-(it->second.throughput*8/duration/1000/1000)>=0.2) {//((double)it->second.epsbearer.gbrQosInfo.gbrDl/10000000)) {
				//	std::cout << it->first << " " <<(double)it->second.epsbearer.gbrQosInfo.gbrDl/1000/1000 << " " << it->second.throughput*8/duration/1000/1000 << " " << (((double)it->second.epsbearer.gbrQosInfo.gbrDl/1000000))-(it->second.throughput*8/duration/1000/1000) << " " << (double)it->second.epsbearer.gbrQosInfo.gbrDl/10000000 << std::endl;
				////std::cout << it->first << " " <<it->second.epsbearer.gbrQosInfo.gbrDl << " " << it->second.throughput <<  std::endl;
				count++;
			}
		}
	}
	return count;
}
void EpcEnbApplication::resetfunc(void) {

	//	std::cout << "---------------------------reset-------------------------------\n\n";
	double tthrpt=0;
	double recvb=0;
	{
		std::map<uint16_t,std::map<uint16_t,uint64_t> >::iterator it_recvbytes;
		std::map<uint16_t, std::map<uint16_t,double> >::iterator it_lastpktrecv=lastpktrecv.begin();
		std::map<uint16_t, std::map<uint16_t,double> >::iterator it_firstpktsend=firstpktsend.begin();
		std::map<uint16_t, std::map<uint16_t,double> >::iterator it_total_delay=total_delay.begin();
		for(it_recvbytes=RecvBytes.begin();it_recvbytes!=RecvBytes.end();++it_recvbytes) {
			std::map<uint16_t,uint64_t>::iterator it1_recvbytes;
			std::map<uint16_t,double>::iterator it1_lastpktrecv=it_lastpktrecv->second.begin();
			std::map<uint16_t,double>::iterator it1_firstpktsend=it_firstpktsend->second.begin();
			std::map<uint16_t,double>::iterator it1_total_delay=it_total_delay->second.begin();
			for(it1_recvbytes=it_recvbytes->second.begin();it1_recvbytes!=it_recvbytes->second.end();++it1_recvbytes) {
				//double thrpt=( ((double)it1_recvbytes->second*8) / (it1_lastpktrecv->second - it1_firstpktsend->second)/1024/1024 );
				double thrpt=(it1_recvbytes->second * 8)/(it1_lastpktrecv->second-it1_firstpktsend->second)/1024/1024;
				////std::cout << it_recvbytes->first << " " << it1_recvbytes->first << " " << it1_recvbytes->second << " " << thrpt<< std::endl;
				tthrpt+=thrpt;
			}
		}

		lastpktrecv.clear();
		firstpktsend.clear();
		total_delay.clear();
		RecvBytes.clear();
	} {

		std::map<uint32_t,double >::iterator it_recvbytes;
		std::map<uint32_t,double >::iterator it_sendbytes=udp_sendbytes.begin();
		std::map<uint32_t, double>::iterator it_lastpktrecv=udp_lastpktrecv.begin();
		std::map<uint32_t, double>::iterator it_firstpktsend=udp_firstpktsend.begin();
		std::map<uint32_t, double>::iterator it_total_delay=udp_totaldelay.begin();
		//////std::cout << udp_recvbytes.size() << std::endl;

		for(it_recvbytes=udp_recvbytes.begin();it_recvbytes!=udp_recvbytes.end();it_recvbytes++) {
			double thrpt=((double)it_recvbytes->second*8) / duration/1000/1000;
			recvb+=it_recvbytes->second;
			flowtable_info f=flowtable[it_recvbytes->first];
			f.throughput=it_recvbytes->second;
			tthrpt+=thrpt;
			f.delay=it_total_delay->second;
			flowtable[it_recvbytes->first]=f;
			//std::cout << it_recvbytes->first << " sendbytes " << it_sendbytes->second << " recvbytes " << it_recvbytes->second <<" throughput " << thrpt<< " current interface " << f.current_interface << " "<< (double)f.Qos_Enabled << " " << f.new_flow<< std::endl;
			////std::cout << it_total_delay->first << " " << it_total_delay->second << std::endl;
			it_total_delay++;
			it_sendbytes++;
		}
		/*std::map<uint32_t, flowtable_info>::iterator it;
		for(it=flowtable.begin();it!=flowtable.end();) {
			if(udp_recvbytes.find(it->first)==udp_recvbytes.end()) {
				flowtable.erase(it++);

			} else
				++it;
		}*/
		udp_lastpktrecv.clear();
		udp_firstpktsend.clear();
		udp_totaldelay.clear();
		udp_recvbytes.clear();

	}
	////std::cout << "ip sinr per \n";
	std::map<Ipv4Address, sinrperinfo>::iterator it_sinr;
	std::map<Ipv4Address, sinrperinfo>::iterator it_per=per_table.begin();
	for(it_sinr=sinr_table.begin();(it_sinr!=sinr_table.end()) && (it_per!=per_table.end());it_sinr++,it_per++) {
		////std::cout << it_sinr->first << " " << sinr_table[it_sinr->first].lteval << " "  << sinr_table[it_sinr->first].wifival << std::endl;
		////std::cout << it_per->first << " " << per_table[it_sinr->first].lteval << " "  << per_table[it_sinr->first].wifival << std::endl;

	}
	std::cout << "lte load " << lte_usedrbs << std::endl;
	std::cout << "wifi load " << wifi_load << std::endl;
	std::cout << "flowtable size " << flowtable.size() << std::endl;
	////std::cout << "sinr_table " << sinr_table.size() << std::endl;
	////std::cout << "per_table " << per_table.size() << std::endl;
	std::cout << "total dl  throughput " << tthrpt << std::endl;

	//std::cout << "total dl throughput duration " << recvb*8/duration/1000/1000 << std::endl;
	std::cout << "total dl lte throughput " << sum1(1) << std::endl;
	std::cout << "total dl wifi throughput " << sum1(2) << std::endl;
	std::cout << "total ul lte throughput " << (double)uplinkthrpt_lte*8/duration/1000/1000 << std::endl;
	std::cout<<" total recvbytes " << uplinkthrpt <<std::endl;
	std::cout << "total ul wifi throughput " << (double)uplinkthrpt_wifi*8/duration/1000/1000 << std::endl;
	std::cout << "total throughput "<<(tthrpt)+(double)(uplinkthrpt*8/duration/1000/1000)<< std::endl;
	std::cout << " new wifi tpt "<< (double)(wifitpt*8/duration/1000/1000)<<std::endl;
	//std::cout << "uplink pkt send " << EpcEnbApplication::uplinksendpkt << std::endl;
	//std::cout << (double)(uplinkthrpt*8/duration/1000/1000)+(recvb*8/duration/1000/1000) << " " << nflows(1) << " " << nflows(2) << " " << no_gbrnotensured(1) << " " << no_gbrnotensured(2)<< std::endl;
	flows  << nflows(1) << " " << nflows(2) << " " << no_gbrnotensured(1) << " " << no_gbrnotensured(2)<< std::endl;


	total<<(tthrpt)<<" "<<((double)uplinkthrpt_lte*8/duration/1000/1000)<<" "<<((double)uplinkthrpt_wifi*8/duration/1000/1000)<<" ";
	total<<(tthrpt)+((double)uplinkthrpt_lte*8/duration/1000/1000)+((double)uplinkthrpt_wifi*8/duration/1000/1000)<<" ";
	total<<(tthrpt)+(double)(uplinkthrpt*8/duration/1000/1000)<<"\n";
	/*double lteload=lte_usedrbs/125;
	double wifiload=wifi_load*66;
	////std::cout << "difference of load " << lteload-wifiload << std::endl;*/
	//	if(EpcEnbApplication::onlylte==4||EpcEnbApplication::onlylte==3){
	//          algo();
	//       }
	lte_usedrbs=0;
	uplinkthrpt_lte=0;
	uplinkthrpt_wifi=0;
	uplinkthrpt=0;
	wifitpt=0;
	uplinksendpkt=0;

	udp_sendbytes.clear();
	//flowtable.clear();
	//ulfirstpktsend=-1;
	Simulator::Schedule(Seconds(duration),&resetfunc);
}

void
EpcEnbApplication::SendToS1uSocket (Ptr<Packet> packet, uint32_t teid)
{

	//////std::cout<<"SendToS1uSocket"<<std::endl;
	NS_LOG_FUNCTION (this << packet << teid <<  packet->GetSize ());
	//packet->Print(std::cout);
	//std::cout << std::endl;
	uplinkthrpt+=packet->GetSize();
	ullastpktrecv=Simulator::Now().GetSeconds();
	Ptr<Packet> packet_copy=packet->Copy();
	/*
	 *Packet Holding (Boost ACK)
	 */
	Ipv4Header ipv4h;
	TcpHeader tcph;
	packet->RemoveHeader(ipv4h);
	packet->RemoveHeader(tcph);

	if(ipv4h.GetProtocol()==6){
		/*
		 * Skip ACK implementation
		 */
		if (EpcEnbApplication::skip_ack)
		{
			std::map<uint16_t,uint32_t>::iterator it=Past_Ack_number_sent.find(tcph.GetDestinationPort());
			std::map<uint16_t,uint64_t>::iterator time_it =timer_value.find(tcph.GetDestinationPort());
			//Adds the new flows which doesn't exist
			if (!it->first)
			{
				Past_Ack_number_sent[tcph.GetDestinationPort()]=tcph.GetAckNumber().GetValue();
				timer_value[tcph.GetDestinationPort()]=0;
			}
			// Checks for the ack number
			if(tcph.GetAckNumber().GetValue() == it->second ){
				if(time_it->second==0)
				{
					time_it->second=Simulator::Now().GetMicroSeconds();
				}
				if((Simulator::Now().GetMicroSeconds()-time_it->second) < EpcEnbApplication::holding_time){
					holded_packets++;
					std::cout<< Simulator::Now().GetMicroSeconds()<<" holded_packets: " <<holded_packets<<std::endl;
					return;
				}
			}
			else{
				Past_Ack_number_sent[tcph.GetDestinationPort()]=tcph.GetAckNumber().GetValue();
				time_it->second=0;
			}
		}

		/*
		 * DIDA: Delayed Pseudo In sequence
		 * DIDA holds the packet and send after fixed amount of time which is sole dependent based on round trip time
		 *
		 */
		/*		if (EpcEnbApplication::DIDA==true)
		{
			std::map<uint16_t,uint32_t>::iterator it=Past_Ack_number_sent.find(tcph.GetDestinationPort());
			std::map<uint16_t,uint64_t>::iterator time_it =timer_value.find(tcph.GetDestinationPort());
			std::map<uint16_t,Ptr<Packet>>::iterator packet_it=last_stored_packet.find(tcph.GetDestinationPort());

			//Adds the new flows which doesn't exist
			if (!it->first)
			{
				Past_Ack_number_sent[tcph.GetDestinationPort()]=tcph.GetAckNumber().GetValue();
				timer_value[tcph.GetDestinationPort()]=0;
			}
			// Checks for the ack number
			if(tcph.GetAckNumber().GetValue() == it->second){
				estimated_window=(estimated_window/2);
				testing_counter++;

				if(time_it->second==0)
				{
					time_it->second=Simulator::Now().GetMicroSeconds();
					local_timer=Simulator::Now().GetMicroSeconds();
					packet_it->second=packet_copy;
				}
				if((Simulator::Now().GetMicroSeconds()-time_it->second) < EpcEnbApplication::holding_time){
					holded_packets++;
					std::cout<< Simulator::Now().GetMicroSeconds()<<" holded_packets: " <<holded_packets<<std::endl;
					return;
				}
			}
			else{
				if(local_timer>0)
				{

				}
				testing_counter=0;
				estimated_window=tcph.GetAckNumber().GetValue()-it->second ;
				//std::cout<< "Difference" <<(Simulator::Now().GetMicroSeconds()-it->second.t)<<std::endl;
				//it->second.t=Simulator::Now().GetMicroSeconds();
				Past_Ack_number_sent[tcph.GetDestinationPort()]=tcph.GetAckNumber().GetValue();
				time_it->second=0;
			}

		}*/

		/*
		 * Boost ACK
		 */
		if(EpcEnbApplication::boost_ack){
			std::map<uint16_t,uint32_t>::iterator it=Past_Ack_number_sent.find(tcph.GetDestinationPort());
			std::map<uint16_t,uint64_t>::iterator time_it =timer_value.find(tcph.GetDestinationPort());
			//Adds the new flows which doesn't exist
			if (!it->first)
			{
				Past_Ack_number_sent[tcph.GetDestinationPort()]=tcph.GetAckNumber().GetValue();
				timer_value[tcph.GetDestinationPort()]=0;
			}
			// Checks for the ack number
			if(tcph.GetAckNumber().GetValue() == it->second){
				estimated_window=(estimated_window/2);
				testing_counter++;
				SequenceNumber32 seq;
				seq=tcph.GetAckNumber();
				seq+=(536*testing_counter);
				std::map<uint16_t,uint32_t>::iterator its= Last_Received_data_sequence_number.find(tcph.GetDestinationPort());
				std::cout<< "Received data =" <<its->second <<" Sent ACK = "<< seq.GetValue()<< " "<<tcph.GetAckNumber().GetValue() <<std::endl;
				//if (seq.GetValue() < its->second)
				//{
				tcph.SetAckNumber(seq);
				//}

				if(time_it->second==0)
				{
					time_it->second=Simulator::Now().GetMicroSeconds();
					local_timer=Simulator::Now().GetMicroSeconds();
				}

				std::cout <<"difference = "<<Simulator::Now().GetMicroSeconds()-time_it->second<<"  Holding time ="<<EpcEnbApplication::holding_time<<std::endl;
				/*if((Simulator::Now().GetMicroSeconds()-time_it->second) < EpcEnbApplication::holding_time){
					holded_packets++;
					std::cout<< Simulator::Now().GetMicroSeconds()<<" holded_packets: " <<holded_packets<<std::endl;
					return;
				}*/
			}
			else{
				if(local_timer>0)
				{

				}
				testing_counter=0;
				estimated_window=tcph.GetAckNumber().GetValue()-it->second ;
				//std::cout<< "Difference" <<(Simulator::Now().GetMicroSeconds()-it->second.t)<<std::endl;
				//it->second.t=Simulator::Now().GetMicroSeconds();
				Past_Ack_number_sent[tcph.GetDestinationPort()]=tcph.GetAckNumber().GetValue();
				time_it->second=0;
			}

		}

	}


	//std::cout<<"  Ack value1= "<<tcph.GetAckNumber().GetValue()<<std::endl;
	packet->AddHeader(tcph);
	packet->AddHeader(ipv4h);

	/*
	 * Packet holding ends
	 */
	GtpuHeader gtpu;
	gtpu.SetTeid (teid);
	// From 3GPP TS 29.281 v10.0.0 Section 5.1
	// Length of the payload + the non obligatory GTP-U header
	gtpu.SetLength (packet->GetSize () + gtpu.GetSerializedSize () - 8);
	packet->AddHeader (gtpu);
	uint32_t flags = 0;
	m_s1uSocket->SendTo (packet, flags, InetSocketAddress(m_sgwS1uAddress, m_gtpuUdpPort));

}
/*
void
EpcEnbApplication::usrDefinedReleaseUe(uint16_t rnti)
{
	m_s1SapUser->releaseusrDefinedReleaseUe(rnti);
}*/
}; // namespace ns3
