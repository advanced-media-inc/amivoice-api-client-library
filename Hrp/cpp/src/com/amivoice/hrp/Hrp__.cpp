#include <cctype>
#include <string>
#include "Poco/BinaryReader.h"
#include "Poco/BinaryWriter.h"
#include "Poco/Exception.h"
#include "Poco/SynchronizedObject.h"
#include "Poco/Timespan.h"
#include "Poco/URI.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "com/amivoice/hrp/Hrp.h"
#include "com/amivoice/hrp/Hrp__.h"

static const std::string NULL_STRING = "\x7F";

namespace com {
namespace amivoice {
namespace hrp {

bool Hrp__::_serverCertificateVerification = true;

Hrp__::Hrp__() : Hrp() {
	httpClientSession_ = NULL;
	out_ = NULL;
	in_ = NULL;
	requestBodyPartedOffset_ = 0;
}

Hrp__::~Hrp__() {
	disconnect_();
}

void Hrp__::connect_(const std::string& serverURL, const std::string& proxyServerName, int connectTimeout, int receiveTimeout) /* override */ {
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
	if (scheme == "https") {
		const Poco::Net::Context::Ptr context = new Poco::Net::Context(Poco::Net::Context::TLSV1_2_CLIENT_USE, "", "", "", (_serverCertificateVerification) ? Poco::Net::Context::VERIFY_RELAXED : Poco::Net::Context::VERIFY_NONE, 9, true);
		httpClientSession_ = new Poco::Net::HTTPSClientSession(hostName, port, context);
	} else {
		httpClientSession_ = new Poco::Net::HTTPClientSession(hostName, port);
	}
	if (proxyServerName != NULL_STRING) {
		httpClientSession_->setProxy(proxyHostName, proxyPort);
		if (proxyUserName != NULL_STRING && proxyPassword != NULL_STRING) {
			httpClientSession_->setProxyCredentials(proxyUserName, proxyPassword);
		}
	}
	if (connectTimeout == 0) {
		connectTimeout = 60000;
	}
	httpClientSession_->setTimeout(Poco::Timespan(connectTimeout / 1000, (connectTimeout % 1000) * 1000));
}

void Hrp__::disconnect_() /* override */ {
	if (httpClientSession_ != NULL) {
		httpClientSession_->reset();
	}
	if (in_ != NULL) {
		delete in_;
		in_ = NULL;
	}
	if (out_ != NULL) {
		delete out_;
		out_ = NULL;
	}
	if (httpClientSession_ != NULL) {
		delete httpClientSession_;
		httpClientSession_ = NULL;
	}
}

bool Hrp__::isConnected_() /* override */ {
	return (httpClientSession_ != NULL);
}

void Hrp__::sendRequestHeader_(const std::string& domainId,
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
	inDataEncoding__.clear();
	result__.clear();
	std::string resource;
	if (type[0] != 'm') {
		Poco::URI uri(resource_);
		if (domainId != NULL_STRING && domainId.length() > 0) {
			uri.addQueryParameter("d", domainId);
		}
		if (codec != NULL_STRING && codec.length() > 0) {
			uri.addQueryParameter("c", codec);
		}
		if (resultType != NULL_STRING && resultType.length() > 0) {
			uri.addQueryParameter("r", resultType);
		}
		if (resultEncoding != NULL_STRING && resultEncoding.length() > 0) {
			uri.addQueryParameter("e", resultEncoding);
		}
		if (serviceAuthorization != NULL_STRING && serviceAuthorization.length() > 0) {
			uri.addQueryParameter("u", serviceAuthorization);
		}
		if (voiceDetection != NULL_STRING) {
			uri.addQueryParameter("v", voiceDetection);
		}
		resource = uri.toString();
	} else {
		resource = resource_;
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
	Poco::Net::HTTPRequest httpRequest(Poco::Net::HTTPRequest::HTTP_POST, resource, Poco::Net::HTTPMessage::HTTP_1_1);
	httpRequest.add("User-Agent", Hrp::getVersion());
	if (type[1] != 'c') {
		if (requestBodyParted_.empty()) {
			httpRequest.setContentLength(dataBytes);
		} else {
			httpRequest.setContentLength(dataBytes + requestBodyParted_.length());
		}
	} else {
		httpRequest.setChunkedTransferEncoding(true);
	}
	if (requestBodyParted_.empty()) {
		if (acceptTopic != NULL_STRING && acceptTopic.length() > 0) {
			httpRequest.add("Accept-Topic", acceptTopic);
		}
		if (contentType != NULL_STRING && contentType.length() > 0) {
			httpRequest.add("Content-Type", contentType);
		}
		if (accept != NULL_STRING && accept.length() > 0) {
			httpRequest.add("accept", accept);
		}
	} else {
		httpRequest.add("Content-Type", "multipart/form-data; boundary=1234567890ABCDEFGHIJ");
	}
	out_ = new Poco::BinaryWriter(httpClientSession_->sendRequest(httpRequest));
}

void Hrp__::sendRequestBody_(const char* data, int dataOffset, int dataBytes) /* override */ {
	if (out_ != NULL) {
		if (data != NULL && dataBytes > 0) {
			if (!requestBodyParted_.empty()) {
				if (requestBodyPartedOffset_ == 0) {
					requestBodyPartedOffset_ = (int)requestBodyParted_.length() - 28;
					out_->writeRaw(requestBodyParted_.c_str(), requestBodyPartedOffset_);
				}
			}
			out_->writeRaw(data + dataOffset, dataBytes);
			out_->flush();
		} else {
			if (!requestBodyParted_.empty()) {
				out_->writeRaw(requestBodyParted_.c_str() + requestBodyPartedOffset_, requestBodyParted_.length() - requestBodyPartedOffset_);
			}
			delete out_;
			out_ = NULL;
			run();
		}
	}
}

void Hrp__::run() {
	try {
		while (true) {
			std::string sessionId = receiveResponseHeader_();
			onOpen_(sessionId);
			while (receiveResponseBody_()) {
				onMessage_(result__);
			}
			onClose_();
		}
	} catch (Poco::IOException& e) {
		onError_(e.message());
	}
}

void Hrp__::checkResponse_(int timeout) /* override */ {
	unlock();
	if (timeout >= 0) {
		wait(timeout);
	} else {
		wait();
	}
	lock();
}

Poco::SynchronizedObject* Hrp__::synchronizedObject_() /* override */ {
	return this;
}

std::string Hrp__::receiveResponseHeader_() {
	Poco::Net::HTTPResponse httpResponse;
	in_ = new Poco::BinaryReader(httpClientSession_->receiveResponse(httpResponse));
	Poco::Net::HTTPResponse::HTTPStatus httpStatus = httpResponse.getStatus();
	if (httpStatus != Poco::Net::HTTPResponse::HTTP_OK) {
		throw Poco::IOException("Invalid response code: " + std::to_string(httpStatus));
	}
	const std::string& sessionId = httpResponse.get("X-Session-ID", Poco::Net::HTTPResponse::EMPTY);
	if (sessionId.empty()) {
		throw Poco::IOException("Missing session id");
	}
	inDataEncoding__ = "UTF-8";
	const std::string& contentType = httpResponse.getContentType();
	std::string::size_type index = contentType.find("; charset=");
	if (index != std::string::npos) {
		index += 10;
		inDataEncoding__ = contentType.substr(index);
		for (int i = 0; i < inDataEncoding__.length(); i++) {
			inDataEncoding__[i] = std::toupper(inDataEncoding__[i]);
		}
		if (inDataEncoding__ == "SHIFT_JIS") {
			inDataEncoding__ = "MS932";
		}
	}
	return sessionId;
}

bool Hrp__::receiveResponseBody_() {
	if (!std::getline(in_->stream(), result__)) {
		return false;
	}
	if (result__[result__.length() - 1] == '\r') {
		result__.erase(result__.length() - 1, 1);
	}
	return true;
}

} // namespace hrp
} // namespace amivoice
} // namespace com
