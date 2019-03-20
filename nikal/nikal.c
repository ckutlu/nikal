/**  This file implements the NI-KAL Kernel Abstraction Layer for Linux

    Copyright 2002-2012
    National Instruments Corporation.
    All rights reserved.

    originated:  17.jun.2002
**/




#undef __NO_VERSION__

#define ___nikal_c___


#ifdef nNIKAL250_kUAPIVersion
#include <generated/uapi/linux/version.h>
#else
#include <linux/version.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   #error ("NI-KAL currently only supports Linux kernel versions 2.6 and above");
#endif


#define nNIKAL180_mUSBSupportedVersion (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25))
#define nNIKAL100_mUSBIsSupported (nNIKAL180_mUSBSupportedVersion && (defined(CONFIG_USB) || defined(CONFIG_USB_MODULE)))


#define nNIKAL1500_mACPIIsSupported (defined(CONFIG_ACPI))


#define nNIKAL100_mSerialCoreIsSupported (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))

#define nNIKAL100_mMin(x,y) ((x)<(y)?(x):(y))
#define nNIKAL100_mMax(x,y) ((x)>(y)?(x):(y))

#ifdef nNIKAL160_kConfig
   #include <linux/config.h>
#endif

#ifdef MODULE
   #if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
      #define MODVERSIONS
   #endif

   #ifdef MODVERSIONS
      #include <config/modversions.h>
   #endif

   #include <linux/module.h>
   #include <linux/kmod.h>
#endif

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/usb.h>
#include <linux/acpi.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/pagemap.h>
#include <linux/kthread.h>
#include <linux/kref.h>
#ifdef nNIKAL150_kMutexMethod
   #include <linux/mutex.h>
#endif

#ifdef nNIKAL200_kCompletion
   #include <linux/completion.h>
#endif

#include <asm/mman.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <asm/pgtable.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/cpumask.h>
#include <linux/pagemap.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/gfp.h>
#include <linux/jiffies.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/serial_core.h>
#include <linux/tty_flip.h>
#include <linux/tty.h>
#include <linux/serial_reg.h>
#include <linux/spinlock.h>

#ifndef nNIKAL1400_kHasCreateProcReadEntry
#include <linux/seq_file.h>
#endif

#ifdef nNIKAL170_kIoctl32
   #include <linux/ioctl32.h>
#endif

#ifdef nNIKAL230_kHasCred
   #include <linux/cred.h>
#endif

#ifdef nNIKAL1400_kHasUidGid
   #include <linux/uidgid.h>
#endif

#ifdef nNIKAL250_kHasNamespacedGenetlink
#include <net/net_namespace.h>
#endif
#include <net/genetlink.h>
#include <linux/sysctl.h>

#include "nikal.h"

#ifdef MODULE_LICENSE
   MODULE_LICENSE("Copyright (c) 2002-2017 National Instruments Corporation.  All Rights Reserved.  Any and all use of the copyrighted materials is subject to the then current terms and conditions of the applicable license agreement, which can be found at <http://www.ni.com/linux/>.");
#endif

#ifdef MODULE_AUTHOR
   MODULE_AUTHOR("National Instruments Corporation");
#endif

#ifdef MODULE_DESCRIPTION
   MODULE_DESCRIPTION("National Instruments Kernel Abstraction Layer");
#endif

#define nNIKAL100_tWorkQueue struct work_struct
#define nNIKAL100_mInitWorkQueue INIT_WORK
#define nNIKAL100_mScheduleWorkQueue schedule_work


#define nNIKAL100_kPCIDeviceIDTableFooterSignature 0x70636964 
#define nNIKAL100_kUSBDeviceIDTableFooterSignature 0x75736274 



#ifdef IRQF_SHARED
   #define NIKAL_IRQ_SHARED IRQF_SHARED
#else
   #define NIKAL_IRQ_SHARED SA_SHIRQ
#endif


#ifdef IRQF_NO_SOFTIRQ_CALL
   #define NIKAL_IRQ_TRY_NO_SOFTIRQ_CALL IRQF_NO_SOFTIRQ_CALL
#else
   #define NIKAL_IRQ_TRY_NO_SOFTIRQ_CALL 0
#endif

#ifdef MAX_ORDER
#define nNIKAL1400_kMaxOrder MAX_ORDER
#else
#error ("MAX_ORDER not defined, but is required.");
#endif

#ifdef nNIKAL100_kUSBmsecTimeout
   #define nNIKAL100_kUSBControlTimeout USB_CTRL_GET_TIMEOUT
#else
   #define nNIKAL100_kUSBControlTimeout USB_CTRL_GET_TIMEOUT * HZ
#endif

#ifdef nNIKAL100_kDebuggingIsActive

#define KAL_STRINGIFY_HELPER(s) #s
#define KAL_STRINGIFY(s) KAL_STRINGIFY_HELPER(s)

#define KAL_DPRINT(fmt,...) \
      printk("[nikal:%p:%s:%d]: " fmt, current, __func__, __LINE__, ##__VA_ARGS__)
#define KAL_ASSERT(test,...)                                             \
           do                                                            \
           {   if (!(test))                                              \
               {                                                         \
                  panic("[nikal: " KAL_STRINGIFY(__LINE__) "]: ASSERT( " #test ") " __VA_ARGS__); \
               }                                                         \
           } while (0)
#define KAL_TRACE_VAR(a) KAL_DPRINT("\t" #a " = 0x%x (%d)\n",            \
                                          (unsigned int) (a), (int) (a))
#define nNIKAL120_mCheckStackUsage nNIKAL120_checkStackUsage()

#else

#define KAL_ASSERT(...)            do{}while(0)
#define KAL_DPRINT(...)            do{}while(0)
#define KAL_TRACE_VAR(...)         do{}while(0)
#define nNIKAL120_mCheckStackUsage do{}while(0)

#endif


#ifdef URB_ASYNC_UNLINK
   #define nNIKAL130_kURBAsyncUnlink
#endif


#define nNIKAL120_mRoundUp(x, r) ((x + (r-1))&(~(r-1)))

/** Compile time assert from Jon Jagger
 * http://www.jaggersoft.com/pubs/CVu11_3.html **/
#define nNIKAL100_compileTimeAssert(pred,comment) switch(0){case 0:case pred:;}


#ifndef PCI_CAP_ID_SSVID
  #define PCI_CAP_ID_SSVID 0x0D
#endif

#define nNIKAL100_kSubsystemIDOffset       4
#define nNIKAL100_kSubsystemVendorIDMask   0x0000FFFF
#define nNIKAL100_kSubsystemDeviceIDShift  0x10

#if defined(__x86_64__)
   #define nNIKAL220_kEnable32BitPageListAlloc
#endif

#if !defined(CONFIG_PCI)
static inline int pci_enable_msi(struct pci_dev *p) { return -ENOSYS; }
static inline void pci_disable_msi(struct pci_dev *p) { }
#endif

#ifdef nNIKAL240_kHasVM_RESERVED
#define nNIKAL240_kVM_RESERVED VM_RESERVED  
#else
#define nNIKAL240_kVM_RESERVED (VM_DONTEXPAND | VM_DONTDUMP)   
#endif





typedef struct sysinfo              nLinux_sysinfo;
typedef struct inode                nLinux_inode;
typedef struct file                 nLinux_fileHandle;
typedef struct vm_area_struct       nLinux_vmArea;
typedef struct vm_fault             nLinux_vmFault;
typedef struct page                 nLinux_physicalPage;
typedef struct pt_regs              nLinux_registers;
typedef struct semaphore            nLinux_semaphore;
typedef struct tasklet_struct       nLinux_tasklet;
typedef struct pci_device_id        nLinux_pciDeviceID;
typedef struct usb_device_id        nLinux_usbDeviceID;
typedef struct mm_struct            nLinux_mm;
typedef struct timeval              nLinux_timeValue;
typedef struct timer_list           nLinux_timerList;
typedef struct timespec             nLinux_timeSpec;
typedef struct pci_dev              nLinux_pciDevice;
typedef struct pci_bus              nLinux_pciBus;
typedef struct pci_driver           nLinux_pciDriver;
typedef struct usb_device           nLinux_usbDevice;
typedef struct usb_interface        nLinux_usbInterface;
typedef struct usb_driver           nLinux_usbDriver;
typedef struct usb_ctrlrequest      nLinux_usbCtrlRequest;
typedef struct urb                  nLinux_urb;
typedef struct usb_host_config      nLinux_usbConfigDescriptorWrapper;
typedef struct usb_host_interface   nLinux_usbInterfaceDescriptorWrapper;
typedef struct usb_host_endpoint    nLinux_usbEndpointDescriptorWrapper;
typedef struct usb_endpoint_descriptor nLinux_usbEndpointDescriptor;
typedef struct acpi_device          nLinux_acpiDevice;
typedef struct acpi_driver          nLinux_acpiDriver;
typedef struct list_head            nLinux_listHead;
typedef struct module               nLinux_driver;
typedef struct file_operations      nLinux_fileOperations;
typedef struct vm_operations_struct nLinux_vmOperations;
#ifdef nNIKAL150_kMutexMethod
typedef struct mutex                nLinux_mutex;
#else
typedef struct semaphore            nLinux_mutex;
#endif
typedef struct task_struct          nLinux_task;
typedef struct cdev                 nLinux_cdev;
typedef        spinlock_t           nLinux_spinlock;
typedef struct uart_driver          nLinux_uartDriver;
typedef struct uart_port            nLinux_uartPort;
typedef struct uart_ops             nLinux_uartOperations;
#ifdef nNIKAL100_kHasRS485Support
typedef struct txvr_ops             nLinux_uartRs485Operations;
typedef struct serial_rs485         nLinux_serialRs485;
#endif
typedef struct tty_port             nLinux_ttyPort;
typedef struct ktermios             nLinux_ktermios;
typedef struct serial_struct        nLinux_serialStruct;
typedef        tcflag_t             nLinux_termiosFlags;





static int __init nNIKAL100_initDriver(void);
static void __exit nNIKAL100_cleanupDriver(void);


#ifdef nNIKAL1400_kHasCreateProcReadEntry
static int nNIKAL200_procRead(char *page, char **start, off_t offset, int count, int *eof, void *data);
#else
static int nNIKAL250_procOpen(struct inode *inode, struct file *file);
#endif


static int nNIKAL100_open(nLinux_inode *the_inode, nLinux_fileHandle *filePtr);
static int nNIKAL100_release(nLinux_inode *the_inode, nLinux_fileHandle *filePtr);
static int nNIKAL100_ioctl(nLinux_inode *the_inode, nLinux_fileHandle *filePtr, unsigned int command, unsigned long param);
#ifdef HAVE_UNLOCKED_IOCTL
static long nNIKAL100_unlockedIoctl(nLinux_fileHandle *filePtr, unsigned int command, unsigned long param);
#endif
#ifdef HAVE_COMPAT_IOCTL
static long nNIKAL100_compatIoctl(nLinux_fileHandle *filePtr, unsigned int command, unsigned long param);
#endif
static ssize_t nNIKAL100_write(nLinux_fileHandle *filePtr, const char __user *buffer, size_t count, loff_t *offset);
static ssize_t nNIKAL220_write(nLinux_fileHandle *filePtr, const char __user *buffer, size_t count, loff_t *offset);
static ssize_t nNIKAL100_read(nLinux_fileHandle *filePtr, char __user *buffer, size_t count, loff_t *offset);
static ssize_t nNIKAL220_read(nLinux_fileHandle *filePtr, char __user *buffer, size_t count, loff_t *offset);
static int nNIKAL100_mmap(nLinux_fileHandle *filePtr, nLinux_vmArea *vma);
static int nNIKAL220_mmap(nLinux_fileHandle *filePtr, nLinux_vmArea *vma);
static loff_t nNIKAL100_llseek(nLinux_fileHandle *filePtr, loff_t inOffset, int seekFrom);


#if nNIKAL100_mSerialCoreIsSupported
static unsigned int nNIKAL100_uartOps_tx_empty(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_set_mctrl(nLinux_uartPort *uport, unsigned int mctrl);
static unsigned int nNIKAL100_uartOps_get_mctrl(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_stop_tx(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_start_tx(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_send_xchar(nLinux_uartPort *uport, char ch);
static void         nNIKAL100_uartOps_stop_rx(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_enable_ms(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_break_ctl(nLinux_uartPort *uport, int ctl);
static int          nNIKAL100_uartOps_startup(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_shutdown(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_flush_buffer(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_set_termios(nLinux_uartPort *uport, nLinux_ktermios *new_term, nLinux_ktermios *old_term);
static void         nNIKAL100_uartOps_pm(nLinux_uartPort *uport, unsigned int state, unsigned int oldstate);
#ifdef nNIKAL1400_kHasUartSetWake
static int          nNIKAL100_uartOps_set_wake(nLinux_uartPort *uport, unsigned int state);
#endif
static const char * nNIKAL100_uartOps_type(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_release_port(nLinux_uartPort *uport);
static int          nNIKAL100_uartOps_request_port(nLinux_uartPort *uport);
static void         nNIKAL100_uartOps_config_port(nLinux_uartPort *uport, int flags);
static int          nNIKAL100_uartOps_verify_port(nLinux_uartPort *uport, nLinux_serialStruct *new_serial);
static int          nNIKAL100_uartOps_ioctl(nLinux_uartPort *uport, unsigned int cmd, unsigned long arg);

#ifdef nNIKAL100_kHasRS485Support
static int          nNIKAL100_uartRs485Ops_enable_transceivers(nLinux_uartPort *uport);
static int          nNIKAL100_uartRs485Ops_disable_transceivers(nLinux_uartPort *uport);
static int          nNIKAL100_uartRs485Ops_config_rs485(nLinux_uartPort *uport, nLinux_serialRs485 *rs485);
#endif

#endif


static void nNIKAL220_vmaClosePageList(nLinux_vmArea *vma);
static void nNIKAL220_vmaClosePhysical(nLinux_vmArea *vma);

static void nNIKAL100_dispatchDPC (unsigned long interruptData);

static nNIKAL100_tBoolean nNIKAL100_isKernelContiguousPointer(const void *ptr);
static nNIKAL100_tUPtr nNIKAL230_kernelVirtualToPhysical(const void *ptr);

static int nNIKAL190_vmaPageFaultHandler(nLinux_vmArea *vma, nLinux_vmFault *vmf);
static int nNIKAL220_vmaPageFaultHandler(nLinux_vmArea *vma, nLinux_vmFault *vmf);

#ifdef nNIKAL160_kIRQRegs
   static irqreturn_t nNIKAL100_dispatchHardwareInterrupt (int irq, void *context,
      nLinux_registers *registers);
#else
   static irqreturn_t nNIKAL100_dispatchHardwareInterrupt (int irq, void *context);
#endif

static inline void nNIKAL100_listInit(nNIKAL100_tListNode *node)
{ node->prev = node->next = node; }

static inline void nNIKAL100_listAdd(nNIKAL100_tListNode *head, nNIKAL100_tListNode *node)
{
   node->prev = head->prev;
   node->next = head;
   head->prev->next = node;
   head->prev = node;
}

static inline void nNIKAL100_listRemove(nNIKAL100_tListNode *head, nNIKAL100_tListNode *node)
{
   node->prev->next = node->next;
   node->next->prev = node->prev;
   nNIKAL100_listInit(node);
}

static inline nNIKAL100_tBoolean nNIKAL100_listIsEmpty(nNIKAL100_tListNode *head)
{ return (head == head->next); }

static void *nNIKAL100_incrementPageRefcount(const void *ptr);
static nNIKAL100_tStatus nNIKAL100_remapPageRange(void *vmaPtr, nNIKAL100_tUPtr physAddr);

static inline void nNIKAL230_Mutex_init(nLinux_mutex *m)
{
#ifdef nNIKAL150_kMutexMethod
   mutex_init(m);
#else
   sema_init(m, 1);
#endif
}




typedef struct
{
   pid_t callingThreadID;
   nNIKAL100_tMemoryAddressType addressType;
} nNIKAL100_tMMapData;

typedef struct
{
   nNIKAL100_tDriver                 *driver;        

   struct nNIKAL100_tListNode        deviceListNode; 

   struct list_head                  procfsNode;     

   struct list_head                  interfacesHead; 

   struct list_head                  devicePropertiesHead; 
   struct list_head                  busPropertiesHead;    

   #define nNIKAL200_kMinorNumbersPerDevice 256
   dev_t                             cdevRegion;     
   struct kref                       kref;
   int                               addDeviceSuccess;

   
   struct proc_dir_entry   *rootDir;
   struct proc_dir_entry   *deviceInterfacesDir;
   struct proc_dir_entry   *busPropertiesDir;
   struct proc_dir_entry   *devicePropertiesDir;

   #define NIKAL_MAX_DEVICE_NAME 64
   char                    rootDirName[NIKAL_MAX_DEVICE_NAME];
   char                    nameString[NIKAL_MAX_DEVICE_NAME]; 

   nNIKAL100_tDeviceInfo   deviceInfo;
   void                    *deviceExtension;
   void                    *creationContext;
} nNIKAL200_tDevice;


typedef struct
{
   struct list_head propertiesNode;          
   const char *name;                         

   nNIKAL200_tPropertyClass   propClass;     

   void *handle;                             

   size_t length;
   const void *content;
} nNIKAL200_tPropertyData;

typedef struct
{
   nNIKAL200_tDevice       *parent;         

   struct list_head        interfaceNode;   

   struct list_head        propertiesHead;  

   dev_t                   cdevNumber;      

   struct cdev             cdev;
   void                    *context;        

   struct kref             kref;            

   struct proc_dir_entry   *interfaceDir;   
   struct proc_dir_entry   *interfacePropertiesDir; 

   const char              *name;           

   nNIKAL200_tPropertyData pathProperty;    
   char                    devicePath[NIKAL_MAX_DEVICE_NAME*3]; 
   char                    dirName[NIKAL_MAX_DEVICE_NAME]; 

   nLinux_mutex            notifyEventLock;
} nNIKAL200_tDeviceInterface;

typedef struct
{
   nNIKAL200_tDeviceInterface *interface;
   void *context;
} nNIKAL200_tFilePrivateData;

typedef struct
{
   pid_t childThreadID;
   nNIKAL100_tThreadFunction threadFunction;
   void *threadContext;
   void * volatile threadCreatedEvent;
   atomic_t counter;
   nNIKAL100_tWorkQueue workQueue;
} nNIKAL100_tCreateThreadParamBlock;

#ifdef nNIKAL160_kWorkqueueNonDelay
typedef struct
{
   nNIKAL100_tWorkQueue workQueue;
   void *data;
} nNIKAL160_tWorkItemWithDataPointer;
#endif



#if   defined(nNIKAL200_kCompletion)
typedef struct completion nNIKAL100_tSingleUseEvent;
#else
typedef struct
{
   wait_queue_head_t waitQueue;
   nNIKAL100_tU32 done;
} nNIKAL100_tSingleUseEvent;
#endif

typedef struct
{
   nNIKAL100_tDPCCallback func;
   void *context;
} nNIKAL100_tDPCCallbackSpec;

typedef struct
{
   nLinux_tasklet tasklet;
   nNIKAL100_tDPCCallbackSpec callback;
} nNIKAL100_tDPC;

typedef struct
{
   nNIKAL100_tUPtr signature;
   nNIKAL100_tU32 capacity;
   nNIKAL100_tU32 size;
   nNIKAL100_tDriver *driver;
   nLinux_pciDeviceID *table;
} nNIKAL100_tPCIDeviceIDTableFooter;

typedef struct
{
   nNIKAL100_tUPtr signature;
   nNIKAL100_tU32 capacity;
   nNIKAL100_tU32 size;
   nNIKAL100_tDriver *driver;
   nLinux_usbDeviceID *table;
} nNIKAL100_tUSBDeviceIDTableFooter;

typedef struct
{
   nNIKAL100_tUPtr key;
   nNIKAL100_tUPtr value;
} nNIKAL100_tMapKeyValuePair;

#define nNIKAL100_tMapBufferNodeSize (128)

#define nNIKAL100_tMapBufferCapacity \
   ((nNIKAL100_tMapBufferNodeSize - sizeof(nNIKAL100_tListNode) - sizeof(nNIKAL100_tUPtr))/sizeof(nNIKAL100_tMapKeyValuePair))

typedef struct
{
   nNIKAL100_tListNode listNode;
   nNIKAL100_tUPtr size;
   nNIKAL100_tMapKeyValuePair keyValuePairs[nNIKAL100_tMapBufferCapacity];
} nNIKAL100_tMapBufferNode;

#define nNIKAL100_tMapHashSizeInBits 7
#define nNIKAL100_tMapHashSize  (1 << nNIKAL100_tMapHashSizeInBits)

typedef nNIKAL100_tListNode nNIKAL100_tMap[nNIKAL100_tMapHashSize];

typedef struct
{
   nNIKAL100_tMap *mapPtr;
   nNIKAL100_tListNode *bucketPtr;
   nNIKAL100_tMapBufferNode *bufferPtr;
   nNIKAL100_tMapKeyValuePair *keyValuePtr;
} nNIKAL100_tMapIterator;

typedef struct
{
   nLinux_vmArea *vma;
   nNIKAL100_tUPtr address;
} nNIKAL100_tUserMemMapAddressPair;

typedef void (*tFuncWithWorkQueuePtrParam)(nNIKAL100_tWorkQueue*);

typedef struct
{
   nNIKAL100_tInterruptCallbackSpec *spec;

   struct list_head list;
   nLinux_spinlock lock;
} nNIKAL210_tInternalInterruptSpec;

#define nNIKAL220_kPageListFromUserBuffer (1UL<<31)
struct nNIKAL220_tPageList {
   size_t        num_pages;
   unsigned long flags;
   struct page*  pages[0];
};

struct nNIKAL220_tSGL {
#ifdef nNIKAL220_kHasChainedSGL
   struct sg_table table;
#else
   struct
   {
      struct scatterlist *sgl;
      int orig_nents;
      int nents;
   } table;
#endif
   struct scatterlist *iterator;
   struct device *dev;
   enum dma_data_direction dir;
   int mapped_nents;
};





static LIST_HEAD(nNIKAL210_sInterruptHandlerList);
static DEFINE_SPINLOCK(nNIKAL210_sInterruptHandlerListLock);
static LIST_HEAD(nNIKAL240_procfsHead);
static nLinux_mutex nNIKAL240_procfsLock;

static nLinux_semaphore nNIKAL100_sMMapSema;
static nLinux_semaphore nNIKAL100_sPageLockOperationSema;

static nNIKAL100_tMap nNIKAL100_sPageLockMap;
static nNIKAL100_tMap nNIKAL100_sMallocContiguousMap;
static nLinux_semaphore nNIKAL100_sMallocContiguousMapLock;
static DEFINE_MUTEX(nNIKAL220_sDriverSimulatedDeviceListLock);

static const nNIKAL100_tMMapData nNIKAL100_gMMapData =
{
   .callingThreadID = 0,
   .addressType = nNIKAL100_kMemoryAddressTypeInvalid
};

static const nNIKAL100_tMMapData* nNIKAL100_gMMapDataPtr = &nNIKAL100_gMMapData;

#ifndef nNIKAL1400_kHasCreateProcReadEntry
static nLinux_fileOperations nNIKAL250_procFops =
{
   .open = nNIKAL250_procOpen,
   .read = seq_read,
   .llseek = seq_lseek,
   .release = single_release
};
#endif

static nLinux_fileOperations nNIKAL100_fops =
{
   .open = nNIKAL100_open,
   .release = nNIKAL100_release,
#ifdef HAVE_UNLOCKED_IOCTL
   .unlocked_ioctl = nNIKAL100_unlockedIoctl,
#else
   .ioctl = nNIKAL100_ioctl,
#endif
#ifdef HAVE_COMPAT_IOCTL
   .compat_ioctl = nNIKAL100_compatIoctl,
#endif
   .read = nNIKAL100_read,
   .write = nNIKAL100_write,
   .mmap = nNIKAL100_mmap,
   .llseek = nNIKAL100_llseek,
};

static nLinux_fileOperations nNIKAL220_fops =
{
   .open = nNIKAL100_open,
   .release = nNIKAL100_release,
#ifdef HAVE_UNLOCKED_IOCTL
   .unlocked_ioctl = nNIKAL100_unlockedIoctl,
#else
   .ioctl = nNIKAL100_ioctl,
#endif
#ifdef HAVE_COMPAT_IOCTL
   .compat_ioctl = nNIKAL100_compatIoctl,
#endif
   .read = nNIKAL220_read,
   .write = nNIKAL220_write,
   .mmap = nNIKAL220_mmap,
   .llseek = nNIKAL100_llseek,
};

#if nNIKAL100_mSerialCoreIsSupported
static nLinux_uartOperations nNIKAL100_uartOps =
{
   .tx_empty      = nNIKAL100_uartOps_tx_empty,
   .set_mctrl     = nNIKAL100_uartOps_set_mctrl,
   .get_mctrl     = nNIKAL100_uartOps_get_mctrl,
   .stop_tx       = nNIKAL100_uartOps_stop_tx,
   .start_tx      = nNIKAL100_uartOps_start_tx,
   .send_xchar    = nNIKAL100_uartOps_send_xchar,
   .stop_rx       = nNIKAL100_uartOps_stop_rx,
   .enable_ms     = nNIKAL100_uartOps_enable_ms,
   .break_ctl     = nNIKAL100_uartOps_break_ctl,
   .startup       = nNIKAL100_uartOps_startup,
   .shutdown      = nNIKAL100_uartOps_shutdown,
   .flush_buffer  = nNIKAL100_uartOps_flush_buffer,
   .set_termios   = nNIKAL100_uartOps_set_termios,
   .pm            = nNIKAL100_uartOps_pm,
#ifdef nNIKAL1400_kHasUartSetWake
   .set_wake      = nNIKAL100_uartOps_set_wake,
#endif
   .type          = nNIKAL100_uartOps_type,
   .release_port  = nNIKAL100_uartOps_release_port,
   .request_port  = nNIKAL100_uartOps_request_port,
   .config_port   = nNIKAL100_uartOps_config_port,
   .verify_port   = nNIKAL100_uartOps_verify_port,
   .ioctl         = nNIKAL100_uartOps_ioctl,
};

#ifdef nNIKAL100_kHasRS485Support
static nLinux_uartRs485Operations nNIKAL100_uartRs485Ops =
{
   .enable_transceivers    =  nNIKAL100_uartRs485Ops_enable_transceivers,
   .disable_transceivers   =  nNIKAL100_uartRs485Ops_disable_transceivers,
#ifndef nNIKAL1500_kHasRS485ConfigOnUart
   .config_rs485           =  nNIKAL100_uartRs485Ops_config_rs485,
#endif
};
#endif

#endif

static nLinux_vmOperations nNIKAL100_vmaOps =
{
   .fault = nNIKAL190_vmaPageFaultHandler,
};

static nLinux_vmOperations nNIKAL220_vmaOpsPageList = {
   .close = nNIKAL220_vmaClosePageList,
   .fault = nNIKAL220_vmaPageFaultHandler,
};

static nLinux_vmOperations nNIKAL220_vmaOpsPhysical = {
   .close = nNIKAL220_vmaClosePhysical,
};

static const char *nNIVersion_versionStringArray[] =
{
   "nNIVersion_CompanyName=National Instruments Corporation",

   "nNIVersion_LegalCopyright=Copyright (c) 2002-2017 National Instruments Corporation. "
   "All Rights Reserved. Any and all use of the copyrighted materials is subject to the "
   "then current terms and conditions of the applicable license agreement, "
   "which can be found at <http://www.ni.com/linux/>.",

   "nNIVersion_ProductName=NIKAL",
   "nNIVersion_OriginalFilename=" nNIKAL100_kTargetName,
   "nNIVersion_FileDescription=NI-KAL Driver",

   #ifdef nNIKAL100_kDebuggingIsActive
      "nNIVersion_ProductVersion=17.0.0f0 debug build",
      "nNIVersion_FileVersion=17.0.0f0 debug build",
      "nNIVersion_InternalName=NIKAL 17.0.0f0 debug build",
   #else
      "nNIVersion_ProductVersion=17.0.0f0",
      "nNIVersion_FileVersion=17.0.0f0",
      "nNIVersion_InternalName=NIKAL 17.0.0f0",
   #endif

   NULL
};

static nNIKAL100_tUPtr nNIKAL120_sStackSize;
static nNIKAL100_tUPtr nNIKAL120_sMaxStackUsageSeen = 0;
#define nNIKAL120_kMinStackUsageForWarning 0xc00  

static struct proc_dir_entry *nNIKAL200_sProcRootDir;

static nNIKAL200_tDevice          nNIKAL200_sPALPseudoDevice = {
   .driver         = NULL,
};
static nNIKAL200_tDeviceInterface nNIKAL200_sPALPseudoDeviceInterface = {
   .parent         = &nNIKAL200_sPALPseudoDevice,
   .name           = "nipalk"
};











inline nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_statusSelect(nNIKAL100_tStatus oldStatus, nNIKAL100_tStatus newStatus)
{
   if (nNIKAL100_statusIsFatal(oldStatus))
      return oldStatus;
   if (nNIKAL100_statusIsFatal(newStatus))
      return newStatus;
   if (oldStatus != 0)
      return oldStatus;
   return newStatus;
}

inline nNIKAL100_tStatus nNIKAL100_convertLinuxToKALStatus(int linuxStatus)
{
   switch(linuxStatus)
   {
      case 0:
         return nNIKAL100_kStatusSuccess;
      case -ENOMEM:
         return nNIKAL100_kStatusMemoryFull;
      case -ENAMETOOLONG:
         return nNIKAL100_kStatusValueConflict;
      case -EINVAL:
         return nNIKAL100_kStatusValueConflict;
      case -EINTR:
         return nNIKAL100_kStatusWaitInterrupted;
      case -EBUSY:
         return nNIKAL100_kStatusResourceBusy;
      case -EPERM:
         return nNIKAL100_kStatusOSFault;
      case -ECONNRESET:
         return nNIKAL100_kStatusTransferAborted;
      case -ENOENT:
         return nNIKAL100_kStatusTransferAborted;
      case -ENODEV:
         return nNIKAL100_kStatusDeviceNotFound;
      case -EPIPE:
      case -ECONNREFUSED:
         return nNIKAL100_kStatusCommunicationsFault;
      case -EMSGSIZE:
         return nNIKAL100_kStatusBadCount;
      case -EXDEV:
         return nNIKAL100_kStatusTransferStopped;
      case -ETIMEDOUT:
         return nNIKAL100_kStatusTransferTimedOut;
      case -ECOMM:
         return nNIKAL100_kStatusPhysicalBufferFull;
      case -ENOSR:
         return nNIKAL100_kStatusPhysicalBufferEmpty;
      case -EINPROGRESS:
         return nNIKAL100_kStatusTransferInProgress;
      case -EREMOTEIO:
         return nNIKAL100_kStatusCommunicationsFault;
      case -ENXIO:
         return nNIKAL100_kStatusBadMode;
      case -EFBIG:
         return nNIKAL100_kStatusBadCount;
      case -EAGAIN:
         return nNIKAL100_kStatusBadOffset;
      case -ENOSPC:
         return nNIKAL100_kStatusResourceBusy;
      
      case -ESHUTDOWN:
      case -EPROTO:
      case -EILSEQ:
      case -EOVERFLOW:
         return nNIKAL100_kStatusHardwareFault;
      case -ERESTARTSYS:
         return nNIKAL100_kStatusWaitInterrupted;
      case -ENOSYS:
         return nNIKAL100_kStatusFeatureNotSupported;
      case 256:
         
         return nNIKAL100_kStatusOSFault;
      default:
         KAL_ASSERT(0, "Unknown linux error code value!: %d\n", linuxStatus);
         return nNIKAL100_kStatusOSFault;
   }
}

inline int nNIKAL100_convertKALToLinuxStatus(nNIKAL100_tStatus kalStatus)
{
   switch(kalStatus)
   {
      case nNIKAL100_kStatusSuccess:
         return 0;
      case nNIKAL100_kStatusMemoryFull:
         return -ENOMEM;
      case nNIKAL100_kStatusValueConflict:
         return -EINVAL;
      case nNIKAL100_kStatusWaitInterrupted:
         return -EINTR;
      case nNIKAL100_kStatusResourceBusy:
         return -EBUSY;
      case nNIKAL100_kStatusDeviceNotFound:
         return -ENODEV;
      case nNIKAL100_kStatusTransferTimedOut:
         return -ETIMEDOUT;
      case nNIKAL100_kStatusFeatureNotSupported:
         return -ENOSYS;
      case nNIKAL100_kStatusNoIoctlSpecified:
         return -ENOIOCTLCMD;
      default:
         KAL_ASSERT(0, "KAL status %d does not have an equivalent linux status!\n", (int)kalStatus);
         return (int)kalStatus;
   }
}


static inline long nNIKAL1600_currentTaskGetUserPages(
   unsigned long start,
   unsigned long nr_pages,
   int write,
   int force,
   struct page **pages,
   struct vm_area_struct **vmas)
{
#if defined(nNIKAL1700_kGetUserPagesCombinedFlags)
   unsigned int flags = 0;
   flags |= write ? FOLL_WRITE : 0;
   flags |= force ? FOLL_FORCE : 0;
   return get_user_pages(start, nr_pages, flags, pages, vmas);
#elif defined(nNIKAL1600_kGetUserPagesImpliesCurrentTask)
   return get_user_pages(start, nr_pages, write, force, pages, vmas);
#else
   return get_user_pages(current, current->mm, start, nr_pages, write, force, pages, vmas);
#endif
}

static nNIKAL100_tStatus nNIKAL1400_createProcess(const char **argv, int wait, int *exitCode)
{
   char path[128];
   char *envp[] = { "HOME=/",
                    "PATH=/usr/local/natinst/nikal/bin:/sbin:/usr/sbin:/usr/bin:/bin:/usr/local/sbin",
                    NULL
               };
   int ret;

#ifdef nNIKAL240_kHasUMHConstants

   
   switch(wait)
   {
      case -1:
         wait = UMH_NO_WAIT;
         break;
      case 0:
         wait = UMH_WAIT_EXEC;
         break;
      case 1:
         wait = UMH_WAIT_PROC;
         break;
      default:
         return nNIKAL100_kStatusInvalidParameter;
   }
#endif

   snprintf(path, sizeof(path), "/usr/local/sbin/%s", argv[0]);
   argv[0] = path;

   ret = call_usermodehelper((char*)argv[0], (char**)argv, envp, wait);
   if (ret < 0)
   {
      KAL_DPRINT("nNIKAL1400_createProcess(%s, ...) failed: %d\n", path, ret);
      return nNIKAL100_kStatusOSFault;
   }
   if (exitCode)
      *exitCode = ret;
   return nNIKAL100_kStatusSuccess;
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL200_createProcess(const char **argv, nNIKAL100_tBoolean wait)
{
   return  nNIKAL1400_createProcess(argv, wait, NULL);
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL1400_createProcessAndWait(const char **argv, int *exitCode)
{
   return nNIKAL1400_createProcess(argv, 1, exitCode);
}


static char nNIKAL200_sNotificationScript[] = "nidevnode";

nNIKAL100_cc void nNIKAL240_notifyUserspaceAddEvent(void *interface)
{
   nNIKAL200_tDeviceInterface *di = interface;
   char major_str[6];
   char minor_str[6];

   const char *argv[] = { nNIKAL200_sNotificationScript,
                           "add",
                           di->devicePath,
                           major_str,
                           minor_str,
                           NULL
                        };

   sprintf(major_str, "%d", MAJOR(di->cdevNumber));
   sprintf(minor_str, "%d", MINOR(di->cdevNumber));

   nNIKAL150_acquireMutex(&di->notifyEventLock);

   nNIKAL200_createProcess(argv, 1 );

   nNIKAL150_releaseMutex(&di->notifyEventLock);
}

static inline void nNIKAL200_notifyUserspaceRemoveEvent(nNIKAL200_tDeviceInterface *di)
{
   const char *argv[] = { nNIKAL200_sNotificationScript,
                           "remove",
                           di->devicePath,
                           NULL
                        };

   nNIKAL150_acquireMutex(&di->notifyEventLock);

   nNIKAL200_createProcess(argv, 1 );

   nNIKAL150_releaseMutex(&di->notifyEventLock);
}



static inline void nNIKAL100_mapInit(nNIKAL100_tMap *map)
{
   nNIKAL100_tU32 i;
   for (i=0; i< nNIKAL100_tMapHashSize; ++i)
   {
      nNIKAL100_listInit(&((*map)[i]));
   }
}


static void nNIKAL100_mapCleanup(nNIKAL100_tMap *map)
{
   nNIKAL100_tListNode *iterator, *temp;
   nNIKAL100_tMapBufferNode *bufferPtr;
   nNIKAL100_tU32 i, j;

   for (i=0; i< nNIKAL100_tMapHashSize; ++i)
   {
      if (nNIKAL100_listIsEmpty(&((*map)[i])))
         continue;

      iterator = (*map)[i].next;
      while (iterator != &((*map)[i]))
      {
         bufferPtr = (nNIKAL100_tMapBufferNode*)iterator;
         if (bufferPtr->size != 0)
         {
            for (j=0; j<bufferPtr->size; ++j)
            {
               KAL_DPRINT("nNIKAL100_mapCleanup: found leaked map node: key: %lx, value: %lx, mapBuffer: %p\n",
                  bufferPtr->keyValuePairs[j].key, bufferPtr->keyValuePairs[j].value, bufferPtr);
            }
         }

         temp = iterator;
         iterator = iterator->next;
         nNIKAL100_listRemove(&((*map)[i]), temp);

         KAL_DPRINT("nNIKAL100_mapCleanup: deleting mapBuffer: %p\n", bufferPtr);
         nNIKAL100_free(bufferPtr);
      }
   }
}


static inline nNIKAL100_tU32 nNIKAL100_mapHashAddress(nNIKAL100_tUPtr key)
{ return ((key >> PAGE_SHIFT) % nNIKAL100_tMapHashSize); }



static nNIKAL100_tStatus nNIKAL100_mapInsert(nNIKAL100_tMap *map, nNIKAL100_tUPtr key, nNIKAL100_tUPtr value)
{
   nNIKAL100_tListNode *listIterator;
   nNIKAL100_tMapBufferNode *bufferPtr;
   nNIKAL100_tU32 hashValue = nNIKAL100_mapHashAddress(key);

   
   listIterator = ((*map)[hashValue]).next;
   for (;listIterator != &((*map)[hashValue]); listIterator = listIterator->next)
   {
      
      bufferPtr = (nNIKAL100_tMapBufferNode*)listIterator;
      if (bufferPtr->size == nNIKAL100_tMapBufferCapacity)
         continue;

      bufferPtr->keyValuePairs[bufferPtr->size].key = key;
      bufferPtr->keyValuePairs[bufferPtr->size].value = value;
      ++(bufferPtr->size);

      
      return nNIKAL100_kStatusSuccess;
   }

   
   
   bufferPtr = (nNIKAL100_tMapBufferNode*)kmalloc(
      sizeof(nNIKAL100_tMapBufferNode), GFP_KERNEL);
   if (bufferPtr == NULL)
   {
      KAL_DPRINT("nNIKAL100_mapInsert: Failed to allocate bufNode: map: %p, key: %lx(hash:%lx), value: %lx\n", map, key, (unsigned long)hashValue, value);
      return nNIKAL100_kStatusMemoryFull;
   }

   nNIKAL100_listAdd(&((*map)[hashValue]), &(bufferPtr->listNode));
   bufferPtr->size = 1;
   bufferPtr->keyValuePairs[0].key = key;
   bufferPtr->keyValuePairs[0].value = value;

   
   return nNIKAL100_kStatusSuccess;
}


static void nNIKAL100_mapFind(nNIKAL100_tMap *map, nNIKAL100_tUPtr key, nNIKAL100_tMapIterator *iterator)
{
   nNIKAL100_tListNode *listIterator;
   nNIKAL100_tMapBufferNode *bufferPtr;
   nNIKAL100_tU32 hashValue = nNIKAL100_mapHashAddress(key);
   nNIKAL100_tU32 i;

   listIterator = ((*map)[hashValue]).next;
   for (;listIterator != &((*map)[hashValue]); listIterator = listIterator->next)
   {
      
      bufferPtr = (nNIKAL100_tMapBufferNode*)listIterator;

      for (i=0; i<bufferPtr->size; ++i)
      {
         if (key == bufferPtr->keyValuePairs[i].key)
         {
            
            iterator->mapPtr = map;
            iterator->bucketPtr = &((*map)[hashValue]);
            iterator->bufferPtr = bufferPtr;
            iterator->keyValuePtr = &(bufferPtr->keyValuePairs[i]);

            
            return;
         }
      }
   }

   
   iterator->mapPtr = NULL;
   iterator->bucketPtr = NULL;
   iterator->bufferPtr = NULL;
   iterator->keyValuePtr = NULL;
   return;
}


static void nNIKAL100_mapRemove(nNIKAL100_tMap *map, const nNIKAL100_tMapIterator *iterator)
{
   nNIKAL100_tU32 numberOfEntriesToShiftUp;
   nNIKAL100_tMapBufferNode *bufferPtr = iterator->bufferPtr;
   nNIKAL100_tMapKeyValuePair *keyValuePtr = iterator->keyValuePtr;

   if (iterator->mapPtr == NULL)
   {
      KAL_DPRINT("nNIKAL100_mapRemove: iterator not valid: mapPtr: %p, bucketPtr: %p, bufferPtr: %p, keyValuePtr: %p\n",
         iterator->mapPtr, iterator->bucketPtr, iterator->bufferPtr, iterator->keyValuePtr);
      return;
   }

   

   
   KAL_ASSERT(bufferPtr->size > 0, "nNIKAL100_mapRemove: trying to remove from 0 sized buffer\n");
   --(bufferPtr->size);
   numberOfEntriesToShiftUp = bufferPtr->size - (iterator->keyValuePtr - &(bufferPtr->keyValuePairs[0]));
   KAL_ASSERT(numberOfEntriesToShiftUp <= bufferPtr->size, "nNIKAL100_mapRemove: trying to shift too many entries during remove.\n");

   for (; numberOfEntriesToShiftUp>0; --numberOfEntriesToShiftUp)
   {
      *keyValuePtr = *(keyValuePtr+1);
      ++keyValuePtr;
   }

   
   if (bufferPtr->size == 0)
   {
      nNIKAL100_listRemove(iterator->bucketPtr, &(bufferPtr->listNode));
      
      kfree(bufferPtr);
   }
}


static inline void nNIKAL100_pageAlignMemoryBlock(void *inPtr, nNIKAL100_tUPtr size, void **pageAlignedPtr,
   nNIKAL100_tUPtr *pageAlignedSize, nNIKAL100_tUPtr *offset)
{
   nNIKAL100_tUPtr pageSize, addr, outAddr, endAddr;
   pageSize = nNIKAL100_getPageSize();
   addr = (nNIKAL100_tUPtr)inPtr;
   *offset = addr & (pageSize-1);
   outAddr = addr & (~(pageSize-1));
   endAddr = ((addr+size) + (pageSize-1)) & (~(pageSize-1));
   *pageAlignedSize = endAddr - outAddr;
   *pageAlignedPtr = (void*)outAddr;
}

static inline dev_t nNIKAL200_allocateDeviceMinorRegion(nNIKAL100_tDriver *driver)
{
   
   dev_t region = driver->number + nNIKAL200_kMinorNumbersPerDevice;
   unsigned int major = MAJOR(region);

   for (; major == MAJOR(region); region += nNIKAL200_kMinorNumbersPerDevice)
   {
      if (register_chrdev_region(region, nNIKAL200_kMinorNumbersPerDevice, driver->name) == 0)
         return region;
   }

   return 0;
}

static inline void nNIKAL200_freeDeviceMinorRegion(nNIKAL100_tDriver *driver, dev_t region)
{
   unregister_chrdev_region(region, nNIKAL200_kMinorNumbersPerDevice);
}

static inline void *nNIKAL200_mallocZeroed(size_t size, int flags)
{
#ifdef nNIKAL200_kHasKzalloc
   return kzalloc(size, flags);
#else
   void * mem = kmalloc(size, flags);
   if (mem)
      memset(mem, 0, size);
   return mem;
#endif
}


static inline nNIKAL200_tDevice *nNIKAL200_constructDevice(nNIKAL100_tDriver *driver)
{
   nNIKAL200_tDevice *d = nNIKAL200_mallocZeroed(sizeof(*d), GFP_KERNEL);
   if (likely(d)) {
      d->driver = driver;
      d->cdevRegion = nNIKAL200_allocateDeviceMinorRegion(driver);

      INIT_LIST_HEAD(&d->interfacesHead);
      INIT_LIST_HEAD(&d->devicePropertiesHead);
      INIT_LIST_HEAD(&d->busPropertiesHead);

      kref_init(&d->kref);
      d->addDeviceSuccess = 0;
   }
   return d;
}
static void nNIKAL200_destructDevice(struct kref *k)
{
   nNIKAL200_tDevice *d = container_of(k, nNIKAL200_tDevice, kref);
   KAL_ASSERT(list_empty(&d->interfacesHead), "Device freed before freeing interfaces!\n");
   if (d->addDeviceSuccess && d->driver->destroyDevice)
   {
      d->driver->destroyDevice(d->deviceExtension);
   }
   nNIKAL200_freeDeviceMinorRegion(d->driver, d->cdevRegion);
   kfree(d);
}


static nNIKAL100_tBoolean nNIKAL240_checkProcfs_nolock(const char *name)
{
   nNIKAL200_tDevice *d;
   list_for_each_entry(d, &nNIKAL240_procfsHead, procfsNode)
   {
      if (strcmp(name, d->nameString) == 0)
      {
         return nNIKAL100_kTrue;
      }
   }
   return nNIKAL100_kFalse;
}

static inline nNIKAL100_tStatus nNIKAL200_createDeviceProcDirs(const char *name, nNIKAL200_tDevice *d)
{
   nNIKAL100_tBoolean oldDeviceProcfsExists;
   int watchdog = 50;
   int jiffies_100m = msecs_to_jiffies(100);

   KAL_ASSERT(d);
   KAL_ASSERT(name);

   do
   {
      nNIKAL150_acquireMutex(&nNIKAL240_procfsLock);
      oldDeviceProcfsExists = nNIKAL240_checkProcfs_nolock(name);
      if (oldDeviceProcfsExists)
      {
         nNIKAL150_releaseMutex(&nNIKAL240_procfsLock);

         if (watchdog == 0)
         {
            KAL_DPRINT("Error: Timed out waiting for old proc directory /proc/driver/ni/%s to go away\n", name);
            return nNIKAL100_kStatusResourceBusy;
         }

         
         nNIKAL100_sleepTimeout(jiffies_100m);
         --watchdog;
      }
      else
      {
         list_add_tail(&d->procfsNode, &nNIKAL240_procfsHead);
         nNIKAL150_releaseMutex(&nNIKAL240_procfsLock);
      }
   } while (oldDeviceProcfsExists);

   snprintf(d->rootDirName, NIKAL_MAX_DEVICE_NAME, "%s", name);

   d->rootDir = proc_mkdir(d->rootDirName, nNIKAL200_sProcRootDir);
   if (!d->rootDir)
      goto err_out;

   d->deviceInterfacesDir = proc_mkdir(nNIKAL200_kDeviceInterfacesDir, d->rootDir);
   if (!d->deviceInterfacesDir)
      goto err_device_interfaces;

   d->busPropertiesDir    = proc_mkdir(nNIKAL200_kBusPropertiesDir, d->rootDir);
   if (!d->busPropertiesDir)
      goto err_bus_properties;

   d->devicePropertiesDir = proc_mkdir(nNIKAL200_kDevicePropertiesDir, d->rootDir);
   if (!d->devicePropertiesDir)
      goto err_device_properties;

   return nNIKAL100_kStatusSuccess;

err_device_properties:
   remove_proc_entry(nNIKAL200_kBusPropertiesDir, d->rootDir);
err_bus_properties:
   remove_proc_entry(nNIKAL200_kDeviceInterfacesDir, d->rootDir);
err_device_interfaces:
   remove_proc_entry(name, nNIKAL200_sProcRootDir);
err_out:
   nNIKAL150_acquireMutex(&nNIKAL240_procfsLock);
   list_del_init(&d->procfsNode);
   nNIKAL150_releaseMutex(&nNIKAL240_procfsLock);
   return nNIKAL100_kStatusMemoryFull;
}

static inline void nNIKAL200_destroyDeviceProcDirs(nNIKAL200_tDevice *d)
{
   KAL_ASSERT(d);

   remove_proc_entry(nNIKAL200_kDevicePropertiesDir, d->rootDir);
   remove_proc_entry(nNIKAL200_kBusPropertiesDir, d->rootDir);
   remove_proc_entry(nNIKAL200_kDeviceInterfacesDir, d->rootDir);
   remove_proc_entry(d->rootDirName, nNIKAL200_sProcRootDir);

   nNIKAL150_acquireMutex(&nNIKAL240_procfsLock);
   list_del_init(&d->procfsNode);
   nNIKAL150_releaseMutex(&nNIKAL240_procfsLock);
}
static inline void nNIKAL200_addInterfaceToDevice(nNIKAL200_tDevice *d, nNIKAL200_tDeviceInterface *di)
{
   list_add(&di->interfaceNode, &d->interfacesHead);
}

static inline void nNIKAL200_removeInterfaceFromDevice(nNIKAL200_tDeviceInterface *di)
{
   list_del(&di->interfaceNode);
}

static inline nNIKAL200_tPropertyData *nNIKAL200_constructPropertyData(const char *name,
                                                                       nNIKAL200_tPropertyClass class,
                                                                       void *handle,
                                                                       const char *content,
                                                                       size_t length)
{
   nNIKAL200_tPropertyData *pd = nNIKAL200_mallocZeroed(sizeof(*pd), GFP_KERNEL);

   KAL_ASSERT(pd->length <= PAGE_SIZE, "Cannot add property with length greater than a page.\n");

   if (likely(pd)) {
      pd->name      = name;
      pd->propClass = class;
      pd->handle    = handle;
      pd->content   = content;
      pd->length    = length;
   }
   return pd;
}

static inline void nNIKAL200_destructPropertyData(nNIKAL200_tPropertyData * pd)
{
   kfree(pd);
}


static inline nNIKAL200_tDeviceInterface *nNIKAL200_constructDeviceInterface(nNIKAL200_tDevice *parent)
{
   nNIKAL200_tDeviceInterface *di = nNIKAL200_mallocZeroed(sizeof(*di), GFP_KERNEL);
   if (likely(di)) {
      di->parent = parent;
      kref_get(&(parent->kref));
      INIT_LIST_HEAD(&di->propertiesHead);
      nNIKAL230_Mutex_init(&di->notifyEventLock);
      kref_init(&di->kref);
   }
   return di;
}

static void nNIKAL200_destructDeviceInterface(struct kref *k)
{
   nNIKAL200_tDeviceInterface *di = container_of(k, nNIKAL200_tDeviceInterface, kref);
   kref_put(&(di->parent->kref), nNIKAL200_destructDevice);
   kfree(di);
}

static nNIKAL100_tStatus nNIKAL200_createKALInterfaceProperties(nNIKAL200_tDeviceInterface *di, nNIKAL100_tBoolean isMultiInterfaceFix)
{
   char *devnodePathFormat;

   KAL_ASSERT(di);
   
   if (isMultiInterfaceFix)
      devnodePathFormat = "/dev/ni/%s\\%s";
   else
      devnodePathFormat = "/dev/ni/%s\\%s\\"; 

   snprintf(di->devicePath, sizeof(di->devicePath), devnodePathFormat,
                                             di->parent->nameString, di->dirName);

   di->devicePath[sizeof(di->devicePath)-1] = '\0';

   di->pathProperty.name    = nNIKAL200_kDeviceInterfacesPathFile;
   di->pathProperty.content = di->devicePath;
   di->pathProperty.length  = strlen(di->devicePath) + 1;

#ifdef nNIKAL1400_kHasCreateProcReadEntry
   return create_proc_read_entry(nNIKAL200_kDeviceInterfacesPathFile,
                                  S_IRUGO, 
                                  di->interfaceDir,
                                  nNIKAL200_procRead,
                                  &di->pathProperty)
          ? nNIKAL100_kStatusSuccess : nNIKAL100_kStatusMemoryFull;
#else
   return proc_create_data(nNIKAL200_kDeviceInterfacesPathFile,
                                  S_IRUGO, 
                                  di->interfaceDir,
                                  &nNIKAL250_procFops,
                                  &di->pathProperty)
          ? nNIKAL100_kStatusSuccess : nNIKAL100_kStatusMemoryFull;
#endif
}

static void nNIKAL200_destroyKALInterfaceProperties(nNIKAL200_tDeviceInterface *di)
{
   if (likely(di))
      remove_proc_entry(nNIKAL200_kDeviceInterfacesPathFile, di->interfaceDir);
}


static void *nNIKAL220_registerDeviceInterfaceCommon(nNIKAL100_tDriver *driver,
                                                     void *device,
                                                     const char *name,
                                                     size_t interfaceNum,
                                                     void *context,
                                                     nLinux_fileOperations *fops,
                                                     nNIKAL100_tBoolean isMultiInterfaceFix,
                                                     nNIKAL100_tBoolean doNotify)
{
   int                        err;
   nNIKAL100_tStatus          status;
   nNIKAL200_tDeviceInterface *di;
   size_t                     strLength;

   nNIKAL200_tDevice          *d = device;

   KAL_ASSERT(driver);
   KAL_ASSERT(name);
   KAL_ASSERT(d);
   KAL_ASSERT(d->deviceInterfacesDir);

   di = nNIKAL200_constructDeviceInterface(d);

   if (unlikely(!di))
      goto out;

   di->context       = context;
   di->cdev.owner    = driver->module;
   di->name          = name;

   if (isMultiInterfaceFix)
   {
      snprintf(di->dirName, sizeof(di->dirName), "%s\\%zu", name, interfaceNum);
   }
   else
   {
      strLength = strlen(name)+1;
      memcpy(di->dirName, name, strLength);
   }

   di->interfaceDir  = proc_mkdir(di->dirName, d->deviceInterfacesDir);

   if (!di->interfaceDir) {
      KAL_DPRINT("Error: Could not create proc directory: '%s'\n", di->dirName);
      goto out_destruct;
   }

   di->interfacePropertiesDir = proc_mkdir(nNIKAL200_kDeviceInterfacesPropertiesDir, di->interfaceDir);

   if (!di->interfacePropertiesDir) {
      KAL_DPRINT("Error: Could not create proc directory: '%s/"
                     nNIKAL200_kDeviceInterfacesPropertiesDir "/'\n", di->dirName);
      goto out_remove_interfacedir;
   }

   status = nNIKAL200_createKALInterfaceProperties(di, isMultiInterfaceFix);

   if (nNIKAL100_statusIsFatal(status)) {
      KAL_DPRINT("Error: Could not create proc entry for device path\n");
      goto out_remove_both_dirs;
   }

   nNIKAL200_addInterfaceToDevice(d, di);

   
   di->cdevNumber = d->cdevRegion + interfaceNum;

   cdev_init(&di->cdev, fops);

   err = cdev_add(&di->cdev, di->cdevNumber, 1);

   if (unlikely(err)) {
      KAL_DPRINT("Error %d. Could not register as char driver: '%s'\n", err, di->name);
      goto out_remove_interface;
   }

   KAL_DPRINT("Registered device interface: device=%p, name=\"%s\", MAJOR(dev)=%d, MINOR(dev)=%d\n",
                              d, name, MAJOR(di->cdevNumber), MINOR(di->cdevNumber));

   
   if (doNotify) {
      nNIKAL240_notifyUserspaceAddEvent(di);
   }

   return di;

out_remove_interface:
   nNIKAL200_removeInterfaceFromDevice(di);
   nNIKAL200_destroyKALInterfaceProperties(di);
out_remove_both_dirs:
   remove_proc_entry(nNIKAL200_kDeviceInterfacesPropertiesDir, di->interfaceDir);
out_remove_interfacedir:
   remove_proc_entry(di->dirName, d->deviceInterfacesDir);
out_destruct:
   nNIKAL200_destructDeviceInterface(&di->kref);
out:
   return NULL;

}


nNIKAL100_cc void *nNIKAL200_registerDeviceInterface(nNIKAL100_tDriver *driver,
                                                     void *device,
                                                     const char *name,
                                                     size_t interfaceNum,
                                                     void *context)
{
   return nNIKAL220_registerDeviceInterfaceCommon(driver, device,name, interfaceNum,
                                                  context, &nNIKAL100_fops, nNIKAL100_kFalse,
                                                  nNIKAL100_kTrue);
}


nNIKAL100_cc void *nNIKAL220_registerDeviceInterface(nNIKAL100_tDriver *driver,
                                                     void *device,
                                                     const char *name,
                                                     size_t interfaceNum,
                                                     void *context)
{
   return nNIKAL220_registerDeviceInterfaceCommon(driver, device,name, interfaceNum,
                                                  context, &nNIKAL220_fops, nNIKAL100_kFalse,
                                                  nNIKAL100_kTrue);
}


nNIKAL100_cc void *nNIKAL230_registerDeviceInterface(nNIKAL100_tDriver *driver,
                                                     void *device,
                                                     const char *name,
                                                     size_t interfaceNum,
                                                     void *context)
{
   return nNIKAL220_registerDeviceInterfaceCommon(driver, device,name, interfaceNum,
                                                  context, &nNIKAL220_fops, nNIKAL100_kTrue,
                                                  nNIKAL100_kTrue);
}


nNIKAL100_cc void *nNIKAL240_registerDeviceInterface(nNIKAL100_tDriver *driver,
                                                     void *device,
                                                     const char *name,
                                                     size_t interfaceNum,
                                                     void *context)
{
   return nNIKAL220_registerDeviceInterfaceCommon(driver, device,name, interfaceNum,
                                                  context, &nNIKAL220_fops, nNIKAL100_kTrue,
                                                  nNIKAL100_kFalse);
}


nNIKAL100_cc void nNIKAL200_unregisterDeviceInterface(void *interface)
{
   nNIKAL200_tDeviceInterface *di = interface;

   if (unlikely(!di))
      return;

   cdev_del(&di->cdev);

   nNIKAL200_removeInterfaceFromDevice(di);
   nNIKAL200_destroyKALInterfaceProperties(di);

   remove_proc_entry(nNIKAL200_kDeviceInterfacesPropertiesDir, di->interfaceDir);
   remove_proc_entry(di->dirName, di->parent->deviceInterfacesDir);

   nNIKAL200_notifyUserspaceRemoveEvent(di);

   kref_put(&di->kref, nNIKAL200_destructDeviceInterface);
}

static inline struct proc_dir_entry *nNIKAL200_propertyHelperGetParentDir(nNIKAL200_tPropertyData *pd)
{
   KAL_ASSERT(pd);

   switch (pd->propClass)
   {
      case nNIKAL200_tPropertyClassDevice:
         return ((nNIKAL200_tDevice *) pd->handle)->devicePropertiesDir;
      case nNIKAL200_tPropertyClassBus:
         return ((nNIKAL200_tDevice *) pd->handle)->busPropertiesDir;
      case nNIKAL200_tPropertyClassDeviceInterface:
         return ((nNIKAL200_tDeviceInterface *) pd->handle)->interfacePropertiesDir;
      default:
         KAL_DPRINT("Error: Unknown property class specified: %d\n", pd->propClass);
         return NULL;
   }
}
static inline struct list_head *nNIKAL200_propertyHelperGetParentList(nNIKAL200_tPropertyData *pd)
{
   KAL_ASSERT(pd);

   switch (pd->propClass)
   {
      case nNIKAL200_tPropertyClassDevice:
         return &((nNIKAL200_tDevice *) pd->handle)->devicePropertiesHead;
      case nNIKAL200_tPropertyClassBus:
         return &((nNIKAL200_tDevice *) pd->handle)->busPropertiesHead;
      case nNIKAL200_tPropertyClassDeviceInterface:
         return &((nNIKAL200_tDeviceInterface *) pd->handle)->propertiesHead;
      default:
         KAL_DPRINT("Error: Unknown property class specified: %d\n", pd->propClass);
         return NULL;
   }
}


nNIKAL100_cc void *nNIKAL200_registerProperty(nNIKAL200_tPropertyClass class,
                                              void *handle,
                                              const char *name,
                                              const void *content,
                                              size_t length)
{
   struct proc_dir_entry   *parentDir, *procEntry;
   struct list_head        *listHead;
   nNIKAL200_tPropertyData *pd;

   KAL_ASSERT(handle);

   pd = nNIKAL200_constructPropertyData(name, class, handle, content, length);

   if (unlikely(!pd)) {
      KAL_DPRINT("Error: Could not allocate memory for property '%s'.\n", name);
      goto out;
   }

   parentDir = nNIKAL200_propertyHelperGetParentDir(pd);
   listHead  = nNIKAL200_propertyHelperGetParentList(pd);

   if (!parentDir || !listHead) {
      KAL_DPRINT("Error: Could not get parent directory of property '%s'.\n", name);
      goto out_destruct;
   }

#ifdef nNIKAL1400_kHasCreateProcReadEntry
   procEntry = create_proc_read_entry(pd->name,
                                      S_IRUGO, 
                                      parentDir,
                                      nNIKAL200_procRead,
                                      pd);
#else
   procEntry = proc_create_data(pd->name,
                                  S_IRUGO, 
                                  parentDir,
                                  &nNIKAL250_procFops,
                                  pd);
#endif

   if (!procEntry) {
      KAL_DPRINT("Error: Could not create proc entry for property '%s'.\n", name);
      goto out_destruct;
   }

   list_add(&pd->propertiesNode, listHead);

   return pd;

out_destruct:
   nNIKAL200_destructPropertyData(pd);
out:
   return NULL;
}



nNIKAL100_cc void nNIKAL200_unregisterProperty(void* handle)
{
   struct proc_dir_entry *parentDir;

   nNIKAL200_tPropertyData *pd = handle;

   
   if (unlikely(!pd))
      return;

   list_del(&pd->propertiesNode);

   parentDir = nNIKAL200_propertyHelperGetParentDir(pd);

   remove_proc_entry(pd->name, parentDir);

   nNIKAL200_destructPropertyData(pd);
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL200_registerDriver(nNIKAL100_tDriver *driver)
{
   int status;

   nNIKAL100_listInit(&driver->deviceListHead);

   
   status = alloc_chrdev_region((dev_t *) &driver->number, 0, nNIKAL200_kMinorNumbersPerDevice, driver->name);

   KAL_DPRINT("Registered driver '%s' (%p).  driver->number = %x\n", driver->name, driver, (unsigned int)driver->number);

   return nNIKAL100_convertLinuxToKALStatus(status);
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_registerDriver(nNIKAL100_tDriver *driver)
{
   int err;
   nNIKAL100_tStatus status;

   
   if (nNIKAL200_sPALPseudoDevice.driver)
   {
      KAL_DPRINT("PAL already registered. Bailing out.\n");
      status =  nNIKAL100_kStatusResourceBusy;
      goto out;
   }

   
   driver->name = "nikal";
   status = nNIKAL200_registerDriver(driver);

   if (nNIKAL100_statusIsFatal(status))
      goto out_unload;

   nNIKAL200_sPALPseudoDevice.driver = driver;

   nNIKAL200_sPALPseudoDeviceInterface.cdev.owner = driver->module;

   cdev_init(&nNIKAL200_sPALPseudoDeviceInterface.cdev, &nNIKAL100_fops);

   err = cdev_add(&nNIKAL200_sPALPseudoDeviceInterface.cdev, driver->number, 1);

   if (unlikely(err)) {
      status = nNIKAL100_convertLinuxToKALStatus(err);
      goto out_unregister;
   }

#ifndef HAVE_COMPAT_IOCTL
   err = register_ioctl32_conversion(0xc018d501, NULL);

   if (unlikely(err)) {
      status = nNIKAL100_convertLinuxToKALStatus(err);
      goto out_cdev_del;
   }
#endif

out:
   return status;

#ifndef HAVE_COMPAT_IOCTL
out_cdev_del:
   KAL_DPRINT("Error %d. Unable to register ioctl32 conversion.\n", (int)status);
   cdev_del(&nNIKAL200_sPALPseudoDeviceInterface.cdev);
#endif
out_unregister:
   KAL_DPRINT("Error %d. Unable to register as character driver.\n", (int)status);
   nNIKAL200_unregisterDriver(driver);
out_unload:
   KAL_DPRINT("Error %d. Unable to allocate major number.\n", (int)status);
   if (driver->unload)
      driver->unload(driver);
   return status;
}

static nNIKAL100_tStatus nNIKAL200_initializeSimulatedDeviceHelper(nNIKAL100_tDriver *driver,
                                                                   nNIKAL200_tDevice *device,
                                                                   const char* devName)
{
   nNIKAL100_tStatus status;

   device->deviceInfo.busType  = nNIKAL200_kBusTypeSimulated;
   device->deviceExtension     = device;

   
   snprintf(device->nameString, NIKAL_MAX_DEVICE_NAME, "%s:%s", driver->name, devName);

   status = nNIKAL200_createDeviceProcDirs(device->nameString, device);

   if (nNIKAL100_statusIsFatal(status)) {
      KAL_DPRINT("Error. Unable to create proc directory for simulated device '%s'", device->nameString);
      goto out;
   }

   status = driver->addDevice(&device->deviceInfo, device, &device->deviceExtension);

   if (nNIKAL100_statusIsFatal(status)) {
      KAL_DPRINT("Error. AddDevice for device '%p' failed.\n", device);
      goto out_destroy_dirs;
   }

   
   mutex_lock(&nNIKAL220_sDriverSimulatedDeviceListLock);
   nNIKAL100_listAdd(&driver->deviceListHead, &device->deviceListNode);
   mutex_unlock(&nNIKAL220_sDriverSimulatedDeviceListLock);

   device->addDeviceSuccess = 1;

   return nNIKAL100_kStatusSuccess;

out_destroy_dirs:
   nNIKAL200_destroyDeviceProcDirs(device);
out:
   return status;
}

nNIKAL100_cc void* nNIKAL200_createSimulatedDevice(nNIKAL100_tDriver *driver,
                                                                  const char* devName,
                                                                  void* creationContext)
{
   nNIKAL200_tDevice *d;
   nNIKAL100_tStatus status = nNIKAL100_kStatusSuccess;

   KAL_ASSERT(driver);
   KAL_ASSERT(devName);

   d = nNIKAL200_constructDevice(driver);

   if (unlikely(!d)) {
      KAL_DPRINT("Error. Unable to construct simulated device.\n");
      return NULL;
   }

   d->creationContext = creationContext;
   status = nNIKAL200_initializeSimulatedDeviceHelper(driver, d, devName);

   if (nNIKAL100_statusIsFatal(status)) {
      KAL_DPRINT("Error.  Could not initialize simulated device %s.\n", devName);
      nNIKAL200_destructDevice(&(d->kref));
      return NULL;
   }

   return d;
}

nNIKAL100_cc void nNIKAL200_destroySimulatedDevice(nNIKAL100_tDriver *driver,
                                                      void *kalDevice)
{
   nNIKAL200_tDevice *d;

   KAL_ASSERT(driver);

   mutex_lock(&nNIKAL220_sDriverSimulatedDeviceListLock);
   list_for_each_entry(d, &driver->deviceListHead, deviceListNode) {
      if (d == (nNIKAL200_tDevice*)kalDevice) {
         nNIKAL100_listRemove(&driver->deviceListHead, &d->deviceListNode);
         mutex_unlock(&nNIKAL220_sDriverSimulatedDeviceListLock); 

         driver->removeDevice(d->deviceExtension);
         nNIKAL200_destroyDeviceProcDirs(d);

         kref_put(&(d->kref), nNIKAL200_destructDevice);
         return;
      }
   }
   mutex_unlock(&nNIKAL220_sDriverSimulatedDeviceListLock);
}

nNIKAL100_cc void* nNIKAL200_getDeviceExtension(void* kalDevice)
{
   nNIKAL200_tDevice *d = (nNIKAL200_tDevice*)kalDevice;
   return d->deviceExtension;
}

nNIKAL100_cc void* nNIKAL200_getDeviceCreationContext(void* kalDevice)
{
   nNIKAL200_tDevice *d = (nNIKAL200_tDevice*)kalDevice;
   return d->creationContext;
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL200_registerSimulatedDeviceDriver(nNIKAL100_tDriver *driver,
                                                                       size_t numDevices)
{
   int i;
   char numStr[4];

   nNIKAL200_tDevice *d;
   nNIKAL100_tStatus status = nNIKAL100_kStatusSuccess;

   KAL_ASSERT(numDevices<999);
   KAL_ASSERT(driver);
   KAL_ASSERT(numDevices >= 0);

   status = nNIKAL200_registerDriver(driver);
   if (nNIKAL100_statusIsFatal(status)) {
      return status;
   }

   nNIKAL100_listInit(&driver->deviceListHead);

   for (i = 0; i < numDevices; i++) {
      d = nNIKAL200_constructDevice(driver);

      if (unlikely(!d)) {
         KAL_DPRINT("Error. Unable to construct simulated device.\n");
         status = nNIKAL100_kStatusMemoryFull;
         goto out_unregister;
      }

      sprintf(numStr, "%d", i);
      status = nNIKAL200_initializeSimulatedDeviceHelper(driver, d, numStr);

      if (nNIKAL100_statusIsFatal(status)) {
         KAL_DPRINT("Error.  Could not initialize simulated device %d.\n", i);
         goto out_destruct;
      }
   }

   return status;

out_destruct:
   nNIKAL200_destructDevice(&(d->kref));
out_unregister:
   nNIKAL200_unregisterSimulatedDeviceDriver(driver);
   return status;
}

nNIKAL100_cc void nNIKAL200_unregisterSimulatedDeviceDriver(nNIKAL100_tDriver *driver)
{
   nNIKAL200_tDevice *d, *safeTmp;
   int kref_result;

   KAL_ASSERT(driver);

   
   list_for_each_entry_safe(d, safeTmp, &driver->deviceListHead, deviceListNode) {
      nNIKAL100_listRemove(&driver->deviceListHead, &d->deviceListNode);
      driver->removeDevice(d->deviceExtension);
      nNIKAL200_destroyDeviceProcDirs(d);

      kref_result = kref_put(&(d->kref), nNIKAL200_destructDevice);
      KAL_ASSERT(kref_result); 
   }

   nNIKAL200_unregisterDriver(driver);
}

nNIKAL100_cc void nNIKAL200_unregisterDriver(nNIKAL100_tDriver *driver)
{
   unregister_chrdev_region(driver->number, nNIKAL200_kMinorNumbersPerDevice);
}

nNIKAL100_cc void nNIKAL100_unregisterDriver(nNIKAL100_tDriver *driver)
{
   KAL_ASSERT(nNIKAL200_sPALPseudoDevice.driver == driver);

#ifndef HAVE_COMPAT_IOCTL
   unregister_ioctl32_conversion(0xc018d501);
#endif

   cdev_del(&nNIKAL200_sPALPseudoDeviceInterface.cdev);

   nNIKAL200_unregisterDriver(driver);

   if (driver->unload)
      driver->unload(driver);

   nNIKAL200_sPALPseudoDevice.driver = NULL;
}


nNIKAL100_tBoolean nNIKAL200_isAddressableMemOver4G()
{
   unsigned int fourGpfn = 0x80000000 >> (PAGE_SHIFT-1);
   unsigned int i;

   
   for (i = 1; i < fourGpfn; i <<= 1)
   {
      if (pfn_valid(fourGpfn + i))
      {
         KAL_DPRINT("nNIKAL200_isAddressableMemOver4G() found valid pfn at: %x\n", (fourGpfn|i));
         return nNIKAL100_kTrue;
      }
   }
   return nNIKAL100_kFalse;
}

#define NLNIKAL_CMD_SEND     1

static struct genl_family nikal_netlink_family =
{
   .id = GENL_ID_GENERATE,
   .name = "nlnikal",
   .version = 1,
   .maxattr = 1
};

static int nlnikal_msg(struct sk_buff *skb, struct genl_info *info) { return 0; }

static struct genl_ops nikal_netlink_ops[] =
{
   {
      .cmd = NLNIKAL_CMD_SEND,
      .doit = nlnikal_msg
   },
};


static int __init nNIKAL100_initDriver(void)
{
   int status = 0;

   
   nNIKAL120_sStackSize = nNIKAL120_mRoundUp(((nNIKAL100_tUPtr)(&status) -
         (nNIKAL100_tUPtr)(current_thread_info())), PAGE_SIZE) -
         sizeof(struct thread_info);
   KAL_DPRINT("Kernel stack size: %lu\n", nNIKAL120_sStackSize);

   
   nNIKAL230_Mutex_init(&(nNIKAL240_procfsLock));
   sema_init(&(nNIKAL100_sMMapSema), 1);
   sema_init(&(nNIKAL100_sPageLockOperationSema), 1);
   nNIKAL100_mapInit(&nNIKAL100_sPageLockMap);
   nNIKAL100_mapInit(&nNIKAL100_sMallocContiguousMap);
   sema_init(&(nNIKAL100_sMallocContiguousMapLock), 1);
   kref_init(&(nNIKAL200_sPALPseudoDevice.kref));
   kref_init(&(nNIKAL200_sPALPseudoDeviceInterface.kref));

#ifdef nNIKAL1400_kHasFamilyGenlOpsGroups
   if ((status = genl_register_family_with_ops(&nikal_netlink_family, nikal_netlink_ops))) return status;
#else
   if ((status = genl_register_family_with_ops(&nikal_netlink_family, nikal_netlink_ops, 1))) return status;
#endif


   nNIKAL200_sProcRootDir = proc_mkdir("driver/ni", NULL);

   if (!nNIKAL200_sProcRootDir)
      return -ENOMEM;

   return status;
}


static void __exit nNIKAL100_cleanupDriver(void)
{
   KAL_DPRINT("Max stack usage seen: %lu\n", nNIKAL120_sMaxStackUsageSeen);

   remove_proc_entry("driver/ni", NULL);
   genl_unregister_family(&nikal_netlink_family);

   nNIKAL100_mapCleanup(&nNIKAL100_sMallocContiguousMap);
   nNIKAL100_mapCleanup(&nNIKAL100_sPageLockMap);
}


static int nNIKAL100_open(nLinux_inode *the_inode, nLinux_fileHandle *filePtr)
{
   nNIKAL100_tStatus kalStatus = nNIKAL100_kStatusSuccess;
   nNIKAL200_tFilePrivateData *data = nNIKAL200_mallocZeroed(sizeof(*data), GFP_KERNEL);

   if (unlikely(!data))
      goto out;

   data->interface = container_of(the_inode->i_cdev, nNIKAL200_tDeviceInterface, cdev);

   nNIKAL100_incrementDriverRefcount(data->interface->parent->driver->module);

   
   data->context = data->interface->context;

   if (data->interface->parent->driver->open)
      kalStatus = data->interface->parent->driver->open(filePtr, &data->context);

   if (nNIKAL100_statusIsFatal(kalStatus))
      goto put_module;

   kref_get(&(data->interface->kref));
   filePtr->private_data = data;

   return 0;

put_module:
   nNIKAL100_decrementDriverRefcount(data->interface->parent->driver->module);
   kfree(data);
out:
   KAL_DPRINT("Error occured during open\n");
   return -ENODEV;
}


static int nNIKAL100_release(nLinux_inode *the_inode, nLinux_fileHandle *filePtr)
{
   nNIKAL200_tFilePrivateData *data = filePtr->private_data;
   nNIKAL200_tDevice *device = data->interface->parent;
   void* moduleHandle = device->driver->module;

   if (device->driver->close)
      device->driver->close(data->context);

   kref_put(&(data->interface->kref), nNIKAL200_destructDeviceInterface);

   nNIKAL100_decrementDriverRefcount(moduleHandle);
   kfree(data);
   return 0;
}


static int nNIKAL100_ioctl(nLinux_inode *the_inode, nLinux_fileHandle *filePtr,
   unsigned int command, unsigned long param)
{
   int status;
   nNIKAL200_tFilePrivateData *data;

   status = -EINVAL;
   data = filePtr->private_data;

   if (data->interface->parent->driver->deviceIOControl)
   {
      nNIKAL100_tStatus kalStatus =
        data->interface->parent->driver->deviceIOControl(data->context, command, (void*) param);

      
      if (nNIKAL100_statusIsFatal(kalStatus))
         status = -ENOTTY;
      else
         status = 0;
   }

   return status;
}
#ifdef HAVE_UNLOCKED_IOCTL
static long nNIKAL100_unlockedIoctl(nLinux_fileHandle *filePtr, unsigned int command,
   unsigned long param)
{
   return (long)nNIKAL100_ioctl(NULL, filePtr, command, param);
}
#endif
#ifdef HAVE_COMPAT_IOCTL
static long nNIKAL100_compatIoctl(nLinux_fileHandle *filePtr, unsigned int command,
   unsigned long param)
{
   return (long)nNIKAL100_ioctl(NULL, filePtr, command, param);
}
#endif


static ssize_t nNIKAL100_read(nLinux_fileHandle *filePtr, char __user *buffer, size_t count, loff_t *offset)
{
   ssize_t ret = -EINVAL;

   nNIKAL100_tStatus status;

   nNIKAL100_tIPtr _offset = *offset;
   nNIKAL100_tIPtr bytesRead;

   nNIKAL200_tFilePrivateData *data = filePtr->private_data;

   if (data->interface->parent->driver->read)
   {
      
      char * readBuffer = nNIKAL100_malloc(count);
      if (readBuffer == NULL)
      {
         return -ENOMEM;
      }

      status = data->interface->parent->driver->read(data->context, readBuffer, count, &_offset, &bytesRead);

      if (nNIKAL100_statusIsNonfatal(status))
         ret = 0;
      else
         ret = -EINVAL;

      nNIKAL100_replicateOutboundIOControlBuffer(buffer, readBuffer, bytesRead);
      nNIKAL100_free(readBuffer);
   }

   *offset = _offset;

   if (ret < 0)
      return ret;
   else
      return bytesRead;
}


static ssize_t nNIKAL220_read(nLinux_fileHandle *filePtr, char __user *buffer, size_t count, loff_t *offset)
{
   nNIKAL100_tStatus status = nNIKAL100_kStatusResourceNotAvailable;
   nNIKAL100_tIPtr bytesRead;
   nNIKAL200_tFilePrivateData *data = filePtr->private_data;
   nNIKAL100_tIPtr _offset = *offset;

   KAL_ASSERT(data);

   if (data->interface->parent->driver->read)
      status = data->interface->parent->driver->read(data->context, buffer, count, &_offset, &bytesRead);
   *offset = _offset;

   if (nNIKAL100_statusIsFatal(status))
      return -EINVAL;

   return bytesRead;
}


static ssize_t nNIKAL100_write(nLinux_fileHandle *filePtr, const char __user *buffer, size_t count, loff_t *offset)
{
   ssize_t ret = -EINVAL;
   nNIKAL100_tIPtr _offset = *offset;
   nNIKAL100_tIPtr bytesWritten;

   nNIKAL200_tFilePrivateData *data = filePtr->private_data;

   if (data->interface->parent->driver->write)
   {
      nNIKAL100_tStatus status;
      
      char * writeBuffer = nNIKAL100_malloc(count);
      if (writeBuffer == NULL)
      {
         return -ENOMEM;
      }
      status = nNIKAL100_replicateInboundIOControlBuffer(writeBuffer, buffer, count);
      if (nNIKAL100_statusIsNonfatal(status))
      {
         status = data->interface->parent->driver->write(data->context, writeBuffer, count, &_offset, &bytesWritten);

         if (nNIKAL100_statusIsNonfatal(status))
            ret = 0;
         else
            ret = -EINVAL;

         nNIKAL100_free(writeBuffer);
      }
      else
      {
         return -EINVAL;
      }
   }

   *offset = _offset;

   if (ret < 0)
      return ret;
   else
      return bytesWritten;
}


static ssize_t nNIKAL220_write(nLinux_fileHandle *filePtr, const char __user *buffer, size_t count, loff_t *offset)
{
   nNIKAL100_tStatus status = nNIKAL100_kStatusResourceNotAvailable;
   nNIKAL100_tIPtr bytesWritten;
   nNIKAL200_tFilePrivateData *data = filePtr->private_data;
   nNIKAL100_tIPtr _offset = *offset;

   KAL_ASSERT(data);

   if (data->interface->parent->driver->write)
      status = data->interface->parent->driver->write(data->context, buffer, count, &_offset, &bytesWritten);
   *offset = _offset;

   if (nNIKAL100_statusIsFatal(status))
      return -EINVAL;

   return bytesWritten;
}


static int nNIKAL100_mmap(nLinux_fileHandle *filePtr, nLinux_vmArea *vma)
{
   int status = -EINVAL;
   nNIKAL100_tUPtr address;
   nNIKAL100_tUPtr physAddr;
   u32 pageSize;
   nNIKAL100_tBoolean isFromIo;
   nNIKAL100_tStatus remapStatus;

   
   if (nNIKAL100_gMMapDataPtr->callingThreadID != current->pid)
   {
      
      status = -EPERM;
   }
   else
   {
      KAL_ASSERT(nNIKAL100_gMMapDataPtr->addressType != nNIKAL100_kMemoryAddressTypeInvalid,
         "addressType passed to nNIKAL100_privateMMap is invalid!\n");

      
      pageSize = nNIKAL100_getPageSize();
      address = vma->vm_pgoff * pageSize;
      physAddr = 0;
      if (nNIKAL100_gMMapDataPtr->addressType == nNIKAL100_kMemoryAddressTypeVirtual)
      {
         nNIKAL100_tUPtr i;
         i = 0;
         nNIKAL100_compileTimeAssert(sizeof(void*)==sizeof(nNIKAL100_tUPtr), "Oops, resizing void* into nNIKAL100_tUPtr!");
         physAddr = nNIKAL230_kernelVirtualToPhysical((void*)address);
         isFromIo = !pfn_valid(physAddr >> PAGE_SHIFT);

         
         #ifdef nNIKAL100_kDebuggingIsActive
         pageSize = nNIKAL100_getPageSize();
         for (i=address+pageSize; i < address + ((vma->vm_end) - (vma->vm_start)); i+=pageSize)
         {
            KAL_ASSERT(isFromIo != pfn_valid(nNIKAL230_kernelVirtualToPhysical((void*)i) >> PAGE_SHIFT),
                  "nNIKAL100_mmap() asked to map a region of non-uniform memory\n");
         }
         #endif
      }
      else 
      {
         physAddr = address;
         isFromIo = nNIKAL100_kTrue;
      }

      if (isFromIo || nNIKAL100_isKernelContiguousPointer((void *)address))
      {
         remapStatus = nNIKAL100_remapPageRange(vma, physAddr);
         KAL_DPRINT("nNIKAL100_mmap() page remap, address: %lx, phys: %lx, size: %lu, remap_pfn_range status: %ld\n", address, physAddr, vma->vm_end - vma->vm_start, (long)remapStatus);
         if (nNIKAL100_statusIsNonfatal(remapStatus))
         {
            status = 0;
         }
         else
         {
            status = -ENOMEM;
         }
      }
      else
      {
           
         vma->vm_ops = &nNIKAL100_vmaOps;
         
         status = 0;
      }
   }

   if (status >= 0)
   {
      vma->vm_flags |= nNIKAL240_kVM_RESERVED | VM_LOCKED | VM_DONTCOPY | VM_DONTEXPAND;
   }

   return status;
}


typedef struct {
   nNIKAL220_tPageList *list;
   void *map_data;
} nNIKAL220_tUserMemMapPrivateData;

static int nNIKAL220_mmap(nLinux_fileHandle *filePtr, nLinux_vmArea *vma)
{
   nNIKAL200_tFilePrivateData *fileData;
   nNIKAL100_tDriver *driver;
   nNIKAL100_tStatus status = nNIKAL100_kStatusFeatureNotSupported;
   nNIKAL220_tUserMemMapFlags vmaFlags;

   void *map_data;

   fileData = filePtr->private_data;
   driver = fileData->interface->parent->driver;

   
   if (!driver->mapMemory)
      return -EINVAL;

   status = driver->mapMemory(fileData->context,
                              vma->vm_pgoff,
                              (nNIKAL220_tUserMemMap *) vma,
                              &vmaFlags,
                              &map_data);

   if (status == nNIKAL100_kStatusMemoryFull)
      return -ENOMEM;

   if (nNIKAL100_statusIsFatal(status))
      return -EINVAL;

   
   KAL_ASSERT(vmaFlags & nNIKAL220_kUserMemMapNoCopyOnFork);

   KAL_ASSERT(vma->vm_ops == &nNIKAL220_vmaOpsPhysical || vma->vm_ops == &nNIKAL220_vmaOpsPageList);

   
   if (vma->vm_ops == &nNIKAL220_vmaOpsPhysical) {
      vma->vm_private_data = map_data;
   } else {
      ((nNIKAL220_tUserMemMapPrivateData *) vma->vm_private_data)->map_data = map_data;
   }

   return 0;
}


nNIKAL100_cc void*    nNIKAL220_tUserMemMap_getVirtualAddress(nNIKAL220_tUserMemMap *_vma)
{
   struct vm_area_struct *vma = (struct vm_area_struct *) _vma;
   return (void*)(vma->vm_start);
}


nNIKAL100_cc size_t   nNIKAL220_tUserMemMap_getSize(nNIKAL220_tUserMemMap *_vma)
{
   struct vm_area_struct *vma = (struct vm_area_struct *) _vma;
   return vma->vm_end - vma->vm_start;
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL220_tUserMemMap_getAccessMode(nNIKAL220_tUserMemMap *_vma)
{
   #if ((VM_READ != nNIKAL220_kUserMemMapAccessModeRead) || (VM_WRITE != nNIKAL220_kUserMemMapAccessModeWrite) || (VM_EXEC != nNIKAL220_kUserMemMapAccessModeExec))
      #error "Unexpected vm_flags constants!"
   #endif
   struct vm_area_struct *vma = (struct vm_area_struct *) _vma;
   return (nNIKAL100_tU32) (vma->vm_flags & (VM_READ | VM_WRITE | VM_EXEC));
}


static loff_t nNIKAL100_llseek(nLinux_fileHandle *filePtr, loff_t inOffset, int seekFrom)
{
   return -ESPIPE;
}

#ifdef nNIKAL1400_kHasCreateProcReadEntry
static int nNIKAL200_procRead(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
   nNIKAL200_tPropertyData *pd = data;

   memcpy(page, pd->content, pd->length);
   *eof = 1;

   return pd->length;
}
#else
static int nNIKAL250_procShow(struct seq_file *m, void *v)
{
   nNIKAL200_tPropertyData *pd = m->private;
   seq_write(m, pd->content, pd->length);
   return 0;
}

static int nNIKAL250_procOpen(struct inode *inode, struct file *file)
{
   return single_open(file, nNIKAL250_procShow, PDE_DATA(inode));
}
#endif


static inline nNIKAL100_tBoolean nNIKAL100_isKernelContiguousPointer(const void *ptr)
{
   return ((nNIKAL100_tUPtr)ptr < (nNIKAL100_tUPtr)VMALLOC_START);
}


static inline nNIKAL100_tBoolean nNIKAL100_isKernelVirtualPointer(const void *ptr)
{
   return ((((nNIKAL100_tUPtr)ptr >= (nNIKAL100_tUPtr)VMALLOC_START) &&
      ((nNIKAL100_tUPtr)ptr < (nNIKAL100_tUPtr)VMALLOC_END)));
}












#if nNIKAL100_mSerialCoreIsSupported
static unsigned int nNIKAL100_uartOps_tx_empty(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->tx_empty)
      return clientOps->tx_empty((void*)uport);
   else
      return 0;
}

static unsigned int nNIKAL100_convertKalToLinuxModemBits(nNIKAL100_tU32 kalBitField)
{
   unsigned int linuxBitField = 0;

   if (kalBitField & nNIKAL100_kSerialModemLine_DTR)
      linuxBitField |= TIOCM_DTR;

   if (kalBitField & nNIKAL100_kSerialModemLine_RTS)
      linuxBitField |= TIOCM_RTS;

   if (kalBitField & nNIKAL100_kSerialModemLine_ST)
      linuxBitField |= TIOCM_ST;

   if (kalBitField & nNIKAL100_kSerialModemLine_SR)
      linuxBitField |= TIOCM_SR;

   if (kalBitField & nNIKAL100_kSerialModemLine_CTS)
      linuxBitField |= TIOCM_CTS;

   if (kalBitField & nNIKAL100_kSerialModemLine_DCD)
      linuxBitField |= TIOCM_CD;

   if (kalBitField & nNIKAL100_kSerialModemLine_RI)
      linuxBitField |= TIOCM_RI;

   if (kalBitField & nNIKAL100_kSerialModemLine_DSR)
      linuxBitField |= TIOCM_DSR;

   if (kalBitField & nNIKAL100_kSerialModemLine_LOOP)
      linuxBitField |= TIOCM_LOOP;

   return linuxBitField;
}

static nNIKAL100_tU32 nNIKAL100_convertLinuxToKalModemBits(unsigned int linuxBitField)
{
   nNIKAL100_tU32 kalBitField = 0;

   if (linuxBitField & TIOCM_DTR)
      kalBitField |= nNIKAL100_kSerialModemLine_DTR;

   if (linuxBitField & TIOCM_RTS)
      kalBitField |= nNIKAL100_kSerialModemLine_RTS;

   if (linuxBitField & TIOCM_ST)
      kalBitField |= nNIKAL100_kSerialModemLine_ST;

   if (linuxBitField & TIOCM_SR)
      kalBitField |= nNIKAL100_kSerialModemLine_SR;

   if (linuxBitField & TIOCM_CTS)
      kalBitField |= nNIKAL100_kSerialModemLine_CTS;

   if (linuxBitField & TIOCM_CD)
      kalBitField |= nNIKAL100_kSerialModemLine_DCD;

   if (linuxBitField & TIOCM_RI)
      kalBitField |= nNIKAL100_kSerialModemLine_RI;

   if (linuxBitField & TIOCM_DSR)
      kalBitField |= nNIKAL100_kSerialModemLine_DSR;

   if (linuxBitField & TIOCM_LOOP)
      kalBitField |= nNIKAL100_kSerialModemLine_LOOP;

   return kalBitField;
}

static void nNIKAL100_uartOps_set_mctrl(nLinux_uartPort *uport, unsigned int mctrl)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->set_mctrl)
   {
      clientOps->set_mctrl(
                     (void*)uport,
                     nNIKAL100_convertLinuxToKalModemBits(mctrl)
                     );
   }
}

static unsigned int nNIKAL100_uartOps_get_mctrl(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->get_mctrl)
   {
      nNIKAL100_tU32 mctrl = clientOps->get_mctrl((void*)uport);
      return nNIKAL100_convertKalToLinuxModemBits(mctrl);
   }
   else
      return 0;
}

static void nNIKAL100_uartOps_stop_tx(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->stop_tx)
      clientOps->stop_tx((void*)uport);
}

static void nNIKAL100_uartOps_start_tx(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->start_tx)
      clientOps->start_tx((void*)uport);
}

static void nNIKAL100_uartOps_send_xchar(nLinux_uartPort *uport, char ch)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->send_xchar)
      clientOps->send_xchar((void*)uport, ch);
}

static void nNIKAL100_uartOps_stop_rx(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->stop_rx)
      clientOps->stop_rx((void*)uport);
}

static void nNIKAL100_uartOps_enable_ms(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->enable_ms)
      clientOps->enable_ms((void*)uport);
}

static void nNIKAL100_uartOps_break_ctl(nLinux_uartPort *uport, int ctl)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->break_ctl)
      clientOps->break_ctl((void*)uport, ctl);
}

static int nNIKAL100_uartOps_startup(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->startup)
      return nNIKAL100_convertKALToLinuxStatus(clientOps->startup((void*)uport));
   else
      return -ENODEV; // If clientOps does not provide a startup, we will treat
                      // it as if there is no device.
}

static void nNIKAL100_uartOps_shutdown(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->shutdown)
      clientOps->shutdown((void*)uport);
}

static void nNIKAL100_uartOps_flush_buffer(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->flush_buffer)
      clientOps->flush_buffer((void*)uport);
}


static unsigned int nNIKAL100_decodeBaud(nLinux_ktermios *termios);
static void nNIKAL100_encodeBaud(unsigned int baud, nLinux_ktermios *termios);
static nNIKAL100_tSerialCorePortSettings nNIKAL100_getSerialSettings(nLinux_uartPort *uartPort, nLinux_ktermios *termios);

static void nNIKAL100_uartOps_set_termios(nLinux_uartPort *uport, nLinux_ktermios *new_term, nLinux_ktermios *old_term)
{
   nNIKAL100_tSerialCoreOperations *clientOps;
   unsigned int baudToSet = 0;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);

   if (clientOps->set_baud)
   {
      nLinux_ktermios *term = new_term;
      while (term != NULL)
      {
         
         baudToSet = nNIKAL100_decodeBaud(term);
         
         if (clientOps->set_baud((void*)uport, baudToSet))
         {
            
            nNIKAL100_encodeBaud(baudToSet, new_term);
            break;
         }
         
         else
         {
            term = old_term;
            old_term = NULL;
         }
      }
   }

   if (clientOps->set_config)
   {
      nNIKAL100_tSerialCorePortSettings portSettings = nNIKAL100_getSerialSettings(uport, new_term);
      clientOps->set_config((void*)uport, &portSettings);
   }
}

static void nNIKAL100_uartOps_pm(nLinux_uartPort *uport, unsigned int state, unsigned int oldstate)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->pm)
      clientOps->pm((void*)uport, state, oldstate);
}

#ifdef nNIKAL1400_kHasUartSetWake
static int nNIKAL100_uartOps_set_wake(nLinux_uartPort *uport, unsigned int state)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->set_wake)
      return nNIKAL100_convertKALToLinuxStatus(clientOps->set_wake((void*)uport, state));
   else
      return -ENOSYS; // If clientOps does not provide set_wake, we will
                      // say that the function is not supported.
}
#endif

static const char * nNIKAL100_uartOps_type(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->type)
      return clientOps->type((void*)uport);
   else
      return "unknown";
}

static void nNIKAL100_uartOps_release_port(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->release_port)
      clientOps->release_port((void*)uport);
}

static int nNIKAL100_uartOps_request_port(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->request_port)
      return nNIKAL100_convertKALToLinuxStatus(clientOps->request_port((void*)uport));
   else
      return -ENOSYS; // If clientOps does not provide a request_port, we
                      // will say that the function is not supported.
                      // request_port is called by the ioctl TIOCSSERIAL which
                      // the client might not support.
}

static void nNIKAL100_uartOps_config_port(nLinux_uartPort *uport, int flags)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->config_port)
      clientOps->config_port((void*)uport, flags);
}

static int nNIKAL100_uartOps_verify_port(nLinux_uartPort *uport, nLinux_serialStruct *new_serial)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->verify_port)
      return nNIKAL100_convertKALToLinuxStatus(clientOps->verify_port((void*)uport, (void*)new_serial));
   else
      return -ENODEV; // The client must provide a verify_port, otherwise we
                      // will treat it as there is no device.
}

static int nNIKAL100_uartOps_ioctl(nLinux_uartPort *uport, unsigned int cmd, unsigned long arg)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->ioctl)
      return nNIKAL100_convertKALToLinuxStatus(clientOps->ioctl((void*)uport, cmd, arg));
   else
      return -ENOIOCTLCMD; // If the client does not handle this IOCTL, we
                           // should just return -ENOIOCTLCMD, so that the TTY
                           // layer will continue searching for the correct
                           // caller.
}

#ifdef nNIKAL100_kHasRS485Support

static nNIKAL100_tSerialRs485WireMode nNIKAL100_getWireMode(nLinux_serialRs485 *rs485)
{
   

   
   if (rs485->flags & SER_RS485_ENABLED)
   {
      
      if (rs485->flags & SER_RS485_RX_DURING_TX)
      {
         return nNIKAL100_kSerialWireMode_2WireDtrEcho;
      }
      else
      {
         
         if (rs485->flags & SER_RS485_RTS_ON_SEND)
         {
            return nNIKAL100_kSerialWireMode_2WireAuto;
         }
         else
         {
            return nNIKAL100_kSerialWireMode_2WireDtr;
         }
      }
   }
   else
   {
      return nNIKAL100_kSerialWireMode_4Wire;
   }
}

static int nNIKAL100_uartRs485Ops_enable_transceivers(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->enable_transceivers)
      return nNIKAL100_convertKALToLinuxStatus(clientOps->enable_transceivers((void*)uport));
   else
      return -ENOSYS;
}

static int nNIKAL100_uartRs485Ops_disable_transceivers(nLinux_uartPort *uport)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->disable_transceivers)
      return nNIKAL100_convertKALToLinuxStatus(clientOps->disable_transceivers((void*)uport));
   else
      return -ENOSYS;
}

static int nNIKAL100_uartRs485Ops_config_rs485(nLinux_uartPort *uport, nLinux_serialRs485 *rs485)
{
   nNIKAL100_tSerialCoreOperations *clientOps;

   KAL_ASSERT(uport != NULL);
   KAL_ASSERT(uport->private_data != NULL);

   clientOps = (nNIKAL100_tSerialCoreOperations*)(uport->private_data);
   if (clientOps->config_rs485)
   {
      nNIKAL100_tSerialRs485WireMode wire_mode = nNIKAL100_getWireMode(rs485);
      int status = nNIKAL100_convertKALToLinuxStatus(clientOps->config_rs485((void*)uport, &wire_mode));
      if (status == 0)
      {
         // Update the new 485 settings in the port structure if we succeeded
         uport->rs485 = *rs485;
      }
      return status;
   }
   else
   {
      return -ENOSYS;
   }
}
#endif




typedef struct
{
   nLinux_termiosFlags  baudFlag;
   unsigned int         baud;
} nNIKAL100_tSerialBaud;


static nNIKAL100_tSerialBaud nNIKAL100_sStandardBauds[] = {
   {  B0          , 0         },
   {  B50         , 50        },
   {  B75         , 75        },
   {  B110        , 110       },
   {  B134        , 134       },
   {  B150        , 150       },
   {  B200        , 200       },
   {  B300        , 300       },
   {  B600        , 600       },
   {  B1200       , 1200      },
   {  B1800       , 1800      },
   {  B2400       , 2400      },
   {  B4800       , 4800      },
   {  B9600       , 9600      },
   {  B19200      , 19200     },
   {  B38400      , 38400     },
   {  B57600      , 57600     },
   {  B115200     , 115200    },
   {  B230400     , 230400    },
   {  B460800     , 460800    },
   {  B500000     , 500000    },
   {  B576000     , 576000    },
   {  B921600     , 921600    },
   {  B1000000    , 1000000   },
   {  B1152000    , 1152000   },
   {  B1500000    , 1500000   },
   {  B2000000    , 2000000   },
   {  B2500000    , 2500000   },
   {  B3000000    , 3000000   },
   {  B3500000    , 3500000   },
   {  B4000000    , 4000000   }
};
static int nNIKAL100_sNumStandardBauds = sizeof(nNIKAL100_sStandardBauds) / sizeof(nNIKAL100_sStandardBauds[0]);


static unsigned int nNIKAL100_decodeBaud(nLinux_ktermios *termios)
{
   int i = 0;
   KAL_ASSERT(termios != NULL);
   
   if ((termios->c_cflag & CBAUD) == BOTHER)
   {
      return termios->c_ospeed;
   }
   
   else
   {
      for (i = 0; i < nNIKAL100_sNumStandardBauds; ++i)
      {
         if ((termios->c_cflag & CBAUD) == nNIKAL100_sStandardBauds[i].baudFlag)
         {
            return nNIKAL100_sStandardBauds[i].baud;
         }
      }
   }

   
   KAL_DPRINT("Unknown baud bit set in termios. Using 9600 baud.\n");
   return 9600;
}


static void nNIKAL100_encodeBaud(unsigned int baud, nLinux_ktermios *termios)
{
   int i = 0;

   
   unsigned int baudMargin = baud / 50;
   unsigned int baudUpperLimit = baud + baudMargin;
   unsigned int baudLowerLimit = baud - baudMargin;

   KAL_ASSERT(termios != NULL);

   
   termios->c_ispeed = baud;
   termios->c_ospeed = baud;

   termios->c_cflag &= ~CBAUD; 

   
   for (i = 0; i < nNIKAL100_sNumStandardBauds; ++i)
   {
      if (   nNIKAL100_sStandardBauds[i].baud <= baudUpperLimit
          && nNIKAL100_sStandardBauds[i].baud >= baudLowerLimit
         )
      {
         termios->c_cflag |= nNIKAL100_sStandardBauds[i].baudFlag;
         return;
      }
   }

   
   termios->c_cflag |= BOTHER;
}


static nNIKAL100_tSerialCorePortSettings nNIKAL100_getSerialSettings(nLinux_uartPort *uartPort, nLinux_ktermios *termios)
{
   nLinux_termiosFlags controlFlags = 0;
   nLinux_termiosFlags inputFlags = 0;
   nLinux_ttyPort *ttyPort = &(uartPort->state->port);

   nNIKAL100_tSerialCorePortSettings ret = {0};

   KAL_ASSERT(uartPort != NULL);
   KAL_ASSERT(termios != NULL);

   controlFlags = termios->c_cflag;
   inputFlags = termios->c_iflag;

   switch (controlFlags & CSIZE)
   {
      case CS5:
         ret.dataSize = nNIKAL100_kSerialDataSize_5Bit;
         break;
      case CS6:
         ret.dataSize = nNIKAL100_kSerialDataSize_6Bit;
         break;
      case CS7:
         ret.dataSize = nNIKAL100_kSerialDataSize_7Bit;
         break;
      case CS8:
      default:
         ret.dataSize = nNIKAL100_kSerialDataSize_8Bit;
         break;
   }

   if (controlFlags & PARENB)
   {
      // If Odd Parity (PARODD) is set, then it could either be odd parity
      // or mark parity depending on the CMSPAR flag.
      if (controlFlags & PARODD)
      {
         ret.parityMode = (controlFlags & CMSPAR) ? nNIKAL100_kSerialParity_Mark : nNIKAL100_kSerialParity_Odd;
      }
      // If Odd Parity (PARODD) is not set, then it could either be even parity
      // or space parity depending on the CMSPAR flag.
      else
      {
         ret.parityMode = (controlFlags & CMSPAR) ? nNIKAL100_kSerialParity_Space : nNIKAL100_kSerialParity_Even;
      }
   }
   else
   {
      ret.parityMode = nNIKAL100_kSerialParity_None;
   }

   ret.enableRx                     = (controlFlags & CREAD)   ? true : false;
   ret.enableTwoStopBits            = (controlFlags & CSTOPB)  ? true : false;
   ret.inputParityCheckEnabled      = (inputFlags & INPCK)     ? true : false;
   ret.ignoreFramingAndParityErrors = (inputFlags & IGNPAR)    ? true : false;
   ret.ignoreBreakErrors            = (inputFlags & IGNBRK)    ? true : false;

   // Set the correct flow control modes
   ret.flowControl.hwFlowControlEnabled       = (controlFlags & CRTSCTS) ? true : false;
   ret.flowControl.inputSwFlowControlEnabled  = (inputFlags & IXOFF) ? true : false;
   ret.flowControl.outputSwFlowControlEnabled = (inputFlags & IXON) ? true : false;
   ret.flowControl.startChar                  = START_CHAR(ttyPort->tty);
   ret.flowControl.stopChar                   = STOP_CHAR(ttyPort->tty);

   return ret;
}

#endif






static void nNIKAL220_vmaClosePageList(nLinux_vmArea *vma)
{
   nNIKAL220_tUserMemMapPrivateData *_vma = vma->vm_private_data;
   nNIKAL200_tFilePrivateData *fileData = vma->vm_file->private_data;
   nNIKAL100_tDriver *driver = fileData->interface->parent->driver;

   if (driver->unmapMemory)
      driver->unmapMemory(_vma->map_data);

   kfree(_vma);
}


static void nNIKAL220_vmaClosePhysical(nLinux_vmArea *vma)
{
   nNIKAL200_tFilePrivateData *fileData = vma->vm_file->private_data;
   nNIKAL100_tDriver *driver = fileData->interface->parent->driver;

   if (driver->unmapMemory)
      driver->unmapMemory(vma->vm_private_data);
}


static int nNIKAL190_vmaPageFaultHandler(nLinux_vmArea *vma, nLinux_vmFault *vmf)
{
   nNIKAL100_tUPtr addr = vmf->pgoff << PAGE_SHIFT;
   vmf->page = (nLinux_physicalPage*)nNIKAL100_incrementPageRefcount((void*)addr);
   return 0;
}


static int nNIKAL220_vmaPageFaultHandler(nLinux_vmArea *vma, nLinux_vmFault *vmf)
{
   nNIKAL220_tUserMemMapPrivateData *_vma = vma->vm_private_data;
   nNIKAL220_tPageList *list = _vma->list;

   KAL_ASSERT(vmf->pgoff < list->num_pages);

   vmf->page = ((list->flags & nNIKAL220_kPageListContiguous) ? (list->pages[0] + vmf->pgoff) : list->pages[vmf->pgoff]);
   get_page(vmf->page);
   return 0;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_replicateOutboundIOControlBuffer(void __user *to, const void *from, nNIKAL100_tI32 count)
{
   if (copy_to_user(to, from, count))
      return nNIKAL100_kStatusOSFault;
   else
      return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_replicateInboundIOControlBuffer(void *to, const void __user *from, nNIKAL100_tI32 count)
{
   if (copy_from_user(to, from, count))
      return nNIKAL100_kStatusOSFault;
   else
      return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc void nNIKAL100_getKernelVersion(nNIKAL100_tU32 *major, nNIKAL100_tU32 *minor)
{
   nNIKAL100_tU32 linVer = LINUX_VERSION_CODE;
   *major = (linVer >> 16);
   *minor = (linVer&0xFFFF) >> 8;
}


nNIKAL100_cc nNIKAL100_tI32 nNIKAL100_vsprintf(nNIKAL100_tText* buf, const nNIKAL100_tText* fmt, va_list arg)
{
   nNIKAL120_mCheckStackUsage;
   return vsprintf(buf, fmt, arg);
}


nNIKAL100_cc nNIKAL100_tI32 nNIKAL100_vsnPrintf(nNIKAL100_tText* buf, nNIKAL100_tU32 size, const nNIKAL100_tText* fmt, va_list arg)
{
   nNIKAL120_mCheckStackUsage;
   return vsnprintf(buf, size, fmt, arg);
}


nNIKAL100_cc void nNIKAL100_printToDebugger(const nNIKAL100_tText* string)
{
   nNIKAL120_mCheckStackUsage;
   printk("%s", string);
}

nNIKAL100_cc void nNIKAL1500_printLineToDebugger(nNIKAL1500_tLogLevel logLevel, const char *message)
{
   switch (logLevel)
   {
      case nNIKAL1500_kLogLevelEmergency:
         printk(KERN_EMERG "%s\n", message);
         break;
      case nNIKAL1500_kLogLevelAlert:
         printk(KERN_ALERT "%s\n", message);
         break;
      case nNIKAL1500_kLogLevelCritical:
         printk(KERN_CRIT "%s\n", message);
         break;
      case nNIKAL1500_kLogLevelError:
         printk(KERN_ERR "%s\n", message);
         break;
      case nNIKAL1500_kLogLevelWarning:
         printk(KERN_WARNING "%s\n", message);
         break;
      case nNIKAL1500_kLogLevelNotice:
         printk(KERN_NOTICE "%s\n", message);
         break;
      case nNIKAL1500_kLogLevelInfo:
         printk(KERN_INFO "%s\n", message);
         break;
      case nNIKAL1500_kLogLevelDebug:
         printk(KERN_DEBUG "%s\n", message);
         break;
   }
}


static nNIKAL100_tU32 nNIKAL100_convertSizeToPageOrder(nNIKAL100_tUPtr size)
{
   return get_order(size);
}


nNIKAL100_cc void *nNIKAL100_malloc(nNIKAL100_tUPtr size)
{
   void *ptr = NULL;

   nNIKAL120_mCheckStackUsage;

   
   if (get_order(size) < nNIKAL1400_kMaxOrder )
   {
      ptr = kmalloc(size, GFP_KERNEL);
   }
   if (ptr == NULL)
   {
      ptr = vmalloc(size);
   }

   return ptr;
}


nNIKAL100_cc void *nNIKAL100_mallocContiguous(nNIKAL100_tUPtr size)
{
   void *ptr = NULL;
   nNIKAL100_tU32 pageOrder;
   nLinux_physicalPage * page;
   nNIKAL100_tStatus status = nNIKAL100_kStatusSuccess;

   nNIKAL120_mCheckStackUsage;

   pageOrder = get_order(size);
   if (pageOrder < nNIKAL1400_kMaxOrder )
   {
      ptr = kmalloc(size, GFP_KERNEL);
   }

   if (ptr == NULL)
   {
      page = alloc_pages(GFP_KERNEL,pageOrder);

      if (!page)
         return NULL;

      ptr = page_address(page);

      KAL_ASSERT((ptr == NULL) || nNIKAL100_isKernelContiguousPointer(ptr),
            "nNIKAL100_mallocContiguous allocated a non contiguous pointer!\n");

      
      down(&nNIKAL100_sMallocContiguousMapLock);
      status = nNIKAL100_mapInsert(&nNIKAL100_sMallocContiguousMap, (nNIKAL100_tUPtr)ptr, size);
      up(&nNIKAL100_sMallocContiguousMapLock);

      if (nNIKAL100_statusIsFatal(status))
      {
         free_pages((unsigned long)ptr,pageOrder);
         ptr = NULL;
      }
   }

   return ptr;
}


nNIKAL100_cc void *nNIKAL100_malloc32BitPhysicalContiguous(nNIKAL100_tUPtr size)
{
   void *ptr = NULL;

   dma_addr_t dma_handle;
   int headerSize;
#ifdef nNIKAL100_kDebuggingIsActive
   nNIKAL100_tUPtr address;
   u32 pageSize;
   nNIKAL100_tUPtr i;
   nNIKAL100_tU64 physAddr;
#endif

   nNIKAL120_mCheckStackUsage;

   headerSize = (sizeof(nNIKAL100_tUPtr) + sizeof(dma_addr_t) + 7) & -8;
   size = size + headerSize;
   ptr = dma_alloc_coherent(NULL, size, &dma_handle, GFP_KERNEL);

   if (ptr == NULL)
      return NULL;

   
   *(nNIKAL100_tUPtr *)ptr = size;
   *(dma_addr_t *)((nNIKAL100_tUPtr *)ptr + 1) = dma_handle;

#ifdef nNIKAL100_kDebuggingIsActive
   address = (nNIKAL100_tUPtr)ptr;
   pageSize = nNIKAL100_getPageSize();
   for (i=address; i < address + size; i+=pageSize)
   {
      physAddr = nNIKAL230_kernelVirtualToPhysical((void*)i);

      KAL_ASSERT( physAddr >> 32 == 0,
         "nNIKAL100_malloc32BitPhysical: Not 32 bit physical! virtualAddr: %lx physAddr: %llx\n",
            i, (long long)physAddr);
   }
#endif

   return (void *)((nNIKAL100_tUPtr)ptr + headerSize);
}


nNIKAL100_cc void nNIKAL100_free(void *pointer)
{
   nNIKAL100_tU32 pageOrder;
   nNIKAL100_tUPtr size = 0;
   nNIKAL100_tMapIterator mapIterator;

   nNIKAL120_mCheckStackUsage;

   if (!pointer)
      return;

   if (nNIKAL100_isKernelVirtualPointer(pointer))
   {
      vfree(pointer);
   }
   else
   {
      
      down(&nNIKAL100_sMallocContiguousMapLock);
      nNIKAL100_mapFind(&nNIKAL100_sMallocContiguousMap, (nNIKAL100_tUPtr)pointer, &mapIterator);
      if (mapIterator.keyValuePtr != NULL)
      {
         KAL_ASSERT(mapIterator.keyValuePtr->value != 0,
               "nNIKAL100_free(): found alloc_pages record but size == 0\n");

         size = mapIterator.keyValuePtr->value;
         nNIKAL100_mapRemove(&nNIKAL100_sMallocContiguousMap, &mapIterator);
      }
      up(&nNIKAL100_sMallocContiguousMapLock);

      if (size == 0)
      {
         kfree(pointer);
      }
      else
      {
         pageOrder = nNIKAL100_convertSizeToPageOrder(size);
         free_pages((unsigned long)pointer,pageOrder);
      }
   }
}


nNIKAL100_cc void nNIKAL100_free32BitPhysicalContiguous(void *pointer)
{
   nNIKAL100_tUPtr size;
   dma_addr_t dma_handle;
   int headerSize;

   if ( pointer == NULL)
      return;

   headerSize = (sizeof(nNIKAL100_tUPtr) + sizeof(dma_addr_t) + 7) & -8;
   pointer = (void *)((nNIKAL100_tUPtr)pointer - headerSize);
   size = *(nNIKAL100_tUPtr *)pointer;
   dma_handle = *(dma_addr_t *)((nNIKAL100_tUPtr *)pointer + 1);

   dma_free_coherent(NULL, size, pointer, dma_handle);
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getPageSize(void)
{
   return PAGE_SIZE;
}


nNIKAL100_cc nNIKAL100_tUPtr nNIKAL100_getPhysicalMemorySize(void)
{
   nNIKAL100_tU64 memorySize;
   nLinux_sysinfo systemInfo;

   KAL_ASSERT( PAGE_SIZE != 0, "nNIKAL100_getPhysicalMemorySize; PAGE_SIZE == 0!\n" );

   si_meminfo(&systemInfo);

   KAL_ASSERT( systemInfo.totalram != 0, "nNIKAL100_getPhysicalMemorySize; systemInfo.totalram == 0!\n" );

   memorySize = (nNIKAL100_tU64) systemInfo.totalram * (nNIKAL100_tU64) PAGE_SIZE;

#ifdef __i386__
   if ((memorySize >> 32) != 0)
   {
      KAL_DPRINT("nNIKAL100_getPhysicalMemorySize() overflow: %llx\n", (long long)memorySize);
      return 0;
   }
#endif

   return (nNIKAL100_tUPtr)memorySize;
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getNumberOfActiveProcessors(void)
{
   return num_online_cpus();
}



static void *nNIKAL100_mapUserKIOBuf(void *start, nNIKAL100_tUPtr size, nNIKAL100_tStatus *status)
{
   const nNIKAL100_tI32 sizeInPages = (size/PAGE_SIZE);
   
   nLinux_physicalPage **buffer;
   nLinux_physicalPage **pages;
   nNIKAL100_tI32 pagesMapped;
   int isWritable = 1;

   KAL_ASSERT(status != NULL, "NULL status passed to internal function\n");
   if (nNIKAL100_statusIsFatal(*status))
      return NULL;

   if (sizeInPages == 0)
   {
      KAL_DPRINT("nNIKAL100_mapUserKIOBuf(): sizeInPages == 0\n");
      *status = nNIKAL100_statusSelect(*status, nNIKAL100_kStatusValueConflict);
      return NULL;
   }

   
   if (current->mm == NULL)
   {
      KAL_DPRINT("nNIKAL100_mapUserKIOBuf(): called when user mode address space is already destroyed\n");
      *status = nNIKAL100_statusSelect(*status, nNIKAL100_kStatusResourceNotAvailable);
      return NULL;
   }

   buffer = (nLinux_physicalPage **)nNIKAL100_malloc(sizeof(nLinux_physicalPage **) * (sizeInPages+2));
   pages = buffer+2; 
   if (buffer == NULL)
   {
      KAL_DPRINT("nNIKAL100_mapUserKIOBuf(): Failed malloc(%lu)\n", sizeof(nLinux_physicalPage **) * (sizeInPages+2));
      *status = nNIKAL100_statusSelect(*status, nNIKAL100_kStatusMemoryFull);
      return NULL;
   }

   down_read(&(current->mm->mmap_sem));
   pagesMapped = nNIKAL1600_currentTaskGetUserPages((unsigned long) start, sizeInPages,
      isWritable, 0 , pages, NULL );
   up_read(&(current->mm->mmap_sem));

   if (pagesMapped < 0)
   {
      
      if (pagesMapped == -EFAULT)
      {
         isWritable = 0;
         down_read(&(current->mm->mmap_sem));
         pagesMapped = nNIKAL1600_currentTaskGetUserPages((unsigned long) start, sizeInPages,
            isWritable, 0 , pages, NULL );
         up_read(&(current->mm->mmap_sem));
      }
      
      if (pagesMapped < 0)
      {
         KAL_DPRINT("nNIKAL100_mapUserKIOBuf: FAILED to map read-only! pagesMapped = %ld sizeInPages = %ld\n", (long)pagesMapped, (long)sizeInPages);
         nNIKAL100_free(buffer);
         *status = nNIKAL100_statusSelect(*status, nNIKAL100_kStatusBadPointer);
         return NULL;
      }
   }
   if (pagesMapped == sizeInPages)
   {
      *(nNIKAL100_tUPtr*)buffer = sizeInPages;
      *((nNIKAL100_tUPtr*)buffer + 1) = (nNIKAL100_tUPtr)isWritable;
      return buffer;
   }

   KAL_DPRINT("nNIKAL100_mapUserKIOBuf: FAILED to map r/w! pagesMapped = %ld sizeInPages = %ld\n", (long)pagesMapped, (long)sizeInPages);
   
   for (;pagesMapped; --pagesMapped)
   {
      if (!PageReserved(pages[pagesMapped-1]))
      {
         put_page(pages[pagesMapped-1]);
      }
   }

   nNIKAL100_free(buffer);
   *status = nNIKAL100_statusSelect(*status, nNIKAL100_kStatusBadPointer);
   return NULL;
}


static void nNIKAL100_unmapUserKIOBuf(void *iobuf)
{
   nLinux_physicalPage **buffer = (nLinux_physicalPage **)iobuf;
   nLinux_physicalPage **pages = buffer+2;
   nNIKAL100_tUPtr pagesMapped = *(nNIKAL100_tUPtr*)buffer;
   nNIKAL100_tUPtr isWritable = *((nNIKAL100_tUPtr*)buffer + 1);

   for (;pagesMapped; --pagesMapped)
   {
      if (!PageReserved(pages[pagesMapped-1]))
      {
         
         if (isWritable) set_page_dirty(pages[pagesMapped-1]);

         
         put_page(pages[pagesMapped-1]);
      }
   }
   nNIKAL100_free(buffer);
}

static inline pmd_t *nNIKAL120_getPageTablePMDEntry(nLinux_mm* mm, const void *address)
{
   pgd_t *pgd;
#ifdef nNIKAL100_kFourLevelPageTable
   pud_t *pud;
#endif
   pmd_t *pmd;

   nNIKAL100_compileTimeAssert(sizeof(pte_t) <= 8, "pte_t is larger then 128 bits!\n");

   pgd = pgd_offset(mm, (nNIKAL100_tUPtr)address);
#ifdef nNIKAL100_kFourLevelPageTable
   pud = pud_offset(pgd, (nNIKAL100_tUPtr)address);
   pmd = pmd_offset(pud, (nNIKAL100_tUPtr)address);
#else
   pmd = pmd_offset(pgd, (nNIKAL100_tUPtr)address);
#endif
   return pmd;
}


static inline pte_t *nNIKAL100_getKernelPageTableEntry(nLinux_mm* mm, void *address)
{
   pmd_t *pmd;
   pte_t *pte;
   pmd = nNIKAL120_getPageTablePMDEntry(mm, address);

#ifdef nNIKAL100_kPTEOffsetKernel
   pte = pte_offset_kernel(pmd, (nNIKAL100_tUPtr)address);
   return pte;
#else
   pte = pte_offset(pmd, (nNIKAL100_tUPtr)address);
   return pte;
#endif
}


static inline pte_t *nNIKAL100_mapUserPageTableEntry(nLinux_mm* mm, void *address)
{
   pmd_t *pmd = nNIKAL120_getPageTablePMDEntry(mm, address);

#ifdef nNIKAL100_kPTEOffsetKernel
   return pte_offset_map(pmd, (nNIKAL100_tUPtr)address);
#else
   return pte_offset(pmd, (nNIKAL100_tUPtr)address);
#endif
}


static inline void nNIKAL100_unmapUserPageTableEntry(pte_t *pteToCleanup)
{
#ifdef nNIKAL100_kPTEOffsetKernel
   pte_unmap(pteToCleanup);
#endif
}


static nNIKAL100_tStatus nNIKAL100_remapPageRange(void *vmaPtr, nNIKAL100_tUPtr physAddr)
{
   int status;
   nLinux_vmArea *vma = (nLinux_vmArea*)vmaPtr;
   vma->vm_flags |= (nNIKAL240_kVM_RESERVED | VM_IO);

   status = remap_pfn_range(vma, vma->vm_start, physAddr >> PAGE_SHIFT,
      (vma->vm_end - vma->vm_start),
      vma->vm_page_prot);

   return nNIKAL100_convertLinuxToKALStatus(status);
}


static nNIKAL100_tUPtr nNIKAL100_doMMap(void *file, nNIKAL100_tUPtr address,
   nNIKAL100_tUPtr length, nNIKAL100_tUPtr offset)
{
   nNIKAL100_tUPtr mmapAddress;
   
   if (current->mm == NULL)
   {
      KAL_DPRINT("nNIKAL100_doMMap(): called when user mode address space is already destroyed\n");
      return -1; 
   }
#ifdef nNIKAL240_kHasVmMmap
   
   mmapAddress = vm_mmap((nLinux_fileHandle*)file, address, length, PROT_READ|PROT_WRITE,
      MAP_SHARED, offset);
#else
   down_write(&(current->mm->mmap_sem));
   mmapAddress = do_mmap((nLinux_fileHandle*)file, address, length, PROT_READ|PROT_WRITE,
      MAP_SHARED, offset);
   up_write(&(current->mm->mmap_sem));
#endif
   return mmapAddress;
}

static inline int nNIKAL240_do_munmap(struct mm_struct *mm, unsigned long addr, size_t len)
{
#ifdef nNIKAL100_kFourParameterDoMunmap
   return do_munmap(mm, addr, len, 1);
#else
   return do_munmap(mm, addr, len);
#endif
}


static nNIKAL100_tStatus nNIKAL100_doMUnmap(nNIKAL100_tUPtr address, nNIKAL100_tUPtr length)
{
   int status = 0;

   
   if (current->mm == NULL)
      return 0;

#ifdef nNIKAL240_kHasVmMunmap
   
   status = vm_munmap(address, length);
#else
   down_write(&current->mm->mmap_sem);
   status = nNIKAL240_do_munmap(current->mm, address, length);
   up_write(&(current->mm->mmap_sem));
#endif

   return nNIKAL100_convertLinuxToKALStatus(status);
}


nNIKAL100_cc void *nNIKAL180_memCpy(void *dest, const void *src, size_t size)
{
   return memcpy(dest, src, size);
}


nNIKAL100_cc void *nNIKAL180_memMove(void *dest, const void *src, size_t size)
{
   return memmove(dest, src, size);
}


nNIKAL100_cc void *nNIKAL240_memSet(void *buffer, nNIKAL100_tU8 data, size_t size)
{
   return memset(buffer, data, size);
}


nNIKAL100_cc int nNIKAL250_memCmp(const void *buffer1, const void* buffer2, size_t count)
{
    return memcmp(buffer1, buffer2, count);
}


nNIKAL100_cc size_t nNIKAL250_strLen(const char *str)
{
   return strlen(str);
}


nNIKAL100_cc int nNIKAL250_strCmp(const char *str1, const char *str2)
{
   return strcmp(str1, str2);
}


nNIKAL100_cc int nNIKAL1700_sScanf(const char *str, const char *format, ...)
{
   va_list ap;
   int count = 0;
   va_start(ap, format);
   count = vsscanf(str, format, ap);
   va_end(ap);

   return count;
}


nNIKAL100_cc int nNIKAL1700_vsScanf(const char *str, const char *format, va_list ap)
{
   int count = 0;
   count = vsscanf(str, format, ap);

   return count;
}


nNIKAL100_cc char *nNIKAL250_strnCpy(char *dest, const char *src, size_t count)
{
   return strncpy(dest, src, count);
}

static nNIKAL100_tUPtr nNIKAL230_kernelVirtualToPhysical(const void *ptr)
{
   nLinux_physicalPage *page;

   
   if (nNIKAL100_isKernelContiguousPointer(ptr))
      return __pa(ptr);

   page = vmalloc_to_page(ptr);
   if (page)
      return page_to_phys(page) + ((unsigned long) ptr & (PAGE_SIZE - 1));

   return 0;
}


nNIKAL100_cc nNIKAL100_tUPtr nNIKAL100_getPhysicalAddress(const void *ptr, nNIKAL100_tAddressOrigin addressOrigin)
{
   pmd_t *_pmd;
   pte_t *_pte;
   nLinux_physicalPage *_page = NULL;

   if (addressOrigin == nNIKAL100_kAddressOriginKernel)
      return nNIKAL230_kernelVirtualToPhysical(ptr);

   spin_lock(&(current->mm->page_table_lock));
   _pmd = nNIKAL120_getPageTablePMDEntry(current->mm, ptr);

   if (pmd_present(*_pmd))
   {
#ifdef nNIKAL100_kPTEOffsetKernel
      _pte = pte_offset_map(_pmd, (nNIKAL100_tUPtr)ptr);
#else
      _pte = pte_offset(_pmd, (nNIKAL100_tUPtr)ptr);
#endif
      if (pte_present(*_pte))
         _page = pte_page(*_pte);

#ifdef nNIKAL100_kPTEOffsetKernel
      pte_unmap(_pte);
#endif
   }
   spin_unlock(&(current->mm->page_table_lock));

   if (_page)
   {
      nNIKAL100_tU64 retVal = (page_to_phys(_page) + ((nNIKAL100_tUPtr)ptr & (PAGE_SIZE-1)));
#ifdef __i386__
      if ((retVal >> 32) != 0)
      {
         KAL_DPRINT("nNIKAL100_getPhysicalAddress() overflow: %llx\n", (long long)retVal);
         return 0;
      }
#endif
      return (nNIKAL100_tUPtr)retVal;
   }
   else
   {
      KAL_ASSERT(0, "nNIKAL100_getPhysicalAddress: Failed to walk the page table!\n");
      return 0;
   }
}


static void *nNIKAL100_incrementPageRefcount(const void *ptr)
{
   nNIKAL100_tUPtr physAddr = nNIKAL230_kernelVirtualToPhysical(ptr);
   nLinux_physicalPage *page = virt_to_page(__va(physAddr)); 
   get_page(page); 
   return page;
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getTimerInterval (void)
{
   return 1000000/HZ;
}




nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_sleepTimeoutInterruptible(
   nNIKAL100_tU32 timeToSleepInJiffies)
{
   set_current_state(TASK_INTERRUPTIBLE);
   return schedule_timeout(timeToSleepInJiffies);
}



nNIKAL100_cc void nNIKAL100_sleepTimeout (nNIKAL100_tU32 timeToSleepInJiffies)
{
   set_current_state(TASK_UNINTERRUPTIBLE);
   schedule_timeout(timeToSleepInJiffies);
   return;
}


nNIKAL100_cc void nNIKAL250_sleepTimeout (nNIKAL100_tU32 timeToSleepInMicroseconds)
{
#ifdef nNIKAL100_kHighresTimerAvailable
   usleep_range(timeToSleepInMicroseconds, timeToSleepInMicroseconds);
#else
   nNIKAL100_tU32 timerInterval = nNIKAL100_getTimerInterval();
   nNIKAL100_tU32 timeInJiffies = (timeToSleepInMicroseconds + timerInterval - 1) / timerInterval;
   nNIKAL100_sleepTimeout(timeInJiffies);
#endif
}


nNIKAL100_cc void nNIKAL200_yield (void)
{
   schedule();
}


nNIKAL100_cc nNIKAL100_tU64 nNIKAL110_getTimerCount (void)
{
   return get_jiffies_64();
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getTimerCount (void)
{
   return jiffies;
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getTimeOfDayInterval (void)
{
   const nNIKAL100_tU32 oneMicrosecond = 1;

#ifdef CONFIG_X86_TSC
   

   return oneMicrosecond;

#else
   
   const nNIKAL100_tU32 microsecondsPerSecond = 1000000;
   nNIKAL100_tU32 clockTickInterval;

   clockTickInterval = microsecondsPerSecond/HZ;
   if (clockTickInterval == 0)
   {
      return oneMicrosecond;
   }
   else
   {
      return clockTickInterval;
   }

#endif
}


nNIKAL100_cc void nNIKAL100_getTimeOfDay (nNIKAL100_tU32 *systemSeconds,
   nNIKAL100_tU32 *systemMicroSeconds)
{
   nLinux_timeSpec systemTime;
   getnstimeofday(&systemTime);
   *systemSeconds = systemTime.tv_sec;
   *systemMicroSeconds = (systemTime.tv_nsec / 1000);
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getThreadID( void )
{
   return (nNIKAL100_tU32)current->pid;
}


nNIKAL100_cc void *nNIKAL100_getThreadHandle( void )
{
   return (void *)current;
}


static int nNIKAL100_threadMain( void * paramBlock )
{
   nNIKAL100_tCreateThreadParamBlock *_paramBlock =
      (nNIKAL100_tCreateThreadParamBlock *)paramBlock;
   nNIKAL100_tThreadFunction _threadFunction = _paramBlock->threadFunction;
   void *_threadContext = _paramBlock->threadContext;

   if (atomic_dec_and_test(&(_paramBlock->counter)))
   {
      nNIKAL100_releaseSingleUseEvent( _paramBlock->threadCreatedEvent );
   }

   return _threadFunction(_threadContext);
}
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_createSystemThread( void *threadContext, nNIKAL100_tI32 (*nNIKAL100_cc threadMain)(void *), nNIKAL100_tUPtr *childPID )
{
   nNIKAL100_tCreateThreadParamBlock paramBlock;
   nLinux_task *task;

   
   nNIKAL100_tSingleUseEvent eventNode;
   nNIKAL100_initializeSingleUseEvent(&eventNode);

   paramBlock.threadCreatedEvent = &eventNode;
   paramBlock.threadContext = threadContext;
   paramBlock.threadFunction = threadMain;

   atomic_set(&(paramBlock.counter),1);
   task = kthread_run(nNIKAL100_threadMain, &paramBlock, "niDriverThread");
   if(IS_ERR(task))
   {
      *childPID=0;
      return nNIKAL100_convertLinuxToKALStatus(PTR_ERR(task));
   }
   paramBlock.childThreadID = task->pid;

   
   nNIKAL100_waitForSingleUseEvent( &eventNode );

   *childPID = paramBlock.childThreadID;
   return nNIKAL100_kStatusSuccess;
}

static int nNIKAL200_threadMain( void * paramBlock )
{
   nNIKAL200_tThread *threadParam = (nNIKAL200_tThread*)paramBlock;
   nNIKAL100_tI32 ret;

   ret = threadParam->threadMain(threadParam->context);

   
   nNIKAL100_releaseSingleUseEvent(&(threadParam->thdDeadEvent));

   
   while (!kthread_should_stop())
   {
      set_current_state(TASK_UNINTERRUPTIBLE);

      
      if (!kthread_should_stop())
      {
         schedule(); 
      }

      set_current_state(TASK_RUNNING);
   }

   return ret;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL250_createNamedThread( nNIKAL200_tThread * threadParam, char const * threadName )
{
   nLinux_task *task;

   KAL_ASSERT(threadParam != NULL);
   KAL_ASSERT(threadName != NULL);

   nNIKAL100_initializeSingleUseEvent(&(threadParam->thdDeadEvent));
   task = kthread_run(nNIKAL200_threadMain, threadParam, "%s", threadName);
   if(IS_ERR(task))
   {
      threadParam->task = NULL;
      threadParam->childPID = 0;
      return nNIKAL100_convertLinuxToKALStatus(PTR_ERR(task));
   }

   threadParam->task = task;
   threadParam->childPID = task->pid;
   return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL200_createThread( nNIKAL200_tThread * threadParam )
{
   return nNIKAL250_createNamedThread(threadParam, "niDriverThread");
}


nNIKAL100_cc void nNIKAL200_joinThread( nNIKAL200_tThread *thread )
{
   
   nNIKAL100_waitForSingleUseEvent(&(thread->thdDeadEvent));
   kthread_stop((nLinux_task*)(thread->task));
}


nNIKAL100_cc void * nNIKAL150_createMutex( void )
{
#ifdef nNIKAL150_kMutexMethod
   nLinux_mutex *_mutex;  

   if((_mutex = nNIKAL100_malloc( sizeof( nLinux_mutex ) )) != NULL )
   {
      
      mutex_init( _mutex );
   }

   
   return (void *)_mutex;
#else
   return nNIKAL100_createSemaphore(1);
#endif
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL150_destroyMutex( void * mutex )
{
#ifdef nNIKAL150_kMutexMethod
   
   nLinux_mutex *_mutex;
   if(!mutex) return nNIKAL100_kStatusSuccess;
   _mutex = (nLinux_mutex *)mutex;

   
   nNIKAL100_free( _mutex );

   return nNIKAL100_kStatusSuccess;
#else
   return nNIKAL100_destroySemaphore(mutex);
#endif
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL150_acquireMutex( void * mutex )
{
#ifdef nNIKAL150_kMutexMethod
   
   nLinux_mutex *_mutex = (nLinux_mutex *)mutex;

   nNIKAL120_mCheckStackUsage;

   
   mutex_lock( _mutex );

   
   return nNIKAL100_kStatusSuccess;
#else
   return nNIKAL100_acquireSemaphore(mutex);
#endif
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL150_acquireMutexInterruptible( void * mutex )
{
#ifdef nNIKAL150_kMutexMethod
   int status;

   
   nLinux_mutex *_mutex = (nLinux_mutex *)mutex;

   nNIKAL120_mCheckStackUsage;

   status = mutex_lock_interruptible( _mutex );
   return nNIKAL100_convertLinuxToKALStatus(status);
#else
   return nNIKAL100_acquireSemaphoreInterruptible(mutex);
#endif
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL150_acquireMutexZeroTimeout( void * mutex )
{
#ifdef nNIKAL150_kMutexMethod
   
   nLinux_mutex *_mutex = (nLinux_mutex *)mutex;

   nNIKAL120_mCheckStackUsage;

   
   if ( mutex_trylock( _mutex ) == 1 )
   {
      return nNIKAL100_kStatusSuccess;
   }
   else
   {
      return nNIKAL100_kStatusSyncTimedOut;
   }
#else
   return nNIKAL100_acquireSemaphoreZeroTimeout(mutex);
#endif
}


nNIKAL100_cc void nNIKAL150_releaseMutex( void * mutex )
{
#ifdef nNIKAL150_kMutexMethod
   
   nLinux_mutex *_mutex = (nLinux_mutex *)mutex;

   nNIKAL120_mCheckStackUsage;

   
   mutex_unlock( _mutex );

#else
   nNIKAL100_releaseSemaphore(mutex);
#endif
}


nNIKAL100_cc void * nNIKAL100_createSemaphore( nNIKAL100_tI32 initVal )
{
   nLinux_semaphore *_sema;  

   if((_sema = nNIKAL100_malloc( sizeof( nLinux_semaphore ) )) != NULL )
   {
      
      sema_init( _sema, initVal );
   }

   
   return (void *)_sema;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_destroySemaphore( void * sema )
{
   
   nLinux_semaphore *_sema;
   if(!sema) return nNIKAL100_kStatusSuccess;
   _sema = (nLinux_semaphore *)sema;

   
   nNIKAL100_free( _sema );

   return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_acquireSemaphore( void * sema )
{
   
   nLinux_semaphore *_sema = (nLinux_semaphore *)sema;

   nNIKAL120_mCheckStackUsage;

   
   down( _sema );

   
   return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_acquireSemaphoreInterruptible( void * sema )
{
   int status;

   
   nLinux_semaphore *_sema = (nLinux_semaphore *)sema;

   nNIKAL120_mCheckStackUsage;

   status = down_interruptible( _sema );
   return nNIKAL100_convertLinuxToKALStatus(status);
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_acquireSemaphoreZeroTimeout( void * sema )
{
   
   nLinux_semaphore *_sema = (nLinux_semaphore *)sema;

   nNIKAL120_mCheckStackUsage;

   
   if ( down_trylock( _sema ) == 0 )
   {
      return nNIKAL100_kStatusSuccess;
   }
   else
   {
      return nNIKAL100_kStatusSyncTimedOut;
   }

}


nNIKAL100_cc nNIKAL100_tI32 nNIKAL100_releaseSemaphore( void * sema )
{
   
   nLinux_semaphore *_sema = (nLinux_semaphore *)sema;

   nNIKAL120_mCheckStackUsage;

   
   up( _sema );

   
   return 0;
}



nNIKAL100_cc void * nNIKAL100_createSpinLock( void )
{
   nLinux_spinlock *_spinlock;  

   if(( _spinlock = nNIKAL100_malloc( sizeof( nLinux_spinlock ) )) != NULL )
   {
      
      spin_lock_init( _spinlock );
   }

   
   return (void *)_spinlock;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_destroySpinLock( void *spinlock )
{
   nLinux_spinlock *_spinlock;
   if(!spinlock) return nNIKAL100_kStatusSuccess;
   _spinlock = (nLinux_spinlock *)spinlock;  

   
   if( spin_is_locked( _spinlock ) )
   {
      
      return nNIKAL100_kStatusResourceBusy;
   }

   nNIKAL100_free( _spinlock );

   
   return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc nNIKAL100_tUPtr nNIKAL110_acquireSpinLockInterrupt( void *spinlock )
{
   nLinux_spinlock *_spinlock = (nLinux_spinlock *)spinlock;  
   nNIKAL100_tUPtr _flags;                              

   spin_lock_irqsave( _spinlock, _flags );

   
   return _flags;
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_acquireSpinLockInterrupt( void *spinlock )
{
   return (nNIKAL100_tU32)nNIKAL110_acquireSpinLockInterrupt( spinlock );
}


nNIKAL100_cc void nNIKAL110_releaseSpinLockInterrupt( void *spinlock, nNIKAL100_tUPtr flags )
{
   
   spin_unlock_irqrestore( (nLinux_spinlock *)spinlock, flags );
}


nNIKAL100_cc void nNIKAL100_releaseSpinLockInterrupt( void *spinlock, nNIKAL100_tU32 flags )
{
   nNIKAL110_releaseSpinLockInterrupt( spinlock, (nNIKAL100_tUPtr)flags );
}


nNIKAL100_cc void nNIKAL100_acquireSpinLockDPC( void * spinlock )
{
   spin_lock_bh( (nLinux_spinlock *)spinlock );
}


nNIKAL100_cc void nNIKAL100_releaseSpinLockDPC( void *spinlock )
{
   
   spin_unlock_bh( (nLinux_spinlock *)spinlock );
}


nNIKAL100_cc void nNIKAL100_initializeSingleUseEvent( void *event )
{
#if   defined(nNIKAL200_kCompletion)
   struct completion *_event = (struct completion *)event;
   init_completion(_event);
#else
   nNIKAL100_tSingleUseEvent *_event = event;
   _event->done = 0;
   init_waitqueue_head(&(_event->waitQueue));
#endif
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getSingleUseEventSize( void )
{
   nNIKAL100_compileTimeAssert(sizeof(nNIKAL100_tSingleUseEvent) < nNIKAL100_kSingleUseEventSize, "nNIKAL100_kSingleUseEventSize is too small\n");
   return sizeof( nNIKAL100_tSingleUseEvent );
}


nNIKAL100_cc void nNIKAL100_releaseSingleUseEvent( void *event )
{
#if   defined(nNIKAL200_kCompletion)
   struct completion *_event = (struct completion *)event;
   complete(_event);
#else
   nNIKAL100_tSingleUseEvent *_event = (nNIKAL100_tSingleUseEvent *)event;
   nNIKAL120_mCheckStackUsage;

   nNIKAL100_tUPtr flags = nNIKAL110_acquireSpinLockInterrupt( &_event->waitQueue.lock );
   _event->done = 1;
   wake_up(&(_event->waitQueue));
   nNIKAL110_releaseSpinLockInterrupt( &_event->waitQueue.lock, flags );
#endif
}


static void nNIKAL100_timeoutCallback( unsigned long package )
{
   nNIKAL100_tTimerCallbackSpec *_timerCallbackPkg =
      (nNIKAL100_tTimerCallbackSpec *)package;

   _timerCallbackPkg->func( _timerCallbackPkg->context );
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_waitForSingleUseEvent( void *event )
{
#if   defined(nNIKAL200_kCompletion)
   struct completion *_event = (struct completion *)event;

   
   if ((current->mm == NULL) && !(current->flags & PF_EXITING)) {
      int status;

      status = wait_for_completion_interruptible(_event);

      KAL_ASSERT(status != -ERESTARTSYS, "Interruptible sleep from kernel thread interrupted.\n");
   } else {
      wait_for_completion(_event);
   }
#else

   nNIKAL100_tSingleUseEvent *_event = (nNIKAL100_tSingleUseEvent *)event;

   
   if ((current->mm == NULL) && !(current->flags & PF_EXITING))
   {
      return nNIKAL100_waitForSingleUseEventInterruptible(event);
   }
   else
   {
      

      
      
      
      DEFINE_WAIT(wait);
      if (!(_event->done))
      {
         prepare_to_wait_exclusive(&(_event->waitQueue), &wait, TASK_UNINTERRUPTIBLE);
         if (!(_event->done))
            schedule();
         finish_wait(&(_event->waitQueue), &wait);
      }
   }
#endif
   nNIKAL120_mCheckStackUsage;

   return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_waitForSingleUseEventInterruptible ( void *event )
{
   nNIKAL100_tStatus retVal = nNIKAL100_kStatusSuccess;

#if   defined(nNIKAL200_kCompletion)
   struct completion *_event = (struct completion *)event;
   retVal = nNIKAL100_convertLinuxToKALStatus(wait_for_completion_interruptible( _event ));
#else
   nNIKAL100_tSingleUseEvent *_event = (nNIKAL100_tSingleUseEvent *)event;
   
   DEFINE_WAIT(wait);
   if (!(_event->done))
   {
      prepare_to_wait_exclusive(&(_event->waitQueue), &wait, TASK_INTERRUPTIBLE);
      while (!(_event->done))
      {
         if (signal_pending(current))
         {
            retVal = nNIKAL100_kStatusWaitInterrupted;
            break;
         }
         schedule();
      }
      finish_wait(&(_event->waitQueue), &wait);
   }
#endif
   nNIKAL120_mCheckStackUsage;

   return retVal;
}


static unsigned long calculateTimerExpire( nNIKAL100_tU32 timeout )
{
   unsigned long expires = 0;

   

   
   if( timeout > 2000000 )
   {
      
      expires = ((timeout / 1000) * HZ) + (((timeout % 1000) * HZ) / 1000) + jiffies + 1;
   }
   else
   {
      expires  = ((timeout * HZ) / 1000 ) + jiffies + 1;
   }

   return expires;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_waitForSingleUseEventTimeout
(
   void *event,
   nNIKAL100_tU32 timeout,
   nNIKAL100_tTimerCallbackSpec *timerCallbackPkg
)
{
   nLinux_timerList timer;
   nNIKAL100_tStatus retVal = nNIKAL100_kStatusSuccess;

   nNIKAL120_mCheckStackUsage;

   
   init_timer( &timer );

   timer.expires  = calculateTimerExpire( timeout );
   nNIKAL100_compileTimeAssert(sizeof(nNIKAL100_tTimerCallbackSpec*)==sizeof(unsigned long), "Oops, resizing nNIKAL100_tTimerCallbackSpec* into unsigned long!");
   timer.data     = (unsigned long) timerCallbackPkg;
   timer.function = nNIKAL100_timeoutCallback;

   
   add_timer( &timer );

   
   retVal = nNIKAL100_waitForSingleUseEvent( event );

   del_timer_sync( &timer );

   return retVal;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_waitForSingleUseEventTimeoutInterruptible
(
   void *event,
   nNIKAL100_tU32 timeout,
   nNIKAL100_tTimerCallbackSpec *timerCallbackPkg
)
{
   nLinux_timerList timer;
   nNIKAL100_tStatus retVal = nNIKAL100_kStatusSuccess;

   nNIKAL120_mCheckStackUsage;

   
   init_timer( &timer );

   timer.expires  = calculateTimerExpire( timeout );
   timer.data     = (unsigned long) timerCallbackPkg;
   timer.function = nNIKAL100_timeoutCallback;

   
   add_timer( &timer );

   retVal = nNIKAL100_waitForSingleUseEventInterruptible(event);

   del_timer_sync( &timer );

   return retVal;  
}


static inline nNIKAL100_tPCIDeviceIDTableFooter *nNIKAL100_getPCIDeviceIDTableFooter(
   const nLinux_pciDeviceID *entry)
{
   nNIKAL100_tPCIDeviceIDTableFooter *tableFooter;
   nNIKAL100_tU32 i = 0;

   
   for (; (entry->vendor != 0) || (entry->device != 0) ||
      (entry->subvendor != 0) || (entry->subdevice != 0) || (entry->class != 0); ++entry)
   {}

   
   ++i;
   ++entry;
   tableFooter = (nNIKAL100_tPCIDeviceIDTableFooter*)entry;
   while (tableFooter->signature != nNIKAL100_kPCIDeviceIDTableFooterSignature)
   {
      ++i;
      ++entry;
      tableFooter = (nNIKAL100_tPCIDeviceIDTableFooter*)entry;
   }
   
   return tableFooter;
}


static int nNIKAL200_commonPCIProbe(
   nLinux_pciDevice *dev, const nLinux_pciDeviceID *id, nNIKAL100_tDriver *driver)
{
   int               status;
   nNIKAL100_tU8     irqPin;
   nNIKAL100_tU32    i, ioCount;
   nNIKAL100_tU8     pciRevision;

   nNIKAL200_tDevice *device = nNIKAL200_constructDevice(driver);

   if (unlikely(!device)) {
      status = -ENOMEM;
      goto out;
   }

   device->deviceInfo.osDevice = (void *) dev;
   device->deviceInfo.busType = nNIKAL100_kBusTypePCI;
   device->deviceInfo.version = 1;
   device->deviceInfo.pci.deviceID.bus      = dev->bus->number;

   

   device->deviceInfo.pci.deviceID.slot     = PCI_SLOT(dev->devfn);
   device->deviceInfo.pci.deviceID.function = PCI_FUNC(dev->devfn);
   device->deviceInfo.pci.deviceID.vendor   = dev->vendor;
   device->deviceInfo.pci.deviceID.device   = dev->device;
   device->deviceInfo.pci.deviceID.subSystemVendor = dev->subsystem_vendor;
   device->deviceInfo.pci.deviceID.subSystemDevice = dev->subsystem_device;

   status = pci_enable_device(dev);

   if (status)
      goto free_device;

   pci_read_config_byte(dev, PCI_CLASS_REVISION, &pciRevision);
   device->deviceInfo.pci.revision = pciRevision;
   pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &irqPin);

   if (irqPin) {
      device->deviceInfo.pci.irqSize = 1;
      device->deviceInfo.pci.irq = dev->irq;
   }

   
   pci_set_master(dev);
   for (i=ioCount=0; i<nNIKAL100_kPCIBARMax; ++i) {
      if (pci_resource_len(dev, i) == 0)
         continue;

      device->deviceInfo.pci.ioResource[i].start  = pci_resource_start(dev, i);
      device->deviceInfo.pci.ioResource[i].size   = pci_resource_len(dev, i);

      if (pci_resource_flags(dev, i) & IORESOURCE_IO)
         device->deviceInfo.pci.ioResource[i].flags |= nNIKAL100_kPCIIOResourceFlagTypeIO;
      else if (pci_resource_flags(dev, i) & IORESOURCE_MEM)
         device->deviceInfo.pci.ioResource[i].flags |= nNIKAL100_kPCIIOResourceFlagTypeMem;

      ++ioCount;
   }
   device->deviceInfo.pci.ioResourceSize = ioCount;

   snprintf(device->nameString, sizeof(device->nameString), "pci:%s", pci_name(dev));

   status = nNIKAL200_createDeviceProcDirs(device->nameString, device);

   if (nNIKAL100_statusIsFatal(status))
      goto disable_dev;

   
   device->deviceExtension = device;

   KAL_ASSERT(device->driver->addDevice);

   status = device->driver->addDevice(&device->deviceInfo,
                                      (void*)id->driver_data,
                                      &device->deviceExtension);

   if (nNIKAL100_statusIsFatal(status))
      goto remove_proc;

   device->addDeviceSuccess = 1;
   pci_set_drvdata(dev, device);

   
   return 0;

remove_proc:
   KAL_DPRINT("Error: %d. Add device failed. Removing proc entries for '%s'\n",
      status, device->nameString);
   nNIKAL200_destroyDeviceProcDirs(device);
disable_dev:
   KAL_DPRINT("Error: %d. Disabling device (dev=%p)\n", status, dev);
   pci_disable_device(dev);
free_device:
   KAL_DPRINT("Error: %d. Destroying device (device=%p)\n", status, device);
   nNIKAL200_destructDevice(&(device->kref));
out:
   return -ENODEV;
}
static int nNIKAL100_pciProbe(nLinux_pciDevice *dev, const nLinux_pciDeviceID *id)
{
   return nNIKAL200_commonPCIProbe(dev, id, nNIKAL100_getPCIDeviceIDTableFooter(id)->driver);
}
static int nNIKAL200_pciProbe(nLinux_pciDevice *dev, const nLinux_pciDeviceID *id)
{
   return nNIKAL200_commonPCIProbe(dev, id, (nNIKAL100_tDriver *)(id->driver_data));
}


static void nNIKAL100_pciRemove(nLinux_pciDevice *dev)
{
   nNIKAL200_tDevice *d = pci_get_drvdata(dev);

   pci_set_drvdata(dev, NULL);

   KAL_ASSERT(d->driver->removeDevice);
   d->driver->removeDevice(d->deviceExtension);

   nNIKAL200_destroyDeviceProcDirs(d);

   pci_disable_msi(dev); 
   pci_disable_device(dev);

   kref_put(&(d->kref), nNIKAL200_destructDevice);
}


nNIKAL100_cc void *nNIKAL100_registerPCIDriver(nNIKAL100_tDriver *driver,
   const nNIKAL100_tText *name, void *pciDevIdTable)
{
   nNIKAL100_tPCIDeviceIDTableFooter* tableFooter =
      (nNIKAL100_tPCIDeviceIDTableFooter*)pciDevIdTable;
   nLinux_pciDriver *pciDriver;
   int ret;

   tableFooter->driver = driver;

   pciDriver = nNIKAL100_malloc(sizeof(nLinux_pciDriver));
   if (pciDriver == NULL)
      return NULL;

   memset(pciDriver, 0, sizeof(nLinux_pciDriver));

   pciDriver->name = (char*)name;
   pciDriver->id_table = tableFooter->table;
   pciDriver->probe = nNIKAL100_pciProbe;
   pciDriver->remove = nNIKAL100_pciRemove;
   pciDriver->suspend = NULL;
   pciDriver->resume = NULL;
   ret = pci_register_driver(pciDriver);
   if ( ret < 0 )
   {
      nNIKAL100_free(pciDriver);
      return NULL;
   }

   return pciDriver;
}


nNIKAL100_cc void *nNIKAL200_registerPCIDriver(void* inPciDriver)
{
   int ret;
   nLinux_pciDriver *pciDriver = (nLinux_pciDriver*)inPciDriver;

   pciDriver->probe = nNIKAL200_pciProbe;
   pciDriver->remove = nNIKAL100_pciRemove;
   pciDriver->suspend = NULL;
   pciDriver->resume = NULL;
   ret = pci_register_driver(pciDriver);
   if ( ret < 0 )
      return NULL;

   return pciDriver;
}


nNIKAL100_cc void nNIKAL100_unregisterPCIDriver(void *pciDriverData, const nNIKAL100_tText ** name,
   void **pciDevIdTable)
{
   nLinux_pciDriver *pciDriver = (nLinux_pciDriver*)pciDriverData;
   nNIKAL100_tPCIDeviceIDTableFooter* tableFooter =
      nNIKAL100_getPCIDeviceIDTableFooter(pciDriver->id_table);
   *name = pciDriver->name;
   *pciDevIdTable = (void*)(tableFooter);
   pci_unregister_driver(pciDriver);
   nNIKAL100_free(pciDriver);
}


nNIKAL100_cc void nNIKAL200_unregisterPCIDriver(void *pciDriverData)
{
   nLinux_pciDriver *pciDriver = (nLinux_pciDriver*)pciDriverData;
   pci_unregister_driver(pciDriver);
}


nNIKAL100_cc void *nNIKAL100_createPCIDeviceIDTable(nNIKAL100_tU32 size)
{
   nNIKAL100_tText *buffer;
   nNIKAL100_tPCIDeviceIDTableFooter *pciDeviceIDTableFooter;

   
   nNIKAL100_tU32 bufferSize = ((size+1)*sizeof(nLinux_pciDeviceID)) + sizeof(nNIKAL100_tPCIDeviceIDTableFooter);
   buffer = (nNIKAL100_tText*)nNIKAL100_malloc(bufferSize);
   if (buffer == NULL)
      return NULL;

   memset(buffer, 0, bufferSize);

   
   pciDeviceIDTableFooter = (nNIKAL100_tPCIDeviceIDTableFooter*)
      (buffer + ((size+1)*sizeof(nLinux_pciDeviceID)));
   pciDeviceIDTableFooter->signature = nNIKAL100_kPCIDeviceIDTableFooterSignature;
   pciDeviceIDTableFooter->capacity = size;
   pciDeviceIDTableFooter->size = 0;
   pciDeviceIDTableFooter->driver = NULL;
   pciDeviceIDTableFooter->table = (nLinux_pciDeviceID*)buffer;
   return (void*)pciDeviceIDTableFooter;
}


nNIKAL100_cc void nNIKAL100_setPCIDeviceIDTableElement(void *pciDevIdTable,
   nNIKAL100_tU32 vendor, nNIKAL100_tU32 device, nNIKAL100_tU32 subsysVendor,
   nNIKAL100_tU32 subsysDevice, nNIKAL100_tU32 devClass,
   nNIKAL100_tU32 devClassMask, void *privateData)
{
   nNIKAL100_tPCIDeviceIDTableFooter* tableFooter =
      (nNIKAL100_tPCIDeviceIDTableFooter*)pciDevIdTable;
   nLinux_pciDeviceID *idTable = tableFooter->table;
   nNIKAL100_tU32 index = tableFooter->size;

   ++(tableFooter->size);
   KAL_ASSERT(tableFooter->size <= tableFooter->capacity, "nNIKAL100_setPCIDeviceIDTableElement: pciDeviceIDTable size exceeds capacity\n");

   idTable[index].vendor = vendor;
   idTable[index].device = device;

   
   if (subsysVendor == nNIKAL100_kPCIDontCare && subsysDevice == nNIKAL100_kPCIDontCare)
   {
      idTable[index].subvendor = PCI_ANY_ID;
      idTable[index].subdevice = PCI_ANY_ID;
   }
   else if ( subsysVendor != nNIKAL100_kPCIDontCare && subsysDevice != nNIKAL100_kPCIDontCare)
   {
      idTable[index].subvendor = subsysVendor;
      idTable[index].subdevice = subsysDevice;
   }
   else
   {
      KAL_ASSERT(0,"nNIKAL100_setPCIDeviceIDTableElement: subsysVendor and subsysDevice should be specified together\n");
   }

   idTable[index].class = devClass;
   idTable[index].class_mask = devClassMask;
   nNIKAL100_compileTimeAssert(sizeof(void*)==sizeof(unsigned long), "Oops, resizing void* into unsigned long!");
   idTable[index].driver_data = (unsigned long)privateData;
}


nNIKAL100_cc void nNIKAL100_destroyPCIDeviceIDTable(void *pciDevIdTable)
{
   nNIKAL100_tPCIDeviceIDTableFooter* tableFooter;
   if(!pciDevIdTable) return;
   tableFooter = (nNIKAL100_tPCIDeviceIDTableFooter*)pciDevIdTable;
   nNIKAL100_free(tableFooter->table);
}


nNIKAL100_cc void nNIKAL100_enumeratePCIBuses(nNIKAL100_tPCIBridgeVisitor visitor,
   void *visitorArgument,
   void *listToEnumerate)
{
#ifdef CONFIG_PCI
   nLinux_listHead *busList = (nLinux_listHead*)listToEnumerate;
   nLinux_listHead *iterator;
   nLinux_pciBus *bus;
   nNIKAL100_tPCIBridgeInfo pciBusInformation;

   
   if (busList == NULL)
      busList = &pci_root_buses;

   
   list_for_each(iterator, busList)
   {
      bus = list_entry(iterator, nLinux_pciBus, node);
      if (bus->self)
      {
         pciBusInformation.deviceID.bus = bus->primary;
         pciBusInformation.deviceID.slot = PCI_SLOT(bus->self->devfn);
         pciBusInformation.deviceID.function = PCI_FUNC(bus->self->devfn);
         pciBusInformation.deviceID.vendor = bus->self->vendor;
         pciBusInformation.deviceID.device = bus->self->device;
         pciBusInformation.deviceID.subSystemVendor = bus->self->subsystem_vendor;
         pciBusInformation.deviceID.subSystemDevice = bus->self->subsystem_device;
         if (bus->self->hdr_type == PCI_HEADER_TYPE_BRIDGE)
         {
            nNIKAL100_tI32 subSystemCapOffset = pci_find_capability(bus->self, PCI_CAP_ID_SSVID);
            if( subSystemCapOffset )
            {
               nNIKAL100_tU32 subSystemID = 0;
               nNIKAL100_pciConfigRead32(bus->self, subSystemCapOffset + nNIKAL100_kSubsystemIDOffset, &subSystemID);
               pciBusInformation.deviceID.subSystemVendor = subSystemID & nNIKAL100_kSubsystemVendorIDMask;
               pciBusInformation.deviceID.subSystemDevice = subSystemID >> nNIKAL100_kSubsystemDeviceIDShift;
            }
         }
         pciBusInformation.primary = bus->primary;
         
#ifdef nNIKAL240_kHasPciBusnRes
         pciBusInformation.secondary = bus->busn_res.start;
         pciBusInformation.subordinate = bus->busn_res.end;
#else
         pciBusInformation.secondary = bus->secondary;
         pciBusInformation.subordinate = bus->subordinate;
#endif
         pciBusInformation.domain = pci_domain_nr(bus);
         (*visitor)(visitorArgument, &pciBusInformation);
      }

      
      nNIKAL100_enumeratePCIBuses(visitor, visitorArgument, &(bus->children));
   }
#endif
}


nNIKAL100_cc nNIKAL100_tI32 nNIKAL240_getPCIDomain(const void * osDevObject)
{
#ifdef CONFIG_PCI_DOMAINS
   return pci_proc_domain( ((nLinux_pciDevice *)osDevObject)->bus );
#else
   return -1;
#endif 
}


#if nNIKAL100_mUSBIsSupported
static inline nNIKAL100_tUSBDeviceIDTableFooter *nNIKAL100_getUSBDeviceIDTableFooter(
   const nLinux_usbDeviceID *entry)
{
   nNIKAL100_tUSBDeviceIDTableFooter *tableFooter;
   nNIKAL100_tU32 i = 0;

   
   for (; (entry->match_flags != 0) ||
          (entry->idVendor != 0) || (entry->idProduct != 0) || (entry->bcdDevice_lo != 0) || (entry->bcdDevice_hi != 0) ||
          (entry->bDeviceClass != 0) || (entry->bDeviceSubClass != 0) || (entry->bDeviceProtocol != 0) ||
          (entry->bInterfaceClass != 0) || (entry->bInterfaceSubClass != 0) || (entry->bInterfaceProtocol != 0);
          ++entry)
   {}

   
   ++i;
   ++entry;
   tableFooter = (nNIKAL100_tUSBDeviceIDTableFooter*)entry;
   while (tableFooter->signature != nNIKAL100_kUSBDeviceIDTableFooterSignature)
   {
      ++i;
      ++entry;
      tableFooter = (nNIKAL100_tUSBDeviceIDTableFooter*)entry;
   }
   
   return tableFooter;
}


static int nNIKAL100_usbProbe(nLinux_usbInterface *interface, const nLinux_usbDeviceID *id)
{
   int status;
   nLinux_usbDevice  *usb_dev;
   nNIKAL200_tDevice *device;

   nNIKAL100_tUSBDeviceIDTableFooter* footer = nNIKAL100_getUSBDeviceIDTableFooter(id);

   
   device = nNIKAL200_constructDevice(footer->driver);

   if (unlikely(!device)) {
      status = -ENOMEM;
      goto out;
   }

   usb_dev = interface_to_usbdev(interface);

   
   device->deviceInfo.osDevice = (void*) usb_dev;
   device->deviceInfo.busType = nNIKAL100_kBusTypeUSB;
   device->deviceInfo.version = 1;
   device->deviceInfo.usb.deviceID.vendor         = usb_dev->descriptor.idVendor;
   device->deviceInfo.usb.deviceID.product        = usb_dev->descriptor.idProduct;
   device->deviceInfo.usb.deviceID.revision       = usb_dev->descriptor.bcdDevice;
   device->deviceInfo.usb.deviceID.deviceClass    = usb_dev->descriptor.bDeviceClass;
   device->deviceInfo.usb.deviceID.deviceSubClass = usb_dev->descriptor.bDeviceSubClass;
   device->deviceInfo.usb.deviceID.deviceProtocol = usb_dev->descriptor.bDeviceProtocol;
#ifdef nNIKAL100_kCurrentAlternateSetting
   device->deviceInfo.usb.deviceID.interfaceClass    = interface->cur_altsetting->desc.bInterfaceClass;
   device->deviceInfo.usb.deviceID.interfaceSubClass = interface->cur_altsetting->desc.bInterfaceSubClass;
   device->deviceInfo.usb.deviceID.interfaceProtocol = interface->cur_altsetting->desc.bInterfaceProtocol;
   device->deviceInfo.usb.interfaceNumber            = interface->cur_altsetting->desc.bInterfaceNumber;
#else
   device->deviceInfo.usb.deviceID.interfaceClass    = interface->altsetting[interface->act_altsetting].desc.bInterfaceClass;
   device->deviceInfo.usb.deviceID.interfaceSubClass = interface->altsetting[interface->act_altsetting].desc.bInterfaceSubClass;
   device->deviceInfo.usb.deviceID.interfaceProtocol = interface->altsetting[interface->act_altsetting].desc.bInterfaceProtocol;
   device->deviceInfo.usb.interfaceNumber            = interface->altsetting[interface->act_altsetting].desc.bInterfaceNumber;
#endif
   device->deviceInfo.usb.parentDevice  = usb_dev;
   device->deviceInfo.usb.deviceAddress = usb_dev->devnum;

   switch (usb_dev->speed) {
      case USB_SPEED_LOW:
         device->deviceInfo.usb.connectionSpeed = nNIKAL120_kUSBSignallingSpeedLow;
         break;
      case USB_SPEED_FULL:
         device->deviceInfo.usb.connectionSpeed = nNIKAL120_kUSBSignallingSpeedFull;
         break;
      case USB_SPEED_HIGH:
         device->deviceInfo.usb.connectionSpeed = nNIKAL120_kUSBSignallingSpeedHigh;
         break;
      case USB_SPEED_UNKNOWN:
      default:
         device->deviceInfo.usb.connectionSpeed = nNIKAL120_kUSBSignallingSpeedUnknown;
         break;
   }

   snprintf(device->nameString, sizeof(device->nameString), "%s", usb_dev->dev.bus_id);

   status = nNIKAL200_createDeviceProcDirs(device->nameString, device);

   if (nNIKAL100_statusIsFatal(status))
      goto out_destruct;

   
   device->deviceExtension = device;

   KAL_ASSERT(device->driver->addDevice);

   status = device->driver->addDevice(&device->deviceInfo,
                                      id->driver_info,
                                      &device->deviceExtension);


   if (nNIKAL100_statusIsFatal(status))
      goto out_remove_proc;

   device->addDeviceSuccess = 1;
   usb_set_intfdata(interface, device);
   usb_get_dev(usb_dev);

#ifdef nNIKAL100_kUSBReferenceCountFunctions
      /** Supposedly we are supposed to call this in order to refcount the device,
       * but no existing drivers do, and the symbol isn't even exported from the
       * kernel for modules to use...
      usb_get_intf(interface);
       **/
#endif

   return status;

out_remove_proc:
   nNIKAL200_destroyDeviceProcDirs(device);
out_destruct:
   nNIKAL200_destructDevice(&(device->kref));
out:
   return -ENODEV;
}


static void nNIKAL100_usbDisconnect(nLinux_usbInterface *interface)
{
   nNIKAL200_tDevice *d = usb_get_intfdata(interface);

   KAL_ASSERT(d->driver->removeDevice);

   d->driver->removeDevice(d->deviceExtension);

   nNIKAL200_destroyDeviceProcDirs(d);

   usb_put_dev(interface_to_usbdev(interface));
#ifdef nNIKAL100_kUSBReferenceCountFunctions
   /** Supposedly we are supposed to call this in order to refcount the device,
    * but no existing drivers do, and the symbol isn't even exported from the
    * kernel for modules to use...
   usb_put_intf(interface);
    **/
#endif
   usb_set_intfdata(interface, NULL);

   kref_put(&(d->kref), nNIKAL200_destructDevice);
}
#endif


nNIKAL100_cc void *nNIKAL100_registerUSBDriver(nNIKAL100_tDriver *driver,
   const nNIKAL100_tText *name, void *usbDevIdTable)
{
#if nNIKAL100_mUSBIsSupported
   nNIKAL100_tUSBDeviceIDTableFooter* tableFooter =
      (nNIKAL100_tUSBDeviceIDTableFooter*)usbDevIdTable;
   nLinux_usbDriver *usbDriver;
   int ret;

   tableFooter->driver = driver;

   usbDriver = nNIKAL100_malloc(sizeof(nLinux_usbDriver));
   if (usbDriver == NULL)
      return NULL;

   memset(usbDriver, 0, sizeof(nLinux_usbDriver));

   #ifdef nNIKAL130_kUSBOwnerMember
      usbDriver->owner = THIS_MODULE;
   #endif
   usbDriver->name = (char*)name;
   usbDriver->probe = nNIKAL100_usbProbe;
   usbDriver->disconnect = nNIKAL100_usbDisconnect;
   usbDriver->ioctl = NULL;
   usbDriver->suspend = NULL;
   usbDriver->resume = NULL;
   usbDriver->id_table = tableFooter->table;
   ret = usb_register(usbDriver);
   if (ret < 0)
   {
      nNIKAL100_free(usbDriver);
      return NULL;
   }

   return usbDriver;
#else
   return NULL;
#endif
}


nNIKAL100_cc void nNIKAL100_unregisterUSBDriver(void *usbDriverData, const nNIKAL100_tText ** name,
   void **usbDevIdTable)
{
#if nNIKAL100_mUSBIsSupported
   nLinux_usbDriver *usbDriver = (nLinux_usbDriver*)usbDriverData;
   nNIKAL100_tUSBDeviceIDTableFooter* tableFooter =
      nNIKAL100_getUSBDeviceIDTableFooter(usbDriver->id_table);
   *name = usbDriver->name;
   *usbDevIdTable = (void*)(tableFooter);
   usb_deregister(usbDriver);
   nNIKAL100_free(usbDriver);
#else
   
   KAL_ASSERT(!usbDriverData, "How did you get a non-NULL usb driver handle? "
                              "KAL doesn't support USB drivers with this kernel!\n");
#endif
}


nNIKAL100_cc void *nNIKAL100_createUSBDeviceIDTable(nNIKAL100_tU32 size)
{
#if nNIKAL100_mUSBIsSupported
   nNIKAL100_tText *buffer;
   nNIKAL100_tUSBDeviceIDTableFooter *usbDeviceIDTableFooter;

   
   nNIKAL100_tU32 bufferSize = ((size+1)*sizeof(nLinux_usbDeviceID)) + sizeof(nNIKAL100_tUSBDeviceIDTableFooter);
   buffer = (nNIKAL100_tText*)nNIKAL100_malloc(bufferSize);
   if (buffer == NULL)
      return NULL;

   memset(buffer, 0, bufferSize);

   
   usbDeviceIDTableFooter = (nNIKAL100_tUSBDeviceIDTableFooter*)
      (buffer + ((size+1)*sizeof(nLinux_usbDeviceID)));
   usbDeviceIDTableFooter->signature = nNIKAL100_kUSBDeviceIDTableFooterSignature;
   usbDeviceIDTableFooter->capacity = size;
   usbDeviceIDTableFooter->size = 0;
   usbDeviceIDTableFooter->driver = 0;
   usbDeviceIDTableFooter->table = (nLinux_usbDeviceID*)buffer;
   return (void*)usbDeviceIDTableFooter;
#else
   return NULL;
#endif
}


nNIKAL100_cc void nNIKAL100_setUSBDeviceIDTableElement(void *usbDevIdTable,
   nNIKAL100_tU32 vendor, nNIKAL100_tU32 product,
   void *privateData)
{
#if nNIKAL100_mUSBIsSupported
   nNIKAL100_tUSBDeviceIDTableFooter* tableFooter;
   nLinux_usbDeviceID *idTable;
   nNIKAL100_tU32 index;

   KAL_ASSERT(usbDevIdTable != NULL, "usbDevIdTable must NOT be NULL!\n");

   tableFooter = (nNIKAL100_tUSBDeviceIDTableFooter*)usbDevIdTable;
   idTable = tableFooter->table;
   index = tableFooter->size;

   ++(tableFooter->size);
   KAL_ASSERT(tableFooter->size <= tableFooter->capacity, "nNIKAL100_setUSBDeviceIDTableElement: usbDeviceIDTable size exceeds capacity\n");

   if( vendor != 0xFFFF ) {
      idTable[index].idVendor = vendor;
      idTable[index].match_flags |= USB_DEVICE_ID_MATCH_VENDOR;
   }
   if( product != 0xFFFF ) {
      idTable[index].idProduct = product;
      idTable[index].match_flags |= USB_DEVICE_ID_MATCH_PRODUCT;
   }



   nNIKAL100_compileTimeAssert(sizeof(void*)==sizeof(unsigned long), "Oops, resizing void* into unsigned long!");
   idTable[index].driver_info = (unsigned long)privateData;
#else
   
#endif
}


nNIKAL100_cc void nNIKAL100_destroyUSBDeviceIDTable(void *usbDevIdTable)
{
#if nNIKAL100_mUSBIsSupported
   nNIKAL100_tUSBDeviceIDTableFooter* tableFooter;
   if(!usbDevIdTable) return;
   tableFooter = (nNIKAL100_tUSBDeviceIDTableFooter*)usbDevIdTable;
   nNIKAL100_free(tableFooter->table);
#else
   
   KAL_ASSERT(usbDevIdTable == NULL, "How did you get a non-NULL usbDevIdTable?  We don't support it on 2.4 Kernels!\n");
#endif
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbSynchronousControlRequest(void* osDevice, nNIKAL100_tUSBSetupPacket* setupPacket, void* data)
{
#if nNIKAL100_mUSBIsSupported
   nNIKAL100_tStatus returnStatus;
   int pipe;
   if( ((setupPacket->bmRequestType) & USB_DIR_IN) == 0 )
   {
      pipe = usb_sndctrlpipe(osDevice, 0);
   }
   else
   {
      pipe = usb_rcvctrlpipe(osDevice, 0);
   }
   returnStatus = usb_control_msg( osDevice,
                                   pipe,
                                   setupPacket->bRequest,
                                   setupPacket->bmRequestType,
                                   setupPacket->wValue,
                                   setupPacket->wIndex,
                                   data,
                                   setupPacket->wLength,
                                   nNIKAL100_kUSBControlTimeout );

   
   if (returnStatus < 0)
   {
      KAL_DPRINT("usb_control_msg() returned %d\n", (int) returnStatus);
      return nNIKAL100_convertLinuxToKALStatus(returnStatus);
   }

   return nNIKAL100_kStatusSuccess;

#else
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetDeviceDescriptor(void* osDevice, void* buffer, nNIKAL100_tU32 size)
{
#if nNIKAL100_mUSBIsSupported
   nLinux_usbDevice* usbDevice = (nLinux_usbDevice*)osDevice;
   nNIKAL100_tU32 bytesToCopy = nNIKAL100_mMin(size, usbDevice->descriptor.bLength);

   memcpy(buffer, &(usbDevice->descriptor), bytesToCopy);
   return nNIKAL100_kStatusSuccess;
#else
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}

#if nNIKAL100_mUSBIsSupported
static nNIKAL100_tStatus nNIKAL100_usbFindConfigurationIndex(nLinux_usbDevice* usbDevice, nNIKAL100_tU8 configurationValue, nNIKAL100_tU8* configurationIndex)
{
   nNIKAL100_tU8 configIndex;

   for(configIndex = 0; configIndex < usbDevice->descriptor.bNumConfigurations; ++configIndex)
   {
      if( usbDevice->config[configIndex].desc.bConfigurationValue == configurationValue )
      {
         *configurationIndex = configIndex;
         return nNIKAL100_kStatusSuccess;
      }
   }
   return nNIKAL100_kStatusBadSelector;
}
#endif

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetConfigurationDescriptor(void* osDevice, nNIKAL100_tU8 configurationValue, void* buffer, nNIKAL100_tU32 size)
{
#if nNIKAL100_mUSBIsSupported
   nLinux_usbDevice* usbDevice = (nLinux_usbDevice*)osDevice;
   nNIKAL100_tU8 configIndex;
   nNIKAL100_tU32 bytesToCopy;

   nNIKAL100_tStatus kalStatus = nNIKAL100_usbFindConfigurationIndex(usbDevice, configurationValue, &configIndex);
   if( kalStatus != nNIKAL100_kStatusSuccess )
   {
      return kalStatus;
   }

   bytesToCopy = nNIKAL100_mMin(size, usbDevice->config[configIndex].desc.bLength);

   memcpy(buffer, &(usbDevice->config[configIndex].desc), bytesToCopy);
   return nNIKAL100_kStatusSuccess;
#else
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}

#ifdef nNIKAL100_kUSBInterfaceCacheMember
#define nNIKAL100_kUSBInterfaceMember intf_cache
#else
#define nNIKAL100_kUSBInterfaceMember interface
#endif

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetInterfaceDescriptor(void* osDevice, nNIKAL100_tU8 configurationValue,
   nNIKAL100_tU8 interfaceNumber, nNIKAL100_tU8 altSettingNumber, void* buffer, nNIKAL100_tU32 size)
{
#if nNIKAL100_mUSBIsSupported
   nLinux_usbDevice* usbDevice = (nLinux_usbDevice*)osDevice;
   nNIKAL100_tU8 configIndex;

   nNIKAL100_tStatus kalStatus = nNIKAL100_usbFindConfigurationIndex(usbDevice, configurationValue, &configIndex);
   if( kalStatus != nNIKAL100_kStatusSuccess )
   {
      return kalStatus;
   }

   if(
      (interfaceNumber < usbDevice->config[configIndex].desc.bNumInterfaces) &&
      (altSettingNumber < usbDevice->config[configIndex].nNIKAL100_kUSBInterfaceMember[interfaceNumber]->num_altsetting)
     )
   {
      nNIKAL100_tU32 bytesToCopy = nNIKAL100_mMin(size, usbDevice->config[configIndex].nNIKAL100_kUSBInterfaceMember[interfaceNumber]->altsetting[altSettingNumber].desc.bLength);

      memcpy(buffer, &(usbDevice->config[configIndex].nNIKAL100_kUSBInterfaceMember[interfaceNumber]->altsetting[altSettingNumber].desc), bytesToCopy);
      return nNIKAL100_kStatusSuccess;
   }
   return nNIKAL100_kStatusBadSelector;
#else
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetEndpointDescriptor(void* osDevice, nNIKAL100_tU8 configurationValue,
   nNIKAL100_tU8 interfaceNumber, nNIKAL100_tU8 altSettingNumber, nNIKAL100_tU8 endpointIndex, void* buffer, nNIKAL100_tU32 size)
{
#if nNIKAL100_mUSBIsSupported
   nLinux_usbDevice* usbDevice = (nLinux_usbDevice*)osDevice;
   nNIKAL100_tU8 configIndex;

   nNIKAL100_tStatus kalStatus = nNIKAL100_usbFindConfigurationIndex(usbDevice, configurationValue, &configIndex);
   if( kalStatus != nNIKAL100_kStatusSuccess )
   {
      return kalStatus;
   }

   if(
      (interfaceNumber < usbDevice->config[configIndex].desc.bNumInterfaces) &&
      (altSettingNumber < usbDevice->config[configIndex].nNIKAL100_kUSBInterfaceMember[interfaceNumber]->num_altsetting)
     )
   {
      nLinux_usbInterfaceDescriptorWrapper* hostInterface = &(usbDevice->config[configIndex].nNIKAL100_kUSBInterfaceMember[interfaceNumber]->altsetting[altSettingNumber]);
      if( endpointIndex < hostInterface->desc.bNumEndpoints )
      {
         nNIKAL100_tU32 bytesToCopy = nNIKAL100_mMin(size, hostInterface->endpoint[endpointIndex].desc.bLength);

         memcpy(buffer, &(hostInterface->endpoint[endpointIndex].desc), bytesToCopy);
         return nNIKAL100_kStatusSuccess;
      }
   }
   return nNIKAL100_kStatusBadSelector;
#else
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbSetConfiguration(void* osDevice, nNIKAL100_tU8 configurationValue)
{
#if nNIKAL100_mUSBIsSupported && defined(nNIKAL100_kUSBSetConfiguration)
   int linuxStatus = usb_set_configuration( osDevice, configurationValue );

   if (linuxStatus != 0)
      KAL_DPRINT("usb_set_configuration returned %d\n", linuxStatus);

   return nNIKAL100_convertLinuxToKALStatus(linuxStatus);
#else
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetConfiguration(void* osDevice, nNIKAL100_tU8* configurationValuePtr)
{
#if nNIKAL100_mUSBIsSupported
   int returnStatus;
   KAL_ASSERT(configurationValuePtr != NULL, "Invalid pointer parameter.\n");
   returnStatus = usb_control_msg(  osDevice,
                                    usb_rcvctrlpipe(osDevice, 0),
                                    USB_REQ_GET_CONFIGURATION,
                                    USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE,
                                    0,
                                    0,
                                    configurationValuePtr,
                                    1,
                                    nNIKAL100_kUSBControlTimeout
                                 );
   if(returnStatus < 0)
   {
      KAL_DPRINT("usb_get_configuration() returned %d\n", (int)returnStatus);
      return nNIKAL100_convertLinuxToKALStatus(returnStatus);
   }
   return nNIKAL100_kStatusSuccess;
#else
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbSetInterface(void* osDevice, nNIKAL100_tU8 interfaceNumber, nNIKAL100_tU8 alternateSetting)
{
#if nNIKAL100_mUSBIsSupported
   int linuxStatus = usb_set_interface( osDevice, interfaceNumber, alternateSetting );
   if( linuxStatus != 0 )
   {
      KAL_DPRINT("usb_set_interface returned %d\n", linuxStatus);
   }
   return nNIKAL100_convertLinuxToKALStatus(linuxStatus);
#else
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetInterface(void* osDevice, nNIKAL100_tU8 interfaceNumber, nNIKAL100_tU8* alternateSettingPtr)
{
#if nNIKAL100_mUSBIsSupported
   int returnStatus;
   KAL_ASSERT(alternateSettingPtr != NULL, "Invalid pointer parameter.\n");
   returnStatus = usb_control_msg(  osDevice,
                                    usb_rcvctrlpipe(osDevice, 0),
                                    USB_REQ_GET_INTERFACE,
                                    USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE,
                                    0,
                                    interfaceNumber,
                                    alternateSettingPtr,
                                    1,
                                    nNIKAL100_kUSBControlTimeout
                                 );
   if(returnStatus < 0)
   {
      KAL_DPRINT("usb_get_interface() returned %d\n", (int)returnStatus);
      return nNIKAL100_convertLinuxToKALStatus(returnStatus);
   }
   return nNIKAL100_kStatusSuccess;
#else
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}



nNIKAL100_cc nNIKAL100_tURB* nNIKAL100_usbAllocateURB(nNIKAL100_tU32 numISODescriptors)
{
#if nNIKAL100_mUSBIsSupported
   nNIKAL100_tURB* kalURB = (nNIKAL100_tURB*) nNIKAL100_malloc(sizeof(nNIKAL100_tURB) +
                                                               numISODescriptors*sizeof(nNIKAL100_tUSBIsochronousPacketDescriptor));
   if( kalURB != NULL )
   {
      kalURB->osURB = usb_alloc_urb(numISODescriptors, GFP_KERNEL);
      if( kalURB->osURB != NULL )
      {
         if( numISODescriptors > 0 )
         {
            kalURB->isochronous.packetArray = (nNIKAL100_tUSBIsochronousPacketDescriptor*)((void*)kalURB + sizeof(nNIKAL100_tURB));
         }
         else
         {
            kalURB->isochronous.packetArray = NULL;
         }

#ifndef nNIKAL160_kWorkqueueNonDelay
         kalURB->control.osWorkItem = nNIKAL100_malloc(sizeof(nNIKAL100_tWorkQueue));
#else
         kalURB->control.osWorkItem = nNIKAL100_malloc(sizeof(nNIKAL160_tWorkItemWithDataPointer));
#endif
         if( kalURB->control.osWorkItem != NULL )
         {
            /** KAL_DPRINT("--> nNIKAL100_usbAllocateURB: Allocated URB 0x%p\n", kalURB->osURB); **/
#ifdef nNIKAL160_kWorkqueueNonDelay
            nNIKAL160_tWorkItemWithDataPointer* tmp_ptr = (nNIKAL160_tWorkItemWithDataPointer*)kalURB->control.osWorkItem;
            tmp_ptr->data = (void *)kalURB;
#endif
            return kalURB; 
         }

         
         usb_free_urb((nLinux_urb*)kalURB->osURB);
      }
      nNIKAL100_free(kalURB);
   }
   return NULL;
#else
   return NULL;
#endif
}



nNIKAL100_cc void nNIKAL100_usbFreeURB(nNIKAL100_tURB* kalURB)
{
#if nNIKAL100_mUSBIsSupported
   if( kalURB == NULL ) return;
   /** KAL_DPRINT("<-- nNIKAL100_usbFreeURB: Freeing URB 0x%p\n", kalURB->osURB); **/
   kfree(kalURB->control.osWorkItem);
   usb_free_urb((nLinux_urb*)kalURB->osURB);
   nNIKAL100_free(kalURB);
#else
   
   KAL_ASSERT(kalURB == NULL, "How did you get a non-NULL kalURB?  We don't support it on 2.4 Kernels!\n");
#endif
}



#if nNIKAL100_mUSBIsSupported
#ifdef nNIKAL160_kIRQRegs
static void nNIKAL100_usbAsynchronousURBCallback(struct urb* osURB, struct pt_regs* unused)
#else
static void nNIKAL100_usbAsynchronousURBCallback(struct urb* osURB)
#endif
{
   nNIKAL100_tURB* kalURB;

   KAL_ASSERT(osURB != NULL, "Got a callback for a NULL osURB!\n");

   kalURB = (nNIKAL100_tURB*)osURB->context;

   KAL_ASSERT(kalURB != NULL, "Got a callback for a NULL kalURB!\n");
   KAL_ASSERT(kalURB->callback != NULL, "Got a callback for a NULL kalURB->callback!\n");

   kalURB->ioStatus = nNIKAL100_convertLinuxToKALStatus(osURB->status);
   kalURB->transferCount = osURB->actual_length;

   /** KAL_DPRINT("<----- nNIKAL100_usbAsynchronousURBCallback: URB 0x%p is complete\n", osURB); **/
   kalURB->callback(kalURB);
}

#ifdef nNIKAL160_kIRQRegs
static void nNIKAL100_usbIsochronousURBCallback(struct urb* osURB, struct pt_regs* unused)
#else
static void nNIKAL100_usbIsochronousURBCallback(struct urb* osURB)
#endif
{
   nNIKAL100_tURB* kalURB;
   nNIKAL100_tU32 packetIndex;

   KAL_ASSERT(osURB != NULL, "Got a callback for a NULL osURB!\n");

   kalURB = (nNIKAL100_tURB*)osURB->context;

   KAL_ASSERT(kalURB != NULL, "Got a callback for a NULL kalURB!\n");
   KAL_ASSERT(kalURB->callback != NULL, "Got a callback for a NULL kalURB->callback!\n");

   kalURB = (nNIKAL100_tURB*)osURB->context;
   kalURB->ioStatus = nNIKAL100_convertLinuxToKALStatus(osURB->status);
   kalURB->transferCount = osURB->actual_length;
   kalURB->isochronous.startFrame = osURB->start_frame;
   kalURB->isochronous.numberOfErrors = osURB->error_count;

   for ( packetIndex = 0; packetIndex < kalURB->isochronous.numberOfPackets; ++packetIndex) {
      kalURB->isochronous.packetArray[packetIndex].ioStatus = osURB->iso_frame_desc[packetIndex].status;
      kalURB->isochronous.packetArray[packetIndex].packetTransferCount = osURB->iso_frame_desc[packetIndex].actual_length;
   }
   /** KAL_DPRINT("<----- nNIKAL100_usbIsochronousURBCallback: URB 0x%p is complete\n", osURB); **/
   kalURB->callback(kalURB);
}

static void nNIKAL100_buildControlURB(nLinux_usbDevice* usbDevice, nNIKAL100_tURB* kalURB, nLinux_urb* osURB)
{
   unsigned int pipe;

   
   if( (kalURB->transferFlags & nNIKAL100_kUSBTransferFlagDirection) == 0)
   {
      pipe = usb_sndctrlpipe(usbDevice,kalURB->endpoint & USB_ENDPOINT_NUMBER_MASK);
   }
   else
   {
      pipe = usb_rcvctrlpipe(usbDevice,kalURB->endpoint & USB_ENDPOINT_NUMBER_MASK);
   }

   usb_fill_control_urb(
      osURB,
      usbDevice,
      pipe,
      (unsigned char*)&kalURB->control.setupPacket,
      kalURB->buffer,
      kalURB->bufferSize,
      nNIKAL100_usbAsynchronousURBCallback,
      kalURB
      );
}

static void nNIKAL100_buildBulkURB(nLinux_usbDevice* usbDevice, nNIKAL100_tURB* kalURB, nLinux_urb* osURB)
{
   unsigned int pipe;

   
   if( (kalURB->transferFlags & nNIKAL100_kUSBTransferFlagDirection) == 0)
   {
      pipe = usb_sndbulkpipe(usbDevice,kalURB->endpoint & USB_ENDPOINT_NUMBER_MASK);
   }
   else
   {
      pipe = usb_rcvbulkpipe(usbDevice,kalURB->endpoint & USB_ENDPOINT_NUMBER_MASK);
   }

   usb_fill_bulk_urb(
      osURB,
      usbDevice,
      pipe,
      kalURB->buffer,
      kalURB->bufferSize,
      nNIKAL100_usbAsynchronousURBCallback,
      kalURB
      );
}


nLinux_usbEndpointDescriptor* nNIKAL100_usbEndpointAddressToEndpointDescriptor(nLinux_usbDevice* usbDevice, nNIKAL100_tU32 endpoint)
{
   nLinux_usbEndpointDescriptor* endpointDesc;

   
#ifdef nNIKAL100_kUSBDeviceEndpointArray
   nLinux_usbEndpointDescriptorWrapper* endpointDescWrapper;

   if( (endpoint & USB_ENDPOINT_DIR_MASK) == 0)
   {
      endpointDescWrapper = usbDevice->ep_out[endpoint & USB_ENDPOINT_NUMBER_MASK];
      KAL_ASSERT( endpointDescWrapper != NULL, "No matching ep_out found on the specified interface.\n");
      endpointDesc = &(endpointDescWrapper->desc);
   }
   else
   {
      endpointDescWrapper = usbDevice->ep_in[endpoint & USB_ENDPOINT_NUMBER_MASK];
      KAL_ASSERT( endpointDescWrapper != NULL, "No matching ep_in found on the specified interface.\n");
      endpointDesc = &(endpointDescWrapper->desc);
   }
#else
   endpointDesc = usb_epnum_to_ep_desc(usbDevice, endpoint);
   KAL_ASSERT( endpointDesc != NULL, "No matching endpoint found on the specified interface.\n");
#endif
   return endpointDesc;
}

static void nNIKAL100_buildInterruptURB(nLinux_usbDevice* usbDevice, nNIKAL100_tURB* kalURB, nLinux_urb* osURB)
{
   unsigned int pipe;

   nLinux_usbEndpointDescriptor* endpointDesc;

   
   if( (kalURB->transferFlags & nNIKAL100_kUSBTransferFlagDirection) == 0)
   {
      pipe = usb_sndintpipe(usbDevice,kalURB->endpoint & USB_ENDPOINT_NUMBER_MASK);
   }
   else
   {
      pipe = usb_rcvintpipe(usbDevice,kalURB->endpoint & USB_ENDPOINT_NUMBER_MASK);
   }

   osURB->dev                    = usbDevice;
   osURB->pipe                   = pipe;
   osURB->transfer_buffer        = kalURB->buffer;
   osURB->transfer_buffer_length = kalURB->bufferSize;
   osURB->complete               = nNIKAL100_usbAsynchronousURBCallback;
   osURB->context                = kalURB;

   endpointDesc = nNIKAL100_usbEndpointAddressToEndpointDescriptor(usbDevice, kalURB->endpoint);

   KAL_ASSERT(
      USB_ENDPOINT_XFER_INT == ((endpointDesc->bmAttributes) & USB_ENDPOINT_XFERTYPE_MASK),
      "This is not an interrupt endpoint.\n");
   if( usbDevice->speed == USB_SPEED_HIGH )
   {
      osURB->interval = 1 << min(15, endpointDesc->bInterval - 1);
   }
   else
   {
      osURB->interval = endpointDesc->bInterval;
   }

   KAL_ASSERT( osURB->interval != 0, "Invalid interval for Interrupt transfer.\n");
}

static void nNIKAL100_buildIsochronousURB(nLinux_usbDevice* usbDevice, nNIKAL100_tURB* kalURB, nLinux_urb* osURB)
{
   unsigned int pipe;

   nLinux_usbEndpointDescriptor *endpointDesc;
   nNIKAL100_tU32 packetIndex;

   
   if( (kalURB->transferFlags & nNIKAL100_kUSBTransferFlagDirection) == 0)
   {
      pipe = usb_sndisocpipe(usbDevice,kalURB->endpoint & USB_ENDPOINT_NUMBER_MASK);
   }
   else
   {
      pipe = usb_rcvisocpipe(usbDevice,kalURB->endpoint & USB_ENDPOINT_NUMBER_MASK);
   }

   osURB->dev                    = usbDevice;
   osURB->pipe                   = pipe;
   osURB->transfer_buffer        = kalURB->buffer;
   osURB->transfer_buffer_length = kalURB->bufferSize;
   osURB->complete               = nNIKAL100_usbIsochronousURBCallback;
   osURB->context                = kalURB;

   endpointDesc = nNIKAL100_usbEndpointAddressToEndpointDescriptor(usbDevice, kalURB->endpoint);

   KAL_ASSERT(
      USB_ENDPOINT_XFER_ISOC == ((endpointDesc->bmAttributes) & USB_ENDPOINT_XFERTYPE_MASK),
      "This is not an isochronous endpoint.\n");
   if( usbDevice->speed == USB_SPEED_HIGH )
   {
      osURB->interval = 1 << min(15, endpointDesc->bInterval - 1);
   }
   else
   {
      osURB->interval = endpointDesc->bInterval;
   }

   KAL_ASSERT( osURB->interval != 0, "Invalid interval for Isochronous transfer.\n");

   
   if( kalURB->transferFlags & nNIKAL100_kUSBTransferFlagStartISOTransferASAP )
   {
      osURB->transfer_flags = URB_ISO_ASAP;
   }
   else
   {
      osURB->start_frame = kalURB->isochronous.startFrame;
   }

   osURB->number_of_packets = kalURB->isochronous.numberOfPackets;

   for ( packetIndex = 0; packetIndex < kalURB->isochronous.numberOfPackets; ++packetIndex) {
      osURB->iso_frame_desc[packetIndex].offset = kalURB->isochronous.packetArray[packetIndex].packetOffset;
      osURB->iso_frame_desc[packetIndex].length = kalURB->isochronous.packetArray[packetIndex].packetLength;
   }
}

static void nNIKAL100_usbClearHaltSynchronous(void *params)
{
#ifndef nNIKAL160_kWorkqueueNonDelay
   nNIKAL100_tURB *kalURB = (nNIKAL100_tURB*)params;
#else
   nNIKAL160_tWorkItemWithDataPointer* tmp_ptr = (nNIKAL160_tWorkItemWithDataPointer*)params;
   nNIKAL100_tURB *kalURB = (nNIKAL100_tURB *)tmp_ptr->data;
#endif
   nLinux_usbDevice* usbDevice = (nLinux_usbDevice*)kalURB->osDeviceObject;
   nNIKAL100_tU32 endpointToClear;
   nLinux_usbEndpointDescriptor* endpointDesc;
   int pipe = 0;
   int status;

   endpointToClear = kalURB->control.setupPacket.wIndex;
   endpointDesc = nNIKAL100_usbEndpointAddressToEndpointDescriptor(usbDevice, endpointToClear);

   switch ((endpointDesc->bmAttributes) & USB_ENDPOINT_XFERTYPE_MASK)
   {
      case USB_ENDPOINT_XFER_CONTROL:
         if( (endpointToClear & USB_ENDPOINT_DIR_MASK) == 0)
         {
            pipe = usb_sndctrlpipe(usbDevice,endpointToClear & USB_ENDPOINT_NUMBER_MASK);
         }
         else
         {
            pipe = usb_rcvctrlpipe(usbDevice,endpointToClear & USB_ENDPOINT_NUMBER_MASK);
         }
         break;
      case USB_ENDPOINT_XFER_BULK:
         if( (endpointToClear & USB_ENDPOINT_DIR_MASK) == 0)
         {
            pipe = usb_sndbulkpipe(usbDevice,endpointToClear & USB_ENDPOINT_NUMBER_MASK);
         }
         else
         {
            pipe = usb_rcvbulkpipe(usbDevice,endpointToClear & USB_ENDPOINT_NUMBER_MASK);
         }
         break;
      case USB_ENDPOINT_XFER_ISOC:
         if( (endpointToClear & USB_ENDPOINT_DIR_MASK) == 0)
         {
            pipe = usb_sndisocpipe(usbDevice,endpointToClear & USB_ENDPOINT_NUMBER_MASK);
         }
         else
         {
            pipe = usb_rcvisocpipe(usbDevice,endpointToClear & USB_ENDPOINT_NUMBER_MASK);
         }
         break;
      case USB_ENDPOINT_XFER_INT:
         if( (endpointToClear & USB_ENDPOINT_DIR_MASK) == 0)
         {
            pipe = usb_sndintpipe(usbDevice,endpointToClear & USB_ENDPOINT_NUMBER_MASK);
         }
         else
         {
            pipe = usb_rcvintpipe(usbDevice,endpointToClear & USB_ENDPOINT_NUMBER_MASK);
         }
         break;
   }

   status = usb_clear_halt (usbDevice, pipe);

   kalURB->ioStatus = nNIKAL100_convertLinuxToKALStatus(status);
   kalURB->transferCount = 0;
   kalURB->callback(kalURB);
}

static nNIKAL100_tStatus nNIKAL100_usbClearHaltAsync(nNIKAL100_tURB* kalURB)
{
   int isScheduleSuccess;

#ifndef nNIKAL160_kWorkqueueNonDelay
   nNIKAL100_mInitWorkQueue((nNIKAL100_tWorkQueue*)(kalURB->control.osWorkItem),
      nNIKAL100_usbClearHaltSynchronous, kalURB);
#else
   nNIKAL100_mInitWorkQueue((nNIKAL100_tWorkQueue*)(kalURB->control.osWorkItem),
      (tFuncWithWorkQueuePtrParam)&nNIKAL100_usbClearHaltSynchronous);
#endif
   isScheduleSuccess = nNIKAL100_mScheduleWorkQueue(
      (nNIKAL100_tWorkQueue*)(kalURB->control.osWorkItem));

   
   KAL_ASSERT(isScheduleSuccess, "Failed to schedule a work item that's just initialized\n");

   return nNIKAL100_kStatusSuccess;
}
#endif


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbSubmitURB(nNIKAL100_tURB* kalURB)
{
#if nNIKAL100_mUSBIsSupported
   nLinux_usbDevice* usbDevice;
   nLinux_urb* osURB;
   int returnStatus;

   KAL_ASSERT(kalURB != NULL, "Trying to submit a NULL kalURB!\n");
   KAL_ASSERT(kalURB->osURB != NULL, "Trying to submit a NULL osURB!\n");
   KAL_ASSERT(kalURB->osDeviceObject != NULL, "Trying to submit a URB with a NULL osDeviceObject!\n");
   KAL_ASSERT(kalURB->callback != NULL, "Trying to submit a URB with a NULL callback!\n");

   usbDevice = (nLinux_usbDevice*)kalURB->osDeviceObject;
   osURB = (nLinux_urb*)kalURB->osURB;

   switch(kalURB->transferType)
   {
      case nNIKAL100_kUSBTransferTypeControl:
      {
         if((kalURB->control.setupPacket.bRequest == USB_REQ_CLEAR_FEATURE) &&
            (kalURB->control.setupPacket.bmRequestType == USB_RECIP_ENDPOINT) &&
            (kalURB->control.setupPacket.wValue == 0 /** ENDPOINT_HALT **/))
         {
            
            
            return nNIKAL100_usbClearHaltAsync(kalURB);
         }
         else
         {
            nNIKAL100_buildControlURB(usbDevice, kalURB, osURB);
         }
         break;
      }
      case nNIKAL100_kUSBTransferTypeBulk:
      {
         nNIKAL100_buildBulkURB(usbDevice, kalURB, osURB);
         break;
      }
      case nNIKAL100_kUSBTransferTypeInterrupt:
      {
         nNIKAL100_buildInterruptURB(usbDevice, kalURB, osURB);
         break;
      }
      case nNIKAL100_kUSBTransferTypeIsochronous:
      {
         nNIKAL100_buildIsochronousURB(usbDevice, kalURB, osURB);
         break;
      }
      default:
         KAL_ASSERT(0, "Invalid USB transfer type\n");
         break;
   }

   /** KAL_DPRINT("-----> nNIKAL100_usbSubmitURB: Submitted URB 0x%p\n", osURB); **/
   #ifdef nNIKAL130_kURBAsyncUnlink
      osURB->transfer_flags |= URB_ASYNC_UNLINK;
   #endif
   osURB->status = 0;
   /** KAL_DPRINT("usb_submit_urb(\n   pipe = 0x%8.8x\n   buffer = 0x%p\n   buffer_size = %d\n   interval = %d\n)\n", osURB->pipe, osURB->transfer_buffer, osURB->transfer_buffer_length, osURB->interval); **/
   returnStatus = usb_submit_urb(osURB, GFP_ATOMIC);

   
   if(returnStatus != 0)
   {
      KAL_DPRINT("usb_submit_urb() returned %d\n", returnStatus);
      kalURB->ioStatus = nNIKAL100_convertLinuxToKALStatus(returnStatus);
      kalURB->transferCount = 0;
      kalURB->callback(kalURB);
   }

   return nNIKAL100_convertLinuxToKALStatus(returnStatus);
#else
   
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbUnlinkURB(nNIKAL100_tURB* kalURB)
{
#if nNIKAL100_mUSBIsSupported
   int unlinkStatus;

   KAL_ASSERT(kalURB != NULL, "Trying to unlink a NULL kal URB!\n");
   KAL_ASSERT(kalURB->osURB != NULL, "Trying to unlink a NULL OS URB!\n");
   #ifdef nNIKAL130_kURBAsyncUnlink
      KAL_ASSERT( ((nLinux_urb*)kalURB->osURB)->transfer_flags & URB_ASYNC_UNLINK, "The URB is going to be unlinked synchronously?\n");
   #endif

   /** KAL_DPRINT("++++++ nNIKAL100_usbUnlinkURB: URB 0x%p is being unlinked\n", kalURB->osURB); **/

   unlinkStatus = usb_unlink_urb((nLinux_urb*)kalURB->osURB);
   if( unlinkStatus != -EINPROGRESS )
   {
      KAL_DPRINT("nNIKAL100_usbUnlinkURB(): usb_unlink_urb(%p) returned %d\n", (nLinux_urb*)kalURB->osURB, unlinkStatus);
   }
   
   return nNIKAL100_kStatusSuccess;
#else
   
   return nNIKAL100_kStatusFeatureNotSupported;
#endif
}


nNIKAL100_cc int nNIKAL1500_helperACPIProbe(void *dev, nNIKAL100_tDriver *driver)
{
#if nNIKAL1500_mACPIIsSupported
   int               status = 0;
   nLinux_acpiDevice *acpiDevice = (nLinux_acpiDevice*)dev;
   nNIKAL200_tDevice *device = NULL;

   KAL_ASSERT(driver);

   device = nNIKAL200_constructDevice(driver);
   if (unlikely(!device)) {
      status = -ENOMEM;
      goto acpi_probe_out;
   }

   device->deviceInfo.osDevice = (void *) dev;
   device->deviceInfo.busType = nNIKAL1500_kBusTypeACPI;
   device->deviceInfo.version = 1;
   strncpy(device->deviceInfo.acpi.deviceID.id, acpiDevice->pnp.bus_id, sizeof(acpiDevice->pnp.bus_id));

   snprintf(device->nameString, min(sizeof(device->nameString), sizeof(acpiDevice->pnp.bus_id)), "acpi:%s", acpiDevice->pnp.bus_id);

   status = nNIKAL200_createDeviceProcDirs(device->nameString, device);

   if (nNIKAL100_statusIsFatal(status))
      goto acpi_probe_free_device;

   
   device->deviceExtension = device;

   KAL_ASSERT(device->driver->addDevice);

   status = device->driver->addDevice(&device->deviceInfo,
                                      (void*)acpi_driver_data(acpiDevice),
                                      &device->deviceExtension);

   if (nNIKAL100_statusIsFatal(status))
      goto acpi_probe_remove_proc;

   device->addDeviceSuccess = 1;
   acpiDevice->driver_data = device;

   
   return 0;

acpi_probe_remove_proc:
   KAL_DPRINT("Error: %d. Add device failed. Removing proc entries for '%s'\n",
      status, device->nameString);
   nNIKAL200_destroyDeviceProcDirs(device);
acpi_probe_free_device:
   KAL_DPRINT("Error: %d. Destroying device (device=%p)\n", status, device);
   nNIKAL200_destructDevice(&(device->kref));
acpi_probe_out:
#endif 
   return -ENODEV;
}


#if nNIKAL1500_mACPIIsSupported
#ifdef nNIKAL1500_kACPIRemoveHasTypeInput
static int nNIKAL1500_acpiRemove(nLinux_acpiDevice *dev, int type)
#else
static int nNIKAL1500_acpiRemove(nLinux_acpiDevice *dev)
#endif
{
   nNIKAL200_tDevice *d = acpi_driver_data(dev);
   dev->driver_data = NULL;

   KAL_ASSERT(d->driver->removeDevice);
   d->driver->removeDevice(d->deviceExtension);

   nNIKAL200_destroyDeviceProcDirs(d);

   kref_put(&(d->kref), nNIKAL200_destructDevice);

   return 0;
}
#endif 


nNIKAL100_cc void *nNIKAL1500_registerACPIDriver(void* inAcpiDriver, void *acpiProbe)
{
#if nNIKAL1500_mACPIIsSupported
   int ret;
   nLinux_acpiDriver *acpiDriver = (nLinux_acpiDriver*)inAcpiDriver;

   acpiDriver->ops.add = acpiProbe;
   acpiDriver->ops.remove = nNIKAL1500_acpiRemove;
   ret = acpi_bus_register_driver(acpiDriver);
   if ( ret < 0 )
      return NULL;

   return acpiDriver;
#else 
   return NULL;
#endif 
}


nNIKAL100_cc void nNIKAL1500_unregisterACPIDriver(void *acpiDriverData)
{
#if nNIKAL1500_mACPIIsSupported
   nLinux_acpiDriver *acpiDriver = (nLinux_acpiDriver*)acpiDriverData;
   acpi_bus_unregister_driver(acpiDriver);
#endif 
}


nNIKAL100_cc int nNIKAL1500_callACPIMethodGetInteger(const void *osDevObject,
   const char *method, nNIKAL100_tU64 *value)
{
#if nNIKAL1500_mACPIIsSupported
   acpi_status result;
   nLinux_acpiDevice *acpiDev = (nLinux_acpiDevice *)osDevObject;

   result = acpi_evaluate_integer(acpiDev->handle, (acpi_string)method, NULL, (unsigned long long *)value);
   if (ACPI_FAILURE(result))
   {
      return nNIKAL100_kStatusValueConflict;
   }
   return 0;
#else 
   return nNIKAL100_kStatusFeatureNotSupported;
#endif 
}


nNIKAL100_cc int nNIKAL1500_callACPIMethod(const void *osDevObject,
   const char *method, nNIKAL100_tU8 *outbuf, nNIKAL100_tU32 size)
{
#if nNIKAL1500_mACPIIsSupported
   acpi_status result;
   struct acpi_buffer method_result;
   nLinux_acpiDevice *acpiDev = (nLinux_acpiDevice *)osDevObject;

   method_result.length = size;
   method_result.pointer = outbuf;

   result = acpi_evaluate_object(acpiDev->handle, (acpi_string)method, NULL, &method_result);
   if (ACPI_FAILURE(result))
   {
      return nNIKAL100_kStatusValueConflict;
   }
   return 0;
#else 
   return nNIKAL100_kStatusFeatureNotSupported;
#endif 
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigWrite8 (
   const void * osDevObject,
   const nNIKAL100_tU32 address,
   const nNIKAL100_tU8 data)
{
   if (pci_write_config_byte((nLinux_pciDevice *)osDevObject, address, data))
      return nNIKAL100_kStatusOSFault;
   else
      return nNIKAL100_kStatusSuccess;
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigWrite16 (
   const void * osDevObject,
   const nNIKAL100_tU32 address,
   const nNIKAL100_tU16 data)
{
   if (pci_write_config_word((nLinux_pciDevice *)osDevObject, address, data))
      return nNIKAL100_kStatusOSFault;
   else
      return nNIKAL100_kStatusSuccess;
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigWrite32 (
   const void * osDevObject,
   const nNIKAL100_tU32 address,
   const nNIKAL100_tU32 data)
{
   if (pci_write_config_dword((nLinux_pciDevice *)osDevObject, address, data))
      return nNIKAL100_kStatusOSFault;
   else
      return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigRead8 (
   const void * osDevObject,
   const nNIKAL100_tU32 address,
   nNIKAL100_tU8 *data)
{
   if (pci_read_config_byte((nLinux_pciDevice *)osDevObject, address, data))
      return nNIKAL100_kStatusOSFault;
   else
      return nNIKAL100_kStatusSuccess;
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigRead16 (
   const void * osDevObject,
   const nNIKAL100_tU32 address,
   nNIKAL100_tU16 *data)
{
   if (pci_read_config_word((nLinux_pciDevice *)osDevObject, address, data))
      return nNIKAL100_kStatusOSFault;
   else
      return nNIKAL100_kStatusSuccess;
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigRead32 (
   const void * osDevObject,
   const nNIKAL100_tU32 address,
   nNIKAL100_tU32 *data)
{
   if (pci_read_config_dword((nLinux_pciDevice *)osDevObject, address, (u32*)data))
      return nNIKAL100_kStatusOSFault;
   else
      return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc void nNIKAL100_ioWrite8 (
   const nNIKAL100_tUPtr address,
   const nNIKAL100_tU8 data)
{
   outb(data, address);
}

nNIKAL100_cc void nNIKAL100_ioWrite16 (
   const nNIKAL100_tUPtr address,
   const nNIKAL100_tU16 data)
{
   outw(data, address);
}

nNIKAL100_cc void nNIKAL100_ioWrite32 (
   const nNIKAL100_tUPtr address,
   const nNIKAL100_tU32 data)
{
   outl(data, address);
}


nNIKAL100_cc nNIKAL100_tU8 nNIKAL100_ioRead8 (
   const nNIKAL100_tUPtr address)
{
   return inb(address);
}

nNIKAL100_cc nNIKAL100_tU16 nNIKAL100_ioRead16 (
   const nNIKAL100_tUPtr address)
{
   return inw(address);
}

nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_ioRead32 (
   const nNIKAL100_tUPtr address)
{
   return inl(address);
}



nNIKAL100_cc void nNIKAL100_memoryWrite8 (
   const nNIKAL100_tUPtr address,
   const nNIKAL100_tU8 data)
{
   writeb(data, (void __iomem *)address);
}

nNIKAL100_cc void nNIKAL100_memoryWrite16 (
   const nNIKAL100_tUPtr address,
   const nNIKAL100_tU16 data)
{
   writew(data, (void __iomem *)address);
}

nNIKAL100_cc void nNIKAL100_memoryWrite32 (
   const nNIKAL100_tUPtr address,
   const nNIKAL100_tU32 data)
{
   writel(data, (void __iomem *)address);
}


nNIKAL100_cc nNIKAL100_tU8 nNIKAL100_memoryRead8 (
   const nNIKAL100_tUPtr address)
{
   return readb((void __iomem *)address);
}

nNIKAL100_cc nNIKAL100_tU16 nNIKAL100_memoryRead16 (
   const nNIKAL100_tUPtr address)
{
   return readw((void __iomem *)address);
}

nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_memoryRead32 (
   const nNIKAL100_tUPtr address)
{
   return readl((void __iomem *)address);
}


#ifdef nNIKAL160_kIRQRegs
static irqreturn_t nNIKAL100_dispatchHardwareInterrupt(int irq, void *context,
   nLinux_registers * registers)
#else
static irqreturn_t nNIKAL100_dispatchHardwareInterrupt(int irq, void *context)
#endif
{
   nNIKAL100_tBoolean handled;
   unsigned long flags;

   nNIKAL210_tInternalInterruptSpec *internal_spec = context;
   nNIKAL100_tInterruptCallbackSpec *spec = internal_spec->spec;

   spin_lock_irqsave(&internal_spec->lock, flags);
   handled = spec->func(irq, spec->context);
   spin_unlock_irqrestore(&internal_spec->lock, flags);

   return handled ? IRQ_HANDLED : IRQ_NONE;
}


static void nNIKAL100_dispatchDPC(unsigned long interruptData)
{
   nNIKAL100_tDPCCallbackSpec *dpcCallback =
      (nNIKAL100_tDPCCallbackSpec *)interruptData;
   dpcCallback->func(dpcCallback->context);
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL230_pciRequestMSI(nNIKAL100_tDeviceInfo *devInfo, nNIKAL100_tU32 numMessages)
{
   nLinux_pciDevice* pciDevice = devInfo->osDevice;
   KAL_ASSERT(devInfo->busType == nNIKAL100_kBusTypePCI);
   
   if (numMessages != 1)
      return nNIKAL100_kStatusFeatureNotSupported;
   if (pci_enable_msi(pciDevice))
      return nNIKAL100_kStatusResourceNotAvailable;
   devInfo->pci.irq = pciDevice->irq;
   return nNIKAL100_kStatusSuccess;
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL1700_pciReleaseMSI(nNIKAL100_tDeviceInfo *devInfo)
{
   nLinux_pciDevice* pciDevice = devInfo->osDevice;
   KAL_ASSERT(devInfo->busType == nNIKAL100_kBusTypePCI);
   pci_disable_msi(pciDevice);
   devInfo->pci.irq = -1;
   return nNIKAL100_kStatusSuccess;
}


static unsigned long translateInterruptHandlerFlags(nNIKAL100_tU32 flags)
{
   unsigned long kflags = 0;
   if (flags & nNIKAL1400_kInterruptFlagShared)
   {
      kflags |= NIKAL_IRQ_SHARED;
   }
   if (flags & nNIKAL1400_kInterruptFlagNoSoftIrqCall)
   {
      kflags |= NIKAL_IRQ_TRY_NO_SOFTIRQ_CALL;
   }
   return kflags;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_reserveInterrupt (nNIKAL100_tU32 irq, void *context,
   const nNIKAL100_tText *deviceName)
{
   return nNIKAL1400_reserveInterrupt(irq, context, deviceName, nNIKAL1400_kInterruptFlagShared);
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL1400_reserveInterrupt (nNIKAL100_tU32 irq, void *context,
   const nNIKAL100_tText *deviceName, nNIKAL100_tU32 flags)
{
   int status;
   nNIKAL210_tInternalInterruptSpec *internal_spec = NULL;

   internal_spec = nNIKAL100_malloc(sizeof(*internal_spec));
   if (unlikely(!internal_spec))
      return nNIKAL100_kStatusMemoryFull;

   internal_spec->spec = context;
   spin_lock_init(&internal_spec->lock);


   status = request_irq(irq, nNIKAL100_dispatchHardwareInterrupt,
                        translateInterruptHandlerFlags(flags),
                        deviceName, internal_spec);

   if (!status)
   {
      spin_lock(&nNIKAL210_sInterruptHandlerListLock);
      list_add(&internal_spec->list, &nNIKAL210_sInterruptHandlerList);
      spin_unlock(&nNIKAL210_sInterruptHandlerListLock);
   }
   else
   {
      nNIKAL100_free(internal_spec);
   }

   if (status == -ENOSYS)
      return nNIKAL100_kStatusBadDevice;
   else
      return nNIKAL100_convertLinuxToKALStatus(status);
}


nNIKAL100_cc void nNIKAL100_relinquishInterrupt (nNIKAL100_tU32 irq, void *context)
{
   nNIKAL210_tInternalInterruptSpec *cursor, *found = NULL;
   nNIKAL100_tInterruptCallbackSpec *spec = context;

   spin_lock(&nNIKAL210_sInterruptHandlerListLock);
   list_for_each_entry(cursor, &nNIKAL210_sInterruptHandlerList, list)
   {
      if (cursor->spec == spec)
      {
         found = cursor;
         list_del(&found->list);
         break;
      }
   }
   spin_unlock(&nNIKAL210_sInterruptHandlerListLock);

   KAL_ASSERT(found, "Trying to uninstall IRQ handler which wasn't installed in the first place. IRQ%lu\n", (unsigned long)irq);

   free_irq(irq, found);
   nNIKAL100_free(found);
}


nNIKAL100_cc void * nNIKAL100_createDPC(nNIKAL100_tDPCCallback dpcCallback)
{
   nNIKAL100_tDPC *dpc = nNIKAL100_malloc(sizeof(*dpc));

   if (dpc)
   {
      tasklet_init(&dpc->tasklet, nNIKAL100_dispatchDPC, (unsigned long) &dpc->callback);
      dpc->callback.func = dpcCallback;
   }

   return dpc;
}


nNIKAL100_cc void nNIKAL100_destroyDPC (void *_dpc)
{
   nNIKAL100_tDPC *dpc = _dpc;
   if (dpc)
      tasklet_kill(&dpc->tasklet); 
   nNIKAL100_free(dpc);
}


nNIKAL100_cc void nNIKAL100_scheduleDPC (void *_dpc, void *context)
{
   nNIKAL100_tDPC *dpc = _dpc;
   dpc->callback.context = context;

   
#ifdef CONFIG_PREEMPT_RT_FULL
   
   nNIKAL100_dispatchDPC(&dpc->callback);
#else
   tasklet_schedule(&dpc->tasklet);
#endif
}


nNIKAL100_cc void nNIKAL100_unmapPhysicalFromKernel (void __iomem * memoryArea)
{
   iounmap(memoryArea);
}


nNIKAL100_cc void __iomem * nNIKAL100_mapPhysicalToKernel (nNIKAL100_tUPtr physicalAddress,
   nNIKAL100_tUPtr byteCount)
{
   return ioremap(physicalAddress, byteCount);
}

nNIKAL100_cc void __iomem * nNIKAL230_mapPhysicalToKernel (nNIKAL100_tU64 physicalAddress,
                                                           size_t size,
                                                           unsigned long flags)
{
#ifdef nNIKAL230_kHas_ioremap_wc
   if (flags & nNIKAL230_kPhysMemFlagWriteCombined)
      return ioremap_wc(physicalAddress, size);
   else
#endif
      return ioremap(physicalAddress, size);
}

nNIKAL100_cc void nNIKAL230_unmapPhysicalFromKernel (void __iomem * memoryArea, unsigned long flags)
{
   iounmap(memoryArea);
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_loadDriver(const nNIKAL100_tText* driverName)
{
   int status = request_module("%s", driverName);

   
   if (status == -ENOSYS)
      return nNIKAL100_kStatusFeatureDisabled;
   else
      return nNIKAL100_convertLinuxToKALStatus(status);
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_incrementDriverRefcount(void* driverHandle)
{
   if (try_module_get((nLinux_driver*)driverHandle))
   {
      return nNIKAL100_kStatusSuccess;
   }
   else
   {
      return nNIKAL100_kStatusResourceNotAvailable;
   }
}


nNIKAL100_cc void nNIKAL100_decrementDriverRefcount(void* driverHandle)
{
   nLinux_driver* module = (nLinux_driver*) driverHandle;
   module_put(module);
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_unmapKernelFromUser (
   nNIKAL100_tUPtr mappedAddr,
   nNIKAL100_tUPtr inSize,
   const void *osData )
{
   void *addr;
   nNIKAL100_tUPtr size, offset;
   nNIKAL100_tStatus status;

   nNIKAL100_compileTimeAssert(sizeof(void*)==sizeof(nNIKAL100_tUPtr), "Oops, resizing void* into nNIKAL100_tUPtr!\n");
   nNIKAL100_pageAlignMemoryBlock((void *)mappedAddr, inSize, &addr, &size, &offset);

   nNIKAL100_compileTimeAssert(sizeof(void*)==sizeof(nNIKAL100_tUPtr), "Oops, resizing void* into nNIKAL100_tUPtr!\n");

   status = nNIKAL100_doMUnmap((nNIKAL100_tUPtr)addr, size);
   

   return status;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_mapKernelToUser (
   void * const file,
   nNIKAL100_tUPtr sourceAddr,
   nNIKAL100_tUPtr inSize,
   nNIKAL100_tMemoryAddressType addrType,
   nNIKAL100_tUPtr *targetAddr,
   void **osData)
{
   void *addr;
   nNIKAL100_tUPtr size, offset, outAddress;
   const nNIKAL100_tMMapData pageMapData =
   {
      .callingThreadID = current->pid,
      .addressType = addrType
   };

   nNIKAL100_compileTimeAssert(sizeof(void*)==sizeof(nNIKAL100_tUPtr), "Oops, resizing void* into nNIKAL100_tUPtr!\n");
   nNIKAL100_pageAlignMemoryBlock((void *)sourceAddr, inSize, &addr, &size, &offset);

   if (addrType == nNIKAL100_kMemoryAddressTypeVirtual)
   {
      KAL_ASSERT(osData != NULL,
         "nNIKAL100_mapKernelToUser(): when addrType is virtual, osData must NOT be NULL\n");
      *osData = addr;
   }

   down(&nNIKAL100_sMMapSema);
   nNIKAL100_gMMapDataPtr = &pageMapData;

   outAddress = nNIKAL100_doMMap(file, 0 , size, (nNIKAL100_tUPtr)addr);

   nNIKAL100_gMMapDataPtr = &nNIKAL100_gMMapData;
   up(&nNIKAL100_sMMapSema);

   

   
   if ( ((nNIKAL100_tIPtr)outAddress <= -1) && ((nNIKAL100_tIPtr)outAddress >= -256) )
   {
      *targetAddr = 0;
      if ((addrType == nNIKAL100_kMemoryAddressTypeVirtual))
      {
         *osData = NULL;
      }

      return nNIKAL100_kStatusOSFault;
   }
   *targetAddr = outAddress+offset;
   return nNIKAL100_kStatusSuccess;
}

static void nNIKAL100_setVMANoCopy(void *pageAlignedInPointer,
   nNIKAL100_tUPtr size, nNIKAL100_tMemPageLockToken *pageLockToken, nNIKAL100_tStatus *status)
{
   nLinux_vmArea *vmArea;
   nNIKAL100_tUPtr ptr, inPointer;
   nNIKAL100_tU32 i=0;
   nNIKAL100_tU32 vmaCount = 0;
   nNIKAL100_tUserMemMapAddressPair *vmaAddressPairs = NULL;
   nNIKAL100_tStatus localStatus = nNIKAL100_kStatusSuccess;
   nNIKAL100_tMapIterator mapIterator;

   KAL_ASSERT(status != NULL, "NULL status passed to internal function\n");
   if (nNIKAL100_statusIsFatal(*status))
      return;

   
   if (current->mm == NULL)
   {
      KAL_DPRINT("nNIKAL100_setVMANoCopy(): called when user mode address space is already destroyed\n");
      *status = nNIKAL100_statusSelect(*status, nNIKAL100_kStatusResourceNotAvailable);
      return;
   }

   
   down_write(&(current->mm->mmap_sem));

   
   inPointer = (nNIKAL100_tUPtr)pageAlignedInPointer;
   ptr = inPointer;
   do
   {
      vmArea = find_vma(current->mm, ptr);
      if (vmArea == NULL)
      {
         KAL_DPRINT("nNIKAL100_setVMANoCopy(): inPtr: %p, size: 0x%lx: cannot find VMA at address: %lx\n", pageAlignedInPointer, size, ptr);
         up_write(&(current->mm->mmap_sem));
         *status = nNIKAL100_statusSelect(*status, nNIKAL100_kStatusBadPointer);
         return;
      }
#ifdef CONFIG_HUGETLB_PAGE
      if (vmArea->vm_flags & VM_HUGETLB)
      {
         KAL_DPRINT("nNIKAL100_setVMANoCopy(): inPtr: %p, size: 0x%lx: Found VM_HUGETLB VMA: %p\n", pageAlignedInPointer, size, vmArea);
         up_write(&(current->mm->mmap_sem));
         *status = nNIKAL100_statusSelect(*status, nNIKAL100_kStatusFeatureNotSupported);
         return;
      }
#endif

      ptr = vmArea->vm_end;
      ++vmaCount;
   } while (ptr < inPointer+size);

   
   pageLockToken->vmaArraySize = vmaCount;
   if (vmaCount > 1)
   {
      vmaAddressPairs = (nNIKAL100_tUserMemMapAddressPair*)nNIKAL100_malloc(vmaCount * sizeof(nNIKAL100_tUserMemMapAddressPair));
      if (vmaAddressPairs == NULL)
      {
         KAL_DPRINT("nNIKAL100_setVMANoCopy(): Failed to allocate memory for vmaAddrPairs: vmaCount: %ld, each data sizeof(): %ld\n", (unsigned long)vmaCount, (unsigned long)sizeof(nNIKAL100_tUserMemMapAddressPair));
         up_write(&(current->mm->mmap_sem));
         *status = nNIKAL100_statusSelect(*status, nNIKAL100_kStatusMemoryFull);
         return;
      }
      pageLockToken->vma = vmaAddressPairs;
   }
   else
      pageLockToken->vma = vmArea;

   
   ptr = inPointer;
   for (i=0; i<vmaCount; ++i)
   {
      if (vmaCount > 1)
      {
         vmArea = find_vma(current->mm, ptr);
         vmaAddressPairs[i].vma = vmArea;
         vmaAddressPairs[i].address = ptr;
      }
      else
      {
         
      }

      nNIKAL100_mapFind(&nNIKAL100_sPageLockMap, (nNIKAL100_tUPtr)vmArea, &mapIterator);
      if (mapIterator.keyValuePtr != NULL)
      {
         ++(mapIterator.keyValuePtr->value);
      }
      else
      {
         if (((vmArea->vm_flags & VM_DONTCOPY) == 0) && ((vmArea->vm_flags & (VM_SHARED | VM_MAYWRITE)) == VM_MAYWRITE))
         {
            localStatus = nNIKAL100_mapInsert(&nNIKAL100_sPageLockMap, (nNIKAL100_tUPtr)vmArea, 1);
            if (nNIKAL100_statusIsNonfatal(localStatus))
            {
               vmArea->vm_flags |= VM_DONTCOPY;
               
            }
            else
               break; 
         }
         else
         {
            
            if (vmaAddressPairs)
            {
               vmaAddressPairs[i].vma = NULL;
               vmaAddressPairs[i].address = 0;
            }
            else
               pageLockToken->vma = NULL;
         }
      }

      
      ptr = vmArea->vm_end;
   }

   if (nNIKAL100_statusIsFatal(localStatus))
   {
      
      for (; i>0; --i)
      {
         vmArea = vmaAddressPairs[i-1].vma;
         nNIKAL100_mapFind(&nNIKAL100_sPageLockMap, (nNIKAL100_tUPtr)vmArea, &mapIterator);
         if (mapIterator.keyValuePtr != NULL)
         {
            --(mapIterator.keyValuePtr->value);
            if (mapIterator.keyValuePtr->value == 0)
            {
               nNIKAL100_mapRemove(&nNIKAL100_sPageLockMap, &mapIterator);
               vmArea->vm_flags &= ~VM_DONTCOPY;
               
            }
         }
      }
      nNIKAL100_free(vmaAddressPairs);
      pageLockToken->vmaArraySize = 0;
      pageLockToken->vma = NULL;

      *status = nNIKAL100_statusSelect(*status, localStatus);
   }

   up_write(&(current->mm->mmap_sem));
}

static void nNIKAL100_unsetVMANoCopy(nNIKAL100_tMemPageLockToken *pageLockToken)
{
   nLinux_vmArea *vmArea = NULL;
   nNIKAL100_tUPtr ptr = 0;
   nNIKAL100_tU32 i=0;
   nNIKAL100_tUserMemMapAddressPair *vmaAddressPairs = NULL;
   nNIKAL100_tMapIterator mapIterator;

   
   if (current->mm != NULL)
      down_write(&(current->mm->mmap_sem));

   if (pageLockToken->vmaArraySize == 1)
   {
      vmArea = (nLinux_vmArea*)(pageLockToken->vma);
      ptr = (nNIKAL100_tUPtr)(pageLockToken->originalPointer);
   }
   else
   {
      vmaAddressPairs = (nNIKAL100_tUserMemMapAddressPair*)(pageLockToken->vma);
      KAL_ASSERT(vmaAddressPairs != NULL, "nNIKAL100_pageUnlockUserPointer(): pageLockToken seems to be corrupted\n");
   }

   for (i=0; i<pageLockToken->vmaArraySize; ++i)
   {
      if (vmaAddressPairs)
      {
         vmArea = vmaAddressPairs[i].vma;
         ptr = vmaAddressPairs[i].address;
      }

      nNIKAL100_mapFind(&nNIKAL100_sPageLockMap, (nNIKAL100_tUPtr)vmArea, &mapIterator);
      if (mapIterator.keyValuePtr != NULL)
      {
         --(mapIterator.keyValuePtr->value);
         if (mapIterator.keyValuePtr->value == 0)
         {
            nNIKAL100_mapRemove(&nNIKAL100_sPageLockMap, &mapIterator);
            if (current->mm != NULL && vmArea == find_vma(current->mm, ptr))
            {
               
               vmArea->vm_flags &= ~VM_DONTCOPY;
            }
            else
            {
               
               
            }
         }
      }
   }

   nNIKAL100_free(vmaAddressPairs);
   pageLockToken->vmaArraySize = 0;
   pageLockToken->vma = NULL;

   if (current->mm != NULL)
      up_write(&(current->mm->mmap_sem));
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pageLockUserPointer(
   void __user * inPointer,
   const nNIKAL100_tUPtr inSize,
   void ** outPageLockedPointer,
   nNIKAL100_tMemPageLockToken *pageLockToken)
{
   
   nNIKAL100_tU32 pageSize;
   void *pageAlignedInPointer;
   nNIKAL100_tUPtr inPointerOffset, size;
   unsigned int sizeInPages;
   nNIKAL100_tStatus status = nNIKAL100_kStatusSuccess;
   size = inSize;
   *outPageLockedPointer = NULL;

   down(&nNIKAL100_sPageLockOperationSema);
   pageSize = nNIKAL100_getPageSize();

   nNIKAL100_pageAlignMemoryBlock(inPointer, inSize, &pageAlignedInPointer, &size, &inPointerOffset);

   if ((pageLockToken->pageLockDescriptor = nNIKAL100_mapUserKIOBuf(pageAlignedInPointer, size, &status)) != NULL)
   {
      nNIKAL100_setVMANoCopy(pageAlignedInPointer, size, pageLockToken, &status);
      if (nNIKAL100_statusIsNonfatal(status))
      {
         sizeInPages = size/pageSize;
         *outPageLockedPointer = vmap(((nLinux_physicalPage**)pageLockToken->pageLockDescriptor)+2, sizeInPages, VM_MAP, PAGE_KERNEL);

         if (*outPageLockedPointer != NULL)
         {
            pageLockToken->originalPointer = pageAlignedInPointer;
            pageLockToken->pageLockedPointer = *outPageLockedPointer;
            pageLockToken->bufferSize = size;
            pageLockToken->addressOrigin = nNIKAL100_kAddressOriginUser;
            
            *outPageLockedPointer = (void*)((nNIKAL100_tUPtr)(*outPageLockedPointer)+inPointerOffset);
            up(&nNIKAL100_sPageLockOperationSema);
            return nNIKAL100_kStatusSuccess;
         }
         else
         {
            KAL_DPRINT("_pageLockUserPointer: Failed to map to kernel\n");
            status = nNIKAL100_statusSelect(status, nNIKAL100_kStatusMemoryFull);
         }

         nNIKAL100_unsetVMANoCopy(pageLockToken);
      }
      else
      {
         KAL_DPRINT("_pageLockUserPointer: Failed to mark VMAs with VM_DONTCOPY (%p, %ld)\n", pageAlignedInPointer, size);
      }

      
      nNIKAL100_unmapUserKIOBuf(pageLockToken->pageLockDescriptor);
   }
   else
   {
      KAL_DPRINT("_pageLockUserPointer: Failed to mark VMAs with VM_DONTCOPY (%p, %ld)\n", pageAlignedInPointer, size);
   }

   
   *outPageLockedPointer = NULL;

   up(&nNIKAL100_sPageLockOperationSema);
   return status;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pageUnlockUserPointer(nNIKAL100_tMemPageLockToken * pageLockToken)
{
   down(&nNIKAL100_sPageLockOperationSema);

   
   flush_cache_all();

   vunmap(pageLockToken->pageLockedPointer);
   nNIKAL100_unsetVMANoCopy(pageLockToken);
   nNIKAL100_unmapUserKIOBuf(pageLockToken->pageLockDescriptor);

   up(&nNIKAL100_sPageLockOperationSema);

   return nNIKAL100_kStatusSuccess;
}

nNIKAL100_cc const nNIKAL100_tText **nNIVersion_NIKAL_getVersionStringArray(void)
{
   return nNIVersion_versionStringArray;
}



nNIKAL100_cc nNIKAL100_tUPtr nNIKAL120_checkStackUsage(void)
{
   nNIKAL100_tUPtr stackUsage = 
      nNIKAL120_mRoundUp(nNIKAL120_sStackSize, PAGE_SIZE);
   
   stackUsage = stackUsage - ((nNIKAL100_tUPtr)(&stackUsage) & (stackUsage-1));

   if (stackUsage > nNIKAL120_sMaxStackUsageSeen)
   {
      nNIKAL120_sMaxStackUsageSeen = stackUsage;
      if (stackUsage > nNIKAL120_kMinStackUsageForWarning)
      {
         KAL_DPRINT("nNIKAL120_checkStackUsage(): WARNING: stackUsage: %lu\n", stackUsage);
         /** Uncomment the line below to get the stack dump when stack is too deep **/
         /** dump_stack(); **/
      }
   }
   return stackUsage;
}


static nNIKAL220_tPageList *nNIKAL220_tPageList_allocContiguous(size_t pages, unsigned long flags)
{
   unsigned int order;
   nNIKAL220_tPageList *list = nNIKAL100_malloc(sizeof(*list) + sizeof(struct page*));

   gfp_t gfp_mask = __GFP_COMP;
#ifdef nNIKAL220_kEnable32BitPageListAlloc
   gfp_mask |= ((flags & nNIKAL220_kPageList32BitAddressable) ? GFP_DMA32 : GFP_KERNEL | __GFP_NORETRY);
#else
   gfp_mask |= GFP_KERNEL | __GFP_NORETRY;
#endif
   if (flags & nNIKAL220_kPageListZeroed)
      gfp_mask |= __GFP_ZERO;

   if (!list)
      return NULL;

   order = get_order(pages << PAGE_SHIFT);

   list->flags     = flags;
   list->num_pages = 1 << order;
   list->pages[0]  = alloc_pages(gfp_mask, order);

   if (!list->pages[0]) {
      nNIKAL100_free(list);
      return NULL;
   }

   return list;
}

static nNIKAL220_tPageList *nNIKAL220_tPageList_allocNonContiguous(size_t pages, unsigned long flags)
{
   int i;
   nNIKAL220_tPageList *list;
#ifdef nNIKAL220_kEnable32BitPageListAlloc
   gfp_t gfp_mask = ((flags & nNIKAL220_kPageList32BitAddressable) ? GFP_DMA32 : GFP_KERNEL | __GFP_NORETRY);
#else
   gfp_t gfp_mask = GFP_KERNEL | __GFP_NORETRY;
#endif
   if (flags & nNIKAL220_kPageListZeroed)
      gfp_mask |= __GFP_ZERO;

   list = nNIKAL100_malloc(sizeof(*list) + pages*sizeof(struct page*));
   if (!list)
      return NULL;

   list->flags     = flags;
   list->num_pages = pages;

   for (i=0; i<pages; i++) {
      list->pages[i] = alloc_page(gfp_mask);

      if (unlikely(!list->pages[i]))
         goto failed_page;
   }

   return list;

failed_page:
   while (i--)
      __free_page(list->pages[i]);
   nNIKAL100_free(list);
   return NULL;
}

nNIKAL100_cc nNIKAL220_tPageList *nNIKAL220_tPageList_createFromAllocation(size_t pages, unsigned long flags)
{
   if (flags & nNIKAL220_kPageListContiguous)
      return nNIKAL220_tPageList_allocContiguous(pages, flags);
   else
      return nNIKAL220_tPageList_allocNonContiguous(pages, flags);
}


nNIKAL100_cc void nNIKAL220_tPageList_destroy(nNIKAL220_tPageList *list)
{
   if (unlikely(!list))
      return;

   if (list->flags & nNIKAL220_kPageListFromUserBuffer) {
      while (list->num_pages--) {
         if (list->flags & nNIKAL220_kPageListAccessModeWrite) {
            set_page_dirty_lock(list->pages[list->num_pages]);
         }
         put_page(list->pages[list->num_pages]);
      }
   } else {
      if (list->flags & nNIKAL220_kPageListContiguous) {
         __free_pages(list->pages[0], get_order(list->num_pages << PAGE_SHIFT));
      } else {
         while (list->num_pages--)
            __free_page(list->pages[list->num_pages]);
      }
   }

   nNIKAL100_free(list);
}


nNIKAL100_cc void *nNIKAL220_tPageList_mapToKernel(nNIKAL220_tPageList *list, size_t offsetInPages, size_t sizeInPages)
{
   void *pointer = NULL;
   struct page **pages;
   int i;

   KAL_ASSERT(list);
   KAL_ASSERT((offsetInPages + sizeInPages) <= list->num_pages);
   KAL_ASSERT((offsetInPages + sizeInPages) >= offsetInPages);

   if (list->flags & nNIKAL220_kPageListContiguous) {
      pages = nNIKAL100_malloc(sizeInPages*sizeof(struct page*));
      if (pages == NULL)
         return NULL;

      for (i=0; i<sizeInPages; i++)
         pages[i] = list->pages[0] + offsetInPages + i;
   } else {
      pages = list->pages + offsetInPages;
   }

   pointer = vmap(pages, sizeInPages, VM_MAP, PAGE_KERNEL);

   if (list->flags & nNIKAL220_kPageListContiguous)
      nNIKAL100_free(pages);

   return pointer;
}


nNIKAL100_cc void nNIKAL220_tPageList_unmapFromKernel(nNIKAL220_tPageList* list, void *pointer)
{
   vunmap(pointer);
}


nNIKAL100_cc void *nNIKAL220_tPageList_mapOnePageToKernel(nNIKAL220_tPageList *list, size_t pageIndex)
{
   KAL_ASSERT(list != NULL);
   KAL_ASSERT(pageIndex < list->num_pages);
   if (list->flags & nNIKAL220_kPageListContiguous)
      return kmap(list->pages[0] + pageIndex);
   else
      return kmap(list->pages[pageIndex]);
}


nNIKAL100_cc void nNIKAL220_tPageList_unmapOnePageFromKernel(nNIKAL220_tPageList *list, size_t pageIndex)
{
   KAL_ASSERT(list != NULL);
   KAL_ASSERT(pageIndex < list->num_pages);
   if (list->flags & nNIKAL220_kPageListContiguous)
      kunmap(list->pages[0] + pageIndex);
   else
      kunmap(list->pages[pageIndex]);
}


nNIKAL100_cc nNIKAL220_tPageList *nNIKAL220_tPageList_createFromUser(void __user *pointer, size_t size, unsigned long flags)
{
   nNIKAL220_tPageList *list;
   int pinned;

   KAL_ASSERT(!((unsigned long) pointer & ~PAGE_MASK));

   list = nNIKAL100_malloc(sizeof(*list) + size*sizeof(struct page *));

   if (!list)
      return NULL;

   list->num_pages   = size;
   list->flags       = flags | nNIKAL220_kPageListFromUserBuffer;

   down_read(&current->mm->mmap_sem);
   pinned = nNIKAL1600_currentTaskGetUserPages((unsigned long) pointer,
                           size,
                           flags & nNIKAL220_kPageListAccessModeWrite,
                           0, 
                           list->pages,
                           NULL );
   up_read(&current->mm->mmap_sem);

   if (pinned < 0)
      goto error_nopages;

   if (pinned != size)
      goto error_somepages;

   return list;

error_somepages:
   while (pinned--)
      put_page(list->pages[pinned]);
error_nopages:
   nNIKAL100_free(list);
   return NULL;
}


nNIKAL100_cc void nNIKAL220_tPageList_getPhysicalAddress(const nNIKAL220_tPageList *list,
                                                         size_t offset,
                                                         nNIKAL100_tU64 *phys,
                                                         size_t *size)
{
   KAL_ASSERT(list && phys && size);
   KAL_ASSERT(offset < (list->num_pages << PAGE_SHIFT));

   if (list->flags & nNIKAL220_kPageListContiguous) {
      KAL_ASSERT(list->pages[0]);

      *phys = (nNIKAL100_tU64) page_to_phys(list->pages[0]) + offset;
      *size = (list->num_pages << PAGE_SHIFT) - offset;
   } else {
      size_t pageIndex = offset >> PAGE_SHIFT;
      size_t pageOffset = offset & (~PAGE_MASK);
      *phys = (nNIKAL100_tU64) page_to_phys(list->pages[pageIndex]) + pageOffset;
      *size = PAGE_SIZE - pageOffset;

      
      while ( pageIndex < list->num_pages - 1
           && list->pages[pageIndex]+1 == list->pages[pageIndex+1]) {
         *size += PAGE_SIZE;
         ++pageIndex;
      }
   }
   KAL_ASSERT(!(list->flags & nNIKAL220_kPageList32BitAddressable) || ((*phys + *size) <= (1ULL << 32)));
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL220_tUserMemMap_mapPageList(nNIKAL220_tUserMemMap *_vma,
                                                          nNIKAL220_tPageList *list,
                                                          size_t offsetInPages,
                                                          size_t sizeInPages)
{
   unsigned long vma_size;
   struct vm_area_struct *vma = (struct vm_area_struct *) _vma;
   nNIKAL220_tUserMemMapPrivateData *priv;

   KAL_ASSERT(vma);
   KAL_ASSERT(list);
   KAL_ASSERT(offsetInPages + sizeInPages <= list->num_pages);

   vma_size = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;

   
   if (vma_size < sizeInPages)
      sizeInPages = vma_size;

   vma->vm_pgoff  = offsetInPages;
   vma->vm_flags |= VM_LOCKED | VM_DONTCOPY | VM_DONTEXPAND;

   priv = nNIKAL200_mallocZeroed(sizeof(nNIKAL220_tUserMemMapPrivateData), GFP_KERNEL);
   if (!priv)
      return nNIKAL100_kStatusMemoryFull;

   priv->list = list;
   vma->vm_private_data = priv;
   vma->vm_ops = &nNIKAL220_vmaOpsPageList;

   return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL220_tUserMemMap_mapPhysicalAddress(nNIKAL220_tUserMemMap *_vma,
                                                                 nNIKAL100_tU64 address,
                                                                 size_t pages)
{
   return nNIKAL230_tUserMemMap_mapPhysicalAddress(_vma, address, pages, nNIKAL230_kPhysMemFlagDefault);
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL230_tUserMemMap_mapPhysicalAddress(nNIKAL220_tUserMemMap *_vma,
                                                                 nNIKAL100_tU64 address,
                                                                 size_t pages,
                                                                 unsigned long flags)
{
   struct vm_area_struct *vma = (struct vm_area_struct *) _vma;
   unsigned long vma_size = vma->vm_end - vma->vm_start;

   if (vma_size >> PAGE_SHIFT < pages)
      pages = vma_size >> PAGE_SHIFT;

#if defined(_PAGE_PWT)
   
   if (flags & nNIKAL230_kPhysMemFlagWriteCombined)
      vma->vm_page_prot = __pgprot(pgprot_val(vma->vm_page_prot) | __PAGE_KERNEL | _PAGE_PWT);
   else
#endif
      vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

   if (io_remap_pfn_range(vma, vma->vm_start,
                       address >> PAGE_SHIFT,
                       pages << PAGE_SHIFT,
                       vma->vm_page_prot))
      return nNIKAL100_kStatusValueConflict;

   vma->vm_flags |= VM_LOCKED | VM_DONTCOPY | VM_DONTEXPAND;
   vma->vm_ops = &nNIKAL220_vmaOpsPhysical;

   return nNIKAL100_kStatusSuccess;

}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL170_getCallStack(nNIKAL100_tU32 callingLevel, nNIKAL100_tU32 startLevel, void **retAddrBuffer)
{
#if defined(nNIKAL100_kDebuggingIsActive) && defined(__x86_64__)
   nNIKAL100_tU32 i;
   nNIKAL100_tU32 j;
   void ** currentAddr;
   void ** bottomOfStack;
   nLinux_driver * module;

   
   for (i=0;i<callingLevel;i++)
   {
      retAddrBuffer[i] = NULL;
   }

   currentAddr = (void **)nNIKAL120_mRoundUp((nNIKAL100_tUPtr)&callingLevel, sizeof(void*));

   bottomOfStack = (void **)nNIKAL120_mRoundUp((nNIKAL100_tUPtr)currentAddr,
                                               nNIKAL120_mRoundUp(nNIKAL120_sStackSize, PAGE_SIZE));

   for (i = j = 0; j < callingLevel && currentAddr < bottomOfStack; ++currentAddr)
   {
      
      if ((*currentAddr >= THIS_MODULE->module_init &&
           *currentAddr < THIS_MODULE->module_init + THIS_MODULE->init_text_size) ||
          (*currentAddr >= THIS_MODULE->module_core &&
           *currentAddr < THIS_MODULE->module_core + THIS_MODULE->core_text_size))
      {
         if (i >= startLevel)
            retAddrBuffer[j++]=*currentAddr;
         ++i;
         continue;
      }
      
      list_for_each_entry(module, &(THIS_MODULE->list), list)
      {
         
         if (module->mkobj.mod == module)
         {
            
            if ((*currentAddr >= module->module_init &&
                 *currentAddr < module->module_init + module->init_text_size) ||
                (*currentAddr >= module->module_core &&
                 *currentAddr < module->module_core + module->core_text_size))
            {
               if (i >= startLevel)
                  retAddrBuffer[j++]=*currentAddr;
               ++i;
               break;
            }
         }
      }
   }
   return j;
#else
   nNIKAL100_tU32 i;
   for (i=0;i<callingLevel;i++)
   {
      retAddrBuffer[i] = NULL;
   }
   return 0;
#endif
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL220_pciSetDMAMask(void *osDevice, nNIKAL100_tU64 mask)
{
   nLinux_pciDevice* pciDevice = osDevice;
   return nNIKAL100_convertLinuxToKALStatus(dma_set_mask(&pciDevice->dev, mask));
}


nNIKAL100_cc nNIKAL220_tSGL *nNIKAL220_tSGL_createForPCI(void *osDevice, size_t maxPages)
{
   nNIKAL220_tSGL *sgl = kzalloc(sizeof(*sgl), GFP_KERNEL);
   nLinux_pciDevice* pciDevice = osDevice;
   if (!sgl)
      return NULL;

   sgl->dev = ((pciDevice == NULL) ? NULL : &pciDevice->dev);

#ifdef nNIKAL220_kHasChainedSGL
   if (sg_alloc_table(&sgl->table, maxPages, GFP_KERNEL))
   {
      kfree(sgl);
      return NULL;
   }
   sgl->table.nents = 0;
#else
   sgl->table.sgl = vmalloc(sizeof(struct scatterlist) * maxPages);
   if (sgl->table.sgl == NULL)
   {
      kfree(sgl);
      return NULL;
   }
   memset(sgl->table.sgl, 0, sizeof(struct scatterlist) * maxPages);
   sgl->table.orig_nents = maxPages;
   sg_mark_end(&(sgl->table.sgl[maxPages - 1]));
#endif
   return sgl;
}

static void nNIKAL220_tSGL_setPage(struct scatterlist *sg, struct page *pg, size_t sizeInBytes, size_t offsetInBytes)
{
   KAL_ASSERT(sg != NULL);
#if defined(nNIKAL220_kHasChainedSGL) || defined(nNIKAL220_kHasSGLPageAPI)
   sg_set_page(sg, pg, sizeInBytes, offsetInBytes);
#else
   sg->page = pg;
   sg->offset = offsetInBytes;
   sg->length = sizeInBytes;
#endif
}

static inline void nNIKAL220_tSGL_unmarkEnd(struct scatterlist *sg)
{
   KAL_ASSERT(sg != NULL);
   sg->page_link &= ~0x02;
}

static struct scatterlist *nNIKAL220_tSGL_nextSG(nNIKAL220_tSGL *kalSgl, struct scatterlist *sg)
{
   KAL_ASSERT(sg != NULL);
#if defined(nNIKAL220_kHasChainedSGL) || defined(nNIKAL220_kHasSGLPageAPI)
   return sg_next(sg);
#else
   if (sg_is_last(sg)) return NULL;
   ++sg;
   if (sg >= (kalSgl->table.sgl + kalSgl->table.orig_nents)) return NULL;
   else return sg;
#endif
}


nNIKAL100_cc nNIKAL100_tStatus nNIKAL220_tSGL_map(nNIKAL220_tSGL *sgl, nNIKAL220_tPageList *pageList, size_t offsetInBytes, size_t sizeInBytes, nNIKAL220_tDMADirection dir)
{
   int nents = 0;
   struct scatterlist *cursg, *nextsg;
   KAL_ASSERT(sgl != NULL);
   KAL_ASSERT(pageList != NULL);
   KAL_ASSERT((offsetInBytes + sizeInBytes + PAGE_SIZE - 1) / PAGE_SIZE <= pageList->num_pages);
   KAL_ASSERT((offsetInBytes + sizeInBytes) >= offsetInBytes);
   sgl->dir = (dir == nNIKAL220_kDMADirectionHostToDevice ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
   if (pageList->flags & nNIKAL220_kPageListContiguous)
   {
      nents = 1;
      nNIKAL220_tSGL_setPage(sgl->table.sgl, pageList->pages[0], sizeInBytes, offsetInBytes);
      sg_mark_end(sgl->table.sgl);
   }
   else
   {
      
      int pageOffset = offsetInBytes & (~PAGE_MASK);
      int pageIndex = offsetInBytes / PAGE_SIZE;
      int lastPage = pageIndex + ((pageOffset + sizeInBytes + PAGE_SIZE - 1) / PAGE_SIZE);
      unsigned int mappedSize = 0;
      struct scatterlist *sg = sgl->table.sgl;
      nents = lastPage - pageIndex;
      while (pageIndex < lastPage)
      {
         unsigned int sgSize = PAGE_SIZE - pageOffset;
         if ((mappedSize + sgSize) > sizeInBytes)
            sgSize = sizeInBytes - mappedSize;
         nNIKAL220_tSGL_setPage(sg, pageList->pages[pageIndex], sgSize, pageOffset);
         mappedSize += sgSize;
         ++pageIndex;
         pageOffset = 0;
         nNIKAL220_tSGL_unmarkEnd(sg);
         if (pageIndex >= lastPage)
            sg_mark_end(sg);
         sg = nNIKAL220_tSGL_nextSG(sgl, sg);
      }
   }
   KAL_ASSERT(nents <= sgl->table.orig_nents);
   sgl->mapped_nents = dma_map_sg(sgl->dev, sgl->table.sgl, nents, sgl->dir);

   
   sgl->table.nents = nents;
   if (sgl->mapped_nents == 0)
      return nNIKAL100_kStatusMemoryFull;

   // Coalesce adjacent entries.  The DMA API generally doesn't do this; it
   // will only coalesce if an IOMMU is able to map the segments into a
   // contiguous device-visible range.
   cursg = sgl->table.sgl;
   while (((nextsg = nNIKAL220_tSGL_nextSG(sgl, cursg)) != NULL))
   {
      if (sg_dma_address(nextsg) == (sg_dma_address(cursg) + sg_dma_len(cursg))) --sgl->mapped_nents;
      cursg = nextsg;
   }

   
   nNIKAL220_tSGL_iterate(sgl);
   return nNIKAL100_kStatusSuccess;
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL220_tSGL_numSegments(nNIKAL220_tSGL *sgl)
{
   KAL_ASSERT(sgl != NULL);
   return (nNIKAL100_tU32) sgl->mapped_nents;
}


nNIKAL100_cc void nNIKAL220_tSGL_iterate(nNIKAL220_tSGL *sgl)
{
   KAL_ASSERT(sgl != NULL);
   sgl->iterator = sgl->table.sgl;
}


nNIKAL100_cc void nNIKAL220_tSGL_nextSegment(nNIKAL220_tSGL *sgl)
{
   struct scatterlist *nextsg;
   KAL_ASSERT(sgl != NULL);
   if (sgl->iterator == NULL) return;
   // Coalesce adjacent mapped segments
   while ((nextsg = nNIKAL220_tSGL_nextSG(sgl, sgl->iterator)) != NULL)
   {
      if (sg_dma_address(nextsg) != (sg_dma_address(sgl->iterator) + sg_dma_len(sgl->iterator))) break;
      sgl->iterator = nextsg;
   }
   sgl->iterator = nextsg;
}


nNIKAL100_cc nNIKAL100_tU64 nNIKAL220_tSGL_getSegmentAddress(nNIKAL220_tSGL *sgl)
{
   KAL_ASSERT(sgl != NULL);
   if (sgl->iterator != NULL)
      return sg_dma_address(sgl->iterator);
   return 0;
}


nNIKAL100_cc size_t nNIKAL220_tSGL_getSegmentSize(nNIKAL220_tSGL *sgl)
{
   struct scatterlist *cursg, *nextsg;
   size_t len;
   KAL_ASSERT(sgl != NULL);
   if (sgl->iterator == NULL) return 0;
   cursg = sgl->iterator;
   len = sg_dma_len(sgl->iterator);
   // Coalesce adjacent mapped segments
   while ((nextsg = nNIKAL220_tSGL_nextSG(sgl, cursg)) != NULL)
   {
      if (sg_dma_address(nextsg) != (sg_dma_address(cursg) + sg_dma_len(cursg))) break;
      len += sg_dma_len(nextsg);
      cursg = nextsg;
   }
   return len;
}


nNIKAL100_cc void nNIKAL220_tSGL_prepareForDMA(nNIKAL220_tSGL *sgl)
{
   KAL_ASSERT(sgl != NULL);
   KAL_ASSERT(sgl->table.sgl != NULL);
   dma_sync_sg_for_device(sgl->dev, sgl->table.sgl, sgl->table.nents, sgl->dir);
}


nNIKAL100_cc void nNIKAL220_tSGL_completeDMA(nNIKAL220_tSGL *sgl)
{
   KAL_ASSERT(sgl != NULL);
   KAL_ASSERT(sgl->table.sgl != NULL);
   dma_sync_sg_for_cpu(sgl->dev, sgl->table.sgl, sgl->table.nents, sgl->dir);
}


nNIKAL100_cc void nNIKAL220_tSGL_unmap(nNIKAL220_tSGL *sgl)
{
   KAL_ASSERT(sgl != NULL);
   if (!sgl->table.nents)
      return;
   KAL_ASSERT(sgl->table.sgl != NULL);
   dma_unmap_sg(sgl->dev, sgl->table.sgl, sgl->table.nents, sgl->dir);
   sgl->table.nents = 0;
   sgl->mapped_nents = 0;
   sgl->iterator = NULL;
}


nNIKAL100_cc void nNIKAL220_tSGL_destroy(nNIKAL220_tSGL *sgl)
{
   if (sgl != NULL)
   {
      KAL_ASSERT(sgl->table.nents == 0);
#ifdef nNIKAL220_kHasChainedSGL
      sg_free_table(&sgl->table);
#else
      vfree(sgl->table.sgl);
#endif
      kfree(sgl);
   }
}

nNIKAL100_cc nNIKAL100_tU32 nNIKAL230_getCurrentEffectiveUserID(void)
{
#ifdef nNIKAL230_kHasCred
   #ifdef nNIKAL1400_kHasUidGid
   return from_kuid(current_user_ns(), current_euid());
   #else
   return current_euid();
   #endif
#else
   return current->euid;
#endif
}

nNIKAL100_cc void* nNIKAL250_createKernelSocketBuffer(size_t sizeInBytes)
{
#ifdef nNIKAL250_kHasGenlmsgNew
   return genlmsg_new(sizeInBytes, GFP_ATOMIC);
#else
   return alloc_skb(NLMSG_ALIGN(NLMSG_HDRLEN + (NLMSG_ALIGN(GENL_HDRLEN + sizeInBytes))), GFP_ATOMIC);
#endif
}

nNIKAL100_cc void nNIKAL250_releaseKernelSocketBuffer(void *socketBuffer)
{
   nlmsg_free((struct sk_buff*)socketBuffer);
}

nNIKAL100_cc nNIKAL100_tStatus nNIKAL250_kernelSocketSend(void *socketBuffer,
                                                          nNIKAL100_tU32 pid,
                                                          nNIKAL100_tU16 attribute,
                                                          const void *message,
                                                          size_t sizeInBytes)
{
   struct sk_buff *skb = socketBuffer;
   void *header;
   int error;
   KAL_ASSERT(sizeInBytes < INT_MAX);
#ifdef nNIKAL250_kHasFamilyGenlmsgPut
   header = genlmsg_put(skb, pid, 0, &nikal_netlink_family, 0, NLNIKAL_CMD_SEND);
#else
   header = genlmsg_put(skb, pid, 0, nikal_netlink_family.id, nikal_netlink_family.hdrsize, 0, NLNIKAL_CMD_SEND, nikal_netlink_family.version);
#endif
   if (unlikely(header == NULL) || (unlikely(nla_put(skb, attribute, sizeInBytes, message))))
   {
      return nNIKAL100_kStatusMemoryFull;
   }
   genlmsg_end(skb, header);
#ifdef nNIKAL250_kHasNamespacedGenetlink
   error = genlmsg_unicast(&init_net, skb, pid);
#else
   error = genlmsg_unicast(skb, pid);
#endif
   return nNIKAL100_convertLinuxToKALStatus(error);
}



nNIKAL100_cc void *nNIKAL100_registerSerialCoreDriver(nNIKAL100_tSerialCoreDriverData *driverData)
{
#if nNIKAL100_mSerialCoreIsSupported
   int retVal;
   nLinux_uartDriver *uartDriver;

   KAL_ASSERT(driverData != NULL);

   uartDriver = nNIKAL200_mallocZeroed(sizeof(nLinux_uartDriver), GFP_KERNEL);
   if (uartDriver == NULL)
   {
      KAL_DPRINT("Failed to allocate memory for uartDriver\n");
      return NULL;
   }

   
   if (driverData->owningModule)
      uartDriver->owner = (nLinux_driver*)(driverData->owningModule);
   else
      uartDriver->owner = THIS_MODULE;

   
   uartDriver->driver_name = driverData->driverName;
   uartDriver->dev_name    = driverData->portPrefix;
   uartDriver->nr          = driverData->maxPorts;

   
   uartDriver->major       = 0; // Automatically allocate a major number
   uartDriver->minor       = 0; // We do not worry about the minor number
   uartDriver->cons        = NULL; // No console support

   retVal = uart_register_driver(uartDriver);

   if (retVal != 0)
   {
      KAL_DPRINT("uart_register_driver failed with a return value of %d\n", retVal);
      nNIKAL100_free(uartDriver);
      return NULL;
   }

   KAL_DPRINT("Registered driver with serial core (handle: %p)\n", uartDriver);

   return uartDriver;
#else
   return NULL;
#endif
}


nNIKAL100_cc void nNIKAL100_unregisterSerialCoreDriver(void *driver)
{
#if nNIKAL100_mSerialCoreIsSupported
   nLinux_uartDriver *uartDriver;

   KAL_ASSERT(driver != NULL);

   uartDriver = (nLinux_uartDriver*)driver;

   uart_unregister_driver(uartDriver);

   KAL_DPRINT("Unregistered driver (handle: %p) from serial core\n", driver);

   nNIKAL100_free(uartDriver);
#endif
}



nNIKAL100_cc void *nNIKAL100_addOneSerialPort(void *driver, nNIKAL100_tSerialCorePortData *portData, nNIKAL100_tSerialCoreTxBuffer *txBuffer)
{
#if nNIKAL100_mSerialCoreIsSupported
   int retVal;
   nLinux_uartPort *uartPort;
   nLinux_uartDriver *uartDriver;

   KAL_ASSERT(driver != NULL);
   KAL_ASSERT(portData != NULL);
   KAL_ASSERT(portData->portOperations != NULL);
   KAL_ASSERT(txBuffer != NULL);

   uartDriver = (nLinux_uartDriver*)driver;

   uartPort = nNIKAL200_mallocZeroed(sizeof(nLinux_uartPort), GFP_KERNEL);
   if (uartPort == NULL)
   {
      KAL_DPRINT("Failed to allocate memory for uartPort\n");
      return NULL;
   }

   
   uartPort->type             = portData->portType;
   uartPort->line             = portData->lineNumber;
   uartPort->private_data     = (void*)(portData->portOperations);

   
   uartPort->ops              = &nNIKAL100_uartOps;

#ifdef nNIKAL100_kHasRS485Support
   if (portData->txvrType == nNIKAL200_kSerialTransceiverType_Rs485)
   {
      uartPort->txvr_ops      = &nNIKAL100_uartRs485Ops;
#ifdef nNIKAL1500_kHasRS485ConfigOnUart
      uartPort->rs485_config  = &nNIKAL100_uartRs485Ops_config_rs485;
#endif
      
      uartPort->rs485.flags = SER_RS485_ENABLED | SER_RS485_RTS_ON_SEND;
   }
#endif

   
   uartPort->iotype = UPIO_MEM; 
   uartPort->flags = UPF_FIXED_PORT; 

   /**
    * The flag UPF_FIXED_TYPE specifies that the UART type is known and should
    * not be probed. However, this flag is not present in version 2.6.18
    * of the kernel which nikal has to support. Refer CAR 415089.
    **/

   retVal = uart_add_one_port(uartDriver, uartPort);

   if (retVal != 0)
   {
      KAL_DPRINT("uart_add_one_port failed with a return value of %d\n", retVal);
      nNIKAL100_free(uartPort);
      return NULL;
   }

   txBuffer->pBuffer   = &(uartPort->state->xmit.buf);
   txBuffer->pHead     = &(uartPort->state->xmit.head);
   txBuffer->pTail     = &(uartPort->state->xmit.tail);
   txBuffer->maxSize   = UART_XMIT_SIZE;

   txBuffer->pTotalTxCount = &((uartPort->icount).tx);

   KAL_DPRINT("Added a new port (driver handle: %p, port handle: %p)\n", driver, uartPort);

   return uartPort;
#else
   return NULL;
#endif
}


nNIKAL100_cc void nNIKAL100_removeOneSerialPort(void *driver, void *port)
{
#if nNIKAL100_mSerialCoreIsSupported
   nLinux_uartDriver *uartDriver;
   nLinux_uartPort *uartPort;
   int retVal;

   KAL_ASSERT(driver != NULL);
   KAL_ASSERT(port != NULL);

   uartDriver = (nLinux_uartDriver*)driver;
   uartPort = (nLinux_uartPort*)port;

   retVal = uart_remove_one_port(uartDriver, uartPort);

   if (retVal != 0)
   {
      KAL_DPRINT("uart_remove_one_port failed with a return value of %d\n", retVal);
      nNIKAL100_free(uartPort);
      return;
   }

   KAL_DPRINT("Removed port (driver handle: %p, port handle: %p)\n", driver, port);

   nNIKAL100_free(uartPort);
#endif
}


nNIKAL100_cc void nNIKAL100_insertCharToSerialRxBuffer(void *port, char ch, nNIKAL100_tSerialCoreError errorFlag, bool ignore)
{
#if nNIKAL100_mSerialCoreIsSupported
   nLinux_uartPort *uartPort;
   nLinux_ttyPort  *ttyPort;

   KAL_ASSERT(port != NULL);
   uartPort = (nLinux_uartPort*)port;

   KAL_ASSERT(uartPort->state != NULL);
   ttyPort = &(uartPort->state->port);

   
   switch(errorFlag)
   {
      case nNIKAL100_kSerialCoreError_None:
         break;
      case nNIKAL100_kSerialCoreError_Break:
         uartPort->icount.brk++;
         break;
      case nNIKAL100_kSerialCoreError_Frame:
         uartPort->icount.frame++;
         break;
      case nNIKAL100_kSerialCoreError_Parity:
         uartPort->icount.parity++;
         break;
      case nNIKAL100_kSerialCoreError_Overrun:
         uartPort->icount.overrun++;
         break;
      default:
         KAL_ASSERT(0, "Unknown serial core error value");
   }

   uartPort->icount.rx++;

   if (ignore)
      return;

   if (tty_insert_flip_char(
#ifdef nNIKAL100_kUseTtyPortParam
         ttyPort,
#else
         ttyPort->tty,
#endif
         ch,
         (unsigned int)errorFlag
         ) == 0)
   {
      
      uartPort->icount.buf_overrun++;
   }
#endif
}


nNIKAL100_cc void nNIKAL100_pushSerialRxBufferToUser(void *port)
{
#if nNIKAL100_mSerialCoreIsSupported
   nLinux_uartPort *uartPort;
   nLinux_ttyPort  *ttyPort;

   KAL_ASSERT(port != NULL);
   uartPort = (nLinux_uartPort*)port;

   KAL_ASSERT(uartPort->state != NULL);
   ttyPort = &(uartPort->state->port);

   tty_flip_buffer_push(
#ifdef nNIKAL100_kUseTtyPortParam
      ttyPort
#else
      ttyPort->tty
#endif
      );
#endif
}

nNIKAL100_cc void nNIKAL250_halt(const char *component, const char *file, int line, const char *message)
{
   panic("[%s|%s|%d]: %s\n", component, file, line, message);
}



nNIKAL100_cc void nNIKAL100_serialCoreTxWakeup(void *port)
{
#if nNIKAL100_mSerialCoreIsSupported
   nLinux_uartPort *uartPort = (nLinux_uartPort*)port;
   KAL_ASSERT(uartPort != NULL);
   uart_write_wakeup(uartPort);
#endif
}


nNIKAL100_cc void nNIKAL100_serialCoreHandleModemChange(void *port, unsigned int msr)
{
#if nNIKAL100_mSerialCoreIsSupported
   nLinux_uartPort *uartPort;
   nLinux_ttyPort  *ttyPort;

   KAL_ASSERT(port != NULL);
   uartPort = (nLinux_uartPort*)port;

   KAL_ASSERT(uartPort->state != NULL);
   ttyPort = &(uartPort->state->port);

   KAL_ASSERT(ttyPort != NULL);

   
   if (msr & UART_MSR_TERI)
      uartPort->icount.rng++;

   if (msr & UART_MSR_DDSR)
      uartPort->icount.dsr++;

   
   if (msr & UART_MSR_DDCD)
   {
      uartPort->icount.dcd++;
      
      if (ttyPort->flags & ASYNC_CHECK_CD)
      {
         if (msr & UART_MSR_DCD)
         {
            wake_up_interruptible(&ttyPort->open_wait);
         }
         else if (ttyPort->tty != NULL)
         {
            tty_hangup(ttyPort->tty);
         }
      }
   }

   
   if (msr & UART_MSR_DCTS)
   {
      uartPort->icount.cts++;

      
      if (ttyPort->flags & ASYNC_CTS_FLOW)
      {
         if (msr & UART_MSR_CTS)
         {
            // CTS is set means we are clear to send. If the tty is stopped,
            // we'll start transmission.
            if (ttyPort->tty->hw_stopped)
            {
               ttyPort->tty->hw_stopped = 0;
               nNIKAL100_uartOps_start_tx(uartPort);
               nNIKAL100_serialCoreTxWakeup(uartPort);
            }
         }
         else
         {
            // CTS is not set, means we should stop sending if we are already
            // not stopped.
            if (!ttyPort->tty->hw_stopped)
            {
               ttyPort->tty->hw_stopped = 1;
               nNIKAL100_uartOps_stop_tx(uartPort);
            }
         }
      }
   }

   
   wake_up_interruptible(&(uartPort->state->port.delta_msr_wait));
#endif
}


nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_serialCoreLockPort(void *port)
{
#if nNIKAL100_mSerialCoreIsSupported
   nLinux_uartPort *uartPort;

   KAL_ASSERT(port != NULL);
   uartPort = (nLinux_uartPort*)port;

   return nNIKAL100_acquireSpinLockInterrupt(&uartPort->lock);
#else
   return 0;
#endif
}


nNIKAL100_cc void nNIKAL100_serialCoreUnlockPort(void *port, nNIKAL100_tU32 flags)
{
#if nNIKAL100_mSerialCoreIsSupported
   nLinux_uartPort *uartPort;

   KAL_ASSERT(port != NULL);
   uartPort = (nLinux_uartPort*)port;

   nNIKAL100_releaseSpinLockInterrupt(&uartPort->lock, flags);
#endif
}


nNIKAL100_cc nNIKAL100_tBoolean nNIKAL1500_serialCoreIsPortLockedOnWiremodeConfig(void)
{
#ifdef nNIKAL1500_kHasRS485ConfigOnUart
   return 1;
#else
   return 0;
#endif
}

nNIKAL100_cc void *nNIKAL1500_registerSysctlInt_debugKALOnly(const char *rootName, const char *sysctlName, int *sysctlVal, unsigned short accessMode)
{
#ifdef nNIKAL100_kDebuggingIsActive

   
   struct ctl_table *table = kzalloc(sizeof(*table) * 4, GFP_KERNEL);
   struct ctl_table_header *header;
   if (table == NULL) return NULL;
   table[0].procname = rootName;
   table[0].mode = 0555;
   table[0].child = &table[2];
   table[2].procname = sysctlName;
   table[2].data = sysctlVal;
   table[2].maxlen = sizeof(*sysctlVal);
   table[2].mode = accessMode;
   table[2].proc_handler = proc_dointvec;

   header = register_sysctl_table(table);
   if (header == NULL) kfree(table);
   return header;
#else
   return NULL;
#endif
}

nNIKAL100_cc void nNIKAL1500_unregisterSysctl_debugKALOnly(void *sysctlObj)
{
#ifdef nNIKAL100_kDebuggingIsActive
   struct ctl_table_header *header = sysctlObj;
   struct ctl_table *table;
   if (header == NULL) return;
   table = header->ctl_table_arg;
   unregister_sysctl_table(header);
   kfree(table);
#endif
}

module_init(nNIKAL100_initDriver);
module_exit(nNIKAL100_cleanupDriver);

#ifdef EXPORT_SYMBOL_NOVERS
#define nNIKAL100_mExportSymbol EXPORT_SYMBOL_NOVERS
#else
#define nNIKAL100_mExportSymbol EXPORT_SYMBOL
#endif
nNIKAL100_mExportSymbol(nNIKAL200_isAddressableMemOver4G);
nNIKAL100_mExportSymbol(nNIKAL100_initializeSingleUseEvent);
nNIKAL100_mExportSymbol(nNIKAL100_relinquishInterrupt);
nNIKAL100_mExportSymbol(nNIKAL100_vsprintf);
nNIKAL100_mExportSymbol(nNIKAL100_createDPC);
nNIKAL100_mExportSymbol(nNIKAL100_getKernelVersion);
nNIKAL100_mExportSymbol(nNIKAL100_setPCIDeviceIDTableElement);
nNIKAL100_mExportSymbol(nNIKAL100_setUSBDeviceIDTableElement);
nNIKAL100_mExportSymbol(nNIKAL100_memoryRead16);
nNIKAL100_mExportSymbol(nNIKAL100_getPhysicalAddress);
nNIKAL100_mExportSymbol(nNIKAL100_ioWrite16);
nNIKAL100_mExportSymbol(nNIKAL100_enumeratePCIBuses);
nNIKAL100_mExportSymbol(nNIKAL100_sleepTimeout);
nNIKAL100_mExportSymbol(nNIKAL250_sleepTimeout);
nNIKAL100_mExportSymbol(nNIKAL100_getTimerInterval);
nNIKAL100_mExportSymbol(nNIKAL100_sleepTimeoutInterruptible);
nNIKAL100_mExportSymbol(nNIKAL200_yield);
nNIKAL100_mExportSymbol(nNIKAL100_createPCIDeviceIDTable);
nNIKAL100_mExportSymbol(nNIKAL100_createUSBDeviceIDTable);
nNIKAL100_mExportSymbol(nNIKAL100_malloc);
nNIKAL100_mExportSymbol(nNIKAL100_malloc32BitPhysicalContiguous);
nNIKAL100_mExportSymbol(nNIKAL100_registerDriver);
nNIKAL100_mExportSymbol(nNIKAL100_getTimerCount);
nNIKAL100_mExportSymbol(nNIKAL110_getTimerCount);
nNIKAL100_mExportSymbol(nNIKAL100_pciConfigRead16);
nNIKAL100_mExportSymbol(nNIKAL100_ioRead32);
nNIKAL100_mExportSymbol(nNIKAL100_destroyPCIDeviceIDTable);
nNIKAL100_mExportSymbol(nNIKAL100_destroyUSBDeviceIDTable);
nNIKAL100_mExportSymbol(nNIKAL100_ioWrite8);
nNIKAL100_mExportSymbol(nNIKAL100_unmapPhysicalFromKernel);
nNIKAL100_mExportSymbol(nNIKAL100_acquireSpinLockDPC);
nNIKAL100_mExportSymbol(nNIKAL110_acquireSpinLockInterrupt);
nNIKAL100_mExportSymbol(nNIKAL100_acquireSpinLockInterrupt);
nNIKAL100_mExportSymbol(nNIKAL100_acquireSemaphoreInterruptible);
nNIKAL100_mExportSymbol(nNIKAL100_getSingleUseEventSize);
nNIKAL100_mExportSymbol(nNIKAL100_destroyDPC);
nNIKAL100_mExportSymbol(nNIKAL100_getPhysicalMemorySize);
nNIKAL100_mExportSymbol(nNIKAL100_getPageSize);
nNIKAL100_mExportSymbol(nNIKAL100_printToDebugger);
nNIKAL100_mExportSymbol(nNIKAL1500_printLineToDebugger);
nNIKAL100_mExportSymbol(nNIKAL100_vsnPrintf);
nNIKAL100_mExportSymbol(nNIKAL100_replicateOutboundIOControlBuffer);
nNIKAL100_mExportSymbol(nNIKAL100_pciConfigWrite32);
nNIKAL100_mExportSymbol(nNIKAL100_pciConfigWrite16);
nNIKAL100_mExportSymbol(nNIKAL100_releaseSemaphore);
nNIKAL100_mExportSymbol(nNIKAL100_acquireSemaphore);
nNIKAL100_mExportSymbol(nNIKAL100_getTimeOfDay);
nNIKAL100_mExportSymbol(nNIKAL100_memoryRead8);
nNIKAL100_mExportSymbol(nNIKAL100_releaseSpinLockDPC);
nNIKAL100_mExportSymbol(nNIKAL100_registerPCIDriver);
nNIKAL100_mExportSymbol(nNIKAL200_registerPCIDriver);
nNIKAL100_mExportSymbol(nNIKAL100_registerUSBDriver);
nNIKAL100_mExportSymbol(nNIKAL1500_helperACPIProbe);
nNIKAL100_mExportSymbol(nNIKAL1500_registerACPIDriver);
nNIKAL100_mExportSymbol(nNIKAL1500_callACPIMethodGetInteger);
nNIKAL100_mExportSymbol(nNIKAL1500_callACPIMethod);
nNIKAL100_mExportSymbol(nNIKAL100_getThreadID);
nNIKAL100_mExportSymbol(nNIKAL100_pciConfigRead8);
nNIKAL100_mExportSymbol(nNIKAL100_free);
nNIKAL100_mExportSymbol(nNIKAL100_free32BitPhysicalContiguous);
nNIKAL100_mExportSymbol(nNIKAL100_waitForSingleUseEventTimeoutInterruptible);
nNIKAL100_mExportSymbol(nNIKAL100_scheduleDPC);
nNIKAL100_mExportSymbol(nNIKAL100_pciConfigRead32);
nNIKAL100_mExportSymbol(nNIKAL100_ioRead16);
nNIKAL100_mExportSymbol(nNIKAL100_pciConfigWrite8);
nNIKAL100_mExportSymbol(nNIKAL100_memoryWrite32);
nNIKAL100_mExportSymbol(nNIKAL100_getTimeOfDayInterval);
nNIKAL100_mExportSymbol(nNIKAL100_createSpinLock);
nNIKAL100_mExportSymbol(nNIKAL100_ioWrite32);
nNIKAL100_mExportSymbol(nNIKAL100_reserveInterrupt);
nNIKAL100_mExportSymbol(nNIKAL1400_reserveInterrupt);
nNIKAL100_mExportSymbol(nNIKAL230_pciRequestMSI);
nNIKAL100_mExportSymbol(nNIKAL1700_pciReleaseMSI);
nNIKAL100_mExportSymbol(nNIKAL100_createSystemThread);
nNIKAL100_mExportSymbol(nNIKAL200_createThread);
nNIKAL100_mExportSymbol(nNIKAL250_createNamedThread);
nNIKAL100_mExportSymbol(nNIKAL200_joinThread);
nNIKAL100_mExportSymbol(nNIKAL100_acquireSemaphoreZeroTimeout);
nNIKAL100_mExportSymbol(nNIKAL100_getThreadHandle);
nNIKAL100_mExportSymbol(nNIKAL100_memoryWrite16);
nNIKAL100_mExportSymbol(nNIKAL100_pageLockUserPointer);
nNIKAL100_mExportSymbol(nNIKAL100_destroySemaphore);
nNIKAL100_mExportSymbol(nNIKAL100_destroySpinLock);
nNIKAL100_mExportSymbol(nNIKAL100_unregisterDriver);
nNIKAL100_mExportSymbol(nNIKAL200_registerDriver);
nNIKAL100_mExportSymbol(nNIKAL200_unregisterDriver);
nNIKAL100_mExportSymbol(nNIKAL100_ioRead8);
nNIKAL100_mExportSymbol(nNIKAL100_memoryRead32);
nNIKAL100_mExportSymbol(nNIKAL100_createSemaphore);
nNIKAL100_mExportSymbol(nNIKAL100_getNumberOfActiveProcessors);
nNIKAL100_mExportSymbol(nNIKAL100_incrementDriverRefcount);
nNIKAL100_mExportSymbol(nNIKAL100_mallocContiguous);
nNIKAL100_mExportSymbol(nNIKAL100_pageUnlockUserPointer);
nNIKAL100_mExportSymbol(nNIKAL100_mapPhysicalToKernel);
nNIKAL100_mExportSymbol(nNIKAL230_mapPhysicalToKernel);
nNIKAL100_mExportSymbol(nNIKAL230_unmapPhysicalFromKernel);
nNIKAL100_mExportSymbol(nNIKAL100_waitForSingleUseEvent);
nNIKAL100_mExportSymbol(nNIKAL100_loadDriver);
nNIKAL100_mExportSymbol(nNIKAL100_releaseSingleUseEvent);
nNIKAL100_mExportSymbol(nNIKAL100_waitForSingleUseEventInterruptible);
nNIKAL100_mExportSymbol(nNIKAL100_unmapKernelFromUser);
nNIKAL100_mExportSymbol(nNIKAL100_replicateInboundIOControlBuffer);
nNIKAL100_mExportSymbol(nNIKAL110_releaseSpinLockInterrupt);
nNIKAL100_mExportSymbol(nNIKAL100_releaseSpinLockInterrupt);
nNIKAL100_mExportSymbol(nNIKAL100_mapKernelToUser);
nNIKAL100_mExportSymbol(nNIKAL100_unregisterPCIDriver);
nNIKAL100_mExportSymbol(nNIKAL200_unregisterPCIDriver);
nNIKAL100_mExportSymbol(nNIKAL1500_unregisterACPIDriver);
nNIKAL100_mExportSymbol(nNIKAL100_unregisterUSBDriver);
nNIKAL100_mExportSymbol(nNIKAL100_decrementDriverRefcount);
nNIKAL100_mExportSymbol(nNIKAL200_registerSimulatedDeviceDriver);
nNIKAL100_mExportSymbol(nNIKAL200_unregisterSimulatedDeviceDriver);
nNIKAL100_mExportSymbol(nNIKAL100_waitForSingleUseEventTimeout);
nNIKAL100_mExportSymbol(nNIKAL100_memoryWrite8);
nNIKAL100_mExportSymbol(nNIKAL100_usbAllocateURB);
nNIKAL100_mExportSymbol(nNIKAL100_usbFreeURB);
nNIKAL100_mExportSymbol(nNIKAL100_usbSubmitURB);
nNIKAL100_mExportSymbol(nNIKAL100_usbUnlinkURB);
nNIKAL100_mExportSymbol(nNIKAL100_usbGetDeviceDescriptor);
nNIKAL100_mExportSymbol(nNIKAL100_usbGetConfigurationDescriptor);
nNIKAL100_mExportSymbol(nNIKAL100_usbGetInterfaceDescriptor);
nNIKAL100_mExportSymbol(nNIKAL100_usbGetEndpointDescriptor);
nNIKAL100_mExportSymbol(nNIKAL100_usbSynchronousControlRequest);
nNIKAL100_mExportSymbol(nNIKAL100_usbSetConfiguration);
nNIKAL100_mExportSymbol(nNIKAL100_usbGetConfiguration);
nNIKAL100_mExportSymbol(nNIKAL100_usbSetInterface);
nNIKAL100_mExportSymbol(nNIKAL100_usbGetInterface);
nNIKAL100_mExportSymbol(nNIVersion_NIKAL_getVersionStringArray);
nNIKAL100_mExportSymbol(nNIKAL120_checkStackUsage);
nNIKAL100_mExportSymbol(nNIKAL170_getCallStack);
nNIKAL100_mExportSymbol(nNIKAL200_registerProperty);
nNIKAL100_mExportSymbol(nNIKAL200_unregisterProperty);
nNIKAL100_mExportSymbol(nNIKAL150_createMutex);
nNIKAL100_mExportSymbol(nNIKAL150_destroyMutex);
nNIKAL100_mExportSymbol(nNIKAL150_acquireMutex);
nNIKAL100_mExportSymbol(nNIKAL150_acquireMutexInterruptible);
nNIKAL100_mExportSymbol(nNIKAL150_acquireMutexZeroTimeout);
nNIKAL100_mExportSymbol(nNIKAL150_releaseMutex);
nNIKAL100_mExportSymbol(nNIKAL180_memCpy);
nNIKAL100_mExportSymbol(nNIKAL180_memMove);
nNIKAL100_mExportSymbol(nNIKAL240_memSet);
nNIKAL100_mExportSymbol(nNIKAL250_memCmp);
nNIKAL100_mExportSymbol(nNIKAL250_strLen);
nNIKAL100_mExportSymbol(nNIKAL250_strCmp);
nNIKAL100_mExportSymbol(nNIKAL250_strnCpy);
nNIKAL100_mExportSymbol(nNIKAL1700_sScanf);
nNIKAL100_mExportSymbol(nNIKAL1700_vsScanf);
nNIKAL100_mExportSymbol(nNIKAL200_registerDeviceInterface);
nNIKAL100_mExportSymbol(nNIKAL220_registerDeviceInterface);
nNIKAL100_mExportSymbol(nNIKAL230_registerDeviceInterface);
nNIKAL100_mExportSymbol(nNIKAL240_registerDeviceInterface);
nNIKAL100_mExportSymbol(nNIKAL200_unregisterDeviceInterface);
nNIKAL100_mExportSymbol(nNIKAL240_notifyUserspaceAddEvent);
nNIKAL100_mExportSymbol(nNIKAL220_tPageList_createFromAllocation);
nNIKAL100_mExportSymbol(nNIKAL220_tPageList_createFromUser);
nNIKAL100_mExportSymbol(nNIKAL220_tPageList_destroy);
nNIKAL100_mExportSymbol(nNIKAL220_tPageList_mapToKernel);
nNIKAL100_mExportSymbol(nNIKAL220_tPageList_unmapFromKernel);
nNIKAL100_mExportSymbol(nNIKAL220_tPageList_getPhysicalAddress);
nNIKAL100_mExportSymbol(nNIKAL220_tPageList_mapOnePageToKernel);
nNIKAL100_mExportSymbol(nNIKAL220_tPageList_unmapOnePageFromKernel);
nNIKAL100_mExportSymbol(nNIKAL220_tUserMemMap_mapPageList);
nNIKAL100_mExportSymbol(nNIKAL220_tUserMemMap_mapPhysicalAddress);
nNIKAL100_mExportSymbol(nNIKAL230_tUserMemMap_mapPhysicalAddress);
nNIKAL100_mExportSymbol(nNIKAL220_tUserMemMap_getVirtualAddress);
nNIKAL100_mExportSymbol(nNIKAL220_tUserMemMap_getSize);
nNIKAL100_mExportSymbol(nNIKAL220_tUserMemMap_getAccessMode);
nNIKAL100_mExportSymbol(nNIKAL200_createProcess);
nNIKAL100_mExportSymbol(nNIKAL1400_createProcessAndWait);
nNIKAL100_mExportSymbol(nNIKAL200_createSimulatedDevice);
nNIKAL100_mExportSymbol(nNIKAL200_destroySimulatedDevice);
nNIKAL100_mExportSymbol(nNIKAL200_getDeviceExtension);
nNIKAL100_mExportSymbol(nNIKAL200_getDeviceCreationContext);
nNIKAL100_mExportSymbol(nNIKAL220_pciSetDMAMask);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_createForPCI);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_map);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_numSegments);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_iterate);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_nextSegment);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_getSegmentAddress);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_getSegmentSize);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_prepareForDMA);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_completeDMA);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_unmap);
nNIKAL100_mExportSymbol(nNIKAL220_tSGL_destroy);
nNIKAL100_mExportSymbol(nNIKAL230_getCurrentEffectiveUserID);
nNIKAL100_mExportSymbol(nNIKAL240_getPCIDomain);
nNIKAL100_mExportSymbol(nNIKAL250_createKernelSocketBuffer);
nNIKAL100_mExportSymbol(nNIKAL250_releaseKernelSocketBuffer);
nNIKAL100_mExportSymbol(nNIKAL250_kernelSocketSend);
nNIKAL100_mExportSymbol(nNIKAL250_halt);
nNIKAL100_mExportSymbol(nNIKAL1500_registerSysctlInt_debugKALOnly);
nNIKAL100_mExportSymbol(nNIKAL1500_unregisterSysctl_debugKALOnly);


nNIKAL100_mExportSymbol(nNIKAL100_registerSerialCoreDriver);
nNIKAL100_mExportSymbol(nNIKAL100_unregisterSerialCoreDriver);
nNIKAL100_mExportSymbol(nNIKAL100_addOneSerialPort);
nNIKAL100_mExportSymbol(nNIKAL100_removeOneSerialPort);
nNIKAL100_mExportSymbol(nNIKAL100_insertCharToSerialRxBuffer);
nNIKAL100_mExportSymbol(nNIKAL100_pushSerialRxBufferToUser);
nNIKAL100_mExportSymbol(nNIKAL100_serialCoreTxWakeup);
nNIKAL100_mExportSymbol(nNIKAL100_serialCoreHandleModemChange);
nNIKAL100_mExportSymbol(nNIKAL100_serialCoreLockPort);
nNIKAL100_mExportSymbol(nNIKAL100_serialCoreUnlockPort);
nNIKAL100_mExportSymbol(nNIKAL1500_serialCoreIsPortLockedOnWiremodeConfig);
