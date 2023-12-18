#ifndef SERVER_STREAM_HPP
#define SERVER_STREAM_HPP

#include "../webserv.hpp"

class ServerStream {
   protected:
    String _rawData;
    String _outputData;
    size_t _outputDataSize;
    size_t _totalRead;
    size_t _totalSentBytes;

    int sendAll(int fd, char *data, size_t size) {
        int sentBytes = send(fd, &data[_totalSentBytes], size - _totalSentBytes, 0);
        _totalSentBytes += sentBytes;

        if (sentBytes == -1 || sentBytes == 0) {
            debug("size", toString(size), YELLOW);
            debug("sent bytes", toString(sentBytes), YELLOW);
            debugErr("Error while writing to fd", &toString(fd)[0]);
            return -1;
        }
        return size - _totalSentBytes;
    }

   public:
    ServerStream() : _totalRead(0), _totalSentBytes(0) {
    }

    String &outputData() {
        return _outputData;
    }
    size_t outputDataSize() {
        return _outputDataSize;
    }
    String &rawData() {
        return _rawData;
    }
    size_t totalRead() {
        return _totalRead;
    }
    size_t totalSent() {
        return _totalSentBytes;
    }

    int recvAll(int fd) {
        _rawData.resize(_totalRead + BUF_SIZE, '\0');

        int readBytes = recv(fd, &_rawData[_totalRead], BUF_SIZE, 0);
        _totalRead += readBytes;

        if (readBytes == -1) {
            debug("Error while reading fd", toString(fd), RED);
            return -1;
        }
        return readBytes;
    }
};

#endif