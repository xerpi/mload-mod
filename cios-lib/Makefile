# devkitARM path
DEVKITARM ?=	/opt/devkitARM

# Prefix
PREFIX	=	$(DEVKITARM)/bin/arm-none-eabi-

# Executables
CC	=	$(PREFIX)gcc
AR	=	$(PREFIX)ar

# Date & Time
ifdef SOURCE_DATE_EPOCH
    BUILD_DATE ?= $(shell LC_ALL=C date -u -d "@$(SOURCE_DATE_EPOCH)" "+'%b %e %Y'" 2>/dev/null || LC_ALL=C date -u -r "$(SOURCE_DATE_EPOCH)" "+'%b %e %Y'" 2>/dev/null || LC_ALL=C date -u "+'%b %e %Y'")
    BUILD_TIME ?= $(shell LC_ALL=C date -u -d "@$(SOURCE_DATE_EPOCH)" "+'%T'" 2>/dev/null || LC_ALL=C date -u -r "$(SOURCE_DATE_EPOCH)" "+'%T'" 2>/dev/null || LC_ALL=C date -u "+'%T'")
else
    BUILD_DATE ?= $(shell LC_ALL=C date "+'%b %e %Y'")
    BUILD_TIME ?= $(shell LC_ALL=C date "+'%T'")
endif

# Flags
ARCH	=	-mcpu=arm926ej-s -mthumb -mthumb-interwork -mbig-endian
CFLAGS	=	$(ARCH) -I. -fomit-frame-pointer -Os -Wall -D__TIME__=\"$(BUILD_TIME)\" -D__DATE__=\"$(BUILD_DATE)\" -Wno-builtin-macro-redefined
CFLAGS2	=	-Wstrict-prototypes -ffunction-sections
ARFLAGS	=	rcs

# Libraries
LIBS	=

# Target
TARGET	=	cios-lib

# Objects
OBJS	=	direct_syscalls.o	\
		fat_tools.o		\
		ios.o			\
		ipc.o			\
		isfs.o			\
		log.o			\
		mem.o			\
		sdio.o			\
		stealth.o		\
		str_utils.o		\
		swi_mload.o		\
		syscalls.o		\
		syscalls_dbg.o		\
		timer.o			\
		tools.o			\
		usbstorage.o		\
		vsprintf.o
		
OBJS2	=	libwbfs/libwbfs.o	\
		libwbfs/rijndael.o	\
		libwbfs/wiidisc.o

# Dependency files
DEPS	= $(OBJS:.o=.d) $(OBJS2:.o=.d)

$(TARGET).a: $(OBJS) $(OBJS2)
	@echo -e " AR\t$@"
	@$(AR) $(ARFLAGS) $@ $(OBJS) $(OBJS2) 

%.o: %.s
	@echo -e " CC\t$@"
	@$(CC) $(CFLAGS) -MMD -MP -D_LANGUAGE_ASSEMBLY -c -x assembler-with-cpp -o $@ $<

libwbfs/%.o: libwbfs/%.c
	@echo -e " CC\t$@"
	@$(CC) $(CFLAGS) $(CFLAGS2) -MMD -MP -c -o $@ $<

%.o: %.c
	@echo -e " CC\t$@"
	@$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

.PHONY: clean

clean:
	@echo -e "Cleaning..."
	@rm -f $(OBJS) $(DEPS) $(OBJS2) $(TARGET).a

-include $(DEPS)
