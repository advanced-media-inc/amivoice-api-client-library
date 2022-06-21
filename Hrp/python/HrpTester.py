# encoding: UTF-8
import os
import sys
import time
import json
# <!-- バイトコードキャッシュファイルの作成を抑制するために...
sys.dont_write_bytecode = True
# -->
import com.amivoice.hrp.Hrp
import com.amivoice.hrp.HrpListener

class HrpTester(com.amivoice.hrp.HrpListener):
	@staticmethod
	def main(args):
		# HTTP 音声認識サーバ URL
		serverURL = None
		# プロキシサーバ名
		proxyServerName = None
		# 音声データファイル名
		audioFileNames = []
		# グラマファイル名
		grammarFileNames = None
		# プロファイル ID
		profileId = None
		# プロファイル登録単語
		profileWords = None
		# セグメンタプロパティ
		segmenterProperties = None
		# フィラー単語を保持するかどうか
		keepFillerToken = None
		# 認識中イベント発行間隔
		resultUpdatedInterval = None
		# 拡張情報
		extension = None
		# サービス認証キー文字列
		authorization = None
		# ドメイン ID (d=)
		domainId = None
		# 音声データ形式 (c=)
		codec = None
		# 認識結果タイプ (r=)
		resultType = None
		# 認識結果文字エンコーディング (e=)
		resultEncoding = None
		# サービス認証キー文字列 (u=)
		serviceAuthorization = None
		# 発話区間検出パラメータ文字列 (v=)
		voiceDetection = None
		# HTTP リクエストヘッダ群
		acceptTopic = None
		contentType = None
		accept = None
		# 処理タイプ ("s"／"sc"／"m"／"mc")
		type = "mc"
		# 接続タイムアウト
		connectTimeout = 5000
		# 受信タイムアウト
		receiveTimeout = 0
		# 処理ループ (1～)
		loop = 1
		# スリープ時間
		sleepTime = -2
		# 詳細出力
		verbose = False
		# 実装タイプ
		implementation = 1

		# 引数のチェック
		for arg in args:
			if arg.startswith("g="):
				grammarFileNames = arg[2:]
			elif arg.startswith("i="):
				profileId = arg[2:]
			elif arg.startswith("w="):
				profileWords = arg[2:]
			elif arg.startswith("op="):
				segmenterProperties = arg[3:]
			elif arg.startswith("of="):
				keepFillerToken = arg[3:]
			elif arg.startswith("oi="):
				resultUpdatedInterval = arg[3:]
			elif arg.startswith("oe="):
				extension = arg[3:]
			elif arg.startswith("ou="):
				authorization = arg[3:]
			elif arg.startswith("d="):
				domainId = arg[2:]
			elif arg.startswith("c="):
				codec = arg[2:]
			elif arg.startswith("r="):
				resultType = arg[2:]
			elif arg.startswith("e="):
				resultEncoding = arg[2:]
			elif arg.startswith("u="):
				serviceAuthorization = arg[2:]
			elif arg.startswith("v="):
				voiceDetection = arg[2:]
			elif arg.startswith("hd="):
				acceptTopic = arg[3:]
			elif arg.startswith("hc="):
				contentType = arg[3:]
			elif arg.startswith("hr="):
				accept = arg[3:]
			elif arg.startswith("-t"):
				type = arg[2:]
			elif arg.startswith("-x"):
				proxyServerName = arg[2:]
			elif arg.startswith("-c"):
				connectTimeout = int(arg[2:])
			elif arg.startswith("-r"):
				receiveTimeout = int(arg[2:])
			elif arg.startswith("-l"):
				loop = int(arg[2:])
			elif arg.startswith("-e"):
				if len(arg) > 2:
					sleepTime = int(arg[2:])
				else:
					sleepTime = -1
			elif arg.startswith("-v"):
				verbose = True
			elif arg.startswith("-2"):
				implementation = 2
			elif serverURL == None:
				serverURL = arg
			else:
				audioFileNames.append(arg)
			if verbose:
				print("DEBUG: %s" % arg)
		if len(audioFileNames) == 0:
			print("Usage: python HrpTester.py [<parameters/options>]")
			print("                            <url>")
			print("                             <audioFileName>...")
			print("Parameters:")
			print("  g=<grammarFileNames>")
			print("  i=<profileId>")
			print("  w=<profileWords>")
			print("  op=<segmenterProperties>")
			print("  of=<keepFillerToken>")
			print("  oi=<resultUpdatedInterval>")
			print("  oe=<extension>")
			print("  ou=<authorization>")
			print("  d=<domainId>")
			print("  c=<codec>")
			print("  r=<resultType>")
			print("  e=<resultEncoding>")
			print("  u=<serviceAuthorization>")
			print("  v=<voiceDetection>")
			print("  hd=<acceptTopic>            HTTP 'Accept-Topic' header instead d=<domainId>")
			print("  hc=<contentType>            HTTP 'Content-Type' header instead c=<codec>")
			print("  hr=<accept>                 HTTP 'Accept' header instead r=<resultType>")
			print("Options:")
			print("  -t<type>                    (default: -tmc)")
			print("      <type>='s'              Single part & no chunked encoding")
			print("      <type>='sc'             Single part & chunked encoding")
			print("      <type>='m'              Multi parts & no chunked encoding")
			print("      <type>='mc'             Multi parts & chunked encoding")
			print("  -x<proxyServerName>         (default: -x)")
			print("  -c<connectionTimeout>       (default: -c5000)")
			print("  -r<receiveTimeout>          (default: -r0)")
			print("  -l<loop>                    (default: -l1)")
			print("  -e                          realtime simulation (default: -)")
			print("  -v                          verbose output (default: -)")
			return

		# 開始時間の取得
		startTime = time.time()

		# スリープ時間の計算
		if sleepTime == -1:
			if codec != None:
				if codec.upper() == "22K" or codec.upper() == "MSB22K" or codec.upper() == "LSB22K":
					sleepTime = 4096 * 1000 / 2 / 22050
				elif codec.upper() == "16K" or codec.upper() == "MSB16K" or codec.upper() == "LSB16K":
					sleepTime = 4096 * 1000 / 2 / 16000
				elif codec.upper() == "11K" or codec.upper() == "MSB11K" or codec.upper() == "LSB11K":
					sleepTime = 4096 * 1000 / 2 / 11025
				elif codec.upper() == "8K" or codec.upper() == "MSB8K" or codec.upper() == "LSB8K":
					sleepTime = 4096 * 1000 / 2 / 8000
				elif codec.upper() == "MULAW" or codec.upper() == "ALAW":
					sleepTime = 4096 * 1000 / 1 / 8000
				else:
					sleepTime = 4096 * 1000 / 2 / 16000 # 16K が指定されたものとして計算...
			else:
				sleepTime = 4096 * 1000 / 2 / 16000 # 16K が指定されたものとして計算...

		# HTTP 音声認識サーバイベントリスナの作成
		listener = HrpTester(verbose)

		# HTTP 音声認識サーバの初期化
		hrp = com.amivoice.hrp.Hrp.construct(implementation)
		hrp.setListener(listener)
		hrp.setServerURL(serverURL)
		hrp.setProxyServerName(proxyServerName)
		hrp.setConnectTimeout(connectTimeout)
		hrp.setReceiveTimeout(receiveTimeout)
		hrp.setGrammarFileNames(grammarFileNames)
		hrp.setProfileId(profileId)
		hrp.setProfileWords(profileWords)
		hrp.setSegmenterProperties(segmenterProperties)
		hrp.setKeepFillerToken(keepFillerToken)
		hrp.setResultUpdatedInterval(resultUpdatedInterval)
		hrp.setExtension(extension)
		hrp.setAuthorization(authorization)
		hrp.setDomainId(domainId)
		hrp.setCodec(codec)
		hrp.setResultType(resultType)
		hrp.setResultEncoding(resultEncoding)
		hrp.setServiceAuthorization(serviceAuthorization)
		hrp.setVoiceDetection(voiceDetection)

		# HTTP 音声認識サーバへの接続
		if not hrp.connect():
			if not verbose:
				print(hrp.getLastMessage())
			print(u"HTTP 音声認識サーバ %s への接続に失敗しました。" % serverURL)
			return

		try:
			# <!--
			if verbose:
				print("")
				print("======================= " + com.amivoice.hrp.Hrp.getVersion())
			# -->
			for i in range(0, loop):
				for audioFileName in audioFileNames:
					# <!--
					if verbose:
						print("")
						if loop > 1:
							print("----------------------- [%d] %s" % (i + 1, audioFileName))
						else:
							print("----------------------- %s" % audioFileName)
						print("")
					# -->

					# <!-- HTTP リクエストヘッダ群
					if acceptTopic != None:
						hrp.setAcceptTopic(acceptTopic)
					if contentType != None:
						if contentType == "*":
							if audioFileName.endswith(".a08"):
								if audioFileName.find("msb") != -1:
									hrp.setContentType("audio/x-pcm;rate=8000;bit=16;byte-order=big-endian")
								else:
									hrp.setContentType("audio/x-pcm;rate=8000;bit=16;byte-order=little-endian")
							elif audioFileName.endswith(".a16") or audioFileName.endswith(".adc"):
								if audioFileName.find("msb") != -1:
									hrp.setContentType("audio/x-pcm;rate=16000;bit=16;byte-order=big-endian")
								else:
									hrp.setContentType("audio/x-pcm;rate=16000;bit=16;byte-order=little-endian")
							elif audioFileName.endswith(".mu"):
								hrp.setContentType("audio/x-mulaw;rate=8000")
							elif audioFileName.endswith(".alaw"):
								hrp.setContentType("audio/x-alaw;rate=8000")
						else:
							hrp.setContentType(contentType)
					if accept != None:
						hrp.setAccept(accept)
					# -->

					# 音声データファイルの総バイト数の取得
					try:
						audioDataBytes = os.path.getsize(audioFileName)
					except:
						print(u"音声データファイル %s の読み込みに失敗しました。" % audioFileName)
						break

					# HTTP 音声認識サーバへの音声データの送信の開始
					if not hrp.feedDataResume(type, audioDataBytes):
						if not verbose:
							print(hrp.getLastMessage())
						print(u"HTTP 音声認識サーバへの音声データの送信の開始に失敗しました。")
						break

					try:
						with open(audioFileName, "rb") as audioStream:
							# 音声データファイルからの音声データの読み込み
							audioData = audioStream.read(4096)
							while len(audioData) > 0:
								# スリープ時間が計算されているかどうかのチェック
								if sleepTime >= 0:
									# スリープ時間が計算されている場合...
									# 微小時間のスリープ
									hrp.sleep(sleepTime)
								else:
									# スリープ時間が計算されていない場合...
									# 微小時間のスリープ
									hrp.sleep(1)

								# HTTP 音声認識サーバへの音声データの送信
								if not hrp.feedData(audioData, 0, len(audioData)):
									if not verbose:
										print(hrp.getLastMessage())
									print(u"HTTP 音声認識サーバへの音声データの送信に失敗しました。")
									break

								# 音声データファイルからの音声データの読み込み
								audioData = audioStream.read(4096)
					except:
						print(u"音声データファイル %s の読み込みに失敗しました。" % audioFileName)

					# HTTP 音声認識サーバへの音声データの送信の完了
					if not hrp.feedDataPause():
						if not verbose:
							print(hrp.getLastMessage())
						print(u"HTTP 音声認識サーバへの音声データの送信の完了に失敗しました。")
						break
			# <!--
			if verbose:
				print("")
				print("=======================")
				print("")
			# -->
		finally:
			# HTTP 音声認識サーバからの切断
			hrp.disconnect()

		# 終了時間の取得
		endTime = time.time()

		# 経過時間の表示
		print("INFO: elapsed time: %.3f [msec.]" % ((endTime - startTime) * 1000.0))

	def __init__(self, verbose):
		self.verbose_ = verbose

	def resultCreated(self, sessionId):
		print("C %s" % sessionId)

	def resultUpdated(self, result):
		print("U %s" % result)
		text = self.text_(result)
		if text != None:
			print("   -> %s" % text)

	def resultFinalized(self, result):
		print("F %s" % result)
		text = self.text_(result)
		if text != None:
			print("   -> %s" % text)

	def TRACE(self, message):
		if self.verbose_:
			print(message)

	def text_(self, result):
		try:
			return json.loads(result)["text"]
		except:
			return None

HrpTester.main(sys.argv[1:])
