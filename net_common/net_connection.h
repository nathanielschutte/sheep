#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"

namespace icd {
	namespace net {

		template<typename T>
		class connection : public std::enable_shared_from_this<connection<T>> {
		public:

			enum class owner {
				server,
				client
			};

			connection(owner parent, asio::io_context& asioContext, 
				asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn) : m_asioContext(asioContext),
				m_socket(std::move(socket)), m_messagesIn(qIn) {
			
				m_ownerType = parent;
			}

			virtual ~connection() {}

			uint32_t getID() const {
				return id;
			}


		public:
			void connectToClient(uint32_t uid = 0) {
				if (m_ownerType == owner::server) {
					if (m_socket.is_open()) {
						id = uid;
						readHeader();
					}
				}
			}

			void connectToServer(const asio::ip::tcp::resolver::results_type& endpoints) {
				if (m_ownerType == owner::client) {
					asio::async_connect(m_socket, endpoints,
						[this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
							if (!ec) {
								readHeader();
							}
						});
				}
			}

			void disconnect() {
				if (isConnected()) {
					asio::post(m_asioContext, [this]() {m_socket.close(); });
				}
			}

			bool isConnected() const {
				return m_socket.is_open();
			}

		public:
			void send(const message<T>& msg) {
				asio::post(m_asioContext,
					[this, msg]() {
						bool writingMessage = !m_messagesOut.empty();
						m_messagesOut.push_back(msg);
						if (!writingMessage) {
							writeHeader();
						}
					});
			}


		private:

			// ASYNC
			void readHeader() {
				asio::async_read(m_socket, asio::buffer(&m_msgTempIn.header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length) {
						if (!ec) {
							if (m_msgTempIn.header.size > 0) {
								m_msgTempIn.body.resize(m_msgTempIn.header.size);
								readBody();
							}
							else {
								addToIncomingMessageQueue();
							}
						}
						else {
							std::cout << "[" << id << "] Read header fail\n";
							m_socket.close();
						}
					});
			}

			void readBody() {
				asio::async_read(m_socket, asio::buffer(m_msgTempIn.body.data(), m_msgTempIn.body.size()),
					[this](std::error_code ec, std::size_t length) {
						if (!ec) {
							addToIncomingMessageQueue();
						}
						else {
							std::cout << "[" << id << "] Read body fail\n";
							m_socket.close();
						}
					});
			}

			void writeHeader() {
				asio::async_write(m_socket, asio::buffer(&m_messagesOut.front().header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length) {
						if (!ec) {
							if (m_messagesOut.front().body.size() > 0) {
								writeBody();
							}
							else {
								m_messagesOut.pop_front();

								if (!m_messagesOut.empty()) {
									writeHeader();
								}
							}
						}
						else {
							std::cout << "[" << id << "] Write header fail\n";
							m_socket.close();
						}
				});
			}

			void writeBody() {
				asio::async_write(m_socket, asio::buffer(&m_messagesOut.front().header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length) {
						if (!ec) {
							m_messagesOut.pop_front();

							if (!m_messagesOut.empty()) {
								writeHeader();
							}
						}
						else {
							std::cout << "[" << id << "] Write body fail\n";
							m_socket.close();
						}
					});
			}


			void addToIncomingMessageQueue() {
				if (m_ownerType == owner::server) {
					m_messagesIn.push_back({ this->shared_from_this(), m_msgTempIn });
				}
				else {
					m_messagesIn.push_back({ nullptr, m_msgTempIn });
				}

				readHeader();
			}

		protected:
			asio::ip::tcp::socket m_socket;

			asio::io_context& m_asioContext;

			tsqueue<message<T>> m_messagesOut;
			tsqueue<owned_message<T>>& m_messagesIn;

			message<T> m_msgTempIn;

			owner m_ownerType = owner::server;
			uint32_t id = 0;
		};
	}
}