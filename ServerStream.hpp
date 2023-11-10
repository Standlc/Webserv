#ifndef SERVER_STREAM_HPP
#define SERVER_STREAM_HPP

#include "webserv.hpp"

class ServerStream
{
protected:
	std::string _rawData;
	int _totalRead;
	bool _isRead;

	std::string _outputData;
	int _outputDataSize;
	int _totalSentBytes;
	int _responseSize;

public:
	ServerStream()
	{
		_isRead = false;
		_totalRead = 0;
		_totalSentBytes = 0;
	}

	bool isRead()
	{
		return _isRead;
	}

	std::string getRawData()
	{
		return _rawData;
	}

	std::string getOuputData()
	{
		return _outputData;
	}

	int recvAll(int fd)
	{
		_isRead = true;

		_rawData.resize(_totalRead + BUF_SIZE + 1, '\0');
		int readBytes = read(fd, &_rawData[_totalRead], BUF_SIZE);
		_totalRead += readBytes;

		return (readBytes == -1 || readBytes == 0) ? -1 : 0;
	}

	int sendAll(int fd)
	{
		updateOutputData();
		_outputDataSize = std::strlen(&_outputData[0]);

		int sentBytes = send(fd, &_outputData[_totalSentBytes], _outputDataSize - _totalSentBytes, 0);
		_totalSentBytes += sentBytes;

		if (sentBytes == -1)
		{
			std::cerr << "Error while sending response\n";
			return -1;
		}
		return _outputDataSize - _totalSentBytes;
	}

	virtual void updateOutputData() = 0;
};

#endif