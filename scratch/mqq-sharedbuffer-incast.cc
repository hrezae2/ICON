#include <vector>
#include <iomanip>


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/dcn-module.h"


using namespace ns3;
using namespace std;

// Parameters
double          d_LinkSpeed               = 10.0;
string          LinkSpeed                 = "10Gbps";
string          LinkDelay                 = "15us";
double          linkDelay                 = 15e-6;
uint32_t        MachinesPerRack           = 20;
uint32_t        NumberofRacks             = 20;
uint32_t        NumberofSpineSwitches     = 10;
uint32_t        Scale                     = 1;
double          load                      = 0.1;
uint32_t 	threshold 		  = 33.5e3;
uint32_t 	queueBytes 		  = 6e6;      // Number of bytes per queue port
uint32_t 	initCwnd 		  = 10;         // TCP Initial Congestion Window
double 		minRto 			  = 10000e-6; 
uint32_t 	segmentSize 		  = 1460;
bool            LinkUtilization           = 0;
string          StatsFileName             = "128Dummy.txt";
double          interval                  = 0.010;

uint32_t	incastDegree		  = 32;
double		incastFactor		  = 0.5;

double          longLoadFactor	          = 0.8;
double          longNodeFactor            = 0.3;

uint32_t        FlowSizeShort             = 16*1024;
uint32_t        FlowSizeLong              = 1024 * 1024;

bool 		pktspray 		  = 0;
bool		dctcp			  = 1; 
double 		testruntime		  = 0.0025;

// Derived Parameters
uint32_t	LongRequestsPerNode;
uint32_t	ShortRequestsPerNode;
uint32_t        IncastRequestsPerNode;
uint32_t        NumberofEndPoints;
uint32_t        NumberofEndPointsLong;
uint32_t	NumberofIncastPoints;
uint32_t        NumberofEndPointsShort;
uint32_t	NumberofEndPointsIncast;
double          InterArrival_long;
double          InterArrival_short;
double          InterArrival_incast;
uint32_t        NumShortFlows;
uint32_t        NumIncastFlows;
uint32_t        NumLongFlows;
double          shortLoadFactor;
double		incastLoadFactor;
uint32_t	NumberofIncastSenders;
uint32_t	NumberofIncastAggregators;


std::ostringstream oss;

// declare the node pointers
vector < Ptr <Node> > nEnd;
vector < Ptr <Node> > nSpine;
vector < Ptr <Node> > nLeaf;

NS_LOG_COMPONENT_DEFINE ("DataCenterSimulator");

std::map<Ptr<Socket>, Time> sockets;

void QueuedPackets(uint32_t oldValue, uint32_t newValue)
{
  NS_LOG_INFO ("Packets in Queue at " << Simulator::Now ().GetSeconds ()<<"are \t"<<newValue);

}

void
QueueStat ()
{
  Config::ConnectWithoutContext ("/NodeList/1/DeviceList/1/$ns3::PointToPointNetDevice/TxQueue/$ns3::DropTailQueue/PacketsInQueue", MakeCallback (&QueuedPackets));
//  Simulator::Schedule (Seconds(0.1), &QueueStat);

}

void Configure_Simulator(){
  bool status = true;
  NS_LOG_INFO ("Configuring the DataCenter Simulator");

  Packet::EnablePrinting (); 

  NumberofRacks             =  NumberofRacks         * Scale;
  NumberofSpineSwitches      = NumberofSpineSwitches  * Scale;
  NumberofEndPoints         =  MachinesPerRack       * NumberofRacks;

  NumberofEndPointsLong  = longNodeFactor * NumberofEndPoints; // each rack has one sender and receiver
  NumberofEndPointsShort = (NumberofEndPoints - NumberofEndPointsLong)*(1.0-incastFactor);
  NumberofEndPointsIncast = NumberofEndPoints - NumberofEndPointsLong - NumberofEndPointsShort;
  cout<<"Number of Long EndPoints :"<<NumberofEndPointsLong<<endl;
  cout<<"Number of Short EndPoints :"<<NumberofEndPointsShort<<endl;
  cout<<"Number of Incasts EndPoints :"<<NumberofEndPointsIncast<<endl;
 

  shortLoadFactor = (1.0 - longLoadFactor)*(1.0 - incastFactor);
  incastLoadFactor = 1.0 - longLoadFactor - shortLoadFactor;
  cout<<"Short Load Factor :"<<shortLoadFactor<<endl;
  cout<<"Long Load Factor :"<<longLoadFactor<<endl;
  cout<<"incast Load Factor :"<<incastLoadFactor<<endl;

    // use these instead of delay:
  NumberofIncastSenders = NumberofEndPointsIncast - 32;//incastDegree * ((int) (NumberofEndPointsIncast/incastDegree));
  if(NumberofIncastSenders == NumberofEndPointsIncast){
	NumberofIncastSenders = incastDegree * (((int)(NumberofEndPointsIncast/incastDegree)) - 1 );
  }
  NumberofIncastAggregators = NumberofEndPointsIncast - NumberofIncastSenders;
  cout<<"Incast Senders are :"<<NumberofIncastSenders<<endl;
  cout<<"Incast Aggregators are :"<<NumberofIncastAggregators<<endl;
  
  if(longLoadFactor > 0)
  {
  	InterArrival_long = (NumberofEndPointsLong*FlowSizeLong*8) / (load * longLoadFactor * d_LinkSpeed * NumberofSpineSwitches*NumberofRacks *1000000000);
  }
  else{
	InterArrival_long = 0;
  }
  InterArrival_short = (NumberofEndPointsShort*FlowSizeShort*8) / (load * shortLoadFactor * d_LinkSpeed * NumberofSpineSwitches*NumberofRacks *1000000000);
  InterArrival_incast = (NumberofIncastAggregators*FlowSizeShort*8*incastDegree) / (load * incastLoadFactor * d_LinkSpeed * NumberofRacks * NumberofSpineSwitches *1000000000); // TODO: Include incast degree in the formula 
 
  cout<<"Short Flow InterArrival Time :"<<InterArrival_short<<endl;
  cout<<"Long Flow InterArrival Time :"<<InterArrival_long<<endl;
  cout<<"Incast Flow InterArrival Time :"<<InterArrival_incast<<endl;
  
  ShortRequestsPerNode = int ((1/InterArrival_short)*testruntime);
  if(InterArrival_long > 0)
  {
	LongRequestsPerNode = int ((1/InterArrival_long)*testruntime);
  }
  else{
	LongRequestsPerNode = 0;
  }

  IncastRequestsPerNode = int ((1/InterArrival_incast)*testruntime);

  cout<<"Short Requests Per Node :"<<ShortRequestsPerNode<<endl;
  cout<<"Long Requests Per Node :"<<LongRequestsPerNode<<endl;
  cout<<"Incast Requests Per Node :"<<IncastRequestsPerNode<<endl;

  cout<<"Short Flow Size :"<<FlowSizeShort<<endl;
  cout<<"Long Flow Size :"<<FlowSizeLong<<endl;

  ofstream outfile;
  outfile.open(StatsFileName.c_str());
  outfile.close();

  double minRtt = (8 * linkDelay);

        /* Set the simple parameters */
  status = Config::SetDefaultFailSafe ("ns3::TcpSocket::SegmentSize", UintegerValue (segmentSize));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocketBase::FCTFileName", StringValue  ( StatsFileName));
  status &= Config::SetDefaultFailSafe ("ns3::RttEstimator::InitialEstimation", TimeValue ( Seconds (minRtt)));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::SndBufSize", UintegerValue (1e9)); //Large value
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::RcvBufSize", UintegerValue (1e9)); //Large value
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::InitialCwnd", UintegerValue (initCwnd));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::ConnTimeout", TimeValue (Seconds (minRtt)));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::ConnCount", UintegerValue (33));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocketBase::MultiQueue", BooleanValue(true));
  if(!pktspray)
  {
  	status &= Config::SetDefaultFailSafe ("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
  }
  else
  {
        status &= Config::SetDefaultFailSafe ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
        status &= Config::SetDefaultFailSafe ("ns3::TcpNewReno::ReTxThreshold", UintegerValue(20));
  }
  status &= Config::SetDefaultFailSafe ("ns3::RttEstimator::MinRTO", TimeValue ( Seconds (minRto)));
  if(!dctcp)
  {
     status &= Config::SetDefaultFailSafe ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
  }
  else
  {
     status &= Config::SetDefaultFailSafe ("ns3::Ipv4L3Protocol::ECN", BooleanValue (true));
     status &= Config::SetDefaultFailSafe ("ns3::TcpL4Protocol::SocketType", TypeIdValue (Dctcp::GetTypeId ()));
  }
  NS_ASSERT(status);
}

void
PrintTime (void)
{
    NS_LOG_UNCOND (Simulator::Now ());
    Simulator::Schedule (Seconds(0.005), &PrintTime);
}

void Build_Topology()
{
  NS_LOG_INFO ("Building the Topology");
  for (uint32_t i=0;i<NumberofEndPoints;i++)
    nEnd.push_back(CreateObject<Node> ());
  NS_LOG_INFO("Allocated " << nEnd.size() << " EndPoint nodes..");
  for (uint32_t i=0;i<NumberofSpineSwitches;i++)
  {
    nSpine.push_back(CreateObject<Node> ());
  }
  NS_LOG_INFO("Allocated " << nSpine.size() << " Spine switches..");
  for (uint32_t i=0;i<NumberofRacks;i++)
    nLeaf.push_back(CreateObject<Node> ());
  NS_LOG_INFO("Allocated " << nLeaf.size() << " Leaf switches..");
  
  NS_LOG_INFO("Installing IP stack on the nodes ..");
  InternetStackHelper internet;
  for (uint32_t i=0;i<NumberofRacks;i++)
  {
    internet.Install (nLeaf[i]);
  }
  for (uint32_t i=0;i<NumberofSpineSwitches;i++)
    internet.Install (nSpine[i]);
  NS_LOG_INFO("Installed IP stack on all switches...");
  for (uint32_t i=0;i<NumberofEndPoints;i++)
    internet.Install(nEnd[i]);
  NS_LOG_INFO("Installed TCP/IP stack on all servers...");

  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute	("DataRate",  StringValue (LinkSpeed));
  p2p.SetChannelAttribute	("Delay",     StringValue (LinkDelay));
  std::string queueType = "ns3::DropTailQueue";
  std::string n1 = "Mode";
  Ptr<AttributeValue> v1 = Create<EnumValue> (DropTailQueue::QUEUE_MODE_BYTES);
  std::string n2 = "MaxBytes";
  Ptr<AttributeValue> v2 = Create<UintegerValue> (queueBytes);
  std::string n3 = "";
  Ptr<AttributeValue> v3 = Create<EmptyAttributeValue> ();

  if(dctcp) {
        n3 = "Th";
        queueType = "ns3::SimpleRedEcnQueue";
        v3 = Create<UintegerValue> (threshold);
  }

  p2p.SetQueue  (queueType,
              n1, *v1,
              n2, *v2,
              n3, *v3);


  PointToPointHelper e2e;
  e2e.SetDeviceAttribute        ("DataRate",  StringValue (LinkSpeed));
  e2e.SetChannelAttribute       ("Delay",     StringValue (LinkDelay));
  e2e.SetDevice ("ns3::SharedNetDevice",
    "PauseTime", UintegerValue (40));
  e2e.SetQueue  ("ns3::SimpleRedEcnQueue",
              n1, *v1,
              n2, *v2,
              n3, *v3);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;

  for (uint32_t i=0;i<NumberofEndPoints;i++){
    devices = e2e.Install (nEnd[i],nLeaf[i/MachinesPerRack]);
    interfaces = ipv4.Assign(devices);
    ipv4.NewNetwork();
  }
  for (uint32_t i=0;i<NumberofSpineSwitches;i++){
    for (uint32_t j=0;j<NumberofRacks;j++){
	devices = e2e.Install(nSpine[i],nLeaf[j]);
	interfaces = ipv4.Assign(devices);
	ipv4.NewNetwork();
    }
  }
 
  NS_LOG_INFO	("Populate routing tables.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

}

void SetupServerTraffic (Ptr<Node> Nd, uint16_t appPort, Time startTime, uint8_t priority) 
{
  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), appPort));
  sink.SetAttribute ("Priority", UintegerValue (priority));
  ApplicationContainer sinkApps = sink.Install (Nd);
  sinkApps.Start(startTime);
}

void SetupClientTraffic(Ptr<Node> Nd,Ptr<Node> ServerNode, uint32_t txsize, uint16_t appPort, Time startTime , uint8_t priority){

  BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress(ServerNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), appPort));
  source.SetAttribute ("MaxBytes", UintegerValue (txsize));
  source.SetAttribute ("SendSize", UintegerValue (txsize));
  source.SetAttribute ("Priority", UintegerValue (priority));
  ApplicationContainer sourceApps = source.Install (Nd);
  sourceApps.Start(startTime);
}

void SetDataRate(Ptr<Node> rnode, int DummyID)
{
  Ptr<Ipv4L3Protocol> rnodeip = rnode->GetObject<Ipv4L3Protocol>();

  //Configure Transmit Data Rate
  for(uint32_t i = 0; i < rnodeip->GetNInterfaces(); i++) {
    Ptr<NetDevice> netdev = rnodeip->GetNetDevice(i);
    Ptr<SharedNetDevice> ptpnetdev = netdev->GetObject<SharedNetDevice>();
    if(!ptpnetdev) {
      continue;
    }
    ptpnetdev->SetAttributeFailSafe("ServerNode",      BooleanValue(true));

    }

}

double  AppDelay(){

  Ptr<UniformRandomVariable> ApplicationDelay = CreateObject<UniformRandomVariable> ();
  ApplicationDelay->SetAttribute ("Min", DoubleValue (1));
  ApplicationDelay->SetAttribute ("Max", DoubleValue (100));
  double  delay;
  uint8_t chance = ApplicationDelay->GetInteger(1,100);
  if (chance < 80)
  {
        delay =  0.00;
  }
  else
  {
        delay = 0.000082;
  }
  return delay;
}


void Setup_Workload(){
  vector <uint32_t> AppIdxLong;
  vector <uint32_t> AppIdxShort;
  vector <uint32_t> AppIdxIncast;
  vector <uint32_t> IncastSenders;
  vector <uint32_t> IncastAggregators;
  map<uint32_t,vector <uint16_t> > Map_Port;
  map<uint32_t,vector <uint32_t> > Map_Client;
  Map_Port.clear();
  Map_Client.clear();
  uint16_t appPort;
  AppIdxShort.clear();
  AppIdxLong.clear();
  AppIdxIncast.clear();
  IncastSenders.clear();
  IncastAggregators.clear();
  Ptr<UniformRandomVariable> UniformlyRandomly = CreateObject<UniformRandomVariable> ();
  UniformlyRandomly->SetAttribute ("Min", DoubleValue (0));
  UniformlyRandomly->SetAttribute ("Max", DoubleValue (65535));
  uint32_t ServerNodeIdx,ClientNodeIdx;
  vector<Time> Prev_Start;
  Prev_Start.clear();
  Ptr<ExponentialRandomVariable> DelayRandomlyShort = CreateObject<ExponentialRandomVariable> ();
  Ptr<ExponentialRandomVariable> DelayRandomlyLong = CreateObject<ExponentialRandomVariable> ();
  Ptr<ExponentialRandomVariable> DelayRandomlyIncast = CreateObject<ExponentialRandomVariable> ();  
  DelayRandomlyShort->SetAttribute ("Mean", DoubleValue(InterArrival_short));
  DelayRandomlyLong->SetAttribute ("Mean", DoubleValue(InterArrival_long));  
  DelayRandomlyIncast->SetAttribute ("Mean", DoubleValue(InterArrival_incast));
  
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  uint32_t min = 0.0;
  uint32_t max = MachinesPerRack-1;
  uint32_t appNode = 0.0;
  for(uint32_t i=0;i<NumberofRacks;i++)
  {
      
      x->SetAttribute ("Min", DoubleValue (min));
      x->SetAttribute ("Max", DoubleValue (max));
      for(uint32_t j=0;j<MachinesPerRack*longNodeFactor;j++)
      {
          do {
		appNode = x->GetInteger ();
             } while ((find(AppIdxShort.begin(),AppIdxShort.end(),appNode) != AppIdxShort.end()) || (find(AppIdxLong.begin(),AppIdxLong.end(),appNode) != AppIdxLong.end()));
	     AppIdxLong.push_back(appNode);
      }
      for(uint32_t j=0;j<MachinesPerRack*(1.0-longNodeFactor)*(1.0 - incastFactor);j++)
      {
	 do {
		appNode = x->GetInteger ();
	    } while ((find(AppIdxShort.begin(),AppIdxShort.end(),appNode) != AppIdxShort.end()) || (find(AppIdxLong.begin(),AppIdxLong.end(),appNode) != AppIdxLong.end()));
	 AppIdxShort.push_back(appNode);
      }
      min = min + MachinesPerRack;
      max = max + MachinesPerRack;
  }

  for (uint32_t i=0;i<NumberofEndPoints;i++)
  {
    bool IncastId = true;
    Prev_Start.push_back(Seconds(0.0));
    for(uint32_t j=0;j<AppIdxLong.size();j++)
    {
        if(AppIdxLong[j] == i)
	{
		IncastId = false;
    	}
    }
    for(uint32_t j=0;j<AppIdxShort.size();j++)
    {
        if(AppIdxShort[j] == i)
        {
                IncastId = false;
        }
    }
    if(IncastId)
    {

	AppIdxIncast.push_back(i);
    }

  }
  NS_ASSERT(AppIdxLong.size() == NumberofEndPointsLong);
  NS_ASSERT(AppIdxShort.size() == NumberofEndPointsShort);
  NS_ASSERT(AppIdxIncast.size() == NumberofEndPointsIncast);

  for (uint32_t k=0;k < ShortRequestsPerNode; k++){
  	random_shuffle(AppIdxShort.begin(),AppIdxShort.end());
  	for (uint32_t i=0;i<AppIdxShort.size(); i=i+2){
      		ServerNodeIdx = AppIdxShort[i];
      		ClientNodeIdx = AppIdxShort[i+1];
      		do {
          		appPort = UniformlyRandomly->GetInteger(0,65535);
        	} while (find(Map_Port[ServerNodeIdx].begin(),Map_Port[ServerNodeIdx].end(),appPort) != Map_Port[ServerNodeIdx].end());
        	Map_Port[ServerNodeIdx].push_back(appPort);
      		Time Curr_Start = Prev_Start[ClientNodeIdx] + Seconds (DelayRandomlyShort->GetValue());
      		Prev_Start[ClientNodeIdx] = Curr_Start;
      		SetupServerTraffic(nEnd[ServerNodeIdx],appPort,Curr_Start,0);
      		SetupClientTraffic(nEnd[ClientNodeIdx],nEnd[ServerNodeIdx],FlowSizeShort,appPort,Curr_Start,0);
		NumShortFlows++;
                Curr_Start = Prev_Start[ServerNodeIdx] + Seconds (DelayRandomlyShort->GetValue());
                Prev_Start[ServerNodeIdx] = Curr_Start;
                SetupServerTraffic(nEnd[ClientNodeIdx],appPort,Curr_Start,0);
                SetupClientTraffic(nEnd[ServerNodeIdx],nEnd[ClientNodeIdx],FlowSizeShort,appPort,Curr_Start,0);
		NumShortFlows++;
	}
  }

        cout<<"Short flows Schedueled: " << NumShortFlows<<endl;

  for (uint32_t k=0;k < LongRequestsPerNode; k++){
        random_shuffle(AppIdxLong.begin(),AppIdxLong.end());
        for (uint32_t i=0;i<AppIdxLong.size(); i=i+2){
                ServerNodeIdx = AppIdxLong[i];
                ClientNodeIdx = AppIdxLong[i+1];
                do {
                        appPort = UniformlyRandomly->GetInteger(0,65535);
                } while (find(Map_Port[ServerNodeIdx].begin(),Map_Port[ServerNodeIdx].end(),appPort) != Map_Port[ServerNodeIdx].end());
                Map_Port[ServerNodeIdx].push_back(appPort);
                Time Curr_Start = Prev_Start[ClientNodeIdx] + Seconds (DelayRandomlyLong->GetValue());
                Prev_Start[ClientNodeIdx] = Curr_Start;
                SetupServerTraffic(nEnd[ServerNodeIdx],appPort,Curr_Start,0);
                SetupClientTraffic(nEnd[ClientNodeIdx],nEnd[ServerNodeIdx],FlowSizeLong,appPort,Curr_Start,0);
		NumLongFlows++;
                Curr_Start = Prev_Start[ServerNodeIdx] + Seconds (DelayRandomlyLong->GetValue());
                Prev_Start[ServerNodeIdx] = Curr_Start;
                SetupServerTraffic(nEnd[ClientNodeIdx],appPort,Curr_Start,0);
                SetupClientTraffic(nEnd[ServerNodeIdx],nEnd[ClientNodeIdx],FlowSizeLong,appPort,Curr_Start,0);
		NumLongFlows++;
        }
  }
	cout<<"long flows Schedueled: " << NumLongFlows<<endl;

 
 random_shuffle(AppIdxIncast.begin(),AppIdxIncast.end());
 
 for(uint32_t i=0; i < AppIdxIncast.size();i++){
     if(i < NumberofIncastSenders)
     {
	IncastSenders.push_back(AppIdxIncast[i]);
     }
     else{
        IncastAggregators.push_back(AppIdxIncast[i]);
     }	
  }

  NS_ASSERT(IncastSenders.size() == NumberofIncastSenders);
  NS_ASSERT(IncastAggregators.size() == NumberofIncastAggregators);
  
 

 for(uint32_t i=0; i <IncastAggregators.size();i++){
  	random_shuffle(IncastSenders.begin(),IncastSenders.end());
	for(uint32_t j=0;j<incastDegree;j++)
	{
		Map_Client[IncastAggregators[i]].push_back(IncastSenders[j]);
	
	}


}


  for (uint32_t k=0;k < IncastRequestsPerNode; k++){
        for (uint32_t i=0;i<IncastAggregators.size(); i++){
                ServerNodeIdx = IncastAggregators[i];
                do {
                	appPort = UniformlyRandomly->GetInteger(0,65535);
                   } while (find(Map_Port[ServerNodeIdx].begin(),Map_Port[ServerNodeIdx].end(),appPort) != Map_Port[ServerNodeIdx].end());
                Map_Port[ServerNodeIdx].push_back(appPort);
                Time Curr_Start = Prev_Start[ServerNodeIdx] + Seconds (DelayRandomlyIncast->GetValue());
                Prev_Start[ServerNodeIdx] = Curr_Start;
                SetupServerTraffic(nEnd[ServerNodeIdx],appPort,Curr_Start,0);
		vector<uint32_t> ClientIdx = Map_Client[ServerNodeIdx];
		for(uint32_t j=0;j<ClientIdx.size();j++)
		{
                	
			ClientNodeIdx = ClientIdx[j];
		//	SetupClientTraffic(nEnd[ClientNodeIdx],nEnd[ServerNodeIdx],FlowSizeShort,appPort,Curr_Start,0);
			SetupClientTraffic(nEnd[ClientNodeIdx],nEnd[ServerNodeIdx],FlowSizeShort,appPort,Curr_Start+Seconds (AppDelay()), 0);
		}
        	NumIncastFlows++;
	}
  }

        cout<<"Incast flows Schedueled: " << NumIncastFlows<<endl;

}

void SetupServer(Ptr<Node> rnode) 
{
  bool set_successful = true;
  Ptr<Ipv4L3Protocol> rnodeip = rnode->GetObject<Ipv4L3Protocol>();
  
  //Configure output queues
  for(uint32_t i = 0; i < rnodeip->GetNInterfaces(); i++) {
    Ptr<NetDevice> netdev = rnodeip->GetNetDevice(i);
    Ptr<MQQNetDevice> ptpnetdev = netdev->GetObject<MQQNetDevice>();
    if(!ptpnetdev) {
      continue;
    }
    
    Ptr<Queue> queue = ptpnetdev->GetQueue();
    
    if(!dctcp){
    	Ptr<DropTailQueue> dtqueue = queue->GetObject<DropTailQueue>();
        set_successful &= dtqueue->SetAttributeFailSafe("Mode",      EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
        set_successful &= dtqueue->SetAttributeFailSafe("MaxBytes",  UintegerValue(4 * 1024 * 1024)); //4 MB typical to accomodate 
	}
    else{
    	Ptr<SimpleRedEcnQueue> dtqueue = queue->GetObject<SimpleRedEcnQueue>();
    	set_successful &= dtqueue->SetAttributeFailSafe("Mode",      EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
    	set_successful &= dtqueue->SetAttributeFailSafe("MaxBytes",  UintegerValue(4 * 1024 * 1024)); //4 MB typical to accomodate 
	set_successful &= dtqueue->SetAttributeFailSafe("Th",      UintegerValue(4 * 1024 * 1024));
    }	
  }
  
  NS_ASSERT(set_successful);
}

void SetupSharedBufferSwitch(Ptr<Node> rnode)
{
  rnode->SetNodeType(1); 
}

void Setup_Servers()
{
  for (uint32_t i=0;i<NumberofEndPoints;i++)
  {
   // SetupServer(nEnd[i]);
    SetDataRate(nEnd[i], i);
  }

  for (uint32_t i=0;i<NumberofRacks;i++)
  {
      SetupSharedBufferSwitch(nLeaf[i]);
  }

  for (uint32_t i=0;i<NumberofSpineSwitches;i++)
  {
    SetupSharedBufferSwitch(nSpine[i]);
  }

}

void MeasureLink (Ptr<OutputStreamWrapper> stream, Ptr<Node> rnode)
{
  double link_utilization;

  Ptr<Ipv4L3Protocol> rnodeip = rnode->GetObject<Ipv4L3Protocol>();
  NS_ASSERT(rnodeip);

  for(uint32_t j = 0; j < rnodeip->GetNInterfaces(); j++)
  {
    Ptr<NetDevice> netdev = rnodeip->GetNetDevice(j);
    Ptr<PointToPointNetDevice> ptpnetdev = netdev->GetObject<PointToPointNetDevice>();
    if(!ptpnetdev) {
      continue;
    }
    NS_ASSERT(ptpnetdev);
    link_utilization = (double) (ptpnetdev->m_bytes_transmitted * 8)/ interval;
    ptpnetdev->m_bytes_transmitted = 0;
    *stream->GetStream ()<<"Node: "<< rnode->GetId()<<"\tInterface:"<< j <<"\tlink utilization:"<<link_utilization<<endl;
  }
}

void MeasureLinkUtilization(Ptr<OutputStreamWrapper> stream)
{
  for (uint32_t i=0;i<NumberofSpineSwitches;i++)
  {
      MeasureLink(stream, nSpine[i]);
  }
  Simulator::Schedule(Seconds(interval),&MeasureLinkUtilization,stream);
}

void ProbeApps(uint32_t junk)
{
  bool stopnow = true;
  int total = 0;
  uint32_t count = 0;
  for (uint32_t i=0;i<NumberofEndPoints;i++)
  {
      for (uint32_t j=0;j<nEnd[i]->GetNApplications();j++)
      {
        Ptr <Application> Traceapp = nEnd[i]->GetApplication(j);
        if (Traceapp)
        {
          Ptr <BulkSendApplication> Tracebulk = Traceapp->GetObject<BulkSendApplication>();
          if (Tracebulk)
          {
            total++;
            if (Tracebulk->m_application_ended == false)
            {
              stopnow = false;
            }
            else
            {
              count++;
            }
          }
        }
      }
  }
  if (stopnow)
  {
    NS_ASSERT(count == (NumShortFlows + NumLongFlows + NumIncastFlows));
    Simulator::Stop();
  }
  else
  {
    Simulator::Schedule(Seconds(interval),&ProbeApps,0);
  }
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.AddValue ("load",         	  "Controls the n/w load",              load);
  cmd.AddValue ("dctcp",        	  "Enable DCTCP",            		dctcp);
  cmd.AddValue ("runtime",                "Test Runtime in Seconds",  		testruntime);
  cmd.AddValue ("random",		  "Load Balancing : Random ECMP", 	pktspray); 
  cmd.AddValue ("incastdegree", 	  "Degree of Incast",			incastDegree);
  cmd.AddValue ("filename",               "FileName to Dump Stats",	        StatsFileName);
  cmd.AddValue ("linkutilization",        "Measure Link Utilization",      	LinkUtilization);
  cmd.Parse (argc, argv);
    if(!dctcp)
  {
	cout<<"TCPNEWRENO"<<endl;
  }
  else
  {
	cout<<"DCTCP"<<endl;
  }
  cout<<"Load "<<load<<endl;
  Time::SetResolution (Time::NS);
  LogComponentEnable ("DataCenterSimulator", LOG_ALL);
  NS_LOG_INFO ("DataCenter Simulation");
 
  Configure_Simulator();
 
  Build_Topology();

  Setup_Servers();

  NS_LOG_INFO ("Setting Up Workload");
 
  Setup_Workload();

  NS_LOG_INFO ("Starting Simulation");

  //Simulator::Schedule (Seconds(0.1), &QueueStat);
  if(LinkUtilization)
  {
         oss << "Util." <<StatsFileName ;
         AsciiTraceHelper asciiTraceHelper;
         Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (oss.str ());
         Simulator::Schedule(Seconds(interval),&MeasureLinkUtilization,stream);
         Simulator::Schedule(Seconds(interval),&ProbeApps,0);
 }

  Simulator::Run();

  Simulator::Destroy ();
  
  NS_LOG_INFO ("Simulation Finished");

  return 0;
}

