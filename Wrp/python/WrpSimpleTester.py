# encoding: UTF-8
import sys
import time
import json
# <!-- バイトコードキャッシュファイルの作成を抑制するために...
sys.dont_write_bytecode = True
# -->
import com.amivoice.wrp.Wrp
import com.amivoice.wrp.WrpListener

class WrpSimpleTester(com.amivoice.wrp.WrpListener):
	@staticmethod
	def main(args):
		if len(args) < 4:
			print("Usage: python WrpSimpleTester.py <url> <audioFileName> <codec> <grammarFileNames> [<authorization>]")
			return

		serverURL = args[0]
		audioFileName = args[1]
		codec = args[2]
		grammarFileNames = args[3]
		authorization = args[4] if len(args) > 4 else None

		# WebSocket 音声認識サーバイベントリスナの作成
		listener = WrpSimpleTester()

		# WebSocket 音声認識サーバの初期化
		wrp = com.amivoice.wrp.Wrp.construct()
		wrp.setListener(listener)
		wrp.setServerURL(serverURL)
		wrp.setCodec(codec)
		wrp.setGrammarFileNames(grammarFileNames)
		wrp.setAuthorization(authorization)

		# WebSocket 音声認識サーバへの接続
		if not wrp.connect():
			print(wrp.getLastMessage())
			print(u"WebSocket 音声認識サーバ %s への接続に失敗しました。" % serverURL)
			return

		try:
			# WebSocket 音声認識サーバへの音声データの送信の開始
			if not wrp.feedDataResume():
				print(wrp.getLastMessage())
				print(u"WebSocket 音声認識サーバへの音声データの送信の開始に失敗しました。")
				return

			try:
				with open(audioFileName, "rb") as audioStream:
					# 音声データファイルからの音声データの読み込み
					audioData = audioStream.read(4096)
					while len(audioData) > 0:
						# 微小時間のスリープ
						wrp.sleep(1)

						# 認識結果情報待機数が 1 以下になるまでスリープ
						maxSleepTime = 50000
						while wrp.getWaitingResults() > 1 and maxSleepTime > 0:
							wrp.sleep(100)
							maxSleepTime -= 100

						# WebSocket 音声認識サーバへの音声データの送信
						if not wrp.feedData(audioData, 0, len(audioData)):
							print(wrp.getLastMessage())
							print(u"WebSocket 音声認識サーバへの音声データの送信に失敗しました。")
							break

						# 音声データファイルからの音声データの読み込み
						audioData = audioStream.read(4096)
			except:
				print(u"音声データファイル %s の読み込みに失敗しました。" % audioFileName)

			# WebSocket 音声認識サーバへの音声データの送信の完了
			if not wrp.feedDataPause():
				print(wrp.getLastMessage())
				print(u"WebSocket 音声認識サーバへの音声データの送信の完了に失敗しました。")
				return
		finally:
			# WebSocket 音声認識サーバからの切断
			wrp.disconnect()

	def __init__(self):
		pass

	def utteranceStarted(self, startTime):
#		print("S %d" % startTime)
		pass

	def utteranceEnded(self, endTime):
#		print("E %d" % endTime)
		pass

	def resultCreated(self):
#		print("C")
		pass

	def resultUpdated(self, result):
#		print("U %s" % result)
		pass

	def resultFinalized(self, result):
#		print("F %s" % result)
		print(result)
		text = self.text_(result)
		if text != None:
			print(" -> %s" % text)

	def eventNotified(self, eventId, eventMessage):
#		print(eventId + " " + eventMessage)
		pass

	def TRACE(self, message):
		pass

	def text_(self, result):
		try:
			return json.loads(result)["text"]
		except:
			return None

WrpSimpleTester.main(sys.argv[1:])
