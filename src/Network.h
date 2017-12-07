#pragma once
#include <iostream>
#include "asio.hpp"

namespace Network {
	struct Config {
		std::string ip;
		unsigned short port;
	};

	struct Packet {
		int clientid;
		std::vector<char> data;
	};

	using packet_handler = std::function<void(int, std::vector<char>&)>;

	class Connection : public std::enable_shared_from_this<Connection> {
	public:
		Connection(unsigned int id, asio::ip::tcp::socket&& s, packet_handler handler) 
			: _id(id)
			, _socket(std::move(s)) 
			, _message_body(1024)
			, _handler(handler)
		{
		}

		/* When using async_read I was telling tcp to wait until it reads
		 1024 packets.... that obviously wasnt happening. read_some seems to 
		 read whatever comes in. Still cant get it to read with a streambuf
		 object. there must be some sort of other flag to tell it when to
		 try to read
		 */
		void Connection::Process() {
			read_message_header();
		}

		void WriteMessage(Network::Packet& packet) {
			auto self(shared_from_this());
			asio::async_write(_socket,
				asio::buffer(packet.data.data(),
					packet.data.size()),
				[this, self, packet](std::error_code ec, std::size_t /*length*/)
			{
				if (!ec) {

				}
			});
		}

	private:
		void read_message_header() {
			auto self(shared_from_this());
			asio::async_read(_socket,
				asio::buffer(&_header, 4),
				asio::transfer_exactly(4),
				[this, self](std::error_code ec, size_t bytes_received) {
				if (!ec) {
					std::cout << "Header: " << _header << std::endl;
					_body_length = _header;
					read_message_body();
				}
			});
		}
		void read_message_body() {
			auto self(shared_from_this());
			asio::async_read(_socket,
				asio::buffer(_message_body),
				asio::transfer_exactly(_body_length),
				[this, self](std::error_code ec, size_t bytes_received) {
				if (!ec) {
					std::cout.write(_message_body.data(), _body_length) << std::endl;
					_handler(_id, _message_body);
				}

				read_message_header();
			});
		}

		unsigned int _id;
		asio::ip::tcp::socket _socket;
		int _header;
		int _body_length;
		std::vector<char> _message_body;
		packet_handler _handler;
	};

	class Server
	{
	public:
		Server(const Config& config, asio::io_service& io_service, packet_handler handler)
			: _config(config)
			, _io_service(io_service)
			, _acceptor(io_service, asio::ip::tcp::endpoint( asio::ip::tcp::v4(), config.port ))
			, _socket(io_service)
			, _handler(handler)
		{
			//_acceptor.set_option(asio::ip::tcp::no_delay(true));
			_acceptor.listen();
			_do_accept();
		}

		void SendMessages(Network::Packet& packet) {
			_io_service.post(std::bind(&Connection::WriteMessage, _connections[packet.clientid].get(), packet));
		}

	private:
		void _do_accept() {
			_acceptor.async_accept(_socket, [this](std::error_code ec) {
				if (!ec) {
					_socket.set_option(asio::ip::tcp::no_delay(true)); // disable nagles
					_connections.push_back(std::move(std::make_shared<Connection>(_connections.size(), std::move(_socket), _handler)));
					_connections.back()->Process();
				}
				_do_accept();
			});
		}

		Config _config;
		asio::io_service& _io_service;
		asio::ip::tcp::acceptor _acceptor;
		asio::ip::tcp::socket _socket;
		packet_handler _handler;
		std::vector<std::shared_ptr<Connection>> _connections;
	};
}

