#pragma once
#include "LockingQueue.h"
#include "BlockingQueue.h"
#include "Network.h"
#include "Simulation.h"

namespace Services {
	void PacketParser(Containers::BlockingQueue<Network::Packet>& messages, Containers::LockingQueue<Game::Event>& events)
	{
		while (true) try {
			auto p = messages.dequeue();
			(std::cout << "Worker handling request '").write(p.data.data(), p.data.size()) << "'\n";
			Game::Event ev;
			ev.clientid = p.clientid;
			events.enqueue(ev);
		}
		catch (std::exception const& e) {
			std::cout << "Worker got " << e.what() << "\n";
			break;
		}
	}

	// Not sure if this is needed...
	void PacketSender(Containers::LockingQueue<Game::ClientMessage>& messages, Network::Server& server)
	{
		while (true) try {
			Game::ClientMessage cm;
			if (messages.dequeue(cm) > 0) {
				Network::RawPacket p(cm.data.size() + 4);
				auto size = cm.data.size();
				std::memcpy(&p[0], &size, 4);
				std::memcpy(&p[4], cm.data.data(), size);
				server.SendMessages(cm.clientid, p);
			}
		}
		catch (std::exception const& e) {
			std::cout << "Worker got " << e.what() << "\n";
			break;
		}
	}
}