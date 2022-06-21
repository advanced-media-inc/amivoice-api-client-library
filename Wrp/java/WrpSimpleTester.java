import java.io.*;

public class WrpSimpleTester implements com.amivoice.wrp.WrpListener {
	public static void main(String[] args) {
		if (args.length < 4) {
			System.out.println("Usage: java WrpSimpleTester <url> <audioFileName> <codec> <grammarFileNames> [<authorization>]");
			return;
		}

		String serverURL = args[0];
		String audioFileName = args[1];
		String codec = args[2];
		String grammarFileNames = args[3];
		String authorization = (args.length > 4) ? args[4] : null;

		// WebSocket 音声認識サーバイベントリスナの作成
		com.amivoice.wrp.WrpListener listener = new WrpSimpleTester();

		// WebSocket 音声認識サーバの初期化
		com.amivoice.wrp.Wrp wrp = com.amivoice.wrp.Wrp.construct();
		wrp.setListener(listener);
		wrp.setServerURL(serverURL);
		wrp.setCodec(codec);
		wrp.setGrammarFileNames(grammarFileNames);
		wrp.setAuthorization(authorization);

		// WebSocket 音声認識サーバへの接続
		if (!wrp.connect()) {
			System.out.println(wrp.getLastMessage());
			System.out.println("WebSocket 音声認識サーバ " + serverURL + " への接続に失敗しました。");
			return;
		}

		try {
			// WebSocket 音声認識サーバへの音声データの送信の開始
			if (!wrp.feedDataResume()) {
				System.out.println(wrp.getLastMessage());
				System.out.println("WebSocket 音声認識サーバへの音声データの送信の開始に失敗しました。");
				return;
			}

			try (FileInputStream audioStream = new FileInputStream(audioFileName)) {
				// 音声データファイルからの音声データの読み込み
				byte[] audioData = new byte[4096];
				int audioDataReadBytes = audioStream.read(audioData, 0, audioData.length);
				while (audioDataReadBytes > 0) {
					// 微小時間のスリープ
					wrp.sleep(1);

					// 認識結果情報待機数が 1 以下になるまでスリープ
					int maxSleepTime = 50000;
					while (wrp.getWaitingResults() > 1 && maxSleepTime > 0) {
						wrp.sleep(100);
						maxSleepTime -= 100;
					}

					// WebSocket 音声認識サーバへの音声データの送信
					if (!wrp.feedData(audioData, 0, audioDataReadBytes)) {
						System.out.println(wrp.getLastMessage());
						System.out.println("WebSocket 音声認識サーバへの音声データの送信に失敗しました。");
						break;
					}

					// 音声データファイルからの音声データの読み込み
					audioDataReadBytes = audioStream.read(audioData, 0, audioData.length);
				}
			} catch (IOException e) {
				System.out.println("音声データファイル " + audioFileName + " の読み込みに失敗しました。");
			}

			// WebSocket 音声認識サーバへの音声データの送信の完了
			if (!wrp.feedDataPause()) {
				System.out.println(wrp.getLastMessage());
				System.out.println("WebSocket 音声認識サーバへの音声データの送信の完了に失敗しました。");
				return;
			}
		} finally {
			// WebSocket 音声認識サーバからの切断
			wrp.disconnect();
		}
	}

	public WrpSimpleTester() {
	}

	@Override
	public void utteranceStarted(int startTime) {
//		System.out.println("S " + startTime);
	}

	@Override
	public void utteranceEnded(int endTime) {
//		System.out.println("E " + endTime);
	}

	@Override
	public void resultCreated() {
//		System.out.println("C");
	}

	@Override
	public void resultUpdated(String result) {
//		System.out.println("U " + result);
	}

	@Override
	public void resultFinalized(String result) {
//		System.out.println("F " + result);
		System.out.println(result);
		String text = text_(result);
		if (text != null) {
			System.out.println(" -> " + text);
		}
	}

	@Override
	public void eventNotified(int eventId, String eventMessage) {
//		System.out.println((char)eventId + " " + eventMessage);
	}

	@Override
	public void TRACE(String message) {
	}

	private String text_(String result) {
		int index = result.lastIndexOf(",\"text\":\"");
		if (index == -1) {
			return null;
		}
		index += 9;
		int resultLength = result.length();
		StringBuilder buffer = new StringBuilder();
		int c = (index >= resultLength) ? 0 : result.charAt(index++);
		while (c != 0) {
			if (c == '"') {
				break;
			}
			if (c == '\\') {
				c = (index >= resultLength) ? 0 : result.charAt(index++);
				if (c == 0) {
					return null;
				}
				if (c == '"' || c == '\\' || c == '/') {
					buffer.append((char)c);
				} else
				if (c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't') {
				} else
				if (c == 'u') {
					int c0 = (index >= resultLength) ? 0 : result.charAt(index++);
					int c1 = (index >= resultLength) ? 0 : result.charAt(index++);
					int c2 = (index >= resultLength) ? 0 : result.charAt(index++);
					int c3 = (index >= resultLength) ? 0 : result.charAt(index++);
					if (c0 >= '0' && c0 <= '9') {c0 -= '0';} else if (c0 >= 'A' && c0 <= 'F') {c0 -= 'A' - 10;} else if (c0 >= 'a' && c0 <= 'f') {c0 -= 'a' - 10;} else {c0 = -1;}
					if (c1 >= '0' && c1 <= '9') {c1 -= '0';} else if (c1 >= 'A' && c1 <= 'F') {c1 -= 'A' - 10;} else if (c1 >= 'a' && c1 <= 'f') {c1 -= 'a' - 10;} else {c1 = -1;}
					if (c2 >= '0' && c2 <= '9') {c2 -= '0';} else if (c2 >= 'A' && c2 <= 'F') {c2 -= 'A' - 10;} else if (c2 >= 'a' && c2 <= 'f') {c2 -= 'a' - 10;} else {c2 = -1;}
					if (c3 >= '0' && c3 <= '9') {c3 -= '0';} else if (c3 >= 'A' && c3 <= 'F') {c3 -= 'A' - 10;} else if (c3 >= 'a' && c3 <= 'f') {c3 -= 'a' - 10;} else {c3 = -1;}
					if (c0 == -1 || c1 == -1 || c2 == -1 || c3 == -1) {
						return null;
					}
					buffer.append((char)((c0 << 12) | (c1 << 8) | (c2 << 4) | c3));
				} else {
					return null;
				}
			} else {
				buffer.append((char)c);
			}
			c = (index >= resultLength) ? 0 : result.charAt(index++);
		}
		return buffer.toString();
	}
}
