#ifndef COM_AMIVOICE_HRP_HRP_H
#define COM_AMIVOICE_HRP_HRP_H

#include <string>
#include "com/amivoice/hrp/HrpListener.h"

#if defined(_WIN32)
 #if defined(_USRDLL)
  #define HRP_API _declspec(dllexport)
 #else
  #define HRP_API __declspec(dllimport)
 #endif
 #define HRP_API_CALL __stdcall
#else
 #define HRP_API
 #define HRP_API_CALL
#endif

namespace Poco {

class SynchronizedObject;

} // namespace Poco

namespace com {
namespace amivoice {
namespace hrp {

class HRP_API Hrp {
	private: static const char* VERSION;

	public: static const char* getVersion();
	public: static Hrp* construct(int implementation = 1);

#if defined(_USRDLL)
	private: HrpListener* listener_;
	private: std::string serverURL_;
	private: std::string proxyServerName_;
	private: int connectTimeout_;
	private: int receiveTimeout_;
	private: std::string grammarFileNames_;
	private: std::string mode_;
	private: std::string profileId_;
	private: std::string profileWords_;
	private: std::string segmenterType_;
	private: std::string segmenterProperties_;
	private: std::string keepFillerToken_;
	private: std::string resultUpdatedInterval_;
	private: std::string extension_;
	private: std::string authorization_;
	private: std::string domainId_;
	private: std::string codec_;
	private: std::string resultType_;
	private: std::string resultEncoding_;
	private: std::string serviceAuthorization_;
	private: std::string voiceDetection_;
	private: std::string acceptTopic_;
	private: std::string contentType_;
	private: std::string accept_;
	private: int state_;
	private: std::string lastMessage_;
#endif

	protected: Hrp();
	public: virtual ~Hrp();
	public: void setListener(HrpListener* listener);
	public: void setServerURL(const char* serverURL);
	public: void setProxyServerName(const char* proxyServerName);
	public: void setConnectTimeout(int connectTimeout);
	public: void setReceiveTimeout(int receiveTimeout);
	public: void setGrammarFileNames(const char* grammarFileNames);
	public: void setMode(const char* mode);
	public: void setProfileId(const char* profileId);
	public: void setProfileWords(const char* profileWords);
	public: void setSegmenterType(const char* segmenterType);
	public: void setSegmenterProperties(const char* segmenterProperties);
	public: void setKeepFillerToken(const char* keepFillerToken);
	public: void setResultUpdatedInterval(const char* resultUpdatedInterval);
	public: void setExtension(const char* extension);
	public: void setAuthorization(const char* authorization);
	public: void setDomainId(const char* domainId);
	public: void setCodec(const char* codec);
	public: void setResultType(const char* resultType);
	public: void setResultEncoding(const char* resultEncoding);
	public: void setServiceAuthorization(const char* serviceAuthorization);
	public: void setVoiceDetection(const char* voiceDetection);
	public: void setAcceptTopic(const char* acceptTopic);
	public: void setContentType(const char* contentType);
	public: void setAccept(const char* accept);
	public: bool connect();
	protected: virtual void connect_(const std::string& serverURL, const std::string& proxyServerName, int connectTimeout, int receiveTimeout) = 0;
	public: bool disconnect();
	protected: virtual void disconnect_() = 0;
	public: bool isConnected();
	protected: virtual bool isConnected_() = 0;
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public: bool feedDataResume();
	// type="sc" dataBytes=0 - Single part & chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public: bool feedDataResume(const char* type);
	// type="m"  dataBytes>0 - Multi parts & no chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public: bool feedDataResume(long long dataBytes);
	// type="s"  dataBytes>0 - Single part & no chunked encoding
	// type="sc" dataBytes=0 - Single part & chunked encoding
	// type="m"  dataBytes>0 - Multi parts & no chunked encoding
	// type="mc" dataBytes=0 - Multi parts & chunked encoding
	public: bool feedDataResume(const char* type, long long dataBytes);
	public: bool feedData(const char* data, int dataOffset, int dataBytes);
	public: bool feedDataPause();
	public: const char* getLastMessage() const;
	public: void sleep(int timeout);
	protected: virtual void sendRequestHeader_(const std::string& domainId,
											   const std::string& codec,
											   const std::string& resultType,
											   const std::string& resultEncoding,
											   const std::string& serviceAuthorization,
											   const std::string& voiceDetection,
											   const std::string& acceptTopic,
											   const std::string& contentType,
											   const std::string& accept, const char* type, long long dataBytes) = 0;
	protected: virtual void sendRequestBody_(const char* data, int dataOffset, int dataBytes) = 0;
	protected: virtual void checkResponse_(int timeout) = 0;
	protected: virtual Poco::SynchronizedObject* synchronizedObject_() = 0;
	protected: void onOpen_(const std::string& sessionId);
	protected: void onClose_();
	protected: void onError_(const std::string& cause);
	protected: void onMessage_(const std::string& resultData);
};

} // namespace hrp
} // namespace amivoice
} // namespace com

#endif /* COM_AMIVOICE_HRP_HRP_H */
