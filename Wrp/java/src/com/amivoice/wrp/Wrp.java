package com.amivoice.wrp;

import java.io.*;

public abstract class Wrp {
	private static final String VERSION = "Wrp/1.0.03 Java/" + System.getProperty("java.version") + " (" + System.getProperty("os.name") + " " + System.getProperty("os.version") + ")";

	public static String getVersion() {
		return VERSION;
	}

	public static Wrp construct() {
		return new Wrp_();
	}

	public static Wrp construct(int implementation) {
		try {
			if (implementation == 1) {
				return (Wrp)Class.forName("com.amivoice.wrp.Wrp_").getDeclaredConstructor().newInstance();
			} else
			if (implementation == 2) {
				return (Wrp)Class.forName("com.amivoice.wrp.Wrp__").getDeclaredConstructor().newInstance();
			} else
			if (implementation == 3) {
				return (Wrp)Class.forName("com.amivoice.wrp.Wrp___").getDeclaredConstructor().newInstance();
			} else {
				throw new IllegalArgumentException("Unknown implementation: " + implementation);
			}
		} catch (Exception e) {
			throw new IllegalArgumentException("Unknown implementation: " + implementation);
		}
	}

	private WrpListener listener_;
	private String serverURL_;
	private String proxyServerName_;
	private int connectTimeout_;
	private int receiveTimeout_;
	private String grammarFileNames_;
	private String mode_;
	private String profileId_;
	private String profileWords_;
	private String segmenterType_;
	private String segmenterProperties_;
	private String keepFillerToken_;
	private String resultUpdatedInterval_;
	private String extension_;
	private String authorization_;
	private String codec_;
	private String resultType_;
	private int state_;
	private String lastMessage_;
	private volatile int waitingResults_;

	protected Wrp() {
		listener_ = null;
		serverURL_ = null;
		proxyServerName_ = null;
		connectTimeout_ = 0;
		receiveTimeout_ = 0;
		grammarFileNames_ = null;
		mode_ = null;
		profileId_ = null;
		profileWords_ = null;
		segmenterType_ = null;
		segmenterProperties_ = null;
		keepFillerToken_ = null;
		resultUpdatedInterval_ = null;
		extension_ = null;
		authorization_ = null;
		codec_ = null;
		resultType_ = null;
		state_ = 0;
		waitingResults_ = 0;
	}

	public void setListener(WrpListener listener) {
		listener_ = listener;
	}

	public void setServerURL(String serverURL) {
		serverURL_ = serverURL;
		if (serverURL_ != null) {
			if (serverURL_.startsWith("http://")) {
				serverURL_ = "ws://" + serverURL_.substring(7);
			} else
			if (serverURL_.startsWith("https://")) {
				serverURL_ = "wss://" + serverURL_.substring(8);
			}
		}
	}

	public void setProxyServerName(String proxyServerName) {
		proxyServerName_ = proxyServerName;
	}

	public void setConnectTimeout(int connectTimeout) {
		connectTimeout_ = connectTimeout;
	}

	public void setReceiveTimeout(int receiveTimeout) {
		receiveTimeout_ = receiveTimeout;
	}

	public void setGrammarFileNames(String grammarFileNames) {
		grammarFileNames_ = grammarFileNames;
	}

	public void setMode(String mode) {
		mode_ = mode;
	}

	public void setProfileId(String profileId) {
		profileId_ = profileId;
	}

	public void setProfileWords(String profileWords) {
		profileWords_ = profileWords;
	}

	public void setSegmenterType(String segmenterType) {
		segmenterType_ = segmenterType;
	}

	public void setSegmenterProperties(String segmenterProperties) {
		segmenterProperties_ = segmenterProperties;
	}

	public void setKeepFillerToken(String keepFillerToken) {
		keepFillerToken_ = keepFillerToken;
	}

	public void setResultUpdatedInterval(String resultUpdatedInterval) {
		resultUpdatedInterval_ = resultUpdatedInterval;
	}

	public void setExtension(String extension) {
		extension_ = extension;
	}

	public void setAuthorization(String authorization) {
		authorization_ = authorization;
	}

	public void setCodec(String codec) {
		codec_ = codec;
	}

	public void setResultType(String resultType) {
		resultType_ = resultType;
	}

	public void setServiceAuthorization(String serviceAuthorization) {
		if (serviceAuthorization != null) {
			authorization_ = serviceAuthorization;
		}
	}

	public void setVoiceDetection(String voiceDetection) {
		if (voiceDetection != null) {
			segmenterType_ = "G4";
			segmenterProperties_ = voiceDetection;
			if (voiceDetection.length() > 3 && voiceDetection.charAt(0) == 'G'
											&& voiceDetection.charAt(2) == ' ') {
				segmenterType_ = voiceDetection.substring(0, 2);
				segmenterProperties_ = voiceDetection.substring(3);
			}
		}
	}

	public boolean connect() {
		if (isConnected_()) {
			lastMessage_ = "WARNING: already connected to WebSocket server";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			return false;
		}
		if (serverURL_ == null) {
			lastMessage_ = "ERROR: can't connect to WebSocket server (Missing server URL)";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			return false;
		}
		try {
			state_ = 0;
			waitingResults_ = 0;
			connect_(serverURL_, proxyServerName_, connectTimeout_, receiveTimeout_);
		} catch (Exception e) {
			lastMessage_ = "ERROR: can't connect to WebSocket server (" + e.getMessage() + "): " + serverURL_;
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			disconnect_();
			return false;
		}
		lastMessage_ = "INFO: connected to WebSocket server: " + serverURL_;
		if (listener_ != null) {
			listener_.TRACE(lastMessage_);
		}
		return true;
	}

	protected abstract void connect_(String serverURL, String proxyServerName, int connectTimeout, int receiveTimeout) throws Exception;

	public boolean disconnect() {
		if (!isConnected_()) {
			lastMessage_ = "WARNING: already disconnected from WebSocket server";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			return false;
		}
		disconnect_();
		lastMessage_ = "INFO: disconnected from WebSocket server";
		if (listener_ != null) {
			listener_.TRACE(lastMessage_);
		}
		return true;
	}

	protected abstract void disconnect_();

	public boolean isConnected() {
		return isConnected_();
	}

	protected abstract boolean isConnected_();

	public boolean feedDataResume() {
		synchronized (this) {
			if (!isConnected_()) {
				lastMessage_ = "WARNING: already disconnected from WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			if (state_ != 0) {
				lastMessage_ = "WARNING: already started feeding data to WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			StringBuilder command = new StringBuilder();
			command.append("s ");
			if (codec_ != null && codec_.length() > 0) {
				command.append(codec_);
			} else {
				command.append("16K");
			}
			if (grammarFileNames_ != null && grammarFileNames_.length() > 0) {
				command.append(' ');
				command.append(grammarFileNames_);
				if (grammarFileNames_.indexOf('\001') != -1 && !grammarFileNames_.endsWith("\001")) {
					command.append('\001');
				}
			} else {
				command.append(" \001");
			}
			if (mode_ != null) {
				command.append(" mode=");
				if (mode_.indexOf(' ') != -1) {
					command.append('"');
					command.append(mode_);
					command.append('"');
				} else {
					command.append(mode_);
				}
			}
			if (profileId_ != null) {
				command.append(" profileId=");
				if (profileId_.indexOf(' ') != -1) {
					command.append('"');
					command.append(profileId_);
					command.append('"');
				} else {
					command.append(profileId_);
				}
			}
			if (profileWords_ != null) {
				command.append(" profileWords=");
				if (profileWords_.indexOf(' ') != -1) {
					command.append('"');
					command.append(profileWords_);
					command.append('"');
				} else {
					command.append(profileWords_);
				}
			}
			if (segmenterType_ != null) {
				command.append(" segmenterType=");
				if (segmenterType_.indexOf(' ') != -1) {
					command.append('"');
					command.append(segmenterType_);
					command.append('"');
				} else {
					command.append(segmenterType_);
				}
			}
			if (segmenterProperties_ != null) {
				command.append(" segmenterProperties=");
				if (segmenterProperties_.indexOf(' ') != -1) {
					command.append('"');
					command.append(segmenterProperties_);
					command.append('"');
				} else {
					command.append(segmenterProperties_);
				}
			}
			if (keepFillerToken_ != null) {
				command.append(" keepFillerToken=");
				if (keepFillerToken_.indexOf(' ') != -1) {
					command.append('"');
					command.append(keepFillerToken_);
					command.append('"');
				} else {
					command.append(keepFillerToken_);
				}
			}
			if (resultUpdatedInterval_ != null) {
				command.append(" resultUpdatedInterval=");
				if (resultUpdatedInterval_.indexOf(' ') != -1) {
					command.append('"');
					command.append(resultUpdatedInterval_);
					command.append('"');
				} else {
					command.append(resultUpdatedInterval_);
				}
			}
			if (extension_ != null) {
				command.append(" extension=");
				if (extension_.indexOf(' ') != -1) {
					command.append('"');
					command.append(extension_.replace("\"", "\"\""));
					command.append('"');
				} else {
					command.append(extension_);
				}
			}
			if (authorization_ != null) {
				command.append(" authorization=");
				if (authorization_.indexOf(' ') != -1) {
					command.append('"');
					command.append(authorization_);
					command.append('"');
				} else {
					command.append(authorization_);
				}
			}
			if (resultType_ != null) {
				command.append(" resultType=");
				if (resultType_.indexOf(' ') != -1) {
					command.append('"');
					command.append(resultType_);
					command.append('"');
				} else {
					command.append(resultType_);
				}
			}
			try {
				state_ = 1;
				byte[] data = command.toString().getBytes("UTF-8");
				sendRequest_(data, 0, data.length, (byte)0);
				while (state_ == 1) {
					wait();
				}
				if (state_ == 5) {
					return false;
				}
			} catch (InterruptedException e) {
				lastMessage_ = "WARNING: interrupted to start feeding data to WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			} catch (IOException e) {
				lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" + e.getMessage() + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public boolean feedData(byte[] data, int dataOffset, int dataBytes) {
		synchronized (this) {
			if (!isConnected_()) {
				lastMessage_ = "WARNING: already disconnected from WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			if (state_ != 2 && state_ != 3) {
				lastMessage_ = "WARNING: already stopped feeding data to WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			try {
				state_ = 3;
				sendRequest_(data, dataOffset, dataBytes, (byte)'p');
			} catch (IOException e) {
				lastMessage_ = "ERROR: can't feed data to WebSocket server (" + e.getMessage() + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public boolean feedDataPause() {
		synchronized (this) {
			if (!isConnected_()) {
				lastMessage_ = "WARNING: already disconnected from WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			if (state_ != 2 && state_ != 3) {
				lastMessage_ = "WARNING: already stopped feeding data to WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			try {
				state_ = 4;
				byte[] data = "e".getBytes();
				sendRequest_(data, 0, data.length, (byte)0);
				while (state_ == 4) {
					wait();
				}
				if (state_ == 5) {
					return false;
				}
			} catch (InterruptedException e) {
				lastMessage_ = "WARNING: interrupted to stop feeding data to WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			} catch (IOException e) {
				lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" + e.getMessage() + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public String getLastMessage() {
		return lastMessage_;
	}

	public int getWaitingResults() {
		return waitingResults_;
	}

	public void sleep(int timeout) {
		try {
			Thread.sleep((long)timeout);
		} catch (InterruptedException e) {}
	}

	protected abstract void sendRequest_(byte[] data, int dataOffset, int dataBytes, byte command) throws IOException;

	protected void onOpen_() {
	}

	protected void onClose_() {
	}

	protected void onError_(Throwable cause) {
		synchronized (this) {
			if (state_ == 0 || state_ == 5) {
				return;
			}
			lastMessage_ = "ERROR: caught exception (" + cause.getMessage() + ")";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			state_ = 5;
			notifyAll();
		}
	}

	protected void onMessage_(String message) {
		if (message.length() == 0) {
			return;
		}
		char command = message.charAt(0);
		if (command == 's') {
			if (message.length() > 1) {
				synchronized (this) {
					lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" + message.substring(2) + ")";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 5;
					notifyAll();
				}
			} else {
				synchronized (this) {
					lastMessage_ = "INFO: started feeding data to WebSocket server";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 2;
					notifyAll();
				}
			}
		} else
		if (command == 'p') {
			if (message.length() > 1) {
				synchronized (this) {
					lastMessage_ = "ERROR: can't feed data to WebSocket server (" + message.substring(2) + ")";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 5;
					notifyAll();
				}
			} else {
				// pass
			}
		} else
		if (command == 'e') {
			if (message.length() > 1) {
				synchronized (this) {
					lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" + message.substring(2) + ")";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 5;
					notifyAll();
				}
			} else {
				synchronized (this) {
					lastMessage_ = "INFO: stopped feeding data to WebSocket server";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 0;
					notifyAll();
				}
			}
		} else
		if (command == 'S') {
			waitingResults_++;
			if (listener_ != null) {
				listener_.utteranceStarted(Integer.parseInt(message.substring(2)));
			}
		} else
		if (command == 'E') {
			if (listener_ != null) {
				listener_.utteranceEnded(Integer.parseInt(message.substring(2)));
			}
		} else
		if (command == 'C') {
			if (listener_ != null) {
				listener_.resultCreated();
			}
		} else
		if (command == 'U') {
			if (listener_ != null) {
				listener_.resultUpdated(message.substring(2));
			}
		} else
		if (command == 'A') {
			if (listener_ != null) {
				listener_.resultFinalized(message.substring(2));
			}
			waitingResults_--;
		} else
		if (command == 'R') {
			if (listener_ != null) {
				listener_.resultFinalized("\001\001\001\001\001" + message.substring(2));
			}
			waitingResults_--;
		} else
		if (command == 'Q') {
			if (listener_ != null) {
				listener_.eventNotified((int)command, message.substring(2));
			}
		} else
		if (command == 'G') {
			if (listener_ != null) {
				listener_.eventNotified((int)command, message.substring(2));
			}
		}
	}
}
