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

			}

			void messageClient(std::shared_ptr<connection<T>> client, const message<T>& msg) {

			}

			void messageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> ignore = nullptr) {

			}

		protected:

			virtual bool clientConnect(std::shared_ptr<connection<T>> client) {
				return false;
			}

			virtual bool clientDisconnect(std::shared_ptr<connection<T>> client) {

			}

			virtual void clientMessage(std::shared_ptr<connection<T>> client, message<T>& msg) {

			}

		protected:

			tsqueue<owned_message<T>> m_messagesIn;

			asio::io_context m_asioContext;
			std::thread m_threadContext;

			asio::ip::tcp::acceptor m_asioAcceptor;

			uint32_t IDCounter = 10000;
		};
	}
}


// PART 2 13:33 ----------------