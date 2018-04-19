#include <vector>
#include <iomanip>


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"


using namespace ns3;
using namespace std;

// Parameters
double          d_LinkSpeed               = 10.0;
string          LinkSpeed                 = "10Gbps";
string          LinkDelay                 = "5us";
double          linkDelay                 = 5e-6;
uint32_t        MachinesPerRack           = 16;
uint32_t        PodSize                   = 4;
uint32_t        NumberofRacks             = 64;
uint32_t        NumberofCoreSwitches      = 16;
uint32_t        Scale                     = 1;
double          load                      = 0.1;
double          longLoadFactor            = 0.6;
double          longNodeFactor            = 0.25;
uint32_t        FlowSizeLong              = 100*1024*1024;
uint32_t        FlowSizeShort             = 4.67*1024;
uint32_t        threshold                 = 22.5e3;
uint32_t        queueBytes                = 225e3;      // Number of bytes per queue port
uint32_t        initCwnd                  = 10;         // TCP Initial Congestion Window
double          minRto                    = 10000e-6;
uint32_t        segmentSize               = 1460;
bool            pktspray                  = 0;
bool            dctcp                     = 1;
bool            LinkUtilization           = 0;
string          StatsFileName             = "Dummy.txt";
double          interval                  = 0.05;
double          testruntime               = 1;
uint32_t	incastDegree		  = 16;


// Derived Parameters
uint32_t	LongRequestsPerNode;
uint32_t	ShortRequestsPerNode;
uint32_t        NumberofEndPoints;
uint32_t        NumberofEndPointsLong;
uint32_t        NumberofEndPointsShort;
double          InterArrival_long;
double          InterArrival_short;
uint32_t        NumShortFlows;
uint32_t        NumLongFlows;
double          shortLoadFactor;

std::ostringstream oss;

// declare the node pointers
vector < Ptr <Node> > nEnd;
vector < Ptr <Node> > nToR;
vector < Ptr <Node> > nAgg;
vector < Ptr <Node> > nCore;

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

 // calculate the derived knobs
  NumberofRacks             = NumberofRacks         * Scale;
  NumberofCoreSwitches      = NumberofCoreSwitches  * Scale;
  NumberofEndPoints         = MachinesPerRack       * NumberofRacks;

  NumberofEndPointsLong  = longNodeFactor * NumberofEndPoints; // each rack has one sender and receiver
  NumberofEndPointsShort = NumberofEndPoints - NumberofEndPointsLong;
  cout<<"Number of Long EndPoints :"<<NumberofEndPointsLong<<endl;
  cout<<"Number of Short EndPoints :"<<NumberofEndPointsShort<<endl;

  shortLoadFactor = 1.0 - longLoadFactor;
  cout<<"Short Load Factor :"<<shortLoadFactor<<endl;
  cout<<"Long Load Factor :"<<longLoadFactor<<endl;

    // use these instead of delay:
  InterArrival_long = (FlowSizeLong)*((NumberofEndPointsLong*8) / (load * longLoadFactor * d_LinkSpeed * NumberofEndPoints *1000000000));
  InterArrival_short = (NumberofEndPointsShort*FlowSizeShort*8*incastDegree) / (load * shortLoadFactor * d_LinkSpeed * NumberofEndPoints *1000000000); // TODO: Include incast degree in the formula 

  cout<<"Short Flow InterArrival Time :"<<InterArrival_short<<endl;
  cout<<"Long Flow InterArrival Time :"<<InterArrival_long<<endl;

  ShortRequestsPerNode = int ((1/InterArrival_short)*testruntime);
  LongRequestsPerNode = int ((1/InterArrival_long)*testruntime);

  cout<<"Short Requests Per Node :"<<ShortRequestsPerNode<<endl;
  cout<<"Long Requests Per Node :"<<LongRequestsPerNode<<endl;

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
  std::cout<<"Allocated " << nEnd.size() << " EndPoint nodes.."<<"\n";
  for (uint32_t i=0;i<NumberofRacks;i++)
  {
    nToR.push_back(CreateObject<Node> ());
    nAgg.push_back(CreateObject<Node> ());
  }
  std::cout<<"Allocated " << nToR.size() << " ToR + Agg switches.."<<"\n";
  for (uint32_t i=0;i<NumberofCoreSwitches;i++)
    nCore.push_back(CreateObject<Node> ());
  std::cout<<"Allocated " << nCore.size() << " Core switches.."<<"\n";


// Install IP
  InternetStackHelper internet;
  for (uint32_t i=0;i<NumberofRacks;i++)
  {
    internet.Install (nToR[i]);
    internet.Install (nAgg[i]);
  }
  for (uint32_t i=0;i<NumberofCoreSwitches;i++)
    internet.Install (nCore[i]);
  std::cout<<"Installed IP stack on all switches...\n";

  for (uint32_t i=0;i<NumberofEndPoints;i++)
    internet.Install(nEnd[i]);
  std::cout<<"Installed TCP/IP stack on all servers...\n";

  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute        ("DataRate",  StringValue (LinkSpeed));
  p2p.SetChannelAttribute       ("Delay",     StringValue (LinkDelay));

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

  // Assign IP address at each interface - first on the interface between EndPoints and TOR switches
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;

  // Between EndPoints and ToR Swicthes - assumes 1 uplink per machines
  for (uint32_t i=0;i<NumberofEndPoints;i++)
  {
    devices = p2p.Install (nEnd[i],nToR[i/MachinesPerRack]);
    interfaces = ipv4.Assign(devices);
    ipv4.NewNetwork();
  }

  // Between ToR Swicthes and Agg Switches (POD connectivity)
  for (uint32_t i=0;i<NumberofRacks;i+=PodSize)
    for (uint32_t j=0;j<PodSize;j++)
      for (uint32_t k=0;k<PodSize;k++)
      {
        devices = p2p.Install (nToR[i+j],nAgg[i+k]);
        interfaces = ipv4.Assign(devices);
        ipv4.NewNetwork();
      }

  // Between Agg swicthes and Core switches - assumes 2 uplinks per Agg
  for (uint32_t i=0;i<NumberofRacks;i++)
  {
    uint32_t coreidx1 = (((NumberofRacks/NumberofCoreSwitches) * i) + 0) % NumberofCoreSwitches;
    uint32_t coreidx2 = (((NumberofRacks/NumberofCoreSwitches) * i) + 1) % NumberofCoreSwitches;
    devices = p2p.Install (nAgg[i],nCore[coreidx1]);
    interfaces = ipv4.Assign(devices);
    ipv4.NewNetwork();

    devices = p2p.Install (nAgg[i],nCore[coreidx2]);
    interfaces = ipv4.Assign(devices);
    ipv4.NewNetwork();
  }

  // setup ip routing tables to get total ip-level connectivity.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  std::cout<<"Populated routing tables...\n";

}

void SetupServerTraffic (Ptr<Node> Nd, uint16_t appPort, Time startTime) 
{
  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), appPort));
  ApplicationContainer sinkApps = sink.Install (Nd);
  sinkApps.Start(startTime);
}

void SetupClientTraffic(Ptr<Node> Nd,Ptr<Node> ServerNode, uint32_t txsize, uint16_t appPort, Time startTime){

  BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress(ServerNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), appPort));
  source.SetAttribute ("MaxBytes", UintegerValue (txsize));
  source.SetAttribute ("SendSize", UintegerValue (txsize));
  ApplicationContainer sourceApps = source.Install (Nd);
  sourceApps.Start(startTime);
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

uint32_t  RandFlowSize(){

  Ptr<UniformRandomVariable> ApplicationDelay = CreateObject<UniformRandomVariable> ();
  ApplicationDelay->SetAttribute ("Min", DoubleValue (1));
  ApplicationDelay->SetAttribute ("Max", DoubleValue (99));
  uint32_t  flowsize;
  uint8_t chance = ApplicationDelay->GetInteger(1,100);
  if (chance < 33)
  {
         flowsize =  4*1024;
  }
  else if (chance > 33 && chance < 66)
  {
         flowsize  = 8*1024;
  }
  else 
  {
	flowsize = 6*1024;
  }
  return flowsize;
}


void Setup_Workload(){
  vector <uint32_t> AppIdxLong;
  vector <uint32_t> AppIdxShort;
  map<uint32_t,vector <uint16_t> > Map_Port;
  map<uint32_t,vector <uint32_t> > Map_Client;
  Map_Port.clear();
  Map_Client.clear();
  uint16_t appPort;
  AppIdxShort.clear();
  AppIdxLong.clear();
  Ptr<UniformRandomVariable> UniformlyRandomly = CreateObject<UniformRandomVariable> ();
  UniformlyRandomly->SetAttribute ("Min", DoubleValue (0));
  UniformlyRandomly->SetAttribute ("Max", DoubleValue (65535));
  uint32_t ServerNodeIdx,ClientNodeIdx;
  vector<Time> Prev_Start;
  Prev_Start.clear();
  Ptr<ExponentialRandomVariable> DelayRandomlyShort = CreateObject<ExponentialRandomVariable> ();
  Ptr<ExponentialRandomVariable> DelayRandomlyLong = CreateObject<ExponentialRandomVariable> ();
  DelayRandomlyShort->SetAttribute ("Mean", DoubleValue(InterArrival_short));
  DelayRandomlyLong->SetAttribute ("Mean", DoubleValue(InterArrival_long));  
 
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
      for(uint32_t j=0;j<MachinesPerRack*(1.0-longNodeFactor);j++)
      {
	 do {
		appNode = x->GetInteger ();
	    } while ((find(AppIdxShort.begin(),AppIdxShort.end(),appNode) != AppIdxShort.end()) || (find(AppIdxLong.begin(),AppIdxLong.end(),appNode) != AppIdxLong.end()));
	 AppIdxShort.push_back(appNode);
      }
      min = min + MachinesPerRack;
      max = max + MachinesPerRack;
  }

  NS_ASSERT(AppIdxLong.size() == NumberofEndPointsLong);
  NS_ASSERT(AppIdxShort.size() == NumberofEndPointsShort);

  for (uint32_t i=0;i<NumberofEndPoints;i++)
  {
    Prev_Start.push_back(Seconds(0.0));
  }

  for (uint32_t k=0;k < ShortRequestsPerNode; k++){
  	random_shuffle(AppIdxShort.begin(),AppIdxShort.end());
  	for (uint32_t i=0;i<AppIdxShort.size(); i=i+incastDegree+1){
      		ServerNodeIdx = AppIdxShort[i];
      		do {
          		appPort = UniformlyRandomly->GetInteger(0,65535);
        	} while (find(Map_Port[ServerNodeIdx].begin(),Map_Port[ServerNodeIdx].end(),appPort) != Map_Port[ServerNodeIdx].end());
        	Map_Port[ServerNodeIdx].push_back(appPort);
      		Time Curr_Start = Prev_Start[ServerNodeIdx] + Seconds (DelayRandomlyShort->GetValue());
      		Prev_Start[ServerNodeIdx] = Curr_Start;
      		SetupServerTraffic(nEnd[ServerNodeIdx],appPort,Curr_Start);
		for(uint32_t j=0;j<incastDegree;j++)
		{
                	
			ClientNodeIdx = AppIdxShort[i+j+1];
			SetupClientTraffic(nEnd[ClientNodeIdx],nEnd[ServerNodeIdx],RandFlowSize(),appPort,Curr_Start+Seconds (AppDelay()));
		}
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
                SetupServerTraffic(nEnd[ServerNodeIdx],appPort,Curr_Start);
                SetupClientTraffic(nEnd[ClientNodeIdx],nEnd[ServerNodeIdx],FlowSizeLong,appPort,Curr_Start);
		NumLongFlows++;
                Curr_Start = Prev_Start[ServerNodeIdx] + Seconds (DelayRandomlyLong->GetValue());
                Prev_Start[ServerNodeIdx] = Curr_Start;
                SetupServerTraffic(nEnd[ClientNodeIdx],appPort,Curr_Start);
                SetupClientTraffic(nEnd[ServerNodeIdx],nEnd[ClientNodeIdx],FlowSizeLong,appPort,Curr_Start);
		NumLongFlows++;

        }
  }
	cout<<"long flows Schedueled: " << NumLongFlows<<endl;


}


void SetupServer(Ptr<Node> rnode) 
{
  bool set_successful = true;
  Ptr<Ipv4L3Protocol> rnodeip = rnode->GetObject<Ipv4L3Protocol>();
  
  //Configure output queues
  for(uint32_t i = 0; i < rnodeip->GetNInterfaces(); i++) {
    Ptr<NetDevice> netdev = rnodeip->GetNetDevice(i);
    Ptr<PointToPointNetDevice> ptpnetdev = netdev->GetObject<PointToPointNetDevice>();
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

void Setup_Servers()
{
  for (uint32_t i=0;i<NumberofEndPoints;i++)
  {
    SetupServer(nEnd[i]);
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
  for (uint32_t i=0;i<NumberofCoreSwitches;i++)
  {
      MeasureLink(stream, nCore[i]);
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
    NS_ASSERT(count == (NumLongFlows));
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

