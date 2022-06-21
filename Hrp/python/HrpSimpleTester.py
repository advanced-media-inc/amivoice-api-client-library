# encoding: UTF-8
import sys
import time
import json
# <!-- バイトコードキャッシュファイルの作成を抑制するために...
sys.dont_write_bytecode = True
# -->
import com.amivoice.hrp.Hrp
import com.amivoice.hrp.HrpListener

class HrpSimpleTester(com.amivoice.hrp.HrpListener):
	@staticmethod
	def main(args):
		if len(args) < 4:
			print("Usage: python HrpSimpleTester.py <url> <audioFileName> <codec> <grammarFileNames> [<authorization>]")
			return

		serverURL = args[0]
		audioFileName = args[1]
		codec = args[2]
		grammarFileNames = args[3]
		authorization = args[4] if len(args) > 4 else None

		# HTTP 音声認識サーバイベントリスナの作成
		listener = HrpSimpleTester()

		# HTTP 音声認識サーバの初期化
		hrp = com.amivoice.hrp.Hrp.construct()
		hrp.setListener(listener)
		hrp.setServerURL(serverURL)
		hrp.setCodec(codec)
		hrp.setGrammarFileNames(grammarFileNames)
		hrp.setAuthorization(authorization)

		# HTTP 音声認識サーバへの接続
		if not hrp.connect():
			print(hrp.getLastMessage())
			print(u"HTTP 音声認識サーバ %s への接続に失敗しました。" % serverURL)
			return

		try:
			# HTTP 音声認識サーバへの音声データの送信の開始
			if not hrp.feedDataResume():
				print(hrp.getLastMessage())
				print(u"HTTP 音声認識サーバへの音声データの送信の開始に失敗しました。")
				return

			try:
				with open(audioFileName, "rb") as audioStream:
					# 音声データファイルからの音声データの読み込み
					audioData = audioStream.read(4096)
					while len(audioData) > 0:
						# 微小時間のスリープ
						hrp.sleep(1)

						# HTTP 音声認識サーバへの音声データの送信
						if not hrp.feedData(audioData, 0, len(audioData)):
							print(hrp.getLastMessage())
							print(u"HTTP 音声認識サーバへの音声データの送信に失敗しました。")
							break

						# 音声データファイルからの音声データの読み込み
						audioData = audioStream.read(4096)
			except:
				print(u"音声データファイル %s の読み込みに失敗しました。" % audioFileName)

			# HTTP 音声認識サーバへの音声データの送信の完了
			if not hrp.feedDataPause():
				print(hrp.getLastMessage())
				print(u"HTTP 音声認識サーバへの音声データの送信の完了に失敗しました。")
				return
		finally:
			# HTTP 音声認識サーバからの切断
			hrp.disconnect()

	def __init__(self):
		pass

	def resultCreated(self, sessionId):
#		print("C %s" % sessionId)
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

	def TRACE(self, message):
		pass

	def text_(self, result):
		try:
			return json.loads(result)["text"]
		except:
			return None

HrpSimpleTester.main(sys.argv[1:])
