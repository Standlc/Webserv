#ifndef SERVER_STREAM_HPP
#define SERVER_STREAM_HPP

#include "webserv.hpp"

class ServerStream {
   protected:
    String _rawData;
    size_t _totalRead;
    size_t _totalSentBytes;

    int sendAll(int fd, char *data, size_t size) {
        int sentBytes = send(fd, &data[_totalSentBytes], size, 0);
        _totalSentBytes += sentBytes;

        if (sentBytes == -1 || sentBytes == 0) {
            debugErr("Error while writing to fd", &std::to_string(fd)[0]);
            return -1;
        }
        return size - _totalSentBytes;
    }

   public:
    ServerStream() : _totalRead(0), _totalSentBytes(0) {
    }

    String &rawData() {
        return _rawData;
    }

    int recvAll(int fd) {
        _rawData.resize(_totalRead + BUF_SIZE + 1, '\0');

        int readBytes = recv(fd, &_rawData[_totalRead], BUF_SIZE, 0);
        _totalRead += readBytes;

        if (readBytes == -1 || readBytes == 0) {
            debug("Error while reading fd", std::to_string(fd), RED);
            return -1;
        }
        return 0;
    }
};

#endif