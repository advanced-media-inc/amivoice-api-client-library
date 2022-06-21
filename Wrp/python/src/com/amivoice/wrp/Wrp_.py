import base64
import random
import socket
import ssl
import threading

from .Wrp import Wrp

class Wrp_(Wrp):
	_serverCertificateVerification = True

	_base64Table = [
		0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, # 'A' 'B' 'C' 'D' 'E' 'F' 'G' 'H'
		0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, # 'I' 'J' 'K' 'L' 'M' 'N' 'O' 'P'
		0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, # 'Q' 'R' 'S' 'T' 'U' 'V' 'W' 'X'
		0x59, 0x5A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, # 'Y' 'Z' 'a' 'b' 'c' 'd' 'e' 'f'
		0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, # 'g' 'h' 'i' 'j' 'k' 'l' 'm' 'n'
		0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, # 'o' 'p' 'q' 'r' 's' 't' 'u' 'v'
		0x77, 0x78, 0x79, 0x7A, 0x30, 0x31, 0x32, 0x33, # 'w' 'x' 'y' 'z' '0' '1' '2' '3'
		0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2B, 0x2F  # '4' '5' '6' '7' '8' '9' '+' '/'
	]

	@staticmethod
	def _basic(userName, password):
		return base64.b64encode((userName + ":" + password).encode())

	def __init__(self):
		super(Wrp_, self).__init__()
		self.socket_ = None
		self.outData_ = bytearray(10 + 1 + 4096)
		self.inData_ = bytearray()
		self.inDataBytes_ = 0
		self.result__ = None
		self.thread_ = None

	def connect_(self, serverURL, proxyServerName, connectTimeout, receiveTimeout):
		colonDoubleSlashIndex = serverURL.find("://")
		if colonDoubleSlashIndex == -1:
			raise ValueError("Invalid server URL")
		scheme = serverURL[:colonDoubleSlashIndex]
		if scheme != "ws" and scheme != "wss":
			raise ValueError("Invalid scheme: " + scheme)
		slashIndex = serverURL.find('/', colonDoubleSlashIndex + 3)
		if slashIndex != -1:
			hostName = serverURL[colonDoubleSlashIndex + 3:slashIndex]
			resource = serverURL[slashIndex:]
		else:
			hostName = serverURL[colonDoubleSlashIndex + 3:]
			resource = "/"
		colonIndex = hostName.find(':', hostName.find(']') + 1)
		if colonIndex != -1:
			try:
				port = int(hostName[colonIndex + 1:])
			except:
				raise ValueError("Invalid port: " + hostName[colonIndex + 1:])
			hostName = hostName[:colonIndex]
		else:
			if scheme == "wss":
				port = 443
			else:
				port = 80
		if port == 80:
			host = hostName
		else:
			host = hostName + ":" + str(port)
		proxyHostName = None
		proxyPort = 0
		proxyUserName = None
		proxyPassword = None
		if proxyServerName != None:
			proxyHostName = proxyServerName
			atIndex = proxyHostName.rfind('@')
			if atIndex != -1:
				colonIndex = proxyHostName.find(':')
				if colonIndex != -1 and colonIndex < atIndex:
					proxyUserName = proxyHostName[:colonIndex]
					proxyPassword = proxyHostName[colonIndex + 1:atIndex]
					proxyHostName = proxyHostName[atIndex + 1:]
			colonIndex = proxyHostName.find(':', proxyHostName.find(']') + 1)
			if colonIndex != -1:
				try:
					proxyPort = int(proxyHostName[colonIndex + 1:])
				except:
					raise ValueError("Invalid port: " + proxyHostName[colonIndex + 1:])
				proxyHostName = proxyHostName[:colonIndex]
			else:
				proxyPort = 80
		if proxyServerName != None:
			if connectTimeout > 0:
				self.socket_ = socket.create_connection((proxyHostName, proxyPort), connectTimeout / 1000.0)
			else:
				self.socket_ = socket.create_connection((proxyHostName, proxyPort))
		else:
			if connectTimeout > 0:
				self.socket_ = socket.create_connection((hostName, port), connectTimeout / 1000.0)
			else:
				self.socket_ = socket.create_connection((hostName, port))
		if receiveTimeout > 0:
			self.socket_.settimeout(receiveTimeout / 1000.0)
		else:
			self.socket_.settimeout(None)
		if proxyServerName != None:
			if scheme == "ws" or scheme == "wss":
				proxyRequestHeader = bytearray()
				proxyRequestHeader.extend(b"CONNECT ")
				proxyRequestHeader.extend(hostName.encode())
				proxyRequestHeader.extend(b":")
				proxyRequestHeader.extend(str(port).encode())
				proxyRequestHeader.extend(b" HTTP/1.1\r\n")
				proxyRequestHeader.extend(b"Host: ")
				proxyRequestHeader.extend(host.encode())
				proxyRequestHeader.extend(b"\r\n")
				proxyRequestHeader.extend(b"User-Agent: ")
				proxyRequestHeader.extend(Wrp.getVersion().encode())
				proxyRequestHeader.extend(b"\r\n")
				if proxyUserName != None and proxyPassword != None:
					proxyRequestHeader.extend(b"Proxy-Authorization: Basic ")
					proxyRequestHeader.extend(Wrp_._basic(proxyUserName, proxyPassword))
					proxyRequestHeader.extend(b"\r\n")
					proxyUserName = None
					proxyPassword = None
				proxyRequestHeader.extend(b"\r\n")
				self.socket_.send(proxyRequestHeader)
				proxyResponseHeader = bytearray()
				CRLFCRLF = 0
				c = self.socket_.recv(1)
				while c:
					proxyResponseHeader.extend(c)
					CRLFCRLF &= 0x00FFFFFF
					CRLFCRLF <<= 8
					CRLFCRLF |= ord(c)
					if CRLFCRLF == 0x0D0A0D0A:
						break
					c = self.socket_.recv(1)
				if (proxyResponseHeader[ 8] != 0x20   # ' '
				 or proxyResponseHeader[ 9] != 0x32   # '2'
				 or proxyResponseHeader[10] != 0x30   # '0'
				 or proxyResponseHeader[11] != 0x30   # '0'
				 or proxyResponseHeader[12] != 0x20): # ' '
					raise IOError("Invalid response code: " + proxyResponseHeader[9:12].decode())
			else:
				resource = serverURL
		if scheme == "wss":
			try:
				if not Wrp_._serverCertificateVerification:
					self.socket_ = ssl.wrap_socket(self.socket_)
				else:
					self.socket_ = ssl.wrap_socket(self.socket_, cert_reqs = ssl.CERT_REQUIRED, ca_certs = ssl.get_default_verify_paths().cafile)
					ssl.match_hostname(self.socket_.getpeercert(), hostName)
			except:
				self.socket_.close()
				self.socket_ = None
				raise
		self.inDataBytes_ = 0
		self.result__ = None
		# -----------------------------------------------------------
		# GET <resource> HTTP/1.1
		# Host: <hostName>:<port>
		# Upgrade: websocket
		# Connection: upgrade
		# Sec-WebSocket-Key: ...
		# 
		# -----------------------------------------------------------
		requestHeader = bytearray()
		requestHeader.extend(b"GET ")
		requestHeader.extend(resource.encode())
		requestHeader.extend(b" HTTP/1.1\r\n")
		requestHeader.extend(b"User-Agent: ")
		requestHeader.extend(Wrp.getVersion().encode())
		requestHeader.extend(b"\r\n")
		requestHeader.extend(b"Host: ")
		requestHeader.extend(host.encode())
		requestHeader.extend(b"\r\n")
		requestHeader.extend(b"Upgrade: websocket\r\n")
		requestHeader.extend(b"Connection: upgrade\r\n")
		requestHeader.extend(b"Sec-WebSocket-Key: ")
		for i in range(0, 20, 4):
			twentyFourBits = random.randint(0, 0xFFFFFFFF) & 0x3F3F3F3F
			requestHeader.append(Wrp_._base64Table[(twentyFourBits >> 24) & 0x3F])
			requestHeader.append(Wrp_._base64Table[(twentyFourBits >> 16) & 0x3F])
			requestHeader.append(Wrp_._base64Table[(twentyFourBits >>  8) & 0x3F])
			requestHeader.append(Wrp_._base64Table[(twentyFourBits      ) & 0x3F])
		eightBits = random.randint(0, 0xFFFFFFFF) & 0x3F300000
		requestHeader.append(Wrp_._base64Table[(eightBits >> 24) & 0x3F])
		requestHeader.append(Wrp_._base64Table[(eightBits >> 16) & 0x3F])
		requestHeader.extend(b"==\r\n")
		requestHeader.extend(b"\r\n")
		self.socket_.send(requestHeader)
		# -----------------------------------------------------------
		# HTTP/1.1 101 Switching Protocols
		# Upgrade: websocket
		# Connection: upgrade
		# Sec-WebSocket-Accept: ...
		# 
		# -----------------------------------------------------------
		responseHeaderBytes = self.read_CRLFCRLF_()
		if responseHeaderBytes == -1:
			raise IOError("Unexpected end of stream")
		responseHeader = self.inData_[:responseHeaderBytes]
		if (responseHeader[ 8] != 0x20   # ' '
		 or responseHeader[ 9] != 0x31   # '1'
		 or responseHeader[10] != 0x30   # '0'
		 or responseHeader[11] != 0x31   # '1'
		 or responseHeader[12] != 0x20): # ' '
			raise IOError("Handshake error (invalld response code: " + responseHeader[9:12].decode() + ")")
		self.shift_(responseHeaderBytes)
		self.thread_ = threading.Thread(target = self.run)
		self.thread_.start()

	def disconnect_(self):
		if self.socket_ != None:
			try:
				self.socket_.shutdown(socket.SHUT_RDWR)
				self.socket_.close()
			except:
				pass
			self.socket_ = None
		if self.thread_ != None:
			try:
				self.thread_.join()
			except:
				pass
			self.thread_ = None

	def isConnected_(self):
		return (self.socket_ != None)

	def sendRequest_(self, data, dataOffset, dataBytes, command):
		# 0                   1                   2                   3
		# 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		# +-+-+-+-+-------+-+-------------+-------------------------------+
		# |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
		# |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
		# |N|V|V|V|       |S|             |   (if payload len==126/127)   |
		# | |1|2|3|       |K|             |                               |
		# +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
		# |     Extended payload length continued, if payload len == 127  |
		# + - - - - - - - - - - - - - - - +-------------------------------+
		# |                               |Masking-key, if MASK set to 1  |
		# +-------------------------------+-------------------------------+
		# | Masking-key (continued)       |          Payload Data         |
		# +-------------------------------- - - - - - - - - - - - - - - - +
		# :                     Payload Data continued ...                :
		# + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
		# |                     Payload Data continued ...                |
		# +---------------------------------------------------------------+
		realDataBytes = (0 if command == 0 else 1) + dataBytes
		if len(self.outData_) < 10 + realDataBytes:
			self.outData_ = bytearray(10 + realDataBytes)
		outDataBytes = 0
		self.outData_[outDataBytes] = (0x80 | (0x01 if command == 0 else 0x02))
		outDataBytes += 1
		if realDataBytes < 126:
			self.outData_[outDataBytes] = realDataBytes
			outDataBytes += 1
		elif realDataBytes < 65536:
			self.outData_[outDataBytes] = 126
			outDataBytes += 1
			self.outData_[outDataBytes] = ((realDataBytes >>  8) & 0xFF)
			outDataBytes += 1
			self.outData_[outDataBytes] = ((realDataBytes      ) & 0xFF)
			outDataBytes += 1
		else:
			self.outData_[outDataBytes] = 127
			outDataBytes += 1
			self.outData_[outDataBytes] = 0
			outDataBytes += 1
			self.outData_[outDataBytes] = 0
			outDataBytes += 1
			self.outData_[outDataBytes] = 0
			outDataBytes += 1
			self.outData_[outDataBytes] = 0
			outDataBytes += 1
			self.outData_[outDataBytes] = ((realDataBytes >> 24) & 0xFF)
			outDataBytes += 1
			self.outData_[outDataBytes] = ((realDataBytes >> 16) & 0xFF)
			outDataBytes += 1
			self.outData_[outDataBytes] = ((realDataBytes >>  8) & 0xFF)
			outDataBytes += 1
			self.outData_[outDataBytes] = ((realDataBytes      ) & 0xFF)
			outDataBytes += 1
		if command == 0:
			pass
		else:
			self.outData_[outDataBytes] = command
			outDataBytes += 1
		self.outData_[outDataBytes:outDataBytes + dataBytes] = data[dataOffset:dataOffset + dataBytes]
		outDataBytes += dataBytes
		self.socket_.send(self.outData_[:outDataBytes])

	def run(self):
		try:
			super(Wrp_, self).onOpen_()
			while self.receiveResponse_():
				super(Wrp_, self).onMessage_(self.result__)
			super(Wrp_, self).onClose_()
		except IOError as e:
			super(Wrp_, self).onError_(e)

	def receiveResponse_(self):
		# 0                   1                   2                   3
		# 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		# +-+-+-+-+-------+-+-------------+-------------------------------+
		# |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
		# |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
		# |N|V|V|V|       |S|             |   (if payload len==126/127)   |
		# | |1|2|3|       |K|             |                               |
		# +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
		# |     Extended payload length continued, if payload len == 127  |
		# + - - - - - - - - - - - - - - - +-------------------------------+
		# |                               |Masking-key, if MASK set to 1  |
		# +-------------------------------+-------------------------------+
		# | Masking-key (continued)       |          Payload Data         |
		# +-------------------------------- - - - - - - - - - - - - - - - +
		# :                     Payload Data continued ...                :
		# + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
		# |                     Payload Data continued ...                |
		# +---------------------------------------------------------------+
		if not self.read_(2):
			if self.inDataBytes_ == 0:
				return False
			raise IOError("Unexpected end of stream")
		inDataBytes = (self.inData_[1] & 0x7F)
		self.shift_(2)
		if inDataBytes == 126:
			if not self.read_(2):
				raise IOError("Unexpected end of stream")
			inDataBytes = ((self.inData_[0] & 0xFF) <<  8) \
						| ((self.inData_[1] & 0xFF)      )
			self.shift_(2)
		elif inDataBytes == 127:
			if not self.read_(8):
				raise IOError("Unexpected end of stream")
			if self.inData_[0] != 0 \
			or self.inData_[1] != 0 \
			or self.inData_[2] != 0 \
			or self.inData_[3] != 0 or (self.inData_[4] & 0x80) != 0:
				raise IOError("Invalid payload length: " + str(inDataBytes))
			inDataBytes = ((self.inData_[4] & 0xFF) << 24) \
						| ((self.inData_[5] & 0xFF) << 16) \
						| ((self.inData_[6] & 0xFF) <<  8) \
						| ((self.inData_[7] & 0xFF)      )
			self.shift_(8)
		if inDataBytes < 0:
			raise IOError("Invalid payload length: " + str(inDataBytes))
		if not self.read_(inDataBytes):
			raise IOError("Unexpected end of stream")
		self.result_(inDataBytes)
		self.shift_(inDataBytes)
		return True

	def read_CRLFCRLF_(self):
		inDataBytes = self.inData_.find(b"\r\n\r\n")
		while inDataBytes == -1:
			inData = self.socket_.recv(4096)
			if not inData:
				return -1
			self.inData_ += inData
			self.inDataBytes_ += len(inData)
			inDataBytes = self.inData_.find(b"\r\n\r\n")
		return inDataBytes + 4

	def read_(self, inDataBytes):
		while self.inDataBytes_ < inDataBytes:
			inData = self.socket_.recv(inDataBytes - self.inDataBytes_)
			if not inData:
				return False
			self.inData_ += inData
			self.inDataBytes_ += len(inData)
		return True

	def result_(self, inDataBytes):
		if inDataBytes > 0 and self.inData_[inDataBytes - 1] == 0x0A: # '\n'
			inDataBytes -= 1
			if inDataBytes > 0 and self.inData_[inDataBytes - 1] == 0x0D: # '\r'
				inDataBytes -= 1
		try:
			result = self.inData_[:inDataBytes].decode("UTF-8")
		except:
			result = self.inData_[:inDataBytes].decode()
		self.result__ = result

	def shift_(self, inDataBytes):
		self.inDataBytes_ -= inDataBytes
		self.inData_[:inDataBytes] = []
