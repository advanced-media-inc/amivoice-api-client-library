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
#include "com/amivoice/hrp/Hrp.h"
#include "com/amivoice/hrp/HrpListener.h"

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

class HrpSimpleTester : private com::amivoice::hrp::HrpListener {
	public: static void main(const StringList& args) {
		if (args.size() < 4) {
			print("Usage: HrpSimpleTester <url> <audioFileName> <codec> <grammarFileNames> [<authorization>]");
			return;
		}

		const char* serverURL = args[0]->toUTF8();
		const char* audioFileName = args[1]->to();
		const char* codec = args[2]->toUTF8();
		const char* grammarFileNames = args[3]->toUTF8();
		const char* authorization = (args.size() > 4) ? args[4]->toUTF8() : NULL;

		{
			// HTTP 音声認識サーバイベントリスナの作成
			Pointer<com::amivoice::hrp::HrpListener> listener = new HrpSimpleTester();

			// HTTP 音声認識サーバの初期化
			Pointer<com::amivoice::hrp::Hrp> hrp = com::amivoice::hrp::Hrp::construct();
			hrp->setListener(listener);
			hrp->setServerURL(serverURL);
			hrp->setCodec(codec);
			hrp->setGrammarFileNames(grammarFileNames);
			hrp->setAuthorization(authorization);

			// HTTP 音声認識サーバへの接続
			if (!hrp->connect()) {
				print("%s", hrp->getLastMessage());
				print("HTTP 音声認識サーバ %s への接続に失敗しました。", serverURL);
				return;
			}

			// HTTP 音声認識サーバへの音声データの送信の開始
			if (!hrp->feedDataResume()) {
				print("%s", hrp->getLastMessage());
				print("HTTP 音声認識サーバへの音声データの送信の開始に失敗しました。");
				return;
			}

			// 音声データファイルのオープン
			FILE* audioStream;
			if (fopen_s(&audioStream, audioFileName, "rb") == 0) {
				// 音声データファイルからの音声データの読み込み
				char audioData[4096];
				int audioDataReadBytes = (int)fread(audioData, 1, 4096, audioStream);
				while (audioDataReadBytes > 0) {
					// 微小時間のスリープ
					hrp->sleep(1);

					// HTTP 音声認識サーバへの音声データの送信
					if (!hrp->feedData(audioData, 0, audioDataReadBytes)) {
						print("%s", hrp->getLastMessage());
						print("HTTP 音声認識サーバへの音声データの送信に失敗しました。");
						break;
					}

					// 音声データファイルからの音声データの読み込み
					audioDataReadBytes = (int)fread(audioData, 1, 4096, audioStream);
				}

				// 音声データファイルのクローズ
				fclose(audioStream);
			} else {
				print("音声データファイル %s の読み込みに失敗しました。", audioFileName);
			}

			// HTTP 音声認識サーバへの音声データの送信の完了
			if (!hrp->feedDataPause()) {
				print("%s", hrp->getLastMessage());
				print("HTTP 音声認識サーバへの音声データの送信の完了に失敗しました。");
				return;
			}

			// HTTP 音声認識サーバからの切断
			hrp->disconnect();
		}
	}

	public: HrpSimpleTester() {
	}

	public: void resultCreated(const char* sessionId) override {
//		print("C %s", String().fromUTF8(sessionId).to());
	}

	public: void resultUpdated(const char* result) override {
//		print("U %s", String().fromUTF8(result).to());
	}

	public: void resultFinalized(const char* result) override {
//		print("F %s", String().fromUTF8(result).to());
		print("%s", String().fromUTF8(result).to());
		unsigned short* text = text_(result);
		if (text != NULL) {
			print(" -> %s", String().fromUTF16(text).to());
			delete[] text;
		}
	}

	public: void TRACE(const char* message) override {
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
	HrpSimpleTester::main(args);
	return 0;
}
