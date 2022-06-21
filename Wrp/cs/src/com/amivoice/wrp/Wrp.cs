using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

#if !UNITY_2018_4_OR_NEWER
[assembly: AssemblyTitle("Wrp")]
[assembly: AssemblyProduct("Wrp")]
[assembly: AssemblyCopyright("Copyright (C) 2019-2021 Advanced Media, Inc.")]
[assembly: ComVisible(false)]
[assembly: Guid("d605a501-80ec-4f48-a8fb-2a70e627a256")]
[assembly: AssemblyVersion("1.0.03")]
[assembly: AssemblyFileVersion("1.0.03")]
#endif

namespace com.amivoice.wrp {

public abstract class Wrp {
	private static string VERSION = "Wrp/1.0.03 CLR/" + System.Environment.Version.ToString() + " (" + System.Environment.OSVersion.ToString() + ")";

	public static string getVersion() {
		return VERSION;
	}

	public static Wrp construct() {
		return new Wrp_();
	}

	public static Wrp construct(int implementation) {
		if (implementation == 1) {
			return new Wrp_();
		} else {
			throw new ArgumentException("Unknown implementation: " + implementation);
		}
	}

	private WrpListener listener_;
	private string serverURL_;
	private string proxyServerName_;
	private int connectTimeout_;
	private int receiveTimeout_;
	private string grammarFileNames_;
	private string profileId_;
	private string profileWords_;
	private string segmenterProperties_;
	private string keepFillerToken_;
	private string resultUpdatedInterval_;
	private string extension_;
	private string authorization_;
	private string codec_;
	private string resultType_;
	private int state_;
	private string lastMessage_;
	private volatile int waitingResults_;

	protected Wrp() {
		listener_ = null;
		serverURL_ = null;
		proxyServerName_ = null;
		connectTimeout_ = 0;
		receiveTimeout_ = 0;
		grammarFileNames_ = null;
		profileId_ = null;
		profileWords_ = null;
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

	public void setServerURL(string serverURL) {
		serverURL_ = serverURL;
		if (serverURL_ != null) {
			if (serverURL_.StartsWith("http://")) {
				serverURL_ = "ws://" + serverURL_.Substring(7);
			} else
			if (serverURL_.StartsWith("https://")) {
				serverURL_ = "wss://" + serverURL_.Substring(8);
			}
		}
	}

	public void setProxyServerName(string proxyServerName) {
		proxyServerName_ = proxyServerName;
	}

	public void setConnectTimeout(int connectTimeout) {
		connectTimeout_ = connectTimeout;
	}

	public void setReceiveTimeout(int receiveTimeout) {
		receiveTimeout_ = receiveTimeout;
	}

	public void setGrammarFileNames(string grammarFileNames) {
		grammarFileNames_ = grammarFileNames;
	}

	public void setProfileId(string profileId) {
		profileId_ = profileId;
	}

	public void setProfileWords(string profileWords) {
		profileWords_ = profileWords;
	}

	public void setSegmenterProperties(string segmenterProperties) {
		segmenterProperties_ = segmenterProperties;
	}

	public void setKeepFillerToken(string keepFillerToken) {
		keepFillerToken_ = keepFillerToken;
	}

	public void setResultUpdatedInterval(string resultUpdatedInterval) {
		resultUpdatedInterval_ = resultUpdatedInterval;
	}

	public void setExtension(string extension) {
		extension_ = extension;
	}

	public void setAuthorization(string authorization) {
		authorization_ = authorization;
	}

	public void setCodec(string codec) {
		codec_ = codec;
	}

	public void setResultType(string resultType) {
		resultType_ = resultType;
	}

	public void setServiceAuthorization(string serviceAuthorization) {
		if (serviceAuthorization != null) {
			authorization_ = serviceAuthorization;
		}
	}

	public bool connect() {
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
			lastMessage_ = "ERROR: can't connect to WebSocket server (" + e.Message + "): " + serverURL_;
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

	protected abstract void connect_(string serverURL, string proxyServerName, int connectTimeout, int receiveTimeout);

	public bool disconnect() {
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

	public bool isConnected() {
		return isConnected_();
	}

	protected abstract bool isConnected_();

	public bool feedDataResume() {
		lock (this) {
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
			command.Append("s ");
			if (codec_ != null && codec_.Length > 0) {
				command.Append(codec_);
			} else {
				command.Append("16K");
			}
			if (grammarFileNames_ != null && grammarFileNames_.Length > 0) {
				command.Append(' ');
				command.Append(grammarFileNames_);
				if (grammarFileNames_.IndexOf('\x01') != -1 && !grammarFileNames_.EndsWith("\x01")) {
					command.Append('\x01');
				}
			} else {
				command.Append(" \x01");
			}
			if (profileId_ != null) {
				command.Append(" profileId=");
				if (profileId_.IndexOf(' ') != -1) {
					command.Append('"');
					command.Append(profileId_);
					command.Append('"');
				} else {
					command.Append(profileId_);
				}
			}
			if (profileWords_ != null) {
				command.Append(" profileWords=");
				if (profileWords_.IndexOf(' ') != -1) {
					command.Append('"');
					command.Append(profileWords_);
					command.Append('"');
				} else {
					command.Append(profileWords_);
				}
			}
			if (segmenterProperties_ != null) {
				command.Append(" segmenterProperties=");
				if (segmenterProperties_.IndexOf(' ') != -1) {
					command.Append('"');
					command.Append(segmenterProperties_);
					command.Append('"');
				} else {
					command.Append(segmenterProperties_);
				}
			}
			if (keepFillerToken_ != null) {
				command.Append(" keepFillerToken=");
				if (keepFillerToken_.IndexOf(' ') != -1) {
					command.Append('"');
					command.Append(keepFillerToken_);
					command.Append('"');
				} else {
					command.Append(keepFillerToken_);
				}
			}
			if (resultUpdatedInterval_ != null) {
				command.Append(" resultUpdatedInterval=");
				if (resultUpdatedInterval_.IndexOf(' ') != -1) {
					command.Append('"');
					command.Append(resultUpdatedInterval_);
					command.Append('"');
				} else {
					command.Append(resultUpdatedInterval_);
				}
			}
			if (extension_ != null) {
				command.Append(" extension=");
				if (extension_.IndexOf(' ') != -1) {
					command.Append('"');
					command.Append(extension_.Replace("\"", "\"\""));
					command.Append('"');
				} else {
					command.Append(extension_);
				}
			}
			if (authorization_ != null) {
				command.Append(" authorization=");
				if (authorization_.IndexOf(' ') != -1) {
					command.Append('"');
					command.Append(authorization_);
					command.Append('"');
				} else {
					command.Append(authorization_);
				}
			}
			if (resultType_ != null) {
				command.Append(" resultType=");
				if (resultType_.IndexOf(' ') != -1) {
					command.Append('"');
					command.Append(resultType_);
					command.Append('"');
				} else {
					command.Append(resultType_);
				}
			}
			try {
				state_ = 1;
				byte[] data = Encoding.UTF8.GetBytes(command.ToString());
				sendRequest_(data, 0, data.Length, (byte)0);
				while (state_ == 1) {
					Monitor.Wait(this);
				}
				if (state_ == 5) {
					return false;
				}
			} catch (ThreadInterruptedException) {
				lastMessage_ = "WARNING: interrupted to start feeding data to WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			} catch (IOException e) {
				lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" + e.Message + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public bool feedData(byte[] data, int dataOffset, int dataBytes) {
		lock (this) {
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
				lastMessage_ = "ERROR: can't feed data to WebSocket server (" + e.Message + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public bool feedDataPause() {
		lock (this) {
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
				byte[] data = Encoding.ASCII.GetBytes("e");
				sendRequest_(data, 0, data.Length, (byte)0);
				while (state_ == 4) {
					Monitor.Wait(this);
				}
				if (state_ == 5) {
					return false;
				}
			} catch (ThreadInterruptedException) {
				lastMessage_ = "WARNING: interrupted to stop feeding data to WebSocket server";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			} catch (IOException e) {
				lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" + e.Message + ")";
				if (listener_ != null) {
					listener_.TRACE(lastMessage_);
				}
				return false;
			}
			return true;
		}
	}

	public string getLastMessage() {
		return lastMessage_;
	}

	public int getWaitingResults() {
		return waitingResults_;
	}

	public void sleep(int timeout) {
		try {
			Thread.Sleep(timeout);
		} catch (ThreadInterruptedException) {}
	}

	protected abstract void sendRequest_(byte[] data, int dataOffset, int dataBytes, byte command);

	protected void onOpen_() {
	}

	protected void onClose_() {
	}

	protected void onError_(Exception cause) {
		lock (this) {
			if (state_ == 0 || state_ == 5) {
				return;
			}
			lastMessage_ = "ERROR: caught exception (" + cause.Message + ")";
			if (listener_ != null) {
				listener_.TRACE(lastMessage_);
			}
			state_ = 5;
			Monitor.PulseAll(this);
		}
	}

	protected void onMessage_(string message) {
		if (message.Length == 0) {
			return;
		}
		char command = message[0];
		if (command == 's') {
			if (message.Length > 1) {
				lock (this) {
					lastMessage_ = "ERROR: can't start feeding data to WebSocket server (" + message.Substring(2) + ")";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 5;
					Monitor.PulseAll(this);
				}
			} else {
				lock (this) {
					lastMessage_ = "INFO: started feeding data to WebSocket server";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 2;
					Monitor.PulseAll(this);
				}
			}
		} else
		if (command == 'p') {
			if (message.Length > 1) {
				lock (this) {
					lastMessage_ = "ERROR: can't feed data to WebSocket server (" + message.Substring(2) + ")";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 5;
					Monitor.PulseAll(this);
				}
			} else {
				// pass
			}
		} else
		if (command == 'e') {
			if (message.Length > 1) {
				lock (this) {
					lastMessage_ = "ERROR: can't stop feeding data to WebSocket server (" + message.Substring(2) + ")";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 5;
					Monitor.PulseAll(this);
				}
			} else {
				lock (this) {
					lastMessage_ = "INFO: stopped feeding data to WebSocket server";
					if (listener_ != null) {
						listener_.TRACE(lastMessage_);
					}
					state_ = 0;
					Monitor.PulseAll(this);
				}
			}
		} else
		if (command == 'S') {
			waitingResults_++;
			if (listener_ != null) {
				listener_.utteranceStarted(int.Parse(message.Substring(2)));
			}
		} else
		if (command == 'E') {
			if (listener_ != null) {
				listener_.utteranceEnded(int.Parse(message.Substring(2)));
			}
		} else
		if (command == 'C') {
			if (listener_ != null) {
				listener_.resultCreated();
			}
		} else
		if (command == 'U') {
			if (listener_ != null) {
				listener_.resultUpdated(message.Substring(2));
			}
		} else
		if (command == 'A') {
			if (listener_ != null) {
				listener_.resultFinalized(message.Substring(2));
			}
			waitingResults_--;
		} else
		if (command == 'R') {
			if (listener_ != null) {
				listener_.resultFinalized("\x01\x01\x01\x01\x01" + message.Substring(2));
			}
			waitingResults_--;
		} else
		if (command == 'Q') {
			if (listener_ != null) {
				listener_.eventNotified((int)command, message.Substring(2));
			}
		} else
		if (command == 'G') {
			if (listener_ != null) {
				listener_.eventNotified((int)command, message.Substring(2));
			}
		}
	}
}

} // namespace
