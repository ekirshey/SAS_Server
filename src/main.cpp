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
#include "Services.h"

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
	Containers::LockingQueue<Game::ClientMessage> send_queue;

	auto msg_handler = [&](int id, std::vector<char>& data) -> void {
		msg_queue.enqueue(Network::BuildPacket(id, data));
	};

	asio::io_service io_service;
	Network::Server server(config, io_service, msg_handler);
	auto msg_proc = std::thread([&msg_queue, &event_queue] {Services::PacketParser(msg_queue, event_queue); });
	auto game_loop = std::thread([&event_queue, &send_queue] {Game::Simulate(event_queue, send_queue); });
	auto msg_sender = std::thread([&send_queue, &server] {Services::PacketSender(send_queue, server); });

	auto io = std::thread([&]() {io_service.run(); });

	msg_proc.join();
	game_loop.join();
	msg_sender.join();
	io.join();

	system("PAUSE");
}