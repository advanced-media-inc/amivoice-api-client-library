package com.amivoice.hrp;

public interface HrpListener {
	void resultCreated(String sessionId);
	void resultUpdated(String result);
	void resultFinalized(String result);
	void TRACE(String message);
}
