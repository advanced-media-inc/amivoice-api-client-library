using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

#if !UNITY_2018_4_OR_NEWER
[assembly: AssemblyTitle("WrpTester")]
[assembly: AssemblyProduct("WrpTester")]
[assembly: AssemblyCopyright("Copyright (C) 2019-2021 Advanced Media, Inc.")]
[assembly: ComVisible(false)]
[assembly: Guid("c7841911-a965-4734-9222-462422948eec")]
[assembly: AssemblyVersion("1.0.03")]
[assembly: AssemblyFileVersion("1.0.03")]
#endif

namespace WrpTester {

public class WrpTester : com.amivoice.wrp.WrpListener {
	#if UNITY_2018_4_OR_NEWER
	public static PseudoConsole Console;
	#endif

	public static void Main(string[] args) {
		// WebSocket 音声認識サーバ URL
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
		// 音声データ形式
		string codec = null;
		// 認識結果タイプ
		string resultType = null;
		// サービス認証キー文字列
		string serviceAuthorization = null;
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
			if (arg.StartsWith("c=")) {
				codec = arg.Substring(2);
			} else
			if (arg.StartsWith("r=")) {
				resultType = arg.Substring(2);
			} else
			if (arg.StartsWith("u=")) {
				serviceAuthorization = arg.Substring(2);
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
			if (arg.StartsWith("-3")) {
				implementation = 3;
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
			Console.WriteLine("Usage: WrpTester [<parameters/options>]");
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
			Console.WriteLine("  c=<codec>");
			Console.WriteLine("  r=<resultType>");
			Console.WriteLine("  u=<serviceAuthorization>");
			Console.WriteLine("Options:");
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

		// WebSocket 音声認識サーバイベントリスナの作成
		com.amivoice.wrp.WrpListener listener = new WrpTester(verbose);

		// WebSocket 音声認識サーバの初期化
		com.amivoice.wrp.Wrp wrp = com.amivoice.wrp.Wrp.construct(implementation);
		wrp.setListener(listener);
		wrp.setServerURL(serverURL);
		wrp.setProxyServerName(proxyServerName);
		wrp.setConnectTimeout(connectTimeout);
		wrp.setReceiveTimeout(receiveTimeout);
		wrp.setGrammarFileNames(grammarFileNames);
		wrp.setProfileId(profileId);
		wrp.setProfileWords(profileWords);
		wrp.setSegmenterProperties(segmenterProperties);
		wrp.setKeepFillerToken(keepFillerToken);
		wrp.setResultUpdatedInterval(resultUpdatedInterval);
		wrp.setExtension(extension);
		wrp.setAuthorization(authorization);
		wrp.setCodec(codec);
		wrp.setResultType(resultType);
		wrp.setServiceAuthorization(serviceAuthorization);

		// WebSocket 音声認識サーバへの接続
		if (!wrp.connect()) {
			if (!verbose) {
				Console.WriteLine(wrp.getLastMessage());
			}
			Console.WriteLine("WebSocket 音声認識サーバ " + serverURL + " への接続に失敗しました。");
			return;
		}

		try {
			// <!--
			if (verbose) {
				Console.WriteLine();
				Console.WriteLine("======================= " + com.amivoice.wrp.Wrp.getVersion());
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

					// WebSocket 音声認識サーバへの音声データの送信の開始
					if (!wrp.feedDataResume()) {
						if (!verbose) {
							Console.WriteLine(wrp.getLastMessage());
						}
						Console.WriteLine("WebSocket 音声認識サーバへの音声データの送信の開始に失敗しました。");
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
									wrp.sleep(sleepTime);
								} else {
									// スリープ時間が計算されていない場合...
									// 微小時間のスリープ
									wrp.sleep(1);

									// 認識結果情報待機数が 1 以下になるまでスリープ
									int maxSleepTime = 50000;
									while (wrp.getWaitingResults() > 1 && maxSleepTime > 0) {
										wrp.sleep(100);
										maxSleepTime -= 100;
									}
								}

								// WebSocket 音声認識サーバへの音声データの送信
								if (!wrp.feedData(audioData, 0, audioDataReadBytes)) {
									if (!verbose) {
										Console.WriteLine(wrp.getLastMessage());
									}
									Console.WriteLine("WebSocket 音声認識サーバへの音声データの送信に失敗しました。");
									break;
								}

								// 音声データファイルからの音声データの読み込み
								audioDataReadBytes = audioStream.Read(audioData, 0, audioData.Length);
							}
						}
					} catch (IOException) {
						Console.WriteLine("音声データファイル " + audioFileName + " の読み込みに失敗しました。");
					}

					// WebSocket 音声認識サーバへの音声データの送信の完了
					if (!wrp.feedDataPause()) {
						if (!verbose) {
							Console.WriteLine(wrp.getLastMessage());
						}
						Console.WriteLine("WebSocket 音声認識サーバへの音声データの送信の完了に失敗しました。");
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
			// WebSocket 音声認識サーバからの切断
			wrp.disconnect();
		}

		// 終了時間の取得
		long endTime = DateTime.Now.Ticks * 100;

		// 経過時間の表示
		Console.WriteLine("INFO: elapsed time: " + ((float)(endTime - startTime) / 1000000.0f) + " [msec.]");
	}

	private bool verbose_;

	public WrpTester(bool verbose) {
		verbose_ = verbose;
	}

	public void utteranceStarted(int startTime) {
		Console.WriteLine("S " + startTime);
	}

	public void utteranceEnded(int endTime) {
		Console.WriteLine("E " + endTime);
	}

	public void resultCreated() {
		Console.WriteLine("C");
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

	public void eventNotified(int eventId, string eventMessage) {
		Console.WriteLine((char)eventId + " " + eventMessage);
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
