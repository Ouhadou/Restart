#ifndef NS3_UDP_ARQ_APPLICATION_H
#define NS3_UDP_ARQ_APPLICATION_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "ns3/packet.h"


using namespace ns3;


namespace ns3
{
  class SimpleUdpApplication : public Application 
  {
    public:
      SimpleUdpApplication ();
      virtual ~SimpleUdpApplication ();

      static TypeId GetTypeId ();
      virtual TypeId GetInstanceTypeId () const;

      void HandleReadOne (Ptr<Socket> socket);
      
      
      void SendPacket ( Ptr<Packet> packet, Ipv4Address destination, uint16_t port);
    
   
   
   
    private:
      
      
      void SetupReceiveSocket (Ptr<Socket> socket, uint16_t port);
      virtual void StartApplication ();
      


      Ptr<Socket> m_recv_socket1;/**< A socket to receive on a specific port */
      uint16_t m_port1;
      uint32_t       m_nPackets= 4;

   
      
      Ptr<Socket> m_send_socket; /**< A socket to listen on a specific port */
  };
}


#endif