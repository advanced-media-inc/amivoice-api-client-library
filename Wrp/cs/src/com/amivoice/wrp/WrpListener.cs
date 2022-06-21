namespace com.amivoice.wrp {

public interface WrpListener {
	void utteranceStarted(int startTime);
	void utteranceEnded(int endTime);
	void resultCreated();
	void resultUpdated(string result);
	void resultFinalized(string result);
	void eventNotified(int eventId, string eventMessage);
	void TRACE(string message);
}

} // namespace
