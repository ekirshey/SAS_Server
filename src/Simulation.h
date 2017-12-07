#pragma once
#include <vector>
#include "LockingQueue.h"

namespace Game {
	struct Event {
		int clientid;
	};

	void Simulate(Containers::LockingQueue<Event>& events, Containers::LockingQueue<Network::Packet>& outmessages) {
		std::vector<Event> v_ev;
		while (true) {
			Event ev;
			v_ev.clear();
			while (events.dequeue(ev) > 0) {
				v_ev.push_back(ev);
				std::cout << ev.clientid << std::endl;
			}

			for (auto msg : v_ev) {
				Network::Packet p;
				p.clientid = msg.clientid;
				p.data.push_back(10);
				outmessages.enqueue(p);
			}
		}
	}
}
