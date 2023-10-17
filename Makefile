# devkitARM path
DEVKITARM ?=	/opt/devkitARM

# Prefix
PREFIX	=	$(DEVKITARM)/bin/arm-none-eabi-

# Executables
CC	=	$(PREFIX)gcc
LD	=	$(PREFIX)gcc
STRIP	=	stripios

# Strip address
SADDR	=	0x13700000

# Date & Time
ifdef SOURCE_DATE_EPOCH
    BUILD_DATE ?= $(shell LC_ALL=C date -u -d "@$(SOURCE_DATE_EPOCH)" "+'%b %e %Y'" 2>/dev/null || LC_ALL=C date -u -r "$(SOURCE_DATE_EPOCH)" "+'%b %e %Y'" 2>/dev/null || LC_ALL=C date -u "+'%b %e %Y'")
    BUILD_TIME ?= $(shell LC_ALL=C date -u -d "@$(SOURCE_DATE_EPOCH)" "+'%T'" 2>/dev/null || LC_ALL=C date -u -r "$(SOURCE_DATE_EPOCH)" "+'%T'" 2>/dev/null || LC_ALL=C date -u "+'%T'")
else
    BUILD_DATE ?= $(shell LC_ALL=C date "+'%b %e %Y'")
    BUILD_TIME ?= $(shell LC_ALL=C date "+'%T'")
endif

ifdef D2XL_VER_COMPILE
   D2XL_VER ?= $(D2XL_VER_COMPILE)
else
   D2XL_VER ?= "unknown"
endif

# Flags
ARCH	=	-mcpu=arm926ej-s -mthumb -mthumb-interwork -mbig-endian
CFLAGS	=	$(ARCH) -iquote ./include -iquote cios-lib -fomit-frame-pointer -Os -Wall -D__TIME__=\"$(BUILD_TIME)\" -D__DATE__=\"$(BUILD_DATE)\" -D__D2XL_VER__=\"$(D2XL_VER)\" -Wno-builtin-macro-redefined
LDFLAGS	=	$(ARCH) -nostartfiles -Wl,-T,link.ld,-Map,$(TARGET).map -Wl,--gc-sections -Wl,-static

# Libraries
LIBS	=	cios-lib/cios-lib.a

# Target
TARGET	=	MLOAD

# Objects
OBJS	=	source/debug.o		\
		source/detect.o	\
		source/elf.o		\
		source/gecko.o		\
		source/gpio.o		\
		source/main.o		\
		source/patches.o	\
		source/start.o		\
		source/swi.o		\
		source/swi_asm.o	\
		source/tid.o

# Dependency files
DEPS	= $(OBJS:.o=.d)

$(TARGET).app: $(TARGET).elf
	@echo -e " STRIP\t$@"
	@$(STRIP) $< $@

$(TARGET).elf: $(OBJS) $(LIBS) link.ld
	@echo -e " LD\t$@"
	@$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -lgcc -o $@

%.o: %.s
	@echo -e " CC\t$@"
	@$(CC) $(CFLAGS) -MMD -MP -D_LANGUAGE_ASSEMBLY -c -x assembler-with-cpp -o $@ $<

%.o: %.c
	@echo -e " CC\t$@"
	@$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

cios-lib/cios-lib.a:
	@$(MAKE) -C cios-lib

.PHONY: clean

clean:
	@echo -e "Cleaning..."
	@rm -f $(OBJS) $(DEPS) $(TARGET).app $(TARGET).elf $(TARGET).map
	@$(MAKE) -C cios-lib clean

-include $(DEPS)
