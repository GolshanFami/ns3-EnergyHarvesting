//1. ***************Define headers***************
// include statments to help users to deal with include files in ns3 and header files
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
#include "ns3/csma-module.h"
//=========================================================



//2. ****************Default Network Topology**********

// STA  STA STA STA STA  AP      controller
//  *   *   *   *   *    *    csma   *
//  |   |   |   |   |    |===========|      
// n4   n3  n2  n1  n0  n5          n6
//=========================================================



using namespace ns3;// namespace declaration

NS_LOG_COMPONENT_DEFINE ("5nodesWifi-csmaEnergyHarvestingwithcontroler");

//3. ***************Functions for Tracing***************



//3.1 ***************Trace function for remaining energy at node***************
template <int node>
void RemainingEnergy (double oldValue, double remainingEnergy)
{

	std::stringstream ss;
  ss << "RemainingEnergy_" << node << ".log";

  static std::fstream f (ss.str ().c_str (), std::ios::out);

  f << Simulator::Now ().GetSeconds () <<   "    "   <<remainingEnergy  << std::endl;

}

//===============================================================================


//3.2 ***************Trace function for total energy consumption at node***************
template <int node>
void TotalEnergy (double oldValue, double TotalEnergy)
{
  std::stringstream ss;
  ss << "TotalEnergy_" << node << ".log";

  static std::fstream f (ss.str ().c_str (), std::ios::out);

  f << Simulator::Now ().GetSeconds ()<<   "    "   << TotalEnergy << std::endl;
}

//=======================================================================

//3.3 ***************Trace function for the total energy harvested by the node***************

template <int node>
void TotalEnergyHarvested (double oldValue, double TotalEnergyHarvested)
{
  std::stringstream ss;
  ss << "TotalEnergyHarvested_" << node << ".log";

  static std::fstream f (ss.str ().c_str (), std::ios::out);

  f << Simulator::Now ().GetSeconds () <<   "    "  << TotalEnergyHarvested << std::endl;
}

//=======================================================================


//Step 4. ***************The Main Program***************

int
main (int argc, char *argv[])  // argc = argumnet count which contains the number of arguments passed to the program. argv = argument vector which is a one-dimensional array of strings.
{

//4.1 ***************Setting Variables********************************


//4.1.1 ***************Setting Variables physical layer***************

	
	std::string dataRate = "15Mbps"; // The Maximum speed at which data is transferred across a wireless link between nodes.
  	uint32_t packetSize = 1500; // bytes-- The size of a data unit

//4.1.2 ***************Setting Variables******************************
	
        double simulationTime =30; //seconds-- Total time in second for simulation to be run

//4.1.3 ***************Setting Variables for Mobility******************

        double distance = 5.0; //meters-- Distance in meters between the station and the access point

//4.1.4 ***************Setting Variables for the Standard IEEE 802.11n ***************

        double frequency = 2.4; //whether 2.4 or 5.0 GHz- For IEEE 802.11n standard there are two different frequencies, 2.4 enable devices to communicated for longer distances and it is less power greedy. 
        

//4.1.5***************Setting Variables for the topology of the Network***************
       	uint32_t nWifi =5; // number of the wifi stations in the network 
        
//4.1.6 ***************Setting Variables for Energy Model***************
	double idleCurrent = 0.273;// The default radio Idle current in Ampere

//4.1.7 ***************Setting Variables for Energy Harvesting Model***************
 	double harvestingUpdateInterval = 1;  // seconds-- Is the interval that every harvestingUpdateInterval update the amount of power that will be provided by the harvester in the next interval


//4.1.8 ***************Setting Variables for logging and tracing***************
        bool verbose = true; // logging enbale--verbose flag is used to determine whether or not the onoff application logging components are enabled (logging components are fo printing)
	bool tracing = true;

//4.1.9 ***************Setting Variables for generating Random number***************

/**The random variable is generated between [0, 0.1)*/

	double min = 0; // The Minimum value for generating the random number
	double max = 0.1;// The Maximum value for generating the random number

//====================================================================

//4.2 ***************Command line argument parser setup***************

/** It enables script changes through command line arguments without editing and building. It is able to set the local and global variables automatically.*/

	CommandLine cmd;
	cmd.AddValue ("dataRate", "Data rate", dataRate);
        cmd.AddValue ("frequency", "Whether working in the 2.4 or 5.0 GHz band (other values gets rejected)", frequency);
	cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
        cmd.AddValue ("distance", "Distance in meters between the station and the access point", distance);
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



//4.3 **********************Print Result**************************

/**logging facility can be used to monitor or debug the progress of simulation program*/

      if (verbose)
        {
     	LogComponentEnable("OnOffApplication",LOG_LEVEL_INFO) ;
	LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
      }  

//=========================================================
//4.3 **********************Creating CSMA******************************


//4.3.1 **********************Creating nodes for Csma******************

	NodeContainer CsmaNodes;
	CsmaNodes.Create (2); // we define one csma as the controller and then ap merge on the second one

//4.3.1 **********************Creating the Channel for Csma************

	 CsmaHelper  csma;
	 csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps")); // The transmission data rate to be provided to devices connected to the channel
	 csma.SetChannelAttribute ("Delay", StringValue ("10e-6")); //Transmission delay through the channel

//4.3.2 **********************Install csmaDevices***********************

        NetDeviceContainer csmaDevices; //set a container for nodes that are connected through a csma link which holds the nodes as objects
        csmaDevices = csma.Install (CsmaNodes); // create csma nodes

//======================================================================


//4.4 ********************Creating wifi and Ap**********************
       /** Node obejects are created as wifi stations and an access point*/
	
	NodeContainer wifiStaNodes; // set a container for stations which holds the stations as objects
        wifiStaNodes.Create (nWifi); // create wifi stations
	NodeContainer wifiApNode = CsmaNodes.Get(1); // define access point as the second node with csma
    
//=========================================================

//4.5 ********************creating WiFi PHY and channel**********************

/** The intent of yans-wifi-helper class is to make it easy to create a channel object
 * which implements the yans channel model. The yans channel model is described
 * in "Yet Another Network Simulator"*/

/** default PHY layer configuration and channel models which are defined in YansWifiphyhelper and YansWifichannelhelper */
        YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
        YansWifiPhyHelper phy = YansWifiPhyHelper::Default();

	phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
        phy.SetChannel (channel.Create ()); // Create the channel object and assiciate it with the physical layer object to make sure that all the PHY layer objects created by the YansWifiPhyHelper share the same underlying channel
        WifiHelper wifi; // This is the new hepler which is necessary for implementing PHY implementation

//=========================================================

//4.6 ********************Creating WiFi Standard**********************
 
// we define an if statment to choose the wifi frequency between 5 or 2.4 GHz 

if (frequency == 5.0)
                {
                  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
                }
              else if (frequency == 2.4)
                {
                  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
                  Config::SetDefault ("ns3::WifiPhy::ChannelNumber", UintegerValue (3)); // It is possible to set the channel number from 1 to 14
                }
              else
                {
                  std::cout << "Wrong frequency value!" << std::endl; // print statment
                  return 0;
                }

		wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs0"),"ControlMode", StringValue ("HtMcs5")); //method tells the helper the type of rate control algorithm and data rate to use**
//=========================================================

//4.7 **************************Set MAC Parameters************************

        WifiMacHelper mac; //WifiMacHelper object is used to set MAC parameters.
	Ssid ssid = Ssid ("ns3-80211n"); // It creates an 802.11 service set identifier (SSID) object that will be used to set the value of the “Ssid” Attribute of the MAC layer implementation (the SSID of the infrastructure network we want to setup)

        mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false),
				   "QosSupported", BooleanValue(true),"VI_MaxAmpduSize",UintegerValue(0),"BE_MaxAmpduSize",UintegerValue(0),"BK_MaxAmpduSize",UintegerValue(0),"VO_MaxAmpduSize",UintegerValue(0)); //configure the type of MAC (non-AP station (STA) in an infrastructure)
        NetDeviceContainer staDevices; // It holds the list of all netdevice objects (wifi stations)
        staDevices = wifi.Install (phy, mac, wifiStaNodes); // Install method of wifiMachelper takes the NodeContainer (wifiStaNodes), phy and mac as parameters

       	mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid)); // configure the type of MAC (Access Point in an infrastructure)


        NetDeviceContainer apDevices;// It holds the list of all netdevice objects (Access point)
        apDevices = wifi.Install (phy, mac, wifiApNode); // Install method of wifiMachelper takes the NodeContainer (wifiApNode), phy and mac as parameters
  
//=========================================================================
        
//4.8 ********************************Mobility*****************************
          MobilityHelper mobility; // Mobility helper uses a two-dimensional grid to initially place the STA nodes
          Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");// we want Ap to be stationary
          
	  //Set position for AP
	  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	  
	  //Set position for STAs
          positionAlloc->Add (Vector (distance, 0.0, 0.0));
	  positionAlloc->Add (Vector (5 + distance, 0.0, 0.0));
  	  positionAlloc->Add (Vector (10 + distance, 0.0, 0.0));
  	  positionAlloc->Add (Vector (15 + distance, 0.0, 0.0));
	  positionAlloc->Add (Vector (20 + distance, 0.0, 0.0));

          mobility.SetPositionAllocator (positionAlloc);

	  mobility.Install (wifiApNode);
	  mobility.Install (wifiStaNodes);
          
         
//=========================================================

//4.9**************************Energy Model*************************

/**BasicEnergySource decreases/increases remaining energy stored in itself in linearly*/


/**A device energy model should represent the energy consumption behavior of a specific device. It will update remaining energy stored in the EnergySource object installed on node. When energy is depleted, each DeviceEnergyModel object installed on the same node will be informed by the EnergySource.*/

  BasicEnergySourceHelper basicSourceHelper; // energy source
  
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (100)); // configure energy source
 
  EnergySourceContainer energysources = basicSourceHelper.Install (wifiStaNodes);  // install energysources on all the wifi stations
  
  WifiRadioEnergyModelHelper radioEnergyHelper; // device energy model
  
  radioEnergyHelper.Set ("IdleCurrentA", DoubleValue (idleCurrent)); // The default radio Idle current in Ampere. configure radio energy model (Amper)
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.466));// The radio Tx current in Ampere. default value 0.0174
  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.3));//The radio Rx current in Ampere
  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.02));//The radio Sleep current in Ampere



  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (staDevices, energysources);  // install devicemodels on all the station and energysources

//=========================================================

//4.10 *************************Energy Harvester Model****************

 /** BasicEnergyHarvester increases remaining energy stored in an associated Energy Source. The BasicEnergyHarvester implements a simple model in which the amount of power provided by the harvester varies over time according to a customizable generic random variable and time update intervals.*/
   
  BasicEnergyHarvesterHelper basicHarvesterHelper; //energy harvester
 
  basicHarvesterHelper.Set ("PeriodicHarvestedPowerUpdateInterval", TimeValue (Seconds (harvestingUpdateInterval))); // configure energy harvester (time update interval)

  basicHarvesterHelper.Set ("HarvestablePower", StringValue ("ns3::UniformRandomVariable[Min=0.1|Max=0.1]")); // configure energy harvester (random variable)


 
  EnergyHarvesterContainer harvesters = basicHarvesterHelper.Install (energysources); // install harvester on all energy sources

//=========================================================

//4.11 *************************Internet Stack**************** 

/** it is helper for internt stack (protocol stack ) and install Internet Stack (TCP, UDP, IP, etc.) on the net devices (wifi stations and access point)*/
     
        InternetStackHelper stack;
	stack.Install (CsmaNodes.Get(0));
        stack.Install (wifiApNode);
        stack.Install (wifiStaNodes);
	
//=========================================================	


//4.12 ***************Assign IP addresses to each node***************


/** associate devices with IP address . Here the helper manages the allocation of IP addresses*/
  	Ipv4AddressHelper address;


	address.SetBase ("10.1.1.0", "255.255.255.0"); 
	Ipv4InterfaceContainer csmaInterfaces;//Ipv4InterfaceContainer holds the list of interfaces which need to be used for assigning IP address to the devices (controler node)
        csmaInterfaces = address.Assign (csmaDevices);

        address.SetBase ("10.1.2.0", "255.255.255.0"); // set the base IP address and network Mask (which is done in side the helper). A subnet mask is a number that defines a range of IP addresses available within a network. A single subnet mask limits the number of valid IPs for a specific network
        Ipv4InterfaceContainer staNodeInterface; //Ipv4InterfaceContainer holds the list of interfaces which need to be used for assigning IP address to the devices (stations)
	staNodeInterface = address.Assign (staDevices);// Actual IP address asignment with the help of interface of stations (association between IP address and the device is done by the interface)

        Ipv4InterfaceContainer apNodeInterface;//Ipv4InterfaceContainer holds the list of interfaces which need to be used for assigning IP address to the devices (Access point)
	apNodeInterface = address.Assign (apDevices); // Actual IP address asignment with the help of interface of access pint (association between IP address and the device is done by the interface)

//=========================================================


//4.13 ***************Setting application (on/off)***************


/*PacketSinkHelper is a helper to make it easier to instantiate an ns3::PacketSinkApplication on a set of nodes*/

// Create a packet sink to receive packets

uint16_t port = 9;// Define the port

// Install UDP Receiver on the Controller
	ApplicationContainer apps;
	Address sinkAddress (InetSocketAddress (csmaInterfaces.GetAddress (0), port));// interface of controller with csma
	
	PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port)); //the name of the protocol to use to receive traffic. This string identifies the socket factory type used to create sockets for the applications.
	
	apps = sinkHelper.Install (CsmaNodes.Get (0));// Install sink on controller to recieved packets from all wifi nodes	

	//Address sinkAddress (InetSocketAddress (staNodeInterface.GetAddress (i), port));// for downlink
	apps.Start (Seconds (0.1)); // start time for sink
  	apps.Stop (Seconds (simulationTime+3e-6)); //stop time for sink	

// create the OnOff application to send UDP dtagrams of size 1024 bytes at rate 15Mbps clientapps starts with a different time by generating a random variable at each iteration for each link. Insatll UDP transmitter on the stations
for (uint32_t i = 0; i<nWifi; i++)
	{
	OnOffHelper onOffA ("ns3::UdpSocketFactory", Address (InetSocketAddress (csmaInterfaces.GetAddress (0), port)));// It is a helper to make it easier to instantiate an ns3::OnOffApplication on a set of nodes. onoffHelper function used to set the underlying application attributes. Traffic flow from one node to the other node (for all the nodes) 

  	onOffA.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate))); //set the data rate
  	onOffA.SetAttribute ("PacketSize", UintegerValue (packetSize)); // set the packet size
  	onOffA.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]")); //set on value time to a constanct value 1.0 second. A RandomVariableStream used to pick the duration of the 'On' state. Schedules the event to start sending data.
  	onOffA.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));//set off value time to a constanct value 0.0 second. A RandomVariableStream used to pick the duration of the 'Off' state. Schedules the event to stop sending data


	apps = onOffA.Install (wifiStaNodes.Get (i)); // traffic sources are installed in all nodes


//4.14 **********************Define Random Variable**************************
/*contains a built-in pseudo-random number generator (PRNG)(UniformRandomVariable,ExponentialRandomVariable,...). Default seed is set to 1*/

Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> (); //random variable objects from the base class RandomVariableStream. objects derive from ns3::Object and are handled by smart pointers. for creating these objects is to use the templated CreateObject<> method.
x->SetAttribute ("Min",DoubleValue(min)); //pointer to the minimum value
x->SetAttribute ("Max",DoubleValue(max)); //pointer to the maximum value
double myRandomNo = x->GetValue(); // Access values by calling methods on the object 
//std::cout<< "randomNomber"<< myRandomNo<< std::endl; // print the random number value

//========================================================

  	apps.Start (Seconds (myRandomNo));  // start time for stations
  	apps.Stop (Seconds (simulationTime+2e-6)); //stop time for stations (senders)	

//=========================================================

/*

OnOffHelper onOffB ("ns3::UdpSocketFactory", Address (InetSocketAddress (csmaInterfaces.GetAddress (0), port)));// It is a helper to make it easier to instantiate an ns3::OnOffApplication on a set of nodes. onoffHelper function used to set the underlying application attributes. Traffic flow from one node to the other node (for all the nodes) 

  	onOffB.SetAttribute ("DataRate", DataRateValue (DataRate (40000))); //set the data rate skype
  	onOffB.SetAttribute ("PacketSize", UintegerValue (packetSize)); // set the packet size
  	onOffB.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]")); //set on value time to a constanct value 1.0 second. A RandomVariableStream used to pick the duration of the 'On' state. Schedules the event to start sending data.
  	onOffB.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));//set off value time to a constanct value 0.0 second. A RandomVariableStream used to pick the duration of the 'Off' state. Schedules the event to stop sending data


	apps = onOffB.Install (wifiStaNodes.Get (1)); // traffic sources are installed in all nodes


	apps.Start (Seconds (myRandomNo));  // start time for stations
  	apps.Stop (Seconds (simulationTime+2e-6)); //stop time for stations (senders)	

//=========================================================


OnOffHelper onOffC ("ns3::UdpSocketFactory", Address (InetSocketAddress (csmaInterfaces.GetAddress (0), port)));// It is a helper to make it easier to instantiate an ns3::OnOffApplication on a set of nodes. onoffHelper function used to set the underlying application attributes. Traffic flow from one node to the other node (for all the nodes) 

  	onOffC.SetAttribute ("DataRate", DataRateValue (DataRate (8000000))); //set the data rate (Advanced Video Coding)
  	onOffC.SetAttribute ("PacketSize", UintegerValue (packetSize)); // set the packet size
  	onOffC.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]")); //set on value time to a constanct value 1.0 second. A RandomVariableStream used to pick the duration of the 'On' state. Schedules the event to start sending data.
  	onOffC.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));//set off value time to a constanct value 0.0 second. A RandomVariableStream used to pick the duration of the 'Off' state. Schedules the event to stop sending data


	apps = onOffC.Install (wifiStaNodes.Get (2)); // traffic sources are installed in all nodes


	apps.Start (Seconds (myRandomNo));  // start time for stations
  	apps.Stop (Seconds (simulationTime+2e-6)); //stop time for stations (senders)
//=========================================================


OnOffHelper onOffD ("ns3::UdpSocketFactory", Address (InetSocketAddress (csmaInterfaces.GetAddress (0), port)));// It is a helper to make it easier to instantiate an ns3::OnOffApplication on a set of nodes. onoffHelper function used to set the underlying application attributes. Traffic flow from one node to the other node (for all the nodes) 

  	onOffD.SetAttribute ("DataRate", DataRateValue (DataRate (40000000))); //set the data rate (4K panoramic video)
  	onOffD.SetAttribute ("PacketSize", UintegerValue (packetSize)); // set the packet size
  	onOffD.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]")); //set on value time to a constanct value 1.0 second. A RandomVariableStream used to pick the duration of the 'On' state. Schedules the event to start sending data.
  	onOffD.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));//set off value time to a constanct value 0.0 second. A RandomVariableStream used to pick the duration of the 'Off' state. Schedules the event to stop sending data


	apps = onOffD.Install (wifiStaNodes.Get (3)); // traffic sources are installed in all nodes


	apps.Start (Seconds (myRandomNo));  // start time for stations
  	apps.Stop (Seconds (simulationTime+2e-6)); //stop time for stations (senders)
//=========================================================

OnOffHelper onOffE ("ns3::UdpSocketFactory", Address (InetSocketAddress (csmaInterfaces.GetAddress (0), port)));// It is a helper to make it easier to instantiate an ns3::OnOffApplication on a set of nodes. onoffHelper function used to set the underlying application attributes. Traffic flow from one node to the other node (for all the nodes) 

  	onOffE.SetAttribute ("DataRate", DataRateValue (DataRate (100000000))); //set the data rate (GSM 4G)
  	onOffE.SetAttribute ("PacketSize", UintegerValue (packetSize)); // set the packet size
  	onOffE.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]")); //set on value time to a constanct value 1.0 second. A RandomVariableStream used to pick the duration of the 'On' state. Schedules the event to start sending data.
  	onOffE.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));//set off value time to a constanct value 0.0 second. A RandomVariableStream used to pick the duration of the 'Off' state. Schedules the event to stop sending data


	apps = onOffE.Install (wifiStaNodes.Get (4)); // traffic sources are installed in all nodes


	apps.Start (Seconds (myRandomNo));  // start time for stations
  	apps.Stop (Seconds (simulationTime+2e-6)); //stop time for stations (senders)
//=========================================================	
*/

//4.15 ********** Connect Trace energysources **************************
  // all sources are connected to nodes


	Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (energysources.Get(i)); // pointer to the BasicEnergySource

 	 // device energy model
 	 Ptr<DeviceEnergyModel> basicRadioModelPtr = basicSourcePtr->FindDeviceEnergyModels ("ns3::WifiRadioEnergyModel").Get (0);
  		NS_ASSERT (basicRadioModelPtr != NULL);
  	
	// energy harvester
 	 Ptr<BasicEnergyHarvester> basicHarvesterPtr = DynamicCast<BasicEnergyHarvester> (harvesters.Get (i));
	

}
	energysources.Get (0)->TraceConnectWithoutContext ("RemainingEnergy",MakeCallback (&RemainingEnergy<0>));
	energysources.Get (1)->TraceConnectWithoutContext ("RemainingEnergy",MakeCallback (&RemainingEnergy<1>));
	energysources.Get (2)->TraceConnectWithoutContext ("RemainingEnergy",MakeCallback (&RemainingEnergy<2>));
	energysources.Get (3)->TraceConnectWithoutContext ("RemainingEnergy",MakeCallback (&RemainingEnergy<3>));
	energysources.Get (4)->TraceConnectWithoutContext ("RemainingEnergy",MakeCallback (&RemainingEnergy<4>));

	harvesters.Get (0)->TraceConnectWithoutContext ("TotalEnergyHarvested",MakeCallback (&TotalEnergyHarvested<0>));
	harvesters.Get (1)->TraceConnectWithoutContext ("TotalEnergyHarvested",MakeCallback (&TotalEnergyHarvested<1>));
	harvesters.Get (2)->TraceConnectWithoutContext ("TotalEnergyHarvested",MakeCallback (&TotalEnergyHarvested<2>));
	harvesters.Get (3)->TraceConnectWithoutContext ("TotalEnergyHarvested",MakeCallback (&TotalEnergyHarvested<3>));
	harvesters.Get (4)->TraceConnectWithoutContext ("TotalEnergyHarvested",MakeCallback (&TotalEnergyHarvested<4>));
//=========================================================

//4.16 ***************Global Routing ***************
        /* adds ns3::Ipv4GlobalRoutingHelper objects, construct a GlobalRoutingHelper to manage global routing tasks. The entire internetwork accessible in the simulation and runs through the all of the nodes created for simulation. It works for each node as it were OSPF (Open Shortest Path First) router that communicates instantly with all other routers. Each node generates link advertisements and communicates them directly to a global route manager which uses this global information to construct the routing tables for each node.

It Builds a routing database and initialize the routing tables of the nodes in the simulation. Makes all nodes in the simulation into routers. 

PopulateRoutingTables () : Remove all routes that were previously installed in a prior call to either PopulateRoutingTables() or RecomputeRoutingTables(), and add a new set of routes. This method does not change the set of nodes over which GlobalRouting is being used, but it will dynamically update its representation of the global topology before recomputing routes. Users must first call PopulateRoutingTables() and then may subsequently call RecomputeRoutingTables() at any later time in the simulation. */

        Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

//=========================================================
 	


 //4.17 ***************FlowMonitor***************
       
 	FlowMonitorHelper flowmon; // This helper is enabling IP flow monitoring on a set of Nodes
	Ptr<FlowMonitor> monitor = flowmon.InstallAll(); //flowmonitor is installed in all the objects

//========================================================= 

       phy.EnablePcap ("AccessPoint", apDevices);
       phy.EnablePcap ("Station", staDevices);
 //4.18 ****************Simulation Stop and Run**************

	/* when there is a recurring event it is necessary to stop the simulator because they always keep the event queue non-empty. 		events such as (flow monitor because it search for lost packets periodically and periodic broadcast of routing tables update -RIPng-)*/


	Simulator::Stop (Seconds (simulationTime+4e-6)); // always before calling Run
	Simulator::Run (); // The system will begin looking through the list of scheduled events and executing them

  //=========================================================

//4.19 ***************Print Per flow statistics***************
       
monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
      
    	  NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
    	  NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
    	  NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
    	  NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps");
        
    }
  //=========================================================


 //4.20 ****************Print the result for the total energy consumption by the radio in each node***********


  for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin (); iter != deviceModels.End (); iter ++)
    {
      double energyConsumed = (*iter)->GetTotalEnergyConsumption ();
      NS_LOG_UNCOND ("End of simulation (" << Simulator::Now ().GetSeconds () << "s) Total energy consumed by radio = " << energyConsumed << "J");
      NS_ASSERT (energyConsumed <= 100);
    }
 //=========================================================

 //4.21 ****************Simulation destory**************
	Simulator::Destroy (); // It is a global function that destroy all the objects that were created in the code
	return 0;
}
