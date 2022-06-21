PRJ = Wrp

CPPC = g++ -std=c++11
LD = g++
LDD = ldd -d

POCO_HOME = ../../../_/_poco

SRC = \
	com/amivoice/wrp/Wrp.cpp \
	com/amivoice/wrp/Wrp_.cpp \
	com/amivoice/wrp/Wrp__.cpp

LIB = \
	-lPocoNet \
	-lPocoNetSSL \
	-lPocoCrypto \
	-lPocoUtil \
	-lPocoJSON \
	-lPocoXML \
	-lPocoEncodings \
	-lPocoFoundation \
	-lpthread \
	-lssl \
	-lcrypto

CPPDEFINES = \
	-DLINUX \
	-DPOSIX \
	-D_USRDLL \
	-D$(if $(debug),_DEBUG,NDEBUG)

CPPFLAGS = \
	-fPIC \
	-w \
	-O$(if $(debug),0 -g,3) \
	-ffunction-sections \
	-I. \
	-I$(POCO_HOME)/include

LDFLAGS = \
	-shared \
	-fPIC \
	-L$(POCO_HOME)/lib/linux64

OUTDIR = ../bin/linux64$(if $(debug),_debug,_release)
OBJDIR = obj/linux64_dynamic$(if $(debug),_debug,_release)
	
OUT = $(OUTDIR)/lib$(PRJ).so
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(notdir $(SRC)))
DEP = $(patsubst %.cpp,$(OBJDIR)/%.d,$(notdir $(SRC)))

VPATH = $(sort $(dir $(SRC)))

build: $(OUT)

$(OUT): $(OBJ)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(OBJ) $(LIB) -o $@
	$(LDD) $@ > $@.ldd

$(OBJ): $(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CPPC) $(CPPFLAGS) $(CPPDEFINES) -c -MMD $< -o $@

clean:
	-rm -f $(OUT) $(OUT).ldd $(OBJ) $(DEP)

-include $(DEP)
