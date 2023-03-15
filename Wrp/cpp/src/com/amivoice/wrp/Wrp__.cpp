#include <string>
#include "Poco/Exception.h"
#include "Poco/SynchronizedObject.h"
#include "Poco/Thread.h"
#include "Poco/Timespan.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/WebSocket.h"
#include "com/amivoice/wrp/Wrp.h"
#include "com/amivoice/wrp/Wrp__.h"

static const std::string NULL_STRING = "\x7F";

namespace com {
namespace amivoice {
namespace wrp {

bool Wrp__::_serverCertificateVerification = true;

Wrp__::Wrp__() : Wrp() {
	outData_.resize(1 + 4096);
	inData_.resize(65536);
	webSocket_ = NULL;
	thread_ = NULL;
}

Wrp__::~Wrp__() {
	disconnect_();
}

void Wrp__::connect_(const std::string& serverURL, const std::string& proxyServerName, int connectTimeout, int receiveTimeout) /* override */ {
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
	if (scheme == "wss") {
		const Poco::Net::Context::Ptr context = new Poco::Net::Context(Poco::Net::Context::TLSV1_2_CLIENT_USE, "", "", "", (_serverCertificateVerification) ? Poco::Net::Context::VERIFY_RELAXED : Poco::Net::Context::VERIFY_NONE, 9, true);
		Poco::Net::HTTPSClientSession httpClientSession(hostName, port, context);
		if (proxyServerName != NULL_STRING) {
			httpClientSession.setProxy(proxyHostName, proxyPort);
			if (proxyUserName != NULL_STRING && proxyPassword != NULL_STRING) {
				httpClientSession.setProxyCredentials(proxyUserName, proxyPassword);
			}
		}
		if (connectTimeout == 0) {
			connectTimeout = 60000;
		}
		httpClientSession.setTimeout(Poco::Timespan(connectTimeout / 1000, (connectTimeout % 1000) * 1000));
		Poco::Net::HTTPRequest httpRequest(Poco::Net::HTTPRequest::HTTP_GET, resource, Poco::Net::HTTPMessage::HTTP_1_1);
		Poco::Net::HTTPResponse httpResponse;
		webSocket_ = new Poco::Net::WebSocket(httpClientSession, httpRequest, httpResponse);
	} else {
		Poco::Net::HTTPClientSession httpClientSession(hostName, port);
		if (proxyServerName != NULL_STRING) {
			httpClientSession.setProxy(proxyHostName, proxyPort);
			if (proxyUserName != NULL_STRING && proxyPassword != NULL_STRING) {
				httpClientSession.setProxyCredentials(proxyUserName, proxyPassword);
			}
		}
		if (connectTimeout == 0) {
			connectTimeout = 60000;
		}
		httpClientSession.setTimeout(Poco::Timespan(connectTimeout / 1000, (connectTimeout % 1000) * 1000));
		Poco::Net::HTTPRequest httpRequest(Poco::Net::HTTPRequest::HTTP_GET, resource, Poco::Net::HTTPMessage::HTTP_1_1);
		Poco::Net::HTTPResponse httpResponse;
		webSocket_ = new Poco::Net::WebSocket(httpClientSession, httpRequest, httpResponse);
	}
	thread_ = new Poco::Thread();
	thread_->start(*this);
}

void Wrp__::disconnect_() /* override */ {
	if (webSocket_ != NULL) {
		try {
			webSocket_->shutdown();
		} catch (Poco::Exception& e) {}
	}
	if (thread_ != NULL) {
		thread_->join();
		delete thread_;
		thread_ = NULL;
	}
	if (webSocket_ != NULL) {
		delete webSocket_;
		webSocket_ = NULL;
	}
}

bool Wrp__::isConnected_() /* override */ {
	return (webSocket_ != NULL);
}

void Wrp__::sendRequest_(const char* data, int dataOffset, int dataBytes, char command) /* override */ {
	if (outData_.size() < 1 + dataBytes) {
		outData_.resize(1 + dataBytes);
	}
	int outDataBytes = 0;
	if (command == 0) {
		// pass
	} else {
		outData_[outDataBytes++] = command;
	}
	memcpy(&outData_[outDataBytes], data + dataOffset, dataBytes);
	outDataBytes += dataBytes;
	webSocket_->sendFrame((void*)&outData_[0], outDataBytes, (command == 0) ? /* Text frame */ Poco::Net::WebSocket::FRAME_TEXT : /* Binary frame */ Poco::Net::WebSocket::FRAME_BINARY);
}

void Wrp__::run() /* override */ {
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

void Wrp__::checkResponse_(int timeout) /* override */ {
	unlock();
	if (timeout >= 0) {
		wait(timeout);
	} else {
		wait();
	}
	lock();
}

Poco::SynchronizedObject* Wrp__::synchronizedObject_() /* override */ {
	return this;
}

bool Wrp__::receiveResponse_() {
	int flags = 0;
	int inDataBytes = webSocket_->receiveFrame((void*)&inData_[0], (int)inData_.size(), flags);
	if (inDataBytes <= 0) {
		return false;
	}
	if (inDataBytes > 0 && inData_[inDataBytes - 1] == '\n') {
		inDataBytes--;
		if (inDataBytes > 0 && inData_[inDataBytes - 1] == '\r') {
			inDataBytes--;
		}
	}
	std::string result(inData_.begin(), inData_.begin() + inDataBytes);
	result__ = result;
	return true;
}

} // namespace wrp
} // namespace amivoice
} // namespace com
