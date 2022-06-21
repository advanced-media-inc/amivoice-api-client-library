using System;
using System.ComponentModel;
using System.IO;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading;

namespace com.amivoice.wrp {

[Browsable(false)]
[EditorBrowsable(EditorBrowsableState.Never)]
class Wrp_ : Wrp {
	private static bool _serverCertificateVerification = true;

	private static bool _certificateValidationCallback(Object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors) {
		return true;
	}

	private static char[] _base64Table = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};

	private static string _basic(string userName, string password) {
		return Convert.ToBase64String(Encoding.ASCII.GetBytes(userName + ":" + password));
	}

	private TcpClient tcpClient_;
	private BinaryWriter out_;
	private BinaryReader in_;
	private byte[] outData_;
	private byte[] inData_;
	private int inDataBytes_;
	private string result__;
	private Thread thread_;

	public Wrp_() {
		tcpClient_ = null;
		out_ = null;
		in_ = null;
		outData_ = new byte[10 + 1 + 4096];
		inData_ = new byte[4096];
		inDataBytes_ = 0;
		result__ = null;
		thread_ = null;
	}

	protected override void connect_(string serverURL, string proxyServerName, int connectTimeout, int receiveTimeout) {
		int colonDoubleSlashIndex = serverURL.IndexOf("://");
		if (colonDoubleSlashIndex == -1) {
			throw new UriFormatException("Invalid server URL");
		}
		string scheme = serverURL.Substring(0, colonDoubleSlashIndex);
		if (!scheme.Equals("ws") && !scheme.Equals("wss")) {
			throw new UriFormatException("Invalid scheme: " + scheme);
		}
		string hostName;
		string resource;
		int slashIndex = serverURL.IndexOf('/', colonDoubleSlashIndex + 3);
		if (slashIndex != -1) {
			hostName = serverURL.Substring(colonDoubleSlashIndex + 3, slashIndex - (colonDoubleSlashIndex + 3));
			resource = serverURL.Substring(slashIndex);
		} else {
			hostName = serverURL.Substring(colonDoubleSlashIndex + 3);
			resource = "/";
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
			if (scheme.Equals("wss")) {
				port = 443;
			} else {
				port = 80;
			}
		}
		string host;
		if (port == 80) {
			host = hostName;
		} else {
			host = hostName + ":" + port;
		}
		string proxyHostName = null;
		int proxyPort = 0;
		string proxyUserName = null;
		string proxyPassword = null;
		if (proxyServerName != null) {
			proxyHostName = proxyServerName;
			int atIndex = proxyHostName.LastIndexOf('@');
			if (atIndex != -1) {
				/* int */ colonIndex = proxyHostName.IndexOf(':');
				if (colonIndex != -1 && colonIndex < atIndex) {
					proxyUserName = proxyHostName.Substring(0, colonIndex);
					proxyPassword = proxyHostName.Substring(colonIndex + 1, atIndex - (colonIndex + 1));
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
			if (scheme.Equals("ws") || scheme.Equals("wss")) {
				BinaryWriter streamOut = new BinaryWriter(stream);
				BinaryReader streamIn = new BinaryReader(stream);
				StringBuilder proxyRequestHeader = new StringBuilder();
				proxyRequestHeader.Append("CONNECT ");
				proxyRequestHeader.Append(hostName);
				proxyRequestHeader.Append(":");
				proxyRequestHeader.Append(port);
				proxyRequestHeader.Append(" HTTP/1.1\r\n");
				proxyRequestHeader.Append("Host: ");
				proxyRequestHeader.Append(host);
				proxyRequestHeader.Append("\r\n");
				proxyRequestHeader.Append("User-Agent: ");
				proxyRequestHeader.Append(Wrp.getVersion());
				proxyRequestHeader.Append("\r\n");
				if (proxyUserName != null && proxyPassword != null) {
					proxyRequestHeader.Append("Proxy-Authorization: Basic ");
					proxyRequestHeader.Append(_basic(proxyUserName, proxyPassword));
					proxyRequestHeader.Append("\r\n");
					proxyUserName = null;
					proxyPassword = null;
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
				resource = serverURL;
			}
		}
		if (scheme.Equals("wss")) {
			SslStream sslStream = new SslStream(stream, false, (!_serverCertificateVerification) ? _certificateValidationCallback : (RemoteCertificateValidationCallback)null);
			sslStream.AuthenticateAsClient(hostName, null, SslProtocols.Tls12, true);
			stream = sslStream;
		}
		out_ = new BinaryWriter(stream);
		in_ = new BinaryReader(stream);
		inDataBytes_ = 0;
		result__ = null;
		// -----------------------------------------------------------
		// GET <resource> HTTP/1.1
		// Host: <hostName>:<port>
		// Upgrade: websocket
		// Connection: upgrade
		// Sec-WebSocket-Key: ...
		// 
		// -----------------------------------------------------------
		StringBuilder requestHeader = new StringBuilder();
		requestHeader.Append("GET ");
		requestHeader.Append(resource);
		requestHeader.Append(" HTTP/1.1\r\n");
		requestHeader.Append("User-Agent: ");
		requestHeader.Append(Wrp.getVersion());
		requestHeader.Append("\r\n");
		requestHeader.Append("Host: ");
		requestHeader.Append(host);
		requestHeader.Append("\r\n");
		requestHeader.Append("Upgrade: websocket\r\n");
		requestHeader.Append("Connection: upgrade\r\n");
		requestHeader.Append("Sec-WebSocket-Key: ");
		Random random = new Random();
		for (int i = 0; i < 20; i += 4) {
			int twentyFourBits = random.Next() & 0x3F3F3F3F;
			requestHeader.Append(_base64Table[(twentyFourBits >> 24) & 0x3F]);
			requestHeader.Append(_base64Table[(twentyFourBits >> 16) & 0x3F]);
			requestHeader.Append(_base64Table[(twentyFourBits >>  8) & 0x3F]);
			requestHeader.Append(_base64Table[(twentyFourBits      ) & 0x3F]);
		}
		int eightBits = random.Next() & 0x3F300000;
		requestHeader.Append(_base64Table[(eightBits >> 24) & 0x3F]);
		requestHeader.Append(_base64Table[(eightBits >> 16) & 0x3F]);
		requestHeader.Append("==\r\n");
		requestHeader.Append("\r\n");
		out_.Write(Encoding.ASCII.GetBytes(requestHeader.ToString()));
		out_.Flush();
		// -----------------------------------------------------------
		// HTTP/1.1 101 Switching Protocols
		// Upgrade: websocket
		// Connection: upgrade
		// Sec-WebSocket-Accept: ...
		// 
		// -----------------------------------------------------------
		int responseHeaderBytes = read_CRLFCRLF_();
		if (responseHeaderBytes == -1) {
			throw new IOException("Unexpected end of stream");
		}
		string responseHeader = Encoding.ASCII.GetString(inData_, 0, responseHeaderBytes);
		if (responseHeader[ 8] != ' '
		 || responseHeader[ 9] != '1'
		 || responseHeader[10] != '0'
		 || responseHeader[11] != '1'
		 || responseHeader[12] != ' ') {
			throw new IOException("Handshake error (invalld response code: " + responseHeader.Substring(9, 12 - 9) + ")");
		}
		shift_(responseHeaderBytes);
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

	protected override void sendRequest_(byte[] data, int dataOffset, int dataBytes, byte command) {
		// 0                   1                   2                   3
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		// +-+-+-+-+-------+-+-------------+-------------------------------+
		// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
		// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
		// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
		// | |1|2|3|       |K|             |                               |
		// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
		// |     Extended payload length continued, if payload len == 127  |
		// + - - - - - - - - - - - - - - - +-------------------------------+
		// |                               |Masking-key, if MASK set to 1  |
		// +-------------------------------+-------------------------------+
		// | Masking-key (continued)       |          Payload Data         |
		// +-------------------------------- - - - - - - - - - - - - - - - +
		// :                     Payload Data continued ...                :
		// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
		// |                     Payload Data continued ...                |
		// +---------------------------------------------------------------+
		int realDataBytes = ((command == 0) ? 0 : 1) + dataBytes;
		if (outData_.Length < 10 + realDataBytes) {
			outData_ = new byte[10 + realDataBytes];
		}
		int outDataBytes = 0;
		outData_[outDataBytes++] = (byte)(0x80 | ((command == 0) ? /* Text frame */ 0x01 : /* Binary frame */ 0x02));
		if (realDataBytes < 126) {
			outData_[outDataBytes++] = (byte)realDataBytes;
		} else
		if (realDataBytes < 65536) {
			outData_[outDataBytes++] = (byte)126;
			outData_[outDataBytes++] = (byte)((realDataBytes >>  8) & 0xFF);
			outData_[outDataBytes++] = (byte)((realDataBytes      ) & 0xFF);
		} else {
			outData_[outDataBytes++] = (byte)127;
			outData_[outDataBytes++] = 0;
			outData_[outDataBytes++] = 0;
			outData_[outDataBytes++] = 0;
			outData_[outDataBytes++] = 0;
			outData_[outDataBytes++] = (byte)((realDataBytes >> 24) & 0xFF);
			outData_[outDataBytes++] = (byte)((realDataBytes >> 16) & 0xFF);
			outData_[outDataBytes++] = (byte)((realDataBytes >>  8) & 0xFF);
			outData_[outDataBytes++] = (byte)((realDataBytes      ) & 0xFF);
		}
		if (command == 0) {
			// pass
		} else {
			outData_[outDataBytes++] = command;
		}
		Array.Copy(data, dataOffset, outData_, outDataBytes, dataBytes);
		outDataBytes += dataBytes;
		out_.Write(outData_, 0, outDataBytes);
		out_.Flush();
	}

	private void run() {
		try {
			onOpen_();
			while (receiveResponse_()) {
				onMessage_(result__);
			}
			onClose_();
		} catch (IOException e) {
			onError_(e);
		} catch (ObjectDisposedException e) {
			onError_(e);
		}
	}

	private bool receiveResponse_() {
		// 0                   1                   2                   3
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		// +-+-+-+-+-------+-+-------------+-------------------------------+
		// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
		// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
		// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
		// | |1|2|3|       |K|             |                               |
		// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
		// |     Extended payload length continued, if payload len == 127  |
		// + - - - - - - - - - - - - - - - +-------------------------------+
		// |                               |Masking-key, if MASK set to 1  |
		// +-------------------------------+-------------------------------+
		// | Masking-key (continued)       |          Payload Data         |
		// +-------------------------------- - - - - - - - - - - - - - - - +
		// :                     Payload Data continued ...                :
		// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
		// |                     Payload Data continued ...                |
		// +---------------------------------------------------------------+
		if (!read_(2)) {
			if (inDataBytes_ == 0) {
				return false;
			}
			throw new IOException("Unexpected end of stream");
		}
		int inDataBytes = (inData_[1] & 0x7F);
		shift_(2);
		if (inDataBytes == 126) {
			if (!read_(2)) {
				throw new IOException("Unexpected end of stream");
			}
			inDataBytes = ((inData_[0] & 0xFF) <<  8)
						| ((inData_[1] & 0xFF)      );
			shift_(2);
		} else
		if (inDataBytes == 127) {
			if (!read_(8)) {
				throw new IOException("Unexpected end of stream");
			}
			if (inData_[0] != 0
			 || inData_[1] != 0
			 || inData_[2] != 0
			 || inData_[3] != 0 || (inData_[4] & 0x80) != 0) {
				throw new IOException("Invalid payload length: " + inDataBytes);
			}
			inDataBytes = ((inData_[4] & 0xFF) << 24)
						| ((inData_[5] & 0xFF) << 16)
						| ((inData_[6] & 0xFF) <<  8)
						| ((inData_[7] & 0xFF)      );
			shift_(8);
		}
		if (inDataBytes < 0) {
			throw new IOException("Invalid payload length: " + inDataBytes);
		}
		if (!read_(inDataBytes)) {
			throw new IOException("Unexpected end of stream");
		}
		result_(inDataBytes);
		shift_(inDataBytes);
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
		if (inDataBytes > 0 && inData_[inDataBytes - 1] == '\n') {
			inDataBytes--;
			if (inDataBytes > 0 && inData_[inDataBytes - 1] == '\r') {
				inDataBytes--;
			}
		}
		string result;
		try {
			result = Encoding.UTF8.GetString(inData_, 0, inDataBytes);
		} catch (ArgumentException) {
			result = Encoding.ASCII.GetString(inData_, 0, inDataBytes);
		}
		result__ = result;
	}

	private void shift_(int inDataBytes) {
		inDataBytes_ -= inDataBytes;
		Array.Copy(inData_, inDataBytes, inData_, 0, inDataBytes_);
	}
}

} // namespace
