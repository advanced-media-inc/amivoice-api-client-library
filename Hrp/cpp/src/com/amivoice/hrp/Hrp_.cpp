#include <cctype>
#include <string>
#include "Poco/Base64Encoder.h"
#include "Poco/Exception.h"
#include "Poco/NumberFormatter.h"
#include "Poco/String.h"
#include "Poco/SynchronizedObject.h"
#include "Poco/Thread.h"
#include "Poco/Timespan.h"
#include "Poco/URI.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/StreamSocket.h"
#include "com/amivoice/hrp/Hrp.h"
#include "com/amivoice/hrp/Hrp_.h"

static const std::string NULL_STRING = "\x7F";
static const std::string RESERVED_QUERY_PARAM = "?#/:;+@&=";

namespace com {
namespace amivoice {
namespace hrp {

bool Hrp_::_serverCertificateVerification = true;

std::string Hrp_::_basic(const std::string& userName, const std::string& password) {
	std::stringstream buffer;
	Poco::Base64Encoder base64Encoder(buffer);
	base64Encoder << userName << ":" << password;
	base64Encoder.close();
	return buffer.str();
}

Hrp_::Hrp_() : Hrp() {
	proxyUserName_ = NULL_STRING;
	proxyPassword_ = NULL_STRING;
	out_ = NULL;
	requestBodyPartedOffset_ = 0;
	inData_.resize(4096);
	inDataBytes_ = 0;
	inDataBytes__ = 0;
	thread_ = NULL;
}

Hrp_::~Hrp_() {
	disconnect_();
}

void Hrp_::connect_(const std::string& serverURL, const std::string& proxyServerName, int connectTimeout, int receiveTimeout) /* override */ {
	std::string::size_type colonDoubleSlashIndex = serverURL.find("://");
	if (colonDoubleSlashIndex == std::string::npos) {
		throw Poco::IOException("Invalid server URL");
	}
	std::string scheme = serverURL.substr(0, colonDoubleSlashIndex);
	if (scheme != "http" && scheme != "https") {
		throw Poco::IOException("Invalid scheme: " + scheme);
	}
	std::string hostName;
	std::string::size_type slashIndex = serverURL.find('/', colonDoubleSlashIndex + 3);
	if (slashIndex != std::string::npos) {
		hostName = serverURL.substr(colonDoubleSlashIndex + 3, slashIndex - (colonDoubleSlashIndex + 3));
		resource_ = serverURL.substr(slashIndex);
	} else {
		hostName = serverURL.substr(colonDoubleSlashIndex + 3);
		resource_ = "/";
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
		if (scheme == "https") {
			port = 443;
		} else {
			port = 80;
		}
	}
	if (port == 80) {
		host_ = hostName;
	} else {
		host_ = hostName + ":" + std::to_string(port);
	}
	std::string proxyHostName = NULL_STRING;
	int proxyPort = 0;
	proxyUserName_ = NULL_STRING;
	proxyPassword_ = NULL_STRING;
	if (proxyServerName != NULL_STRING) {
		proxyHostName = proxyServerName;
		std::string::size_type atIndex = proxyHostName.rfind('@');
		if (atIndex != std::string::npos) {
			/* std::string::size_type */ colonIndex = proxyHostName.find(':');
			if (colonIndex != std::string::npos && colonIndex < atIndex) {
				proxyUserName_ = proxyHostName.substr(0, colonIndex);
				proxyPassword_ = proxyHostName.substr(colonIndex + 1, atIndex - (colonIndex + 1));
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
		if (scheme == "https") {
			Poco::Net::SocketStream out(socket);
			std::string proxyRequestHeader;
			proxyRequestHeader += "CONNECT ";
			proxyRequestHeader += hostName;
			proxyRequestHeader += ":";
			Poco::NumberFormatter::append(proxyRequestHeader, port);
			proxyRequestHeader += " HTTP/1.1\r\n";
			proxyRequestHeader += "Host: ";
			proxyRequestHeader += host_;
			proxyRequestHeader += "\r\n";
			proxyRequestHeader += "User-Agent: ";
			proxyRequestHeader += Hrp::getVersion();
			proxyRequestHeader += "\r\n";
			if (proxyUserName_ != NULL_STRING && proxyPassword_ != NULL_STRING) {
				proxyRequestHeader += "Proxy-Authorization: Basic ";
				proxyRequestHeader += _basic(proxyUserName_, proxyPassword_);
				proxyRequestHeader += "\r\n";
				proxyUserName_ = NULL_STRING;
				proxyPassword_ = NULL_STRING;
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
			resource_ = serverURL;
		}
	}
	if (scheme == "https") {
		const Poco::Net::Context::Ptr context = new Poco::Net::Context(Poco::Net::Context::TLSV1_2_CLIENT_USE, "", "", "", (_serverCertificateVerification) ? Poco::Net::Context::VERIFY_RELAXED : Poco::Net::Context::VERIFY_NONE, 9, true);
		socket_ = Poco::Net::SecureStreamSocket::attach(socket, hostName, context);
	} else {
		socket_ = socket;
	}
	out_ = new Poco::Net::SocketStream(socket_);
	thread_ = new Poco::Thread();
	thread_->start(*this);
}

void Hrp_::disconnect_() /* override */ {
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

bool Hrp_::isConnected_() /* override */ {
	return socket_.impl()->initialized();
}

void Hrp_::sendRequestHeader_(const std::string& domainId,
							  const std::string& codec,
							  const std::string& resultType,
							  const std::string& resultEncoding,
							  const std::string& serviceAuthorization,
							  const std::string& voiceDetection,
							  const std::string& acceptTopic,
							  const std::string& contentType,
							  const std::string& accept, const char* type, long long dataBytes) /* override */ {
	requestBodyChunked_.clear();
	requestBodyParted_.clear();
	requestBodyPartedOffset_ = 0;
	inDataBytes_ = 0;
	inDataBytes__ = 0;
	inDataEncoding__.clear();
	result__.clear();
	std::string requestHeader;
	requestHeader += "POST ";
	requestHeader += resource_;
	if (type[0] != 'm') {
		char delimiter = (resource_.find('?') == -1) ? '?' : '&';
		if (domainId != NULL_STRING && domainId.length() > 0) {
			requestHeader += delimiter;
			delimiter = '&';
			requestHeader += "d=";
			Poco::URI::encode(domainId, RESERVED_QUERY_PARAM, requestHeader);
		}
		if (codec != NULL_STRING && codec.length() > 0) {
			requestHeader += delimiter;
			delimiter = '&';
			requestHeader += "c=";
			Poco::URI::encode(codec, RESERVED_QUERY_PARAM, requestHeader);
		}
		if (resultType != NULL_STRING && resultType.length() > 0) {
			requestHeader += delimiter;
			delimiter = '&';
			requestHeader += "r=";
			Poco::URI::encode(resultType, RESERVED_QUERY_PARAM, requestHeader);
		}
		if (resultEncoding != NULL_STRING && resultEncoding.length() > 0) {
			requestHeader += delimiter;
			delimiter = '&';
			requestHeader += "e=";
			Poco::URI::encode(resultEncoding, RESERVED_QUERY_PARAM, requestHeader);
		}
		if (serviceAuthorization != NULL_STRING && serviceAuthorization.length() > 0) {
			requestHeader += delimiter;
			delimiter = '&';
			requestHeader += "u=";
			Poco::URI::encode(serviceAuthorization, RESERVED_QUERY_PARAM, requestHeader);
		}
		if (voiceDetection != NULL_STRING) {
			requestHeader += delimiter;
			delimiter = '&';
			requestHeader += "v=";
			Poco::URI::encode(voiceDetection, RESERVED_QUERY_PARAM, requestHeader);
		}
	} else {
		std::string requestBody;
		if (domainId != NULL_STRING && domainId.length() > 0) {
			requestBody += "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"d\"\r\n\r\n";
			requestBody += domainId;
		}
		if (codec != NULL_STRING && codec.length() > 0) {
			requestBody += "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"c\"\r\n\r\n";
			requestBody += codec;
		}
		if (resultType != NULL_STRING && resultType.length() > 0) {
			requestBody += "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"r\"\r\n\r\n";
			requestBody += resultType;
		}
		if (resultEncoding != NULL_STRING && resultEncoding.length() > 0) {
			requestBody += "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"e\"\r\n\r\n";
			requestBody += resultEncoding;
		}
		if (serviceAuthorization != NULL_STRING && serviceAuthorization.length() > 0) {
			requestBody += "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"u\"\r\n\r\n";
			requestBody += serviceAuthorization;
		}
		if (voiceDetection != NULL_STRING) {
			requestBody += "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"v\"\r\n\r\n";
			requestBody += voiceDetection;
		}
		requestBody += "\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"a\"\r\n\r\n";
		requestBody += "\r\n--1234567890ABCDEFGHIJ--\r\n";
		requestBodyParted_ = requestBody; // [70+n+][70+n+][70+n+][70+n+][70+n+][70+n+]70+28 bytes
	}
	requestHeader += " HTTP/1.1\r\n";
	requestHeader += "Host: ";
	requestHeader += host_;
	requestHeader += "\r\n";
	requestHeader += "User-Agent: ";
	requestHeader += Hrp::getVersion();
	requestHeader += "\r\n";
	if (proxyUserName_ != NULL_STRING && proxyPassword_ != NULL_STRING) {
		requestHeader += "Proxy-Authorization: Basic ";
		requestHeader += _basic(proxyUserName_, proxyPassword_);
		requestHeader += "\r\n";
	}
	if (type[1] != 'c') {
		requestHeader += "Content-Length: ";
		if (requestBodyParted_.empty()) {
			Poco::NumberFormatter::append(requestHeader, dataBytes);
		} else {
			Poco::NumberFormatter::append(requestHeader, dataBytes + requestBodyParted_.length());
		}
		requestHeader += "\r\n";
	} else {
		requestBodyChunked_ = "0\r\n\r\n";
		requestHeader += "Transfer-Encoding: chunked\r\n";
	}
	if (requestBodyParted_.empty()) {
		if (acceptTopic != NULL_STRING && acceptTopic.length() > 0) {
			requestHeader += "Accept-Topic: ";
			requestHeader += acceptTopic;
			requestHeader += "\r\n";
		}
		if (contentType != NULL_STRING && contentType.length() > 0) {
			requestHeader += "Content-Type: ";
			requestHeader += contentType;
			requestHeader += "\r\n";
		}
		if (accept != NULL_STRING && accept.length() > 0) {
			requestHeader += "Accept: ";
			requestHeader += accept;
			requestHeader += "\r\n";
		}
	} else {
		requestHeader += "Content-Type: multipart/form-data; boundary=1234567890ABCDEFGHIJ\r\n";
	}
	requestHeader += "\r\n";
	out_->write(requestHeader.c_str(), requestHeader.length());
	out_->flush();
}

void Hrp_::sendRequestBody_(const char* data, int dataOffset, int dataBytes) /* override */ {
	if (data != NULL && dataBytes > 0) {
		if (!requestBodyParted_.empty()) {
			if (requestBodyPartedOffset_ == 0) {
				requestBodyPartedOffset_ = (int)requestBodyParted_.length() - 28;
				if (!requestBodyChunked_.empty()) {
					std::string hex;
					Poco::NumberFormatter::appendHex(hex, requestBodyPartedOffset_);
					out_->write(hex.c_str(), hex.length());
					out_->write(requestBodyChunked_.c_str() + 1, 2);
				}
				out_->write(requestBodyParted_.c_str(), requestBodyPartedOffset_);
				if (!requestBodyChunked_.empty()) {
					out_->write(requestBodyChunked_.c_str() + 3, 2);
				}
			}
		}
		if (!requestBodyChunked_.empty()) {
			std::string hex;
			Poco::NumberFormatter::appendHex(hex, dataBytes);
			out_->write(hex.c_str(), hex.length());
			out_->write(requestBodyChunked_.c_str() + 1, 2);
		}
		out_->write(data + dataOffset, dataBytes);
		if (!requestBodyChunked_.empty()) {
			out_->write(requestBodyChunked_.c_str() + 3, 2);
		}
		out_->flush();
	} else {
		if (!requestBodyParted_.empty()) {
			if (!requestBodyChunked_.empty()) {
				std::string hex;
				Poco::NumberFormatter::appendHex(hex, requestBodyParted_.length() - requestBodyPartedOffset_);
				out_->write(hex.c_str(), hex.length());
				out_->write(requestBodyChunked_.c_str() + 1, 2);
			}
			out_->write(requestBodyParted_.c_str() + requestBodyPartedOffset_, requestBodyParted_.length() - requestBodyPartedOffset_);
			if (!requestBodyChunked_.empty()) {
				out_->write(requestBodyChunked_.c_str() + 3, 2);
				out_->write(requestBodyChunked_.c_str(), 5);
			}
		} else {
			if (!requestBodyChunked_.empty()) {
				out_->write(requestBodyChunked_.c_str(), 5);
			}
		}
		out_->flush();
	}
}

void Hrp_::run() /* override */ {
	try {
		while (true) {
			std::string sessionId = receiveResponseHeader_();
			onOpen_(sessionId);
			if (inDataBytes__ > 0) {
				receiveResponseBody_();
				int crlfIndex = result_CRLF_();
				while (crlfIndex != -1) {
					std::string result = result__.substr(0, crlfIndex);
					onMessage_(result);
					result__.erase(0, crlfIndex + 2);
					crlfIndex = result_CRLF_();
				}
			} else {
				while (receiveChunkedResponseBody_()) {
					int crlfIndex = result_CRLF_();
					while (crlfIndex != -1) {
						std::string result = result__.substr(0, crlfIndex);
						onMessage_(result);
						result__.erase(0, crlfIndex + 2);
						crlfIndex = result_CRLF_();
					}
				}
			}
			onClose_();
		}
	} catch (Poco::TimeoutException& e) {
		onError_(e.name());
	} catch (Poco::IOException& e) {
		onError_(e.name());
	}
}

void Hrp_::checkResponse_(int timeout) /* override */ {
	unlock();
	if (timeout >= 0) {
		wait(timeout);
	} else {
		wait();
	}
	lock();
}

Poco::SynchronizedObject* Hrp_::synchronizedObject_() /* override */ {
	return this;
}

std::string Hrp_::receiveResponseHeader_() {
	int responseHeaderBytes = read_CRLFCRLF_();
	if (responseHeaderBytes == -1) {
		throw Poco::IOException("Unexpected end of stream");
	}
	std::string responseHeader(inData_.begin(), inData_.begin() + responseHeaderBytes);
	if (responseHeader[ 8] != ' '
	 || responseHeader[ 9] != '2'
	 || responseHeader[10] != '0'
	 || responseHeader[11] != '0'
	 || responseHeader[12] != ' ') {
		throw Poco::IOException("Invalid response code: " + responseHeader.substr(9, 12 - 9));
	}
	std::string::size_type index1 = responseHeader.find("\r\nX-Session-ID: ");
	if (index1 == std::string::npos) {
		throw Poco::IOException("Missing session id");
	}
	index1 += 16;
	std::string::size_type index2 = responseHeader.find("\r\n", index1);
	if (index2 == std::string::npos) {
		throw Poco::IOException("Missing session id");
	}
	std::string sessionId = responseHeader.substr(index1, index2 - index1);
	inDataEncoding__ = "UTF-8";
	index1 = responseHeader.find("\r\nContent-Type: ");
	if (index1 != std::string::npos) {
		index1 += 16;
		index2 = responseHeader.find("\r\n", index1);
		if (index2 != std::string::npos) {
			std::string::size_type index3 = responseHeader.find("; charset=", index1);
			if (index3 != std::string::npos && index3 < index2) {
				index3 += 10;
				inDataEncoding__ = Poco::toUpper(responseHeader.substr(index3, index2 - index3));
				if (inDataEncoding__ == "SHIFT_JIS") {
					inDataEncoding__ = "MS932";
				}
			}
		}
	}
	index1 = responseHeader.find("\r\nContent-Length: ");
	if (index1 != std::string::npos) {
		index1 += 18;
		index2 = responseHeader.find("\r\n", index1);
		if (index2 != std::string::npos) {
			inDataBytes__ = atoi(responseHeader.substr(index1, index2 - index1).c_str());
			if (inDataBytes__ == 0) {
				throw Poco::IOException("Invalid content length: " + responseHeader.substr(index1, index2 - index1));
			}
		}
	}
	shift_(responseHeaderBytes);
	return sessionId;
}

void Hrp_::receiveResponseBody_() {
	if (!read_(inDataBytes__)) {
		throw Poco::IOException("Unexpected end of stream");
	}
	result_(inDataBytes__);
	shift_(inDataBytes__);
}

bool Hrp_::receiveChunkedResponseBody_() {
	int chunkedResponseHeaderBytes = read_CRLF_();
	if (chunkedResponseHeaderBytes == -1) {
		throw Poco::IOException("Unexpected end of stream");
	}
	inDataBytes__ = 0;
	for (int i = 0; i < chunkedResponseHeaderBytes; i++) {
		int c = inData_[i];
		if (c >= '0' && c <= '9') {
			inDataBytes__ <<= 4;
			inDataBytes__ += c - '0';
		} else
		if (c >= 'A' && c <= 'F') {
			inDataBytes__ <<= 4;
			inDataBytes__ += c - 'A' + 10;
		} else
		if (c >= 'a' && c <= 'f') {
			inDataBytes__ <<= 4;
			inDataBytes__ += c - 'a' + 10;
		} else {
			break;
		}
	}
	shift_(chunkedResponseHeaderBytes);
	if (inDataBytes__ == 0) {
		if (!read_(2)) {
			throw Poco::IOException("Unexpected end of stream");
		}
		shift_(2);
		return false;
	}
	if (!read_(inDataBytes__ + 2)) {
		throw Poco::IOException("Unexpected end of stream");
	}
	result_(inDataBytes__);
	shift_(inDataBytes__ + 2);
	return true;
}

int Hrp_::read_CRLFCRLF_() {
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

int Hrp_::find_CRLFCRLF_() {
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

int Hrp_::read_CRLF_() {
	int inDataBytes = find_CRLF_();
	while (inDataBytes == -1) {
		ensure_(inDataBytes_ + 4096);
		int inDataReadBytes = socket_.receiveBytes(&inData_[inDataBytes_], 4096);
		if (inDataReadBytes == 0) {
			return -1;
		}
		inDataBytes_ += inDataReadBytes;
		inDataBytes = find_CRLF_();
	}
	return inDataBytes + 2;
}

int Hrp_::find_CRLF_() {
	for (int i = 0; i < inDataBytes_ - 1; i++) {
		if (inData_[i    ] == '\r'
		 && inData_[i + 1] == '\n') {
			return i;
		}
	}
	return -1;
}

bool Hrp_::read_(int inDataBytes) {
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

void Hrp_::ensure_(int inDataBytes) {
	if ((int)inData_.size() < inDataBytes) {
		inData_.resize(inDataBytes);
	}
}

void Hrp_::result_(int inDataBytes) {
	std::string result(inData_.begin(), inData_.begin() + inDataBytes);
	result__ += result;
}

int Hrp_::result_CRLF_() {
	for (int i = 0; i < (int)result__.length() - 1; i++) {
		if (result__[i    ] == '\r'
		 && result__[i + 1] == '\n') {
			return i;
		}
	}
	return -1;
}

void Hrp_::shift_(int inDataBytes) {
	inDataBytes_ -= inDataBytes;
	inData_.erase(inData_.begin(), inData_.begin() + inDataBytes);
}

} // namespace hrp
} // namespace amivoice
} // namespace com
