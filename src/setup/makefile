# EPOS Setup Makefile

include ../../makedefs

TARGET := $(if $(shell find setup_$(MMOD)* 2> /dev/null), $(if $(SETUP_ADDR), install, install.o))

all:	$(TARGET)
#!P2: Switch to mode-based ruling
setup_$(MMOD):	setup_$(MMOD).o
		$(LD) $(LDFLAGS) -L$(CCLIB) --trace --omagic --section-start .init=$(SETUP_ADDR) -o $@ $^ -l$(LINIT) -l$(LMACH) -l$(LARCH) -l$(LUTIL) -lgcc

install:	setup_$(MMOD)
		$(STRIP) $< -o $<_striped
		mv $<_striped $(IMG)/$<

install.o:	setup_$(MMOD).o
		$(INSTALL) $< $(LIB)

clean:
		$(CLEAN) *.o $(shell find -maxdepth 1 -executable -not -type d)
