/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Adrian S. Tam
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

#define NS_LOG_APPEND_CONTEXT \
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << " dev " << m_ifIndex << "] "; }

#define __STDC_LIMIT_MACROS 1
#include <stdint.h>
#include "shared-net-device.h"
//#include "md5sum.h"
extern "C" {
#include "hsieh.h"
}
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/object-vector.h"
#include "ns3/pause-header.h"
#include "ns3/infinite-queue.h"
#include "priority-tag.h"
#include "ns3/infinite-simple-red-ecn-queue.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/assert.h"
#include "ns3/ipv4.h"
#include "ns3/simulator.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/random-variable.h"
#include "ns3/flow-id-tag.h"

NS_LOG_COMPONENT_DEFINE ("SharedNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SharedNetDevice);

TypeId 
SharedNetDevice::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::SharedNetDevice")
	 .SetParent<PointToPointNetDevice> ()
	 .AddConstructor<SharedNetDevice> ()
         .AddAttribute ("ServerNode",
                       "Whether the node is switch or server.",
                        BooleanValue (false),
                        MakeBooleanAccessor (&SharedNetDevice::m_servernode),
                        MakeBooleanChecker ())
	 .AddAttribute ("PauseTime",
			"Number of microseconds to pause upon congestion",
			UintegerValue (1),
			MakeUintegerAccessor (&SharedNetDevice::m_pausetime),
			MakeUintegerChecker<uint32_t> ())
	 .AddAttribute ("TxQ",
			"The list of Tx queues for different priority classes",
			ObjectVectorValue (),
			MakeObjectVectorAccessor (&SharedNetDevice::m_queues),
			MakeObjectVectorChecker<Queue> ())
	;
	return tid;
}


SharedNetDevice::SharedNetDevice () :  m_lastQ(qCnt-1)
{
	NS_LOG_FUNCTION (this);

	// Set all the queues used are infinite queues
	for (unsigned i=0; i<qCnt; i++) {
		m_queues.push_back( CreateObject<DropTailQueue>() );
		m_paused[i] = false;
    		Ptr<DropTailQueue> dtqueue = m_queues[i]->GetObject<DropTailQueue>();
        	dtqueue->SetAttributeFailSafe("Mode",      EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
        	dtqueue->SetAttributeFailSafe("MaxBytes",  UintegerValue(4 * 1024 * 1024)); //4 MB typical to accomodate 
		if(i>=0 && i<32) {
			m_pauseT.push_back(Seconds(0.000012));//
		}
		else if(i>=32 && i<64) {
                        m_pauseT.push_back(Seconds(0.000006));
                }
		else if(i>=64 && i<96) {
                        m_pauseT.push_back(Seconds(0.000003));
                }
		else {
			m_pauseT.push_back(Seconds(0.0000));
		}

	};
}

SharedNetDevice::~SharedNetDevice ()
{
	NS_LOG_FUNCTION(this);
	m_queues.clear();
}


int
SharedNetDevice::CheckClass (Ptr<Packet> p)
{
  NS_LOG_FUNCTION(this);
  PriorityTag ptag;
  bool found = p->PeekPacketTag (ptag);
  uint32_t pTag =  (found) ? ptag.GetPriority () : 0;
  uint32_t ecn = pTag & 0xFFF0;
  ecn = ecn >> 8;
//  double pTag = 0;
  return ecn;

}

Ptr<Queue>
SharedNetDevice::GetQueue (void) const
{ 
  NS_LOG_FUNCTION_NOARGS ();
  return sw_queue;
}


void
SharedNetDevice::SetQueue (Ptr<Queue> q)
{
  NS_LOG_FUNCTION (this << q);
  sw_queue = q;
}

void 
SharedNetDevice::DoDispose()
{
	NS_LOG_FUNCTION(this);
	// Cancel all the Qbb events
	for (unsigned i=0; i<qCnt; i++) {
		Simulator::Cancel(m_resumeEvt[i]);
		Simulator::Cancel(m_recheckEvt[i]);
	};
	m_queues.clear();
	PointToPointNetDevice::DoDispose ();
}

void 
SharedNetDevice::TransmitComplete (void)
{
	NS_LOG_FUNCTION(this);
	NS_ASSERT_MSG(m_txMachineState == BUSY, "Must be BUSY if transmitting");
	m_txMachineState = READY;

	NS_ASSERT_MSG (m_currentPkt != 0, "MQQNetDevice::TransmitComplete(): m_currentPkt zero");

	m_phyTxEndTrace (m_currentPkt);
	m_currentPkt = 0;

	DequeueAndTransmit();
}


void
SharedNetDevice::DequeueAndTransmit (void)
{
	NS_LOG_FUNCTION(this);
	// Quit if channel busy
	if (m_txMachineState == BUSY) return;

	if(m_servernode)
	{
	// Look for a packet in a round robin
	unsigned qIndex = 0;
	for (unsigned i=0; i<qCnt; i++) {
		qIndex = i;
		if (m_queues[qIndex]->IsEmpty()) continue;
		if (m_paused[qIndex]) continue;
		Ptr<Packet> p = m_queues[qIndex]->Dequeue();
		if (p != 0) {
			NS_LOG_INFO("Dequeue from queue " << qIndex << ", now has len=" << m_queues[qIndex]->GetNPackets());
			m_snifferTrace (p);
			m_promiscSnifferTrace (p);
                        m_paused[qIndex]=true;
                        m_resumeEvt[qIndex] = Simulator::Schedule(m_pauseT[i],
                                                &SharedNetDevice::PauseFinish, this, qIndex);
			TransmitStart(p);
			m_lastQ = qIndex;
			return;

		};
	};
	}
	else
	{
	 Ptr<Packet> packet = sw_queue->Dequeue ();
         if(packet !=0)
	 {
	  m_snifferTrace (packet);
          m_promiscSnifferTrace (packet);
	  m_node->m_sharedbuffer->RemoveFromBuffer(m_ifIndex, packet->GetSize());
          TransmitStart (packet);
	  return;
	 }
	}
	return;
}

void
SharedNetDevice::Resume (unsigned qIndex)
{
	NS_LOG_FUNCTION(this << qIndex);
	NS_ASSERT_MSG(m_paused[qIndex], "Must be PAUSEd");
	m_paused[qIndex] = false;
	NS_LOG_INFO("Node "<< m_node->GetId() <<" dev "<< m_ifIndex <<" queue "<< qIndex <<
			" resumed at "<< Simulator::Now().GetSeconds());
	DequeueAndTransmit();
}

void
SharedNetDevice::PauseFinish(unsigned qIndex)
{
	// Wrapper to Resume() so that it can be overloaded without
	// interfering with the Scheduler hook
	Resume(qIndex);
}

void
SharedNetDevice::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  PointToPointNetDevice::Receive(packet);

}

bool
SharedNetDevice::Send(Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber)
{ 
   
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("p=" << packet << ", dest=" << &dest);
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  //
  // If IsLinkUp() is false it means there is no channel to send any packet 
  // over so we just hit the drop trace on the packet and return an error.
  //
  if (IsLinkUp () == false)
    {
      m_macTxDropTrace (packet);
      return false;
    }

  //
  // Stick a point to point protocol header on the packet in preparation for
  // shoving it out the door.
  //
  AddHeader (packet, protocolNumber);
  m_macTxTrace (packet);

  //
  // If there's a transmission in progress, we enque the packet for later
  // transmission; otherwise we send it now.
  //
   if(!m_servernode)
   {
      // 
      // Even if the transmitter is immediately available, we still enqueue and
      // dequeue the packet to hit the tracing hooks.
      //
      if(m_node->m_sharedbuffer->CheckBufferforAdmission(m_ifIndex, packet->GetSize()))
      {
      			m_node->m_sharedbuffer->UpdatebufferforAdmission(m_ifIndex, packet->GetSize());
			sw_queue->Enqueue (packet);
        
      }
      else
      {
          		// Enqueue may fail (overflow)
          		m_macTxDropTrace (packet);
          		return false;
      }
	                DequeueAndTransmit();

    }
   else
   {
    unsigned qIndex = CheckClass(packet);
    m_queues[qIndex]->Enqueue(packet);
    DequeueAndTransmit();

   }

   return true;
}

} // namespace ns3
