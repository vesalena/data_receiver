#pragma once

#include <vector>

struct IReceiver
{
    virtual void Receive(const char* data, unsigned int size) = 0;
};

struct ICallback
{
    virtual void BinaryPacket(const char* data, unsigned int size) = 0;
    virtual void TextPacket(const char* data, unsigned int size) = 0;
};

class Receiver : public IReceiver {
private:
    enum State {
        NONE,
        BINARY,
        BINARY_DATA,
        TEXT,
    };

    static const int InitialBufferSize = 1024;

    // buffer state
    State data_type = NONE;
    std::vector<char> buf;
    int read_length = 0;
    size_t end_index = 0;
    uint32_t binary_size = 0;

    // user callback
    ICallback *callback;

    // reset states
    void Reset();

    // process text byte
    bool ProcessByte(char byte);

public:
    Receiver(ICallback *cb) : callback(cb) {
        buf.reserve(InitialBufferSize);
    }

    void Receive(const char *data, unsigned int size) override;
};
