/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "priority-tag.h"
#include "simple-priority-ecn-queue.h"

NS_LOG_COMPONENT_DEFINE ("SimplePriorityEcnQueue");

using namespace std;
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SimplePriorityEcnQueue);

TypeId SimplePriorityEcnQueue::GetTypeId (void) 
{
  static TypeId tid = TypeId ("ns3::SimplePriorityEcnQueue")
    .SetParent<Queue> ()
    .AddConstructor<SimplePriorityEcnQueue> ()
    .AddAttribute ("Mode", 
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&SimplePriorityEcnQueue::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets accepted by this DropTailQueue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&SimplePriorityEcnQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes", 
                   "The maximum number of bytes accepted by this DropTailQueue.",
                   UintegerValue (100 * 65535),
                   MakeUintegerAccessor (&SimplePriorityEcnQueue::m_maxBytes),
		   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Th",
                   "Instantaneous mark length threshold in packets/bytes",
                   UintegerValue (5),
                   MakeUintegerAccessor (&SimplePriorityEcnQueue::m_th),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}

SimplePriorityEcnQueue::SimplePriorityEcnQueue () :
  Queue (),
  m_Q1packets(),
  m_Q2packets(),
  m_bytesInQueue (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

SimplePriorityEcnQueue::~SimplePriorityEcnQueue ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
SimplePriorityEcnQueue::SetMode (SimplePriorityEcnQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

SimplePriorityEcnQueue::QueueMode
SimplePriorityEcnQueue::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

bool 
SimplePriorityEcnQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p << "bytes: " << m_bytesInQueue + p->GetSize ());

  int priority = CheckPriority(p);
  if(priority == 1)
   {
  	if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () < m_maxBytes))
    	{
                m_bytesInQueue += p->GetSize ();
                m_Q1packets.push_back (p);

                NS_LOG_LOGIC ("Number packets " << m_Q1packets.size ());
                NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
        	return true;
	}
	else
	{
		while(m_bytesInQueue + p->GetSize () > m_maxBytes && (!m_Q2packets.empty()))
		{
			Ptr<Packet> p = m_Q2packets.back ();
			m_Q2packets.pop_back();
        		m_bytesInQueue -= p->GetSize ();
		}
        	if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () < m_maxBytes))
        	{
                	m_bytesInQueue += p->GetSize ();
                	m_Q1packets.push_back (p);
	
        	        NS_LOG_LOGIC ("Number packets " << m_Q1packets.size ());
               		NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
                	return true;
        	}
		NS_LOG_LOGIC ("Queue full (packet would exceed max bytes) -- dropping pkt");
                Drop (p);
		return false;	
	}
   }
				//Enqueue in higher priority
  if(priority == 0)	//Verify this logic
  {
	
  	if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_th))
    	{
      		NS_LOG_LOGIC ("Queue (" << (m_bytesInQueue + p->GetSize ()) << ") above threshold (" << m_th << ") -- marking pkt");
      		Mark (p);
    	}

  	if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes))
    	{
      		NS_LOG_LOGIC ("Queue full (packet would exceed max bytes) -- dropping pkt");
      		Drop (p);
      		return false;
    	}

  	m_bytesInQueue += p->GetSize ();
  	m_Q2packets.push_back (p);

  	NS_LOG_LOGIC ("Number packets " << m_Q2packets.size ());
  	NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
	return true;
  }
   return false;
}

int
SimplePriorityEcnQueue::CheckPriority (Ptr<Packet> p)
{
  NS_LOG_FUNCTION(this);
  PriorityTag ptag;
  bool found = p->PeekPacketTag (ptag);
  uint32_t pTag =  (found) ? ptag.GetPriority () : 0;
  pTag = pTag & 0x000F;
  if(pTag == 3)
        return 1;
  PacketMetadata::ItemIterator metadataIterator = p->BeginItem();
  PacketMetadata::Item item;
  while (metadataIterator.HasNext())
    {
      item = metadataIterator.Next();
      NS_LOG_DEBUG(this << "item name: " << item.tid.GetName());
      if(item.tid.GetName() == "ns3::Ipv4Header")
        {
          NS_LOG_DEBUG("IP packet found");
          Buffer::Iterator i = item.current;
          i.Next (1); // Move to TOS byte
          uint8_t tos = i.ReadU8 ();
          uint8_t ecn = tos & 0x3;
          if(ecn == 3 && pTag == 2)
           {
                return 1;
           }
	  else
	   {
		return 0;
	   }
          break;
        }
    }
   return 0;
}

void
SimplePriorityEcnQueue::Mark (Ptr<Packet> p)
{
  NS_LOG_FUNCTION(this);
  PacketMetadata::ItemIterator metadataIterator = p->BeginItem();
  PacketMetadata::Item item;
  while (metadataIterator.HasNext())
    {
      item = metadataIterator.Next();
      NS_LOG_DEBUG(this << "item name: " << item.tid.GetName());
      if(item.tid.GetName() == "ns3::Ipv4Header")
        {
          NS_LOG_DEBUG("IP packet found");
          // This is a dirty hack to access the ECN byte: directly read/write from the buffer
          Buffer::Iterator i = item.current;
          i.Next (1); // Move to TOS byte
          uint8_t tos = i.ReadU8 ();
          uint8_t ecn = tos & 0x3;
          uint8_t ecncount = tos & 0x1C;
          ecncount = ecncount >> 2;
	  if (ecn == 1 || ecn == 2 || ecn == 3)
            {
              ecncount++;
              ecncount = ecncount << 2;
              tos |= 0xF;
              tos &= ecncount;
              tos |= 0x3;
              i.Prev (1); // Go back to the TOS byte after ReadU8()
              i.WriteU8 (tos);
            }
           if ((tos & 0x3) == 0x3)
            {
              NS_LOG_LOGIC ("Marking IP packet");
            }
          else
            {
              NS_LOG_LOGIC ("Unable to mark packet");
            }
          break;
        }
    }
}

Ptr<Packet>
SimplePriorityEcnQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);
  if (m_Q1packets.empty () && m_Q2packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  if(m_Q1packets.size () > 0)
    {
  	Ptr<Packet> p = m_Q1packets.front ();
    	m_Q1packets.pop_front ();
	m_bytesInQueue -= p->GetSize ();
	NS_LOG_LOGIC ("Popped " << p);
  	NS_LOG_LOGIC ("Number packets " << m_Q1packets.size ());
	NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
    	return p;
    }
  else 
    {
  	Ptr<Packet> p = m_Q2packets.front ();
  	m_Q2packets.pop_front ();
  	m_bytesInQueue -= p->GetSize ();
  	NS_LOG_LOGIC ("Popped " << p);
  	NS_LOG_LOGIC ("Number packets " << m_Q2packets.size ());
  	NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
  	return p;
     }
}

Ptr<const Packet>
SimplePriorityEcnQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_Q1packets.empty () && m_Q2packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }
  
  if(m_Q1packets.size () > 0)
    {
        Ptr<Packet> p = m_Q1packets.front ();
  
	NS_LOG_LOGIC ("Number packets " << m_Q1packets.size ());
  	NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  	return p;
    }
  else
    {
        Ptr<Packet> p = m_Q2packets.front ();

  	NS_LOG_LOGIC ("Number packets " << m_Q2packets.size ());
  	NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  	return p;
    }

}
int
SimplePriorityEcnQueue::Getqueuesize(){
return m_bytesInQueue;
}

} // namespace ns3

