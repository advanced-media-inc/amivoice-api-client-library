PRJ = HrpTester

CPPC = g++ -std=c++11
LD = g++
LDD = ldd -d

SRC = \
	$(PRJ).cpp

LIB = \
	-lHrp

CPPDEFINES = \
	-DLINUX \
	-DPOSIX \
	-D$(if $(debug),_DEBUG,NDEBUG)

CPPFLAGS = \
	-w \
	-O$(if $(debug),0 -g,3) \
	-Isrc

LDFLAGS = \
	-L$(OUTDIR)

OUTDIR = bin/linux64$(if $(debug),_debug,_release)
OBJDIR = obj/linux64$(if $(debug),_debug,_release)/$(PRJ)
	
OUT = $(OUTDIR)/$(PRJ)
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(notdir $(SRC)))
DEP = $(patsubst %.cpp,$(OBJDIR)/%.d,$(notdir $(SRC)))

VPATH = $(sort $(dir $(SRC)))

build: $(OUT)

$(OUT): $(OBJ)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(OBJ) $(LIB) -o $@

$(OBJ): $(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CPPC) $(CPPFLAGS) $(CPPDEFINES) -c -MMD $< -o $@

clean:
	-rm -f $(OUT) $(OBJ) $(DEP)

-include $(DEP)
