#ifndef COM_AMIVOICE_HRP_HRP___H
#define COM_AMIVOICE_HRP_HRP___H

#include <string>
#include <vector>
#include "Poco/BinaryReader.h"
#include "Poco/BinaryWriter.h"
#include "Poco/Thread.h"
#include "Poco/Net/HTTPClientSession.h"
#include "com/amivoice/hrp/Hrp.h"

namespace com {
namespace amivoice {
namespace hrp {

class Hrp__ : public Hrp, private Poco::SynchronizedObject {
	private: static bool _serverCertificateVerification;

	private: std::string resource_;
	private: Poco::Net::HTTPClientSession* httpClientSession_;
	private: Poco::BinaryWriter* out_;
	private: Poco::BinaryReader* in_;
	private: std::string requestBodyChunked_;
	private: std::string requestBodyParted_;
	private: int requestBodyPartedOffset_;
	private: std::string inDataEncoding__;
	private: std::string result__;

	public: Hrp__();
	public: virtual ~Hrp__();
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
	private: void run();
	protected: virtual void checkResponse_(int timeout) override;
	protected: virtual Poco::SynchronizedObject* synchronizedObject_() override;
	private: std::string receiveResponseHeader_();
	private: bool receiveResponseBody_();
};

} // namespace hrp
} // namespace amivoice
} // namespace com

#endif /* COM_AMIVOICE_HRP_HRP___H */
