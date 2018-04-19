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
string          LinkDelay                 = "10us";
double          linkDelay                 = 10e-6;
uint32_t        MachinesPerRack           = 10;
uint32_t        NumberofRacks             = 20;
uint32_t        NumberofSpineSwitches     = 10;
uint32_t        Scale                     = 1;
double          load                      = 0.3;
uint32_t	RequestsPerNode		  = 100;
uint32_t 	threshold 		  = 22.5e3;
uint32_t 	queueBytes 		  = 225e3;      // Number of bytes per queue port
uint32_t 	initCwnd 		  = 10;         // TCP Initial Congestion Window
double 		minRto 			  = 10000e-6; 
uint32_t 	segmentSize 		  = 1460;
uint32_t	FlowSize		  = 16*1024;
bool 		pktspray 		  = 0;
bool		dctcp			  = 0; 
double		interval		  = 0.01;

// Derived Parameters
uint32_t        NumberofEndPoints;
double  	RequestsPerSeconds;
double	        InterArrival_delay;
uint32_t	NumberofFlows;

// declare the node pointers
vector < Ptr <Node> > nEnd;
vector < Ptr <Node> > nSpine;
vector < Ptr <Node> > nLeaf;

std::string simPrefix = "DataCenterSim";
Ptr<OutputStreamWrapper> stream;

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

    // use these instead of delay:
  InterArrival_delay = (NumberofEndPoints*FlowSize*8) / (load * d_LinkSpeed * NumberofSpineSwitches*NumberofRacks *1000000000);

  cout<<"RequestsPerNode :"<<RequestsPerNode<<endl;

  ofstream outfile;
  outfile.open("Client.txt");
  outfile.close();

  double minRtt = (8 * linkDelay);

        /* Set the simple parameters */
  status = Config::SetDefaultFailSafe ("ns3::TcpSocket::SegmentSize", UintegerValue (segmentSize));
  status &= Config::SetDefaultFailSafe ("ns3::RttEstimator::InitialEstimation", TimeValue ( Seconds (minRtt)));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::SndBufSize", UintegerValue (1e8)); //Large value
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::RcvBufSize", UintegerValue (1e8)); //Large value
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::ConnTimeout", TimeValue (Seconds (minRtt)));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::ConnCount", UintegerValue (33));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::InitialCwnd", UintegerValue (initCwnd));
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
 
  p2p.SetQueue	(queueType,
              n1, *v1,
              n2, *v2,
              n3, *v3);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;

  for (uint32_t i=0;i<NumberofEndPoints;i++){
    devices = p2p.Install (nEnd[i],nLeaf[i/MachinesPerRack]);
    interfaces = ipv4.Assign(devices);
    ipv4.NewNetwork();
  }
  for (uint32_t i=0;i<NumberofSpineSwitches;i++){
    for (uint32_t j=0;j<NumberofRacks;j++){
	devices = p2p.Install(nSpine[i],nLeaf[j]);
	interfaces = ipv4.Assign(devices);
	ipv4.NewNetwork();
    }
  }
 
  NS_LOG_INFO	("Populate routing tables.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

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

void Setup_Workload(){
  vector <uint32_t> AppIdx;
  map<uint32_t,vector <uint16_t> > Map_Port;
  Map_Port.clear();
  uint16_t appPort;
  AppIdx.clear();
  Ptr<UniformRandomVariable> UniformlyRandomly = CreateObject<UniformRandomVariable> ();
  UniformlyRandomly->SetAttribute ("Min", DoubleValue (0));
  UniformlyRandomly->SetAttribute ("Max", DoubleValue (65535));
  uint32_t ServerNodeIdx,ClientNodeIdx;
  vector<Time> Prev_Start;
  Prev_Start.clear();
  Ptr<ExponentialRandomVariable> DelayRandomly = CreateObject<ExponentialRandomVariable> ();
  DelayRandomly->SetAttribute ("Mean", DoubleValue(InterArrival_delay));
  for (uint32_t i=0;i<NumberofEndPoints;i++)
  {
    Prev_Start.push_back(Seconds(0.0));
    AppIdx.push_back(i);
  }
    
  for (uint32_t k=0;k < RequestsPerNode; k++){
  	random_shuffle(AppIdx.begin(),AppIdx.end());
  	for (uint32_t i=0;i<AppIdx.size(); i=i+2){
      		ServerNodeIdx = AppIdx[i];
      		ClientNodeIdx = AppIdx[i+1];
      		do {
          		appPort = UniformlyRandomly->GetInteger(0,65535);
        	} while (find(Map_Port[ServerNodeIdx].begin(),Map_Port[ServerNodeIdx].end(),appPort) != Map_Port[ServerNodeIdx].end());
           	Map_Port[ServerNodeIdx].push_back(appPort);
      		Time Curr_Start = Prev_Start[ClientNodeIdx] + Seconds (DelayRandomly->GetValue ());
      		Prev_Start[ClientNodeIdx] = Curr_Start;
      		SetupServerTraffic(nEnd[ServerNodeIdx],appPort,Curr_Start);
      		SetupClientTraffic(nEnd[ClientNodeIdx],nEnd[ServerNodeIdx],FlowSize,appPort,Curr_Start);
		NumberofFlows++;
                Curr_Start = Prev_Start[ServerNodeIdx] + Seconds (DelayRandomly->GetValue ());
                Prev_Start[ServerNodeIdx] = Curr_Start;
                SetupServerTraffic(nEnd[ClientNodeIdx],appPort,Curr_Start);
                SetupClientTraffic(nEnd[ServerNodeIdx],nEnd[ClientNodeIdx],FlowSize,appPort,Curr_Start);
                NumberofFlows++;

	}
  }

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
    Ptr<DropTailQueue> dtqueue = queue->GetObject<DropTailQueue>();
    
    set_successful &= dtqueue->SetAttributeFailSafe("Mode",      EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
    set_successful &= dtqueue->SetAttributeFailSafe("MaxBytes",  UintegerValue(4 * 1024 * 1024)); //4 MB typical to accomodate 
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

void MeasureLink (Ptr<Node> rnode)
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
    cout<<"Node: "<< rnode->GetId()<<"\tInterface:"<< j <<"\tlink utilization:"<<link_utilization<<endl;
  }
}


void MeasureLinkUtilization(uint32_t junk)
{
  for (uint32_t i=0;i<NumberofSpineSwitches;i++)
  {
      MeasureLink(nSpine[i]);
  }
  Simulator::Schedule(Seconds(interval),&MeasureLinkUtilization,0);
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
    NS_ASSERT(count == (NumberofFlows));
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
  
  cmd.AddValue ("load",                   "Controls the n/w load",                        load);
  cmd.AddValue ("dctcp",                  "Enable DCTCP",            dctcp);
  cmd.AddValue ("flowsize",               "Size of Flow",            FlowSize);
  cmd.AddValue ("Random",	"Load Balancing : Random ECMP", pktspray); 
  cmd.Parse (argc, argv);

  if(!dctcp)
  {
	cout<<"TCPNEWRENO"<<endl;
  }
  else
  {
	cout<<"DCTCP"<<endl;
  }
  cout<<"FlowSize "<<FlowSize<<endl;
  cout<<"Load "<<load<<endl;
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("DataCenterSimulator", LOG_ALL);
  LogComponentEnable("Dctcp", LOG_ALL);
  NS_LOG_INFO ("DataCenter Simulation");
 
  Configure_Simulator();
 
  Build_Topology();

  Setup_Servers();
	
  NS_LOG_INFO ("Setting Up Workload");
 
  Setup_Workload();

  NS_LOG_INFO ("Starting Simulation");

  //Simulator::Schedule (Seconds(0.1), &QueueStat);
  Simulator::Schedule(Seconds(interval),&MeasureLinkUtilization,0);
  Simulator::Schedule(Seconds(interval),&ProbeApps,0);

  Simulator::Run();
  Simulator::Destroy ();
  
  NS_LOG_INFO ("Simulation Finished");

  return 0;
}

