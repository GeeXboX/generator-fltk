PROJ=generator
OBJS=audio.o compile.o fs.o generator.o language.o network.o remote.o theme.o system.o utils.o
OBJS+=Fl_Gel_Tabs/Fl_Gel_Tabs.o 
OBJS+=FLU/Flu_Tree_Browser.o FLU/FluSimpleString.o FLU/flu_pixmaps.o
OBJS+=libmd/md5c.o libmd/md5hl.o
FLOBJS=generatorUI.fl

FLTKCONFIG?=fltk-config
FLUID?=fluid
FLTKCXXFLAGS?=$(shell $(FLTKCONFIG) --cxxflags)
FLTKLDFLAGS?=$(shell $(FLTKCONFIG) --ldstaticflags)

INCFLAGS+=-IFl_Gel_Tabs
INCFLAGS+=-IFLU
INCFLAGS+=-Ilibmd

CXX?=g++
STRIP?=strip
LDFLAGS+=-static
LDFLAGS+=$(FLTKLDFLAGS)
CXXFLAGS+=-Wall -Werror -pedantic $(INCFLAGS)
CXXFLAGS+=$(FLTKCXXFLAGS)
EXEEXT?=

PROGOBJS=$(OBJS) $(FLOBJS:.fl=.o)

all: $(PROJ)$(EXEEXT)

$(PROJ)$(EXEEXT): $(PROGOBJS)
	$(CXX) $(CXXFLAGS) $(PROGOBJS) -o $@ $(LDFLAGS)
	$(STRIP) $@
	$(FLTKCONFIG) --post $@

.SUFFIXES: .fl
.SUFFIXES: .cxx

%.cxx %.h: %.fl
	$(FLUID) -c $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(PROJ)$(EXEEXT) $(PROGOBJS)

.PHONY: distclean
distclean: clean
	rm -f $(FLOBJS:.fl=.cxx) $(FLOBJS:.fl=.h)

.PHONY: depend
depend: $(PROGOBJS:.o=.cxx)
	makedepend -Y -f Dependencies $(INCFLAGS) -- $(PROGOBJS:.o=.cxx)

include Dependencies
