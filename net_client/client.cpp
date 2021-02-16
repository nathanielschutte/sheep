#include <iostream>
#include <icd_net.h>


const size_t KEY_C = 3;
const char KEY_QUIT = '3';
const char KEY_PING = '1';

enum class MessageTypes : uint32_t {
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class SampleClient : public icd::net::client_interface<MessageTypes> {
public:
	void pingServer() {
		icd::net::message<MessageTypes> msg;
		msg.header.id = MessageTypes::ServerPing;

		std::chrono::system_clock::time_point time_ping = std::chrono::system_clock::now();

		msg << time_ping;

		std::cout << msg << "\n";

		send(msg);
	}
};



int main() {

	SampleClient c;
	c.connect("127.0.0.1", 60000);

	bool key[KEY_C];
	bool old_key[KEY_C];

	for (int i = 0; i < KEY_C; i++) { key[i] = false; old_key[i] = false; }


	bool quit = false;
	while (!quit) {

		if (GetForegroundWindow() == GetConsoleWindow()) {
			key[0] = GetAsyncKeyState(KEY_PING) & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState(KEY_QUIT) & 0x8000;
		}

		if (key[0] && !old_key[0]) c.pingServer();
		if (key[2] && !old_key[2]) quit = true;

		for (int i = 0; i < KEY_C; i++) old_key[i] = key[i];

		if (c.isConnected()) {
			if (!c.incoming().empty()) {
				auto msg = c.incoming().pop_front().msg;

				switch (msg.header.id) {
				case MessageTypes::ServerPing: {
					std::chrono::system_clock::time_point time_now = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point time_then;
					msg >> time_then;

					std::cout << "rtt: " << std::chrono::duration<double>(time_now - time_then).count() << "\n";
				}
				break;
				}
			}
		}

	}


	return 0;
}