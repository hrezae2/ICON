#include <iostream>
#include <fstream>
#include "node.h"
#include "node-list.h"
#include "net-device.h"
#include "application.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/object-vector.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/global-value.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/sharedbuffer-node.h"
#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE("SharedBufferNode");

namespace ns3 {

	NS_OBJECT_ENSURE_REGISTERED(SharedBufferNode);

	TypeId
		SharedBufferNode::GetTypeId(void)
	{
		static TypeId tid = TypeId("ns3::SharedBufferNode")
			.SetParent<Object>()
			.AddConstructor<SharedBufferNode>();
		return tid;
	}


	SharedBufferNode::SharedBufferNode()
	{
		m_maxBufferBytes = 9000000; //9MB
		m_usedTotalBytes = 0;
		m_port_control_threshold = (2*m_maxBufferBytes); 
		for (uint32_t i = 0; i < pCnt; i++)
		{
			m_usedPortBytes[i] = 0;
			m_usedPortFlags[i] = false;
		}
		m_time_step = 0.001;
		m_alpha = 2;
		m_scheduleddynamicth = false;
	}

	SharedBufferNode::~SharedBufferNode()
	{}

	bool
		SharedBufferNode::CheckBufferforAdmission(uint32_t port, uint32_t psize)
	{
		if( m_usedTotalBytes + psize > m_maxBufferBytes ) //Switch limit exceeds
		{
			return false;
		}
		if(m_usedPortBytes[port] + psize > m_port_control_threshold) //Control threshold reached
		{
			return false;
		}
		return true;
	}


        void
                SharedBufferNode::DoDispose()
        {
                Simulator::Cancel(m_resumeEvt);
		for(uint32_t i=0; i< pCnt; i++)
		{
			Simulator::Cancel(m_unactiveEvent[i]);	
		}
        }

	void 
		SharedBufferNode::ResetFlags(uint32_t port)
	{
		m_usedPortFlags[port] = false;
	}
	void
		SharedBufferNode::UpdatebufferforAdmission(uint32_t port, uint32_t psize)
	{
		m_usedPortBytes[port] += psize;
		m_usedTotalBytes += psize;
		m_usedPortFlags[port] = true;
		if(!m_scheduleddynamicth)
		{
			m_scheduleddynamicth = true;
			m_resumeEvt = Simulator::Schedule(Seconds(m_time_step), &SharedBufferNode::CalculateControlThreshold, this, 0);
		}
		m_unactiveEvent[port].Cancel ();
		m_unactiveEvent[port] = Simulator::Schedule (Seconds(m_time_step), &SharedBufferNode::ResetFlags, this, port);
		return;
	}
	
	void
		SharedBufferNode::RemoveFromBuffer(uint32_t port, uint32_t psize)
	{
		m_usedPortBytes[port] -= psize;
                m_usedTotalBytes -= psize;
		return;
	}

        void
                SharedBufferNode::CalculateControlThreshold(unsigned junk)
	{
		uint32_t activePorts = 0;
		uint32_t unactivePorts = 0;

		uint32_t unactiveQueuesLength = 0;
		for (uint32_t i = 0; i < pCnt; i++)
                {
                        if(m_usedPortFlags[i])
			{
				activePorts++;
			}
			else
			{
				unactivePorts++;
				unactiveQueuesLength += m_usedPortBytes[i];
			}
                }
		m_port_control_threshold = m_alpha * (m_maxBufferBytes - unactiveQueuesLength )/ ( 1 + activePorts * m_alpha);	
		if(unactivePorts == pCnt)
		{
			m_scheduleddynamicth = false;	
		}
		else
		{
			m_resumeEvt = Simulator::Schedule(Seconds(m_time_step), &SharedBufferNode::CalculateControlThreshold, this, 0);
		}
	}
	uint32_t
		SharedBufferNode::GetUsedBufferTotal()
	{
		return m_usedTotalBytes;
	}
} // namespace ns3
