PROJ=generator
OBJS=Fl_Gel_Tabs/Fl_Gel_Tabs.o audio.o compile.o fs.o generator.o language.o network.o remote.o theme.o system.o utils.o
FLOBJS=generatorUI.fl

FLTKCONFIG?=fltk-config
FLUID?=fluid
FLTKCXXFLAGS?=$(shell $(FLTKCONFIG) --cxxflags)
FLTKLDFLAGS?=$(shell $(FLTKCONFIG) --ldstaticflags)

SED?=sed
CXX?=g++
STRIP?=strip
LDFLAGS+=$(FLTKLDFLAGS) -static
CXXFLAGS+=-Wall -Werror -pedantic -IFl_Gel_Tabs $(FLTKCXXFLAGS)
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
	makedepend -Y -f Dependencies -- $(PROGOBJS:.o=.cxx)

include Dependencies
