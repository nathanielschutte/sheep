#include <iostream>
#include <icd_net.h>

enum class MessageTypes : uint32_t {
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class SampleServer : public icd::net::server_interface<MessageTypes> {

public:
	SampleServer(uint16_t port) : icd::net::server_interface<MessageTypes>(port) {

	}

protected:
	virtual bool clientConnect(std::shared_ptr<icd::net::connection<MessageTypes>> client) {
		return true;
	}

	virtual bool clientDisconnect(std::shared_ptr<icd::net::connection<MessageTypes>> client) {
		
	}

	virtual bool clientMessage(std::shared_ptr<icd::net::connection<MessageTypes>> client,
		icd::net::message<MessageTypes>) {
		
	}
};

int main() {
	SampleServer server(60000);
	server.start();

	while (1) {
		server.update();
	}

	return 0;
}