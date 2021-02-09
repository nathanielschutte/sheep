#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"

namespace icd {
	namespace net {
		template<typename T>
		class client_interface {

			client_interface() : m_socket(m_context) {

			}

			virtual ~client_interface() {
				disconnect();
			}

		public:
			bool connect(const std::string& host, const uint16_t port) {

				try {
					// create connection
					m_connection = std::make_unique<connection<T>>();

					asio::ip::tcp::resolver resolver(m_context);
					m_endpoints = resolver.resolve(host, std::to_string(port));

					m_connection->connectToServer(m_endpoints);

					thrContext = std::thread([this]() { m_context.run(); });


				}
				catch (std::exception& e) {
					std::cerr << "Client Exception: " << e.what() << "\n";
					return false;
				}

				return true;
			}

			void disconnect() {
				if (isConnected()) {
					m_connection->disconnect();
				}

				m_context.stop();

				if (thrContext.joinable())
					thrContext.join();
			}

			bool isConnected() {
				if (m_connection) {
					return m_connection->isConnected();
				else
					return false;
				}
			}

			tsqueue<owned_message<T>>& incoming() {
				return m_messagesIn;
			}

		protected:
			asio::io_context m_context;
			std::thread thrContext;
			asio::ip::tcp::socket m_socket;
			std::unique_ptr<connection<T>> m_connection;

		private:
			tsqueue<owned_message<T>> m_messagesIn;
		};
	}
}
