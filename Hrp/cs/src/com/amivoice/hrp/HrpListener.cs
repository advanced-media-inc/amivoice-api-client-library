namespace com.amivoice.hrp {

public interface HrpListener {
	void resultCreated(string sessionId);
	void resultUpdated(string result);
	void resultFinalized(string result);
	void TRACE(string message);
}

} // namespace
