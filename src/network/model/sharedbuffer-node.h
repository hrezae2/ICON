#ifndef SHAREDBUFFER_NODE_H
#define SHAREDBUFFER_NODE_H

#include <vector>

#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/event-id.h"

namespace ns3 {

	class Application;
	class Packet;
	class Address;


	/**
	 * \ingroup network
	 *
	 * \brief A network Node.
	 *
	 * This class holds together:
	 *   - a list of NetDevice objects which represent the network interfaces
	 *     of this node which are connected to other Node instances through
	 *     Channel instances.
	 *   - a list of Application objects which represent the userspace
	 *     traffic generation applications which interact with the Node
	 *     through the Socket API.
	 *   - a node Id: a unique per-node identifier.
	 *   - a system Id: a unique Id used for parallel simulations.
	 *
	 * Every Node created is added to the NodeList automatically.
	 */
	class SharedBufferNode : public Object
	{
	public:

		static const unsigned pCnt = 48;	// Number of ports used

		static TypeId GetTypeId(void);

		SharedBufferNode();
		/**
		 * \param systemId a unique integer used for parallel simulations.
		 */
 		virtual void DoDispose(void);
		virtual ~SharedBufferNode();


		bool CheckBufferforAdmission(uint32_t port, uint32_t psize);
		void UpdatebufferforAdmission(uint32_t port, uint32_t psize);
		void RemoveFromBuffer(uint32_t port, uint32_t psize);
		void ResetFlags(uint32_t port);

		uint32_t GetUsedBufferTotal();

	private:

		uint32_t m_usedTotalBytes;
		uint32_t m_maxBufferBytes;

		uint32_t m_usedPortBytes[pCnt];
		bool m_usedPortFlags[pCnt];
		bool m_scheduleddynamicth;
		//dynamic threshold
		double m_port_control_threshold;
		double m_alpha;
		// time to calculate the threshold
		double m_time_step;
		void CalculateControlThreshold(unsigned junk);

		EventId m_unactiveEvent[pCnt]; //< Unactive Queues Event

		EventId m_resumeEvt; //< Keeping the next resume event
	};

} // namespace ns3

#endif /* SHAREDBUFFER_NODE_H */
