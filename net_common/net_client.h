#pragma once

#include "net_common.h"

namespace icd {
	namespace net {

		template<typename T>
		class client_interface {
		public:
			client_interface() {}

			virtual ~client_interface() {
				disconnect();
			}

		public:
			bool connect(const std::string& host, const uint16_t port) {

				try {

					asio::ip::tcp::resolver resolver(m_context);
					asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

					// create connection
					m_connection = std::make_unique<connection<T>>(connection<T>::owner::client, 
						m_context, asio::ip::tcp::socket(m_context), m_messagesIn);

					m_connection->connectToServer(endpoints);

					thrContext = std::thread([this]() { m_context.run(); });


				}
				catch (std::exception& e) {
					std::cerr << "Client exception: " << e.what() << "\n";
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

				m_connection.release();
			}

			bool isConnected() {
				if (m_connection) {
					return m_connection->isConnected();
				}
				else
					return false;
			}

		public:

			void send(const message<T>& msg) {
				if (isConnected()) {
					m_connection->send(msg);
				}
			}

			tsqueue<owned_message<T>>& incoming() {
				return m_messagesIn;
			}

		protected:
			asio::io_context m_context;
			std::thread thrContext;
			std::unique_ptr<connection<T>> m_connection;

		private:
			tsqueue<owned_message<T>> m_messagesIn;
		};
	}
}
