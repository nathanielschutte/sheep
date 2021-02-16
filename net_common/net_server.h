#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"

namespace icd {
	namespace net {

		template<typename T>
		class server_interface {
		public:
			server_interface(uint16_t port) : 
				m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {


			}

			virtual ~server_interface() {
				stop();
			}

			bool start() {

				try {
					waitForClientConnection();

					m_threadContext = std::thread([this]() { m_asioContext.run(); });
				}
				catch (std::exception& e) {
					std::cerr << "[SERVER] Exception: " << e.what() << "\n";
					return false;
				}

				std::cout << "[SERVER] Started\n";
				return true;
			}

			void stop() {
				m_asioContext.stop();

				if (m_threadContext.joinable()) m_threadContext.join();

				std::cout << "[SERVER] Stopped\n";
			}

			// ASYNC
			void waitForClientConnection() {
				m_asioAcceptor.async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket) {
						if (!ec) {
							std::cout << "[SERVER] New conection: " << socket.remote_endpoint() << "\n";

							std::shared_ptr<connection<T>> new_conn = std::make_shared<connection<T>>(connection<T>::owner::server,
								m_asioContext, std::move(socket), m_messagesIn);

							if (clientConnect(new_conn)) {

								m_deqConns.push_back(std::move(new_conn));
								m_deqConns.back()->connectToClient(IDCounter++);
								std::cout << "[" << m_deqConns.back()->getID() << "] Connection approved\n";

							}
							else {
								std::cout << "[-----] Connection denied\n";
							}
						}
						else {
							std::cout << "[SERVER] New connection error: " << ec.message() << "\n";
						}

						// re-prime asio context for next connection
						waitForClientConnection();
					});
			}

			void messageClient(std::shared_ptr<connection<T>> client, const message<T>& msg) {

				if (client && client->isConnected()) {
					client->send(msg);
				}

				// assume client should be disconnected
				else {
					clientDisconnect(client);
					client.reset();
					m_deqConns.erase(
						std::remove(m_deqConns.begin(), m_deqConns.end(), client), m_deqConns);
				}
			}

			void messageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> ignore = nullptr) {
				bool bInvalidClient = false;
				
				for (auto& client : m_deqConns) {
					if (client && client->isConnected()) {
						if (client != ignore)
							client->send(msg);
					}

					else {
						clientDisconnect(client);
						client.reset();
						bInvalidClient = true;
					}
				}

				if (bInvalidClient)
					m_deqConns.erase(
						std::remove(m_deqConns.begin(), m_deqConns.end(), nullptr), m_deqConns);
			}

			void update(size_t nMaxMessages = -1) {

				if (q_size != m_messagesIn.count()) {
					std::cout << "QUEUE SIZE: " << m_messagesIn.count() << "\n";
					q_size = m_messagesIn.count();
				}

				size_t nMessageCount = 0;
				while (nMessageCount < nMaxMessages && !m_messagesIn.empty()) {
					auto msg = m_messagesIn.pop_front();

					clientMessage(msg.remote, msg.msg);

					nMessageCount++;
				}
			}

		protected:

			virtual bool clientConnect(std::shared_ptr<connection<T>> client) {
				return false;
			}

			virtual void clientDisconnect(std::shared_ptr<connection<T>> client) {

			}

			virtual void clientMessage(std::shared_ptr<connection<T>> client, message<T>& msg) {
				
			}

		protected:

			tsqueue<owned_message<T>> m_messagesIn;

			// contains shared pointers to user connections
			std::deque<std::shared_ptr<connection<T>>> m_deqConns;

			asio::io_context m_asioContext;
			std::thread m_threadContext;

			asio::ip::tcp::acceptor m_asioAcceptor;

			uint32_t IDCounter = 10000;

			size_t q_size = 0;
		};
	}
}