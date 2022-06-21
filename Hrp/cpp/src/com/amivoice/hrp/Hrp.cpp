#include <string>
#include "Poco/Exception.h"
#include "Poco/SynchronizedObject.h"
#include "Poco/Thread.h"
#include "Poco/URI.h"
#include "com/amivoice/hrp/Hrp.h"
#include "com/amivoice/hrp/Hrp_.h"
#include "com/amivoice/hrp/Hrp__.h"
#include "com/amivoice/hrp/HrpListener.h"

static const std::string NULL_STRING = "\x7F";
static const std::string RESERVED_FRAGMENT = "";

namespace com {
namespace amivoice {
namespace hrp {

#define __STRING(x) #x
#define _STRING(x) __STRING(x)
const char* Hrp::VERSION = "Hrp/1.0.03"
#ifdef _MSC_VER
	" MSVC/" _STRING(_MSC_VER)
#endif
#ifdef __GNUC__
	" GCC/" _STRING(__GNUC__) "." _STRING(__GNUC_MINOR__)
#endif
#ifdef _WIN32
	" (Windows)"
#endif
#ifdef __unix__
	" (Linux)"
#endif
#ifdef __CYGWIN__
	" (Cygwin)"
#endif
#ifdef __MINGW32__
	" (MinGW)"
#endif
#ifdef __MINGW64__
	" (MinGW-w64)"
#endif
	;

const char* Hrp::getVersion() {
	return VERSION;
}

Hrp* Hrp::construct(int implementation /* = 1 */) {
	if (implementation == 1) {
		return new Hrp_();
	} else
	if (implementation == 2) {
		return new Hrp__();
	} else {
		char message[256];
		std::sprintf(message, "Unknown implementation: %d", implementation);
		throw std::runtime_error(message);
	}
}

Hrp::Hrp() {
	listener_ = NULL;
	serverURL_ = NULL_STRING;
	proxyServerName_ = NULL_STRING;
	connectTimeout_ = 0;
	receiveTimeout_ = 0;
	grammarFileNames_ = NULL_STRING;
	profileId_ = NULL_STRING;
	profileWords_ = NULL_STRING;
	segmenterProperties_ = NULL_STRING;
	keepFillerToken_ = NULL_STRING;
	resultUpdatedInterval_ = NULL_STRING;
	extension_ = NULL_STRING;
	authorization_ = NULL_STRING;
	domainId_ = NULL_STRING;
	codec_ = NULL_STRING;
	resultType_ = NULL_STRING;
	resultEncoding_ = NULL_STRING;
	serviceAuthorization_ = NULL_STRING;
	voiceDetection_ = NULL_STRING;
	acceptTopic_ = NULL_STRING;
	contentType_ = NULL_STRING;
	accept_ = NULL_STRING;
	state_ = 0;
}

Hrp::~Hrp() {
}

void Hrp::setListener(HrpListener* listener) {
	listener_ = listener;
}

void Hrp::setServerURL(const char* serverURL) {
	serverURL_ = (serverURL == NULL) ? NULL_STRING : serverURL;
}

void Hrp::setProxyServerName(const char* proxyServerName) {
	proxyServerName_ = (proxyServerName == NULL) ? NULL_STRING : proxyServerName;
}

void Hrp::setConnectTimeout(int connectTimeout) {
	connectTimeout_ = connectTimeout;
}

void Hrp::setReceiveTimeout(int receiveTimeout) {
	receiveTimeout_ = receiveTimeout;
}

void Hrp::setGrammarFileNames(const char* grammarFileNames) {
	grammarFileNames_ = (grammarFileNames == NULL) ? NULL_STRING : grammarFileNames;
}

void Hrp::setProfileId(const char* profileId) {
	profileId_ = (profileId == NULL) ? NULL_STRING : profileId;
}

void Hrp::setProfileWords(const char* profileWords) {
	profileWords_ = (profileWords == NULL) ? NULL_STRING : profileWords;
}

void Hrp::setSegmenterProperties(const char* segmenterProperties) {
	segmenterProperties_ = (segmenterProperties == NULL) ? NULL_STRING : segmenterProperties;
}

void Hrp::setKeepFillerToken(const char* keepFillerToken) {
	keepFillerToken_ = (keepFillerToken == NULL) ? NULL_STRING : keepFillerToken;
}

void Hrp::setResultUpdatedInterval(const char* resultUpdatedInterval) {
	resultUpdatedInterval_ = (resultUpdatedInterval == NULL) ? NULL_STRING : resultUpdatedInterval;
}

void Hrp::setExtension(const char* extension) {
	extension_ = (extension == NULL) ? NULL_STRING : extension;
}

void Hrp::setAuthorization(const char* authorization) {
	authorization_ = (authorization == NULL) ? NULL_STRING : authorization;
}

void Hrp::setDomainId(const char* domainId) {
	domainId_ = (domainId == NULL) ? NULL_STRING : domainId;
}

void Hrp::setCodec(const char* codec) {
	codec_ = (codec == NULL) ? NULL_STRING : codec;
}

void Hrp::setResultType(const char* resultType) {
	resultType_ = (resultType == NULL) ? NULL_STRING : resultType;
}

void Hrp::setResultEncoding(const char* resultEncoding) {
	resultEncoding_ = (resultEncoding == NULL) ? NULL_STRING : resultEncoding;
}

void Hrp::setServiceAuthorization(const char* serviceAuthorization) {
	serviceAuthorization_ = (serviceAuthorization == NULL) ? NULL_STRING : serviceAuthorization;
}

void Hrp::setVoiceDetection(const char* voiceDetection) {
	voiceDetection_ = (voiceDetection == NULL) ? NULL_STRING : voiceDetection;
}

void Hrp::setAcceptTopic(const char* acceptTopic) {
	acceptTopic_ = (acceptTopic == NULL) ? NULL_STRING : acceptTopic;
}

void Hrp::setContentType(const char* contentType) {
	contentType_ = (contentType == NULL) ? NULL_STRING : contentType;
}

void Hrp::setAccept(const char* accept) {
	accept_ = (accept == NULL) ? NULL_STRING : accept;
}

bool Hrp::connect() {
	if (isConnected_()) {
		lastMessage_ = "WARNING: already connected to HTTP server";
		if (listener_ != NULL) {
			listener_->TRACE(lastMessage_.c_str());
		}
		return false;
	}
	if (serverURL_ == NULL_STRING) {
		lastMessage_ = "ERROR: can't connect to HTTP server (Missing server URL)";
		if (listener_ != NULL) {
			listener_->TRACE(lastMessage_.c_str());
		}
		return false;
	}
	try {
		state_ = 0;
		connect_(serverURL_, proxyServerName_, connectTimeout_, receiveTimeout_);
	} catch (Poco::Exception& e) {
		lastMessage_ = "ERROR: can't connect to HTTP server (" + e.message() + "): " + serverURL_;
		if (listener_ != NULL) {
			listener_->TRACE(lastMessage_.c_str());
		}
		disconnect_();
		return false;
	}
	lastMessage_ = "INFO: connected to HTTP server: " + serverURL_;
	if (listener_ != NULL) {
		listener_->TRACE(lastMessage_.c_str());
	}
	return true;
}

bool Hrp::disconnect() {
	if (!isConnected_()) {
		lastMessage_ = "WARNING: already disconnected from HTTP server";
		if (listener_ != NULL) {
			listener_->TRACE(lastMessage_.c_str());
		}
		return false;
	}
	disconnect_();
	lastMessage_ = "INFO: disconnected from HTTP server";
	if (listener_ != NULL) {
		listener_->TRACE(lastMessage_.c_str());
	}
	return true;
}

bool Hrp::isConnected() {
	return isConnected_();
}

// type="mc" dataBytes=0 - Multi parts & chunked encoding
bool Hrp::feedDataResume() {
	return feedDataResume(/* type */ "mc", /* dataBytes */ 0);
}

// type="sc" dataBytes=0 - Single part & chunked encoding
// type="mc" dataBytes=0 - Multi parts & chunked encoding
bool Hrp::feedDataResume(const char* type) {
	return feedDataResume(type, /* dataBytes */ 0);
}

// type="m"  dataBytes>0 - Multi parts & no chunked encoding
// type="mc" dataBytes=0 - Multi parts & chunked encoding
bool Hrp::feedDataResume(long long dataBytes) {
	return feedDataResume(/* type */ "mc", dataBytes);
}

// type="s"  dataBytes>0 - Single part & no chunked encoding
// type="sc" dataBytes=0 - Single part & chunked encoding
// type="m"  dataBytes>0 - Multi parts & no chunked encoding
// type="mc" dataBytes=0 - Multi parts & chunked encoding
bool Hrp::feedDataResume(const char* type, long long dataBytes) {
	if (type == NULL) {
		type = "mc";
	}
	if (dataBytes > 0) {
		if (std::strchr(type, 'm') == NULL) {
			if (std::strchr(type, 'c') == NULL) {
				type = "s ";
			} else {
				type = "sc";
			}
		} else {
			if (std::strchr(type, 'c') == NULL) {
				type = "m ";
			} else {
				type = "mc";
			}
		}
	} else {
		if (std::strchr(type, 'm') == NULL) {
			type = "sc";
		} else {
			type = "mc";
		}
	}
	Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
	/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
		if (!isConnected_()) {
			lastMessage_ = "WARNING: already disconnected from HTTP server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		if (state_ != 0) {
			lastMessage_ = "WARNING: already started feeding data to HTTP server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		std::string domainId;
		if (domainId_ != NULL_STRING) {
			domainId += domainId_;
		} else {
			if (grammarFileNames_ != NULL_STRING) {
				domainId += "grammarFileNames=";
				Poco::URI::encode(grammarFileNames_, RESERVED_FRAGMENT, domainId);
			}
			if (profileId_ != NULL_STRING) {
				if (domainId.length() > 0) {
					domainId += ' ';
				}
				domainId += "profileId=";
				Poco::URI::encode(profileId_, RESERVED_FRAGMENT, domainId);
			}
			if (profileWords_ != NULL_STRING) {
				if (domainId.length() > 0) {
					domainId += ' ';
				}
				domainId += "profileWords=";
				Poco::URI::encode(profileWords_, RESERVED_FRAGMENT, domainId);
			}
			if (segmenterProperties_ != NULL_STRING) {
				if (domainId.length() > 0) {
					domainId += ' ';
				}
				domainId += "segmenterProperties=";
				Poco::URI::encode(segmenterProperties_, RESERVED_FRAGMENT, domainId);
			}
			if (keepFillerToken_ != NULL_STRING) {
				if (domainId.length() > 0) {
					domainId += ' ';
				}
				domainId += "keepFillerToken=";
				Poco::URI::encode(keepFillerToken_, RESERVED_FRAGMENT, domainId);
			}
			if (resultUpdatedInterval_ != NULL_STRING) {
				if (domainId.length() > 0) {
					domainId += ' ';
				}
				domainId += "resultUpdatedInterval=";
				Poco::URI::encode(resultUpdatedInterval_, RESERVED_FRAGMENT, domainId);
			}
			if (extension_ != NULL_STRING) {
				if (domainId.length() > 0) {
					domainId += ' ';
				}
				domainId += "extension=";
				Poco::URI::encode(extension_, RESERVED_FRAGMENT, domainId);
			}
			if (authorization_ != NULL_STRING) {
				if (domainId.length() > 0) {
					domainId += ' ';
				}
				domainId += "authorization=";
				Poco::URI::encode(authorization_, RESERVED_FRAGMENT, domainId);
			}
		}
		try {
			state_ = 1;
			sendRequestHeader_(domainId,
							   codec_,
							   resultType_,
							   resultEncoding_,
							   serviceAuthorization_,
							   voiceDetection_,
							   acceptTopic_,
							   contentType_,
							   accept_, type, dataBytes);
			state_ = 2;
			lastMessage_ = "INFO: started feeding data to HTTP server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
		} catch (Poco::IOException& e) {
			lastMessage_ = "ERROR: can't start feeding data to HTTP server (" + e.message() + ")";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		return true;
	}
}

bool Hrp::feedData(const char* data, int dataOffset, int dataBytes) {
	Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
	/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
		if (!isConnected_()) {
			lastMessage_ = "WARNING: already disconnected from HTTP server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		if (state_ != 2 && state_ != 3) {
			lastMessage_ = "WARNING: already stopped feeding data to HTTP server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		try {
			state_ = 3;
			sendRequestBody_(data, dataOffset, dataBytes);
		} catch (Poco::IOException& e) {
			lastMessage_ = "ERROR: can't feed data to HTTP server (" + e.message() + ")";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		return true;
	}
}

bool Hrp::feedDataPause() {
	Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
	/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
		if (!isConnected_()) {
			lastMessage_ = "WARNING: already disconnected from HTTP server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		if (state_ != 2 && state_ != 3) {
			lastMessage_ = "WARNING: already stopped feeding data to HTTP server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		try {
			state_ = 4;
			sendRequestBody_(NULL, 0, 0);
			while (state_ == 4) {
				checkResponse_(-1);
			}
			if (state_ == 5) {
				lastMessage_ = "ERROR: can't stop feeding data to HTTP server";
				if (listener_ != NULL) {
					listener_->TRACE(lastMessage_.c_str());
				}
				return false;
			}
			lastMessage_ = "INFO: stopped feeding data to HTTP server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
		} catch (Poco::IOException& e) {
			lastMessage_ = "ERROR: can't stop feeding data to HTTP server (" + e.message() + ")";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		return true;
	}
}

const char* Hrp::getLastMessage() const {
	return lastMessage_.c_str();
}

void Hrp::sleep(int timeout) {
	Poco::Thread::sleep((long)timeout);
}

void Hrp::onOpen_(const std::string& sessionId) {
	if (listener_ != NULL) {
		listener_->resultCreated(sessionId.c_str());
	}
}

void Hrp::onClose_() {
	Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
	/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
		state_ = 0;
		synchronizedObject->notify();
	}
}

void Hrp::onError_(const std::string& cause) {
	Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
	/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
		if (state_ == 0 || state_ == 5) {
			return;
		}
		lastMessage_ = "ERROR: caught exception (" + cause + ")";
		if (listener_ != NULL) {
			listener_->TRACE(lastMessage_.c_str());
		}
		state_ = 5;
		synchronizedObject->notify();
	}
}

void Hrp::onMessage_(const std::string& resultData) {
	if (listener_ != NULL) { // {
		if (resultData.length() - 3 >= 0 && resultData.find("...", resultData.length() - 3) != std::string::npos || resultData.length() - 5 >= 0 && resultData.find("...\"}", resultData.length() - 5) != std::string::npos) {
			listener_->resultUpdated(resultData.c_str());
		} else {
			listener_->resultFinalized(resultData.c_str());
		}
	}
}

} // namespace hrp
} // namespace amivoice
} // namespace com
