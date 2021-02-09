#include <iostream>
#include <chrono>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

std::vector<char> vBuffer(20 * 1024);

void grabData(asio::ip::tcp::socket& socket) {
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()), 
		[&](std::error_code ec, std::size_t length) {
			if (!ec) {
				std::cout << "\n\nRead " << length << " bytes\n\n";

				for (int i = 0; i < length; i++) {
					std::cout << vBuffer[i];
				}

				grabData(socket);
			}
		}
	);
}


int main() {

	asio::error_code ec;

	// platform specifc context
	asio::io_context context;
	asio::io_context::work idleWork(context);
	std::thread thrContext = std::thread([&]() {context.run(); });

	// HTTP port 80, typical port for connection
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

	asio::ip::tcp::socket socket(context);

	socket.connect(endpoint, ec);

	if (!ec) {
		std::cout << "Connected!" << std::endl;
	}
	else {
		std::cout << "Failed to connect to address:\n" << ec.message() << std::endl;
	}

	if (socket.is_open()) {

		grabData(socket);

		std::string sRequest = "GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);



		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20000ms);

		context.stop();
		if (thrContext.joinable()) thrContext.join();
	}


	system("pause");

	return 0;
}