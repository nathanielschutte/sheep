#include <iostream>
#include <icd_net.h>

enum class MessageTypes : uint32_t {
	FireBullet,
	MovePlayer
};

int main() {

	icd::net::message<MessageTypes> msg;
	msg.header.id = MessageTypes::FireBullet;

	int a = 1;
	bool b = true;
	float c = 3.14;

	struct {
		float x;
		float y;
	} d[5];

	msg << a << b << c << d;

	a = 0;
	b = false;
	c = 0;

	msg >> d >> c >> b >> a;

	return 0;
}