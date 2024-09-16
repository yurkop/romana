# commands:
# make : compile
# make -j3 : compile using 3 CPUs
# make clean : clear compilation
# make cyusblib : compile and install cyusb library (need root password)
# make r2a : compile r2a (root2ascii)

# make TPROC=1 to compile with TPROC option
#      TPROC -> cpu usage in FillHist, only in singlethread
# make TIMES=1 to compile with TIMES option
# make DEBUG=1 to compile with debug option
# make PROF=1 to compile with profiling option
# make P_LIBUSB=1 to compile with printing libusb messages
# make P_CMD=1 to compile with printing cmd32 & cmd2 messages
# make BITS=N to compile with cutting lower bits in sData by N
# removed!!!   make simul to compile with SIMUL option 


GIT_VERSION := $(shell git describe --abbrev=4 --always --tags --dirty)
SRC_D = src
OBJ_D = obj

#LIB_D = ../libcrs

HELP := \"$(abspath $(SRC_D)/help.pdf)\"
MACRO := \"$(abspath $(PWD)/Macro/)\"
CPPFLAGS += -DHELP=$(HELP) -DMACRO=$(MACRO)

SRC := $(wildcard $(SRC_D)/*.cpp)
OBJ := $(addprefix $(OBJ_D)/,$(notdir $(SRC:.cpp=.o)))
#SRCL := $(wildcard $(LIB_D)/*.cpp)
#OBJL := $(addprefix $(OBJ_D)/,$(notdir $(SRCL:.cpp=.oo)))

CPPFLAGS += -DGITVERSION=\"$(GIT_VERSION)\"

LFS=$(shell getconf LFS_CFLAGS)

ifdef PROF
  CPPFLAGS += -pg
  LDFLAGS += -pg
endif

ifdef DEBUG
  CPPFLAGS += -g
  #CPPFLAGS += -D_GLIBCXX_DEBUG
  #CPPFLAGS += -fsanitize=address
endif

ifdef TPROC
  CPPFLAGS += -D TPROC=1
endif

ifdef TIMES
  CPPFLAGS += -D TIMES=1
endif

ifdef P_LIBUSB
  CPPFLAGS += -D P_LIBUSB=1
endif

ifdef P_CMD
  CPPFLAGS += -D P_CMD=1
endif

ifdef BITS
  CPPFLAGS += -D BITS=$(BITS)
endif

ifeq (yumo,$(findstring $(MAKECMDGOALS),yumo))
  CPPFLAGS += -D YUMO=1
endif

#ifeq (simul,$(findstring $(MAKECMDGOALS),simul))
#  CPPFLAGS += -D SIMUL=1
#endif

CPPFLAGS += $(LFS) -MD -MP -DLINUX=1 -I. #-I$(LIB_D)
CXXFLAGS += -O3 -Wall #-Wno-maybe-uninitialized #-Wextra
LDFLAGS += -lz
#SVNVER=\"$(shell svnversion)\"

ifneq ("$(wildcard /usr/local/lib/libcyusb.so)","")
CYUSB_LIB = 1
endif

ifdef CYUSB_LIB
CPPFLAGS += -DCYUSB=1
LDFLAGS += -l cyusb -l usb-1.0
endif

PROG=romana

ifdef ROOTSYS
#  CPPFLAGS += -I$(shell $(ROOTSYS)/bin/root-config --incdir)
  CPPFLAGS += $(shell  $(ROOTSYS)/bin/root-config --cflags)
  LDFLAGS   += $(shell  $(ROOTSYS)/bin/root-config --glibs) # -lXMLIO
  LDFLAGS   += -lSpectrum

  VER = $(shell root-config --version)
  VER2 = $(findstring 5.,$(VER))
endif

#yumo1:
#	echo $(MAKECMDGOALS) $(YM)

all: $(OBJ_D) $(PROG).x
#all: svnver $(PROG).x

yumo: all

#simul: all

#$(PROG).x: $(OBJ) $(OBJL) $(OBJ_D)/romdict.o
$(PROG).x: $(OBJ) $(OBJ_D)/romdict.o
	g++ -o $@ $^ $(LDFLAGS)

$(OBJ_D)/%.o: $(SRC_D)/%.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

#$(OBJ_D)/%.oo: $(LIB_D)/%.cpp
#	g++ $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

#svnver: Makefile
#	echo \#define SVNVERSION $(SVNVER) >$(OBJ_D)/svn.h


ifeq ($(VER2),5.) # root 5
$(OBJ_D)/romdict.cpp: $(SRC_D)/LinkDef.h $(SRC_D)/*.h Makefile #$(LIB_D)/*.h
	rootcint -f $(OBJ_D)/romdict.cpp -c -p $(SRC_D)/$(PROG).h $(SRC_D)/popframe.h $(SRC_D)/LinkDef.h
else # root 6+
$(OBJ_D)/romdict.cpp: $(SRC_D)/LinkDef.h $(SRC_D)/*.h Makefile #$(LIB_D)/*.h
	rootcling -f $(OBJ_D)/romdict.cpp -s romana.pcm $(SRC_D)/$(PROG).h $(SRC_D)/popframe.h $(SRC_D)/LinkDef.h
endif

$(OBJ_D)/romdict.o: $(OBJ_D)/romdict.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJ_D):
	mkdir -p $(OBJ_D)

dec34: utils/dec34.cpp $(SRC_D)/toptions.cpp (SRC_D)c/toptions.h
	g++ $(CXXFLAGS) $(CPPFLAGS) -o dec34.x utils/dec34.cpp $(SRC_D)/toptions.cpp $(LDFLAGS)

r2a: utils/root2ascii.cxx
	g++ $(CXXFLAGS) $(CPPFLAGS) -o r2a.x utils/root2ascii.cxx $(LDFLAGS)

r2a2: utils/root2ascii2.cxx
	g++ $(CXXFLAGS) $(CPPFLAGS) -o r2a2.x utils/root2ascii2.cxx $(LDFLAGS)

$(OBJ_D)/headerdict.cpp: utils/LinkDef.h
	rootcint -f $(OBJ_D)/headerdict.cpp -c -p utils/gz_header.h utils/LinkDef.h

gz_header: utils/gz_header.cpp $(OBJ_D)/headerdict.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -o gz_header.x utils/gz_header.cpp $(OBJ_D)/headerdict.cpp $(LDFLAGS)

ifeq ($(VER2),5.) # root 5
$(OBJ_D)/paramdict.cpp: utils/param.h utils/LinkDef.h
	rootcint -f $(OBJ_D)/paramdict.cpp -c -p utils/param.h utils/LinkDef.h
else # root 6+
$(OBJ_D)/paramdict.cpp: utils/param.h utils/LinkDef.h
	rootcling -f $(OBJ_D)/paramdict.cpp -s param.pcm -p utils/param.h utils/LinkDef.h
endif

param: $(OBJ_D)/paramdict.cpp utils/param.cpp $(SRC_D)/toptions.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -o param.x utils/param.cpp $(OBJ_D)/paramdict.cpp $(SRC_D)/toptions.cpp $(LDFLAGS)

cyusblib:
	cd cyusb && sudo ./install.sh

install:
	rm -f $(HOME)/bin/romana
	rm -f $(HOME)/bin/romana_rdict.pcm
	rm -f $(HOME)/bin/romana.x

	ln -s $(PWD)/romana.x $(HOME)/bin/romana
	ln -s $(PWD)/romana.x $(HOME)/bin/romana.x
	ln -s $(PWD)/romana_rdict.pcm $(HOME)/bin

clean:
	rm -f $(OBJ_D)/* $(SRC_D)/*~

cleanx: clean
	rm -f *.x

test_target:
	@echo $(CYUSB_LIB)
ifdef CYUSB_LIB
	echo "Do something"
else
	echo "Do nothing"
endif

-include $(OBJ:.o=.d)
