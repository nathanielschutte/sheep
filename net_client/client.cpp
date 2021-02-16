#include <iostream>
#include <icd_net.h>

enum class MessageTypes : uint32_t {
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class SampleClient : public icd::net::client_interface<MessageTypes> {

};



int main() {

	SampleClient c;
	c.connect("127.0.0.1", 60000);
	return 0;
}