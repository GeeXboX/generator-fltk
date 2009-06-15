PROJ=generator

SRCS_CXX=audio.cxx autoplay.cxx compile.cxx curl.cxx dvdnav.cxx extrafiles.cxx fs.cxx generator.cxx isolinux.cxx keymap.cxx language.cxx lcd.cxx ndiswrapper.cxx network.cxx nfs.cxx packages.cxx remote.cxx samba.cxx theme.cxx system.cxx utils.cxx video.cxx
SRCS_C=configparser.c
SRCS_CXX+=Fl_Gel_Tabs/Fl_Gel_Tabs.cxx 
SRCS_CXX+=FLU/Flu_Tree_Browser.cxx FLU/FluSimpleString.cxx FLU/flu_pixmaps.cxx
SRCS_C+=libmd/md5c.c libmd/md5hl.c
SRCS_C+=libbz2/bzlib.c libbz2/crctable.c libbz2/decompress.c libbz2/huffman.c libbz2/randtable.c
SRCS_RC=icon.rc
FLSRCS=generatorUI.fl

FLTKCONFIG?=fltk-config
FLUID?=fluid
FLTKCXXFLAGS?=$(shell $(FLTKCONFIG) --cxxflags)
FLTKLDFLAGS?=$(shell $(FLTKCONFIG) --ldflags) -lXpm

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
#LDFLAGS+=-static
LDFLAGS+=$(FLTKLDFLAGS)
LDFLAGS+=$(CURLLDFLAGS)
CXXFLAGS+=-Wall -Wno-strict-aliasing -Werror $(INCFLAGS)
CXXFLAGS+=$(FLTKCXXFLAGS)
CXXFLAGS+=$(CURLCFLAGS)
#CXXFLAGS+=-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_LARGE_FILES -D_FILE_OFFSET_BITS=64
EXEEXT?=

PROGSRCS=$(SRCS_CXX) $(SRCS_C) $(FLSRCS:.fl=.cxx) $(SRCS_RC)
PROGOBJS=$(SRCS_CXX:.cxx=.o) $(SRCS_C:.c=.o) $(FLSRCS:.fl=.o)

all: $(PROJ)$(EXEEXT)

ifeq ($(PROJ),generator.exe)
PROGOBJS+= ${SRCS_RC:.rc=.o}
endif

$(PROJ)$(EXEEXT): $(PROGOBJS)
	$(CXX) $(CXXFLAGS) $(PROGOBJS) -o $@ $(LDFLAGS)
	$(STRIP) $@

.SUFFIXES: .fl
.SUFFIXES: .c
.SUFFIXES: .cxx
.SUFFIXES: .rc

%.cxx %.h: %.fl
	$(FLUID) -c $<

%.o: %.c
	$(CC) $(CXXFLAGS) -c $< -o $@

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.rc
	${RC} $< $@

.PHONY: clean
clean:
	rm -f $(PROJ)$(EXEEXT) $(PROGOBJS) ${SRCS_RC:.rc=.o}

.PHONY: distclean
distclean: clean
	rm -f $(FLSRCS:.fl=.cxx) $(FLSRCS:.fl=.h)

.PHONY: depend
depend: $(PROGSRCS)
	makedepend -Y -f Dependencies -- $(INCFLAGS) -- $(PROGSRCS)

include Dependencies
