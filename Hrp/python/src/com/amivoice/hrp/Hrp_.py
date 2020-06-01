import base64
import random
import socket
import ssl
import threading

from .Hrp import Hrp

try:
	from urllib import quote
	def bq(s):
		return quote(s.encode("UTF-8"))
except:
	from urllib.parse import quote
	def bq(s):
		return quote(s, "UTF-8").encode()

class Hrp_(Hrp):
	_serverCertificateVerification = True

	@staticmethod
	def _basic(userName, password):
		return base64.b64encode((userName + ":" + password).encode())

	def __init__(self):
		super(Hrp_, self).__init__()
		self.host_ = None
		self.resource_ = None
		self.proxyUserName_ = None
		self.proxyPassword_ = None
		self.socket_ = None
		self.requestBodyChunked_ = None
		self.requestBodyParted_ = None
		self.requestBodyPartedOffset_ = 0
		self.inData_ = bytearray()
		self.inDataBytes_ = 0
		self.inDataBytes__ = 0
		self.inDataEncoding__ = None
		self.result__ = None
		self.thread_ = None

	def connect_(self, serverURL, proxyServerName, connectTimeout, receiveTimeout):
		colonDoubleSlashIndex = serverURL.find("://")
		if colonDoubleSlashIndex == -1:
			raise ValueError("Invalid server URL")
		scheme = serverURL[:colonDoubleSlashIndex]
		if scheme != "http" and scheme != "https":
			raise ValueError("Invalid scheme: " + scheme)
		slashIndex = serverURL.find('/', colonDoubleSlashIndex + 3)
		if slashIndex != -1:
			hostName = serverURL[colonDoubleSlashIndex + 3:slashIndex]
			self.resource_ = serverURL[slashIndex:]
		else:
			hostName = serverURL[colonDoubleSlashIndex + 3:]
			self.resource_ = "/"
		colonIndex = hostName.find(':', hostName.find(']') + 1)
		if colonIndex != -1:
			try:
				port = int(hostName[colonIndex + 1:])
			except:
				raise ValueError("Invalid port: " + hostName[colonIndex + 1:])
			hostName = hostName[:colonIndex]
		else:
			if scheme == "https":
				port = 443
			else:
				port = 80
		if port == 80:
			self.host_ = hostName
		else:
			self.host_ = hostName + ":" + str(port)
		proxyHostName = None
		proxyPort = 0
		self.proxyUserName_ = None
		self.proxyPassword_ = None
		if proxyServerName != None:
			proxyHostName = proxyServerName
			atIndex = proxyHostName.rfind('@')
			if atIndex != -1:
				colonIndex = proxyHostName.find(':')
				if colonIndex != -1 and colonIndex < atIndex:
					self.proxyUserName_ = proxyHostName[:colonIndex]
					self.proxyPassword_ = proxyHostName[colonIndex + 1:atIndex]
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
			if scheme == "https":
				proxyRequestHeader = bytearray()
				proxyRequestHeader.extend(b"CONNECT ")
				proxyRequestHeader.extend(hostName.encode())
				proxyRequestHeader.extend(b":")
				proxyRequestHeader.extend(str(port).encode())
				proxyRequestHeader.extend(b" HTTP/1.1\r\n")
				proxyRequestHeader.extend(b"Host: ")
				proxyRequestHeader.extend(self.host_.encode())
				proxyRequestHeader.extend(b"\r\n")
				proxyRequestHeader.extend(b"User-Agent: ")
				proxyRequestHeader.extend(Hrp.getVersion().encode())
				proxyRequestHeader.extend(b"\r\n")
				if self.proxyUserName_ != None and self.proxyPassword_ != None:
					proxyRequestHeader.extend(b"Proxy-Authorization: Basic ")
					proxyRequestHeader.extend(Hrp_._basic(self.proxyUserName_, self.proxyPassword_))
					proxyRequestHeader.extend(b"\r\n")
					self.proxyUserName_ = None
					self.proxyPassword_ = None
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
				self.resource_ = serverURL
		if scheme == "https":
			try:
				if not Hrp_._serverCertificateVerification:
					self.socket_ = ssl.wrap_socket(self.socket_)
				else:
					self.socket_ = ssl.wrap_socket(self.socket_, cert_reqs = ssl.CERT_REQUIRED, ca_certs = ssl.get_default_verify_paths().cafile)
					ssl.match_hostname(self.socket_.getpeercert(), hostName)
			except:
				self.socket_.close()
				self.socket_ = None
				raise
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

	def sendRequestHeader_(self, domainId,
								 codec,
								 resultType,
								 resultEncoding,
								 serviceAuthorization,
								 voiceDetection,
								 acceptTopic,
								 contentType,
								 accept, type, dataBytes):
		self.requestBodyChunked_ = None
		self.requestBodyParted_ = None
		self.requestBodyPartedOffset_ = 0
		self.inDataBytes_ = 0
		self.inDataBytes__ = 0
		self.inDataEncoding__ = None
		self.result__ = ""
		requestHeader = bytearray()
		requestHeader.extend(b"POST ")
		requestHeader.extend(self.resource_.encode())
		if type[0] != 'm':
			delimiter = 0x3F if self.resource_.find('?') == -1 else 0x26 # '?' '&'
			if domainId != None and len(domainId) > 0:
				requestHeader.append(delimiter)
				delimiter = 0x26 # '&'
				requestHeader.extend(b"d=")
				requestHeader.extend(bq(domainId))
			if codec != None and len(codec) > 0:
				requestHeader.append(delimiter)
				delimiter = 0x26 # '&'
				requestHeader.extend(b"c=")
				requestHeader.extend(bq(codec))
			if resultType != None and len(resultType) > 0:
				requestHeader.append(delimiter)
				delimiter = 0x26 # '&'
				requestHeader.extend(b"r=")
				requestHeader.extend(bq(resultType))
			if resultEncoding != None and len(resultEncoding) > 0:
				requestHeader.append(delimiter)
				delimiter = 0x26 # '&'
				requestHeader.extend(b"e=")
				requestHeader.extend(bq(resultEncoding))
			if serviceAuthorization != None and len(serviceAuthorization) > 0:
				requestHeader.append(delimiter)
				delimiter = 0x26 # '&'
				requestHeader.extend(b"u=")
				requestHeader.extend(bq(serviceAuthorization))
			if voiceDetection != None:
				requestHeader.append(delimiter)
				delimiter = 0x26 # '&'
				requestHeader.extend(b"v=")
				requestHeader.extend(bq(voiceDetection))
		else:
			requestBody = bytearray()
			if domainId != None and len(domainId) > 0:
				requestBody.extend(b"\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"d\"\r\n\r\n")
				requestBody.extend(domainId.encode("UTF-8"))
			if codec != None and len(codec) > 0:
				requestBody.extend(b"\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"c\"\r\n\r\n")
				requestBody.extend(codec.encode("UTF-8"))
			if resultType != None and len(resultType) > 0:
				requestBody.extend(b"\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"r\"\r\n\r\n")
				requestBody.extend(resultType.encode("UTF-8"))
			if resultEncoding != None and len(resultEncoding) > 0:
				requestBody.extend(b"\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"e\"\r\n\r\n")
				requestBody.extend(resultEncoding.encode("UTF-8"))
			if serviceAuthorization != None and len(serviceAuthorization) > 0:
				requestBody.extend(b"\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"u\"\r\n\r\n")
				requestBody.extend(serviceAuthorization.encode("UTF-8"))
			if voiceDetection != None:
				requestBody.extend(b"\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"v\"\r\n\r\n")
				requestBody.extend(voiceDetection.encode("UTF-8"))
			requestBody.extend(b"\r\n--1234567890ABCDEFGHIJ\r\nContent-Disposition: form-data; name=\"a\"\r\n\r\n")
			requestBody.extend(b"\r\n--1234567890ABCDEFGHIJ--\r\n")
			self.requestBodyParted_ = requestBody # [70+n+][70+n+][70+n+][70+n+][70+n+][70+n+]70+28 bytes
		requestHeader.extend(b" HTTP/1.1\r\n")
		requestHeader.extend(b"Host: ")
		requestHeader.extend(self.host_.encode())
		requestHeader.extend(b"\r\n")
		requestHeader.extend(b"User-Agent: ")
		requestHeader.extend(Hrp.getVersion().encode())
		requestHeader.extend(b"\r\n")
		if self.proxyUserName_ != None and self.proxyPassword_ != None:
			requestHeader.extend(b"Proxy-Authorization: Basic ")
			requestHeader.extend(Hrp_._basic(self.proxyUserName_, self.proxyPassword_))
			requestHeader.extend(b"\r\n")
		if type[1] != 'c':
			requestHeader.extend(b"Content-Length: ")
			if self.requestBodyParted_ == None:
				requestHeader.extend(str(dataBytes).encode())
			else:
				requestHeader.extend(str(dataBytes + len(self.requestBodyParted_)).encode())
			requestHeader.extend(b"\r\n")
		else:
			self.requestBodyChunked_ = b"0\r\n\r\n"
			requestHeader.extend(b"Transfer-Encoding: chunked\r\n")
		if self.requestBodyParted_ == None:
			if acceptTopic != None and len(acceptTopic) > 0:
				requestHeader.extend(b"Accept-Topic: ")
				requestHeader.extend(acceptTopic.encode())
				requestHeader.extend(b"\r\n")
			if contentType != None and len(contentType) > 0:
				requestHeader.extend(b"Content-Type: ")
				requestHeader.extend(contentType.encode())
				requestHeader.extend(b"\r\n")
			if accept != None and len(accept) > 0:
				requestHeader.extend(b"Accept: ")
				requestHeader.extend(accept.encode())
				requestHeader.extend(b"\r\n")
		else:
			requestHeader.extend(b"Content-Type: multipart/form-data; boundary=1234567890ABCDEFGHIJ\r\n")
		requestHeader.extend(b"\r\n")
		self.socket_.send(requestHeader)

	def sendRequestBody_(self, data, dataOffset, dataBytes):
		if data != None and dataBytes > 0:
			if self.requestBodyParted_ != None:
				if self.requestBodyPartedOffset_ == 0:
					self.requestBodyPartedOffset_ = len(self.requestBodyParted_) - 28
					if self.requestBodyChunked_ != None:
						self.socket_.send(format(self.requestBodyPartedOffset_, 'x').encode())
						self.socket_.send(self.requestBodyChunked_[1:3])
					self.socket_.send(self.requestBodyParted_[:self.requestBodyPartedOffset_])
					if self.requestBodyChunked_ != None:
						self.socket_.send(self.requestBodyChunked_[3:5])
			if self.requestBodyChunked_ != None:
				self.socket_.send(format(dataBytes, 'x').encode())
				self.socket_.send(self.requestBodyChunked_[1:3])
			self.socket_.send(data[dataOffset:dataOffset + dataBytes])
			if self.requestBodyChunked_ != None:
				self.socket_.send(self.requestBodyChunked_[3:5])
		else:
			if self.requestBodyParted_ != None:
				if self.requestBodyChunked_ != None:
					self.socket_.send(format(len(self.requestBodyParted_) - self.requestBodyPartedOffset_, 'x').encode())
					self.socket_.send(self.requestBodyChunked_[1:3])
				self.socket_.send(self.requestBodyParted_[self.requestBodyPartedOffset_:])
				if self.requestBodyChunked_ != None:
					self.socket_.send(self.requestBodyChunked_[3:5])
					self.socket_.send(self.requestBodyChunked_)
			else:
				if self.requestBodyChunked_ != None:
					self.socket_.send(self.requestBodyChunked_)

	def run(self):
		try:
			while True:
				sessionId = self.receiveResponseHeader_()
				super(Hrp_, self).onOpen_(sessionId)
				if self.inDataBytes__ > 0:
					self.receiveResponseBody_()
					crlfIndex = self.result__.find("\r\n")
					while crlfIndex != -1:
						result = self.result__[:crlfIndex]
						super(Hrp_, self).onMessage_(result)
						self.result__ = self.result__[crlfIndex + 2:]
						crlfIndex = self.result__.find("\r\n")
				else:
					while self.receiveChunkedResponseBody_():
						crlfIndex = self.result__.find("\r\n")
						while crlfIndex != -1:
							result = self.result__[:crlfIndex]
							super(Hrp_, self).onMessage_(result)
							self.result__ = self.result__[crlfIndex + 2:]
							crlfIndex = self.result__.find("\r\n")
				super(Hrp_, self).onClose_()
		except IOError as e:
			super(Hrp_, self).onError_(e)

	def receiveResponseHeader_(self):
		responseHeaderBytes = self.read_CRLFCRLF_()
		if responseHeaderBytes == -1:
			raise IOError("Unexpected end of stream")
		responseHeader = self.inData_[:responseHeaderBytes]
		if (responseHeader[ 8] != 0x20   # ' '
		 or responseHeader[ 9] != 0x32   # '2'
		 or responseHeader[10] != 0x30   # '0'
		 or responseHeader[11] != 0x30   # '0'
		 or responseHeader[12] != 0x20): # ' '
			raise IOError("Invalid response code: " + responseHeader[9:12].decode())
		index1 = responseHeader.find(b"\r\nX-Session-ID: ")
		if index1 == -1:
			raise IOError("Missing session id")
		index1 += 16
		index2 = responseHeader.find(b"\r\n", index1)
		if index2 == -1:
			raise IOError("Missing session id")
		sessionId = responseHeader[index1:index2].decode()
		self.inDataEncoding__ = "UTF-8"
		index1 = responseHeader.find(b"\r\nContent-Type: ")
		if index1 != -1:
			index1 += 16
			index2 = responseHeader.find(b"\r\n", index1)
			if index2 != -1:
				index3 = responseHeader.find(b"; charset=", index1)
				if index3 != -1 and index3 < index2:
					index3 += 10
					self.inDataEncoding__ = responseHeader[index3:index2].decode().upper()
					if self.inDataEncoding__ == "SHIFT_JIS":
						self.inDataEncoding__ = "MS932"
		index1 = responseHeader.find(b"\r\nContent-Length: ")
		if index1 != -1:
			index1 += 18
			index2 = responseHeader.find(b"\r\n", index1)
			if index2 != -1:
				try:
					self.inDataBytes__ = int(responseHeader[index1:index2].decode())
				except:
					raise IOError("Invalid content length: " + responseHeader[index1:index2].decode())
		self.shift_(responseHeaderBytes)
		return sessionId

	def receiveResponseBody_(self):
		if not self.read_(self.inDataBytes__):
			raise IOError("Unexpected end of stream")
		self.result_(self.inDataBytes__)
		self.shift_(self.inDataBytes__)

	def receiveChunkedResponseBody_(self):
		chunkedResponseHeaderBytes = self.read_CRLF_()
		if chunkedResponseHeaderBytes == -1:
			raise IOError("Unexpected end of stream")
		self.inDataBytes__ = 0
		for i in range(0, chunkedResponseHeaderBytes):
			c = self.inData_[i]
			if c >= 0x30 and c <= 0x39: # '0' '9'
				self.inDataBytes__ <<= 4
				self.inDataBytes__ += c - 0x30 # '0'
			elif c >= 0x41 and c <= 0x46: # 'A' 'F'
				self.inDataBytes__ <<= 4
				self.inDataBytes__ += c - 0x41 + 10 # 'A'
			elif c >= 0x61 and c <= 0x66: # 'a' 'f'
				self.inDataBytes__ <<= 4
				self.inDataBytes__ += c - 0x61 + 10 # 'a'
			else:
				break
		self.shift_(chunkedResponseHeaderBytes)
		if self.inDataBytes__ == 0:
			if not self.read_(2):
				raise IOError("Unexpected end of stream")
			self.shift_(2)
			return False
		if not self.read_(self.inDataBytes__ + 2):
			raise IOError("Unexpected end of stream")
		self.result_(self.inDataBytes__)
		self.shift_(self.inDataBytes__ + 2)
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

	def read_CRLF_(self):
		inDataBytes = self.inData_.find(b"\r\n")
		while inDataBytes == -1:
			inData = self.socket_.recv(4096)
			if not inData:
				return -1
			self.inData_ += inData
			self.inDataBytes_ += len(inData)
			inDataBytes = self.inData_.find(b"\r\n")
		return inDataBytes + 2

	def read_(self, inDataBytes):
		while self.inDataBytes_ < inDataBytes:
			inData = self.socket_.recv(inDataBytes - self.inDataBytes_)
			if not inData:
				return False
			self.inData_ += inData
			self.inDataBytes_ += len(inData)
		return True

	def result_(self, inDataBytes):
		try:
			result = self.inData_[:inDataBytes].decode(self.inDataEncoding__)
		except:
			result = self.inData_[:inDataBytes].decode()
		self.result__ += result

	def shift_(self, inDataBytes):
		self.inDataBytes_ -= inDataBytes
		self.inData_[:inDataBytes] = []
