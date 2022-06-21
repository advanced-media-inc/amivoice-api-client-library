using System;
using System.ComponentModel;
using System.IO;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading;
using System.Web;

namespace com.amivoice.hrp {

[Browsable(false)]
[EditorBrowsable(EditorBrowsableState.Never)]
class Hrp_ : Hrp {
	private static bool _serverCertificateVerification = true;

	private static bool _certificateValidationCallback(Object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors) {
		return true;
	}

	private static string _basic(string userName, string password) {
		return Convert.ToBase64String(Encoding.ASCII.GetBytes(userName + ":" + password));
	}

	private string host_;
	private string resource_;
	private string proxyUserName_;
	private string proxyPassword_;
	private TcpClient tcpClient_;
	private BinaryWriter out_;
	private BinaryReader in_;
	private byte[] requestBodyChunked_;
	private byte[] requestBodyParted_;
	private int requestBodyPartedOffset_;
	private byte[] inData_;
	private int inDataBytes_;
	private int inDataBytes__;
	private string inDataEncoding__;
	private StringBuilder result__;
	private Thread thread_;

	public Hrp_() {
		host_ = null;
		resource_ = null;
		proxyUserName_ = null;
		proxyPassword_ = null;
		tcpClient_ = null;
		out_ = null;
		in_ = null;
		requestBodyChunked_ = null;
		requestBodyParted_ = null;
		requestBodyPartedOffset_ = 0;
		inData_ = new byte[4096];
		inDataBytes_ = 0;
		inDataBytes__ = 0;
		inDataEncoding__ = null;
		result__ = null;
		thread_ = null;
	}

	protected override void connect_(string serverURL, string proxyServerName, int connectTimeout, int receiveTimeout) {
		int colonDoubleSlashIndex = serverURL.IndexOf("://");
		if (colonDoubleSlashIndex == -1) {
			throw new UriFormatException("Invalid server URL");
		}
		string scheme = serverURL.Substring(0, colonDoubleSlashIndex);
		if (!scheme.Equals("http") && !scheme.Equals("https")) {
			throw new UriFormatException("Invalid scheme: " + scheme);
		}
		string hostName;
		int slashIndex = serverURL.IndexOf('/', colonDoubleSlashIndex + 3);
		if (slashIndex != -1) {
			hostName = serverURL.Substring(colonDoubleSlashIndex + 3, slashIndex - (colonDoubleSlashIndex + 3));
			resource_ = serverURL.Substring(slashIndex);
		} else {
			hostName = serverURL.Substring(colonDoubleSlashIndex + 3);
			resource_ = "/";
		}
		int port;
		int colonIndex = hostName.IndexOf(':', hostName.IndexOf(']') + 1);
		if (colonIndex != -1) {
			try {
				port = int.Parse(hostName.Substring(colonIndex + 1));
			} catch (FormatException) {
				throw new UriFormatException("Invalid port: " + hostName.Substring(colonIndex + 1));
			}
			hostName = hostName.Substring(0, colonIndex);
		} else {
			if (scheme.Equals("https")) {
				port = 443;
			} else {
				port = 80;
			}
		}
		if (port == 80) {
			host_ = hostName;
		} else {
			host_ = hostName + ":" + port;
		}
		string proxyHostName = null;
		int proxyPort = 0;
		proxyUserName_ = null;
		proxyPassword_ = null;
		if (proxyServerName != null) {
			proxyHostName = proxyServerName;
			int atIndex = proxyHostName.LastIndexOf('@');
			if (atIndex != -1) {
				/* int */ colonIndex = proxyHostName.IndexOf(':');
				if (colonIndex != -1 && colonIndex < atIndex) {
					proxyUserName_ = proxyHostName.Substring(0, colonIndex);
					proxyPassword_ = proxyHostName.Substring(colonIndex + 1, atIndex - (colonIndex + 1));
					proxyHostName = proxyHostName.Substring(atIndex + 1);
				}
			}
			/* int */ colonIndex = proxyHostName.IndexOf(':', proxyHostName.IndexOf(']') + 1);
			if (colonIndex != -1) {
				try {
					proxyPort = int.Parse(proxyHostName.Substring(colonIndex + 1));
				} catch (FormatException) {
					throw new UriFormatException("Invalid port: " + proxyHostName.Substring(colonIndex + 1));
				}
				proxyHostName = proxyHostName.Substring(0, colonIndex);
			} else {
				proxyPort = 80;
			}
		}
		tcpClient_ = new TcpClient();
		tcpClient_.ReceiveTimeout = receiveTimeout;
		IAsyncResult result;
		if (proxyServerName != null) {
			result = tcpClient_.BeginConnect(proxyHostName, proxyPort, null, null);
		} else {
			result = tcpClient_.BeginConnect(hostName, port, null, null);
		}
		if (!result.AsyncWaitHandle.WaitOne((connectTimeout == 0) ? -1 : connectTimeout)) {
			throw new IOException("Failed to connect");
		}
		tcpClient_.EndConnect(result);
		Stream stream = tcpClient_.GetStream();
		if (proxyServerName != null) {
			if (scheme.Equals("https")) {
				BinaryWriter streamOut = new BinaryWriter(stream);
				BinaryReader streamIn = new BinaryReader(stream);
				StringBuilder proxyRequestHeader = new StringBuilder();
				proxyRequestHeader.Append("CONNECT ");
				proxyRequestHeader.Append(hostName);
				proxyRequestHeader.Append(":");
				proxyRequestHeader.Append(port);
				proxyRequestHeader.Append(" HTTP/1.1\r\n");
				proxyRequestHeader.Append("Host: ");
				proxyRequestHeader.Append(host_);
				proxyRequestHeader.Append("\r\n");
				proxyRequestHeader.Append("User-Agent: ");
				proxyRequestHeader.Append(Hrp.getVersion());
				proxyRequestHeader.Append("\r\n");
				if (proxyUserName_ != null && proxyPassword_ != null) {
					proxyRequestHeader.Append("Proxy-Authorization: Basic ");
					proxyRequestHeader.Append(_basic(proxyUserName_, proxyPassword_));
					proxyRequestHeader.Append("\r\n");
					proxyUserName_ = null;
					proxyPassword_ = null;
				}
				proxyRequestHeader.Append("\r\n");
				streamOut.Write(Encoding.ASCII.GetBytes(proxyRequestHeader.ToString()));
				streamOut.Flush();
				StringBuilder proxyResponseHeader = new StringBuilder();
				int CRLFCRLF = 0;
				int c = streamIn.ReadByte();
				while (c != -1) {
					proxyResponseHeader.Append((char)c);
					CRLFCRLF <<= 8;
					CRLFCRLF |= c;
					if (CRLFCRLF == 0x0D0A0D0A) {
						break;
					}
					c = streamIn.ReadByte();
				}
				if (proxyResponseHeader[ 8] != ' '
				 || proxyResponseHeader[ 9] != '2'
				 || proxyResponseHeader[10] != '0'
				 || proxyResponseHeader[11] != '0'
				 || proxyResponseHeader[12] != ' ') {
					throw new IOException("Invalid response code: " + proxyResponseHeader.ToString(9, 12 - 9));
				}
			} else {
				resource_ = serverURL;
			}
		}
		if (scheme.Equals("https")) {
			SslStream sslStream = new SslStream(stream, false, (!_serverCertificateVerification) ? _certificateValidationCallback : (RemoteCertificateValidationCallback)null);
			sslStream.AuthenticateAsClient(hostName, null, SslProtocols.Tls12, true);
			stream = sslStream;
		}
		out_ = new BinaryWriter(stream);
		in_ = new BinaryReader(stream);
		thread_ = new Thread(new ThreadStart(this.run));
		thread_.Start();
	}

	protected override void disconnect_() {
		if (tcpClient_ != null) {
			try {
				tcpClient_.Close();
			} catch (IOException) {}
			tcpClient_ = null;
		}
		if (thread_ != null) {
			try {
				thread_.Join();
			} catch (ThreadInterruptedException) {}
			thread_ = null;
		}
	}

	protected override bool isConnected_() {
		return (tcpClient_ != null);
	}

	protected override void sendRequestHeader_(string domainId,
											   string codec,
											   string resultType,
											   string resultEncoding,
											   string serviceAuthorization,
											   string voiceDetection,
											   string acceptTopic,
											   string contentType,
											   string accept, string type, long dataBytes) {
		requestBodyChunked_ = null;
		requestBodyParted_ = null;
		requestBodyPartedOffset_ = 0;
		inDataBytes_ = 0;
		inDataBytes__ = 0;
		inDataEncoding__ = null;
		result__ = new StringBuilder();
		StringBuilder requestHeader = new StringBuilder();
		requestHeader.Append("POST ");
		requestHeader.Append(resource_);
		if (type[0] != 'm') {
			char delimiter = (resource_.IndexOf('?') == -1) ? '?' : '&';
			if (domainId != null && domainId.Length > 0) {
				requestHeader.Append(delimiter);
				delimiter = '&';
				requestHeader.Append("d=");
				requestHeader.Append(HttpUtility.UrlEncode(domainId, Encoding.UTF8));
			}
			if (codec != null && codec.Length > 0) {
				requestHeader.Append(delimiter);
				delimiter = '&';
				requestHeader.Append("c=");
				requestHeader.Append(HttpUtility.UrlEncode(codec, Encoding.UTF8));
			}
			if (resultType != null && resultType.Length > 0) {
				requestHeader.Append(delimiter);
				delimiter = '&';
				requestHeader.Append("r=");
				requestHeader.Append(HttpUtility.UrlEncode(resultType, Encoding.UTF8));
			}
			if (resultEncoding != null && resultEncoding.Length > 0) {
				requestHeader.Append(delimiter);
				delimiter = '&';
				requestHeader.Append("e=");
				requestHeader.Append(HttpUtility.UrlEncode(resultEncoding, Encoding.UTF8));
			}
			if (serviceAuthorization != null && serviceAuthorization.Length > 0) {
				requestHeader.Append(delimiter);
				delimiter = '&';
				requestHeader.Append("u=");
				requestHeader.Append(HttpUtility.UrlEncode(serviceAuthorization, Encoding.UTF8));
			}
			if (voiceDetection != null) {
				requestHeader.Append(delimiter);
				delimiter = '&';
				requestHeader.Append("v=");
				requestHeader.Append(HttpUtility.UrlEncode(voiceDetection, Encoding.UTF8));
			}
		} else {
			StringBuilder requestBody = new StringBuilder();
			if (domainId != null && domainId.Length > 0) {
				requestBody.Append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"d\"\r\n\r\n");
				requestBody.Append(domainId);
			}
			if (codec != null && codec.Length > 0) {
				requestBody.Append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"c\"\r\n\r\n");
				requestBody.Append(codec);
			}
			if (resultType != null && resultType.Length > 0) {
				requestBody.Append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"r\"\r\n\r\n");
				requestBody.Append(resultType);
			}
			if (resultEncoding != null && resultEncoding.Length > 0) {
				requestBody.Append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"e\"\r\n\r\n");
				requestBody.Append(resultEncoding);
			}
			if (serviceAuthorization != null && serviceAuthorization.Length > 0) {
				requestBody.Append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"u\"\r\n\r\n");
				requestBody.Append(serviceAuthorization);
			}
			if (voiceDetection != null) {
				requestBody.Append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"v\"\r\n\r\n");
				requestBody.Append(voiceDetection);
			}
			requestBody.Append("\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"a\"\r\n\r\n");
			requestBody.Append("\r\n--1234567890ABCDEFGHIJ--\r\n");
			requestBodyParted_ = Encoding.UTF8.GetBytes(requestBody.ToString()); // [70+n+][70+n+][70+n+][70+n+][70+n+][70+n+]70+28 bytes
		}
		requestHeader.Append(" HTTP/1.1\r\n");
		requestHeader.Append("Host: ");
		requestHeader.Append(host_);
		requestHeader.Append("\r\n");
		requestHeader.Append("User-Agent: ");
		requestHeader.Append(Hrp.getVersion());
		requestHeader.Append("\r\n");
		if (proxyUserName_ != null && proxyPassword_ != null) {
			requestHeader.Append("Proxy-Authorization: Basic ");
			requestHeader.Append(_basic(proxyUserName_, proxyPassword_));
			requestHeader.Append("\r\n");
		}
		if (type[1] != 'c') {
			requestHeader.Append("Content-Length: ");
			if (requestBodyParted_ == null) {
				requestHeader.Append(dataBytes);
			} else {
				requestHeader.Append(dataBytes + requestBodyParted_.Length);
			}
			requestHeader.Append("\r\n");
		} else {
			requestBodyChunked_ = Encoding.ASCII.GetBytes("0\r\n\r\n");
			requestHeader.Append("Transfer-Encoding: chunked\r\n");
		}
		if (requestBodyParted_ == null) {
			if (acceptTopic != null && acceptTopic.Length > 0) {
				requestHeader.Append("Accept-Topic: ");
				requestHeader.Append(acceptTopic);
				requestHeader.Append("\r\n");
			}
			if (contentType != null && contentType.Length > 0) {
				requestHeader.Append("Content-Type: ");
				requestHeader.Append(contentType);
				requestHeader.Append("\r\n");
			}
			if (accept != null && accept.Length > 0) {
				requestHeader.Append("Accept: ");
				requestHeader.Append(accept);
				requestHeader.Append("\r\n");
			}
		} else {
			requestHeader.Append("Content-Type: multipart/form-data; boundary=1234567890ABCDEFGHIJ\r\n");
		}
		requestHeader.Append("\r\n");
		out_.Write(Encoding.ASCII.GetBytes(requestHeader.ToString()));
		out_.Flush();
	}

	protected override void sendRequestBody_(byte[] data, int dataOffset, int dataBytes) {
		if (data != null && dataBytes > 0) {
			if (requestBodyParted_ != null) {
				if (requestBodyPartedOffset_ == 0) {
					requestBodyPartedOffset_ = requestBodyParted_.Length - 28;
					if (requestBodyChunked_ != null) {
						out_.Write(Encoding.ASCII.GetBytes(Convert.ToString(requestBodyPartedOffset_, 16)));
						out_.Write(requestBodyChunked_, 1, 2);
					}
					out_.Write(requestBodyParted_, 0, requestBodyPartedOffset_);
					if (requestBodyChunked_ != null) {
						out_.Write(requestBodyChunked_, 3, 2);
					}
				}
			}
			if (requestBodyChunked_ != null) {
				out_.Write(Encoding.ASCII.GetBytes(Convert.ToString(dataBytes, 16)));
				out_.Write(requestBodyChunked_, 1, 2);
			}
			out_.Write(data, dataOffset, dataBytes);
			if (requestBodyChunked_ != null) {
				out_.Write(requestBodyChunked_, 3, 2);
			}
			out_.Flush();
		} else {
			if (requestBodyParted_ != null) {
				if (requestBodyChunked_ != null) {
					out_.Write(Encoding.ASCII.GetBytes(Convert.ToString(requestBodyParted_.Length - requestBodyPartedOffset_, 16)));
					out_.Write(requestBodyChunked_, 1, 2);
				}
				out_.Write(requestBodyParted_, requestBodyPartedOffset_, requestBodyParted_.Length - requestBodyPartedOffset_);
				if (requestBodyChunked_ != null) {
					out_.Write(requestBodyChunked_, 3, 2);
					out_.Write(requestBodyChunked_, 0, 5);
				}
			} else {
				if (requestBodyChunked_ != null) {
					out_.Write(requestBodyChunked_, 0, 5);
				}
			}
			out_.Flush();
		}
	}

	private void run() {
		try {
			while (true) {
				string sessionId = receiveResponseHeader_();
				onOpen_(sessionId);
				if (inDataBytes__ > 0) {
					receiveResponseBody_();
					int crlfIndex = result_CRLF_();
					while (crlfIndex != -1) {
						string result = result__.ToString(0, crlfIndex);
						onMessage_(result);
						result__.Remove(0, crlfIndex + 2);
						crlfIndex = result_CRLF_();
					}
				} else {
					while (receiveChunkedResponseBody_()) {
						int crlfIndex = result_CRLF_();
						while (crlfIndex != -1) {
							string result = result__.ToString(0, crlfIndex);
							onMessage_(result);
							result__.Remove(0, crlfIndex + 2);
							crlfIndex = result_CRLF_();
						}
					}
				}
				onClose_();
			}
		} catch (IOException e) {
			onError_(e);
		} catch (ObjectDisposedException e) {
			onError_(e);
		}
	}

	private string receiveResponseHeader_() {
		int responseHeaderBytes = read_CRLFCRLF_();
		if (responseHeaderBytes == -1) {
			throw new IOException("Unexpected end of stream");
		}
		string responseHeader = Encoding.ASCII.GetString(inData_, 0, responseHeaderBytes);
		if (responseHeader[ 8] != ' '
		 || responseHeader[ 9] != '2'
		 || responseHeader[10] != '0'
		 || responseHeader[11] != '0'
		 || responseHeader[12] != ' ') {
			throw new IOException("Invalid response code: " + responseHeader.Substring(9, 12 - 9));
		}
		int index1 = responseHeader.IndexOf("\r\nX-Session-ID: ");
		if (index1 == -1) {
			throw new IOException("Missing session id");
		}
		index1 += 16;
		int index2 = responseHeader.IndexOf("\r\n", index1);
		if (index2 == -1) {
			throw new IOException("Missing session id");
		}
		string sessionId = responseHeader.Substring(index1, index2 - index1);
		inDataEncoding__ = "UTF-8";
		index1 = responseHeader.IndexOf("\r\nContent-Type: ");
		if (index1 != -1) {
			index1 += 16;
			index2 = responseHeader.IndexOf("\r\n", index1);
			if (index2 != -1) {
				int index3 = responseHeader.IndexOf("; charset=", index1);
				if (index3 != -1 && index3 < index2) {
					index3 += 10;
					inDataEncoding__ = responseHeader.Substring(index3, index2 - index3).ToUpper();
				}
			}
		}
		index1 = responseHeader.IndexOf("\r\nContent-Length: ");
		if (index1 != -1) {
			index1 += 18;
			index2 = responseHeader.IndexOf("\r\n", index1);
			if (index2 != -1) {
				try {
					inDataBytes__ = int.Parse(responseHeader.Substring(index1, index2 - index1));
				} catch (FormatException) {
					throw new IOException("Invalid content length: " + responseHeader.Substring(index1, index2 - index1));
				}
			}
		}
		shift_(responseHeaderBytes);
		return sessionId;
	}

	private void receiveResponseBody_() {
		if (!read_(inDataBytes__)) {
			throw new IOException("Unexpected end of stream");
		}
		result_(inDataBytes__);
		shift_(inDataBytes__);
	}

	private bool receiveChunkedResponseBody_() {
		int chunkedResponseHeaderBytes = read_CRLF_();
		if (chunkedResponseHeaderBytes == -1) {
			throw new IOException("Unexpected end of stream");
		}
		inDataBytes__ = 0;
		for (int i = 0; i < chunkedResponseHeaderBytes; i++) {
			int c = inData_[i];
			if (c >= '0' && c <= '9') {
				inDataBytes__ <<= 4;
				inDataBytes__ += c - '0';
			} else
			if (c >= 'A' && c <= 'F') {
				inDataBytes__ <<= 4;
				inDataBytes__ += c - 'A' + 10;
			} else
			if (c >= 'a' && c <= 'f') {
				inDataBytes__ <<= 4;
				inDataBytes__ += c - 'a' + 10;
			} else {
				break;
			}
		}
		shift_(chunkedResponseHeaderBytes);
		if (inDataBytes__ == 0) {
			if (!read_(2)) {
				throw new IOException("Unexpected end of stream");
			}
			shift_(2);
			return false;
		}
		if (!read_(inDataBytes__ + 2)) {
			throw new IOException("Unexpected end of stream");
		}
		result_(inDataBytes__);
		shift_(inDataBytes__ + 2);
		return true;
	}

	private int read_CRLFCRLF_() {
		int inDataBytes = find_CRLFCRLF_();
		while (inDataBytes == -1) {
			ensure_(inDataBytes_ + 4096);
			int inDataReadBytes = in_.Read(inData_, inDataBytes_, 4096);
			if (inDataReadBytes == -1) {
				return -1;
			}
			inDataBytes_ += inDataReadBytes;
			inDataBytes = find_CRLFCRLF_();
		}
		return inDataBytes + 4;
	}

	private int find_CRLFCRLF_() {
		for (int i = 0; i < inDataBytes_ - 3; i++) {
			if (inData_[i    ] == '\r'
			 && inData_[i + 1] == '\n'
			 && inData_[i + 2] == '\r'
			 && inData_[i + 3] == '\n') {
				return i;
			}
		}
		return -1;
	}

	private int read_CRLF_() {
		int inDataBytes = find_CRLF_();
		while (inDataBytes == -1) {
			ensure_(inDataBytes_ + 4096);
			int inDataReadBytes = in_.Read(inData_, inDataBytes_, 4096);
			if (inDataReadBytes == -1) {
				return -1;
			}
			inDataBytes_ += inDataReadBytes;
			inDataBytes = find_CRLF_();
		}
		return inDataBytes + 2;
	}

	private int find_CRLF_() {
		for (int i = 0; i < inDataBytes_ - 1; i++) {
			if (inData_[i    ] == '\r'
			 && inData_[i + 1] == '\n') {
				return i;
			}
		}
		return -1;
	}

	private bool read_(int inDataBytes) {
		ensure_(inDataBytes);
		while (inDataBytes_ < inDataBytes) {
			int inDataReadBytes = in_.Read(inData_, inDataBytes_, inDataBytes - inDataBytes_);
			if (inDataReadBytes == -1) {
				return false;
			}
			inDataBytes_ += inDataReadBytes;
		}
		return true;
	}

	private void ensure_(int inDataBytes) {
		if (inData_.Length < inDataBytes) {
			int inDataNewBytes = inData_.Length;
			while (inDataNewBytes < inDataBytes) {
				inDataNewBytes *= 2;
			}
			byte[] inData = new byte[inDataNewBytes];
			Array.Copy(inData_, 0, inData, 0, inDataBytes_);
			inData_ = inData;
		}
	}

	private void result_(int inDataBytes) {
		string result;
		try {
			result = Encoding.GetEncoding(inDataEncoding__).GetString(inData_, 0, inDataBytes);
		} catch (ArgumentException) {
			result = Encoding.ASCII.GetString(inData_, 0, inDataBytes);
		}
		result__.Append(result);
	}

	private int result_CRLF_() {
		for (int i = 0; i < result__.Length - 1; i++) {
			if (result__[i    ] == '\r'
			 && result__[i + 1] == '\n') {
				return i;
			}
		}
		return -1;
	}

	private void shift_(int inDataBytes) {
		inDataBytes_ -= inDataBytes;
		Array.Copy(inData_, inDataBytes, inData_, 0, inDataBytes_);
	}
}

} // namespace
