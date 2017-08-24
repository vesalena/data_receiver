#include <memory>
#include <Winsock2.h>
#include <iostream>
#include <stdlib.h>
#include "Receiver.h"

#include <vector>
#include <cctype>

class FakeReceiver : public IReceiver {
private:
    ICallback *callback;

public:
    FakeReceiver(ICallback *cb) : callback(cb) {}

    void Receive(const char *data, unsigned int size) {
        callback->BinaryPacket(data, size);
    }
};

class MockCallback : public ICallback {
public:
    void BinaryPacket(const char* data, unsigned int size) override;
    void TextPacket(const char* data, unsigned int size) override;
};

void MockCallback::BinaryPacket(const char* data, unsigned int size)
{
    std::cout << "binary packet\nsize: " << size << "\n---HEX-START-------\n";
    for (unsigned i = 0; i < size; i++) {
        std::cout << static_cast<unsigned>(data[i]) << " ";
    }
    std::cout << "\n----HEX-END--------\n" << std::endl;
}

void MockCallback::TextPacket(const char* data, unsigned int size)
{
    std::cout << "text packet\nsize: " << size << "\n-------START-------\n";
    for (unsigned i = 0; i < size; i++) {
        if (std::isgraph(data[i])) {
            std::cout << data[i];
        }
        else {
            std::cout << "\\x" << static_cast<int>(data[i]);
        }
    }
    std::cout << "\n--------END--------\n" << std::endl;
}

int main()
{
    MockCallback mock;
    Receiver receiver(&mock);

    // Test #1
    std::vector<char> test{

        // simple text packet
        'a', 'b', 'c', 'd', 'e', '\r', '\n', '\r', '\n',

        // simple binary packet
        0x24, 0, 0, 0, 3, 2, 4, 16,

        // empty binary packet
        0x24, 0, 0, 0, 0,

        // empty text packet
        '\r', '\n', '\r', '\n',

        // text packet with parts of end marker inside,
        // real text marker right after a "fake" one
        'a', '\r', '\r', '\n', 'b', '\r', '\n', '\r', '\r', '\n', '\r', '\n',
    };

    std::vector<double> partials = { 0.0, 0.4, 0.6, 0.7, 0.8 };
    std::vector<unsigned> chunkStarts;
    for (double partial : partials) {
        chunkStarts.push_back(static_cast<unsigned>(test.size() * partial));
    }
    chunkStarts.push_back(test.size());

    for (size_t i = 0; i < chunkStarts.size() - 1; i++) {
        receiver.Receive(&test[0] + chunkStarts[i], chunkStarts[i + 1] - chunkStarts[i]);
    }

    // Test #2
    //char buf0[1024];
    //char buf1[1024];
    //memset(buf1, '1', 1024);
    //buf1[1020] = '\r';
    //buf1[1021] = '\n';
    //buf1[1022] = '\r';
    //buf1[1023] = '\n';

    //buf0[0] = '$';
    //uint32_t*  p = (uint32_t*)(buf0 + 1);
    //*p = htonl(1019);

    //char buf[2048];
    //memcpy(buf, buf0, 1024);
    //memcpy(buf + 1024, buf1, 1024);

    //size_t curpos = 0;
    //const size_t len = 2048;

    //size_t counter = 0;

    //while (counter < 1024 * 500 / 8 / 2 * 20) {
    //    size_t l = rand() % 70;
    //    //printf("%d\n", l);
    //    if (curpos + l > len) {
    //        l = len - curpos;
    //        counter++;
    //    }

    //    receiver.Receive(buf + curpos, l);
    //    curpos = (curpos + l) % len;
    //}
}