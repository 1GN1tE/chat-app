#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <inttypes.h>

#include "message.hpp"

int main() {
    try {
        // Create a ProtocolMessage (e.g., !login Alice securepass123)
        Message request;
        request.setType(0x01); // Request message
        request.setCommand(0x10); // Login command
        
        // request.addArg("Bob");
        // request.addArg("WhoAmI");

        std::vector<uint8_t> vec(1020, 1);

        request.setPayload(vec);

        // Serialize the message
        std::vector<uint8_t> serialized = request.serialize();
        
        // Deserialize the message
        Message deserialized_msg = Message::deserialize(serialized);
        
        // Print the deserialized message
        // deserialized_msg.print();
        
        // Get arguments from the payload
        std::vector<std::string> args = deserialized_msg.getArgs();
        for (const auto& arg : args) {
            std::cout << "Argument: " << arg << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
