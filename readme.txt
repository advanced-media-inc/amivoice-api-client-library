Hrp ライブラリ／Wrp ライブラリについて
==========================================================================

■ サポートするプログラミング言語

・Java
    - 以下の開発環境にてビルド
       - Java 7
    - 以下のバージョンにて動作確認
       - Java 7
       - Java 9
    - 以下のプラットフォームにて動作確認
       - Windows 10
       - Cent OS 7

・Python
    - 以下のバージョンにて動作確認
       - 2.7.16
       - 3.7.4
    - 以下のプラットフォームにて動作確認
       - Windows 10
       - Cent OS 7

・PHP
    - 以下のバージョンにて動作確認
       - 5.6.40
       - 7.3.11
    - 以下のプラットフォームにて動作確認
       - Windows 10
       - Cent OS 7

・C#
    - 以下の開発環境にてビルド
       - Visual Studio 2015 (.NET Framework 4.5.2)
    - 以下のプラットフォームにて動作確認
       - Windows 10

・C++
    - 以下の開発環境にてビルド
       - Visual Studio 2015 (Visuaul C++ 14.0)
       - gcc 4.8.5
    - 以下のプラットフォームにて動作確認
       - Windows 10
       - Cent OS 7


■ 注意事項

・共通

  ・Hrp/Wrp ライブラリは HTTP や WebSocket  といったプロトコルを使って音声
    認識サーバと通信をするためのライブラリですが、可能な限り各プログラミン
    グ言語間の差異を排除するために、たとえ各プログラミング言語が  HTTP  や
    WebSocket のためのライブラリを持っていたとしても、あえてそれらのライブ
    ラリを使用せずに低レベルなソケットライブラリを使用して実装しています。
  ・全てのソースファイルは UTF-8 文字エンコーディングで記述しています。

・Java

  ・ライブラリは jar ファイルとして提供されます。
  ・java/src フォルダには、jar  ファイルをビルドするために必要な全てのソー
    スファイル群が含まれています。
  ・TLS ソケットを作成する際、SSLContext.getInstance() メソッドの protocol
    引数には "TLSv1.2" を指定しています。
  ・(Hrp|Hrp)(Tester|SimpleTester).java で定義している text_()  メソッドは
    JSON の形式の認識結果情報から text  情報を抽出するという処理を行ってい
    まずが、Java の標準ライブラリに JSON  文字列をパースする処理が存在しな
    いため、当メソッドでは簡便な方法で当処理を実装しています。当メソッドが
    JSON  文字列をパースする処理として正しいことを保証するものではありませ
    ん。

・Python

  ・ライブラリは .py ファイル群として提供されます。
  ・同一のソースファイルにて Python 2 系と Python 3 系の両方をサポートして
    います。
  ・Python 2 系でライブラリを使用する際、 ライブラリが提供する各メソッドの
    文字列引数には str 型／unicode 型どちらの型でも指定できます。
  ・Windows  プラットフォームでライブラリを使用する際、 サーバ証明書データ
    ベースファイルへのパスを環境変数 SSL_CERT_FILE  に指定する必要がありま
    す。

・PHP

  ・ライブラリは .php ファイル群として提供されます。
  ・同一のソースファイルにて PHP 5 系と PHP  7  系の両方をサポートしていま
    す。
  ・PHP 5 系でライブラリを使用する際、ライブラリが提供する各メソッドの文字
    列引数には UTF-8  文字エンコーディングでエンコードされた文字列を指定す
    る必要があり、 ライブラリのイベントリスナを通して  UTF-8  文字エンコー
    ディングでエンコードされた文字列が通知されます。
  ・Windows  プラットフォームでライブラリを使用する際、 サーバ証明書データ
    ベースファイルへのパスを php.ini ディレクティブ openssl.cafile  に指定
    する必要があります。
  ・PHP の標準ライブラリに接続済み TCP ソケットから TLS ソケットを生成する
    処理が存在しないため、HTTP プロキシを通して スキームが https や wss の
    URL に接続できない (接続できな場合がある) ので注意してください。

・C#

  ・ライブラリはマネージドクラスライブラリ (.dll ファイル)  として提供され
    ます。
  ・cs/src フォルダには、 マネージドクラスライブラリをビルドするために必要
    な全てのソースファイル群が含まれています。
  ・TLS  ソケットを作成する際、 SslStream.AuthenticateAsClient  メソッドの
    enabledSslProtocols 引数には SslProtocols.Tls12 を指定しています。
  ・(Hrp|Hrp)(Tester|SimpleTester).cs  で定義している  text_()  メソッドは
    JSON の形式の認識結果情報から text  情報を抽出するという処理を行ってい
    まずが、C# の標準ライブラリに JSON  文字列をパースする処理が存在しない
    ため、当メソッドでは簡便な方法で当処理を実装しています。  当メソッドが
    JSON  文字列をパースする処理として正しいことを保証するものではありませ
    ん。

・C++

  ・ライブラリはダイナミックライブラリ (.dll ファイル／.so ファイル)  とし
    て提供されます。
  ・cpp/src フォルダには、ダイナミックライブラリをビルドするために必要な全
    てのソースファイル群が含まれています。
  ・POCO C++ Libraries 1.9.4 を使用しています。 ライセンスを当文書の末尾に
    記載します。
  ・POCO C++ Libraries プロジェクトがビルドした OpenSSL version 1.1.0 を使
    用しています。ライセンスを当文書の末尾に記載します。
  ・ダイナミックライブラリ  (.dll ファイル／.so ファイル)   をビルドするに
    は、別途 C++ Libraries や OpenSSL のヘッダファイル群やスタティックライ
    ブラリ群が必要です。https://pocoproject.org/download.html の指示に従っ
    てダウンロードやビルドを行ってください。
  ・ライブラリが提供する各メソッドの文字列引数には UTF-8  文字エンコーディ
    ングでエンコードされた文字列への const char*  ポインタを指定する必要が
    あり、ライブラリのイベントリスナを通して UTF-8文字エンコーディングでエ
    ンコードされた文字列への const char* ポインタが通知されます。
  ・Windows  プラットフォームでライブラリを使用する際、 サーバ証明書データ
    ベースファイルへのパスを環境変数 SSL_CERT_FILE  に指定する必要がありま
    す。
  ・(Hrp|Hrp)(Tester|SimpleTester).cpp で定義している  text_()  メソッドは
    JSON の形式の認識結果情報から text  情報を抽出するという処理を行ってい
    まずが、C++ の標準ライブラリに  JSON  文字列をパースする処理が存在しな
    いため、当メソッドでは簡便な方法で当処理を実装しています。当メソッドが
    JSON  文字列をパースする処理として正しいことを保証するものではありませ
    ん。
  ・(Hrp|Hrp)(Tester|SimpleTester).cpp  で定義している  Pointer   クラス／
    String クラス／StringList クラスは、サンプルプログラムを実現するために
    必要最低限な処理のみを実装したクラスです。それぞれ汎用的な Pointer  ク
    ラス／String クラス／StringList クラスとしての動作を保証するものではあ
    りません。


■ POCO C++ Libraries ライセンス

--------------------------------------------------------------------------
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

---------------------------------------------------------------------------
Note:
Individual files contain the following tag instead of the full license text.

	SPDX-License-Identifier:	BSL-1.0

This enables machine processing of license information based on the SPDX
License Identifiers that are here available: http://spdx.org/licenses/
--------------------------------------------------------------------------


■ OpenSSL ライセンス

--------------------------------------------------------------------------
  OpenSSL License
  ---------------

/* ====================================================================
 * Copyright (c) 1998-2017 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

 Original SSLeay License
 -----------------------

/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
--------------------------------------------------------------------------
