/* main.c main 'C' file for the linux dk driver */

/*
                Copyright (c) 2014 Qualcomm Atheros, Inc. All rights reserved.
                Qualcomm is a trademark of Qualcomm Incorporated, registered in the United States and other countries.
                All Qualcomm Incorporated trademarks are used with permission.
                Atheros is a trademark of Qualcomm Atheros, Inc., registered in the United States and other countries.
                Other products and brand names may be trademarks or registered trademarks of their respective owners.
*/



// Include files
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/platform_device.h>

#ifdef AP83
#define WASP_OSPREY
#define PYTHON_EMU
#endif



#ifdef AKRONITE_ART
#define DUAL_PCIE
#define OWL_PB42
#endif

#if defined(OWL_PB42) || defined(PYTHON_EMU)
#include <linux/pci.h>
#endif
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/page-flags.h>
#include <asm/io.h>

#include "dk.h"
#include "client.h"
#if defined(OWL_PB42) || defined(PYTHON_EMU)
//#ifdef PCI_BUS
#include "dk_pci_bus.h"
//#endif
#endif
#define CHIP_ID_LOCATION 0xb8060090
#define HOWL_WMAC_BASE_PHY_ADDRESS 0x180c0000
#define MERLIN_PCI_COMMAND_REG_ADDRESS 0xb4000004
#define SCORPION_PCI_COMMAND_REG_ADDRESS 0xb6000004
#define VIRIAN_BASE_ADDRESS 0xb80f0000
#define SCORPION_BASE_ADDRESS 0xb8280000
#define WASP_WMAC_BASE_PHY_ADDRESS 0x18100000
#define HORNET_WMAC_BASE_PHY_ADDRESS 0x18100000
#define PCIE_1_LINK_ADDRESS 0xb80f0018
#define PCIE_2_LINK_ADDRESS 0xb8280018

#define CHIP_REV_ID_SCORPION_A 0x013 // last nibble is for Chip revision which is ignored
#define CHIP_REV_ID_SCORPION_B 0x113 // last nibble is for Chip revision which is ignored
#define CHIP_REV_ID_DRANGONFLY 0x15  

extern INT32  dk_dev_init(void);
#if  defined(DUAL_PCIE)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
extern u_int32_t get_pci_reg_addr(u_int slot);
#else
extern resource_size_t get_pci_reg_addr(u_int slot);
#endif
//extern u_int get_pci_irq(u_int slot);
#endif
extern void dk_dev_exit(void);
extern INT32 get_chip_id(INT32 cli_id,INT32 offset,INT32 size,INT32 *ret_val);
int init_wmac_device(void);

#ifdef AP83
#ifdef USE_PLATFORM_FRAMEWORK
#include <wlan/ah_devid.h>
static struct platform_device *spdev;

int
get_wmac_irq(u_int16_t wmac_num)
{
	int ret;
	struct resource *res;

	if (spdev == NULL) {
		printk("Invalid platform device pointer\n");
		return;
	}

	res = platform_get_resource(spdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		printk("no IRQ resource found\n");
		ret = -ENXIO;
		goto out;
	}

	ret = res->start;
out:
	return ret;
}

static const struct platform_device_id ath9k_platform_id_table[] = {
	{
		.name = "ath9k",
		.driver_data = AR5416_AR9100_DEVID,
	},
	{
		.name = "ar933x_wmac",
		.driver_data = AR9300_DEVID_AR9330,
	},
	{
		.name = "ar934x_wmac",
		.driver_data = AR9300_DEVID_AR9340,
	},
	{
		.name = "qca953x_wmac",
		.driver_data = AR9300_DEVID_AR953X,
	},
	{
		.name = "qca955x_wmac",
		.driver_data = AR9300_DEVID_AR955X,
	},
	{
		.name = "qca956x_wmac",
		.driver_data = AR9300_DEVID_AR956X,
	},
	{},
};

static int ath_ahb_probe(struct platform_device *pdev)
{
	int ret = 0;

	spdev = pdev;

	if (!pdev->dev.platform_data) {
		printk("no platform data specified\n");
		ret = -EINVAL;
		goto out;
	}

out:
	return ret;
}

static int ath_ahb_remove(struct platform_device *pdev)
{
	spdev = NULL;
	return 0;
}

static struct platform_driver ath_ahb_driver = {
	.probe      = ath_ahb_probe,
	.remove     = ath_ahb_remove,
	.driver		= {
		.name	= "art_internal_ver_1", // Changed the ath_ahb_driver name from ath9k to art_internal_ver_1, since this is not an ath9k driver instead an internal driver for legacy radios in FTM mode
		.owner	= THIS_MODULE,
	},
	.id_table    = ath9k_platform_id_table,
};

MODULE_DEVICE_TABLE(platform, ath9k_platform_id_table);
#endif
#endif

static INT32 __init dk_module_init(void)
{
	INT32 error, ret_val, chip_rev_id, legacy_pci_device; //25-02-2015
        INT32 skip_pcie_init;
	
	//INT32 error, chip_rev_id, legacy_pci_device;
	UINT32 *addr;
	struct pci_dev *pdev;
		
#if  defined(DUAL_PCIE)
#if (CFG_64BIT == 1)
	resource_size_t baseaddr[1];
#else
	A_UINT_PTR baseaddr[1];
#endif
	UINT32 len[1];
	UINT32 irq;
#ifndef PYTHON_EMU
	//UINT8  csz; //25_02_2015
	//UINT32 val; // 25_02_2015
#endif
	UINT32 iIndex, numBars;
#ifdef DK_DEBUG
    UINT32 device_id, vendor_id;
#endif
	//INT8 ret_val; //25_02_2015
	UINT32 sIndex = WMAC_FN_DEV_START_NUM;
 	VOID *dev=NULL; //25-02-2015
#endif
    addr = NULL;
    ret_val = chip_rev_id = 0;
    legacy_pci_device=0;
    skip_pcie_init = 0;
    if((pdev=pci_get_device(0x168C,0x2D,NULL)))
    {
        legacy_pci_device=1;
    }
    else if((pdev=pci_get_device(0x168C,0x2E,NULL)))
    {
        legacy_pci_device=1;
    }
    else if((pdev=pci_get_device(0x168C,0x30,NULL)))
    {
        legacy_pci_device=1;
    }
    else if((pdev=pci_get_device(0x168C,0x33,NULL)))  // peacock
    {
        legacy_pci_device=1;
    }
    else if((pdev=pci_get_device(0x168C,0x3c,NULL)))
    {
        skip_pcie_init = 1;
    }
    else if((pdev=pci_get_device(0x168C,0x40,NULL)))
    {
        skip_pcie_init = 1;
    }
    else if((pdev=pci_get_device(0x168C,0x46,NULL)))
    {
        skip_pcie_init = 1;
    }
    else if((pdev=pci_get_device(0x168C,0x56,NULL)))
    {
        skip_pcie_init = 1;
    }
	printk("DK:legacy_pci_device %d \n",legacy_pci_device);
	printk("DK:pcie init %d \n",skip_pcie_init);

#ifdef AKRONITE_ART
	if(legacy_pci_device==0){
		printk("Akronite ART: bailing out gracefully\n");
		return 0;
	}
#endif

#if  !defined(DUAL_PCIE)

#ifdef DK_DEBUG
		printk("DK::Module init \n");
#endif // DK_DEBUG

#ifndef OWL_PB42
        get_chip_id(0,CHIP_ID_LOCATION,4,&chip_rev_id); // for getting the chip rev_id; to differentiate between PB and AP
        printk("CHIP REV ID: %x\n",chip_rev_id);
#endif
#if  defined(PYTHON_EMU)
	// Scorpion packages A and B (PCIE_2_LINK_ADDRESS is only valid for Scorpion)
    // Dragonfly has one pcie at PCIE_2_LINK_ADDRESS.
	if ((((chip_rev_id& 0xfff0)>>4) == CHIP_REV_ID_SCORPION_A)||
		(((chip_rev_id& 0xfff0)>>4) == CHIP_REV_ID_SCORPION_B)||
		(((chip_rev_id& 0x0ff0)>>4) == CHIP_REV_ID_DRANGONFLY)){ 
	          addr = (UINT32 *)(PCIE_2_LINK_ADDRESS);
	          ret_val = readl(addr);
		if(ret_val==7){
	         	printk("Writing value 0x6 to  PCI_2 command register\n");
	         	addr = (UINT32 *)(SCORPION_PCI_COMMAND_REG_ADDRESS);
	        	writel(0x6,addr); // enabling ddr and dma of Merlin
		}else{
	         	printk("No link on PCIe_2\n");
		}
	}
	if (((chip_rev_id& 0x0ff0)>>4) != CHIP_REV_ID_DRANGONFLY) {
		addr = (UINT32 *)(PCIE_1_LINK_ADDRESS);
		ret_val = readl(addr);
		if(ret_val==7){
			printk("Writing value 0x6 to PCI_1 command register\n");
			addr = (UINT32 *)(MERLIN_PCI_COMMAND_REG_ADDRESS);
			//writel(0x6,addr); // enabling ddr and dma of Merlin
		}else{
			printk("No link on PCIe_1\n");
		}
	}
        if (!skip_pcie_init) {
        printk("Not skipping pcie init \n");
	if((chip_rev_id==0x100)||(chip_rev_id==0x1100)||(chip_rev_id==0x101)||(chip_rev_id==0x1101)
	||(chip_rev_id==0x2120)||(chip_rev_id==0x1120)||(chip_rev_id==0x0120) // Wasp 1.0 package C, B and A
	||(chip_rev_id==0x2121)||(chip_rev_id==0x1121)||(chip_rev_id==0x0121) // Wasp 1.1 package C, B and A
	||(chip_rev_id==0x2122)||(chip_rev_id==0x1122)||(chip_rev_id==0x0122) // Wasp 1.2 package C, B and A
	||(chip_rev_id==0x2123)||(chip_rev_id==0x1123)||(chip_rev_id==0x0123) // Wasp 1.3 package C, B and A
	||(chip_rev_id==0x12123)||(chip_rev_id==0x11123)||(chip_rev_id==0x10123)) {// Wasp 1.3.0.1 package C, B and A
	        addr = (UINT32 *)(VIRIAN_BASE_ADDRESS);
        	writel(readl(addr)& 0xfffeffff,addr);
	        printk("Resetting bit 16 of VIRIAN register 0xb80f0000\n");
	} else if (((chip_rev_id& 0xfff0)>>4 == CHIP_REV_ID_SCORPION_A)||((chip_rev_id& 0xfff0)>>4 == CHIP_REV_ID_SCORPION_B)){ // Scorpion packages A and B
          	addr = (UINT32 *)(PCIE_2_LINK_ADDRESS);
          	ret_val = readl(addr);
		if(ret_val==7){
	        	printk("Bit 16 of 0xb8280000 need not be reset\n");
		}
          	addr = (UINT32 *)(PCIE_1_LINK_ADDRESS);
          	ret_val = readl(addr);
		if(ret_val==7){
	        	printk("Resetting bit 16 of VIRIAN register 0xb80f0000\n");
	        	addr = (UINT32 *)(VIRIAN_BASE_ADDRESS);
        		writel(readl(addr)& 0xfffeffff,addr);
		}
	} else if (((chip_rev_id& 0x0ff0)>>4) == CHIP_REV_ID_DRANGONFLY) {
			addr = (UINT32 *)(PCIE_2_LINK_ADDRESS);
			ret_val = readl(addr);
			if(ret_val==7){
				printk("Bit 16 of 0xb8280000 need not be reset\n");
			}
   	} else {
                addr = (UINT32 *)(0xb80f001c );
                writel(readl(addr)& 0xfffeffff,addr);
                printk("Resetting bit 16 of Python  register 0xb80f001c \n");

	}
        } else {
        printk("Skipping pcie init \n");
        }

#endif
#endif // #if  !defined(P1020)

		error = dk_dev_init();
		if (error < 0) {
			printk("DK::Cannot register device \n");
			return error;
		}
		init_client();
#if  !defined(DUAL_PCIE)
#ifdef AP83
#if USE_PLATFORM_FRAMEWORK
		platform_driver_register(&ath_ahb_driver);
#endif
                if (init_wmac_device()){ // enabling the wmac ; setting the handle for applications
                         printk("Error in initializing wmac \n");
                         return error;
                }

#ifndef WASP_OSPREY
                printk(" WASP_OSPREY not defined. Returning \n");
		return 0;
#endif

#endif
#endif
	if(legacy_pci_device==0){
                printk(" No Legacy PCIe device found. Returning\n");
		return 0;
	}

#if defined(OWL_PB42) || defined(PYTHON_EMU)
#if  !defined(DUAL_PCIE)
		     error = bus_module_init();
#endif
#if  defined(DUAL_PCIE)
	iIndex=0;
#if (CFG_64BIT == 1)
	baseaddr[iIndex] = (resource_size_t)get_pci_reg_addr(iIndex);   
	printk(KERN_ERR" Base Phsycal address :0x%llx\n", baseaddr[iIndex]);
#else
	baseaddr[iIndex] = (A_UINT_PTR)get_pci_reg_addr(iIndex);
	printk(KERN_ERR" Base Phsycal address :0x%08lx\n", baseaddr[iIndex]);
#endif
	len[iIndex] = 0x20000;
    numBars = 1;
	//irq = get_pci_irq(iIndex); 10.4 does not have get_pci_irq 
	irq = pdev->irq;
	sIndex = 0;

	if (add_client(dev,baseaddr,len,irq, numBars, sIndex,0) < 0) {
		printk(KERN_ERR "DK:: unable to add client \n");
#if LINUX_VERSION_CODE > 132098
//		pci_disable_device(dev);
#endif
		return -ENODEV;
	}
#endif
#endif

#if !defined(OWL_PB42) && !defined(PYTHON_EMU)
		if (error < 0) {
			cleanup_client();
			dk_dev_exit();
			printk("DK::Cannot locate device. Reset the machine \n");
			return error;
		}
#endif
		return 0;
}

#ifdef AP83
int init_wmac_device()
{

        VOID *dev;
	UINT32 baseaddr[MAX_BARS];
	UINT32 len[MAX_BARS];
	UINT32 irq;
	UINT32 iIndex, numBars;
#ifdef DK_DEBUG
    UINT16 device_id, vendor_id;
#endif
	UINT32 sIndex = WMAC_FN_DEV_START_NUM;
	dev=0;
for (iIndex=0; iIndex<1; iIndex++) { // assume that only one wmac
#ifdef WASP
	  baseaddr[iIndex] =WASP_WMAC_BASE_PHY_ADDRESS;
#elif HORNET
	  baseaddr[iIndex] =HORNET_WMAC_BASE_PHY_ADDRESS;
#else
	  baseaddr[iIndex] =HOWL_WMAC_BASE_PHY_ADDRESS;
#endif
	printk(KERN_ERR" Base Phsycal address :0x%x\n", baseaddr[iIndex]);
	  len[iIndex] = 0x00ffffff;
      if (len[iIndex] == 0) break;
    }
    numBars = iIndex;
    irq=2;

#ifdef USE_PLATFORM_FRAMEWORK
    irq = get_wmac_irq(sIndex);
#endif

#if (CFG_64BIT == 1)
if (add_client(dev,(INT32 *)baseaddr,len,irq, numBars, sIndex,0) < 0) {
#else
if (add_client(dev,(A_UINT_PTR *)baseaddr,len,irq, numBars, sIndex,0) < 0) {
#endif
		printk(KERN_ERR "DK:: unable to add client \n");
#if LINUX_VERSION_CODE > 132098
		//pci_disable_device(dev);
                //MKDEV
                //processEepromWriteByteBasedBlockCmd
                //pci_enable_device
#endif
		return -1;
	}
	return 0;


}
#endif


static void __exit  dk_module_exit(void)
{
    INT32 legacy_pci_device = 0;
    INT32 skip_pcie_init;
    struct pci_dev *pdev;
	skip_pcie_init = 0;
    if((pdev=pci_get_device(0x168C,0x2D,NULL)))
    {
        legacy_pci_device=1;
    }
    else if((pdev=pci_get_device(0x168C,0x2E,NULL)))
    {
        legacy_pci_device=1;
    }
    else if((pdev=pci_get_device(0x168C,0x30,NULL)))
    {
        legacy_pci_device=1;
    }
    else if((pdev=pci_get_device(0x168C,0x33,NULL)))
    {
        legacy_pci_device=1;
    }
	else if((pdev=pci_get_device(0x168C,0x3c,NULL)))
    {
         skip_pcie_init = 1;
    }
	else if((pdev=pci_get_device(0x168C,0x40,NULL)))
    {
         skip_pcie_init = 1;
    }
	else if((pdev=pci_get_device(0x168C,0x46,NULL)))
    {
         skip_pcie_init = 1;
    }
	else if((pdev=pci_get_device(0x168C,0x56,NULL)))
    {
         skip_pcie_init = 1;
    }
#ifdef AKRONITE_ART
	if(legacy_pci_device==0){
		printk("Akronite ART bail out gracefully\n");
		return;
	}
#endif

#ifdef DK_DEBUG
		printk("DK::Module exit \n");
#endif // DK_DEBUG
#if defined(OWL_PB42) || defined(PYTHON_EMU)
#if !defined(DUAL_PCIE)
       if (legacy_pci_device != 0)
		bus_module_exit();
#endif
#endif
		cleanup_client();
#ifdef AP83
#if USE_PLATFORM_FRAMEWORK
        platform_driver_unregister(&ath_ahb_driver);
#endif
#endif

		dk_dev_exit();

		return;
}

#ifdef MODULE

#if LINUX_VERSION_CODE > 132098
	MODULE_LICENSE(MOD_LICENCE);
#endif

MODULE_AUTHOR(MOD_AUTHOR);
MODULE_DESCRIPTION(MOD_DESCRIPTION);

module_init(dk_module_init);
module_exit(dk_module_exit);

#endif // MODULE

