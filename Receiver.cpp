#include <Winsock2.h>
#include <iterator>
#include "Receiver.h"

static const char *Crlf = "\r\n\r\n";

void Receiver::Reset() {
    data_type = NONE;
    buf.clear();
    read_length = 0;
    end_index = 0;
    binary_size = 0;
}

bool Receiver::ProcessByte(char byte) {
    if (byte == Crlf[end_index])
        end_index++;
    else
        buf.push_back(byte);
    if (end_index == 4)
        return true;
    return false;
}

void Receiver::Receive(const char *data, unsigned int size) {
    unsigned int i = 0;
    while (i < size) {

        // determine packet type
        if (data_type == NONE) {
            if (data[i] == 0x24) {
                i++;
                data_type = BINARY;
                read_length = 0;
            }
            else {
                data_type = TEXT;
            }
        }

        // process text packet
        if (data_type == TEXT) {
            char byte = data[i];
            i++;
            if (end_index > 0 && byte != Crlf[end_index]) {
                for (size_t j = 0; j < end_index; j++)
                    buf.push_back(Crlf[j]);
                end_index = 0;
            }
            if (ProcessByte(byte)) {

                // packet end
                if (buf.size() == 0)
                    callback->TextPacket(nullptr, 0);
                else
                    callback->TextPacket(&buf[0], buf.size());
                Reset();
            }
        }

        // process binary packet
        if (data_type == BINARY) {

            // read size of binary packet
            if (read_length < 4) {
                binary_size |= (data[i] << 8 * read_length);
                read_length++;
                i++;
            }
            else {
                binary_size = htonl(binary_size);
                data_type = BINARY_DATA;
            }
        }

        // process binary data
        if (data_type == BINARY_DATA) {

            // read packet
            if (0 == binary_size) {

                // packet end
                callback->BinaryPacket(nullptr, 0);
                Reset();
            }
            else if (size - i >= binary_size) {
                std::copy(data + i, data + i + binary_size, std::back_inserter(buf));
                i += binary_size;

                // packet end
                callback->BinaryPacket(&buf[0], buf.size());
                Reset();
            }
            else {
                std::copy(data + i, data + size, std::back_inserter(buf));
                binary_size -= size - i;
                i = size;
            }
        }
    }
}
