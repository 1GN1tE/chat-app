#include "message.hpp"

// Utility to convert integer to byte array (big-endian)
template <typename T>
void Message::to_bytes(T value, std::vector<uint8_t> &bytes)
{
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        bytes.push_back((value >> (8 * (sizeof(T) - 1 - i))) & 0xFF);
    }
}

// Utility to convert byte array to integer (big-endian)
template <typename T>
T Message::from_bytes(const std::vector<uint8_t> &bytes, size_t &idx)
{
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        value = (value << 8) | bytes[idx++];
    }
    return value;
}

// Placeholder encryption (XOR with 0xFF for now)
void Message::encrypt(std::vector<uint8_t> &data) const
{
    for (auto &byte : data)
    {
        byte ^= 0xFF; // Simple XOR encryption with 0xFF
    }
}

// Placeholder decryption (XOR with 0xFF for now)
void Message::decrypt(std::vector<uint8_t> &data) const
{
    for (auto &byte : data)
    {
        byte ^= 0xFF; // Simple XOR decryption with 0xFF
    }
}

Message::Message() : type(0), command(0) {}

void Message::setType(uint8_t t) { type = t; }
uint8_t Message::getType() const { return type; }

void Message::setCommand(uint8_t c) { command = c; }
int Message::getCommand() const { return static_cast<int>(command); }

void Message::addArg(const std::string &arg)
{
    payload.insert(payload.end(), arg.begin(), arg.end());
    payload.push_back(0); // Null-terminate the argument
}

void Message::clearArgs()
{
    payload.clear(); // Clear all elements in the payload
}

// Setter for payload
void Message::setPayload(const std::vector<uint8_t> &newPayload)
{
    payload = newPayload;
}

// Set a payload from a raw array or a pointer
void Message::setPayload(const uint8_t *data, size_t size)
{
    payload.assign(data, data + size);
}

// Getter for payload (if needed)
const std::vector<uint8_t> & Message::getPayload() const
{
    return payload;
}

std::vector<std::string> Message::getArgs() const
{
    std::vector<std::string> args;
    size_t start = 0;

    for (size_t i = 0; i < payload.size(); ++i)
    {
        if (payload[i] == 0)
        { // Null-terminated string found
            args.push_back(std::string(reinterpret_cast<const char *>(&payload[start]), i - start));
            start = i + 1; // Move to the next argument
        }
    }

    return args;
}

std::vector<uint8_t> Message::serialize() const
{
    std::vector<uint8_t> buffer;

    uint16_t length = payload.size();

    if (length > 1020)
    {
        throw std::invalid_argument("Serialization failed: Payload too large " + std::to_string(length));
    }

    // Serialize Header: Type, Command, Payload Length
    buffer.push_back(type);
    buffer.push_back(command);
    to_bytes(length, buffer);

    // Serialize Payload
    buffer.insert(buffer.end(), payload.begin(), payload.end()); // Append actual payload data

    // Encrypt the serialized data (placeholder encryption)
    encrypt(buffer);

    return buffer;
}

Message Message::deserialize(const std::vector<uint8_t> &buffer)
{
    size_t idx = 0;

    // Create a new message object
    Message message;

    // Copy the buffer for decryption
    std::vector<uint8_t> decrypted_buffer = buffer;

    // Decrypt the data (placeholder decryption)
    message.decrypt(decrypted_buffer);

    // Deserialize Header
    if (decrypted_buffer.size() < 3)
    {
        throw std::invalid_argument("Deserialization failed: Insufficient data for header");
    }

    message.type = decrypted_buffer[idx++];
    message.command = decrypted_buffer[idx++];
    uint16_t length = from_bytes<uint16_t>(decrypted_buffer, idx);

    // Validate total size and resize decrypted_buffer if necessary
    if (length > decrypted_buffer.size() - idx)
    {
        throw std::invalid_argument("Deserialization failed: Declared payload size exceeds buffer size");
    }

    // Resize buffer to only include header + payload for this message
    decrypted_buffer.resize(idx + length);

    // Deserialize Payload
    message.payload.resize(length);
    for (size_t i = 0; i < length; ++i)
    {
        message.payload[i] = decrypted_buffer[idx++];
    }

    return message;
}

void Message::print() const
{
    std::cout << "Length: " << (1 + 1 + 2 + payload.size()) << ", Type: " << (int)type
              << ", Command: " << (int)command
              << ", Payload: ";

    for (auto byte : payload)
    {
        std::cout << (char)byte;
    }
    std::cout << std::endl;
}
