#pragma once
#include <vector>
#include "LockingQueue.h"
#include <chrono>
#include <ctime>
#include <unordered_set>

namespace Game {
	struct Event {
		int clientid;
	};
	
	struct ClientMessage {
		int clientid;
		std::vector<char> data;
	};

	void Simulate(Containers::LockingQueue<Event>& events, Containers::LockingQueue<ClientMessage>& outmessages) {
		std::unordered_set<int> clients;
		std::vector<Event> v_ev;
		auto start = std::chrono::system_clock::now();
		while (true) {
			Event ev;
			v_ev.clear();
			while (events.dequeue(ev) > 0) {
				v_ev.push_back(ev);
				clients.insert(ev.clientid);
				std::cout << ev.clientid << std::endl;
			}

			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			if (elapsed_seconds.count() >= 0.33) {
				if (clients.size() > 0) {
					ClientMessage p;
					auto c = clients.begin();
					p.clientid = *c;
					p.data.push_back('h');
					p.data.push_back('e');
					p.data.push_back('l');
					p.data.push_back('l');
					p.data.push_back('o');
					p.data.push_back('\0');
					outmessages.enqueue(p);
				}
				start = end;
			}
		}
	}
}
