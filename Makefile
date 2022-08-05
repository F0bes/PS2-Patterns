EE_OBJS	= patterns.o fontx_improved.o vmode.o pattern_funcs.o
#EE_OPTFLAGS =
EE_BIN = patterns.elf
EE_LIBS = -lkernel -ldebug -lpacket -ldraw -lgraph -ldma -laudsrv -lpatches -lpad
EE_CXXFLAGS= -Werror
EE_DVP = dvp-as
EE_VCL = vcl

all: $(EE_BIN)

%.o: %.vsm
	$(EE_DVP) $< -o $@

clean:
	rm -f $(EE_BIN) $(EE_OBJS) 

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

wsl: $(EE_BIN)
	$(PCSX2) --elf="$(shell wslpath -w $(shell pwd))/$(EE_BIN)"

emu: $(EE_BIN)
	$(PCSX2) --elf="$(shell pwd)/$(EE_BIN)"

reset:
	ps2client reset
	ps2client netdump

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
