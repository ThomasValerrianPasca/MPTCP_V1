/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-module.h"
#include <string>
#include <fstream>
#include "ns3/packet-sink.h"
// Default Network Topology
// 
// ================
// |              |
// n1   n2   n3   n4
// |    |    |    |
// ================
// LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
	bool verbose = true;

	/*LogComponentEnable("MpTcpSocketBase", LOG_LEVEL_ALL);
	LogComponentEnable("MpTcpBulkSendApplication", LOG_LEVEL_ALL);
	LogComponentEnable("MpTcpPacketSink", LOG_LEVEL_ALL);
	LogComponentEnable("MpTcpSubflow", LOG_LEVEL_ALL);
	*/


	Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
	Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
	Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
	Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(100));
	Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
	Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(2)); // Sink
	Config::SetDefault("ns3::MpTcpSocketBase::CongestionControl", StringValue("Linked_Increases"));
	Config::SetDefault("ns3::MpTcpSocketBase::PathManagement", StringValue("FullMesh"));//NdiffPorts


	CommandLine cmd;
	cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

	cmd.Parse (argc,argv);

	if (verbose)
	{
		LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
		LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
	}

	int nCsma = 4,i=0,j=0;


	NodeContainer csmaNodes;
	csmaNodes.Create(nCsma);

	NodeContainer additional_interfaces;
	additional_interfaces.Add(csmaNodes.Get(0));
	additional_interfaces.Add(csmaNodes.Get(3));

	NodeContainer Combined;
	Combined.Add(csmaNodes);
	Combined.Add(additional_interfaces);


	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
	csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

	NetDeviceContainer csmaDevices, additional_link;
	csmaDevices = csma.Install (csmaNodes);
	additional_link= csma.Install(additional_interfaces);

	InternetStackHelper stack;
	stack.Install (csmaNodes);

	Ipv4AddressHelper address;
	address.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer csmaInterfaces, additional_interface_ip;
	csmaInterfaces = address.Assign (csmaDevices);
	additional_interface_ip= address.Assign (additional_link);
	for (i=0;i<nCsma;i++)
	{
		for (j=0;j<1;j++)
		{
	std::cout<<"IP Address = "<<csmaInterfaces.GetAddress(i,j)<<" Additional interface IP= "<<additional_interface_ip.GetAddress(0,0)<<" Additional interface IP= "<<additional_interface_ip.GetAddress(1,0)<<std::endl;
		}
	}
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	uint16_t port = 9;
	MpTcpPacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
	ApplicationContainer sinkApps = sink.Install(csmaNodes.Get(3));
	sinkApps.Start(Seconds(1.0));
	sinkApps.Stop(Seconds(10.0));


	MpTcpBulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address(additional_interface_ip.GetAddress(1,0)), port));
	source.SetAttribute("MaxBytes", UintegerValue(0));
	ApplicationContainer sourceApps = source.Install(csmaNodes.Get(0));
	sourceApps.Start(Seconds(1.0));
	sourceApps.Stop(Seconds(10.0));
	csma.EnablePcap ("second", Combined, true);


	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor;

	//monitor= flowmon.Install (csmaNodes);
	//flowmon.Install(csmaNodes);

Simulator::Stop(Seconds(11));
	Simulator::Run ();
/*
	monitor->CheckForLostPackets ();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
	double Thrpt=0;double received_bytes=0, received_pkts=0, transmitted_packets=0;ns3::Time total_time;
	double Delay=0,PLoss=0;
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
	{

		if (1)
		{

			Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
			std::string interface=(t.destinationPort %2==0 )? "LTE":"Wi-Fi";
			std::cout << "Flow " << i->first << " (" << t.sourceAddress << "(" << t.sourcePort <<")" << " -> " << t.destinationAddress <<"("<<t.destinationPort<<")"<< interface<<")\n";

			std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
			std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
			received_bytes+=i->second.rxBytes;
			std::cout << "  Transmitted Packets: " << i->second.txPackets << std::endl;
			std::cout << "  Received Packets: " << i->second.rxPackets << std::endl;
			std::cout << "  First Tx time:   " << i->second.timeFirstTxPacket << "\n";
			std::cout << "  Last Rx time:   " << i->second.timeLastRxPacket << "\n";
			std::cout << "  Delay = " << (i->second.delaySum.GetSeconds()/i->second.rxPackets*1000)<< "msec \n";
			total_time+=i->second.timeLastRxPacket-i->second.timeFirstTxPacket;
			std::cout << "  Throughput: " << ( ((double)i->second.rxBytes*8) / (i->second.timeLastRxPacket - i->second.timeFirstTxPacket).GetSeconds()/1024/1024 ) << "Mbps" << std::endl;
			Thrpt +=( ((double)i->second.rxBytes*8) / (i->second.timeLastRxPacket - i->second.timeFirstTxPacket).GetSeconds()/1024/1024 );
			Delay += i->second.delaySum.GetSeconds();
			received_pkts+=i->second.rxPackets;
			PLoss+=i->second.txPackets-i->second.rxPackets ;
			transmitted_packets+=i->second.txPackets;
		}
	}

	std::cout << "  Packet loss = " << PLoss<< "\n";
	std::cout << "Percentage of Lost packets = "<<((PLoss/transmitted_packets)*100)<<std::endl;
	std::cout << "Total  Delay = " << (Delay/received_pkts*1000)<< " msec" <<std::endl;
	std::cout << " Total Rx Bytes: " << received_bytes<<std::endl;
	std::cout <<" Total_Throughput: " << Thrpt<<std::endl;

*/


	Simulator::Destroy ();
	return 0;
}
