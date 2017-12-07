/****************************
*
* Some description
*
* Author: ekirshey
* Created: Mon Nov 27 22:14:32 2017
****************************/
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include "third_party/INIReader.h"
#include "Network.h"
#include "BlockingQueue.h"
#include "Simulation.h"

Network::Packet process_message(int id, std::vector<char>& data) {
	Network::Packet packet;
	packet.clientid = id;
	packet.data = data;
	return packet;
}

void worker( Containers::BlockingQueue<Network::Packet>& messages, Containers::LockingQueue<Game::Event>& events) 
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

void sender(Containers::LockingQueue<Network::Packet>& messages, Network::Server& server)
{
	while (true) try {
		Network::Packet p;
		if (messages.dequeue(p) > 0) {
			server.SendMessages(p);
		}
	}
	catch (std::exception const& e) {
		std::cout << "Worker got " << e.what() << "\n";
		break;
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Must provide an ini file" << std::endl;
	}
	Network::Config config;
	std::string ini = argv[1];

	INIReader reader(ini);

	if (reader.ParseError() < 0) {
		std::cerr << "Could not load file " << ini << std::endl;
		throw std::runtime_error("Could not load file " + ini);
	}

	config.ip = reader.Get("network", "ip", "default");
	config.port = reader.GetInteger("network", "port", 25977);

	Containers::BlockingQueue<Network::Packet> msg_queue;
	Containers::LockingQueue<Game::Event> event_queue;
	Containers::LockingQueue<Network::Packet> send_queue;

	auto msg_handler = [&](int id, std::vector<char>& data) -> void {
		msg_queue.enqueue(process_message(id, data));
	};

	asio::io_service io_service;
	Network::Server server(config, io_service, msg_handler);

	auto msg_proc = std::thread([&msg_queue, &event_queue] {worker(msg_queue, event_queue); });
	auto game_loop = std::thread([&event_queue, &send_queue] {Game::Simulate(event_queue, send_queue); });
	auto msg_sender = std::thread([&send_queue, &server] {sender(send_queue, server); });

	io_service.run();

	msg_proc.join();

	system("PAUSE");
}