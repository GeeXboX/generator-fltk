PROJ=generator

SRCS_CXX=audio.cxx compile.cxx curl.cxx dvdnav.cxx fs.cxx generator.cxx language.cxx network.cxx packages.cxx recorder.cxx remote.cxx theme.cxx system.cxx
SRCS_C=utils.c
SRCS_CXX+=Fl_Gel_Tabs/Fl_Gel_Tabs.cxx 
SRCS_CXX+=FLU/Flu_Tree_Browser.cxx FLU/FluSimpleString.cxx FLU/flu_pixmaps.cxx
SRCS_C+=libmd/md5c.c libmd/md5hl.c
SRCS_C+=libbz2/bzlib.c libbz2/crctable.c libbz2/decompress.c libbz2/huffman.c libbz2/randtable.c
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
INCFLAGS+=-Ilibbz2

CC?=gcc
CXX?=g++
STRIP?=strip
LDFLAGS+=-static
LDFLAGS+=$(FLTKLDFLAGS)
LDFLAGS+=$(CURLLDFLAGS)
CXXFLAGS+=-Wall -Werror $(INCFLAGS)
CXXFLAGS+=$(FLTKCXXFLAGS)
CXXFLAGS+=$(CURLCFLAGS)
CXXFLAGS+=-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_LARGE_FILES -D_FILE_OFFSET_BITS=64
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
	$(CC) $(CXXFLAGS) -c $< -o $@

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
