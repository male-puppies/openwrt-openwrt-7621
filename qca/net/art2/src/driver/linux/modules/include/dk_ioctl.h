#ifndef __DK_IOCTL_H_
#define __DK_IOCTL_H_

#define DK_IOCTL_GET_VERSION 801
#define DK_IOCTL_GET_CLIENT_INFO 802
#define DK_IOCTL_CFG_READ 803
#define DK_IOCTL_CFG_WRITE 804
#define DK_IOCTL_CREATE_EVENT 805
#define DK_IOCTL_GET_NEXT_EVENT 806
#define DK_IOCTL_SYS_REG_READ_32 807
#define DK_IOCTL_SYS_REG_WRITE_32 808
#define DK_IOCTL_FLASH_READ 809
#define DK_IOCTL_FLASH_WRITE 810
#define DK_IOCTL_MAC_WRITE 811
#define DK_IOCTL_GET_CHIP_ID 812
#define DK_IOCTL_RTC_REG_READ 813
#define DK_IOCTL_RTC_REG_WRITE 814
#define DK_IOCTL_FULL_ADDR_READ 815
#define DK_IOCTL_FULL_ADDR_WRITE 816

#undef MAX_BARS
#define MAX_BARS    6

struct cfg_op {
	int offset;
	int size;
	int value;
};
struct client_info {
    unsigned long reg_phy_addr;
    int reg_range;
    unsigned long mem_phy_addr;
    int mem_size;
    int irq;
    unsigned long areg_phy_addr[MAX_BARS];
    int areg_range[MAX_BARS];
    int numBars;
    int device_class;
    unsigned int dma_mem_addr;
};

struct event_op {
	unsigned int valid;
	unsigned int param[16];
};

struct flash_op{
	int fcl;
	int offset;
	int len;
	int retlen;
	unsigned char value;
};

struct flash_op_wr{
	int fcl;
	int offset;
	int len;
	int retlen;
	unsigned char *pvalue;
};

#endif
