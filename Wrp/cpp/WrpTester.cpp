#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#if defined(_WIN32)
 #include <windows.h>
 #define _CRTDBG_MAP_ALLOC
 #include <crtdbg.h>
#else
 #include <fcntl.h>
 #include <time.h>
 #include <unistd.h>
 #include <sys/stat.h>
 #include <sys/time.h>
 #define _stricmp strcasecmp
 #define fopen_s(f,n,m) ((*(f))=fopen((n),(m)))==NULL
#endif
#include "com/amivoice/wrp/Wrp.h"
#include "com/amivoice/wrp/WrpListener.h"

// Pointer<T> class
template<typename T>
class Pointer {
  private:
	T* p_;
  public:
	Pointer(T* p) {
		p_ = p;
	}
	~Pointer() {
		if (p_ != NULL) {
			delete p_;
			p_ = NULL;
		}
	}
	T& operator=(T* p) {
		if (p_ != NULL) {
			delete p_;
			p_ = NULL;
		}
		p_ = p;
		return *this;
	}
	T* operator->() {
		return p_;
	}
	operator T*() {
		return p_;
	}
};

// String class
class String {
  private:
	char* p_;
	char* pUTF8_;
	bool necessaryToDelete_;
  public:
	String() {
		p_ = NULL;
		pUTF8_ = NULL;
		necessaryToDelete_ = false;
	}
	String(const char* p) {
		p_ = pUTF8_ = const_cast<char*>(p);
		necessaryToDelete_ = false;
	}
	~String() {
		if (necessaryToDelete_) {
			if (p_ != NULL) {
				delete[] p_;
				p_ = NULL;
			}
			if (pUTF8_ != NULL) {
				delete[] pUTF8_;
				pUTF8_ = NULL;
			}
			necessaryToDelete_ = false;
		}
	}
	String& from(const char* p) {
		if (necessaryToDelete_) {
			if (p_ != NULL) {
				delete[] p_;
				p_ = NULL;
			}
			if (pUTF8_ != NULL) {
				delete[] pUTF8_;
				pUTF8_ = NULL;
			}
			necessaryToDelete_ = false;
		}
		if (p != NULL) {
#if defined(_WIN32)
			int pLength = (int)strlen(p) + 1;
			p_ = new char[pLength];
			memcpy(p_, p, pLength);
			int pLengthUNICODE = ::MultiByteToWideChar(CP_ACP, 0, p, pLength, NULL, 0);
			wchar_t* pUNICODE = new wchar_t[pLengthUNICODE];
			::MultiByteToWideChar(CP_ACP, 0, p, pLength, pUNICODE, pLengthUNICODE);
			int pLengthUTF8 = ::WideCharToMultiByte(CP_UTF8, 0, pUNICODE, pLengthUNICODE, NULL, 0, NULL, NULL);
			pUTF8_ = new char[pLengthUTF8];
			::WideCharToMultiByte(CP_UTF8, 0, pUNICODE, pLengthUNICODE, pUTF8_, pLengthUTF8, NULL, NULL);
			delete[] pUNICODE;
			necessaryToDelete_ = true;
#else
			p_ = pUTF8_ = const_cast<char*>(p);
#endif
		}
		return *this;
	}
	String& fromUTF8(const char* pUTF8) {
		if (necessaryToDelete_) {
			if (p_ != NULL) {
				delete[] p_;
				p_ = NULL;
			}
			if (pUTF8_ != NULL) {
				delete[] pUTF8_;
				pUTF8_ = NULL;
			}
			necessaryToDelete_ = false;
		}
		if (pUTF8 != NULL) {
#if defined(_WIN32)
			int pLengthUTF8 = (int)strlen(pUTF8) + 1;
			pUTF8_ = new char[pLengthUTF8];
			memcpy(pUTF8_, pUTF8, pLengthUTF8);
			int pLengthUNICODE = ::MultiByteToWideChar(CP_UTF8, 0, pUTF8, pLengthUTF8, NULL, 0);
			wchar_t* pUNICODE = new wchar_t[pLengthUNICODE];
			::MultiByteToWideChar(CP_UTF8, 0, pUTF8, pLengthUTF8, pUNICODE, pLengthUNICODE);
			int pLength = ::WideCharToMultiByte(CP_ACP, 0, pUNICODE, pLengthUNICODE, NULL, 0, NULL, NULL);
			p_ = new char[pLength];
			::WideCharToMultiByte(CP_ACP, 0, pUNICODE, pLengthUNICODE, p_, pLength, NULL, NULL);
			delete[] pUNICODE;
			necessaryToDelete_ = true;
#else
			p_ = pUTF8_ = const_cast<char*>(pUTF8);
#endif
		}
		return *this;
	}
	String& fromUTF16(const unsigned short* pUTF16) {
		if (necessaryToDelete_) {
			if (p_ != NULL) {
				delete[] p_;
				p_ = NULL;
			}
			if (pUTF8_ != NULL) {
				delete[] pUTF8_;
				pUTF8_ = NULL;
			}
			necessaryToDelete_ = false;
		}
		if (pUTF16 != NULL) {
			int pLengthUTF8 = 0;
			const unsigned short* pPointerUTF16 = pUTF16;
			unsigned short c = *pPointerUTF16++;
			while (c != 0) {
				if ((c & 0xFF80) == 0x0000) {
					// 00000000 0yyyyyyy
					pLengthUTF8++;
				} else
				if ((c & 0xF800) == 0x0000) {
					// 00000xxx yyyyyyyy
					pLengthUTF8 += 2;
				} else {
					if ((c & 0xFC00) == 0xD800) {
						unsigned short cc = *pPointerUTF16++;
						if (cc != 0) {
							if ((cc & 0xFC00) == 0xDC00) {
								// 110110ZZ ZZxxxxxx 110111xx yyyyyyyy
								pLengthUTF8 += 4;
								c = *pPointerUTF16++;
								continue;
							}
						}
					}
					// xxxxxxxx yyyyyyyy
					pLengthUTF8 += 3;
				}
				c = *pPointerUTF16++;
			}
			pLengthUTF8++;
			pUTF8_ = new char[pLengthUTF8];
			pLengthUTF8 = 0;
			pPointerUTF16 = pUTF16;
			c = *pPointerUTF16++;
			while (c != 0) {
				if ((c & 0xFF80) == 0x0000) {
					// 00000000 0yyyyyyy → 1 バイト文字 0yyyyyyy
					pUTF8_[pLengthUTF8++] = (char)(c & 0x007F);
				} else
				if ((c & 0xF800) == 0x0000) {
					// 00000xxx yyyyyyyy → 2 バイト文字 110xxxyy 10yyyyyy
					pUTF8_[pLengthUTF8++] = (char)(0xC0 | ((c >> 6) & 0x1F));
					pUTF8_[pLengthUTF8++] = (char)(0x80 | ((c     ) & 0x3F));
				} else {
					if ((c & 0xFC00) == 0xD800) {
						unsigned short cc = *pPointerUTF16++;
						if (cc != 0) {
							if ((cc & 0xFC00) == 0xDC00) {
								// 110110ZZ ZZxxxxxx 110111xx yyyyyyyy → 4 バイト文字 11110zzz 10zzxxxx 10xxxxyy 10yyyyyy
								pUTF8_[pLengthUTF8++] = (char)(0xF0 | (((((c >> 6) & 0xF) + 1) >> 2) & 0x07));
								pUTF8_[pLengthUTF8++] = (char)(0x80 | (((((c >> 6) & 0xF) + 1) << 4) & 0x30) | ((c >> 2) & 0x0F));
								pUTF8_[pLengthUTF8++] = (char)(0x80 | ((c << 4) & 0x30) | ((cc >> 6) & 0x0F));
								pUTF8_[pLengthUTF8++] = (char)(0x80 | (cc & 0x3F));
								c = *pPointerUTF16++;
								continue;
							}
						}
					}
					// xxxxxxxx yyyyyyyy → 3 バイト文字 1110xxxx 10xxxxyy 10yyyyyy
					pUTF8_[pLengthUTF8++] = (char)(0xE0 | ((c >> 12) & 0x0F));
					pUTF8_[pLengthUTF8++] = (char)(0x80 | ((c >>  6) & 0x3F));
					pUTF8_[pLengthUTF8++] = (char)(0x80 | ((c      ) & 0x3F));
				}
				c = *pPointerUTF16++;
			}
			pUTF8_[pLengthUTF8++] = 0;
#if defined(_WIN32)
			int pLength = ::WideCharToMultiByte(CP_ACP, 0, reinterpret_cast<const wchar_t*>(pUTF16), -1, NULL, 0, NULL, NULL);
			p_ = new char[pLength];
			::WideCharToMultiByte(CP_ACP, 0, reinterpret_cast<const wchar_t*>(pUTF16), -1, p_, pLength, NULL, NULL);
#else
			int pLength = pLengthUTF8;
			p_ = new char[pLength];
			memcpy(p_, pUTF8_, pLengthUTF8);
#endif
			necessaryToDelete_ = true;
		}
		return *this;
	}
	const char* to() const {
		return p_;
	}
	const char* toUTF8() const {
		return pUTF8_;
	}
	operator const char*() const {
		return p_;
	}
	bool startsWith(const char* prefix) const {
		size_t pLength = strlen(p_);
		size_t prefixLength = strlen(prefix);
		return (pLength - prefixLength >= 0 && strncmp(p_, prefix, prefixLength) == 0);
	}
	bool endsWith(const char* suffix) const {
		size_t pLength = strlen(p_);
		size_t suffixLength = strlen(suffix);
		return (pLength - suffixLength >= 0 && strncmp(p_ + (pLength - suffixLength), suffix, suffixLength) == 0);
	}
	int indexOf(const char* string) const {
		char* pPointer = strstr(p_, string);
		if (pPointer == NULL) {
			return -1;
		}
		return static_cast<int>(pPointer - p_);
	}
	int lastIndexOf(const char* string) const {
		char* pLastPointer = NULL;
		char* pPointer = strstr(p_, string);
		while (pPointer != NULL) {
			pLastPointer = pPointer;
			pPointer = strstr(pPointer + strlen(string), string);
		}
		if (pLastPointer == NULL) {
			return -1;
		}
		return static_cast<int>(pLastPointer - p_);
	}
	int length() const {
		return (int)strlen(p_);
	}
};

// StringList class
class StringList {
  private:
	const String** strings_;
	bool* deletables_;
	int capacity_;
	int size_;
  public:
	StringList() {
		strings_ = NULL;
		deletables_ = NULL;
		capacity_ = 0;
		size_ = 0;
	}
	~StringList() {
		for (int i = 0; i < size_; i++) {
			if (deletables_[i]) {
				delete const_cast<String*>(strings_[i]);
			}
			strings_[i] = NULL;
		}
		if (strings_ != NULL) {
			free(strings_);
			strings_ = NULL;
		}
		if (deletables_ != NULL) {
			free(deletables_);
			deletables_ = NULL;
		}
	}
	void ensureCapacity(int capacity) {
		if (capacity > capacity_) {
			const String** strings = (const String**)malloc(capacity * sizeof(const String*));
			memset(strings, 0, capacity * sizeof(const String*));
			if (size_ > 0) {
				memcpy(strings, strings_, size_ * sizeof(const String*));
			}
			bool* deletables = (bool*)malloc(capacity * sizeof(bool));
			memset(deletables, 0, capacity * sizeof(bool));
			if (size_ > 0) {
				memcpy(deletables, deletables_, size_ * sizeof(bool));
			}
			strings_ = strings;
			deletables_ = deletables;
			capacity_ = capacity;
		}
	}
	void add(const char* p) {
		if (size_ + 1 > capacity_) {
			ensureCapacity(capacity_ + 16);
		}
		String* string = new String();
		string->from(p);
		strings_[size_] = string;
		deletables_[size_] = true;
		size_++;
	}
	void add(const String* string) {
		if (size_ + 1 > capacity_) {
			ensureCapacity(capacity_ + 16);
		}
		strings_[size_] = string;
		deletables_[size_] = false;
		size_++;
	}
	int size() const {
		return size_;
	}
	const String* operator[](int index) const {
		if (index < 0 || index >= size_) {
			return NULL;
		}
		return strings_[index];
	}
};

// getFileSize()
long long getFileSize(const char* fileName) {
	unsigned long long fileSize = 0;
#if defined(_WIN32)
	HANDLE fileHandle = ::CreateFileA(fileName, FILE_SHARE_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		DWORD fileSizeHigh = 0;
		DWORD fileSizeLow = ::GetFileSize(fileHandle, &fileSizeHigh);
		if (fileSizeLow != INVALID_FILE_SIZE) {
			fileSize = (unsigned long long)fileSizeLow | ((unsigned long long)fileSizeHigh << 32);
		}
		::CloseHandle(fileHandle);
	}
#else
	int fileHandle = open(fileName, O_RDONLY);
	if (fileHandle != -1) {
		struct stat fileStatus;
		if (fstat(fileHandle, &fileStatus) != -1) {
			fileSize = (unsigned long long)fileStatus.st_size;
		}
		close(fileHandle);
	}
#endif
	return (long long)fileSize;
}

// getNanoTime()
long long getNanoTime() {
	unsigned long long nanoTime = 0;
#if defined(_WIN32)
	LARGE_INTEGER frequencyInfo;
	::QueryPerformanceFrequency(&frequencyInfo);
	LARGE_INTEGER counterInfo;
	::QueryPerformanceCounter(&counterInfo);
	unsigned long long frequency = frequencyInfo.QuadPart;
	unsigned long long counter = counterInfo.QuadPart;
	const unsigned long long counterBase = 0x7FFFFFFFFFFFFFFFull / 1000000000ull;
	if (counter < counterBase) {
		nanoTime = counter * 1000000000ull / frequency;
	} else {
		int counterPerFrequencyDelta = (int)(counterBase / frequency);
		unsigned long long counterDelta = frequency * counterPerFrequencyDelta;
		int counterPerFrequency = 0;
		do {
			counterPerFrequency += counterPerFrequencyDelta;
			counter -= counterDelta;
		} while (counter >= counterBase);
		nanoTime = counter * 1000000000ull / frequency + 1000000000ull * counterPerFrequency;
	}
#else
 #if defined(CLOCK_MONOTONIC)
	struct timespec currentTime;
	clock_gettime(CLOCK_MONOTONIC, &currentTime);
	nanoTime = (unsigned long long)currentTime.tv_sec * 1000000000 + (unsigned long long)currentTime.tv_nsec;
 #else
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	nanoTime = (unsigned long long)currentTime.tv_sec * 1000000000 + (unsigned long long)currentTime.tv_usec * 1000;
 #endif
#endif
	return (long long)nanoTime;
}

// print()
void print(const char* format = NULL, ...) {
	if (format != NULL) {
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
	printf("\n");
}

class WrpTester : private com::amivoice::wrp::WrpListener {
	public: static void main(const StringList& args) {
		// WebSocket 音声認識サーバ URL
		const char* serverURL = NULL;
		// プロキシサーバ名
		const char* proxyServerName = NULL;
		// 音声データファイル名
		StringList audioFileNames;
		// グラマファイル名
		const char* grammarFileNames = NULL;
		// プロファイル ID
		const char* profileId = NULL;
		// プロファイル登録単語
		const char* profileWords = NULL;
		// セグメンタプロパティ
		const char* segmenterProperties = NULL;
		// フィラー単語を保持するかどうか
		const char* keepFillerToken = NULL;
		// 認識中イベント発行間隔
		const char* resultUpdatedInterval = NULL;
		// 拡張情報
		const char* extension = NULL;
		// サービス認証キー文字列
		const char* authorization = NULL;
		// 音声データ形式
		const char* codec = NULL;
		// 認識結果タイプ
		const char* resultType = NULL;
		// サービス認証キー文字列
		const char* serviceAuthorization = NULL;
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
		for (int i = 0; i < args.size(); i++) {
			const String* arg = args[i];
			if (arg->startsWith("g=")) {
				grammarFileNames = arg->toUTF8() + 2;
			} else
			if (arg->startsWith("i=")) {
				profileId = arg->toUTF8() + 2;
			} else
			if (arg->startsWith("w=")) {
				profileWords = arg->toUTF8() + 2;
			} else
			if (arg->startsWith("op=")) {
				segmenterProperties = arg->toUTF8() + 3;
			} else
			if (arg->startsWith("of=")) {
				keepFillerToken = arg->toUTF8() + 3;
			} else
			if (arg->startsWith("oi=")) {
				resultUpdatedInterval = arg->toUTF8() + 3;
			} else
			if (arg->startsWith("oe=")) {
				extension = arg->toUTF8() + 3;
			} else
			if (arg->startsWith("ou=")) {
				authorization = arg->toUTF8() + 3;
			} else
			if (arg->startsWith("c=")) {
				codec = arg->toUTF8() + 2;
			} else
			if (arg->startsWith("r=")) {
				resultType = arg->toUTF8() + 2;
			} else
			if (arg->startsWith("u=")) {
				serviceAuthorization = arg->toUTF8() + 2;
			} else
			if (arg->startsWith("-x")) {
				proxyServerName = arg->toUTF8() + 2;
			} else
			if (arg->startsWith("-c")) {
				connectTimeout = atoi(arg->to() + 2);
			} else
			if (arg->startsWith("-r")) {
				receiveTimeout = atoi(arg->to() + 2);
			} else
			if (arg->startsWith("-l")) {
				loop = atoi(arg->to() + 2);
			} else
			if (arg->startsWith("-e")) {
				if (arg->length() > 2) {
					sleepTime = atoi(arg->to() + 2);
				} else {
					sleepTime = -1;
				}
			} else
			if (arg->startsWith("-v")) {
				verbose = true;
			} else
			if (arg->startsWith("-2")) {
				implementation = 2;
			} else
			if (arg->startsWith("-3")) {
				implementation = 3;
			} else
			if (serverURL == NULL) {
				serverURL = arg->toUTF8();
			} else {
				audioFileNames.add(arg);
			}
			if (verbose) {
				print("DEBUG: %s", arg->to());
			}
		}
		if (audioFileNames.size() == 0) {
			print("Usage: WrpTester [<parameters/options>]");
			print("                  <url>");
			print("                   <audioFileName>...");
			print("Parameters:");
			print("  g=<grammarFileNames>");
			print("  i=<profileId>");
			print("  w=<profileWords>");
			print("  op=<segmenterProperties>");
			print("  of=<keepFillerToken>");
			print("  oi=<resultUpdatedInterval>");
			print("  oe=<extension>");
			print("  ou=<authorization>");
			print("  c=<codec>");
			print("  r=<resultType>");
			print("  u=<serviceAuthorization>");
			print("Options:");
			print("  -x<proxyServerName>         (default: -x)");
			print("  -c<connectionTimeout>       (default: -c5000)");
			print("  -r<receiveTimeout>          (default: -r0)");
			print("  -l<loop>                    (default: -l1)");
			print("  -e                          realtime simulation (default: -)");
			print("  -v                          verbose output (default: -)");
			return;
		}

		// 開始時間の取得
		long long startTime = getNanoTime();

		// スリープ時間の計算
		if (sleepTime == -1) {
			if (codec != NULL) {
				if (_stricmp(codec, "22K") == 0 || _stricmp(codec, "MSB22K") == 0 || _stricmp(codec, "LSB22K") == 0) {
					sleepTime = 4096 * 1000 / 2 / 22050;
				} else
				if (_stricmp(codec, "16K") == 0 || _stricmp(codec, "MSB16K") == 0 || _stricmp(codec, "LSB16K") == 0) {
					sleepTime = 4096 * 1000 / 2 / 16000;
				} else
				if (_stricmp(codec, "11K") == 0 || _stricmp(codec, "MSB11K") == 0 || _stricmp(codec, "LSB11K") == 0) {
					sleepTime = 4096 * 1000 / 2 / 11025;
				} else
				if (_stricmp(codec, "8K") == 0 || _stricmp(codec, "MSB8K") == 0 || _stricmp(codec, "LSB8K") == 0) {
					sleepTime = 4096 * 1000 / 2 / 8000;
				} else
				if (_stricmp(codec, "MULAW") == 0 || _stricmp(codec, "ALAW") == 0) {
					sleepTime = 4096 * 1000 / 1 / 8000;
				} else {
					sleepTime = 4096 * 1000 / 2 / 16000; // 16K が指定されたものとして計算...
				}
			} else {
				sleepTime = 4096 * 1000 / 2 / 16000; // 16K が指定されたものとして計算...
			}
		}

		{
			// WebSocket 音声認識サーバイベントリスナの作成
			Pointer<com::amivoice::wrp::WrpListener> listener = new WrpTester(verbose);

			// WebSocket 音声認識サーバの初期化
			Pointer<com::amivoice::wrp::Wrp> wrp = com::amivoice::wrp::Wrp::construct(implementation);
			wrp->setListener(listener);
			wrp->setServerURL(serverURL);
			wrp->setProxyServerName(proxyServerName);
			wrp->setConnectTimeout(connectTimeout);
			wrp->setReceiveTimeout(receiveTimeout);
			wrp->setGrammarFileNames(grammarFileNames);
			wrp->setProfileId(profileId);
			wrp->setProfileWords(profileWords);
			wrp->setSegmenterProperties(segmenterProperties);
			wrp->setKeepFillerToken(keepFillerToken);
			wrp->setResultUpdatedInterval(resultUpdatedInterval);
			wrp->setExtension(extension);
			wrp->setAuthorization(authorization);
			wrp->setCodec(codec);
			wrp->setResultType(resultType);
			wrp->setServiceAuthorization(serviceAuthorization);

			// WebSocket 音声認識サーバへの接続
			if (!wrp->connect()) {
				if (!verbose) {
					print("%s", wrp->getLastMessage());
				}
				print("WebSocket 音声認識サーバ %s への接続に失敗しました。", serverURL);
				return;
			}

			// <!--
			if (verbose) {
				print();
				print("======================= %s", com::amivoice::wrp::Wrp::getVersion());
			}
			// -->
			for (int i = 0; i < loop; i++) {
				for (int j = 0; j < audioFileNames.size(); j++) {
					const String* audioFileName = audioFileNames[j];
					// <!--
					if (verbose) {
						print();
						if (loop > 1) {
							print("----------------------- [%d] %s", i + 1, audioFileName->to());
						} else {
							print("----------------------- %s", audioFileName->to());
						}
						print();
					}
					// -->

					// WebSocket 音声認識サーバへの音声データの送信の開始
					if (!wrp->feedDataResume()) {
						if (!verbose) {
							print("%s", wrp->getLastMessage());
						}
						print("WebSocket 音声認識サーバへの音声データの送信の開始に失敗しました。");
						break;
					}

					// 音声データファイルのオープン
					FILE* audioStream;
					if (fopen_s(&audioStream, audioFileName->to(), "rb") == 0) {
						// 音声データファイルからの音声データの読み込み
						char audioData[4096];
						int audioDataReadBytes = (int)fread(audioData, 1, 4096, audioStream);
						while (audioDataReadBytes > 0) {
							// スリープ時間が計算されているかどうかのチェック
							if (sleepTime >= 0) {
								// スリープ時間が計算されている場合...
								// 微小時間のスリープ
								wrp->sleep(sleepTime);
							} else {
								// スリープ時間が計算されていない場合...
								// 微小時間のスリープ
								wrp->sleep(1);

								// 認識結果情報待機数が 1 以下になるまでスリープ
								int maxSleepTime = 50000;
								while (wrp->getWaitingResults() > 1 && maxSleepTime > 0) {
									wrp->sleep(100);
									maxSleepTime -= 100;
								}
							}

							// WebSocket 音声認識サーバへの音声データの送信
							if (!wrp->feedData(audioData, 0, audioDataReadBytes)) {
								if (!verbose) {
									print("%s", wrp->getLastMessage());
								}
								print("WebSocket 音声認識サーバへの音声データの送信に失敗しました。");
								break;
							}

							// 音声データファイルからの音声データの読み込み
							audioDataReadBytes = (int)fread(audioData, 1, 4096, audioStream);
						}

						// 音声データファイルのクローズ
						fclose(audioStream);
					} else {
						print("音声データファイル %s の読み込みに失敗しました。", audioFileName->to());
					}

					// WebSocket 音声認識サーバへの音声データの送信の完了
					if (!wrp->feedDataPause()) {
						if (!verbose) {
							print("%s", wrp->getLastMessage());
						}
						print("WebSocket 音声認識サーバへの音声データの送信の完了に失敗しました。");
						break;
					}
				}
			}
			// <!--
			if (verbose) {
				print();
				print("=======================");
				print();
			}
			// -->

			// WebSocket 音声認識サーバからの切断
			wrp->disconnect();
		}

		// 終了時間の取得
		long long endTime = getNanoTime();

		// 経過時間の表示
		print("INFO: elapsed time: %.3f [msec.]", (float)(endTime - startTime) / 1000000.0f);
	}

	private: bool verbose_;

	public: WrpTester(bool verbose) {
		verbose_ = verbose;
	}

	public: void utteranceStarted(int startTime) override {
		print("S %d", startTime);
	}

	public: void utteranceEnded(int endTime) override {
		print("E %d", endTime);
	}

	public: void resultCreated() override {
		print("C");
	}

	public: void resultUpdated(const char* result) override {
		print("U %s", String().fromUTF8(result).to());
		unsigned short* text = text_(result);
		if (text != NULL) {
			print("   -> %s", String().fromUTF16(text).to());
			delete[] text;
		}
	}

	public: void resultFinalized(const char* result) override {
		print("F %s", String().fromUTF8(result).to());
		unsigned short* text = text_(result);
		if (text != NULL) {
			print("   -> %s", String().fromUTF16(text).to());
			delete[] text;
		}
	}

	public: void eventNotified(int eventId, const char* eventMessage) override {
		print("%c %s", (char)eventId, String().fromUTF8(eventMessage).to());
	}

	public: void TRACE(const char* message) override {
		if (verbose_) {
			print("%s", String().fromUTF8(message).to());
		}
	}

	private: unsigned short* text_(const char* result) {
		int index = String(result).lastIndexOf(",\"text\":\"");
		if (index == -1) {
			return NULL;
		}
		index += 9;
		int resultLength = String(result).length();
		unsigned short* buffer = new unsigned short[resultLength - index];
		int bufferLength = 0;
		int c = (index >= resultLength) ? 0 : result[index++];
		while (c != 0) {
			if (c == '"') {
				break;
			}
			if (c == '\\') {
				c = (index >= resultLength) ? 0 : result[index++];
				if (c == 0) {
					delete[] buffer;
					return NULL;
				}
				if (c == '"' || c == '\\' || c == '/') {
					buffer[bufferLength++] = (unsigned short)(char)c;
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
						delete[] buffer;
						return NULL;
					}
					buffer[bufferLength++] = (unsigned short)((c0 << 12) | (c1 << 8) | (c2 << 4) | c3);
				} else {
					delete[] buffer;
					return NULL;
				}
			} else {
				buffer[bufferLength++] = (unsigned short)(char)c;
			}
			c = (index >= resultLength) ? 0 : result[index++];
		}
		buffer[bufferLength++] = 0;
		return buffer;
	}
};

int main(int argc, char* argv[]) {
#if defined(_WIN32)
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	StringList args;
	for (int i = 1; i < argc; i++) {
		args.add(argv[i]);
	}
	WrpTester::main(args);
	return 0;
}
