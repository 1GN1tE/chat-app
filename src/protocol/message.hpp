#ifndef __MESSAGE_HEADER
#define __MESSAGE_HEADER

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <inttypes.h>

class Message
{
private:
    uint8_t type;                 // Type of message (request/response)
    uint8_t command;              // Command byte
    std::vector<uint8_t> payload; // Payload data

    // Utility functions for serialization and deserialization
    template <typename T>
    static void to_bytes(T value, std::vector<uint8_t> &bytes);

    template <typename T>
    static T from_bytes(const std::vector<uint8_t> &bytes, size_t &idx);

    // Placeholder encryption (XOR with 0xFF for now)
    void encrypt(std::vector<uint8_t> &data) const;

    // Placeholder decryption (XOR with 0xFF for now)
    void decrypt(std::vector<uint8_t> &data) const;

public:
    // Constructor
    Message();

    // Setters and Getters for type, flags, and command
    void setType(uint8_t t);
    uint8_t getType() const;

    void setCommand(uint8_t c);
    int getCommand() const;

    // Add an argument to the payload
    void addArg(const std::string &arg);

    // Clear arguments from the payload
    void clearArgs();

    // Set payload, file transfer
    void setPayload(const std::vector<uint8_t> &newPayload);
    void setPayload(const uint8_t *data, size_t size);

    // Get payload, file transfer
    const std::vector<uint8_t> &getPayload() const;

    // Get arguments from the payload
    std::vector<std::string> getArgs() const;

    // Serialize the message
    std::vector<uint8_t> serialize() const;

    // Deserialize the message and validate length
    static Message deserialize(const std::vector<uint8_t> &buffer);

    // Print the message for debugging
    void print() const;
};

#endif // MESSAGE_HPP
