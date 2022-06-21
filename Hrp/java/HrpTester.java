import java.io.*;
import java.util.ArrayList;

public class HrpTester implements com.amivoice.hrp.HrpListener {
	public static void main(String[] args) {
		// HTTP 音声認識サーバ URL
		String serverURL = null;
		// プロキシサーバ名
		String proxyServerName = null;
		// 音声データファイル名
		ArrayList<String> audioFileNames = new ArrayList<String>();
		// グラマファイル名
		String grammarFileNames = null;
		// プロファイル ID
		String profileId = null;
		// プロファイル登録単語
		String profileWords = null;
		// セグメンタプロパティ
		String segmenterProperties = null;
		// フィラー単語を保持するかどうか
		String keepFillerToken = null;
		// 認識中イベント発行間隔
		String resultUpdatedInterval = null;
		// 拡張情報
		String extension = null;
		// サービス認証キー文字列
		String authorization = null;
		// ドメイン ID (d=)
		String domainId = null;
		// 音声データ形式 (c=)
		String codec = null;
		// 認識結果タイプ (r=)
		String resultType = null;
		// 認識結果文字エンコーディング (e=)
		String resultEncoding = null;
		// サービス認証キー文字列 (u=)
		String serviceAuthorization = null;
		// 発話区間検出パラメータ文字列 (v=)
		String voiceDetection = null;
		// HTTP リクエストヘッダ群
		String acceptTopic = null;
		String contentType = null;
		String accept = null;
		// 処理タイプ ("s"／"sc"／"m"／"mc")
		String type = "mc";
		// 接続タイムアウト
		int connectTimeout = 5000;
		// 受信タイムアウト
		int receiveTimeout = 0;
		// 処理ループ (1～)
		int loop = 1;
		// スリープ時間
		int sleepTime = -2;
		// 詳細出力
		boolean verbose = false;
		// 実装タイプ
		int implementation = 1;

		// 引数のチェック
		for (String arg: args) {
			if (arg.startsWith("g=")) {
				grammarFileNames = arg.substring(2);
			} else
			if (arg.startsWith("i=")) {
				profileId = arg.substring(2);
			} else
			if (arg.startsWith("w=")) {
				profileWords = arg.substring(2);
			} else
			if (arg.startsWith("op=")) {
				segmenterProperties = arg.substring(3);
			} else
			if (arg.startsWith("of=")) {
				keepFillerToken = arg.substring(3);
			} else
			if (arg.startsWith("oi=")) {
				resultUpdatedInterval = arg.substring(3);
			} else
			if (arg.startsWith("oe=")) {
				extension = arg.substring(3);
			} else
			if (arg.startsWith("ou=")) {
				authorization = arg.substring(3);
			} else
			if (arg.startsWith("d=")) {
				domainId = arg.substring(2);
			} else
			if (arg.startsWith("c=")) {
				codec = arg.substring(2);
			} else
			if (arg.startsWith("r=")) {
				resultType = arg.substring(2);
			} else
			if (arg.startsWith("e=")) {
				resultEncoding = arg.substring(2);
			} else
			if (arg.startsWith("u=")) {
				serviceAuthorization = arg.substring(2);
			} else
			if (arg.startsWith("v=")) {
				voiceDetection = arg.substring(2);
			} else
			if (arg.startsWith("hd=")) {
				acceptTopic = arg.substring(3);
			} else
			if (arg.startsWith("hc=")) {
				contentType = arg.substring(3);
			} else
			if (arg.startsWith("hr=")) {
				accept = arg.substring(3);
			} else
			if (arg.startsWith("-t")) {
				type = arg.substring(2);
			} else
			if (arg.startsWith("-x")) {
				proxyServerName = arg.substring(2);
			} else
			if (arg.startsWith("-c")) {
				connectTimeout = Integer.parseInt(arg.substring(2));
			} else
			if (arg.startsWith("-r")) {
				receiveTimeout = Integer.parseInt(arg.substring(2));
			} else
			if (arg.startsWith("-l")) {
				loop = Integer.parseInt(arg.substring(2));
			} else
			if (arg.startsWith("-e")) {
				if (arg.length() > 2) {
					sleepTime = Integer.parseInt(arg.substring(2));
				} else {
					sleepTime = -1;
				}
			} else
			if (arg.startsWith("-v")) {
				verbose = true;
			} else
			if (arg.startsWith("-2")) {
				implementation = 2;
			} else
			if (serverURL == null) {
				serverURL = arg;
			} else {
				audioFileNames.add(arg);
			}
			if (verbose) {
				System.out.println("DEBUG: " + arg);
			}
		}
		if (audioFileNames.size() == 0) {
			System.out.println("Usage: java HrpTester [<parameters/options>]");
			System.out.println("                       <url>");
			System.out.println("                        <audioFileName>...");
			System.out.println("Parameters:");
			System.out.println("  g=<grammarFileNames>");
			System.out.println("  i=<profileId>");
			System.out.println("  w=<profileWords>");
			System.out.println("  op=<segmenterProperties>");
			System.out.println("  of=<keepFillerToken>");
			System.out.println("  oi=<resultUpdatedInterval>");
			System.out.println("  oe=<extension>");
			System.out.println("  ou=<authorization>");
			System.out.println("  d=<domainId>");
			System.out.println("  c=<codec>");
			System.out.println("  r=<resultType>");
			System.out.println("  e=<resultEncoding>");
			System.out.println("  u=<serviceAuthorization>");
			System.out.println("  v=<voiceDetection>");
			System.out.println("  hd=<acceptTopic>            HTTP 'Accept-Topic' header instead d=<domainId>");
			System.out.println("  hc=<contentType>            HTTP 'Content-Type' header instead c=<codec>");
			System.out.println("  hr=<accept>                 HTTP 'Accept' header instead r=<resultType>");
			System.out.println("Options:");
			System.out.println("  -t<type>                    (default: -tmc)");
			System.out.println("      <type>='s'              Single part & no chunked encoding");
			System.out.println("      <type>='sc'             Single part & chunked encoding");
			System.out.println("      <type>='m'              Multi parts & no chunked encoding");
			System.out.println("      <type>='mc'             Multi parts & chunked encoding");
			System.out.println("  -x<proxyServerName>         (default: -x)");
			System.out.println("  -c<connectionTimeout>       (default: -c5000)");
			System.out.println("  -r<receiveTimeout>          (default: -r0)");
			System.out.println("  -l<loop>                    (default: -l1)");
			System.out.println("  -e                          realtime simulation (default: -)");
			System.out.println("  -v                          verbose output (default: -)");
			return;
		}

		// 開始時間の取得
		long startTime = System.nanoTime();

		// スリープ時間の計算
		if (sleepTime == -1) {
			if (codec != null) {
				if (codec.equalsIgnoreCase("22K") || codec.equalsIgnoreCase("MSB22K") || codec.equalsIgnoreCase("LSB22K")) {
					sleepTime = 4096 * 1000 / 2 / 22050;
				} else
				if (codec.equalsIgnoreCase("16K") || codec.equalsIgnoreCase("MSB16K") || codec.equalsIgnoreCase("LSB16K")) {
					sleepTime = 4096 * 1000 / 2 / 16000;
				} else
				if (codec.equalsIgnoreCase("11K") || codec.equalsIgnoreCase("MSB11K") || codec.equalsIgnoreCase("LSB11K")) {
					sleepTime = 4096 * 1000 / 2 / 11025;
				} else
				if (codec.equalsIgnoreCase("8K") || codec.equalsIgnoreCase("MSB8K") || codec.equalsIgnoreCase("LSB8K")) {
					sleepTime = 4096 * 1000 / 2 / 8000;
				} else
				if (codec.equalsIgnoreCase("MULAW") || codec.equalsIgnoreCase("ALAW")) {
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
				System.out.println(hrp.getLastMessage());
			}
			System.out.println("HTTP 音声認識サーバ " + serverURL + " への接続に失敗しました。");
			return;
		}

		try {
			// <!--
			if (verbose) {
				System.out.println();
				System.out.println("======================= " + com.amivoice.hrp.Hrp.getVersion());
			}
			// -->
			for (int i = 0; i < loop; i++) {
				for (String audioFileName: audioFileNames) {
					// <!--
					if (verbose) {
						System.out.println();
						if (loop > 1) {
							System.out.println("----------------------- [" + (i + 1) + "] " + audioFileName);
						} else {
							System.out.println("----------------------- " + audioFileName);
						}
						System.out.println();
					}
					// -->

					// <!-- HTTP リクエストヘッダ群
					if (acceptTopic != null) {
						hrp.setAcceptTopic(acceptTopic);
					}
					if (contentType != null) {
						if (contentType.equals("*")) {
							if (audioFileName.endsWith(".a08")) {
								if (audioFileName.indexOf("msb") != -1) {
									hrp.setContentType("audio/x-pcm;rate=8000;bit=16;byte-order=big-endian");
								} else {
									hrp.setContentType("audio/x-pcm;rate=8000;bit=16;byte-order=little-endian");
								}
							} else
							if (audioFileName.endsWith(".a16") || audioFileName.endsWith(".adc")) {
								if (audioFileName.indexOf("msb") != -1) {
									hrp.setContentType("audio/x-pcm;rate=16000;bit=16;byte-order=big-endian");
								} else {
									hrp.setContentType("audio/x-pcm;rate=16000;bit=16;byte-order=little-endian");
								}
							} else
							if (audioFileName.endsWith(".mu")) {
								hrp.setContentType("audio/x-mulaw;rate=8000");
							} else
							if (audioFileName.endsWith(".alaw")) {
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
					long audioDataBytes = new File(audioFileName).length();
					if (audioDataBytes == 0) {
						System.out.println("音声データファイル " + audioFileName + " の読み込みに失敗しました。");
						break;
					}

					// HTTP 音声認識サーバへの音声データの送信の開始
					if (!hrp.feedDataResume(type, audioDataBytes)) {
						if (!verbose) {
							System.out.println(hrp.getLastMessage());
						}
						System.out.println("HTTP 音声認識サーバへの音声データの送信の開始に失敗しました。");
						break;
					}

					try (FileInputStream audioStream = new FileInputStream(audioFileName)) {
						// 音声データファイルからの音声データの読み込み
						byte[] audioData = new byte[4096];
						int audioDataReadBytes = audioStream.read(audioData, 0, audioData.length);
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
									System.out.println(hrp.getLastMessage());
								}
								System.out.println("HTTP 音声認識サーバへの音声データの送信に失敗しました。");
								break;
							}

							// 音声データファイルからの音声データの読み込み
							audioDataReadBytes = audioStream.read(audioData, 0, audioData.length);
						}
					} catch (IOException e) {
						System.out.println("音声データファイル " + audioFileName + " の読み込みに失敗しました。");
					}

					// HTTP 音声認識サーバへの音声データの送信の完了
					if (!hrp.feedDataPause()) {
						if (!verbose) {
							System.out.println(hrp.getLastMessage());
						}
						System.out.println("HTTP 音声認識サーバへの音声データの送信の完了に失敗しました。");
						break;
					}
				}
			}
			// <!--
			if (verbose) {
				System.out.println();
				System.out.println("=======================");
				System.out.println();
			}
			// -->
		} finally {
			// HTTP 音声認識サーバからの切断
			hrp.disconnect();
		}

		// 終了時間の取得
		long endTime = System.nanoTime();

		// 経過時間の表示
		System.out.println("INFO: elapsed time: " + ((float)(endTime - startTime) / 1000000.0f) + " [msec.]");
	}

	private boolean verbose_;

	public HrpTester(boolean verbose) {
		verbose_ = verbose;
	}

	@Override
	public void resultCreated(String sessionId) {
		System.out.println("C " + sessionId);
	}

	@Override
	public void resultUpdated(String result) {
		System.out.println("U " + result);
		String text = text_(result);
		if (text != null) {
			System.out.println("   -> " + text);
		}
	}

	@Override
	public void resultFinalized(String result) {
		System.out.println("F " + result);
		String text = text_(result);
		if (text != null) {
			System.out.println("   -> " + text);
		}
	}

	@Override
	public void TRACE(String message) {
		if (verbose_) {
			System.out.println(message);
		}
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
