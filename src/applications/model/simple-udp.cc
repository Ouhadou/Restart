#include "ns3/log.h"
#include "simple-udp.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/packet.h"
#include "ns3/tag.h"


//#include "/home/fadoua/NS3/ns-3-allinone/ns-3.35/src/applications/RaptorQ/encoder_decoder.h"
//#include "/home/fadoua/eclipse-workspace/Bachelorarbeit/ns-3.35/scratch/Start/encoder_decoder.h"

#include "/home/fadoua/bake/source/ns-3.35/src/applications/helper/encoder_decoder.h"
#include <stdint.h>

#define PURPLE_CODE "\033[95m"
#define CYAN_CODE "\033[96m"
#define TEAL_CODE "\033[36m"
#define BLUE_CODE "\033[94m"
#define GREEN_CODE "\033[32m"
#define YELLOW_CODE "\033[33m"
#define LIGHT_YELLOW_CODE "\033[93m"
#define RED_CODE "\033[91m"
#define BOLD_CODE "\033[1m"
#define END_CODE "\033[0m"




namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("SimpleUdpApplication");
  NS_OBJECT_ENSURE_REGISTERED(SimpleUdpApplication);

  TypeId
  SimpleUdpApplication::GetTypeId()
  {
    static TypeId tid = TypeId("ns3::SimpleUdpApplication")
                            .AddConstructor<SimpleUdpApplication>()
                            .SetParent<Application>();
    return tid;
  }

  TypeId
  SimpleUdpApplication::GetInstanceTypeId() const
  {
    return SimpleUdpApplication::GetTypeId();
  }

  SimpleUdpApplication::SimpleUdpApplication()
  {
    m_port1 = 7777;

  
  }
  SimpleUdpApplication::~SimpleUdpApplication()
  {
  }
  void SimpleUdpApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
  {
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
    if (socket->Bind(local) == -1)
    {
      NS_FATAL_ERROR("Failed to bind socket");
    }
  }
  
  void SimpleUdpApplication::StartApplication()
  { 
   
    //Receive sockets
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
   
    
    m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
  
    SetupReceiveSocket(m_recv_socket1, m_port1);
    

    m_recv_socket1->SetRecvCallback(MakeCallback(&SimpleUdpApplication::HandleReadOne, this));
    

    //Send Socket
    m_send_socket = Socket::CreateSocket(GetNode(), tid);
    
  }

  void SimpleUdpApplication::HandleReadOne(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet; 
    //Ptr<Packet>p= Create<Packet>();
    Address from;
    Address localAddress;
//	std::cout<<"4\n";
    while ((packet = socket->RecvFrom(from)))
     {
      NS_LOG_INFO(TEAL_CODE << " Received a Packet of size: " << packet->GetSize() << " at time " << Now().GetSeconds() << END_CODE);
       //packet->AddAtEnd(p);
     }


  }

void SimpleUdpApplication::SendPacket( Ptr<Packet> packet, Ipv4Address destination, uint16_t port)   
  { 
    NS_LOG_FUNCTION (this << packet << destination << port);
    m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), port));  

    uint8_t *new_buffer = new uint8_t[packet ->GetSize()]; 
    packet->CopyData(new_buffer, packet->GetSize()); 
    std::vector<uint8_t> vectorBuffer(&new_buffer[0],&new_buffer[packet->GetSize()]);
    /*for (long unsigned int i = 0; i < vectorBuffer.size(); i++) {
            std::cout << vectorBuffer[i] << " ";
        } */
    

    if (!kodierer( vectorBuffer, (uint32_t)packet->GetSize()))
    { 
        std::cout << "failed\n";

    } 
    std::vector<uint8_t>V = decoded_output(); 
    uint8_t *buf = V.data(); 
   /* for (long unsigned int i = 0; i < V.size(); i++) {
            std::cout << V[i] << " ";
        } */
    
    
    Ptr<Packet> packet_i = Create<Packet>(buf, V.size());




    m_send_socket->Send(packet_i);
  
  }

}


