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
LDFLAGS+=$(FLTKLDFLAGS)
CXXFLAGS+=-Wall -Werror -pedantic -IFl_Gel_Tabs $(FLTKCXXFLAGS)
EXEEXT?=

PROGOBJS=$(OBJS) $(FLOBJS:.fl=.o)

all: $(PROJ)$(EXEEXT)

$(PROJ)$(EXEEXT): $(PROGOBJS)
	$(CXX) $(CXXFLAGS) $(PROGOBJS) -o $@ $(LDFLAGS)
	$(STRIP) $@

.SUFFIXES: .fl
.SUFFIXES: .cxx

%.cxx %.h: %.fl
	$(FLUID) -c $<
	$(SED) 's%Fl_Tabs%Fl_Gel_Tabs%g' $(<:.fl=.cxx) > $(<:.fl=.cxx).new
	$(SED) 's%Fl_Tabs%Fl_Gel_Tabs%g' $(<:.fl=.h) > $(<:.fl=.h).new
	mv $(<:.fl=.cxx).new $(<:.fl=.cxx)
	mv $(<:.fl=.h).new $(<:.fl=.h)

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(PROJ)$(EXEEXT) $(PROGOBJS) $(FLOBJS:.fl=.cxx) $(FLOBJS:.fl=.h)

.PHONY: depend
depend: $(PROGOBJS:.o=.cxx)
	makedepend -Y -f Dependencies -- $(PROGOBJS:.o=.cxx)

include Dependencies
