PROJ=generator

SRCS_CXX=audio.cxx compile.cxx curl.cxx fs.cxx generator.cxx language.cxx network.cxx packages.cxx remote.cxx theme.cxx system.cxx
SRCS_C=utils.c
SRCS_CXX+=Fl_Gel_Tabs/Fl_Gel_Tabs.cxx 
SRCS_CXX+=FLU/Flu_Tree_Browser.cxx FLU/FluSimpleString.cxx FLU/flu_pixmaps.cxx
SRCS_C+=libmd/md5c.c libmd/md5hl.c
FLSRCS=generatorUI.fl

FLTKCONFIG?=fltk-config
FLUID?=fluid
FLTKCXXFLAGS?=$(shell $(FLTKCONFIG) --cxxflags)
FLTKLDFLAGS?=$(shell $(FLTKCONFIG) --ldstaticflags)

CURLCONFIG?=curl-config
CURLCFLAGS?=$(shell $(CURLCONFIG) --cflags) -DCURL_STATICLIB
CURLLDFLAGS?=$(shell $(CURLCONFIG) --libs)

INCFLAGS+=-IFl_Gel_Tabs
INCFLAGS+=-IFLU
INCFLAGS+=-Ilibmd

CXX?=g++
STRIP?=strip
LDFLAGS+=-static
LDFLAGS+=$(FLTKLDFLAGS)
LDFLAGS+=$(CURLLDFLAGS)
CXXFLAGS+=-Wall -Werror $(INCFLAGS)
CXXFLAGS+=$(FLTKCXXFLAGS)
CXXFLAGS+=$(CURLCFLAGS)
EXEEXT?=

PROGSRCS=$(SRCS_CXX) $(SRCS_C) $(FLSRCS:.fl=.cxx)
PROGOBJS=$(SRCS_CXX:.cxx=.o) $(SRCS_C:.c=.o) $(FLSRCS:.fl=.o)

all: $(PROJ)$(EXEEXT)

$(PROJ)$(EXEEXT): $(PROGOBJS)
	$(CXX) $(CXXFLAGS) $(PROGOBJS) -o $@ $(LDFLAGS)
	$(STRIP) $@
	$(FLTKCONFIG) --post $@

.SUFFIXES: .fl
.SUFFIXES: .c
.SUFFIXES: .cxx

%.cxx %.h: %.fl
	$(FLUID) -c $<

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(PROJ)$(EXEEXT) $(PROGOBJS)

.PHONY: distclean
distclean: clean
	rm -f $(FLSRCS:.fl=.cxx) $(FLSRCS:.fl=.h)

.PHONY: depend
depend: $(PROGSRCS)
	makedepend -Y -f Dependencies $(INCFLAGS) -- $(PROGSRCS)

include Dependencies
