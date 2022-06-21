#ifndef COM_AMIVOICE_WRP_WRPLISTENER_H
#define COM_AMIVOICE_WRP_WRPLISTENER_H

namespace com {
namespace amivoice {
namespace wrp {

class WrpListener {
	public: virtual void utteranceStarted(int startTime) = 0;
	public: virtual void utteranceEnded(int endTime) = 0;
	public: virtual void resultCreated() = 0;
	public: virtual void resultUpdated(const char* result) = 0;
	public: virtual void resultFinalized(const char* result) = 0;
	public: virtual void eventNotified(int eventId, const char* eventMessage) = 0;
	public: virtual void TRACE(const char* message) = 0;
};

} // namespace wrp
} // namespace amivoice
} // namespace com

#endif /* COM_AMIVOICE_WRP_WRPLISTENER_H */
