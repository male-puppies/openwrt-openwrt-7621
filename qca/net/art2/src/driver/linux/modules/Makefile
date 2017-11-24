EXTRA_CFLAGS += -I$(PWD)/modules/include  -DPCI_BUS -D__KERNEL__ -DMODULE 
IX_CFLAGS += -I$(ROOTDIR)

ifeq ($(PB42),1)
EXTRA_CFLAGS += -DOWL_PB42
endif

ifeq ($(CFG_64BIT),1)
EXTRA_CFLAGS += -DOWL_PB42
EXTRA_CFLAGS += -DCFG_64BIT=1
endif

ifeq ($(IOCTL_REG_ACCESS),1)
EXTRA_CFLAGS += -DIOCTL_REG_ACCESS
endif

ifeq ($(PYTHON_EMU),1)
EXTRA_CFLAGS += -DPYTHON_EMU
endif

ifeq ($(DEBUG),1)
EXTRA_CFLAGS += -DDK_DEBUG
endif

ifeq ($(AP83),1)
EXTRA_CFLAGS += -DAP83
endif

ifeq ($(DUAL_PCIE),1)
EXTRA_CFLAGS += -DDUAL_PCIE
endif

ifeq ($(THIRD_PARTY_INTEL),1)
EXTRA_CFLAGS += -DTHIRD_PARTY_INTEL
endif

ifeq ($(WASP),1)
EXTRA_CFLAGS += -DAP83
EXTRA_CFLAGS += -DWASP
endif

ifeq ($(WASP_OSPREY),1)
EXTRA_CFLAGS += -DWASP_OSPREY
endif

ifeq ($(PCI_2),1)
EXTRA_CFLAGS += -DPCI_2
endif

ifeq ($(HORNET),1)
EXTRA_CFLAGS += -DAP83
EXTRA_CFLAGS += -DHORNET
endif

ifeq ($(OCTEON),1)
EXTRA_CFLAGS += -DOCTEON
endif

ifeq ($(USE_PLATFORM_FRAMEWORK),1)
EXTRA_CFLAGS += -DUSE_PLATFORM_FRAMEWORK=1
endif

obj-m := art.o
art-objs := dk_func.o client.o main.o dk_pci_bus.o dk_isr.o dk_event.o

clean:
	rm -rf *.o *.ko .*.cmd *mod.c .tmp_versions


