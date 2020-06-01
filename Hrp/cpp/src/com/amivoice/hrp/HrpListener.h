#ifndef COM_AMIVOICE_HRP_HRPLISTENER_H
#define COM_AMIVOICE_HRP_HRPLISTENER_H

namespace com {
namespace amivoice {
namespace hrp {

class HrpListener {
	public: virtual void resultCreated(const char* sessionId) = 0;
	public: virtual void resultUpdated(const char* result) = 0;
	public: virtual void resultFinalized(const char* result) = 0;
	public: virtual void TRACE(const char* message) = 0;
};

} // namespace hrp
} // namespace amivoice
} // namespace com

#endif /* COM_AMIVOICE_HRP_HRPLISTENER_H */
