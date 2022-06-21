using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

#if !UNITY_2018_4_OR_NEWER
[assembly: AssemblyTitle("HrpSimpleTester")]
[assembly: AssemblyProduct("HrpSimpleTester")]
[assembly: AssemblyCopyright("Copyright (C) 2019-2021 Advanced Media, Inc.")]
[assembly: ComVisible(false)]
[assembly: Guid("c7841911-a965-4734-9222-462422948eeb")]
[assembly: AssemblyVersion("1.0.03")]
[assembly: AssemblyFileVersion("1.0.03")]
#endif

namespace HrpSimpleTester {

public class HrpSimpleTester : com.amivoice.hrp.HrpListener {
	#if UNITY_2018_4_OR_NEWER
	public static PseudoConsole Console;
	#endif

	public static void Main(string[] args) {
		if (args.Length < 4) {
			Console.WriteLine("Usage: HrpSimpleTester <url> <audioFileName> <codec> <grammarFileNames> [<authorization>]");
			return;
		}

		string serverURL = args[0];
		string audioFileName = args[1];
		string codec = args[2];
		string grammarFileNames = args[3];
		string authorization = (args.Length > 4) ? args[4] : null;

		// HTTP 音声認識サーバイベントリスナの作成
		com.amivoice.hrp.HrpListener listener = new HrpSimpleTester();

		// HTTP 音声認識サーバの初期化
		com.amivoice.hrp.Hrp hrp = com.amivoice.hrp.Hrp.construct();
		hrp.setListener(listener);
		hrp.setServerURL(serverURL);
		hrp.setCodec(codec);
		hrp.setGrammarFileNames(grammarFileNames);
		hrp.setAuthorization(authorization);

		// HTTP 音声認識サーバへの接続
		if (!hrp.connect()) {
			Console.WriteLine(hrp.getLastMessage());
			Console.WriteLine("HTTP 音声認識サーバ " + serverURL + " への接続に失敗しました。");
			return;
		}

		try {
			// HTTP 音声認識サーバへの音声データの送信の開始
			if (!hrp.feedDataResume()) {
				Console.WriteLine(hrp.getLastMessage());
				Console.WriteLine("HTTP 音声認識サーバへの音声データの送信の開始に失敗しました。");
				return;
			}

			try {
				using (FileStream audioStream = new FileStream(audioFileName, FileMode.Open, FileAccess.Read)) {
					// 音声データファイルからの音声データの読み込み
					byte[] audioData = new byte[4096];
					int audioDataReadBytes = audioStream.Read(audioData, 0, audioData.Length);
					while (audioDataReadBytes > 0) {
						// 微小時間のスリープ
						hrp.sleep(1);

						// HTTP 音声認識サーバへの音声データの送信
						if (!hrp.feedData(audioData, 0, audioDataReadBytes)) {
							Console.WriteLine(hrp.getLastMessage());
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
				Console.WriteLine(hrp.getLastMessage());
				Console.WriteLine("HTTP 音声認識サーバへの音声データの送信の完了に失敗しました。");
				return;
			}
		} finally {
			// HTTP 音声認識サーバからの切断
			hrp.disconnect();
		}
	}

	public HrpSimpleTester() {
	}

	public void resultCreated(string sessionId) {
//		Console.WriteLine("C " + sessionId);
	}

	public void resultUpdated(string result) {
//		Console.WriteLine("U " + result);
	}

	public void resultFinalized(string result) {
//		Console.WriteLine("F " + result);
		Console.WriteLine(result);
		string text = text_(result);
		if (text != null) {
			Console.WriteLine(" -> " + text);
		}
	}

	public void TRACE(string message) {
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
