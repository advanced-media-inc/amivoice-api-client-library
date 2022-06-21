import sys
import threading
import time

try:
	from urllib import quote
	def u(s):
		if isinstance(s, str):
			return s.decode(sys.stdin.encoding)
		return s
	def q(s):
		return quote(s.encode("UTF-8"))
except:
	from urllib.parse import quote
	def u(s):
		return s
	def q(s):
		return quote(s, "UTF-8")

class Hrp(object):
	VERSION = "Hrp/1.0.03 Python/%d.%d.%d (%s)" % (sys.version_info[0], sys.version_info[1], sys.version_info[2], sys.platform)

	@staticmethod
	def getVersion():
		return Hrp.VERSION

	@staticmethod
	def construct(implementation = 1):
		if implementation == 1:
			from .Hrp_ import Hrp_
			return Hrp_()
		else:
			raise ValueError("Unknown implementation: " + implementation)

	def __init__(self):
		self.condition_ = threading.Condition()
		self.listener_ = None
		self.serverURL_ = None
		self.proxyServerName_ = None
		self.connectTimeout_ = 0
		self.receiveTimeout_ = 0
		self.grammarFileNames_ = None
		self.mode_ = None
		self.profileId_ = None
		self.profileWords_ = None
		self.segmenterType_ = None
		self.segmenterProperties_ = None
		self.keepFillerToken_ = None
		self.resultUpdatedInterval_ = None
		self.extension_ = None
		self.authorization_ = None
		self.domainId_ = None
		self.codec_ = None
		self.resultType_ = None
		self.resultEncoding_ = None
		self.serviceAuthorization_ = None
		self.voiceDetection_ = None
		self.acceptTopic_ = None
		self.contentType_ = None
		self.accept_ = None
		self.state_ = 0

	def setListener(self, listener):
		self.listener_ = listener

	def setServerURL(self, serverURL):
		self.serverURL_ = u(serverURL)

	def setProxyServerName(self, proxyServerName):
		self.proxyServerName_ = proxyServerName

	def setConnectTimeout(self, connectTimeout):
		self.connectTimeout_ = connectTimeout

	def setReceiveTimeout(self, receiveTimeout):
		self.receiveTimeout_ = receiveTimeout

	def setGrammarFileNames(self, grammarFileNames):
		self.grammarFileNames_ = u(grammarFileNames)

	def setMode(self, mode):
		self.mode_ = u(mode)

	def setProfileId(self, profileId):
		self.profileId_ = u(profileId)

	def setProfileWords(self, profileWords):
		self.profileWords_ = u(profileWords)

	def setSegmenterType(self, segmenterType):
		self.segmenterType_ = u(segmenterType)

	def setSegmenterProperties(self, segmenterProperties):
		self.segmenterProperties_ = u(segmenterProperties)

	def setKeepFillerToken(self, keepFillerToken):
		self.keepFillerToken_ = keepFillerToken

	def setResultUpdatedInterval(self, resultUpdatedInterval):
		self.resultUpdatedInterval_ = u(resultUpdatedInterval)

	def setExtension(self, extension):
		self.extension_ = u(extension)

	def setAuthorization(self, authorization):
		self.authorization_ = u(authorization)

	def setDomainId(self, domainId):
		self.domainId_ = u(domainId)

	def setCodec(self, codec):
		self.codec_ = u(codec)

	def setResultType(self, resultType):
		self.resultType_ = u(resultType)

	def setResultEncoding(self, resultEncoding):
		self.resultEncoding_ = u(resultEncoding)

	def setServiceAuthorization(self, serviceAuthorization):
		self.serviceAuthorization_ = u(serviceAuthorization)

	def setVoiceDetection(self, voiceDetection):
		self.voiceDetection_ = u(voiceDetection)

	def setAcceptTopic(self, acceptTopic):
		self.acceptTopic_ = u(acceptTopic)

	def setContentType(self, contentType):
		self.contentType_ = u(contentType)

	def setAccept(self, accept):
		self.accept_ = u(accept)

	def connect(self):
		if self.isConnected_():
			self.lastMessage_ = "WARNING: already connected to HTTP server"
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			return False
		if self.serverURL_ == None:
			self.lastMessage_ = "ERROR: can't connect to HTTP server (Missing server URL)"
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			return False
		try:
			self.state_ = 0
			self.connect_(self.serverURL_, self.proxyServerName_, self.connectTimeout_, self.receiveTimeout_)
		except Exception as e:
			self.lastMessage_ = "ERROR: can't connect to HTTP server (" + u(str(e)) + "): " + self.serverURL_
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			self.disconnect_()
			return False
		self.lastMessage_ = "INFO: connected to HTTP server: " + self.serverURL_
		if self.listener_ != None:
			self.listener_.TRACE(self.lastMessage_)
		return True

	def connect_(self, serverURL, proxyServerName, connectTimeout, receiveTimeout):
		raise NotImplementedError()

	def disconnect(self):
		if not self.isConnected_():
			self.lastMessage_ = "WARNING: already disconnected from HTTP server"
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			return False
		self.disconnect_()
		self.lastMessage_ = "INFO: disconnected from HTTP server"
		if self.listener_ != None:
			self.listener_.TRACE(self.lastMessage_)
		return True

	def disconnect_(self):
		raise NotImplementedError()

	def isConnected(self):
		return self.isConnected_()

	def isConnected_(self):
		raise NotImplementedError()

	# type="s"  dataBytes>0 - Single part & no chunked encoding
	# type="sc" dataBytes=0 - Single part & chunked encoding
	# type="m"  dataBytes>0 - Multi parts & no chunked encoding
	# type="mc" dataBytes=0 - Multi parts & chunked encoding
	def feedDataResume(self, type = "mc", dataBytes = 0):
		if type == None:
			type = "mc"
		if dataBytes > 0:
			if type.find('m') == -1:
				if type.find('c') == -1:
					type = "s "
				else:
					type = "sc"
			else:
				if type.find('c') == -1:
					type = "m "
				else:
					type = "mc"
		else:
			if type.find('m') == -1:
				type = "sc"
			else:
				type = "mc"
		with self.condition_:
			if not self.isConnected_():
				self.lastMessage_ = "WARNING: already disconnected from HTTP server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			if self.state_ != 0:
				self.lastMessage_ = "WARNING: already started feeding data to HTTP server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			domainId = ""
			if self.domainId_ != None:
				domainId += self.domainId_
			else:
				if self.grammarFileNames_ != None:
					domainId += "grammarFileNames="
					try:
						domainId += q(self.grammarFileNames_)
					except:
						pass
				if self.mode_ != None:
					if len(domainId) > 0:
						domainId += ' '
					domainId += "mode="
					try:
						domainId += q(self.mode_)
					except:
						pass
				if self.profileId_ != None:
					if len(domainId) > 0:
						domainId += ' '
					domainId += "profileId="
					try:
						domainId += q(self.profileId_)
					except:
						pass
				if self.profileWords_ != None:
					if len(domainId) > 0:
						domainId += ' '
					domainId += "profileWords="
					try:
						domainId += q(self.profileWords_)
					except:
						pass
				if self.segmenterType_ != None:
					if len(domainId) > 0:
						domainId += ' '
					domainId += "segmenterType="
					try:
						domainId += q(self.segmenterType_)
					except:
						pass
				if self.segmenterProperties_ != None:
					if len(domainId) > 0:
						domainId += ' '
					domainId += "segmenterProperties="
					try:
						domainId += q(self.segmenterProperties_)
					except:
						pass
				if self.keepFillerToken_ != None:
					if len(domainId) > 0:
						domainId += ' '
					domainId += "keepFillerToken="
					try:
						domainId += q(self.keepFillerToken_)
					except:
						pass
				if self.resultUpdatedInterval_ != None:
					if len(domainId) > 0:
						domainId += ' '
					domainId += "resultUpdatedInterval="
					try:
						domainId += q(self.resultUpdatedInterval_)
					except:
						pass
				if self.extension_ != None:
					if len(domainId) > 0:
						domainId += ' '
					domainId += "extension="
					try:
						domainId += q(self.extension_)
					except:
						pass
				if self.authorization_ != None:
					if len(domainId) > 0:
						domainId += ' '
					domainId += "authorization="
					try:
						domainId += q(self.authorization_)
					except:
						pass
			try:
				self.state_ = 1
				self.sendRequestHeader_(domainId,
										self.codec_,
										self.resultType_,
										self.resultEncoding_,
										self.serviceAuthorization_,
										self.voiceDetection_,
										self.acceptTopic_,
										self.contentType_,
										self.accept_, type, dataBytes)
				self.state_ = 2
				self.lastMessage_ = "INFO: started feeding data to HTTP server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
			except (IOError, OSError) as e:
				self.lastMessage_ = "ERROR: can't start feeding data to HTTP server (" + u(str(e)) + ")"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			return True

	def feedData(self, data, dataOffset, dataBytes):
		with self.condition_:
			if not self.isConnected_():
				self.lastMessage_ = "WARNING: already disconnected from HTTP server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			if self.state_ != 2 and self.state_ != 3:
				self.lastMessage_ = "WARNING: already stopped feeding data to HTTP server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			try:
				self.state_ = 3
				self.sendRequestBody_(data, dataOffset, dataBytes)
			except (IOError, OSError) as e:
				self.lastMessage_ = "ERROR: can't feed data to HTTP server (" + u(str(e)) + ")"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			return True

	def feedDataPause(self):
		with self.condition_:
			if not self.isConnected_():
				self.lastMessage_ = "WARNING: already disconnected from HTTP server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			if self.state_ != 2 and self.state_ != 3:
				self.lastMessage_ = "WARNING: already stopped feeding data to HTTP server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			try:
				self.state_ = 4
				self.sendRequestBody_(None, 0, 0)
				while self.state_ == 4:
					self.condition_.wait()
				if self.state_ == 5:
					self.lastMessage_ = "ERROR: can't stop feeding data to HTTP server"
					if self.listener_ != None:
						self.listener_.TRACE(self.lastMessage_)
					return False
				self.lastMessage_ = "INFO: stopped feeding data to HTTP server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
			except InterruptedError:
				self.lastMessage_ = "WARNING: interrupted to stop feeding data to HTTP server"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			except (IOError, OSError) as e:
				self.lastMessage_ = "ERROR: can't stop feeding data to HTTP server (" + u(str(e)) + ")"
				if self.listener_ != None:
					self.listener_.TRACE(self.lastMessage_)
				return False
			return True

	def getLastMessage(self):
		return self.lastMessage_

	def sleep(self, timeout):
		try:
			time.sleep(timeout / 1000.0)
		except:
			pass

	def sendRequestHeader_(self, domainId,
								 codec,
								 resultType,
								 resultEncoding,
								 serviceAuthorization,
								 voiceDetection,
								 acceptTopic,
								 contentType,
								 accept, type, dataBytes):
		raise NotImplementedError()

	def sendRequestBody_(self, data, dataOffset, dataBytes):
		raise NotImplementedError()

	def onOpen_(self, sessionId):
		if self.listener_ != None:
			self.listener_.resultCreated(sessionId)

	def onClose_(self):
		with self.condition_:
			self.state_ = 0
			self.condition_.notifyAll()

	def onError_(self, cause):
		with self.condition_:
			if self.state_ == 0 or self.state_ == 5:
				return
			self.lastMessage_ = "ERROR: caught exception (" + u(str(cause)) + ")"
			if self.listener_ != None:
				self.listener_.TRACE(self.lastMessage_)
			self.state_ = 5
			self.condition_.notifyAll()

	def onMessage_(self, resultData):
		if self.listener_ != None: # {
			if resultData.endswith("...") or resultData.endswith("...\"}"):
				self.listener_.resultUpdated(resultData)
			else:
				self.listener_.resultFinalized(resultData)
