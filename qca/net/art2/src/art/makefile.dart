OUTPUT_DIR=obj

CFLAGS += -Wno-pointer-arith -Wno-extra -Wno-implicit

CFLAGS += -I$(ART_ROOT)/art \
	-I$(ART_ROOT)/common \
	-I$(ART_ROOT)/client \
	-I$(ART_ROOT)/shared \
	-I$(ART_ROOT)/devlib \
	-I$(ART_ROOT)/devlib/ar5416 \
	-I$(ART_ROOT)/devlib/ar9287 \
	-I$(ART_ROOT)/wlan/hal \
	-I$(ART_ROOT)/wlan/include \
	-I$(ART_ROOT)/wlan/hal/ar5416 \
	-I$(ART_ROOT)/wlan/hal/nartlinux \
    -I$(ART_ROOT)/driver/linux/modules/include \
    -I$(ART_ROOT)/HALdll_proj/include \
    -I$(ART_ROOT)/calibration/cal-2p



VPATH = $(ART_ROOT)/art \
	$(ART_ROOT)/common \
	$(ART_ROOT)/devlib \
    $(ART_ROOT)/HALdll_proj/common \

SRCS = 	CalibrationLoad.c \
	Calibrate.c \
	Card.c \
	Channel.c \
	ChipIdentify.c \
	configCmd.c \
	ConfigurationCommand.c \
	DeviceLoad.c \
    DllIfOs_linux.c \
	GainTable.c \
	Interpolate.c \
	Link.c \
	nart.c \
	NartRegister.c \
	NewArt.c \
	NoiseFloor.c \
	PacketLog.c \
	ParameterParseNart.c \
	rate_constants.c \
	TargetPowerGet.c \
	Template.c \
	SetConfig.c \
	SleepMode.c \
	RfBbTestPoint.c \
	vrate_constants.c

OBJS    =  $(patsubst %.c,$(OUTPUT_DIR)/%.o,$(SRCS))

LDFLAGS = -L./ -L../shared/Linux -L../field/Linux -L../anwi/Linux -L../Link/Linux -L../libtlv/outputTemplate/Linux -L../libtlv/libtlvutil/Linux -L../LinkQc9K/Linux -L../calibration/cal-2p/Linux_ar9300 -L../devlib/Linux_ar9287 -L../devlib/Linux_qc98xx -L../devlib/Linux_ar9300 \
-rdynamic \
-lm -lpthread -ldl \
-lfield -lpart -lanwi -llinkAr9k -ltlvtemplate -ltlvutil -lLinkQc9K -lcal-2p -lar9300 -lar9287 -lqc98xx\

BIN = $(OUTPUT_DIR)/nart.out

all: $(OUTPUT_DIR)  $(BIN)

$(BIN): $(OBJS)	
ifndef LD_FLAGS
	$(CC) $(LDFLAGS) -o $@ $(OBJS) 
else
	$(CC) $(LD_FLAGS) $(LDFLAGS) -o $@ $(OBJS)
endif

obj_list:
	rm -f $(BIN) 
	rm -f $(OUTPUT_DIR)/.obj_list
	echo $(OUTPUT_DIR)/*.o > $(OUTPUT_DIR)/.obj_list

$(OUTPUT_DIR): 
	echo $(OUTPUT_DIR)
	mkdir -p ./$(OUTPUT_DIR)
	
$(OUTPUT_DIR)/%.o : %.c
ifndef C_FLAGS
	$(CC) -c $(CFLAGS) $< -o $@
else
	$(CC) -c $(C_FLAGS) $(CFLAGS) $< -o $@
endif

clean:
	rm -fr $(OUTPUT_DIR)

