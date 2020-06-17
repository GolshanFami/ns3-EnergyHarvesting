	#include <iostream>
	#include <fstream>
	#include <vector>
	#include <string>
	#include "ns3/netanim-module.h"
	#include "ns3/core-module.h"
	#include "ns3/network-module.h"
	#include "ns3/internet-module.h"
	#include "ns3/applications-module.h"
	#include "ns3/wifi-module.h"
	#include "ns3/yans-wifi-helper.h"
	#include "ns3/yans-wifi-channel.h"
	#include "ns3/ssid.h"
	#include "ns3/mobility-module.h"
	#include "ns3/flow-monitor-module.h"
	#include "ns3/flow-monitor-helper.h"
	#include "ns3/basic-energy-source.h"
	#include "ns3/wifi-radio-energy-model.h"
	#include "ns3/basic-energy-source-helper.h"
	#include "ns3/wifi-radio-energy-model-helper.h"
	#include "ns3/energy-source-container.h"
	#include "ns3/device-energy-model-container.h"
	#include "ns3/energy-module.h"
	#include "ns3/command-line.h"
	#include "ns3/log.h"
	#include "ns3/node.h"
	#include "ns3/simulator.h"
	#include "ns3/double.h"
	#include "ns3/config.h"
	#include "ns3/string.h"
	#include "ns3/on-off-helper.h"
	#include "ns3/netanim-module.h"
	#include "ns3/propagation-loss-model.h"
	#include "ns3/propagation-delay-model.h"
	#include "ns3/rv-battery-model.h"
	#include "ns3/sun-harvester-module.h"
	#include "ns3/coap-client-server-helper.h"
//=========================================================
	using namespace ns3;
	NS_LOG_COMPONENT_DEFINE ("1nodesWifi-EnergyHarvestingwithcontroler-rvbattery");
//3.1 ***************Trace function for remaining energy at node***************
	template <int node>
	void RemainingEnergy (double oldValue, double remainingEnergy){
	std::stringstream ss;
  	ss << "RemainingEnergy_" << node << ".log";
 	static std::fstream f (ss.str ().c_str (), std::ios::out);
  	f << Simulator::Now ().GetSeconds () <<   "    "   <<remainingEnergy  << std::endl;
	}
//===============================================================================
	template <int node>
	void RvBatteryModelBatteryLevel (double oldValue, double RvBatteryModelBatteryLevel){
	std::stringstream ss;
  	ss << "RvBatteryModelBatteryLevel_" << node << ".log";
 	static std::fstream f (ss.str ().c_str (), std::ios::out);
  	f << Simulator::Now ().GetSeconds () <<   "    "   <<RvBatteryModelBatteryLevel  << std::endl;
	}
//3.2 ***************Trace function for total energy consumption at node***************
	template <int node>
	void TotalEnergy (double oldValue, double TotalEnergy){
  	std::stringstream ss;
  	ss << "TotalEnergy_" << node << ".log";
  	static std::fstream f (ss.str ().c_str (), std::ios::out);
 	f << Simulator::Now ().GetSeconds ()<<   "    "   << TotalEnergy << std::endl;
 	}
//=======================================================================
	template <int node>
	void PhyStateTrace (std::string context, Time start, Time duration, WifiPhyState state){
	std::stringstream ss;
	ss << "state_" << node << ".log";
	static std::fstream f (ss.str ().c_str (), std::ios::out);
	f << Simulator::Now ().GetSeconds () << "    state=" << state << " start=" << start << " duration=" << duration << std::endl;
	}
	/*template <int node>
	void PhyRxOkTrace (std::string context, Ptr<const Packet> packet, double snr, WifiMode mode, WifiPreamble preamble){
	std::stringstream ss;
	ss << "please_" << node << ".log";
	static std::fstream f (ss.str ().c_str (), std::ios::out);
		f << Simulator::Now ().GetSeconds () << "PHYRXOK mode=" << mode << " snr=" << snr << " " << *packet << "throughput =" <<2400* log2(1+snr) << std::endl;
	}*/
	/*template <int node>
	void ContentionWindowTrace (std::string context, QosTxop Cw){
  	std::stringstream ss;
  	ss << "CwTrace_" << node << ".log";
  	static std::fstream f (ss.str ().c_str (), std::ios::out);
 	f << Simulator::Now ().GetSeconds ()<<   "    "   << Cw << std::endl;
 	}*/
//3.3 ***************Trace function for the total energy harvested by the node***************
	template <int node>
	void TotalEnergyHarvested (double oldValue, double TotalEnergyHarvested){
  	std::stringstream ss;
  	ss << "TotalEnergyHarvested_" << node << ".log";
  	static std::fstream f (ss.str ().c_str (), std::ios::out);
  	f << Simulator::Now ().GetSeconds () <<   "    "  << TotalEnergyHarvested << std::endl;
  	}
//=======================================================================
	static inline std::string
	PrintID (Ptr<Node> n){
	    uint32_t id = n->GetId();
	    std::ostringstream oss;
	    oss << "NODE ID: " << id << "  " << std::endl;
	    return oss.str ();
	}
	void
	RemainingEnergy (std::string context, double oldValue, double remainingEnergy){
  		NS_LOG_UNCOND ( Simulator::Now ().GetSeconds () << "s Current remaining energy = " << remainingEnergy << "J , "<< context);
  	}
	void TotalEnergy (std::string context, double oldValue, double totalEnergy){
    		NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "s Total energy consumed by radio = " << totalEnergy << "J , " << context);
	}
	
	void TotalEnergyHarvested (std::string context, double oldValue, double TotalEnergyHarvested){
	NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "s TotalEnergyHarvested = " <<  TotalEnergyHarvested  << "J , " << context);
	}

	static void CourseChange (NodeContainer nodes,std::string context){
		for(NodeContainer::Iterator iNode = nodes.Begin (); iNode != nodes.End (); ++iNode){
			Ptr<Node> object = *iNode;
			Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
			NS_ASSERT (position != 0);
			Vector pos = position->GetPosition ();
			std::cout << "At " << Simulator::Now ().GetSeconds () << ": Position(" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
		}
		std::cout << std::endl;
	}
//=======================================================================	
	/*static void SetInitialEnergy (Ptr<Node> n, double x){
	
 	Ptr<BasicEnergySource> energy = n->GetObject<BasicEnergySource>();
 	double tos = energy->GetInitialEnergy();
	tos = x;
	std::cout<< "energy"<< tos << std::endl;
  	energy->SetInitialEnergy(tos);
	}*/
//***************sleep function***************
	void ThroughputMonitor (FlowMonitorHelper *flowmon, Ptr<FlowMonitor> flowMo){
		std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMo->GetFlowStats();
		Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (flowmon->GetClassifier());
		std::cout<<"----------------------Instantaneous Flowmonitor Statistics-------------------"<<std::endl;
		for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){
			Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
			std::cout<<"Flow ID:  " << stats->first << " Src Addr "<< fiveTuple.sourceAddress << " Dst Addr "<<fiveTuple.destinationAddress<<std::endl;
			std::cout<<"Lost Packets =  " << stats->second.lostPackets <<"\n";
			std::cout<<"Tx Packets = " << stats->second.txPackets<<"\n";
			std::cout<<"Rx Packets = " << stats->second.rxPackets<<"\n";
			std::cout<<"Tx Bytes = " << stats->second.txBytes <<"\n";
			std::cout<<"Rx Bytes =  " << stats->second.rxBytes <<"\n";

			std::cout<<"Packet Loss Ratio (PLR) =  " << (float (stats->second.lostPackets*100) / (float(stats->second.txPackets)))<<"%"<<"\n";
			std::cout<<"Packet Delivery Ratio (PDR) =  " << (float (stats->second.rxPackets *100) / (float(stats->second.txPackets)))<< "%" <<"\n";
			std::cout<<"First Received Packet:  "<< stats->second.timeFirstTxPacket.GetSeconds()<<" Seconds"<<std::endl;
			std::cout<<"Duration:  "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<std::endl;
			std::cout<<"Last Received Packet:  "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
			std::cout<<"Throughput: " << stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1000000 << " Mbps"<<std::endl;
			std::cout<<"---------------------------------------------------------------------------"<<std::endl;
		}
			Simulator::Schedule(Seconds(1),&ThroughputMonitor, flowmon, flowMo);

	}

//***************Wake up function***************
//Step 4. ***************The Main Program***************
	int main (int argc, char *argv[]){
//4.1 ***************Setting Variables********************************
//4.1.1 ***************Setting Variables physical layer***************
	int seed = 2;
	std::string phyRate = "HtMcs7";
//4.1.2 ***************Setting Variables******************************
	double simulationTime =20;
	double flowmonstart = 0.0;
	double flowmonend = simulationTime - 1e-6;
//4.1.4 ***************Setting Variables for the Standard IEEE 802.11n ***************
    	double frequency = 2.4; 
//4.1.5***************Setting Variables for the topology of the Network***************
    	int nWifi =1;
	int theta = 360;
	int r =2;
//4.1.8 ***************Setting Variables for logging and tracing***************
   	bool verbose = true;
	bool tracing = true;
//4.1.9 ***************Setting Variables for Application layer***************
	bool Video = true;	
	bool ECG = false;
	bool EEG = false;
	bool EMR = false;
	bool TelemetryAlarm = false;
	int ECGDataRate = 11880; // 11.88 Kbps
	int ECGPacketSize = 119;// 119 bytes
	int VideoDataRate = 86800;// 86.8 kbps
	int VideoPacketSize = 172; // 160 bytes + 12 RTP header
	int EEGDataRate = 32000;//32kbps
	int EEGPacketSize = 127;//127 byte 
	int EMRDataRate = 4100000;//4.1Mbps
	int EMRPacketSize = 1500; // 1500 bytes, not a real time application
	int TelemetryAlarmDataRate = 5000;//5kbps
	int TelemetryAlarmPacketSize = 640; // 640 bytes 
//====================================================================
//4.1.9 ***************Setting AC paprameters for medical Application***************

	int Txoplimit_VO = 320;//us
	int Aifsn_VO = 2; 	// number of timeslot
	int Cwmax_VO = 16;
	int Cwmin_VO = 8;
	int Txoplimit_VI = 320; //microsecond
	int Aifsn_VI = 2;
	int Cwmax_VI = 32;
	int Cwmin_VI = 16;
	int Txoplimit_BE = 640; //microsecond
	int Aifsn_BE = 3;
	int Cwmax_BE = 1024;
	int Cwmin_BE = 32;
//4.1.9.2 ***************Setting Variables for generating Random number***************
	//std::string dataRate = "71000"; //71kbps
  	//int packetSize = 65500;
	//int packetSize = 1000;
//====================================================================
	double harvestingUpdateInterval = 1;
// ******************Battery setup*************************************
	double alpha =5000;//4002.7;
	double beta = 0.276;

	/*double mean = 5;
	double bound = 0;
	Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable> (); 
	x->SetAttribute ("Mean",DoubleValue(mean));
	x->SetAttribute ("Bound", DoubleValue (bound));*/

//4.2 ***************Command line argument parser setup***************
	CommandLine cmd;
	cmd.Parse (argc, argv);
	//cmd.AddValue ("dataRate", "Data rate", dataRate);
    	cmd.AddValue ("frequency", "Whether working in the 2.4 or 5.0 GHz band (other values gets rejected)", frequency);
	//cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
    	cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
    	cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
   	cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
    	cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
//=====================================================================
//4.3 **********************Turn off RTSS/CTS for frames below 2200 bytes******************************
 	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
//===================================================================== 
//4.3 **********************Disable fragmentation for frames below 2200 bytes******************************
	Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
//===================================================================== 
	RngSeedManager::SetSeed(seed);
	RngSeedManager::SetRun(2);
//4.3 **********************Print Result**************************
    if (verbose){
	//LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL); 	
	//LogComponentEnable("OnOffApplication",LOG_LEVEL_INFO) ;
	//LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
	//LogComponentEnable ("Txop", LOG_LEVEL_ALL);
	//LogComponentEnable ("BasicEnergySource", LOG_LEVEL_DEBUG);
	//LogComponentEnable ("EnergySource", LOG_LEVEL_DEBUG);
	//LogComponentEnable ("RvBatteryModel", LOG_LEVEL_DEBUG);
    }  
//=========================================================
	
//4.4 ********************Creating wifi and Ap**********************
	NodeContainer wifiStaNodes; 
	NodeContainer wifiApNode;
    	wifiStaNodes.Create (nWifi);
	wifiApNode.Create(AP);
//=========================================================
//4.8 ********************************Mobility*****************************
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAllocstation = CreateObject<ListPositionAllocator>();		 
	double posx, posy, posz = 1.5;
	for (int m = 0; m<nWifi; m++){
		posy = (r*sin(theta*m*3.14159/180));
		posx = (r*cos(theta*m*3.14159/180));
		positionAllocstation->Add (Vector( posx, posy, posz));
		//std::cout<< "posx"<< posx<< "posy"<< posy << "posz"<< posz << std::endl;
		mobility.SetPositionAllocator (positionAllocstation);
      	}
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (wifiStaNodes);

  	Ptr<ListPositionAllocator> positionAllocap = CreateObject<ListPositionAllocator> ();
	positionAllocap->Add (Vector (0.0, 0.0, 3.0));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");	  
	mobility.SetPositionAllocator (positionAllocap);
	mobility.Install (wifiApNode);
	CourseChange (wifiStaNodes,"wifiStaNodeslocation");
	CourseChange (wifiApNode,"wifiApNodelocation");
//4.5 ********************creating WiFi PHY and channel**********************
	
	WifiHelper wifi;
	wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
	YansWifiChannelHelper channel;
	channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel");
	
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
	//Config::SetDefault ("ns3::WifiPhy::EnergyDetectionThreshold", DoubleValue (-66));

	phy.SetChannel (channel.Create ());
	phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
	//channel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));
	channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel", "Distance0", DoubleValue (5));
	channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel","Distance1", DoubleValue (60));
	channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel","Distance2", DoubleValue (250));
	channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel","Exponent0", DoubleValue (2.7));
	channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel","Exponent1", DoubleValue (2));
	channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel","Exponent2", DoubleValue (4));
	channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel","ReferenceLoss", DoubleValue (46.6777));
//4.6 ********************Creating WiFi Standard**********************
    	Config::SetDefault ("ns3::WifiPhy::ChannelNumber", UintegerValue (3));
	//wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs5"),"ControlMode", StringValue ("HtMcs0"));ns3::MinstrelHtWifiManager
	wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");
//=========================================================
	Config::Set ("/Nodelist/*/Devicelist/*/$ns3::WifiNetDevice/phy/ChannelWidth",UintegerValue(20));
	Config::Set ("/Nodelist/*/Devicelist/*/$ns3::WifiNetDevice/HtConfiguration/ShortGuardInterval",BooleanValue(1));
//=========================================================
//4.7 **************************Set MAC Parameters************************
	WifiMacHelper mac; 
	Ssid ssid = Ssid ("ns3-80211n"); 
	mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false),"QosSupported", BooleanValue(true),"VI_MaxAmpduSize",UintegerValue(0),"BE_MaxAmpduSize",UintegerValue(0),"BK_MaxAmpduSize",UintegerValue(0),"VO_MaxAmpduSize",UintegerValue(0));
	NetDeviceContainer staDevices; 
	staDevices = wifi.Install (phy, mac, wifiStaNodes); 
	mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
	NetDeviceContainer apDevices;
	apDevices = wifi.Install (phy, mac, wifiApNode);
	
//=========================================================================  

//4.9**************************Energy Model*************************
	
	/*BasicEnergySourceHelper basicSourceHelper;
	basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (30));
	basicSourceHelper.Set ("PeriodicEnergyUpdateInterval", TimeValue (Seconds (1)));
	basicSourceHelper.Set ("BasicEnergySupplyVoltageV", DoubleValue (3.3));
	EnergySourceContainer energysources = basicSourceHelper.Install (wifiStaNodes);*/
	RvBatteryModelHelper rvModelHelper;
	rvModelHelper.Set ("RvBatteryModelAlphaValue", DoubleValue (alpha));
  	rvModelHelper.Set ("RvBatteryModelBetaValue", DoubleValue (beta));
  	rvModelHelper.Set ("RvBatteryModelLowBatteryThreshold", DoubleValue (0.1));
	//rvModelHelper.Set ("RvBatteryModelOpenCircuitVoltage", DoubleValue (3.3));
	EnergySourceContainer energysources = rvModelHelper.Install (wifiStaNodes);

	//Simulator::Schedule (Seconds (5.0), &EnergyUpdateInterval, wifiStaNodes.Get(0), 50);
	/*Callback <void, Ptr<Node> , double> energyUpdate;
	energyUpdate = MakeCallback (&EnergyUpdateInterval);
	Simulator::Schedule (Seconds (5.0), energyUpdate, wifiStaNodes.Get(0), 1);*/
	WifiRadioEnergyModelHelper radioEnergyHelper;
	radioEnergyHelper.Set ("IdleCurrentA", DoubleValue (0.273));
	radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.466));
	radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.3));
	//radioEnergyHelper.Set ("CcaBusyCurrentA", DoubleValue (0.3));
	//radioEnergyHelper.Set ("SwitchingCurrentA", DoubleValue (0.3));
	radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.02));
	DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (staDevices, energysources); 

	//Simulator::Schedule (Seconds (5.0), &SetInitialEnergy, wifiStaNodes.Get(0), (double)60.0);
	
//=========================================================
//4.10 *************************Energy Harvester Model (Basic Energy Harvester)****************
	/*BasicEnergyHarvesterHelper basicHarvesterHelper;
	basicHarvesterHelper.Set ("PeriodicHarvestedPowerUpdateInterval", TimeValue (Seconds (1)));
	basicHarvesterHelper.Set ("HarvestablePower", StringValue ("ns3::UniformRandomVariable[Min=2|Max=3]")); //[Min=0.010204|Max=0.010204]")); 
  	EnergyHarvesterContainer harvesters = basicHarvesterHelper.Install (energysources);*/
//=========================================================
//4.10 *************************Energy Harvester Model (Solar Harvester)****************
  	SolarEnergyHarvesterHelper solarHarvesterHelper;
  	solarHarvesterHelper.Set ("PeriodicHarvestedPowerUpdateInterval", TimeValue (Seconds (harvestingUpdateInterval)));		
 	EnergyHarvesterContainer harvesters= solarHarvesterHelper.Install (energysources);
//========================================================== 
	//Ptr<WifiNetDevice> wifiDevice0 = DynamicCast<WifiNetDevice> (staDevices.Get(0));
	//Ptr<WifiPhy> wifiPhy0 = wifiDevice0->GetPhy();
	/*Simulator::Schedule(Seconds(5), &WifiPhy::SetSleepMode, wifiPhy0);
	Simulator::Schedule(Seconds(10), &WifiPhy::ResumeFromSleep, wifiPhy0);
	Simulator::Schedule(Seconds(15), &WifiPhy::SetSleepMode, wifiPhy0);
	Simulator::Schedule(Seconds(20), &WifiPhy::ResumeFromSleep, wifiPhy0);*/
	//Simulator::Schedule(Seconds(20), &WifiPhy::SetSleepMode, wifiPhy0);
	//Simulator::Schedule(Seconds(18), &WifiPhy::ResumeFromSleep, wifiPhy0);
//4.11 *************************Internet Stack**************** 
	InternetStackHelper stack;
	stack.Install (wifiApNode);
	stack.Install (wifiStaNodes);
//=========================================================	
//4.12 ***************Assign IP addresses to each node***************
  	Ipv4AddressHelper address;
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	address.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer staNodeInterface; 
	staNodeInterface = address.Assign (staDevices);
	Ipv4InterfaceContainer apNodeInterface;
	apNodeInterface = address.Assign (apDevices);
//=========================================================
//4.13 ***************Setting application (on/off)***************
	int port = 9;
	int addtime = 0.1;
	ApplicationContainer apps;
	//Address sinkAddress (InetSocketAddress (apNodeInterface.GetAddress (0), ));
	InetSocketAddress socketaddress (apNodeInterface.GetAddress (0), port);
	PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
	apps = sinkHelper.Install (wifiApNode.Get(0));
	apps.Start (Seconds (0.1)); 
  	apps.Stop (Seconds (simulationTime+5e-6));


	Ptr<NetDevice> dev_VO = wifiStaNodes.Get (0)->GetDevice (0);
	Ptr<WifiNetDevice> wifi_dev_VO = DynamicCast<WifiNetDevice> (dev_VO);
	PointerValue ptr_VO;
	Ptr<QosTxop> edca_VO;
	wifi_dev_VO->GetMac ()->GetAttribute ("VO_Txop", ptr_VO);
	edca_VO = ptr_VO.Get<QosTxop> ();
	edca_VO->SetTxopLimit (MicroSeconds (Txoplimit_VO));
	edca_VO->SetAifsn (Aifsn_VO);
	edca_VO->SetMaxCw (Cwmax_VO);
	edca_VO->SetMinCw (Cwmin_VO);
	
	Ptr<NetDevice> dev_VI = wifiStaNodes.Get (0)->GetDevice (0);
	Ptr<WifiNetDevice> wifi_dev_VI = DynamicCast<WifiNetDevice> (dev_VI);
	PointerValue ptr_VI;
	Ptr<QosTxop> edca_VI;
	wifi_dev_VI->GetMac ()->GetAttribute ("VI_Txop", ptr_VI);
	edca_VI = ptr_VI.Get<QosTxop> ();
	edca_VI->SetTxopLimit (MicroSeconds (Txoplimit_VI));
	edca_VI->SetAifsn (Aifsn_VI);
	edca_VI->SetMaxCw (Cwmax_VI);
	edca_VI->SetMinCw (Cwmin_VI);
	
	Ptr<NetDevice> dev_BE = wifiStaNodes.Get (0)->GetDevice (0);
	Ptr<WifiNetDevice> wifi_dev_BE = DynamicCast<WifiNetDevice> (dev_BE);
	PointerValue ptr_BE;
	Ptr<QosTxop> edca_BE;
	wifi_dev_BE->GetMac ()->GetAttribute ("BE_Txop", ptr_BE);
	edca_BE = ptr_BE.Get<QosTxop> ();
	edca_BE->SetTxopLimit (MicroSeconds (Txoplimit_BE));
	edca_BE->SetAifsn (Aifsn_BE);
	edca_BE->SetMaxCw (Cwmax_BE);
	edca_BE->SetMinCw (Cwmin_BE);


		 

	if (Video){
		socketaddress.SetTos (0xc0); // user priority for AC_VO
		OnOffHelper onOffVideo("ns3::UdpSocketFactory", socketaddress);
	  	onOffVideo.SetAttribute ("DataRate", DataRateValue (DataRate (VideoDataRate)));
	  	onOffVideo.SetAttribute ("PacketSize", UintegerValue (VideoPacketSize));
		onOffVideo.SetAttribute ("OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=0.352]")); 
	  	onOffVideo.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=0.65]"));
		apps = onOffVideo.Install (wifiStaNodes.Get (0));
	  	apps.Start (Seconds (1+addtime));
	  	apps.Stop (Seconds (simulationTime+2e-6));
		

		 CoapClientHelper client (apNodeInterface.GetAddress (0), port);
		 client.SetAttribute ("Interval", TimeValue (0.14));
		 client.SetAttribute ("PacketSize", UintegerValue (ECGPacketSize));
		 apps = client.Install (wifiStaNodes.Get (0));

	}



	if (ECG){
		socketaddress.SetTos (0xb8); // user priority for AC_VI
		OnOffHelper onOffECG ("ns3::UdpSocketFactory", socketaddress);
		onOffECG.SetConstantRate(DataRate (ECGDataRate), ECGPacketSize);
	  	onOffECG.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.065]")); 
	  	onOffECG.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.935]"));
		apps = onOffECG.Install (wifiStaNodes.Get (0));
		apps.Start (Seconds (1.05+ addtime));
	  	apps.Stop (Seconds (simulationTime+3e-6));
	}

	if (EEG){
		socketaddress.SetTos (0xb8); // set the type of service by defining user priority for AC_VI
		OnOffHelper onOffEEG ("ns3::UdpSocketFactory", socketaddress);
	  	onOffEEG.SetConstantRate(DataRate (EEGDataRate), EEGPacketSize);
	  	onOffEEG.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.140]")); 
	  	onOffEEG.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.860]"));
		apps = onOffEEG.Install (wifiStaNodes.Get (0));
		apps.Start (Seconds (2+addtime));
	  	apps.Stop (Seconds (simulationTime+4e-6));
	}
	
	if (EMR){
		socketaddress.SetTos (0x28); // user priority for AC_BE
		OnOffHelper onOffEMR ("ns3::UdpSocketFactory", socketaddress);
	  	onOffEMR.SetAttribute ("DataRate", DataRateValue (DataRate (EMRDataRate)));
	  	onOffEMR.SetAttribute ("PacketSize", UintegerValue (EMRPacketSize));
	  	onOffEMR.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]")); 
	  	onOffEMR.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
		apps = onOffEMR.Install (wifiStaNodes.Get (0));
		apps.Start (Seconds (2.05+addtime));
	  	apps.Stop (Seconds (simulationTime+5e-6));
	}

	if (TelemetryAlarm){
		socketaddress.SetTos (0xc0); // user priority for AC_VO
		OnOffHelper onOffTelemetryAlarm ("ns3::UdpSocketFactory", socketaddress);
		onOffTelemetryAlarm.SetAttribute ("DataRate", DataRateValue (DataRate (TelemetryAlarmDataRate)));
	  	onOffTelemetryAlarm.SetAttribute ("PacketSize", UintegerValue (TelemetryAlarmPacketSize));
	  	onOffTelemetryAlarm.SetAttribute ("OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=0.04877]")); 
	  	onOffTelemetryAlarm.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=0.95123]"));
		apps = onOffTelemetryAlarm.Install (wifiStaNodes.Get (0));
		apps.Start (Seconds (3+addtime));
	  	apps.Stop (Seconds (simulationTime+6e-6));
	}
	//std::cout<< " exponetial random number " << ExRandom <<"\n";*/
	/*UdpServerHelper server (9);
	ApplicationContainer serverApp = server.Install (wifiApNode.Get (0));
	serverApp.Start (Seconds (0.0));
	serverApp.Stop (Seconds (simulationTime + 1e-6));
	 
	UdpClientHelper client (apNodeInterface.GetAddress (0), 9);
	client.SetAttribute ("MaxPackets", UintegerValue (71000));
	client.SetAttribute ("Interval", TimeValue (Seconds (1.0))); //packets/s
	client.SetAttribute ("PacketSize", UintegerValue (packetSize)); //bytes
	 
	ApplicationContainer clientApp = client.Install (wifiStaNodes.Get (0));
	clientApp.Start (Seconds (1.0));
	clientApp.Stop (Seconds (simulationTime));*/
//=========================================================
//4.15 ********** Connect Trace energysources **************************
	
	//Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (energysources.Get(0));
 	//Ptr<DeviceEnergyModel> basicRadioModelPtr = basicSourcePtr->FindDeviceEnergyModels ("ns3::WifiRadioEnergyModel").Get (0);
  	//NS_ASSERT (basicRadioModelPtr != NULL);
 	//Ptr<BasicEnergyHarvester> basicHarvesterPtr = DynamicCast<BasicEnergyHarvester> (harvesters.Get (0));

	//basicSourcePtr->TraceConnect ("RemainingEnergy",PrintID(energysources.Get(0)->GetNode()), MakeCallback (&RemainingEnergy));
  	//basicRadioModelPtr->TraceConnect ("TotalEnergyConsumption",PrintID(energysources.Get(0)->GetNode()), MakeCallback (&TotalEnergy));
	//basicSourcePtr->TraceConnect ("TotalEnergyHarvested",PrintID(harvesters.Get(0)->GetNode()), MakeCallback (&TotalEnergyHarvested));
	//Ptr<QosTxop> QosTxopPtr = DynamicCast<QosTxop> (wifiStaNodes.Get(0));
	//wifiStaNodes.Get(0)->TraceConnectWithoutContext ("ContentionWindow",MakeCallback (&ContentionWindow<0>));
	//energysources.Get (0)->TraceConnectWithoutContext ("RemainingEnergy",MakeCallback (&RemainingEnergy<0>));
	//harvesters.Get (0)->TraceConnectWithoutContext ("TotalEnergyHarvested",MakeCallback (&TotalEnergyHarvested<0>));
	Ptr<RvBatteryModel> rvbatteryPtr = DynamicCast<RvBatteryModel> (energysources.Get(0));
	energysources.Get (0)->TraceConnectWithoutContext ("RvBatteryModelBatteryLevel",MakeCallback (&RvBatteryModelBatteryLevel<0>));
//=========================================================

//4.17 ***************FlowMonitor***************    
 	FlowMonitorHelper flowmon; 
	Ptr<FlowMonitor> monitor = flowmon.InstallAll(); 
	Simulator::Schedule(Seconds(1),&ThroughputMonitor,&flowmon, monitor);
	monitor-> Start (Seconds(flowmonstart));
	monitor-> Stop (Seconds(flowmonend));
//========================================================= 
	phy.EnablePcap ("AccessPoint", apDevices);
	phy.EnablePcap ("Station", staDevices);
//4.18 ****************Simulation Stop and Run*************
	Simulator::Stop (Seconds (simulationTime+4e-6));
	AnimationInterface anim ("propagation.xml");

	//Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/Txop/$ns3::QosTxop/Cw", MakeCallback(&ContentionWindow<0>));
	Config::Connect ("/NodeList/0/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<0>));//station
	Config::Connect ("/NodeList/1/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<1>));//Ap
	// Config::Connect ("/NodeList/0/DeviceList/*/Phy/State/RxOk", MakeCallback (&PhyRxOkTrace<0>));
	//Config::Connect ("/NodeList/1/DeviceList/*/Phy/State/RxOk", MakeCallback (&PhyRxOkTrace<1>));
	Simulator::Run ();
//=========================================================
	/*Time actualLifetime;
	Ptr<RvBatteryModel> srcPtr = DynamicCast<RvBatteryModel> (energysources.Get (0));
	actualLifetime = srcPtr->GetLifetime ();

	Double batterylevel;
	Ptr<RvBatteryModel> srcPtrr = DynamicCast<RvBatteryModel> (energysources.Get (0));
	batterylevel = srcPtrr->GetBatteryLevel ();


  	std::cout<< " At Time = " <<Simulator::Now ().GetSeconds () << " the actual lifetime = " << actualLifetime.GetSeconds () << "s"<<"\n";*/
	//std::cout<< " At Time = " <<Simulator::Now ().GetSeconds () << " the actual lifetime = " << batterylevel.Get (0) << "s"<<"\n";
  
ThroughputMonitor(&flowmon ,monitor);
//4.19 ***************Print Per flow statistics***************
  	std::cout<< "--------Flow Monitor Statistics--------"<< std::endl;
	monitor->CheckForLostPackets ();
  	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
	double OverallThroughput = 0;
  	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter){
	  	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
	    	std::cout<< "Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress<<"\n";
	    	std::cout<<"Lost Packets =  " << iter->second.lostPackets <<"\n";
		std::cout<<"Tx Packets = " << iter->second.txPackets <<"\n";
	    	std::cout<<"Rx Packets = " << iter->second.rxPackets <<"\n";
		std::cout<<"Tx Bytes = " << iter->second.txBytes <<"\n";
		std::cout<<"Rx Bytes =  " << iter->second.rxBytes <<"\n";
		std::cout<< "Mean Delay =  " <<((iter->second.delaySum)/float(iter->second.rxPackets))<<"\n";
		std::cout<<"Mean Jitter =  " << ((iter->second.jitterSum) /float(iter->second.rxPackets-1))<<"\n";
		std::cout<<"Packet Loss Ratio (PLR) =  " << (float (iter->second.lostPackets*100) / (float(iter->second.txPackets)))<<"%"<<"\n";
		std::cout<<"Packet Delivery Ratio (PDR) =  " << (float (iter->second.rxPackets *100) / (float(iter->second.txPackets)))<< "%" <<"\n";
    	  	if(iter->second.rxBytes > 0){
	 			std::cout<<"Throughput = " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1000000  << " Mbps"<<"\n"; 
				std::stringstream pp;
  				pp << "flowmonitor" << ".csv";
  				static std::fstream f (pp.str ().c_str (), std::ios::out);
 				f << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress<<","<< iter->second.lostPackets <<","<< iter->second.txPackets << ","<< iter->second.rxPackets <<"," << iter->second.txBytes <<"," << iter->second.rxBytes <<","<< iter->second.delaySum / iter->second.rxPackets << "," <<iter->second.jitterSum / (iter->second.rxPackets-1)<<","<< iter->second.lostPackets / iter->second.txPackets<<","<< (float (iter->second.lostPackets*100) / (float(iter->second.txPackets))) << "," << (float (iter->second.rxPackets *100) / (float(iter->second.txPackets)))<<","<< iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1000000  <<std::endl;
			} 
 		OverallThroughput+=(iter->second.rxBytes*8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) /1000000 );
	}
	std::cout << "===========================\n" << std::flush;
	std::cout << "Overall Throughput= " << OverallThroughput << "Mbps" <<"\n" << std::flush;
//=========================================================
//4.20 ****************Print the result for the total energy consumption by the radio in each node***********
	for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin (); iter != deviceModels.End (); iter ++){
    		double energyConsumed = (*iter)->GetTotalEnergyConsumption ();
    		std::cout<<"End of simulation (" << Simulator::Now ().GetSeconds () << "s) Total energy consumed by radio = " << energyConsumed << "J"<<"\n";
    		NS_ASSERT (energyConsumed <= 1000);
	}
//=========================================================
 //4.21 ****************Simulation destory**************
		Simulator::Destroy ();
		return 0;
	}


