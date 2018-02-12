# make DEBUG=1 to compile with debug option
SRC_D = .
OBJ_D = ../obj

#LIB_D = ../libcrs

SRC := $(wildcard $(SRC_D)/*.cpp)
OBJ := $(addprefix $(OBJ_D)/,$(notdir $(SRC:.cpp=.o)))
#SRCL := $(wildcard $(LIB_D)/*.cpp)
#OBJL := $(addprefix $(OBJ_D)/,$(notdir $(SRCL:.cpp=.oo)))

LFS=$(shell getconf LFS_CFLAGS)

ifdef DEBUG
  CPPFLAGS += -g
endif

CPPFLAGS += $(LFS) -MD -MP -D LINUX=1 -I. #-I$(LIB_D)
CXXFLAGS += -O3 -Wall
#LDFLAGS += -lz

PROG=decoder

ifdef ROOTSYS
#  CPPFLAGS += -I$(shell $(ROOTSYS)/bin/root-config --incdir)
  CPPFLAGS += $(shell  $(ROOTSYS)/bin/root-config --cflags)
  LDFLAGS   += $(shell  $(ROOTSYS)/bin/root-config --glibs) # -lXMLIO
  LDFLAGS   += -lSpectrum
endif

all: $(OBJ_D) ../$(PROG).x
#all: svnver $(PROG).x

#$(PROG).x: $(OBJ) $(OBJL) $(OBJ_D)/romdict.o
../$(PROG).x: $(OBJ)
	g++ -o $@ $^ $(LDFLAGS)

$(OBJ_D)/%.o: $(SRC_D)/%.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJ_D):
	mkdir -p $(OBJ_D)

clean:
	rm -f $(OBJ_D)/* $(SRC_D)/*~

cleanx: clean
	rm -f *.x

-include $(OBJ:.o=.d)
