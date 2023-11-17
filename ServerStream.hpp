#ifndef SERVER_STREAM_HPP
#define SERVER_STREAM_HPP

#include "webserv.hpp"

// #define MAX_WRITE_SIZE 100000000

class ServerStream {
   private:
    // size_t adjustWriteSize(size_t size) {
    //     if (size > MAX_WRITE_SIZE) {
    //         if (MAX_WRITE_SIZE > size - _totalSentBytes) {
    //             return size - _totalSentBytes;
    //         }
    //         return MAX_WRITE_SIZE;
    //     }
    //     return size;
    // }

   protected:
    std::string _rawData;

    size_t _totalRead;
    size_t _totalSentBytes;

    int sendAll(int fd, char *data, size_t size) {
        int sentBytes = write(fd, &data[_totalSentBytes], size);
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

    std::string &rawData() {
        return _rawData;
    }

    int recvAll(int fd) {
        _rawData.resize(_totalRead + BUF_SIZE + 1, '\0');
        int readBytes = read(fd, &_rawData[_totalRead], BUF_SIZE);
        _totalRead += readBytes;

        if (readBytes == -1 || readBytes == 0) {
            debug("Error while reading fd", std::to_string(fd), RED);
            return -1;
        }
        return 0;
    }
};

#endif