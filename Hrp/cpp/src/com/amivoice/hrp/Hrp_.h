#ifndef COM_AMIVOICE_HRP_HRP__H
#define COM_AMIVOICE_HRP_HRP__H

#include <string>
#include <vector>
#include "Poco/Thread.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/StreamSocket.h"
#include "com/amivoice/hrp/Hrp.h"

namespace com {
namespace amivoice {
namespace hrp {

class Hrp_ : public Hrp, private Poco::Runnable, private Poco::SynchronizedObject {
	private: static bool _serverCertificateVerification;

	private: static std::string _basic(const std::string& userName, const std::string& password);

	private: std::string host_;
	private: std::string resource_;
	private: std::string proxyUserName_;
	private: std::string proxyPassword_;
	private: Poco::Net::StreamSocket socket_;
	private: Poco::Net::SocketStream* out_;
	private: std::string requestBodyChunked_;
	private: std::string requestBodyParted_;
	private: int requestBodyPartedOffset_;
	private: std::vector<char> inData_;
	private: int inDataBytes_;
	private: int inDataBytes__;
	private: std::string inDataEncoding__;
	private: std::string result__;
	private: Poco::Thread* thread_;

	public: Hrp_();
	public: virtual ~Hrp_();
	protected: virtual void connect_(const std::string& serverURL, const std::string& proxyServerName, int connectTimeout, int receiveTimeout) override;
	protected: virtual void disconnect_() override;
	protected: virtual bool isConnected_() override;
	protected: virtual void sendRequestHeader_(const std::string& domainId,
											   const std::string& codec,
											   const std::string& resultType,
											   const std::string& resultEncoding,
											   const std::string& serviceAuthorization,
											   const std::string& voiceDetection,
											   const std::string& acceptTopic,
											   const std::string& contentType,
											   const std::string& accept, const char* type, long long dataBytes) override;
	protected: virtual void sendRequestBody_(const char* data, int dataOffset, int dataBytes) override;
	private: void run() override;
	protected: virtual void checkResponse_(int timeout) override;
	protected: virtual Poco::SynchronizedObject* synchronizedObject_() override;
	private: std::string receiveResponseHeader_();
	private: void receiveResponseBody_();
	private: bool receiveChunkedResponseBody_();
	private: int read_CRLFCRLF_();
	private: int find_CRLFCRLF_();
	private: int read_CRLF_();
	private: int find_CRLF_();
	private: bool read_(int inDataBytes);
	private: void ensure_(int inDataBytes);
	private: void result_(int inDataBytes);
	private: int result_CRLF_();
	private: void shift_(int inDataBytes);
};

} // namespace hrp
} // namespace amivoice
} // namespace com

#endif /* COM_AMIVOICE_HRP_HRP__H */
