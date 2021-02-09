#pragma once

#include "net_common.h"

namespace icd {
	namespace net {

		template <typename T>
		struct message_header {
			T id{};
			uint32_t size = 0;
		};

		template <typename T>
		struct message {
			message_header<T> header{};
			std::vector<uint8_t> body;

			size_t size() const {
				return sizeof(message_header<T>) + body.size();
			}

			// output stream override
			friend std::ostream& operator << (std::ostream& os, const message<T>& msg) {
				os << "ID: " << int(msg.header.id) << " size: " << msg.header.size;
				return os;
			}
		};
	}
}