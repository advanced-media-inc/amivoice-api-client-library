using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

#if !UNITY_2018_4_OR_NEWER
[assembly: AssemblyTitle("HrpTester")]
[assembly: AssemblyProduct("HrpTester")]
[assembly: AssemblyCopyright("Copyright (C) 2019-2021 Advanced Media, Inc.")]
[assembly: ComVisible(false)]
[assembly: Guid("c7841911-a965-4734-9222-462422948eed")]
[assembly: AssemblyVersion("1.0.03")]
[assembly: AssemblyFileVersion("1.0.03")]
#endif

namespace HrpTester {

public class HrpTester : com.amivoice.hrp.HrpListener {
	#if UNITY_2018_4_OR_NEWER
	public static PseudoConsole Console;
	#endif

	public static void Main(string[] args) {
		// HTTP 音声認識サーバ URL
		string serverURL = null;
		// プロキシサーバ名
		string proxyServerName = null;
		// 音声データファイル名
		List<string> audioFileNames = new List<string>();
		// グラマファイル名
		string grammarFileNames = null;
		// プロファイル ID
		string profileId = null;
		// プロファイル登録単語
		string profileWords = null;
		// セグメンタプロパティ
		string segmenterProperties = null;
		// フィラー単語を保持するかどうか
		string keepFillerToken = null;
		// 認識中イベント発行間隔
		string resultUpdatedInterval = null;
		// 拡張情報
		string extension = null;
		// サービス認証キー文字列
		string authorization = null;
		// ドメイン ID (d=)
		string domainId = null;
		// 音声データ形式 (c=)
		string codec = null;
		// 認識結果タイプ (r=)
		string resultType = null;
		// 認識結果文字エンコーディング (e=)
		string resultEncoding = null;
		// サービス認証キー文字列 (u=)
		string serviceAuthorization = null;
		// 発話区間検出パラメータ文字列 (v=)
		string voiceDetection = null;
		// HTTP リクエストヘッダ群
		string acceptTopic = null;
		string contentType = null;
		string accept = null;
		// 処理タイプ ("s"／"sc"／"m"／"mc")
		string type = "mc";
		// 接続タイムアウト
		int connectTimeout = 5000;
		// 受信タイムアウト
		int receiveTimeout = 0;
		// 処理ループ (1～)
		int loop = 1;
		// スリープ時間
		int sleepTime = -2;
		// 詳細出力
		bool verbose = false;
		// 実装タイプ
		int implementation = 1;

		// 引数のチェック
		foreach (string arg in args) {
			if (arg.StartsWith("g=")) {
				grammarFileNames = arg.Substring(2);
			} else
			if (arg.StartsWith("i=")) {
				profileId = arg.Substring(2);
			} else
			if (arg.StartsWith("w=")) {
				profileWords = arg.Substring(2);
			} else
			if (arg.StartsWith("op=")) {
				segmenterProperties = arg.Substring(3);
			} else
			if (arg.StartsWith("of=")) {
				keepFillerToken = arg.Substring(3);
			} else
			if (arg.StartsWith("oi=")) {
				resultUpdatedInterval = arg.Substring(3);
			} else
			if (arg.StartsWith("oe=")) {
				extension = arg.Substring(3);
			} else
			if (arg.StartsWith("ou=")) {
				authorization = arg.Substring(3);
			} else
			if (arg.StartsWith("d=")) {
				domainId = arg.Substring(2);
			} else
			if (arg.StartsWith("c=")) {
				codec = arg.Substring(2);
			} else
			if (arg.StartsWith("r=")) {
				resultType = arg.Substring(2);
			} else
			if (arg.StartsWith("e=")) {
				resultEncoding = arg.Substring(2);
			} else
			if (arg.StartsWith("u=")) {
				serviceAuthorization = arg.Substring(2);
			} else
			if (arg.StartsWith("v=")) {
				voiceDetection = arg.Substring(2);
			} else
			if (arg.StartsWith("hd=")) {
				acceptTopic = arg.Substring(3);
			} else
			if (arg.StartsWith("hc=")) {
				contentType = arg.Substring(3);
			} else
			if (arg.StartsWith("hr=")) {
				accept = arg.Substring(3);
			} else
			if (arg.StartsWith("-t")) {
				type = arg.Substring(2);
			} else
			if (arg.StartsWith("-x")) {
				proxyServerName = arg.Substring(2);
			} else
			if (arg.StartsWith("-c")) {
				connectTimeout = int.Parse(arg.Substring(2));
			} else
			if (arg.StartsWith("-r")) {
				receiveTimeout = int.Parse(arg.Substring(2));
			} else
			if (arg.StartsWith("-l")) {
				loop = int.Parse(arg.Substring(2));
			} else
			if (arg.StartsWith("-e")) {
				if (arg.Length > 2) {
					sleepTime = int.Parse(arg.Substring(2));
				} else {
					sleepTime = -1;
				}
			} else
			if (arg.StartsWith("-v")) {
				verbose = true;
			} else
			if (arg.StartsWith("-2")) {
				implementation = 2;
			} else
			if (serverURL == null) {
				serverURL = arg;
			} else {
				audioFileNames.Add(arg);
			}
			if (verbose) {
				Console.WriteLine("DEBUG: " + arg);
			}
		}
		if (audioFileNames.Count == 0) {
			Console.WriteLine("Usage: HrpTester [<parameters/options>]");
			Console.WriteLine("                  <url>");
			Console.WriteLine("                   <audioFileName>...");
			Console.WriteLine("Parameters:");
			Console.WriteLine("  g=<grammarFileNames>");
			Console.WriteLine("  i=<profileId>");
			Console.WriteLine("  w=<profileWords>");
			Console.WriteLine("  op=<segmenterProperties>");
			Console.WriteLine("  of=<keepFillerToken>");
			Console.WriteLine("  oi=<resultUpdatedInterval>");
			Console.WriteLine("  oe=<extension>");
			Console.WriteLine("  ou=<authorization>");
			Console.WriteLine("  d=<domainId>");
			Console.WriteLine("  c=<codec>");
			Console.WriteLine("  r=<resultType>");
			Console.WriteLine("  e=<resultEncoding>");
			Console.WriteLine("  u=<serviceAuthorization>");
			Console.WriteLine("  v=<voiceDetection>");
			Console.WriteLine("  hd=<acceptTopic>            HTTP 'Accept-Topic' header instead d=<domainId>");
			Console.WriteLine("  hc=<contentType>            HTTP 'Content-Type' header instead c=<codec>");
			Console.WriteLine("  hr=<accept>                 HTTP 'Accept' header instead r=<resultType>");
			Console.WriteLine("Options:");
			Console.WriteLine("  -t<type>                    (default: -tmc)");
			Console.WriteLine("      <type>='s'              Single part & no chunked encoding");
			Console.WriteLine("      <type>='sc'             Single part & chunked encoding");
			Console.WriteLine("      <type>='m'              Multi parts & no chunked encoding");
			Console.WriteLine("      <type>='mc'             Multi parts & chunked encoding");
			Console.WriteLine("  -x<proxyServerName>         (default: -x)");
			Console.WriteLine("  -c<connectionTimeout>       (default: -c5000)");
			Console.WriteLine("  -r<receiveTimeout>          (default: -r0)");
			Console.WriteLine("  -l<loop>                    (default: -l1)");
			Console.WriteLine("  -e                          realtime simulation (default: -)");
			Console.WriteLine("  -v                          verbose output (default: -)");
			return;
		}

		// 開始時間の取得
		long startTime = DateTime.Now.Ticks * 100;

		// スリープ時間の計算
		if (sleepTime == -1) {
			if (codec != null) {
				if (codec.Equals("22K", StringComparison.OrdinalIgnoreCase) || codec.Equals("MSB22K", StringComparison.OrdinalIgnoreCase) || codec.Equals("LSB22K", StringComparison.OrdinalIgnoreCase)) {
					sleepTime = 4096 * 1000 / 2 / 22050;
				} else
				if (codec.Equals("16K", StringComparison.OrdinalIgnoreCase) || codec.Equals("MSB16K", StringComparison.OrdinalIgnoreCase) || codec.Equals("LSB16K", StringComparison.OrdinalIgnoreCase)) {
					sleepTime = 4096 * 1000 / 2 / 16000;
				} else
				if (codec.Equals("11K", StringComparison.OrdinalIgnoreCase) || codec.Equals("MSB11K", StringComparison.OrdinalIgnoreCase) || codec.Equals("LSB11K", StringComparison.OrdinalIgnoreCase)) {
					sleepTime = 4096 * 1000 / 2 / 11025;
				} else
				if (codec.Equals("8K", StringComparison.OrdinalIgnoreCase) || codec.Equals("MSB8K", StringComparison.OrdinalIgnoreCase) || codec.Equals("LSB8K", StringComparison.OrdinalIgnoreCase)) {
					sleepTime = 4096 * 1000 / 2 / 8000;
				} else
				if (codec.Equals("MULAW", StringComparison.OrdinalIgnoreCase) || codec.Equals("ALAW", StringComparison.OrdinalIgnoreCase)) {
					sleepTime = 4096 * 1000 / 1 / 8000;
				} else {
					sleepTime = 4096 * 1000 / 2 / 16000; // 16K が指定されたものとして計算...
				}
			} else {
				sleepTime = 4096 * 1000 / 2 / 16000; // 16K が指定されたものとして計算...
			}
		}

		// HTTP 音声認識サーバイベントリスナの作成
		com.amivoice.hrp.HrpListener listener = new HrpTester(verbose);

		// HTTP 音声認識サーバの初期化
		com.amivoice.hrp.Hrp hrp = com.amivoice.hrp.Hrp.construct(implementation);
		hrp.setListener(listener);
		hrp.setServerURL(serverURL);
		hrp.setProxyServerName(proxyServerName);
		hrp.setConnectTimeout(connectTimeout);
		hrp.setReceiveTimeout(receiveTimeout);
		hrp.setGrammarFileNames(grammarFileNames);
		hrp.setProfileId(profileId);
		hrp.setProfileWords(profileWords);
		hrp.setSegmenterProperties(segmenterProperties);
		hrp.setKeepFillerToken(keepFillerToken);
		hrp.setResultUpdatedInterval(resultUpdatedInterval);
		hrp.setExtension(extension);
		hrp.setAuthorization(authorization);
		hrp.setDomainId(domainId);
		hrp.setCodec(codec);
		hrp.setResultType(resultType);
		hrp.setResultEncoding(resultEncoding);
		hrp.setServiceAuthorization(serviceAuthorization);
		hrp.setVoiceDetection(voiceDetection);

		// HTTP 音声認識サーバへの接続
		if (!hrp.connect()) {
			if (!verbose) {
				Console.WriteLine(hrp.getLastMessage());
			}
			Console.WriteLine("HTTP 音声認識サーバ " + serverURL + " への接続に失敗しました。");
			return;
		}

		try {
			// <!--
			if (verbose) {
				Console.WriteLine();
				Console.WriteLine("======================= " + com.amivoice.hrp.Hrp.getVersion());
			}
			// -->
			for (int i = 0; i < loop; i++) {
				foreach (string audioFileName in audioFileNames) {
					// <!--
					if (verbose) {
						Console.WriteLine();
						if (loop > 1) {
							Console.WriteLine("----------------------- [" + (i + 1) + "] " + audioFileName);
						} else {
							Console.WriteLine("----------------------- " + audioFileName);
						}
						Console.WriteLine();
					}
					// -->

					// <!-- HTTP リクエストヘッダ群
					if (acceptTopic != null) {
						hrp.setAcceptTopic(acceptTopic);
					}
					if (contentType != null) {
						if (contentType.Equals("*")) {
							if (audioFileName.EndsWith(".a08")) {
								if (audioFileName.IndexOf("msb") != -1) {
									hrp.setContentType("audio/x-pcm;rate=8000;bit=16;byte-order=big-endian");
								} else {
									hrp.setContentType("audio/x-pcm;rate=8000;bit=16;byte-order=little-endian");
								}
							} else
							if (audioFileName.EndsWith(".a16") || audioFileName.EndsWith(".adc")) {
								if (audioFileName.IndexOf("msb") != -1) {
									hrp.setContentType("audio/x-pcm;rate=16000;bit=16;byte-order=big-endian");
								} else {
									hrp.setContentType("audio/x-pcm;rate=16000;bit=16;byte-order=little-endian");
								}
							} else
							if (audioFileName.EndsWith(".mu")) {
								hrp.setContentType("audio/x-mulaw;rate=8000");
							} else
							if (audioFileName.EndsWith(".alaw")) {
								hrp.setContentType("audio/x-alaw;rate=8000");
							}
						} else {
							hrp.setContentType(contentType);
						}
					}
					if (accept != null) {
						hrp.setAccept(accept);
					}
					// -->

					// 音声データファイルの総バイト数の取得
					long audioDataBytes = new FileInfo(audioFileName).Length;
					if (audioDataBytes == 0) {
						Console.WriteLine("音声データファイル " + audioFileName + " の読み込みに失敗しました。");
						break;
					}

					// HTTP 音声認識サーバへの音声データの送信の開始
					if (!hrp.feedDataResume(type, audioDataBytes)) {
						if (!verbose) {
							Console.WriteLine(hrp.getLastMessage());
						}
						Console.WriteLine("HTTP 音声認識サーバへの音声データの送信の開始に失敗しました。");
						break;
					}

					try {
						using (FileStream audioStream = new FileStream(audioFileName, FileMode.Open, FileAccess.Read)) {
							// 音声データファイルからの音声データの読み込み
							byte[] audioData = new byte[4096];
							int audioDataReadBytes = audioStream.Read(audioData, 0, audioData.Length);
							while (audioDataReadBytes > 0) {
								// スリープ時間が計算されているかどうかのチェック
								if (sleepTime >= 0) {
									// スリープ時間が計算されている場合...
									// 微小時間のスリープ
									hrp.sleep(sleepTime);
								} else {
									// スリープ時間が計算されていない場合...
									// 微小時間のスリープ
									hrp.sleep(1);
								}

								// HTTP 音声認識サーバへの音声データの送信
								if (!hrp.feedData(audioData, 0, audioDataReadBytes)) {
									if (!verbose) {
										Console.WriteLine(hrp.getLastMessage());
									}
									Console.WriteLine("HTTP 音声認識サーバへの音声データの送信に失敗しました。");
									break;
								}

								// 音声データファイルからの音声データの読み込み
								audioDataReadBytes = audioStream.Read(audioData, 0, audioData.Length);
							}
						}
					} catch (IOException) {
						Console.WriteLine("音声データファイル " + audioFileName + " の読み込みに失敗しました。");
					}

					// HTTP 音声認識サーバへの音声データの送信の完了
					if (!hrp.feedDataPause()) {
						if (!verbose) {
							Console.WriteLine(hrp.getLastMessage());
						}
						Console.WriteLine("HTTP 音声認識サーバへの音声データの送信の完了に失敗しました。");
						break;
					}
				}
			}
			// <!--
			if (verbose) {
				Console.WriteLine();
				Console.WriteLine("=======================");
				Console.WriteLine();
			}
			// -->
		} finally {
			// HTTP 音声認識サーバからの切断
			hrp.disconnect();
		}

		// 終了時間の取得
		long endTime = DateTime.Now.Ticks * 100;

		// 経過時間の表示
		Console.WriteLine("INFO: elapsed time: " + ((float)(endTime - startTime) / 1000000.0f) + " [msec.]");
	}

	private bool verbose_;

	public HrpTester(bool verbose) {
		verbose_ = verbose;
	}

	public void resultCreated(string sessionId) {
		Console.WriteLine("C " + sessionId);
	}

	public void resultUpdated(string result) {
		Console.WriteLine("U " + result);
		string text = text_(result);
		if (text != null) {
			Console.WriteLine("   -> " + text);
		}
	}

	public void resultFinalized(string result) {
		Console.WriteLine("F " + result);
		string text = text_(result);
		if (text != null) {
			Console.WriteLine("   -> " + text);
		}
	}

	public void TRACE(string message) {
		if (verbose_) {
			Console.WriteLine(message);
		}
	}

	private string text_(string result) {
		int index = result.LastIndexOf(",\"text\":\"");
		if (index == -1) {
			return null;
		}
		index += 9;
		int resultLength = result.Length;
		StringBuilder buffer = new StringBuilder();
		int c = (index >= resultLength) ? 0 : result[index++];
		while (c != 0) {
			if (c == '"') {
				break;
			}
			if (c == '\\') {
				c = (index >= resultLength) ? 0 : result[index++];
				if (c == 0) {
					return null;
				}
				if (c == '"' || c == '\\' || c == '/') {
					buffer.Append((char)c);
				} else
				if (c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't') {
				} else
				if (c == 'u') {
					int c0 = (index >= resultLength) ? 0 : result[index++];
					int c1 = (index >= resultLength) ? 0 : result[index++];
					int c2 = (index >= resultLength) ? 0 : result[index++];
					int c3 = (index >= resultLength) ? 0 : result[index++];
					if (c0 >= '0' && c0 <= '9') {c0 -= '0';} else if (c0 >= 'A' && c0 <= 'F') {c0 -= 'A' - 10;} else if (c0 >= 'a' && c0 <= 'f') {c0 -= 'a' - 10;} else {c0 = -1;}
					if (c1 >= '0' && c1 <= '9') {c1 -= '0';} else if (c1 >= 'A' && c1 <= 'F') {c1 -= 'A' - 10;} else if (c1 >= 'a' && c1 <= 'f') {c1 -= 'a' - 10;} else {c1 = -1;}
					if (c2 >= '0' && c2 <= '9') {c2 -= '0';} else if (c2 >= 'A' && c2 <= 'F') {c2 -= 'A' - 10;} else if (c2 >= 'a' && c2 <= 'f') {c2 -= 'a' - 10;} else {c2 = -1;}
					if (c3 >= '0' && c3 <= '9') {c3 -= '0';} else if (c3 >= 'A' && c3 <= 'F') {c3 -= 'A' - 10;} else if (c3 >= 'a' && c3 <= 'f') {c3 -= 'a' - 10;} else {c3 = -1;}
					if (c0 == -1 || c1 == -1 || c2 == -1 || c3 == -1) {
						return null;
					}
					buffer.Append((char)((c0 << 12) | (c1 << 8) | (c2 << 4) | c3));
				} else {
					return null;
				}
			} else {
				buffer.Append((char)c);
			}
			c = (index >= resultLength) ? 0 : result[index++];
		}
		return buffer.ToString();
	}
}

} // namespace
