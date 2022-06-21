#include <string>
#include "Poco/Base64Encoder.h"
#include "Poco/Exception.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Random.h"
#include "Poco/SynchronizedObject.h"
#include "Poco/Thread.h"
#include "Poco/Timespan.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/StreamSocket.h"
#include "com/amivoice/wrp/Wrp.h"
#include "com/amivoice/wrp/Wrp_.h"

static const std::string NULL_STRING = "\x7F";

namespace com {
namespace amivoice {
namespace wrp {

bool Wrp_::_serverCertificateVerification = true;

const char Wrp_::_base64Table[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

std::string Wrp_::_basic(const std::string& userName, const std::string& password) {
	std::stringstream buffer;
	Poco::Base64Encoder base64Encoder(buffer);
	base64Encoder << userName << ":" << password;
	base64Encoder.close();
	return buffer.str();
}

Wrp_::Wrp_() : Wrp() {
	out_ = NULL;
	outData_.resize(10 + 1 + 4096);
	inData_.resize(4096);
	inDataBytes_ = 0;
	thread_ = NULL;
}

Wrp_::~Wrp_() {
	disconnect_();
}

void Wrp_::connect_(const std::string& serverURL, const std::string& proxyServerName, int connectTimeout, int receiveTimeout) /* override */ {
	std::string::size_type colonDoubleSlashIndex = serverURL.find("://");
	if (colonDoubleSlashIndex == std::string::npos) {
		throw Poco::IOException("Invalid server URL");
	}
	std::string scheme = serverURL.substr(0, colonDoubleSlashIndex);
	if (scheme != "ws" && scheme != "wss") {
		throw Poco::IOException("Invalid scheme: " + scheme);
	}
	std::string hostName;
	std::string resource;
	std::string::size_type slashIndex = serverURL.find('/', colonDoubleSlashIndex + 3);
	if (slashIndex != std::string::npos) {
		hostName = serverURL.substr(colonDoubleSlashIndex + 3, slashIndex - (colonDoubleSlashIndex + 3));
		resource = serverURL.substr(slashIndex);
	} else {
		hostName = serverURL.substr(colonDoubleSlashIndex + 3);
		resource = "/";
	}
	int port;
	std::string::size_type colonIndex = hostName.find(':', hostName.find(']') + 1);
	if (colonIndex != std::string::npos) {
		port = atoi(hostName.substr(colonIndex + 1).c_str());
		if (port == 0) {
			throw Poco::IOException("Invalid port: " + hostName.substr(colonIndex + 1));
		}
		hostName = hostName.substr(0, colonIndex);
	} else {
		if (scheme == "wss") {
			port = 443;
		} else {
			port = 80;
		}
	}
	std::string host;
	if (port == 80) {
		host = hostName;
	} else {
		host = hostName + ":" + std::to_string(port);
	}
	std::string proxyHostName = NULL_STRING;
	int proxyPort = 0;
	std::string proxyUserName = NULL_STRING;
	std::string proxyPassword = NULL_STRING;
	if (proxyServerName != NULL_STRING) {
		proxyHostName = proxyServerName;
		std::string::size_type atIndex = proxyHostName.rfind('@');
		if (atIndex != std::string::npos) {
			/* std::string::size_type */ colonIndex = proxyHostName.find(':');
			if (colonIndex != std::string::npos && colonIndex < atIndex) {
				proxyUserName = proxyHostName.substr(0, colonIndex);
				proxyPassword = proxyHostName.substr(colonIndex + 1, atIndex - (colonIndex + 1));
				proxyHostName = proxyHostName.substr(atIndex + 1);
			}
		}
		/* std::string::size_type */ colonIndex = proxyHostName.find(':', proxyHostName.find(']') + 1);
		if (colonIndex != std::string::npos) {
			proxyPort = atoi(proxyHostName.substr(colonIndex + 1).c_str());
			if (proxyPort == 0) {
				throw Poco::IOException("Invalid port: " + proxyHostName.substr(colonIndex + 1));
			}
			proxyHostName = proxyHostName.substr(0, colonIndex);
		} else {
			proxyPort = 80;
		}
	}
	Poco::Net::StreamSocket socket;
	if (connectTimeout == 0) {
		connectTimeout = 60000;
	}
	if (proxyServerName != NULL_STRING) {
		socket.connect(Poco::Net::SocketAddress(proxyHostName, proxyPort), Poco::Timespan(connectTimeout / 1000, (connectTimeout % 1000) * 1000));
	} else {
		socket.connect(Poco::Net::SocketAddress(hostName, port), Poco::Timespan(connectTimeout / 1000, (connectTimeout % 1000) * 1000));
	}
	socket.setReceiveTimeout(Poco::Timespan(receiveTimeout / 1000, (receiveTimeout % 1000) * 1000));
	if (proxyServerName != NULL_STRING) {
		if (scheme == "ws" || scheme == "wss") {
			Poco::Net::SocketStream out(socket);
			std::string proxyRequestHeader;
			proxyRequestHeader += "CONNECT ";
			proxyRequestHeader += hostName;
			proxyRequestHeader += ":";
			Poco::NumberFormatter::append(proxyRequestHeader, port);
			proxyRequestHeader += " HTTP/1.1\r\n";
			proxyRequestHeader += "Host: ";
			proxyRequestHeader += host;
			proxyRequestHeader += "\r\n";
			proxyRequestHeader += "User-Agent: ";
			proxyRequestHeader += Wrp::getVersion();
			proxyRequestHeader += "\r\n";
			if (proxyUserName != NULL_STRING && proxyPassword != NULL_STRING) {
				proxyRequestHeader += "Proxy-Authorization: Basic ";
				proxyRequestHeader += _basic(proxyUserName, proxyPassword);
				proxyRequestHeader += "\r\n";
				proxyUserName = NULL_STRING;
				proxyPassword = NULL_STRING;
			}
			proxyRequestHeader += "\r\n";
			out.write(proxyRequestHeader.c_str(), proxyRequestHeader.length());
			out.flush();
			std::string proxyResponseHeader;
			int CRLFCRLF = 0;
			int c = 0;
			if (socket.receiveBytes((char*)&c, 1) != 1) {
				c = -1;
			}
			while (c != -1) {
				proxyResponseHeader += (char)c;
				CRLFCRLF <<= 8;
				CRLFCRLF |= c;
				if (CRLFCRLF == 0x0D0A0D0A) {
					break;
				}
				c = 0;
				if (socket.receiveBytes((char*)&c, 1) != 1) {
					c = -1;
				}
			}
			if (proxyResponseHeader[ 8] != ' '
			 || proxyResponseHeader[ 9] != '2'
			 || proxyResponseHeader[10] != '0'
			 || proxyResponseHeader[11] != '0'
			 || proxyResponseHeader[12] != ' ') {
				throw Poco::IOException("Invalid response code: " + proxyResponseHeader.substr(9, 12 - 9));
			}
		} else {
			resource = serverURL;
		}
	}
	if (scheme == "wss") {
		const Poco::Net::Context::Ptr context = new Poco::Net::Context(Poco::Net::Context::TLSV1_2_CLIENT_USE, "", "", "", (_serverCertificateVerification) ? Poco::Net::Context::VERIFY_RELAXED : Poco::Net::Context::VERIFY_NONE, 9, true);
		socket_ = Poco::Net::SecureStreamSocket::attach(socket, hostName, context);
	} else {
		socket_ = socket;
	}
	out_ = new Poco::Net::SocketStream(socket_);
	inDataBytes_ = 0;
	result__.clear();
	// -----------------------------------------------------------
	// GET <resource> HTTP/1.1
	// Host: <hostName>:<port>
	// Upgrade: websocket
	// Connection: upgrade
	// Sec-WebSocket-Key: ...
	// 
	// -----------------------------------------------------------
	std::string requestHeader;
	requestHeader += "GET ";
	requestHeader += resource;
	requestHeader += " HTTP/1.1\r\n";
	requestHeader += "User-Agent: ";
	requestHeader += Wrp::getVersion();
	requestHeader += "\r\n";
	requestHeader += "Host: ";
	requestHeader += host;
	requestHeader += "\r\n";
	requestHeader += "Upgrade: websocket\r\n";
	requestHeader += "Connection: upgrade\r\n";
	requestHeader += "Sec-WebSocket-Key: ";
	Poco::Random random;
	for (int i = 0; i < 20; i += 4) {
		int twentyFourBits = random.next() & 0x3F3F3F3F;
		requestHeader += _base64Table[(twentyFourBits >> 24) & 0x3F];
		requestHeader += _base64Table[(twentyFourBits >> 16) & 0x3F];
		requestHeader += _base64Table[(twentyFourBits >>  8) & 0x3F];
		requestHeader += _base64Table[(twentyFourBits      ) & 0x3F];
	}
	int eightBits = random.next() & 0x3F300000;
	requestHeader += _base64Table[(eightBits >> 24) & 0x3F];
	requestHeader += _base64Table[(eightBits >> 16) & 0x3F];
	requestHeader += "==\r\n";
	requestHeader += "\r\n";
	out_->write(requestHeader.c_str(), requestHeader.length());
	out_->flush();
	// -----------------------------------------------------------
	// HTTP/1.1 101 Switching Protocols
	// Upgrade: websocket
	// Connection: upgrade
	// Sec-WebSocket-Accept: ...
	// 
	// -----------------------------------------------------------
	int responseHeaderBytes = read_CRLFCRLF_();
	if (responseHeaderBytes == -1) {
		throw Poco::IOException("Unexpected end of stream");
	}
	std::string responseHeader(inData_.begin(), inData_.begin() + responseHeaderBytes);
	if (responseHeader[ 8] != ' '
	 || responseHeader[ 9] != '1'
	 || responseHeader[10] != '0'
	 || responseHeader[11] != '1'
	 || responseHeader[12] != ' ') {
		throw Poco::IOException("Handshake error (invalld response code: " + responseHeader.substr(9, 12 - 9) + ")");
	}
	shift_(responseHeaderBytes);
	thread_ = new Poco::Thread();
	thread_->start(*this);
}

void Wrp_::disconnect_() /* override */ {
	if (out_ != NULL) {
		delete out_;
		out_ = NULL;
	}
	if (socket_.impl()->initialized()) {
		try {
			socket_.shutdown();
		} catch (Poco::Exception& e) {}
	}
	if (thread_ != NULL) {
		thread_->join();
		delete thread_;
		thread_ = NULL;
	}
	if (socket_.impl()->initialized()) {
		socket_.close();
	}
}

bool Wrp_::isConnected_() /* override */ {
	return socket_.impl()->initialized();
}

void Wrp_::sendRequest_(const char* data, int dataOffset, int dataBytes, char command) /* override */ {
	// 0                   1                   2                   3
	// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	// +-+-+-+-+-------+-+-------------+-------------------------------+
	// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	// | |1|2|3|       |K|             |                               |
	// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	// |     Extended payload length continued, if payload len == 127  |
	// + - - - - - - - - - - - - - - - +-------------------------------+
	// |                               |Masking-key, if MASK set to 1  |
	// +-------------------------------+-------------------------------+
	// | Masking-key (continued)       |          Payload Data         |
	// +-------------------------------- - - - - - - - - - - - - - - - +
	// :                     Payload Data continued ...                :
	// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	// |                     Payload Data continued ...                |
	// +---------------------------------------------------------------+
	int realDataBytes = ((command == 0) ? 0 : 1) + dataBytes;
	if (outData_.size() < 10 + realDataBytes) {
		outData_.resize(10 + realDataBytes);
	}
	int outDataBytes = 0;
	outData_[outDataBytes++] = (char)(0x80 | ((command == 0) ? /* Text frame */ 0x01 : /* Binary frame */ 0x02));
	if (realDataBytes < 126) {
		outData_[outDataBytes++] = (char)realDataBytes;
	} else
	if (realDataBytes < 65536) {
		outData_[outDataBytes++] = (char)126;
		outData_[outDataBytes++] = (char)((realDataBytes >>  8) & 0xFF);
		outData_[outDataBytes++] = (char)((realDataBytes      ) & 0xFF);
	} else {
		outData_[outDataBytes++] = (char)127;
		outData_[outDataBytes++] = 0;
		outData_[outDataBytes++] = 0;
		outData_[outDataBytes++] = 0;
		outData_[outDataBytes++] = 0;
		outData_[outDataBytes++] = (char)((realDataBytes >> 24) & 0xFF);
		outData_[outDataBytes++] = (char)((realDataBytes >> 16) & 0xFF);
		outData_[outDataBytes++] = (char)((realDataBytes >>  8) & 0xFF);
		outData_[outDataBytes++] = (char)((realDataBytes      ) & 0xFF);
	}
	if (command == 0) {
		// pass
	} else {
		outData_[outDataBytes++] = command;
	}
	memcpy(&outData_[outDataBytes], data + dataOffset, dataBytes);
	outDataBytes += dataBytes;
	out_->write(&outData_[0], outDataBytes);
	out_->flush();
}

void Wrp_::run() /* override */ {
	try {
		onOpen_();
		while (receiveResponse_()) {
			onMessage_(result__);
		}
		onClose_();
	} catch (Poco::TimeoutException& e) {
		onError_(e.name());
	} catch (Poco::IOException& e) {
		onError_(e.name());
	}
}

void Wrp_::checkResponse_(int timeout) /* override */ {
	unlock();
	if (timeout >= 0) {
		wait(timeout);
	} else {
		wait();
	}
	lock();
}

Poco::SynchronizedObject* Wrp_::synchronizedObject_() /* override */ {
	return this;
}

bool Wrp_::receiveResponse_() {
	// 0                   1                   2                   3
	// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	// +-+-+-+-+-------+-+-------------+-------------------------------+
	// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	// | |1|2|3|       |K|             |                               |
	// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	// |     Extended payload length continued, if payload len == 127  |
	// + - - - - - - - - - - - - - - - +-------------------------------+
	// |                               |Masking-key, if MASK set to 1  |
	// +-------------------------------+-------------------------------+
	// | Masking-key (continued)       |          Payload Data         |
	// +-------------------------------- - - - - - - - - - - - - - - - +
	// :                     Payload Data continued ...                :
	// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	// |                     Payload Data continued ...                |
	// +---------------------------------------------------------------+
	if (!read_(2)) {
		if (inDataBytes_ == 0) {
			return false;
		}
		throw Poco::IOException("Unexpected end of stream");
	}
	int inDataBytes = (inData_[1] & 0x7F);
	shift_(2);
	if (inDataBytes == 126) {
		if (!read_(2)) {
			throw Poco::IOException("Unexpected end of stream");
		}
		inDataBytes = ((inData_[0] & 0xFF) <<  8)
					| ((inData_[1] & 0xFF)      );
		shift_(2);
	} else
	if (inDataBytes == 127) {
		if (!read_(8)) {
			throw Poco::IOException("Unexpected end of stream");
		}
		if (inData_[0] != 0
		 || inData_[1] != 0
		 || inData_[2] != 0
		 || inData_[3] != 0 || (inData_[4] & 0x80) != 0) {
			throw Poco::IOException("Invalid payload length: " + std::to_string(inDataBytes));
		}
		inDataBytes = ((inData_[4] & 0xFF) << 24)
					| ((inData_[5] & 0xFF) << 16)
					| ((inData_[6] & 0xFF) <<  8)
					| ((inData_[7] & 0xFF)      );
		shift_(8);
	}
	if (inDataBytes < 0) {
		throw Poco::IOException("Invalid payload length: " + std::to_string(inDataBytes));
	}
	if (!read_(inDataBytes)) {
		throw Poco::IOException("Unexpected end of stream");
	}
	result_(inDataBytes);
	shift_(inDataBytes);
	return true;
}

int Wrp_::read_CRLFCRLF_() {
	int inDataBytes = find_CRLFCRLF_();
	while (inDataBytes == -1) {
		ensure_(inDataBytes_ + 4096);
		int inDataReadBytes = socket_.receiveBytes(&inData_[inDataBytes_], 4096);
		if (inDataReadBytes == 0) {
			return -1;
		}
		inDataBytes_ += inDataReadBytes;
		inDataBytes = find_CRLFCRLF_();
	}
	return inDataBytes + 4;
}

int Wrp_::find_CRLFCRLF_() {
	for (int i = 0; i < inDataBytes_ - 3; i++) {
		if (inData_[i    ] == '\r'
		 && inData_[i + 1] == '\n'
		 && inData_[i + 2] == '\r'
		 && inData_[i + 3] == '\n') {
			return i;
		}
	}
	return -1;
}

bool Wrp_::read_(int inDataBytes) {
	ensure_(inDataBytes);
	while (inDataBytes_ < inDataBytes) {
		int inDataReadBytes = socket_.receiveBytes(&inData_[inDataBytes_], inDataBytes - inDataBytes_);
		if (inDataReadBytes == 0) {
			return false;
		}
		inDataBytes_ += inDataReadBytes;
	}
	return true;
}

void Wrp_::ensure_(int inDataBytes) {
	if ((int)inData_.size() < inDataBytes) {
		inData_.resize(inDataBytes);
	}
}

void Wrp_::result_(int inDataBytes) {
	if (inDataBytes > 0 && inData_[inDataBytes - 1] == '\n') {
		inDataBytes--;
		if (inDataBytes > 0 && inData_[inDataBytes - 1] == '\r') {
			inDataBytes--;
		}
	}
	std::string result(inData_.begin(), inData_.begin() + inDataBytes);
	result__ = result;
}

void Wrp_::shift_(int inDataBytes) {
	inDataBytes_ -= inDataBytes;
	inData_.erase(inData_.begin(), inData_.begin() + inDataBytes);
}

} // namespace wrp
} // namespace amivoice
} // namespace com
