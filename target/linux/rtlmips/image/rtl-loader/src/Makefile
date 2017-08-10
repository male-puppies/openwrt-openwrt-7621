CROSS_COMPILE ?= 
TARGET_DIR ?= $(LINUX_DIR)
KERNEL_ROOT := $(LINUX_DIR)

LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip
AS=$(CROSS_COMPILE)as
NM=$(CROSS_COMPILE)nm
OBJCOPY=$(CROSS_COMPILE)objcopy

GZIP_COMPRESS=yes

LOAD_START_ADDR ?= 0x80C00000
FLASH_OFFSET ?= 40000

CV_OPTION=linux-ro
CV_SIGNATURE=

SEDFLAGS = s/LOAD_ADDR/$(LOAD_START_ADDR)/;

CVIMG=$(STAGING_DIR_HOST)/bin/cvimg
LZMA=$(STAGING_DIR_HOST)/bin/lzma

LDSCRIPT = ld.script
COPTIONS = -DROM_MEMORY -DCOMPRESSED_KERNEL -D__KERNEL__
LDFLAGS = -static -nostdlib

CFLAGS = -Os -g -fno-pic -mno-abicalls
CFLAGS += -DEMBEDDED -I$(KERNEL_ROOT)/include/linux -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/lib $(COPTIONS) -G 0
CFLAGS += -I$(KERNEL_ROOT)/arch/mips/include -I$(KERNEL_ROOT)/arch/mips/include/asm/mach-generic 
ASFLAGS = -g -fno-pic -mno-abicalls -DEMBEDDED -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/arch/mips/include

ifdef BZ2_COMPRESS
CFLAGS += -DBZ2_COMPRESS
ASFLAGS += -DBZ2_COMPRESS
else
ifndef GZIP_COMPRESS
CFLAGS += -DLZMA_COMPRESS
ASFLAGS += -DLZMA_COMPRESS
endif
endif

SOURCES = cache.c
ifdef BZ2_COMPRESS
	SOURCES += misc.c hfload.c start.S
else
ifdef GZIP_COMPRESS
	SOURCES += string.c ctype.c misc.c hfload.c start.S read_memory.c 
else
	SOURCES += string.c ctype.c misc.c hfload.c start.S read_memory.c LzmaDecode.c
endif	
endif

SUPPORT_FILES = cache.o
ifdef BZ2_COMPRESS
	SUPPORT_FILES += misc.o 
else
ifdef GZIP_COMPRESS
	SUPPORT_FILES += vsprintf.o prom_printf.o string.o ctype.o misc.o 
else
    SUPPORT_FILES += vsprintf.o prom_printf.o string.o ctype.o misc.o LzmaDecode.o  
endif	
endif

LOADER_FILES = hfload.o read_memory.o
START_FILE = start.o

# .SUFFIXES : .c .o .h
.NOTPARALLEL : all
.PHONY: rtk-vmlinux_img all clean

%.o: .depend %.c %.h %.S
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

all: clean rtk-vmlinux_img $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES)
	$(CC) -fno-pic -mno-abicalls -Os -fomit-frame-pointer -D__KERNEL__ -c vmlinux_img.c -o vmlinux_img.o	
	$(OBJCOPY) --add-section .vmlinux=vmlinux_img.gz vmlinux_img.o
	sed "$(SEDFLAGS)" < ld.script.in > $(LDSCRIPT)
	$(LD) $(LDFLAGS) -G 0 -T $(LDSCRIPT) -o memload-partial $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES) vmlinux_img.o
	$(NM) memload-partial | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | sort > system.map
	
	cp memload-partial memload-full
	$(OBJCOPY) -Obinary memload-full nfjrom
	$(CVIMG) $(CV_OPTION) nfjrom linux.bin $(LOAD_START_ADDR) $(FLASH_OFFSET) $(CV_SIGNATURE)
#	$(CVIMG) size_chk vmlinux_img $(LOAD_START_ADDR) || exit $$?

rtk-vmlinux_img:
	cp $(TARGET_DIR)/vmlinux.debug vmlinux-stripped
	$(STRIP) vmlinux-stripped $(STRIP-OPTIONS-y)
	$(OBJCOPY) -Obinary vmlinux-stripped vmlinux_img
	rm -f vmlinux_img.gz
ifdef BZ2_COMPRESS
	cat vmlinux_img | bzip2 -9v > vmlinux_img.gz
else
ifdef GZIP_COMPRESS
	cat vmlinux_img | gzip -9v > vmlinux_img.gz	
else
	@echo "default lzma..."
	$(LZMA) e vmlinux_img vmlinux_img.gz
endif
endif

clean:
	rm -f *.o linux.bin nfjrom memload-full system.map memload-partial vmlinux_img vmlinux-stripped vmlinux_img.gz .depend ld.script

.depend:
	$(CC) $(CFLAGS) -MM $(SOURCES) >.depend
