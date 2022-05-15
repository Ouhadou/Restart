#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "model/simple-udp.h"

#include "ns3/point-to-point-module.h"

const uint32_t mysize = 100; 
using namespace ns3;

std::vector<uint8_t> output(mysize, 0); 

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
 
  
  NodeContainer nodes;
  nodes.Create (2);


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices; 
  devices = pointToPoint.Install(nodes);

  InternetStackHelper stack;
  stack.Install (nodes);
  
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer ifaces;
  //ifaces = address.Assign (csmaDevs);
  ifaces = address.Assign(devices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Packet::EnablePrinting (); 

  //Create our Two UDP applications
  Ptr <SimpleUdpApplication> udp0 = CreateObject <SimpleUdpApplication> ();
  Ptr <SimpleUdpApplication> udp1 = CreateObject <SimpleUdpApplication> ();
  
  //Set the start & stop times
  udp0->SetStartTime (Seconds(0));
  udp0->SetStopTime (Seconds (100));
  udp1->SetStartTime (Seconds(0));
  udp1->SetStopTime (Seconds (100));
  
  //install one application at node 0, and the other at node 1
  nodes.Get(0)->AddApplication (udp0);
  nodes.Get(1)->AddApplication (udp1);
  
  
  
  //This is the IP address of node 1
  Ipv4Address dest_ip ("10.1.1.2");

  
  Ptr <Packet> packet1 = Create <Packet> (mysize);

  
  
  
  Simulator::Schedule(Seconds(1), &SimpleUdpApplication::SendPacket, udp0,packet1, dest_ip, 7777);
/*while (maxPacket <= 0)
  {
    Simulator::Schedule(Seconds(1), &SimpleUdpApplication::SendPacket, udp0,packet1, dest_ip, 7777);
    maxPacket--; 
  } */
  
  
  
    
  LogComponentEnable ("SimpleUdpApplication", LOG_LEVEL_INFO);

  Simulator::Stop (Seconds (10));
  Simulator::Run ();
  Simulator::Destroy ();

  
 }
