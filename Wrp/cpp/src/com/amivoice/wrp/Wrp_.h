#ifndef COM_AMIVOICE_WRP_WRP__H
#define COM_AMIVOICE_WRP_WRP__H

#include <string>
#include <vector>
#include "Poco/Thread.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/StreamSocket.h"
#include "com/amivoice/wrp/Wrp.h"

namespace com {
namespace amivoice {
namespace wrp {

class Wrp_ : public Wrp, private Poco::Runnable, private Poco::SynchronizedObject {
	private: static bool _serverCertificateVerification;
	private: static const char _base64Table[];

	private: static std::string _basic(const std::string& userName, const std::string& password);

	private: Poco::Net::StreamSocket socket_;
	private: Poco::Net::SocketStream* out_;
	private: std::vector<char> outData_;
	private: std::vector<char> inData_;
	private: int inDataBytes_;
	private: std::string result__;
	private: Poco::Thread* thread_;

	public: Wrp_();
	public: virtual ~Wrp_();
	protected: virtual void connect_(const std::string& serverURL, const std::string& proxyServerName, int connectTimeout, int receiveTimeout) override;
	protected: virtual void disconnect_() override;
	protected: virtual bool isConnected_() override;
	protected: virtual void sendRequest_(const char* data, int dataOffset, int dataBytes, char command) override;
	private: void run() override;
	protected: virtual void checkResponse_(int timeout) override;
	protected: virtual Poco::SynchronizedObject* synchronizedObject_() override;
	private: bool receiveResponse_();
	private: int read_CRLFCRLF_();
	private: int find_CRLFCRLF_();
	private: bool read_(int inDataBytes);
	private: void ensure_(int inDataBytes);
	private: void result_(int inDataBytes);
	private: void shift_(int inDataBytes);
};

} // namespace wrp
} // namespace amivoice
} // namespace com

#endif /* COM_AMIVOICE_WRP_WRP__H */
