class WrpListener:
	#@abstractmethod
	def utteranceStarted(startTime):
		pass
	#@abstractmethod
	def utteranceEnded(endTime):
		pass
	#@abstractmethod
	def resultCreated():
		pass
	#@abstractmethod
	def resultUpdated(result):
		pass
	#@abstractmethod
	def resultFinalized(result):
		pass
	#@abstractmethod
	def eventNotified(eventId, eventMessage):
		pass
	#@abstractmethod
	def TRACE(message):
		pass
