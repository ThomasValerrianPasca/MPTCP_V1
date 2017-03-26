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
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/epc-enb-application.h"
#include "ns3/routerlayer.h"
#include "map"
#include "sstream"
#include "time.h"
#include "ns3/ipv4-interface.h"
#include "ns3/bridge-helper.h"
//#include "ns3/common.h"
//#include "ns3/ltewifiinterface.h"

//#include "ns3/gtk-config-store.h"

using namespace ns3;

/*void callbackltewifiinterface(Ptr<LteWifiInterface> ltewifi)
{
	ltewifi->ReadData();
	std::cout<<"callback called"<<std::endl;
}*/
/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */
NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");
//LogComponentEnable("PacketSink", LOG_LEVEL_INFO);

/*static void
CwndTracer (uint32_t oldval, uint32_t newval)
{
  std::cout<<"Moving cwnd from " << oldval << " to " << newval<<std::endl;
}*/


//Topology
//		 RH   (1.0.0.2)
//	    |  |
//	 eNB	WifiAP
//    \    /
//		UE
int
main (int argc, char *argv[])
{
	LogComponentEnable("MpTcpSocketBase", LOG_LEVEL_DEBUG);
	int seedval=1;
	uint64_t runval=1;
	int lte=1;
	uint16_t number_of_IBX = 1;
	uint16_t numberOfClients=1;//Users
	double simTime = 4.8;
	double distance = 60;
	long xvalue=30;
	long yvalue=10;
	double interPacketInterval =1000;
	bool UDP=false;
	uint32_t queue_size_lte_rlc_um=10*1024; //Bytes default 10*1024
	uint32_t queue_size_lte_rlc_tm=2*1024*1024; //Bytes
	/*uint16_t tx_Queue_Hold_Delay_um=;
	uint16_t tx_Queue_Hold_Delay_tm=;*/
	//Wi-Fi Queue length and delay
	uint32_t MaxPacketNumber_in_wifi_queue=100; //Number of packets == default 400
	Time MaxDelay_in_wifi_queue=Seconds(0.5); //In Seconds == default 0.05
	int tcp_type=1;
	double epc_delay=0; //15 msec
	double backhaul_delay=0; //10 msec
	uint64_t holdingtime=0;
	uint32_t lte_frac=5;
	uint32_t wifi_frac=5;

	bool very_low_lte_utilization=false;
	bool skip_ack=false;
	bool DIDA=false;
	bool boost_ack=false;
	//256=32 mbps
	//512= 16 mbps
	//1024==8 mbps
	//2048 = 4 Mbps
	//4096 = 2 Mbps
	//8192==1 mbps
	ns3::PacketMetadata::Enable ();
	// Command line arguments
	CommandLine cmd;
	cmd.AddValue("number_of_IBX", "Number of eNodeBs + Wi-Fi pairs", number_of_IBX);
	cmd.AddValue("numberOfClients", "Number of UEs", numberOfClients);
	cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
	cmd.AddValue("distance", "Distance between eNBs [m]", distance);
	cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
	cmd.AddValue("xvalue", "X co-ordinate postion", xvalue);
	cmd.AddValue("yvalue", "Y co-ordinate postion", yvalue);
	cmd.AddValue("lte", "1- only lte, 2- only wifi, 3- Lte+wifi packet split, 4- flow split LTE +Wi-fi  5- flow based LTE UL DL and Wi-Fi DL, 6- flow based LTE UL DL and Wi-Fi (TCP Data) DL, 7 - Packet split with Wi-Fi only in downlink", lte);
	cmd.AddValue("queue_size_lte_rlc_um","LTE Queue Size for RLC UM",queue_size_lte_rlc_um);
	cmd.AddValue("queue_size_lte_rlc_tm","LTE Queue Size for RLC UM",queue_size_lte_rlc_tm);
	cmd.AddValue("MaxPacketNumber_in_wifi_queue","Maximum number of Wi-Fi packets in Wi-Fi Queue",MaxPacketNumber_in_wifi_queue);
	cmd.AddValue("MaxDelay_in_wifi_queue","Maximum delay Wi-Fi packets can be hold on Wi-Fi Queue",MaxDelay_in_wifi_queue);
	cmd.AddValue("UDP","Traffic Type",UDP);
	cmd.AddValue("tcp_type","tye of tcp  1- reno, 2 - cubic",tcp_type);
	cmd.AddValue("seedval", "1 or more", seedval);
	cmd.AddValue("runval", "1 or more", runval);
	cmd.AddValue("epc_delay", "Delay will be set for epc",epc_delay);
	cmd.AddValue("backhaul_delay", "Delay will be set for backhaul internet",backhaul_delay);
	cmd.AddValue("holdingtime","Holding Time to be set",holdingtime);
	cmd.AddValue("very_low_lte_utilization","When the Number of flows through LTE is very small ",very_low_lte_utilization);
	cmd.AddValue("lte_frac","Fraction sent to LTE", lte_frac);
	cmd.AddValue("wifi_frac","Fraction sent to WiFi", wifi_frac);
	cmd.AddValue("skip_ack","Skipping ACK with varying holding time",skip_ack);
	cmd.AddValue("DIDA","Pseudo delay insequence", DIDA);
	cmd.AddValue("boost_ack","Boost Acknowledgement",boost_ack);

	cmd.Parse(argc, argv);

	//std::cout<<"queue size= "<<MaxPacketNumber_in_wifi_queue<<std::endl;
	ns3::RngSeedManager::SetRun(runval);
	ns3::RngSeedManager::SetSeed(seedval) ;
	Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
	Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
	epcHelper->SetAttribute("S1uLinkDelay",TimeValue(Seconds(epc_delay)));
	lteHelper->SetEpcHelper (epcHelper);
	lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults();
	Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(queue_size_lte_rlc_um));
	Config::SetDefault ("ns3::LteRlcTm::MaxTxBufferSize", UintegerValue(queue_size_lte_rlc_tm));
	std::cout<<"LTE buffer size="<<queue_size_lte_rlc_um<<std::endl;
	//Config::SetDefault ("ns3::WifiMacQueue::MaxPacketNumber", UintegerValue(MaxPacketNumber_in_wifi_queue));
	Config::SetDefault ("ns3::WifiMacQueue::MaxDelay", TimeValue (MaxDelay_in_wifi_queue));
	Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
	Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
	Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
	Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(100));
	Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
	Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(2)); // Sink
	Config::SetDefault("ns3::MpTcpSocketBase::CongestionControl", StringValue("UNCOUPLED")); //Linked_Increases, COUPLED_INC, UNCOUPLED
	Config::SetDefault("ns3::MpTcpSocketBase::PathManagement", StringValue("FullMesh"));//NdiffPorts,FullMesh


	Ptr<Node> pgw = epcHelper->GetPgwNode ();



	NodeContainer integrated_box;
	NodeContainer integrated_ue;
	NodeContainer accespoint;

	//ADD Wi-Fi and LTE into UE (Lets call wifi with lte devices as integrated UE)
	integrated_ue.Create(numberOfClients);
	integrated_box.Create(number_of_IBX);
	accespoint.Create(1);


	// Create a single RemoteHost
	NodeContainer remoteHostContainer;
	remoteHostContainer.Create (1);
	Ptr<Node> remoteHost = remoteHostContainer.Get (0);
	InternetStackHelper internet;
	internet.Install (remoteHostContainer);


	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", StringValue ("10Gbps"));
	csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));


	NodeContainer Wifiroute;
	Wifiroute.Add(remoteHostContainer.Get(0));
	Wifiroute.Add(accespoint);
	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install (Wifiroute);


	// Create the Internet
	PointToPointHelper p2ph;
	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
	p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (backhaul_delay)));
	NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
	Ipv4AddressHelper ipv4h;
	ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
	Ipv4AddressHelper ipv4h_wifi;

	internet.Install(Wifiroute.Get(1));
	ipv4h_wifi.SetBase ("20.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer csma_internetIpIfaces = ipv4h_wifi.Assign(csmaDevices);
	// interface 0 is localhost, 1 is the p2p device
	Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
	std::cout<<"Remote host Address first interface address = : "<<internetIpIfaces.GetAddress(1,0)<< " Remote host second interface address= "<< csma_internetIpIfaces.GetAddress(0,0)<<std::endl;
	std::cout<<"Wifi link interface address = : "<<csma_internetIpIfaces.GetAddress(0,0)<<"  "<<csma_internetIpIfaces.GetAddress(1,0)<<std::endl;





	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
	remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


	// Install Mobility Model
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	positionAlloc->Add (Vector(20, 10, 4));
	positionAlloc->Add (Vector(30, 30, 4));
	positionAlloc->Add (Vector(40, 40, 4));
	positionAlloc->Add (Vector(50, 50, 4));

	MobilityHelper mobility;
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.SetPositionAllocator(positionAlloc);
	mobility.Install(integrated_box);

	mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator","X",StringValue("0.0"),"Y",StringValue("0.0"),"Rho",StringValue("ns3::UniformRandomVariable[Min=0|Max=30]"),"Theta",StringValue("ns3::UniformRandomVariable[Min=0|Max=360]"));
	mobility.Install(integrated_ue);





	/*WifiHelper wifi=WifiHelper::Default ();
	wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
	//wifi.SetRemoteStationManager ("ns3::AarfWifiManager", "FragmentationThreshold", UintegerValue (2500));
	StringValue DataRate;
	DataRate = StringValue("OfdmRate72_2MbpsBW20MHz"); //OfdmRate72_2MbpsBW20MHz   or OfdmRate65MbpsBW20MHz
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate, "ControlMode", DataRate);
	HtWifiMacHelper mac = HtWifiMacHelper::Default ();
	//mac.SetMsduAggregatorForAc (AC_BE, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (2000));*/



	WifiHelper wifi=WifiHelper::Default ();
	wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
	wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
	NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();




	YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	YansWifiPhy yph;
	yph.SetTxPowerStart(23.0);
	yph.SetTxPowerEnd(23.0);
	phy.SetChannel (channel.Create ());

	// Install LTE Devices to the nodes and wifi devices to node
	NetDeviceContainer enbLteDevs;
	for(uint16_t i =0; i<number_of_IBX; i++){
		enbLteDevs.Add(lteHelper->InstallEnbDevice(integrated_box.Get(i)));
	}
	NetDeviceContainer ueLteDevs;
	for(uint16_t i=0;i<numberOfClients;i++) {
		ueLteDevs.Add(lteHelper->InstallUeDevice (integrated_ue.Get(i)));
	}

	Ptr<LteEnbPhy> enb0Phy = enbLteDevs.Get (0)-> GetObject<LteEnbNetDevice> ()->GetPhy ();
	enb0Phy->SetTxPower (23.0);

	Ssid ssid = Ssid ("Integrated_Box0");
	mac.SetType ("ns3::StaWifiMac",
			"Ssid", SsidValue (ssid),
			"ActiveProbing", BooleanValue (true));

	NetDeviceContainer wifiDevs;
	for(uint16_t i=0;i<((numberOfClients/number_of_IBX)*(1));i++) 
	{
		wifiDevs.Add(wifi.Install(phy,mac,integrated_ue.Get(i)));
	}

	MobilityHelper mobility1;
	mobility1.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility1.SetPositionAllocator(positionAlloc);
	mobility1.Install(accespoint);

	mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
	NetDeviceContainer wifiApDevs =wifi.Install(phy,mac,accespoint);



	// Install the IP stack on the UEs///////////////////////////////////
	InternetStackHelper stack;
	stack.Install (integrated_ue);
	//stack.Install (accespoint);

	Ipv4InterfaceContainer ueIpIface;
	ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

	Ipv4AddressHelper address;

	//Just commented
	//Ipv4AddressHelper ipv4h_wifi;
	//ipv4h_wifi.SetBase ("20.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer wifiIpIface;
	wifiIpIface=ipv4h_wifi.Assign(wifiDevs);
	wifiIpIface=ipv4h_wifi.Assign(wifiApDevs);

	for (uint16_t i = 0; i < numberOfClients; i++)
	{
		std::cout<<" UE Interface address"<<ueIpIface.GetAddress(i,0)<<std::endl;
		std::cout<<" Wifi Interface address"<<wifiIpIface.GetAddress(i,0)<<std::endl;
	}

	// Assign IP address to UEs, and install applications

	for (uint32_t u = 0; u < numberOfClients; ++u)
	{
		Ptr<Node> ueNode = integrated_ue.Get (u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
		ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
		std::cout<< "Gateway= "<<epcHelper->GetUeDefaultGatewayAddress ()<<std::endl;
	}

	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	Ipv4StaticRoutingHelper ipv4RoutingHelperwifi;
	Ptr<Ipv4StaticRouting> remoteHostStaticRoutingwifi = ipv4RoutingHelperwifi.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
	remoteHostStaticRoutingwifi->AddNetworkRouteTo (Ipv4Address ("20.0.0.0"), Ipv4Mask ("255.0.0.0"),2);

	//remoteHostStaticRoutingwifi->AddNetworkRouteTo (Ipv4Address ("20.0.0.0"), Ipv4Mask ("255.0.0.0"),1);
	//Ptr<Ipv4StaticRouting> wifiaprouting = ipv4RoutingHelperwifi.GetStaticRouting ( accespoint.Get(0)->GetObject<Ipv4>());
	//wifiaprouting->AddNetworkRouteTo(Ipv4Address("1.0.0.0"), Ipv4Mask("255.0.0.0"),2);
	for ( uint16_t u = 0 ; u < numberOfClients; u++){
		Ptr<Ipv4StaticRouting> wifiuerouting = ipv4RoutingHelperwifi.GetStaticRouting ( integrated_ue.Get(u)->GetObject<Ipv4>());
		wifiuerouting->SetDefaultRoute (Ipv4Address("20.0.0.2"), 2);
	}

	BridgeHelper bridge;

	NetDeviceContainer bridgeDev;
	NetDeviceContainer NDC_wifi_ap;
	NDC_wifi_ap.Add(wifiApDevs);
	NDC_wifi_ap.Add(csmaDevices.Get(1));
	bridgeDev = bridge.Install (accespoint.Get(0),NDC_wifi_ap);


	uint16_t ue_num = 0;
	for(ue_num = 0;ue_num<((numberOfClients/number_of_IBX)*(1));ue_num++) 
	{
		lteHelper->Attach (ueLteDevs.Get(ue_num), enbLteDevs.Get(0));
	}

	// Install and start applications on UEs and remote host
	//TCP Application============================
	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


	if(!UDP)
	{

		uint16_t port=2000;
		uint16_t port1=1001;
		for (uint32_t u = 0; u < numberOfClients; ++u)
		{
			/*
			BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress (remoteHostAddr, port));
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source.SetAttribute ("MaxBytes", UintegerValue (0));
			source.SetAttribute("SendSize",UintegerValue (1024));
			ApplicationContainer sourceApps = source.Install (integrated_ue.Get(u));
			sourceApps.Start (Seconds (1.0));
			sourceApps.Stop (Seconds (simTime));
			PacketSinkHelper sink ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
			ApplicationContainer sinkApps = sink.Install (remoteHost);
			sinkApps.Start (Seconds (1.0));
			sinkApps.Stop (Seconds (simTime));
	        //std::cout<<"Uplink UE = "<<u<<" Source Port "<<port<<std::endl;
			port++;


			BulkSendHelper source1 ("ns3::TcpSocketFactory",InetSocketAddress (remoteHostAddr, port));
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source1.SetAttribute ("MaxBytes", UintegerValue (0));
			source1.SetAttribute("SendSize",UintegerValue (1024));
			ApplicationContainer sourceApps1 = source1.Install (integrated_ue.Get(u));
			sourceApps1.Start (Seconds (1.0));
			sourceApps1.Stop (Seconds (simTime));
			PacketSinkHelper sink1 ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
			ApplicationContainer sinkApps1 = sink1.Install (remoteHost);
			sinkApps1.Start (Seconds (1.0));
			sinkApps1.Stop (Seconds (simTime));
	        //std::cout<<"Uplink UE = "<<u<<" Source Port "<<port<<std::endl;
			port++;
			 */




			MpTcpBulkSendHelper source4 ("ns3::TcpSocketFactory",InetSocketAddress (ueIpIface.GetAddress (u), port1));
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source4.SetAttribute ("MaxBytes", UintegerValue (0));
			source4.SetAttribute("SendSize",UintegerValue (1024));
			ApplicationContainer sourceApps4 = source4.Install (remoteHost);
			sourceApps4.Start (Seconds (1.0));
			sourceApps4.Stop (Seconds (simTime));
			MpTcpPacketSinkHelper sink4 ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port1));
			ApplicationContainer sinkApps4 = sink4.Install (integrated_ue.Get(u));
			sinkApps4.Start (Seconds (1.0));
			sinkApps4.Stop (Seconds (simTime));
			//	std::cout<<"Downlink UE = "<<u<<" Source Port "<<port1<<std::endl;
			port1++;


			MpTcpBulkSendHelper source5 ("ns3::TcpSocketFactory",InetSocketAddress (ueIpIface.GetAddress (u), port1));
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source5.SetAttribute ("MaxBytes", UintegerValue (0));
			source5.SetAttribute("SendSize",UintegerValue (1024));
			ApplicationContainer sourceApps5 = source5.Install (remoteHost);
			sourceApps5.Start (Seconds (1.0));
			sourceApps5.Stop (Seconds (simTime));
			MpTcpPacketSinkHelper sink5 ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port1));
			ApplicationContainer sinkApps5 = sink5.Install (integrated_ue.Get(u));
			sinkApps5.Start (Seconds (1.0));
			sinkApps5.Stop (Seconds (simTime));
			//	std::cout<<"Downlink UE = "<<u<<" Source Port "<<port1<<std::endl;
			port1++;


			/*MpTcpBulkSendHelper source6 ("ns3::TcpSocketFactory",InetSocketAddress (ueIpIface.GetAddress (u), port1));
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source6.SetAttribute ("MaxBytes", UintegerValue (0));
			 *//*source6.SetAttribute("SendSize",UintegerValue (1024));
			ApplicationContainer sourceApps6 = source6.Install (remoteHost);
			sourceApps6.Start (Seconds (1.0));
			sourceApps6.Stop (Seconds (simTime));
			PacketSinkHelper sink6 ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port1));
			ApplicationContainer sinkApps6 = sink6.Install (integrated_ue.Get(u));
			sinkApps6.Start (Seconds (1.0));
			sinkApps6.Stop (Seconds (simTime));
			//	std::cout<<"Downlink UE = "<<u<<" Source Port "<<port1<<std::endl;
			port1++;

			BulkSendHelper source7 ("ns3::TcpSocketFactory",InetSocketAddress (ueIpIface.GetAddress (u), port1));
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source7.SetAttribute ("MaxBytes", UintegerValue (0));
			source7.SetAttribute("SendSize",UintegerValue (1024));
			ApplicationContainer sourceApps7 = source7.Install (remoteHost);
			sourceApps7.Start (Seconds (1.0));
			sourceApps7.Stop (Seconds (simTime));
			PacketSinkHelper sink7 ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port1));
			ApplicationContainer sinkApps7 = sink7.Install (integrated_ue.Get(u));
			sinkApps7.Start (Seconds (1.0));
			sinkApps7.Stop (Seconds (simTime));
			//	std::cout<<"Downlink UE = "<<u<<" Source Port "<<port1<<std::endl;
			port1++;*/


			// This is to make the single flow active
			if(very_low_lte_utilization){
				UdpClientHelper source01 (remoteHostAddr, port);
				// Set the amount of data to send in bytes.  Zero is unlimited.
				source01.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
				source01.SetAttribute ("MaxPackets", UintegerValue(1000000));
				source01.SetAttribute ("PacketSize", UintegerValue(1024));
				ApplicationContainer sourceApps01 = source01.Install (integrated_ue.Get(u));
				sourceApps01.Start (Seconds (1.0));
				sourceApps01.Stop (Seconds (simTime));
				PacketSinkHelper sink01 ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
				ApplicationContainer sinkApps01 = sink01.Install (remoteHost);
				sinkApps01.Start (Seconds (1.0));
				sinkApps01.Stop (Seconds (simTime));
				port++;
			}
		}
	}
	else{


		//Config::ConnectWithoutContext("BulkSendApplication",MakeCallback(&trace))
		/*for (uint32_t k=0;k<integrated_ue.GetN();k++){
		Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (integrated_ue.Get (k), TcpSocketFactory::GetTypeId ());
		  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndTracer));
		}
		oss << "/NodeList/"<< integrated_ue.Get(k)->GetId()<< "$/ns3::TcpL4Protocol/SocketList/"<<0<<"/CongestionWindow";
		Config::ConnectWithoutContext (oss.str(), MakeCallback (&CwndTracer));

		Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/1/CongestionWindow", MakeCallback (&CwndTracer)); */


		//UDP Application======================================
		//uint16_t port=2000;
		uint16_t port1=1000;
		for (uint32_t u = 0; u < numberOfClients; ++u)
		{
			/*UdpClientHelper source (remoteHostAddr, port);
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
			source.SetAttribute ("MaxPackets", UintegerValue(1000000));
			source.SetAttribute ("PacketSize", UintegerValue(1024));
			ApplicationContainer sourceApps = source.Install (integrated_ue.Get(u));
			sourceApps.Start (Seconds (1.0));
			sourceApps.Stop (Seconds (simTime));
			PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
			ApplicationContainer sinkApps = sink.Install (remoteHost);
			sinkApps.Start (Seconds (1.0));
			sinkApps.Stop (Seconds (simTime));
			port++;

			UdpClientHelper source01 (remoteHostAddr, port);
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source01.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
			source01.SetAttribute ("MaxPackets", UintegerValue(1000000));
			source01.SetAttribute ("PacketSize", UintegerValue(1024));
			ApplicationContainer sourceApps01 = source01.Install (integrated_ue.Get(u));
			sourceApps01.Start (Seconds (1.0));
			sourceApps01.Stop (Seconds (simTime));
			PacketSinkHelper sink01 ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
			ApplicationContainer sinkApps01 = sink01.Install (remoteHost);
			sinkApps01.Start (Seconds (1.0));
			sinkApps01.Stop (Seconds (simTime));
			port++;*/



			UdpClientHelper source1 (ueIpIface.GetAddress (u), port1);
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source1.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
			source1.SetAttribute ("MaxPackets", UintegerValue(1000000));
			source1.SetAttribute ("PacketSize", UintegerValue(1024));
			ApplicationContainer sourceApps1 = source1.Install (remoteHost);
			sourceApps1.Start (Seconds (1.0));
			sourceApps1.Stop (Seconds (simTime));
			PacketSinkHelper sink1 ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port1));
			ApplicationContainer sinkApps1 = sink1.Install (integrated_ue.Get(u));
			sinkApps1.Start (Seconds (1.0));
			sinkApps1.Stop (Seconds (simTime));
			port1++;

			//------------------------test
			UdpClientHelper source11 (ueIpIface.GetAddress (u), port1);
			// Set the amount of data to send in bytes.  Zero is unlimited.
			source11.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
			source11.SetAttribute ("MaxPackets", UintegerValue(1000000));
			source11.SetAttribute ("PacketSize", UintegerValue(1024));
			ApplicationContainer sourceApps11 = source11.Install (remoteHost);
			sourceApps11.Start (Seconds (1.0));
			sourceApps11.Stop (Seconds (simTime));
			PacketSinkHelper sink11 ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port1));
			ApplicationContainer sinkApps11 = sink11.Install (integrated_ue.Get(u));
			sinkApps11.Start (Seconds (1.0));
			sinkApps11.Stop (Seconds (simTime));
			port1++;
			//-----------------------------try
		}

	}
	//=======================================================================================

	/* uint16_t port=2000;
    uint16_t port1=1001;
    std::cout<<"Himank Gupta"<<std::endl;
    for (uint32_t u = 0; u < numberOfClients; ++u)
    {
  	 UdpClientHelper source (ueIpIface.GetAddress (u), port);
  	 	    // Set the amount of data to send in bytes.  Zero is unlimited.
  	  source.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
  	  source.SetAttribute ("MaxPackets", UintegerValue(1000000));
  	  source.SetAttribute ("PacketSize", UintegerValue(1024));
  	  ApplicationContainer sourceApps = source.Install (remoteHost);
  	  sourceApps.Start (Seconds (1.0));
  	   sourceApps.Stop (Seconds (3.0));
  	   PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
  	   ApplicationContainer sinkApps = sink.Install (integrated_ue.Get(u));
  	   sinkApps.Start (Seconds (1.0));
  	   sinkApps.Stop (Seconds (3.0));
  	   port++;
  	   //------------------------test
  	   UdpClientHelper source01 (ueIpIface.GetAddress (u), port);
  	   	 	    // Set the amount of data to send in bytes.  Zero is unlimited.
  	   	  source01.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
  	   	  source01.SetAttribute ("MaxPackets", UintegerValue(1000000));
  	   	  source01.SetAttribute ("PacketSize", UintegerValue(1024));
  	   	  ApplicationContainer sourceApps01 = source01.Install (remoteHost);
  	   	  sourceApps01.Start (Seconds (1.0));
  	   	   sourceApps01.Stop (Seconds (3.0));
  	   	   PacketSinkHelper sink01 ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
  	   	   ApplicationContainer sinkApps01 = sink01.Install (integrated_ue.Get(u));
  	   	   sinkApps01.Start (Seconds (1.0));
  	   	   sinkApps01.Stop (Seconds (3.0));
  	   	   port++;
  	   //-----------------------------try


  	   UdpClientHelper source1 (ueIpIface.GetAddress (u), port1);
  	   	 	    // Set the amount of data to send in bytes.  Zero is unlimited.
  	    // Set the amount of data to send in bytes.  Zero is unlimited.
  	     source1.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
  	  	  source1.SetAttribute ("MaxPackets", UintegerValue(1000000));
  	  	  source1.SetAttribute ("PacketSize", UintegerValue(1024));
  	   	  ApplicationContainer sourceApps1 = source1.Install (remoteHost);
  	   	  sourceApps1.Start (Seconds (1.0));
  	   	   sourceApps1.Stop (Seconds (3.0));
  	   	   PacketSinkHelper sink1 ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port1));
  	   	   ApplicationContainer sinkApps1 = sink1.Install (integrated_ue.Get(u));
  	   	   sinkApps1.Start (Seconds (1.0));
  	   	   sinkApps1.Stop (Seconds (3.0));
  	   	   port1++;

  	   	   //------------------------test
  	   		   UdpClientHelper source11 (ueIpIface.GetAddress (u), port1);
  	   		   	 	    // Set the amount of data to send in bytes.  Zero is unlimited.
  	   		   	  source11.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
  	   		   	  source11.SetAttribute ("MaxPackets", UintegerValue(1000000));
  	   		   	  source11.SetAttribute ("PacketSize", UintegerValue(1024));
  	   		   	  ApplicationContainer sourceApps11 = source11.Install (remoteHost);
  	   		   	  sourceApps11.Start (Seconds (1.0));
  	   		   	   sourceApps11.Stop (Seconds (3.0));
  	   		   	   PacketSinkHelper sink11 ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port1));
  	   		   	   ApplicationContainer sinkApps11 = sink11.Install (integrated_ue.Get(u));
  	   		   	   sinkApps11.Start (Seconds (1.0));
  	   		   	   sinkApps11.Stop (Seconds (3.0));
  	   		   	   port1++;*/
	//-----------------------------try

	// }




	/* uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  //uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  //for (uint32_t u = 0; u < integrated_ue.GetN (); ++u)

  for (uint32_t u = 0; u < numberOfClients; ++u)
    {
	  std::cout<<"UE number"<<u<<std::endl;
     // ++ulPort;
      //++otherPort;
	  ++dlPort;
	  std::cout << "port " << dlPort << std::endl;
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
    //  PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
//      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
    // serverApps.Add (dlPacketSinkHelper.Install (integrated_ue.Get(u)));
     // serverApps.Add (ulPacketSinkHelper.Install (integrated_ue.Get(u)));

      serverApps.Add (dlPacketSinkHelper.Install (integrated_ue.Get(u)));
     // serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
//      serverApps.Add (packetSinkHelper.Install (integrated_ue.Get(u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);

      dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));
      dlClient.SetAttribute ("PacketSize", UintegerValue(1024));

//      UdpClientHelper ulClient (remoteHostAddr, ulPort);
//      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
//      ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));
//
     // UdpClientHelper client (ueIpIface.GetAddress (u), dlPort);
     // client.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
     // client.SetAttribute ("MaxPackets", UintegerValue(1000000));
     // client.SetAttribute ("PacketSize", UintegerValue(1024));

      clientApps.Add (dlClient.Install (remoteHost));
     // clientApps.Add (client.Install (remoteHost));
//      if (u+1 < integrated_ue.GetN ())
//        {
//          clientApps.Add (client.Install (integrated_ue.Get(u+1)));
//        }
//      else
//        {
//          clientApps.Add (client.Install (integrated_ue.Get(0)));
//        }
      std::cout<<"UE number"<<u<<std::endl;
            ++ulPort;
            //++otherPort;
      	 // ++dlPort;
      	 std::cout << "port " << dlPort << std::endl;
           // PacketSinkHelper dlPacketSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
           PacketSinkHelper ulPacketSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      //      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
          // serverApps.Add (dlPacketSinkHelper.Install (integrated_ue.Get(u)));
           // serverApps.Add (ulPacketSinkHelper.Install (integrated_ue.Get(u)));

         //   serverApps.Add (dlPacketSinkHelper1.Install (integrated_ue));
            serverApps.Add (ulPacketSinkHelper1.Install (remoteHost));
      //      serverApps.Add (packetSinkHelper.Install (integrated_ue.Get(u)));

            UdpClientHelper dlClient1 (remoteHostAddr, dlPort);

            dlClient1.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
            dlClient1.SetAttribute ("MaxPackets", UintegerValue(1000000));
            dlClient1.SetAttribute ("PacketSize", UintegerValue(1024));

      //      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      //      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      //      ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));
      //

            clientApps.Add (dlClient1.Install (integrated_ue.Get(u)));

    }
  serverApps.Start (Seconds (1.0));
  clientApps.Start (Seconds (1.0));
  serverApps.Stop(Seconds(3.0));
  clientApps.Stop(Seconds(3.0));*/

	lteHelper->EnableTraces ();
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor;

	monitor= flowmon.Install (integrated_ue);

	flowmon.Install (remoteHost);
	//	for(uint16_t i=0;i<numberOfClients;i++) {
	//		phy.EnablePcap ("Wifi", wifiDevs[i]);
	//	}
	phy.EnablePcap ("Wifidev", wifiDevs);
	/*
	 epcHelper->EnablePcap("Lte",ueLteDevs);
	 */
	Simulator::Stop(Seconds(simTime));

	// Uncomment to enable PCAP tracing
	p2ph.EnablePcapAll("Mptcp-1-first-link");
	csma.EnablePcapAll("Mptcp-2-second-link");
	//Simulator::Schedule(MilliSeconds(100),&callbackltewifiinterface,ltewifiinterface);
	//AsciiTraceHelper ascii;
	//epcHelper->EnableAsciiAll (ascii.CreateFileStream ("trace.tr"));

	//-----------------REM Code------------------------
	/*Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
	  remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
	  remHelper->SetAttribute ("OutputFile", StringValue ("rem.out"));
	  remHelper->SetAttribute ("XMin", DoubleValue (-1500.0));
	  remHelper->SetAttribute ("XMax", DoubleValue (2500.0));
	  remHelper->SetAttribute ("YMin", DoubleValue (-1500.0));
	  remHelper->SetAttribute ("YMax", DoubleValue (2500.0));
	  remHelper->SetAttribute ("Z", DoubleValue (0.0));
	  remHelper->Install ();*/
	// --------------------------------------------------------------------

	//	 Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
	//remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/2"));
	//remHelper->SetAttribute ("OutputFile", StringValue ("rem.out"));
	//remHelper->SetAttribute ("XMin", DoubleValue (-400.0));
	//remHelper->SetAttribute ("XMax", DoubleValue (400.0));
	//remHelper->SetAttribute ("XRes", UintegerValue (100));
	//remHelper->SetAttribute ("YMin", DoubleValue (-300.0));
	//remHelper->SetAttribute ("YMax", DoubleValue (300.0));
	//remHelper->SetAttribute ("YRes", UintegerValue (75));
	//remHelper->SetAttribute ("Z", DoubleValue (0.0));
	//remHelper->SetAttribute ("UseDataChannel", BooleanValue (true));
	//remHelper->SetAttribute ("RbId", IntegerValue (10));
	//remHelper->Install ();

	Simulator::Run();

	/*GtkConfigStore config;
  config.ConfigureAttributes();*/

	monitor->CheckForLostPackets ();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
	//uint32_t lostpkt=0;
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
			//  std::cout << "  local Delay = " << i->second.delaySum.GetSeconds()/i->second.rxPackets << "\n";
			PLoss+=i->second.txPackets-i->second.rxPackets ;
			transmitted_packets+=i->second.txPackets;
			//lostpkt+=i->second.lostPackets;
			//    std::iterator<int,float> s=i->second.packetsDropped.iterator;

		}
	}



	std::cout << "  Packet loss = " << PLoss<< "\n";
	std::cout << "Percentage of Lost packets = "<<((PLoss/transmitted_packets)*100)<<std::endl;
	std::cout << "Total  Delay = " << (Delay/received_pkts*1000)<< " msec" <<std::endl;
	//" "<< UDP <<" "<< queue_size_lte_rlc_um<< " "<< MaxDelay_in_wifi_queue<<" " <<lte << " interPacketInterval = " <<interPacketInterval<<"\n";
	std::cout << " Total Rx Bytes: " << received_bytes<<std::endl;
	// std::cout << " Total Lost Pkt : " << lostpkt;
	//std::cout << " Expected Throughput : " << (received_bytes*8)/total_time;
	std::cout <<"split "<<lte_frac<< " Holding_time: "<< holdingtime+0.1<<" Total_Throughput: " << Thrpt<<std::endl;
	std::cout << " interPacketInterval = " <<interPacketInterval<<"\n"  <<std::endl;
	Simulator::Destroy();
	//return 0;
	return 0;

}

