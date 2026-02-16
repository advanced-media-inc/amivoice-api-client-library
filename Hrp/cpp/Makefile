# Hrp (C++) ビルド用 Makefile
#
# 目的:
# - 既存の *.makefile を使わずに、Hrp の共有ライブラリとサンプルをビルドする
# - macOS / Linux の両方を想定（Windows は対象外）
#
# 使い方（例）:
#   make                   # Release
#   make CONFIG=debug       # Debug
#   make clean
#
# 依存関係:
# - POCO C++ Libraries + OpenSSL のヘッダ/ライブラリが必要です。
# - このリポジトリに同梱されているパッケージがあれば自動検出を試みます。
#   見つからない場合は、make 実行時に POCO_HOME を指定してください。
#     例: make POCO_HOME=/path/to/_poco

NAME := Hrp

# ---- ツール類 ----
CXX ?= c++
AR ?= ar

# ---- ビルド種別 ----
CONFIG ?= release

# ---- OS/ARCH 判定 ----
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
  PLATFORM := macos
  SHLIB_EXT := dylib
  SHLIB_LDFLAGS := -dynamiclib
  SHLIB_INSTALL_NAME := -Wl,-install_name,@rpath/lib$(NAME).$(SHLIB_EXT)
  EXE_RPATH := -Wl,-rpath,@executable_path
  PLATFORM_DEFINES := -DMACOS -DPOSIX
  SYS_LIBS := -lz
else
  PLATFORM := linux
  SHLIB_EXT := so
  SHLIB_LDFLAGS := -shared
  EXE_RPATH := -Wl,-rpath,'$$ORIGIN'
  PLATFORM_DEFINES := -DLINUX -DPOSIX
  SYS_LIBS := -ldl -lpthread
endif

ARCH := $(UNAME_M)

# ---- 出力先 ----
OUT_DIR := bin/$(PLATFORM)_$(ARCH)_$(CONFIG)
OBJ_DIR := obj/$(PLATFORM)_$(ARCH)_$(CONFIG)

LIB_OUT := $(OUT_DIR)/lib$(NAME).$(SHLIB_EXT)
SIMPLE_OUT := $(OUT_DIR)/$(NAME)SimpleTester
TESTER_OUT := $(OUT_DIR)/$(NAME)Tester

# ---- ソース ----
SRC_DIR := src
LIB_SRCS := \
	$(SRC_DIR)/com/amivoice/hrp/Hrp.cpp \
	$(SRC_DIR)/com/amivoice/hrp/Hrp_.cpp \
	$(SRC_DIR)/com/amivoice/hrp/Hrp__.cpp

LIB_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_SRCS))

# ---- 依存ライブラリ（POCO/OpenSSL） ----
# 同梱物の自動検出（見つからない場合は POCO_HOME を指定）
POCO_HOME_AUTO := $(firstword \
	$(wildcard ../../_/_poco) \
	$(wildcard ../../../_/_poco) \
)
POCO_HOME ?= $(POCO_HOME_AUTO)

# clean/help のときは依存チェックを行わない（ファイルが無くても掃除できるようにする）
ifneq ($(filter clean help,$(MAKECMDGOALS)),)
  SKIP_DEPS_CHECK := 1
endif

ifndef SKIP_DEPS_CHECK
  ifeq ($(strip $(POCO_HOME)),)
    $(error POCO_HOME が未設定です。POCO/OpenSSL を用意し、make POCO_HOME=... を指定してください)
  endif
  ifeq ($(wildcard $(POCO_HOME)/include/Poco/Poco.h),)
    $(error POCO_HOME=$(POCO_HOME) が正しくありません（include/Poco/Poco.h が見つかりません）)
  endif
endif

POCO_INCLUDE_DIR := $(POCO_HOME)/include

# lib ディレクトリはプラットフォームで異なる（同梱物の構成に合わせる）
ifeq ($(PLATFORM),macos)
  POCO_LIB_DIR ?= $(POCO_HOME)/lib/macos
else
  POCO_LIB_DIR ?= $(POCO_HOME)/lib/linux/x86_64
endif

POCO_COMPONENT_LIBS := -lPocoNet -lPocoNetSSL -lPocoCrypto -lPocoUtil -lPocoJSON -lPocoXML -lPocoFoundation

# macOS 同梱物は libPoco.a（統合）を優先して使う
ifneq ($(wildcard $(POCO_LIB_DIR)/libPoco.a),)
  POCO_LIBS ?= $(POCO_LIB_DIR)/libPoco.a
else
  POCO_LIBS ?= $(POCO_COMPONENT_LIBS)
endif

# OpenSSL も同梱の *.a があればそれを使う
ifneq ($(wildcard $(POCO_LIB_DIR)/libssl.a),)
  OPENSSL_LIBS ?= $(POCO_LIB_DIR)/libssl.a $(POCO_LIB_DIR)/libcrypto.a
else
  OPENSSL_LIBS ?= -lssl -lcrypto
endif

# 追加のリンクオプション（必要に応じて上書きしてください）
EXTRA_LDFLAGS ?=
EXTRA_LIBS ?=

# ---- フラグ ----
COMMON_CXXFLAGS := -std=c++17 -w

ifeq ($(CONFIG),debug)
  OPT_CXXFLAGS := -O0 -g
  COMMON_DEFINES := -D_DEBUG
else
  OPT_CXXFLAGS := -O3
  COMMON_DEFINES := -DNDEBUG
endif

INCLUDES := -I$(SRC_DIR) -I$(POCO_INCLUDE_DIR)

# ライブラリ本体は _USRDLL を定義してビルドする（Hrp.h のメンバ定義に必要）
LIB_DEFINES := $(PLATFORM_DEFINES) $(COMMON_DEFINES) -D_USRDLL
APP_DEFINES := $(PLATFORM_DEFINES) $(COMMON_DEFINES)

LIB_CXXFLAGS := $(COMMON_CXXFLAGS) $(OPT_CXXFLAGS) -fPIC $(INCLUDES) $(LIB_DEFINES)
APP_CXXFLAGS := $(COMMON_CXXFLAGS) $(OPT_CXXFLAGS) $(INCLUDES) $(APP_DEFINES)

# ---- ターゲット ----
.PHONY: all lib simple tester clean help

all: lib simple tester

help:
	@echo "Hrp (C++) ビルド用 Makefile"
	@echo ""
	@echo "主なターゲット:"
	@echo "  make                : Release ビルド（lib + SimpleTester + Tester）"
	@echo "  make CONFIG=debug    : Debug ビルド"
	@echo "  make lib             : 共有ライブラリのみ"
	@echo "  make simple          : SimpleTester のみ"
	@echo "  make tester          : Tester のみ"
	@echo "  make clean           : 生成物の削除"
	@echo ""
	@echo "変数:"
	@echo "  POCO_HOME=...        : POCO/OpenSSL の配置先（include/ と lib/ が必要）"
	@echo "  POCO_LIB_DIR=...     : POCO/OpenSSL の lib ディレクトリ（必要なら上書き）"
	@echo "  EXTRA_LDFLAGS=...    : 追加のリンクフラグ"
	@echo "  EXTRA_LIBS=...       : 追加のライブラリ"

lib: $(LIB_OUT)

simple: $(SIMPLE_OUT)

tester: $(TESTER_OUT)

# ---- ビルドルール ----
$(OUT_DIR):
	@mkdir -p "$@"

$(OBJ_DIR):
	@mkdir -p "$@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p "$(dir $@)"
	$(CXX) $(LIB_CXXFLAGS) -c "$<" -o "$@"

$(LIB_OUT): $(LIB_OBJS) | $(OUT_DIR)
	$(CXX) $(SHLIB_LDFLAGS) $(SHLIB_INSTALL_NAME) -o "$@" $(LIB_OBJS) \
		-L"$(POCO_LIB_DIR)" $(POCO_LIBS) $(OPENSSL_LIBS) $(SYS_LIBS) $(EXTRA_LIBS) $(EXTRA_LDFLAGS)

$(SIMPLE_OUT): HrpSimpleTester.cpp $(LIB_OUT) | $(OUT_DIR)
	$(CXX) $(APP_CXXFLAGS) -o "$@" HrpSimpleTester.cpp \
		-L"$(OUT_DIR)" -l$(NAME) $(EXE_RPATH) $(SYS_LIBS)

$(TESTER_OUT): HrpTester.cpp $(LIB_OUT) | $(OUT_DIR)
	$(CXX) $(APP_CXXFLAGS) -o "$@" HrpTester.cpp \
		-L"$(OUT_DIR)" -l$(NAME) $(EXE_RPATH) $(SYS_LIBS)

clean:
	@# 注意: 実行環境のファイルを削除します。必要に応じて対象を調整してください。
	@rm -rf "$(OUT_DIR)" "$(OBJ_DIR)"
