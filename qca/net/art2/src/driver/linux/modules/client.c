/* client.c */
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
#include <linux/version.h>
#include <asm/io.h>
#if defined(OWL_PB42) || defined(PYTHON_EMU)
//#ifdef PCI_BUS
#include "dk_pci_bus.h"
//#endif
#endif

#include "dk.h"
#include "dk_ioctl.h"
#include "dk_event.h"
#include "client.h"

#define MIN_OFFSET 0x00000000
#define MAX_OFFSET 0x1fffffff

#define ORDER 9
#define ORDER_COMM_CLASS 8
#define ATHEROS_VENDOR_ID 0x168c
#define HOWL_RTC_BASE_ADDRESS 0xb80e0000
#define HOWL_WMAC_BASE_VIR_ADDRESS 0xb80c0000
#define HORNET_RTC_BASE_ADDRESS 0xb8100000
#define HORNET_WMAC_BASE_VIR_ADDRESS 0xb8100000
/* global variables */
#if defined(DUAL_PCIE)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
extern u_int32_t get_pci_virt_addr(u_int slot);
#else
extern resource_size_t get_pci_virt_addr(u_int slot);
#endif

#endif
static atheros_dev dev_table[MAX_CLIENTS_SUPPORTED];
static long unsigned int mem_table[MAX_CLIENTS_SUPPORTED];
static UINT32 mem_page_order[MAX_CLIENTS_SUPPORTED];

/* extern declarations */
extern irqreturn_t dk_intr_handler(INT32,VOID *,struct pt_regs *);

/* forward declarations */
static VOID reset_device(INT32 cli_id);

#if (CFG_64BIT == 1) 
resource_size_t pci_phy_baseaddr[MAX_BARS];
#endif

VOID init_client
(
 	VOID
)
{
	UINT32 i;

#ifdef DK_DEBUG
	printk("DK::Init dev table \n");
#endif

	for (i=0;i<MAX_CLIENTS_SUPPORTED;i++) {
			dev_table[i].cli_id = INVALID_CLIENT;
			mem_table[i] = 0;
	}
		
}

static VOID init_atheros_dev
(
 	p_atheros_dev dev
)
{
        int iIndex;

		dev->bus_dev = NULL;
		dev->reg_phy_addr = 0;
		dev->reg_ker_vir_addr = 0;
		dev->reg_range = 0;
		dev->mem_phy_addr = 0;
		dev->mem_ker_vir_addr = 0;
		dev->mem_size = 0;
		dev->irq = 0;
		dev->dev_busy = 0;
		dev->cli_id = INVALID_CLIENT;
		memset(&dev->isr_event_q,0,sizeof(event_queue));
		memset(&dev->trigered_event_q,0,sizeof(event_queue));
        for(iIndex=0; iIndex<MAX_BARS; iIndex++) {
		     dev->areg_phy_addr[iIndex] = 0;
		     dev->areg_ker_vir_addr[iIndex] = 0;
		     dev->areg_range[iIndex] = 0;
        }
}

/*
 * Get the client for this device
 * The table give the mapping between the device and the client 
 * in the dev_table. The minor number is used to idenitfy the client
 * for the device.
 *  Device    client       minor 
 * /dev/dk0 - dev_table[0]   0
 * /dev/dk1 - dev_table[1]   1
 * /dev/dk2 - dev_table[2]   2
 * /dev/dk3 - dev_table[3]   3
 */
static INT32 get_client_id
(
 	INT32 major,
 	INT32 minor
)
{
#ifdef DK_DEBUG
	printk("DK::get_client_id:major=%d:minor=%d\n", major, minor);
#endif
	if (major == DK_UART_MAJOR_NUMBER) {
		return dev_table[minor + UART_FN_DEV_START_NUM].cli_id;
	}
	else
		return dev_table[minor].cli_id;
}

p_atheros_dev get_client
(
 	INT32 cli_id
)
{
	p_atheros_dev dev;

	dev = &dev_table[cli_id];
	if (!VALID_CLIENT(dev))  {
		return NULL;
	}

	return dev;
}

INT32 add_client
(
 	VOID *bus_dev,
#if (CFG_64BIT == 1)
	resource_size_t baseaddr[MAX_BARS],
#else
	A_UINT_PTR baseaddr[MAX_BARS],
#endif
	UINT32 len[MAX_BARS],
	UINT32 irq,
    UINT32 numBars,
    UINT32 sIndex,
    int pci
)
{
		UINT32 i;
		INT32 cli_id;
		p_atheros_dev dev;
		A_UINT_PTR reg_vir_addr[MAX_BARS] = {0};
		A_UINT_PTR mem_phy_addr;
#if (CFG_64BIT == 1)
		resource_size_t *mem;
#else
		long unsigned int *mem;
#endif
		struct page *page;
		UINT32 no_pages, iIndex;


#if defined(OWL_PB42) || defined(PYTHON_EMU)
        INT8 ret_val;
#endif
        UINT32 page_order=ORDER;
#if defined(OCTEON)
        dma_addr_t dma_handle;
#endif


		dev = NULL;
		mem = NULL;
		cli_id = INVALID_CLIENT;
		mem_phy_addr = 0;

		for (i=sIndex;i<MAX_CLIENTS_SUPPORTED;i++) {
				if (dev_table[i].cli_id == INVALID_CLIENT) {
						cli_id = i;
						dev = &dev_table[i];
						mem = &mem_table[i];
						break;
				}
		}

		if (cli_id == INVALID_CLIENT) {
			printk("DK:: Only %d clients supported \n",MAX_CLIENTS_SUPPORTED);
			return -1;
		}
		
		printk("DK::Add client %d \n",cli_id);

		init_atheros_dev(dev);		

        for(iIndex=0; iIndex<numBars; iIndex++) {
#if defined(OWL_PB42) || defined(PYTHON_EMU)
#if defined(DUAL_PCIE)

                      reg_vir_addr[iIndex] = (A_UINT_PTR)get_pci_virt_addr(iIndex);

			printk(KERN_ERR" Reg Virtual address :0x%08lx\n", reg_vir_addr[iIndex]);
#if (CFG_64BIT == 1)
	pci_phy_baseaddr[cli_id] = baseaddr[iIndex];
#endif
#endif
if(pci){
            (void)pci_read_config_byte(bus_dev, PCI_BASE_ADDRESS_0 + (iIndex *4), &ret_val);
            if (ret_val&0x1) {
               // IO region mapping
               /*
		       printk("DK:: Requesting IO region=%x:range=%d\n", baseaddr[iIndex], len[iIndex]);
		       if (request_region(baseaddr[iIndex],len[iIndex],DRV_NAME) == NULL) {
		          printk(KERN_ERR "DK:: unable to request io region for Bar %d: len = %d\n", iIndex, len[iIndex]);
				  return -1;
	           }
               */
            }
            else {
#if (CFG_64BIT == 1)
		printk("DK:: Requesting MEM region=%llx:range=%d\n", baseaddr[iIndex], len[iIndex]);
#else
		      printk("DK:: Requesting MEM region=%lx:range=%d\n", baseaddr[iIndex], len[iIndex]);
#endif
		      if (request_mem_region(baseaddr[iIndex],len[iIndex],DRV_NAME) == NULL) {
		          printk(KERN_ERR "DK:: unable to request mem region for Bar %d: len = %d\n", iIndex, len[iIndex]);
				  return -1;
	          }
#if (CFG_64BIT == 1)
	pci_phy_baseaddr[cli_id] = baseaddr[iIndex];
#endif
                      reg_vir_addr[iIndex] = (A_UINT_PTR)ioremap_nocache(baseaddr[iIndex],len[iIndex]);
                      if ((VOID *)reg_vir_addr[iIndex] == NULL) {
                                free_irq(irq,(void *)dev);
                                for(iIndex=0; iIndex<numBars; iIndex++) {
                                          iounmap((VOID *)reg_vir_addr[iIndex]);
                                          //release_mem_region(baseaddr[iIndex],len[iIndex]);
                                }
                                printk(KERN_ERR "DK:: unable to remap registers \n");
                                return -1;
                      }
            }
}
#endif
#ifdef AP83
if(!pci){
		      reg_vir_addr[iIndex] = (UINT32)ioremap_nocache(baseaddr[iIndex],len[iIndex]);
		      if ((VOID *)reg_vir_addr[iIndex] == NULL) {
				free_irq(irq,(void *)dev);
                                for(iIndex=0; iIndex<numBars; iIndex++) {
			        	  iounmap((VOID *)reg_vir_addr[iIndex]);
		        	  	  //release_mem_region(baseaddr[iIndex],len[iIndex]);
                                }   
				printk(KERN_ERR "DK:: unable to remap registers \n");
				return -1;
		      }
}
#endif
        } // end of for

		   /* Pass the pointer to the client id */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
		   if (request_irq(irq,(void *)dk_intr_handler,SA_SHIRQ,DRV_NAME,(void *)dev) != 0) {
#else
		   if (request_irq(irq,(void *)dk_intr_handler,IRQF_SHARED,DRV_NAME,(void *)dev) != 0) {
#endif
              for(iIndex=0; iIndex<numBars; iIndex++) {
				iounmap((VOID *)reg_vir_addr[iIndex]);
				release_mem_region(baseaddr[iIndex],len[iIndex]);
              }
		        printk(KERN_ERR "DK:: unable to request irq \n");
				return -1;
		   }


        for(iIndex=0; iIndex<numBars; iIndex++) {
#if (CFG_64BIT == 1)
	printk("DK::Reg phy addr = %llx vir Addr = %llx \n",baseaddr[iIndex],reg_vir_addr[iIndex]);
#else	   
		   printk("DK::Reg phy addr = %lx vir Addr = %lx \n",baseaddr[iIndex],reg_vir_addr[iIndex]);
#endif
        }
		printk("DK::Irq = %x \n",irq);


		if (*mem == 0) {
#if defined(OWL_PB42) || defined(PYTHON_EMU)
#if defined (DUAL_PCIE)

		    *mem = __get_free_pages(GFP_KERNEL|GFP_DMA,ORDER);
                    page_order = ORDER;
#endif
if(pci){
                (void)pci_read_config_byte(bus_dev, 0xb, &ret_val);
                if (ret_val == SIMPLE_COMM_CLASS) {
				    *mem = __get_free_pages(GFP_KERNEL,ORDER_COMM_CLASS);
                    page_order = ORDER_COMM_CLASS;
                }
                else {
				    *mem = __get_free_pages(GFP_KERNEL|GFP_DMA,ORDER);
				      //*mem = ioremap_nocache(0xf00000, 0x100000);
                    page_order = ORDER;
                }
}
#endif
#ifdef AP83
if(!pci){
                    *mem = __get_free_pages(GFP_KERNEL|GFP_DMA,ORDER);
                    page_order = ORDER;

}
#endif
		if (*mem == 0) {
                    for(iIndex=0; iIndex<numBars; iIndex++) {
					   iounmap((VOID *)reg_vir_addr[iIndex]);
					   free_irq(irq,(void *)dev);
					   release_mem_region(baseaddr[iIndex],len[iIndex]);
                    }
					   printk(KERN_ERR "DK:: unable to allocate contigous memory \n");
					   return -1;
		}
				// map the pages as reserved,
				// otherwise remap_page_range wont 
				// do what we expect

				page =  virt_to_page((void *)(*mem));
				no_pages = 1 << page_order;
                		mem_page_order[cli_id] = page_order;
				for (i=0;i<no_pages;i++) {
					SetPageReserved(page);
					page++;
				}
				
				printk("DK::Allocated 0x%x bytes memory \n", (UINT32) (PAGE_SIZE * (1<<page_order)));
		} else {
				printk("DK::Using memory already allocated at %lx \n",*mem);
		}
#if defined(OCTEON)
                mem_phy_addr = virt_to_phys(pci_alloc_consistent(bus_dev, PAGE_SIZE * (1<<page_order), &dma_handle));
		printk("DK::DMA addr 0x%lx, mem phy addr %lx\n", dma_handle, mem_phy_addr);
#else
                mem_phy_addr = __pa(*mem);
#endif
		printk("DK::memory vir Addr = %lx phy Addr = %lx \n",*mem,mem_phy_addr);

		/* 
		 * TO DO: (if necessary)
		 * Kernel already has a mapping for this physical pages. 
		 * They are part of the identity mapping, so they may be 
		 * cached. Remap it to another virtual address space 
		 * if the memory will be accessed from the kernel
		 */
		
		dev->bus_dev = bus_dev;
        for(iIndex=0; iIndex<numBars; iIndex++) {
		   dev->areg_phy_addr[iIndex] = baseaddr[iIndex];
		   dev->areg_ker_vir_addr[iIndex] = reg_vir_addr[iIndex];
		   dev->areg_range[iIndex] = len[iIndex];
        }
		dev->reg_phy_addr = baseaddr[0];
		dev->reg_ker_vir_addr = reg_vir_addr[0];
		dev->reg_range = len[0];
        	dev->numBars = numBars;
		dev->mem_ker_vir_addr = *mem;
		dev->mem_phy_addr = mem_phy_addr;
		dev->mem_size = (1 << page_order) * PAGE_SIZE;
		dev->irq = irq;
		dev->cli_id = cli_id;
#if defined(OCTEON)
        dev->dma_mem_addr = dma_handle;
#endif 

#if defined(OWL_PB42) || defined(PYTHON_EMU)
if(pci){
		cli_cfg_read(cli_id,8,4,&dev->device_class);
}
#endif 
        	dev->device_class >>= 24;

		reset_device(cli_id);
		return 0;
}


VOID remove_client
(
 	VOID *bus_dev, int pci
)
{
		UINT32 i, iIndex;
		p_atheros_dev dev;

		printk("DK::Remove client \n");

		dev = NULL;
		for (i=0;i<MAX_CLIENTS_SUPPORTED;i++) {
				if (dev_table[i].cli_id == INVALID_CLIENT) continue;
			   	if (dev_table[i].bus_dev == bus_dev) {
						dev = &dev_table[i];
						break;
				}
		}
		
		if (dev == NULL) {
			printk("DK:: Invalid client in remove_client \n");
			return;
		}
		
		reset_device(dev->cli_id);
	if(pci)
	{	
        for(iIndex=0; iIndex<dev->numBars; iIndex++) {
		    iounmap((VOID *)dev->areg_ker_vir_addr[iIndex]);
		    //release_mem_region(dev->areg_phy_addr[iIndex], dev->areg_range[iIndex]);
		    printk("DK::Free reg space phy=%lx vir=%lx \n",dev->areg_phy_addr[iIndex],dev->areg_ker_vir_addr[iIndex]);
		}
    }
		
		free_irq(dev->irq,(void *)dev);
		printk("DK::Free irq = %x \n",dev->irq);

		init_atheros_dev(dev);		

		return;
}


VOID cleanup_client
(
 	VOID
)
{
		UINT32 i;
		UINT32 j;
		struct page *page;
		UINT32 no_pages;

#ifdef DK_DEBUG
		printk("DK::Cleanup dev table \n");
#endif // DK_DEBUG
		for (i=0;i<MAX_CLIENTS_SUPPORTED;i++) {
				if (dev_table[i].cli_id != INVALID_CLIENT) {
						remove_client(dev_table[i].bus_dev,0);
						dev_table[i].cli_id = INVALID_CLIENT;
				}
				if (mem_table[i] != 0) {
						printk("DK::Freeing memory at %lx \n",mem_table[i]);
						// remove the reserved flag
						
						page = virt_to_page((void *)mem_table[i]);
						no_pages = 1 << mem_page_order[i];
                        printk("page order=%d\n", mem_page_order[i]);
						for (j=0;j<no_pages;j++) {
							 ClearPageReserved(page);
							page++;
						}
						
						free_pages(mem_table[i],8);
						mem_table[i] = 0;
				}
		}
		return;
}

INT32 register_client
(
 	INT32 major,
 	INT32 minor
)
{
	INT32 cli_id;
	p_atheros_dev dev;
#ifndef AKRONITE_ART
#if defined(OWL_PB42) || defined(PYTHON_EMU)
	UINT32 vendor_id;
#endif
#endif

	/* get the client for this device */
	cli_id = get_client_id(major, minor);

	if (cli_id == INVALID_CLIENT) {
		printk("DK::register_client:Device not found \n");
	 	return -ENODEV;
	}

	printk("DK::Regsitering client %d \n",cli_id);
	dev = &dev_table[cli_id];

	if (!VALID_CLIENT(dev)) {
		printk("DK::register_client:Invalid client \n");
	 	return -ENODEV;
    }

	if (BUSY_CLIENT(dev)) {
		printk("DK::register_client:Client alreay in use \n");
		return -EACCES;
	}
#if !defined (DUAL_PCIE)	

	// check whether the device is present
	// by reading the vendor id
#if defined(OWL_PB42) || defined(PYTHON_EMU)
#ifdef WASP_OSPREY
   if(cli_id!=0){ // For DBDC operation, Wasp radio's client ID is zero; 
#endif
	cli_cfg_read(cli_id,0,4,&vendor_id);
	if ((vendor_id & 0xffff) != ATHEROS_VENDOR_ID) {
		printk("DK::Device not present \n");
	 	return -ENODEV;
	}

#ifdef WASP_OSPREY
   }
#endif
#ifndef PYTHON_EMU
	if (bus_dev_init(dev->bus_dev) < 0) {
		printk("DK::register_client:Cannot initialize client \n");
		return -EACCES;
	}
#endif
#endif
#endif
	initEventQueue(&dev->isr_event_q);
	initEventQueue(&dev->trigered_event_q);
	
	reset_device(cli_id);
					
	dev->dev_busy = 1;
				  
	return dev->cli_id;
}

VOID unregister_client
(
 	INT32 cli_id
)
{
	p_atheros_dev dev;

	printk("DK::Unregsitering client %d \n",cli_id);
	
	dev = &dev_table[cli_id];

	if (!VALID_CLIENT(dev)) {
		printk("DK::unregister_client:Invalid client \n");
	 	return;
    }
	if (!BUSY_CLIENT(dev)) {
		printk("DK::unregister_client:Client not registered \n");
		return;
	}
	
	if ((cli_id >= 0) && (cli_id < MAX_CLIENTS_SUPPORTED))
	{
		reset_device(dev->cli_id);
	}
	deleteEventQueue(&dev->isr_event_q);
	deleteEventQueue(&dev->trigered_event_q);
#if defined(OWL_PB42) || defined(PYTHON_EMU)	
	bus_dev_exit(dev->bus_dev);
#endif
	dev->dev_busy = 0;
	
	return;
}

INT32 cli_reg_read
(
 	INT32 cli_id,
	INT32 offset,
	UINT32 *data
)
{
	p_atheros_dev dev;
	UINT32 *addr = NULL;
	
	dev = &dev_table[cli_id];
	if (!VALID_CLIENT(dev)) {
		printk("DK::reg_read:Invalid client \n");
	 	return -1;
    }
#if defined(OWL_PB42) || defined(PYTHON_EMU)	
#if (CFG_64BIT == 1)
	addr = (resource_size_t *)(dev->areg_ker_vir_addr[0] + offset);	
#else	
	addr = (UINT32 *)(dev->areg_ker_vir_addr[0] + offset);
#endif
#endif
#ifdef AP83
#ifndef WASP_OSPREY
	#ifdef HORNET
        addr = (UINT32 *)(HORNET_WMAC_BASE_VIR_ADDRESS + offset);
	#else
        addr = (UINT32 *)(HOWL_WMAC_BASE_VIR_ADDRESS + offset);
	#endif        
#endif
#endif
	*data = readl(addr);
#ifdef DK_DEBUG
	printk("DK::Reg read @ 0x%08lx : 0x%04x \n",(A_UINT_PTR)addr,*data);
#endif

	return 0;
}

INT32 cli_reg_write
(
 	INT32 cli_id,
	INT32 offset,
	UINT32 data
)
{
	p_atheros_dev dev;
	UINT32 *addr = NULL;

	dev = &dev_table[cli_id];
	if (!VALID_CLIENT(dev)) {
		printk("DK::reg_write:Invalid client \n");
	 	return -1;
    }
#if defined(OWL_PB42) || defined(PYTHON_EMU)	
	addr = (UINT32 *)(dev->areg_ker_vir_addr[0] + offset);
#endif
#ifdef AP83
#ifndef WASP_OSPREY
	#ifdef HORNET
        addr = (UINT32 *)(HORNET_WMAC_BASE_VIR_ADDRESS + offset);
	#else
        addr = (UINT32 *)(HOWL_WMAC_BASE_VIR_ADDRESS + offset);
	#endif        
#endif
#endif
	writel(data,addr);
#ifdef DK_DEBUG
	printk("DK::Reg write @ 0x%08lx : 0x%04x \n",(A_UINT_PTR)addr,data);
#endif

	return 0;
}
#if defined(OWL_PB42) || defined(PYTHON_EMU)
INT32 cli_cfg_read
(
 	INT32 cli_id,
	INT32 offset,
	INT32 size,
	INT32 *ret_val
)
{
	p_atheros_dev dev;

	dev = &dev_table[cli_id];
	
	if (!VALID_CLIENT(dev)) {
		printk("DK::cfg_read:Invalid client \n");
	 	return -1;
    }
	
	return bus_cfg_read(dev->bus_dev,offset,size,ret_val);
}
#endif

#if defined(AP83) || defined(PYTHON_EMU)
INT32 full_addr_read
(
        INT32 cli_id,           
        INT32 offset,                   
        INT32 *ret_val                  
)                               
{
        p_atheros_dev dev;      
        UINT32 *addr;           

	dev = &dev_table[cli_id];
	
	if (((UINT32)offset<MIN_OFFSET)||(offset>MAX_OFFSET)) {
		printk("DK::reg_read:Invalid offset \n");
		return -1;      
	}
	if (!VALID_CLIENT(dev)) {
		printk("DK::rtc_reg_read:Invalid client \n");
		return -1;      
	}                                           
	addr = (UINT32 *)(0x00000000+offset);
	*ret_val = readl(addr);
	return 0;

} 
INT32 full_addr_write
(
        INT32 cli_id,
        INT32 offset,
        UINT32 data
)
{
        p_atheros_dev dev;
        UINT32 *addr;

	dev = &dev_table[cli_id];
	if (((UINT32)offset<MIN_OFFSET)||(offset>MAX_OFFSET)) {
		printk("DK::reg_write:Invalid offset \n");
		return -1;      
	}
	if (!VALID_CLIENT(dev)) {
		printk("DK::reg_write:Invalid client \n");
		return -1;
	}

        addr = (UINT32 *)(0x000000 + offset);
        writel(data,addr);


        return 0;
}
#endif

#if !defined(OCTEON) 
INT32 rtc_reg_read
(
        INT32 cli_id,           
        INT32 offset,                   
        INT32 *ret_val                  
)                               
{
        p_atheros_dev dev;      
        UINT32 *addr;           

	dev = &dev_table[cli_id];
	if (((UINT32)offset<MIN_OFFSET)||(offset>MAX_OFFSET)) {
		printk("DK::reg_read:Invalid offset \n");
		return -1;      
	}
	if (!VALID_CLIENT(dev)) {
		printk("DK::rtc_reg_read:Invalid client \n");
		return -1;      
	}                                           
#ifdef HORNET
	addr = (UINT32 *)(HORNET_RTC_BASE_ADDRESS+offset);
#else
	addr = (UINT32 *)(HOWL_RTC_BASE_ADDRESS+offset);
#endif          
	*ret_val = readl(addr);
	return 0;

} 

INT32 get_chip_id                     
(                               
        INT32 cli_id,                   
        INT32 offset,           
        INT32 size,             
        INT32 *ret_val          
)                       
{                               
	p_atheros_dev dev;                      
	UINT32 *addr;           
	dev = &dev_table[cli_id];

	
	if (((UINT32)offset<MIN_OFFSET)||(offset>MAX_OFFSET)) {
		printk("DK::reg_read:Invalid offset \n");
		return -1;      
	}
	if (!VALID_CLIENT(dev)) { 
		printk("DK::cfg_read:Invalid client \n");
		return -1;      
	}                           
	addr = (UINT32 *)(0x00000000+offset);
	*ret_val = readl(addr);               
	return 0;             

}

INT32 rtc_reg_write
(
        INT32 cli_id,
        INT32 offset,
        UINT32 data
)
{
        p_atheros_dev dev;
        UINT32 *addr;

	dev = &dev_table[cli_id];
	if (((UINT32)offset<MIN_OFFSET)||(offset>MAX_OFFSET)) {
		printk("DK::reg_write:Invalid offset \n");
		return -1;      
	}
	if (!VALID_CLIENT(dev)) {
		printk("DK::reg_write:Invalid client \n");
		return -1;
	}

			#ifdef HORNET
        addr = (UINT32 *)(HORNET_RTC_BASE_ADDRESS + offset);
			#else
        addr = (UINT32 *)(HOWL_RTC_BASE_ADDRESS + offset);
			#endif        
        writel(data,addr);


        return 0;
}
#endif  //end OCTEON def


#if defined(OWL_PB42) || defined(PYTHON_EMU)  
INT32 cli_cfg_write
(
 	INT32 cli_id,
	INT32 offset,
	INT32 size,
	INT32 ret_val
)
{
	p_atheros_dev dev;
	
	dev = &dev_table[cli_id];
	
	if (!VALID_CLIENT(dev)) {
		printk("DK::cfg_write:Invalid client \n");
	 	return -1;
    }
	
	return bus_cfg_write(dev->bus_dev,offset,size,ret_val);
}
#endif


INT32 get_cli_info
(
 	INT32 cli_id,
	struct client_info *ci
)
{
	p_atheros_dev dev;
    int iIndex;
	
	dev = &dev_table[cli_id];
	
	if (!VALID_CLIENT(dev)) {
		printk("DK::get_client_info:Invalid client \n");
	 	return -1;
    }
	
	//memset(ci->areg_phy_addr,0,sizeof(ci->areg_phy_addr));
	
	//memset(ci->areg_range, 0, sizeof(ci->areg_range));
	
	ci->reg_phy_addr = dev->areg_phy_addr[0];
	ci->reg_range = dev->areg_range[0];
    for (iIndex=0; iIndex<dev->numBars; iIndex++) {
	   ci->areg_phy_addr[iIndex] = dev->areg_phy_addr[iIndex];
	   ci->areg_range[iIndex] = dev->areg_range[iIndex];
    }
	
	//Need to change
	 for (iIndex=dev->numBars; iIndex<MAX_BARS; iIndex++) {
	   ci->areg_phy_addr[iIndex] = 0;
	   ci->areg_range[iIndex] = 0;
    }
      


    ci->numBars = dev->numBars;
	ci->mem_phy_addr = dev->mem_phy_addr;
	ci->mem_size = dev->mem_size;
	ci->irq = dev->irq;
    ci->device_class = dev->device_class;
    ci->dma_mem_addr = dev->dma_mem_addr;

	return 0;
}

static VOID reset_device
(
 	INT32 cli_id
)
{
	UINT32 out;
	out = 0;
#ifdef DK_DEBUG
	printk("DK::Disable the interrupts and reset the device \n");	
#ifdef PYTHON_EMU
	cli_reg_read(cli_id, 0x4020, &out);
	printk("Device Mac Rev : 0x%x\n",out);			
#endif 
#endif
#if defined(OWL_PB42) || defined(PYTHON_EMU)
	if (dev_table[cli_id].device_class == NETWORK_CLASS) {
	   // disable interrupts 	
	   cli_reg_write(cli_id,0x0024,0x0);
	   // put the device in reset state		
	   cli_reg_write(cli_id,0x704c,0x1); 				
	   cli_reg_write(cli_id,0x7040,0x0); 				
	   cli_reg_write(cli_id,0x7040,0x1); 	

	   cli_reg_read(cli_id, 0x7044, &out);
	   printk("Device Status is : 0x%x\n",out);			
	}
	if (dev_table[cli_id].device_class == SIMPLE_COMM_CLASS) {
	   cli_reg_write(cli_id,0x4,0x0); // IER
	   cli_reg_write(cli_id,0x114,0x0); // Extended IER
	   cli_reg_write(cli_id,0x104,0x1); // RC
	}
#endif
#ifdef AP83
#ifndef WASP
        //if (dev_table[cli_id].device_class == NETWORK_CLASS) {
           // disable interrupts
           cli_reg_write(cli_id,0x0024,0x0);
           // put the device in reset state
           rtc_reg_write(cli_id,0x004c,0x1);
           rtc_reg_write(cli_id,0x0040,0x0);
           rtc_reg_write(cli_id,0x0040,0x1);

           rtc_reg_read(cli_id, 0x0044, &out);
           printk("Device Status is : 0x%x\n",out);

//}
        /*if (dev_table[cli_id].device_class == SIMPLE_COMM_CLASS) {
           cli_reg_write(cli_id,0x4,0x0); // IER
           cli_reg_write(cli_id,0x114,0x0); // Extended IER
           cli_reg_write(cli_id,0x104,0x1); // RC
        }*/


#endif
#endif


	return;
}	
