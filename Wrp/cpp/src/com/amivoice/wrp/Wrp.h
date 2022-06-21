#ifndef COM_AMIVOICE_WRP_WRP_H
#define COM_AMIVOICE_WRP_WRP_H

#include <string>
#include "com/amivoice/wrp/WrpListener.h"

#if defined(_WIN32)
 #if defined(_USRDLL)
  #define WRP_API _declspec(dllexport)
 #else
  #define WRP_API __declspec(dllimport)
 #endif
 #define WRP_API_CALL __stdcall
#else
 #define WRP_API
 #define WRP_API_CALL
#endif

namespace Poco {

class SynchronizedObject;

} // namespace Poco

namespace com {
namespace amivoice {
namespace wrp {

class WRP_API Wrp {
	private: static const char* VERSION;

	public: static const char* getVersion();
	public: static Wrp* construct(int implementation = 1);

#if defined(_USRDLL)
	private: WrpListener* listener_;
	private: std::string serverURL_;
	private: std::string proxyServerName_;
	private: int connectTimeout_;
	private: int receiveTimeout_;
	private: std::string grammarFileNames_;
	private: std::string profileId_;
	private: std::string profileWords_;
	private: std::string segmenterProperties_;
	private: std::string keepFillerToken_;
	private: std::string resultUpdatedInterval_;
	private: std::string extension_;
	private: std::string authorization_;
	private: std::string codec_;
	private: std::string resultType_;
	private: int state_;
	private: int waitingResults_;
	private: std::string lastMessage_;
#endif

	protected: Wrp();
	public: virtual ~Wrp();
	public: void setListener(WrpListener* listener);
	public: void setServerURL(const char* serverURL);
	public: void setProxyServerName(const char* proxyServerName);
	public: void setConnectTimeout(int connectTimeout);
	public: void setReceiveTimeout(int receiveTimeout);
	public: void setGrammarFileNames(const char* grammarFileNames);
	public: void setProfileId(const char* profileId);
	public: void setProfileWords(const char* profileWords);
	public: void setSegmenterProperties(const char* segmenterProperties);
	public: void setKeepFillerToken(const char* keepFillerToken);
	public: void setResultUpdatedInterval(const char* resultUpdatedInterval);
	public: void setExtension(const char* extension);
	public: void setAuthorization(const char* authorization);
	public: void setCodec(const char* codec);
	public: void setResultType(const char* resultType);
	public: void setServiceAuthorization(const char* serviceAuthorization);
	public: bool connect();
	protected: virtual void connect_(const std::string& serverURL, const std::string& proxyServerName, int connectTimeout, int receiveTimeout) = 0;
	public: bool disconnect();
	protected: virtual void disconnect_() = 0;
	public: bool isConnected();
	protected: virtual bool isConnected_() = 0;
	public: bool feedDataResume();
	public: bool feedData(const char* data, int dataOffset, int dataBytes);
	public: bool feedDataPause();
	public: const char* getLastMessage() const;
	public: int getWaitingResults() const;
	public: void sleep(int timeout);
	protected: virtual void sendRequest_(const char* data, int dataOffset, int dataBytes, char command) = 0;
	protected: virtual void checkResponse_(int timeout) = 0;
	protected: virtual Poco::SynchronizedObject* synchronizedObject_() = 0;
	protected: void onOpen_();
	protected: void onClose_();
	protected: void onError_(const std::string& cause);
	protected: void onMessage_(const std::string& message);
};

} // namespace wrp
} // namespace amivoice
} // namespace com

#endif /* COM_AMIVOICE_WRP_WRP_H */
