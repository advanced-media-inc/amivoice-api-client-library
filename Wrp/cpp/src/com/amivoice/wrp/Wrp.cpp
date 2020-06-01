#include <string>
#include "Poco/Exception.h"
#include "Poco/SynchronizedObject.h"
#include "Poco/Thread.h"
#include "com/amivoice/wrp/Wrp.h"
#include "com/amivoice/wrp/Wrp_.h"
#include "com/amivoice/wrp/WrpListener.h"

static const std::string NULL_STRING = "\x7F";
static const std::string RESERVED_FRAGMENT = "";

namespace com {
namespace amivoice {
namespace wrp {

#define __STRING(x) #x
#define _STRING(x) __STRING(x)
const char* Wrp::VERSION = "Wrp/1.0.01"
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

const char* Wrp::getVersion() {
	return VERSION;
}

Wrp* Wrp::construct(int implementation /* = 1 */) {
	if (implementation == 1) {
		return new Wrp_();
	} else {
		char message[256];
		std::sprintf(message, "Unknown implementation: %d", implementation);
		throw std::runtime_error(message);
	}
}

Wrp::Wrp() {
	listener_ = NULL;
	serverURL_ = NULL_STRING;
	proxyServerName_ = NULL_STRING;
	connectTimeout_ = 0;
	receiveTimeout_ = 0;
	grammarFileNames_ = NULL_STRING;
	mode_ = NULL_STRING;
	profileId_ = NULL_STRING;
	profileWords_ = NULL_STRING;
	segmenterType_ = NULL_STRING;
	segmenterProperties_ = NULL_STRING;
	resultUpdatedInterval_ = NULL_STRING;
	extension_ = NULL_STRING;
	authorization_ = NULL_STRING;
	codec_ = NULL_STRING;
	resultType_ = NULL_STRING;
	state_ = 0;
	waitingResults_ = 0;
}

Wrp::~Wrp() {
}

void Wrp::setListener(WrpListener* listener) {
	listener_ = listener;
}

void Wrp::setServerURL(const char* serverURL) {
	serverURL_ = (serverURL == NULL) ? NULL_STRING : serverURL;
}

void Wrp::setProxyServerName(const char* proxyServerName) {
	proxyServerName_ = (proxyServerName == NULL) ? NULL_STRING : proxyServerName;
}

void Wrp::setConnectTimeout(int connectTimeout) {
	connectTimeout_ = connectTimeout;
}

void Wrp::setReceiveTimeout(int receiveTimeout) {
	receiveTimeout_ = receiveTimeout;
}

void Wrp::setGrammarFileNames(const char* grammarFileNames) {
	grammarFileNames_ = (grammarFileNames == NULL) ? NULL_STRING : grammarFileNames;
}

void Wrp::setMode(const char* mode) {
	mode_ = (mode == NULL) ? NULL_STRING : mode;
}

void Wrp::setProfileId(const char* profileId) {
	profileId_ = (profileId == NULL) ? NULL_STRING : profileId;
}

void Wrp::setProfileWords(const char* profileWords) {
	profileWords_ = (profileWords == NULL) ? NULL_STRING : profileWords;
}

void Wrp::setSegmenterType(const char* segmenterType) {
	segmenterType_ = (segmenterType == NULL) ? NULL_STRING : segmenterType;
}

void Wrp::setSegmenterProperties(const char* segmenterProperties) {
	segmenterProperties_ = (segmenterProperties == NULL) ? NULL_STRING : segmenterProperties;
}

void Wrp::setResultUpdatedInterval(const char* resultUpdatedInterval) {
	resultUpdatedInterval_ = (resultUpdatedInterval == NULL) ? NULL_STRING : resultUpdatedInterval;
}

void Wrp::setExtension(const char* extension) {
	extension_ = (extension == NULL) ? NULL_STRING : extension;
}

void Wrp::setAuthorization(const char* authorization) {
	authorization_ = (authorization == NULL) ? NULL_STRING : authorization;
}

void Wrp::setCodec(const char* codec) {
	codec_ = (codec == NULL) ? NULL_STRING : codec;
}

void Wrp::setResultType(const char* resultType) {
	resultType_ = (resultType == NULL) ? NULL_STRING : resultType;
}

void Wrp::setServiceAuthorization(const char* serviceAuthorization) {
	authorization_ = (serviceAuthorization == NULL) ? NULL_STRING : serviceAuthorization;
}

void Wrp::setVoiceDetection(const char* voiceDetection) {
	if (voiceDetection != NULL) {
		segmenterType_ = "G4";
		segmenterProperties_ = voiceDetection;
		if (strlen(voiceDetection) > 3 && voiceDetection[0] == 'G'
									   && voiceDetection[2] == ' ') {
			segmenterType_[1] = voiceDetection[1];
			segmenterProperties_ = &voiceDetection[3];
		}
	}
}

bool Wrp::connect() {
	if (isConnected_()) {
		lastMessage_ = "WARNING: already connected to WebSocket server";
		if (listener_ != NULL) {
			listener_->TRACE(lastMessage_.c_str());
		}
		return false;
	}
	if (serverURL_ == NULL_STRING) {
		lastMessage_ = "ERROR: can't connect to WebSocket server (Missing server URL)";
		if (listener_ != NULL) {
			listener_->TRACE(lastMessage_.c_str());
		}
		return false;
	}
	try {
		state_ = 0;
		waitingResults_ = 0;
		connect_(serverURL_, proxyServerName_, connectTimeout_, receiveTimeout_);
	} catch (Poco::Exception& e) {
		lastMessage_ = "ERROR: can't connect to WebSocket server (" + e.message() + "): " + serverURL_;
		if (listener_ != NULL) {
			listener_->TRACE(lastMessage_.c_str());
		}
		disconnect_();
		return false;
	}
	lastMessage_ = "INFO: connected to WebSocket server: " + serverURL_;
	if (listener_ != NULL) {
		listener_->TRACE(lastMessage_.c_str());
	}
	return true;
}

bool Wrp::disconnect() {
	if (!isConnected_()) {
		lastMessage_ = "WARNING: already disconnected from WebSocket server";
		if (listener_ != NULL) {
			listener_->TRACE(lastMessage_.c_str());
		}
		return false;
	}
	disconnect_();
	lastMessage_ = "INFO: disconnected from WebSocket server";
	if (listener_ != NULL) {
		listener_->TRACE(lastMessage_.c_str());
	}
	return true;
}

bool Wrp::isConnected() {
	return isConnected_();
}

bool Wrp::feedDataResume() {
	Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
	/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
		if (!isConnected_()) {
			lastMessage_ = "WARNING: already disconnected from WebSocket server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		if (state_ != 0) {
			lastMessage_ = "WARNING: already started feeding data to WebSocket server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		std::string command;
		command += "s ";
		if (codec_ != NULL_STRING && codec_.length() > 0) {
			command += codec_;
		} else {
			command += "16K";
		}
		if (grammarFileNames_ != NULL_STRING && grammarFileNames_.length() > 0) {
			command += ' ';
			command += grammarFileNames_;
			if (grammarFileNames_.find('\001') != std::string::npos && grammarFileNames_.find("\001", grammarFileNames_.length() - 1) == std::string::npos) {
				command += '\001';
			}
		} else {
			command += " \001";
		}
		if (mode_ != NULL_STRING) {
			command += " mode=";
			if (mode_.find(' ') != std::string::npos) {
				command += '"';
				command += mode_;
				command += '"';
			} else {
				command += mode_;
			}
		}
		if (profileId_ != NULL_STRING) {
			command += " profileId=";
			if (profileId_.find(' ') != std::string::npos) {
				command += '"';
				command += profileId_;
				command += '"';
			} else {
				command += profileId_;
			}
		}
		if (profileWords_ != NULL_STRING) {
			command += " profileWords=";
			if (profileWords_.find(' ') != std::string::npos) {
				command += '"';
				command += profileWords_;
				command += '"';
			} else {
				command += profileWords_;
			}
		}
		if (segmenterType_ != NULL_STRING) {
			command += " segmenterType=";
			if (segmenterType_.find(' ') != std::string::npos) {
				command += '"';
				command += segmenterType_;
				command += '"';
			} else {
				command += segmenterType_;
			}
		}
		if (segmenterProperties_ != NULL_STRING) {
			command += " segmenterProperties=";
			if (segmenterProperties_.find(' ') != std::string::npos) {
				command += '"';
				command += segmenterProperties_;
				command += '"';
			} else {
				command += segmenterProperties_;
			}
		}
		if (resultUpdatedInterval_ != NULL_STRING) {
			command += " resultUpdatedInterval=";
			if (resultUpdatedInterval_.find(' ') != std::string::npos) {
				command += '"';
				command += resultUpdatedInterval_;
				command += '"';
			} else {
				command += resultUpdatedInterval_;
			}
		}
		if (extension_ != NULL_STRING) {
			command += " extension=";
			if (extension_.find(' ') != std::string::npos) {
				command += '"';
				command += extension_;
				command += '"';
			} else {
				command += extension_;
			}
		}
		if (authorization_ != NULL_STRING) {
			command += " authorization=";
			if (authorization_.find(' ') != std::string::npos) {
				command += '"';
				command += authorization_;
				command += '"';
			} else {
				command += authorization_;
			}
		}
		if (resultType_ != NULL_STRING) {
			command += " resultType=";
			if (resultType_.find(' ') != std::string::npos) {
				command += '"';
				command += resultType_;
				command += '"';
			} else {
				command += resultType_;
			}
		}
		try {
			state_ = 1;
			const char* data = command.c_str();
			int dataLength = command.length();
			sendRequest_(data, 0, dataLength, (char)0);
			while (state_ == 1) {
				checkResponse_(-1);
			}
			if (state_ == 5) {
				return false;
			}
		} catch (Poco::IOException& e) {
			lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" + e.message() + ")";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		return true;
	}
}

bool Wrp::feedData(const char* data, int dataOffset, int dataBytes) {
	Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
	/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
		if (!isConnected_()) {
			lastMessage_ = "WARNING: already disconnected from WebSocket server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		if (state_ != 2 && state_ != 3) {
			lastMessage_ = "WARNING: already stopped feeding data to WebSocket server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		try {
			state_ = 3;
			sendRequest_(data, dataOffset, dataBytes, (char)'p');
		} catch (Poco::IOException& e) {
			lastMessage_ = "ERROR: can't feed data to WebSocket server (" + e.message() + ")";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		return true;
	}
}

bool Wrp::feedDataPause() {
	Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
	/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
		if (!isConnected_()) {
			lastMessage_ = "WARNING: already disconnected from WebSocket server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		if (state_ != 2 && state_ != 3) {
			lastMessage_ = "WARNING: already stopped feeding data to WebSocket server";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		try {
			state_ = 4;
			const char* data = "e";
			int dataLength = 1;
			sendRequest_(data, 0, dataLength, (char)0);
			while (state_ == 4) {
				checkResponse_(-1);
			}
			if (state_ == 5) {
				return false;
			}
		} catch (Poco::IOException& e) {
			lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" + e.message() + ")";
			if (listener_ != NULL) {
				listener_->TRACE(lastMessage_.c_str());
			}
			return false;
		}
		return true;
	}
}

const char* Wrp::getLastMessage() const {
	return lastMessage_.c_str();
}

int Wrp::getWaitingResults() const {
	return waitingResults_;
}

void Wrp::sleep(int timeout) {
	Poco::Thread::sleep((long)timeout);
}

void Wrp::onOpen_() {
}

void Wrp::onClose_() {
}

void Wrp::onError_(const std::string& cause) {
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

void Wrp::onMessage_(const std::string& message) {
	if (message.length() == 0) {
		return;
	}
	char command = message[0];
	if (command == 's') {
		if (message.length() > 1) {
			Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
			/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
				lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" + message.substr(2) + ")";
				if (listener_ != NULL) {
					listener_->TRACE(lastMessage_.c_str());
				}
				state_ = 5;
				synchronizedObject->notify();
			}
		} else {
			Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
			/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
				lastMessage_ = "INFO: started feeding data to WebSocket server";
				if (listener_ != NULL) {
					listener_->TRACE(lastMessage_.c_str());
				}
				state_ = 2;
				synchronizedObject->notify();
			}
		}
	} else
	if (command == 'p') {
		if (message.length() > 1) {
			Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
			/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
				lastMessage_ = "ERROR: can't feed data to WebSocket server (" + message.substr(2) + ")";
				if (listener_ != NULL) {
					listener_->TRACE(lastMessage_.c_str());
				}
				state_ = 5;
				synchronizedObject->notify();
			}
		} else {
			// pass
		}
	} else
	if (command == 'e') {
		if (message.length() > 1) {
			Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
			/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
				lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" + message.substr(2) + ")";
				if (listener_ != NULL) {
					listener_->TRACE(lastMessage_.c_str());
				}
				state_ = 5;
				synchronizedObject->notify();
			}
		} else {
			Poco::SynchronizedObject* synchronizedObject = synchronizedObject_();
			/* synchronized (this) */ { Poco::ScopedLock<Poco::SynchronizedObject> synchronized(*synchronizedObject);
				lastMessage_ = "INFO: stopped feeding data to WebSocket server";
				if (listener_ != NULL) {
					listener_->TRACE(lastMessage_.c_str());
				}
				state_ = 0;
				synchronizedObject->notify();
			}
		}
	} else
	if (command == 'S') {
		waitingResults_++;
		if (listener_ != NULL) {
			listener_->utteranceStarted(atoi(message.c_str() + 2));
		}
	} else
	if (command == 'E') {
		if (listener_ != NULL) {
			listener_->utteranceEnded(atoi(message.c_str() + 2));
		}
	} else
	if (command == 'C') {
		if (listener_ != NULL) {
			listener_->resultCreated();
		}
	} else
	if (command == 'U') {
		if (listener_ != NULL) {
			listener_->resultUpdated(message.c_str() + 2);
		}
	} else
	if (command == 'A') {
		if (listener_ != NULL) {
			listener_->resultFinalized(message.c_str() + 2);
		}
		waitingResults_--;
	} else
	if (command == 'R') {
		if (listener_ != NULL) {
			listener_->resultFinalized(("\001\001\001\001\001" + message.substr(2)).c_str());
		}
		waitingResults_--;
	}
}

} // namespace wrp
} // namespace amivoice
} // namespace com
