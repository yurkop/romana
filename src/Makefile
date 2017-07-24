# make DEBUG=1 to compile with debug option
SRC_D = src
OBJ_D = obj

#LIB_D = ../libcrs

SRC := $(wildcard $(SRC_D)/*.cpp)
OBJ := $(addprefix $(OBJ_D)/,$(notdir $(SRC:.cpp=.o)))
#SRCL := $(wildcard $(LIB_D)/*.cpp)
#OBJL := $(addprefix $(OBJ_D)/,$(notdir $(SRCL:.cpp=.oo)))

LFS=$(shell getconf LFS_CFLAGS)

ifdef DEBUG
  CPPFLAGS += -g
  CXXFLAGS += -O1 -Wall
else
  CXXFLAGS += -O3 -Wall
endif

CPPFLAGS += $(LFS) -MD -MP -D LINUX=1 -I. #-I$(LIB_D)
LDFLAGS += -lz -l cyusb -l usb-1.0
#SVNVER=\"$(shell svnversion)\"

PROG=romana

ifdef ROOTSYS
#  CPPFLAGS += -I$(shell $(ROOTSYS)/bin/root-config --incdir)
  CPPFLAGS += $(shell  $(ROOTSYS)/bin/root-config --cflags)
  LDFLAGS   += $(shell  $(ROOTSYS)/bin/root-config --glibs) # -lXMLIO
  LDFLAGS   += -lSpectrum
endif

all: $(OBJ_D) $(PROG).x
#all: svnver $(PROG).x

#$(PROG).x: $(OBJ) $(OBJL) $(OBJ_D)/romdict.o
$(PROG).x: $(OBJ) $(OBJ_D)/romdict.o
	g++ -o $@ $^ $(LDFLAGS)

$(OBJ_D)/%.o: $(SRC_D)/%.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

#$(OBJ_D)/%.oo: $(LIB_D)/%.cpp
#	g++ $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

#svnver: Makefile
#	echo \#define SVNVERSION $(SVNVER) >$(OBJ_D)/svn.h

$(OBJ_D)/romdict.cpp: src/LinkDef.h $(SRC_D)/*.h #$(LIB_D)/*.h
	rootcint -f $(OBJ_D)/romdict.cpp -c -p $(SRC_D)/$(PROG).h $(SRC_D)/LinkDef.h

$(OBJ_D)/romdict.o: $(OBJ_D)/romdict.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJ_D):
	mkdir -p $(OBJ_D)

r2a: utils/root2ascii.cxx
	g++ $(CXXFLAGS) $(CPPFLAGS) -o r2a.x utils/root2ascii.cxx $(LDFLAGS)

r2a2: utils/root2ascii2.cxx
	g++ $(CXXFLAGS) $(CPPFLAGS) -o r2a2.x utils/root2ascii2.cxx $(LDFLAGS)

$(OBJ_D)/headerdict.cpp: utils/LinkDef.h
	rootcint -f $(OBJ_D)/headerdict.cpp -c -p utils/gz_header.h utils/LinkDef.h

gz_header: utils/gz_header.cpp $(OBJ_D)/headerdict.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -o gz_header.x utils/gz_header.cpp $(OBJ_D)/headerdict.cpp $(LDFLAGS)

clean:
	rm -f $(OBJ_D)/* $(SRC_D)/*~

cleanx: clean
	rm -f *.x

-include $(OBJ:.o=.d)
