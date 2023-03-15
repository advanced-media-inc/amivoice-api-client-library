#ifndef COM_AMIVOICE_WRP_WRP___H
#define COM_AMIVOICE_WRP_WRP___H

#include <string>
#include <vector>
#include "Poco/Thread.h"
#include "Poco/Net/WebSocket.h"
#include "com/amivoice/wrp/Wrp.h"

namespace com {
namespace amivoice {
namespace wrp {

class Wrp__ : public Wrp, private Poco::Runnable, private Poco::SynchronizedObject {
	private: static bool _serverCertificateVerification;

	private: Poco::Net::WebSocket* webSocket_;
	private: std::vector<char> outData_;
	private: std::vector<char> inData_;
	private: std::string result__;
	private: Poco::Thread* thread_;

	public: Wrp__();
	public: virtual ~Wrp__();
	protected: virtual void connect_(const std::string& serverURL, const std::string& proxyServerName, int connectTimeout, int receiveTimeout) override;
	protected: virtual void disconnect_() override;
	protected: virtual bool isConnected_() override;
	protected: virtual void sendRequest_(const char* data, int dataOffset, int dataBytes, char command) override;
	private: void run() override;
	protected: virtual void checkResponse_(int timeout) override;
	protected: virtual Poco::SynchronizedObject* synchronizedObject_() override;
	private: bool receiveResponse_();
};

} // namespace wrp
} // namespace amivoice
} // namespace com

#endif /* COM_AMIVOICE_WRP_WRP___H */
