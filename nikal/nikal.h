/*  This header file declares the NI Kernel Abstraction Layer for Linux

    Copyright 2001-2005,
    National Instruments Corporation.
    All rights reserved.

    originated:  19.mar.2001
*/

#ifndef     ___nikal_h___
#define     ___nikal_h___

#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <linux/timer.h>

/* ---------------------------------------------------------------------- */
/* KAL macros */
/* ---------------------------------------------------------------------- */

#define nNIKAL100_mInsertNamespacePaster(x,y) x ## y
#define nNIKAL100_mInsertNamespace(x,y)  nNIKAL100_mInsertNamespacePaster(x,y)

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------- */
/* KAL constants */
/* ---------------------------------------------------------------------- */

#define nNIKAL100_kPCIBARMax                    6
#define nNIKAL100_kPCIIOResourceFlagTypeIO   0x1
#define nNIKAL100_kPCIIOResourceFlagTypeMem  0x2
#define nNIKAL100_kPCIDontCare (~0)

#define nNIKAL1500_kACPIIDLength                9

#if ((defined(__arm__) || defined(__mips__)))
#define nNIKAL100_cc
#else
#define nNIKAL100_cc __attribute__((regparm(0)))
#endif

/* ---------------------------------------------------------------------- */
/* KAL status codes */
/* ---------------------------------------------------------------------- */

#define nNIKAL100_kStatusSuccess                    0
#define nNIKAL100_kStatusOffset                     -51000
#define nNIKAL100_kStatusValueConflict             (0    + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusIrrelevantAttribute       (-1   + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusBadDevice                 (-2   + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusBadSelector               (-3   + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusBadPointer                (-4   + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusBadMode                   (-6   + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusBadOffset                 (-7   + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusBadCount                  (-8   + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusInvalidParameter          (-9   + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusResourceNotAvailable      (-101 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusResourceNotInitialized    (-104 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusResourceInitialized       (-105 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusResourceBusy              (-106 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusSoftwareFault             (-150 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusHardwareFault             (-152 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusOSFault                   (-202 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusFeatureNotSupported       (-256 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusFeatureDisabled           (-265 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusDeviceNotFound            (-300 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusMemoryFull                (-352 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusMemoryPageLockFailed      (-353 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusTransferTimedOut          (-400 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusCommunicationsFault       (-401 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusTransferInProgress        (-403 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusTransferStopped           (-404 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusTransferAborted           (-405 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusPhysicalBufferEmpty       (-408 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusPhysicalBufferFull        (-409 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusSyncTimedOut              (-550 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusThreadCouldNotRun         (-600 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusWaitInterrupted           (-700 + nNIKAL100_kStatusOffset)
#define nNIKAL100_kStatusNoIoctlSpecified          (-800 + nNIKAL100_kStatusOffset)

/* ---------------------------------------------------------------------- */
/* KAL types */
/* ---------------------------------------------------------------------- */

#if ((defined(__GNUG__) || defined(__GNUC__)) && (defined(__i386__) || defined(__arm__) || defined(__mips__)))
   typedef char               nNIKAL100_tText;
   typedef unsigned char      nNIKAL100_tU8;
   typedef char               nNIKAL100_tI8;
   typedef unsigned short     nNIKAL100_tU16;
   typedef short              nNIKAL100_tI16;
   typedef unsigned long      nNIKAL100_tU32;
   typedef long               nNIKAL100_tI32;
   typedef unsigned long long nNIKAL100_tU64;
   typedef long long          nNIKAL100_tI64;
   typedef nNIKAL100_tU32     nNIKAL100_tUPtr;
   typedef nNIKAL100_tI32     nNIKAL100_tIPtr;
   typedef nNIKAL100_tI32     nNIKAL100_tStatus;
   typedef nNIKAL100_tU32     nNIKAL100_tBoolean;
   typedef nNIKAL100_tU32     nNIKAL100_tSizeType;
#elif ((defined(__GNUG__) || defined(__GNUC__)) && defined(__x86_64__))
   typedef char               nNIKAL100_tText;
   typedef unsigned char      nNIKAL100_tU8;
   typedef char               nNIKAL100_tI8;
   typedef unsigned short     nNIKAL100_tU16;
   typedef short              nNIKAL100_tI16;
   typedef unsigned int       nNIKAL100_tU32;
   typedef int                nNIKAL100_tI32;
   typedef unsigned long      nNIKAL100_tU64;
   typedef long               nNIKAL100_tI64;
   typedef nNIKAL100_tU64     nNIKAL100_tUPtr;
   typedef nNIKAL100_tI64     nNIKAL100_tIPtr;
   typedef nNIKAL100_tI32     nNIKAL100_tStatus;
   typedef nNIKAL100_tU32     nNIKAL100_tBoolean;
   typedef size_t             nNIKAL100_tSizeType;
#else
   #error ("NI-KAL standard types are not defined for this compiler or architecture")
#endif
#define nNIKAL100_kFalse 0
#define nNIKAL100_kTrue 1

/* ---------------------------------------------------------------------- */
/* Sparse static analysis defines                                         */
/* ---------------------------------------------------------------------- */
#ifndef __user
   #define __user
#endif
#ifndef __iomem
   #define __iomem
#endif

typedef void (*nNIKAL100_cc nNIKAL100_tDPCCallback)(void *context);

typedef nNIKAL100_tBoolean (*nNIKAL100_cc nNIKAL100_tInterruptCallback)(nNIKAL100_tU32 irq, void *context);
typedef struct
{
   nNIKAL100_tInterruptCallback func;
   void *context;
} nNIKAL100_tInterruptCallbackSpec;

typedef void (*nNIKAL100_cc nNIKAL100_tTimerCallback)( void * context);
typedef struct
{
   nNIKAL100_tTimerCallback func;
   void *context;
   struct timer_list timer;
} nNIKAL100_tTimerCallbackSpec;

typedef struct nNIKAL100_tListNode
{
   struct nNIKAL100_tListNode *prev;
   struct nNIKAL100_tListNode *next;
} nNIKAL100_tListNode;

typedef struct
{
   nNIKAL100_tU16 bus;
   nNIKAL100_tU16 slot;
   nNIKAL100_tU16 function;
   nNIKAL100_tU16 vendor;
   nNIKAL100_tU16 device;
   nNIKAL100_tU16 subSystemVendor;
   nNIKAL100_tU16 subSystemDevice;
} nNIKAL100_tPCIDeviceID;

typedef struct
{
   nNIKAL100_tUPtr start;
   nNIKAL100_tUPtr size;
   nNIKAL100_tU32 flags;
} nNIKAL100_tPCIIOResource;

/**
 * Clients of NI-KAL should not instantiate nNIKAL100_tPCIDeviceInfo.
 * One will be provided to the client, when appropriate, via the
 * nNIKAL100_tDeviceInfo struct.  See the comment above nNIKAL100_tDeviceInfo.
 **/
typedef struct
{
   nNIKAL100_tPCIDeviceID deviceID;
   nNIKAL100_tU16 revision;
   nNIKAL100_tU32 irqSize;
   nNIKAL100_tU32 irq;
   nNIKAL100_tU32 ioResourceSize;
   nNIKAL100_tPCIIOResource ioResource[nNIKAL100_kPCIBARMax];
} nNIKAL100_tPCIDeviceInfo;

typedef struct
{
   nNIKAL100_tU8 id[nNIKAL1500_kACPIIDLength];
} nNIKAL1500_tACPIDeviceID;

/**
 * Clients of NI-KAL should not instantiate nNIKAL1500_tACPIDeviceInfo.
 * One will be provided to the client, when appropriate, via the
 * nNIKAL100_tDeviceInfo struct.  See the comment above nNIKAL100_tDeviceInfo.
 **/
typedef struct
{
   nNIKAL1500_tACPIDeviceID deviceID;
} nNIKAL1500_tACPIDeviceInfo;

typedef struct
{
   nNIKAL100_tU16 vendor;
   nNIKAL100_tU16 product;
   nNIKAL100_tU16 revision;
   nNIKAL100_tU8  deviceClass;
   nNIKAL100_tU8  deviceSubClass;
   nNIKAL100_tU8  deviceProtocol;
   nNIKAL100_tU8  interfaceClass;
   nNIKAL100_tU8  interfaceSubClass;
   nNIKAL100_tU8  interfaceProtocol;
} nNIKAL100_tUSBDeviceID;

typedef enum
{
   nNIKAL100_kUSBTransferTypeControl,
   nNIKAL100_kUSBTransferTypeBulk,
   nNIKAL100_kUSBTransferTypeInterrupt,
   nNIKAL100_kUSBTransferTypeIsochronous
} nNIKAL100_tUSBTransferType;

/*
 * This structure is defined by the USB spec,
 * section 9.3.
 */
struct nNIKAL100_tUSBSetupPacket
{
   nNIKAL100_tU8  bmRequestType;
   nNIKAL100_tU8  bRequest;
   nNIKAL100_tU16 wValue;
   nNIKAL100_tU16 wIndex;
   nNIKAL100_tU16 wLength;
} __attribute__ ((packed));
typedef struct nNIKAL100_tUSBSetupPacket nNIKAL100_tUSBSetupPacket;

typedef struct
{
   nNIKAL100_tU32                   packetOffset;        /* [in]  The offset into the urb's 'buffer' to/from which this packet should be transferred */
   nNIKAL100_tU32                   packetLength;        /* [in]  The number of bytes that should be transferred to/from this packet */
   nNIKAL100_tU32                   packetTransferCount; /* [out] The number of bytes that were actually transferred to/from this packet */
   nNIKAL100_tStatus                ioStatus;            /* [out] The status of this packet */
} nNIKAL100_tUSBIsochronousPacketDescriptor;

typedef struct
{
   nNIKAL100_tUSBSetupPacket setupPacket;
   void *                    osWorkItem;
} nNIKAL100_tControlURBData;

typedef struct
{
   nNIKAL100_tU32                               startFrame;       /* [in/out] The USB [micro]frame on which this Isochronous transfer [should/did] start */
   nNIKAL100_tU32                               numberOfPackets;  /* [in]     The number of packets expected */
   nNIKAL100_tU32                               numberOfErrors;   /* [out]    The number of packets that had errors */
   nNIKAL100_tUSBIsochronousPacketDescriptor *  packetArray;      /* [in/out] The packets */
} nNIKAL100_tIsochronousURBData;

enum nNIKAL100_tUSBTransferFlags
{
   nNIKAL100_kUSBTransferFlagDirection             = 0x00000001, /* OUT == 0, IN == 1, just like in the USB endpoint descriptor */
   nNIKAL100_kUSBTransferFlagStartISOTransferASAP  = 0x00000002  /* Set this bit to indicate ISO transfer should occur at the next available frame */
};

struct nNIKAL100_tURB;

typedef void (*nNIKAL100_cc nNIKAL100_tUSBCallback)( struct nNIKAL100_tURB* urb );

typedef enum
{
   nNIKAL100_kSerialCoreError_None        = 0x00,
   nNIKAL100_kSerialCoreError_Break       = 0x01,
   nNIKAL100_kSerialCoreError_Frame       = 0x02,
   nNIKAL100_kSerialCoreError_Parity      = 0x03,
   nNIKAL100_kSerialCoreError_Overrun     = 0x04
} nNIKAL100_tSerialCoreError;

typedef enum
{
   nNIKAL100_kSerialDataSize_5Bit,
   nNIKAL100_kSerialDataSize_6Bit,
   nNIKAL100_kSerialDataSize_7Bit,
   nNIKAL100_kSerialDataSize_8Bit
} nNIKAL100_tSerialDataSize;

typedef enum
{
   nNIKAL100_kSerialParity_None,
   nNIKAL100_kSerialParity_Odd,
   nNIKAL100_kSerialParity_Even,
   nNIKAL100_kSerialParity_Mark,
   nNIKAL100_kSerialParity_Space
} nNIKAL100_tSerialParityMode;

typedef enum
{
   nNIKAL100_kSerialWireMode_2WireAuto,
   nNIKAL100_kSerialWireMode_2WireDtr,
   nNIKAL100_kSerialWireMode_2WireDtrEcho,
   nNIKAL100_kSerialWireMode_4Wire,
} nNIKAL100_tSerialRs485WireMode;

typedef enum
{
   nNIKAL200_kSerialTransceiverType_Rs232,
   nNIKAL200_kSerialTransceiverType_Rs485,
} nNIKAL200_tSerialTransceiverType;

/*
 * Modem Line Bit Field
 */
#define nNIKAL100_kSerialModemLine_DTR  (1 << 0) // Data Terminal Ready
#define nNIKAL100_kSerialModemLine_RTS  (1 << 1) // Request To Send
#define nNIKAL100_kSerialModemLine_ST   (1 << 2) // Secondary TXD
#define nNIKAL100_kSerialModemLine_SR   (1 << 3) // Secondary RXD
#define nNIKAL100_kSerialModemLine_CTS  (1 << 4) // Clear To Send
#define nNIKAL100_kSerialModemLine_DCD  (1 << 5) // Data Carrier Detect
#define nNIKAL100_kSerialModemLine_RI   (1 << 6) // Ring Indicator
#define nNIKAL100_kSerialModemLine_DSR  (1 << 7) // Data Set Ready
#define nNIKAL100_kSerialModemLine_LOOP (1 << 8) // Loopback Mode

typedef struct
{
   void *            owningModule;
   /*
    * The driverName and portPrefix pointers should be valid till we unregister
    * the driver. If these pointers are freed before you unregister the driver,
    * memory corruption could occur.
    */
   const nNIKAL100_tText * driverName;
   const nNIKAL100_tText * portPrefix;
   nNIKAL100_tI32          maxPorts;
} nNIKAL100_tSerialCoreDriverData;

typedef struct
{
   bool           hwFlowControlEnabled;
   bool           inputSwFlowControlEnabled;
   bool           outputSwFlowControlEnabled;
   unsigned char  startChar;
   unsigned char  stopChar;
} nNIKAL100_tSerialFlowControl;

typedef struct
{
   bool                          enableRx;
   nNIKAL100_tSerialDataSize     dataSize;
   nNIKAL100_tSerialParityMode   parityMode;
   bool                          inputParityCheckEnabled;
   bool                          ignoreFramingAndParityErrors;
   bool                          ignoreBreakErrors;
   bool                          enableTwoStopBits;
   nNIKAL100_tSerialFlowControl  flowControl;
} nNIKAL100_tSerialCorePortSettings;

typedef struct
{
   unsigned int      (*tx_empty)(void *);
   void              (*set_mctrl)(void *, nNIKAL100_tU32);
   nNIKAL100_tU32    (*get_mctrl)(void *);
   void              (*stop_tx)(void *);
   void              (*start_tx)(void *);
   void              (*send_xchar)(void *, char);
   void              (*stop_rx)(void *);
   void              (*enable_ms)(void *);
   void              (*break_ctl)(void *, int);
   nNIKAL100_tStatus (*startup)(void *);
   void              (*shutdown)(void *);
   void              (*flush_buffer)(void *);
   void              (*set_config)(void *, nNIKAL100_tSerialCorePortSettings *);
   bool              (*set_baud)(void *, unsigned int);
   void              (*pm)(void *, unsigned int, unsigned int);
   nNIKAL100_tStatus (*set_wake)(void *, unsigned int);
   const char *      (*type)(void *);
   void              (*release_port)(void *);
   nNIKAL100_tStatus (*request_port)(void *);
   void              (*config_port)(void *, int);
   nNIKAL100_tStatus (*verify_port)(void *, void *);
   nNIKAL100_tStatus (*ioctl)(void *, unsigned int, unsigned long);
   nNIKAL100_tStatus (*enable_transceivers)(void *);
   nNIKAL100_tStatus (*disable_transceivers)(void *);
   nNIKAL100_tStatus (*config_rs485)(void *, nNIKAL100_tSerialRs485WireMode *rs485);
} nNIKAL100_tSerialCoreOperations;

typedef struct
{
   nNIKAL100_tU32                          portType;
   nNIKAL100_tU32                          lineNumber;
   const nNIKAL100_tSerialCoreOperations * portOperations;
   nNIKAL200_tSerialTransceiverType        txvrType;
} nNIKAL100_tSerialCorePortData;

typedef struct
{
   char **              pBuffer;
   int *                pHead;
   int *                pTail;
   nNIKAL100_tSizeType  maxSize;
   unsigned int *       pTotalTxCount;
} nNIKAL100_tSerialCoreTxBuffer;

typedef struct nNIKAL100_tURB /* The tURB token needs to be here because tUSBCallback refers to it */
{
   void *                           osURB;
   void *                           osDeviceObject;
   nNIKAL100_tU32                   endpoint; /* endpoint address which includes direction, not just number */
   nNIKAL100_tU8 *                  buffer;
   nNIKAL100_tU32                   bufferSize;
   nNIKAL100_tU32                   transferCount;
   nNIKAL100_tU32                   transferFlags;
   nNIKAL100_tUSBCallback           callback;
   void *                           context;
   nNIKAL100_tStatus                ioStatus;
   nNIKAL100_tUSBTransferType       transferType;
   union {
      nNIKAL100_tControlURBData     control;
      nNIKAL100_tIsochronousURBData isochronous;
   };
} nNIKAL100_tURB;

typedef enum
{
   nNIKAL120_kUSBSignallingSpeedUnknown,
   nNIKAL120_kUSBSignallingSpeedLow,
   nNIKAL120_kUSBSignallingSpeedFull,
   nNIKAL120_kUSBSignallingSpeedHigh
} nNIKAL120_tUSBSignallingSpeed;

/**
 * Clients of NI-KAL should not instantiate nNIKAL100_tUSBDeviceInfo.
 * One will be provided to the client, when appropriate, via the
 * nNIKAL100_tDeviceInfo struct.  See the comment above nNIKAL100_tDeviceInfo.
 **/
typedef struct
{
   nNIKAL100_tUSBDeviceID           deviceID;
   void*                            parentDevice;
   nNIKAL100_tU8                    deviceAddress;
   nNIKAL100_tU8                    interfaceNumber;
   nNIKAL120_tUSBSignallingSpeed    connectionSpeed;
} nNIKAL100_tUSBDeviceInfo;

typedef enum
{
   nNIKAL100_kBusTypePCI,
   nNIKAL100_kBusTypeUSB,
   nNIKAL100_kBusType1394,
   nNIKAL200_kBusTypeSimulated,
   nNIKAL1500_kBusTypeACPI
} nNIKAL100_tBusType;

/**
 * Clients of NI-KAL should not instantiate nNIKAL100_tDeviceInfo, or any of
 * the structs that it contains.  A pointer to this struct, allocated
 * internally by NI-KAL, will be provided to the client via the addDevice
 * callback.  This restriction is in place so that NI-KAL may freely add
 * members to this struct in order to support new buses without compromising
 * backwards compatibility.
 **/
typedef struct
{
   void *osDevice;
   nNIKAL100_tBusType busType;
   nNIKAL100_tU32 version;
   union
   {
      nNIKAL100_tPCIDeviceInfo pci;
      nNIKAL100_tUSBDeviceInfo usb;
      nNIKAL1500_tACPIDeviceInfo acpi;
   };
} nNIKAL100_tDeviceInfo;

/**
 * Clients of NI-KAL should not instantiate nNIKAL100_tPCIBridgeInfo, or any of
 * the structs that it contains.  A pointer to this struct, allocated internally
 * by NI-KAL, will be provided to the client via the nNIKAL100_enumeratePCIBuses
 * callback.  This restriction is in place so that NI-KAL may freely add
 * members to this struct in order to support new bridge information without
 * compromising backwards compatibility.
 **/
typedef struct
{
   nNIKAL100_tPCIDeviceID deviceID;
   nNIKAL100_tU16 primary;
   nNIKAL100_tU16 secondary;
   nNIKAL100_tU16 subordinate;
   nNIKAL100_tU16 domain;
} nNIKAL100_tPCIBridgeInfo;

typedef void (*nNIKAL100_cc nNIKAL100_tPCIBridgeVisitor)(void *arg,
   const nNIKAL100_tPCIBridgeInfo *pciBus);

typedef struct nNIKAL220_tUserMemMap           nNIKAL220_tUserMemMap;
typedef enum {
   nNIKAL220_kUserMemMapNoCopyOnFork = (1<<0)
} nNIKAL220_tUserMemMapFlags;

typedef struct nNIKAL100_tDriver
{
   nNIKAL100_tListNode deviceListHead; /* reserved */
   void *module; /* NULL IFF the driver gets linked to KAL, otherwise: THIS_MODULE */
   const nNIKAL100_tText *name; /* name of the sub driver */
   nNIKAL100_tU32 number; /* minor number to dispatch to this driver */
   nNIKAL100_tStatus (*nNIKAL100_cc open)(void *filePtr, void **fileContext);
   void (*nNIKAL100_cc close)(void *fileContext);
   nNIKAL100_tStatus (*nNIKAL100_cc read)(void *fileContext, nNIKAL100_tText *buffer, nNIKAL100_tSizeType count, nNIKAL100_tIPtr *offset, nNIKAL100_tIPtr *bytesRead);
   nNIKAL100_tStatus (*nNIKAL100_cc write)(void *fileContext, const nNIKAL100_tText *buffer, nNIKAL100_tSizeType count, nNIKAL100_tIPtr *offset, nNIKAL100_tIPtr *bytesWritten);
   nNIKAL100_tStatus (*nNIKAL100_cc deviceIOControl)(void *fileContext, nNIKAL100_tU32 command, void *param);

   void (*nNIKAL100_cc unload)(struct nNIKAL100_tDriver *driver);
   nNIKAL100_tStatus (*nNIKAL100_cc addDevice)(const nNIKAL100_tDeviceInfo *pciDevice,
      void *deviceClassData, void **deviceExtension);
   void (*nNIKAL100_cc removeDevice)(void *deviceExtension);

   nNIKAL100_tStatus (*nNIKAL100_cc mapMemory)(void *fileContext, unsigned long type, nNIKAL220_tUserMemMap *vma, nNIKAL220_tUserMemMapFlags *flags, void **private_data);
   void (*nNIKAL100_cc unmapMemory)(void *private_data);

   void (*nNIKAL100_cc destroyDevice)(void *deviceExtension);
} nNIKAL100_tDriver;

typedef enum
{
   nNIKAL100_kAddressOriginUser,
   nNIKAL100_kAddressOriginKernel
} nNIKAL100_tAddressOrigin;

/* More defines and types for page lock&unlock operation only.
   hopefully pte is <= 64bit (8 bytes)
   We used to reserve 16bytes for pte, but I need to put more info into the
   token. Since we only support x86-32, and PAE only need 64bit, we really
   only need 8 bytes for PTE.
*/
#define nNIKAL100_kMemPteSize 8
typedef struct
{
   void *originalPointer;
   void *pageLockedPointer;
   void *pageLockDescriptor;
   nNIKAL100_tUPtr bufferSize;
   nNIKAL100_tU32 originalPageTableEntry[nNIKAL100_kMemPteSize/sizeof(nNIKAL100_tU32)];
   nNIKAL100_tUPtr vmaArraySize;
   void *vma;
   nNIKAL100_tAddressOrigin addressOrigin;
} nNIKAL100_tMemPageLockToken;

typedef enum
{
   nNIKAL100_kMemoryAddressTypeInvalid,
   nNIKAL100_kMemoryAddressTypePhysical,
   nNIKAL100_kMemoryAddressTypeVirtual
} nNIKAL100_tMemoryAddressType;

#define nNIKAL100_kSingleUseEventSize (sizeof(void*) * 16)
typedef nNIKAL100_tI32 (*nNIKAL100_cc nNIKAL100_tThreadFunction)(void *data);

typedef struct
{
   nNIKAL100_tThreadFunction threadMain;
   void* context;
   void* task;
   nNIKAL100_tUPtr childPID;
   void* thdDeadEvent[nNIKAL100_kSingleUseEventSize/sizeof(void*)];
} nNIKAL200_tThread;

typedef enum
{
   nNIKAL1400_kInterruptFlagShared                  = 0x00000001,
   nNIKAL1400_kInterruptFlagNoSoftIrqCall           = 0x00000002
} nNIKAL1400_tInterruptFlags;

/* ---------------------------------------------------------------------- */
/* KAL interface */
/* ---------------------------------------------------------------------- */
static inline nNIKAL100_cc nNIKAL100_tBoolean nNIKAL100_statusIsFatal(nNIKAL100_tStatus status)
{
   return (status < 0);
}
static inline nNIKAL100_cc nNIKAL100_tBoolean nNIKAL100_statusIsNonfatal(nNIKAL100_tStatus status)
{
   return (status >= 0);
}

/* argv[0] must contain the user executable relative file path under sbin */
nNIKAL100_cc nNIKAL100_tStatus nNIKAL200_createProcess(const char **argv, nNIKAL100_tBoolean wait);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL1400_createProcessAndWait(const char **argv, int *exitCode);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_replicateOutboundIOControlBuffer(void __user *to, const void *from, nNIKAL100_tI32 count);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_replicateInboundIOControlBuffer(void *to, const void __user *from, nNIKAL100_tI32 count);

nNIKAL100_cc void nNIKAL100_getKernelVersion(nNIKAL100_tU32 *major, nNIKAL100_tU32 *minor);
nNIKAL100_cc nNIKAL100_tBoolean nNIKAL200_isAddressableMemOver4G(void);

nNIKAL100_cc nNIKAL100_tI32 nNIKAL100_vsprintf(nNIKAL100_tText* buf, const nNIKAL100_tText* fmt, va_list arg);
nNIKAL100_cc nNIKAL100_tI32 nNIKAL100_vsnPrintf(nNIKAL100_tText* buf, nNIKAL100_tU32 size, const nNIKAL100_tText* fmt, va_list arg);
nNIKAL100_cc void nNIKAL100_printToDebugger(const nNIKAL100_tText* string);

typedef enum
{
   nNIKAL1500_kLogLevelEmergency, /* <== Most severe, syslogd will probably spit this out to all ttys */
   nNIKAL1500_kLogLevelAlert,
   nNIKAL1500_kLogLevelCritical,
   nNIKAL1500_kLogLevelError,
   nNIKAL1500_kLogLevelWarning,
   nNIKAL1500_kLogLevelNotice,
   nNIKAL1500_kLogLevelInfo,
   nNIKAL1500_kLogLevelDebug, /* <== Least severe */
} nNIKAL1500_tLogLevel;

nNIKAL100_cc void nNIKAL1500_printLineToDebugger(nNIKAL1500_tLogLevel logLevel, const char *message);

nNIKAL100_cc void *nNIKAL100_malloc(nNIKAL100_tUPtr size);
nNIKAL100_cc void *nNIKAL100_mallocContiguous(nNIKAL100_tUPtr size);
nNIKAL100_cc void *nNIKAL100_malloc32BitPhysicalContiguous(nNIKAL100_tUPtr size);
nNIKAL100_cc void nNIKAL100_free(void *pointer);
nNIKAL100_cc void nNIKAL100_free32BitPhysicalContiguous(void *pointer);

nNIKAL100_cc void *nNIKAL180_memCpy(void *dest, const void *src, size_t size);
nNIKAL100_cc void *nNIKAL180_memMove(void *dest, const void *src, size_t size);
nNIKAL100_cc void *nNIKAL240_memSet(void *buffer, nNIKAL100_tU8 data, size_t size);
nNIKAL100_cc int   nNIKAL250_memCmp(const void *buffer1, const void* buffer2, size_t count);

nNIKAL100_cc size_t nNIKAL250_strLen(const char *str);
nNIKAL100_cc int    nNIKAL250_strCmp(const char *str1, const char *str2);
nNIKAL100_cc char * nNIKAL250_strnCpy(char *dest, const char *src, size_t count);
nNIKAL100_cc int    nNIKAL1700_sScanf(const char *str, const char *format, ...);
nNIKAL100_cc int    nNIKAL1700_vsScanf(const char *str, const char *format, va_list ap);

nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getPageSize(void);
nNIKAL100_cc nNIKAL100_tUPtr nNIKAL100_getPhysicalMemorySize(void);
nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getNumberOfActiveProcessors(void);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_unmapKernelFromUser (nNIKAL100_tUPtr targetAddr,
   nNIKAL100_tUPtr size, const void *osData);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_mapKernelToUser (void * const file, nNIKAL100_tUPtr sourceAddr,
   nNIKAL100_tUPtr size, nNIKAL100_tMemoryAddressType type, nNIKAL100_tUPtr *targetAddr, void **osData);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pageLockUserPointer(void __user * inPointer, const nNIKAL100_tUPtr inSize,
   void ** outPageLockedPointer, nNIKAL100_tMemPageLockToken *pageLockToken);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pageUnlockUserPointer(nNIKAL100_tMemPageLockToken * pageLockPtr);

nNIKAL100_cc void __iomem * nNIKAL100_mapPhysicalToKernel (nNIKAL100_tUPtr physicalAddress,
   nNIKAL100_tUPtr byteCount);
nNIKAL100_cc void nNIKAL100_unmapPhysicalFromKernel (void __iomem * memoryArea);

nNIKAL100_cc nNIKAL100_tUPtr nNIKAL100_getPhysicalAddress(const void *ptr, nNIKAL100_tAddressOrigin addressOrigin);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_loadDriver(const nNIKAL100_tText* moduleName);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_incrementDriverRefcount(void* driverHandle);
nNIKAL100_cc void nNIKAL100_decrementDriverRefcount(void* driverHandle);

nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getTimerInterval (void);
nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_sleepTimeoutInterruptible(
   nNIKAL100_tU32 timeToSleepInNanoseconds);
nNIKAL100_cc void nNIKAL100_sleepTimeout(nNIKAL100_tU32 timeToSleepInNanoseconds);
nNIKAL100_cc void nNIKAL250_sleepTimeout(nNIKAL100_tU32 timeToSleepInMicroseconds);
nNIKAL100_cc void nNIKAL200_yield(void);
nNIKAL100_cc nNIKAL100_tU64 nNIKAL1_getMonotonicCounter (void);
nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getTimerCount (void);
nNIKAL100_cc nNIKAL100_tU64 nNIKAL110_getTimerCount (void);
nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getTimeOfDayInterval (void);
nNIKAL100_cc void nNIKAL100_getTimeOfDay (nNIKAL100_tU32 *systemSeconds,
   nNIKAL100_tU32 *systemMicroseconds);

nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getThreadID( void );
nNIKAL100_cc void *nNIKAL100_getThreadHandle( void );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_createSystemThread( void *threadContext,
   nNIKAL100_tThreadFunction threadMain, nNIKAL100_tUPtr *childPID );

nNIKAL100_cc nNIKAL100_tStatus nNIKAL250_createNamedThread( nNIKAL200_tThread * threadParam, char const * threadName );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL200_createThread( nNIKAL200_tThread * threadParam );
nNIKAL100_cc void nNIKAL200_joinThread( nNIKAL200_tThread *thread );

nNIKAL100_cc void *nNIKAL100_createSemaphore( nNIKAL100_tI32 initVal );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_destroySemaphore( void * sema );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_acquireSemaphore( void * sema );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_acquireSemaphoreInterruptible( void * sema );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_acquireSemaphoreZeroTimeout( void * sema );
nNIKAL100_cc nNIKAL100_tI32 nNIKAL100_releaseSemaphore( void * sema );
nNIKAL100_cc void *nNIKAL100_createSpinLock( void );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_destroySpinLock( void *spinlock );
nNIKAL100_cc nNIKAL100_tUPtr nNIKAL110_acquireSpinLockInterrupt( void *spinlock );
nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_acquireSpinLockInterrupt( void *spinlock );
nNIKAL100_cc void nNIKAL110_releaseSpinLockInterrupt( void *spinlock, nNIKAL100_tUPtr flags );
nNIKAL100_cc void nNIKAL100_releaseSpinLockInterrupt( void *spinlock, nNIKAL100_tU32 flags );
nNIKAL100_cc void nNIKAL100_acquireSpinLockDPC( void * spinlock );
nNIKAL100_cc void nNIKAL100_releaseSpinLockDPC( void *spinlock );
nNIKAL100_cc void *nNIKAL150_createMutex( void );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL150_destroyMutex( void * mutex );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL150_acquireMutex( void * mutex );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL150_acquireMutexInterruptible( void * mutex );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL150_acquireMutexZeroTimeout( void * mutex );
nNIKAL100_cc void nNIKAL150_releaseMutex( void * mutex );

nNIKAL100_cc void nNIKAL100_initializeSingleUseEvent( void *event );
nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_getSingleUseEventSize( void );
nNIKAL100_cc void nNIKAL100_releaseSingleUseEvent( void *event );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_waitForSingleUseEvent( void *event );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_waitForSingleUseEventInterruptible( void *event );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_waitForSingleUseEventTimeout( void *event,
   nNIKAL100_tU32 timeout, nNIKAL100_tTimerCallbackSpec *timerCallbackPkg );
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_waitForSingleUseEventTimeoutInterruptible ( void *event,
   nNIKAL100_tU32 timeout, nNIKAL100_tTimerCallbackSpec *timerCallbackPkg );

nNIKAL100_cc void *nNIKAL100_registerUSBDriver(nNIKAL100_tDriver *driver,
   const nNIKAL100_tText *name, void *usbDevIdTable);
nNIKAL100_cc void nNIKAL100_unregisterUSBDriver(void *usbDriverData, const nNIKAL100_tText ** name,
   void **usbDevIdTable);

nNIKAL100_cc void *nNIKAL100_createUSBDeviceIDTable(nNIKAL100_tU32 size);
nNIKAL100_cc void nNIKAL100_setUSBDeviceIDTableElement(void *usbDevIdTable,
   nNIKAL100_tU32 vendor, nNIKAL100_tU32 product, void *privateData);
nNIKAL100_cc void nNIKAL100_destroyUSBDeviceIDTable(void *usbDevIdTable);

nNIKAL100_cc nNIKAL100_tURB* nNIKAL100_usbAllocateURB(nNIKAL100_tU32 numISODescriptors);
nNIKAL100_cc void nNIKAL100_usbFreeURB(nNIKAL100_tURB* urb);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbSubmitURB(nNIKAL100_tURB* urb);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbUnlinkURB(nNIKAL100_tURB* urb);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbSynchronousControlRequest(void* osDevice,
   nNIKAL100_tUSBSetupPacket* setupPacket, void* data);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetDeviceDescriptor(void* osDevice, void* buffer, nNIKAL100_tU32 size);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetConfigurationDescriptor(void* osDevice, nNIKAL100_tU8 configurationValue, void* buffer, nNIKAL100_tU32 size);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetInterfaceDescriptor(void* osDevice, nNIKAL100_tU8 configurationValue,
   nNIKAL100_tU8 interfaceNumber, nNIKAL100_tU8 altSettingNumber, void* buffer, nNIKAL100_tU32 size);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetEndpointDescriptor(void* osDevice, nNIKAL100_tU8 configurationValue,
   nNIKAL100_tU8 interfaceNumber, nNIKAL100_tU8 altSettingNumber, nNIKAL100_tU8 endpointNumber, void* buffer, nNIKAL100_tU32 size);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbSetConfiguration(void* osDevice, nNIKAL100_tU8  configurationValue);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetConfiguration(void* osDevice, nNIKAL100_tU8* configurationValuePtr);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbSetInterface(void* osDevice, nNIKAL100_tU8 interfaceNumber, nNIKAL100_tU8  alternateSetting);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_usbGetInterface(void* osDevice, nNIKAL100_tU8 interfaceNumber, nNIKAL100_tU8* alternateSettingPtr);

nNIKAL100_cc void *nNIKAL100_registerPCIDriver(nNIKAL100_tDriver *driver,
   const nNIKAL100_tText *name, void *pciDevIdTable);
nNIKAL100_cc void nNIKAL100_unregisterPCIDriver(void *pciDriverData, const nNIKAL100_tText ** name,
   void **pciDevIdTable);

nNIKAL100_cc void *nNIKAL200_registerPCIDriver(void *driver);
nNIKAL100_cc void nNIKAL200_unregisterPCIDriver(void *pciDriverData);

nNIKAL100_cc void *nNIKAL100_createPCIDeviceIDTable(nNIKAL100_tU32 size);
nNIKAL100_cc void nNIKAL100_setPCIDeviceIDTableElement(void *pciDevIdTable,
   nNIKAL100_tU32 vendor, nNIKAL100_tU32 device, nNIKAL100_tU32 subsysVendor,
   nNIKAL100_tU32 subsysDevice, nNIKAL100_tU32 devClass, nNIKAL100_tU32 devClassMask,
   void *privateData);
nNIKAL100_cc void nNIKAL100_destroyPCIDeviceIDTable(void *pciDevIdTable);

nNIKAL100_cc void nNIKAL100_enumeratePCIBuses(nNIKAL100_tPCIBridgeVisitor visitor,
                                    void *visitorArgument,
                                    void *listToEnumerate);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigWrite8 (const void * osDevObject,
   const nNIKAL100_tU32 address, const nNIKAL100_tU8 data);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigWrite16 (const void * osDevObject,
   const nNIKAL100_tU32 address, const nNIKAL100_tU16 data);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigWrite32 (const void * osDevObject,
   const nNIKAL100_tU32 address, const nNIKAL100_tU32 data);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigRead8 (const void * osDevObject,
   const nNIKAL100_tU32 address, nNIKAL100_tU8 *data);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigRead16 (const void * osDevObject,
   const nNIKAL100_tU32 address, nNIKAL100_tU16 *data);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_pciConfigRead32 (const void * osDevObject,
   const nNIKAL100_tU32 address, nNIKAL100_tU32 *data);

nNIKAL100_cc void nNIKAL100_ioWrite8 ( const nNIKAL100_tUPtr address,
   const nNIKAL100_tU8 data);
nNIKAL100_cc void nNIKAL100_ioWrite16 ( const nNIKAL100_tUPtr address,
   const nNIKAL100_tU16 data);
nNIKAL100_cc void nNIKAL100_ioWrite32 ( const nNIKAL100_tUPtr address,
   const nNIKAL100_tU32 data);

nNIKAL100_cc nNIKAL100_tU8 nNIKAL100_ioRead8 ( const nNIKAL100_tUPtr address);
nNIKAL100_cc nNIKAL100_tU16 nNIKAL100_ioRead16 ( const nNIKAL100_tUPtr address);
nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_ioRead32 ( const nNIKAL100_tUPtr address);

nNIKAL100_cc void nNIKAL100_memoryWrite8 ( const nNIKAL100_tUPtr address,
   const nNIKAL100_tU8 data);
nNIKAL100_cc void nNIKAL100_memoryWrite16 ( const nNIKAL100_tUPtr address,
   const nNIKAL100_tU16 data);
nNIKAL100_cc void nNIKAL100_memoryWrite32 ( const nNIKAL100_tUPtr address,
   const nNIKAL100_tU32 data);

nNIKAL100_cc nNIKAL100_tU8 nNIKAL100_memoryRead8 ( const nNIKAL100_tUPtr address);
nNIKAL100_cc nNIKAL100_tU16 nNIKAL100_memoryRead16 ( const nNIKAL100_tUPtr address);
nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_memoryRead32 ( const nNIKAL100_tUPtr address);

nNIKAL100_cc nNIKAL100_tI32 nNIKAL240_getPCIDomain(const void * osDevObject);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL230_pciRequestMSI(nNIKAL100_tDeviceInfo *devInfo, nNIKAL100_tU32 numMessages);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL1700_pciReleaseMSI(nNIKAL100_tDeviceInfo *devInfo);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_reserveInterrupt (nNIKAL100_tU32 irq,
   void *context, const nNIKAL100_tText *deviceName);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL1400_reserveInterrupt (nNIKAL100_tU32 irq,
   void *context, const nNIKAL100_tText *deviceName,
   nNIKAL100_tU32 flags);
nNIKAL100_cc void nNIKAL100_relinquishInterrupt (nNIKAL100_tU32 irq, void *context);
nNIKAL100_cc void * nNIKAL100_createDPC (nNIKAL100_tDPCCallback dpcCallback);
nNIKAL100_cc void nNIKAL100_destroyDPC (void * dpc);
nNIKAL100_cc void nNIKAL100_scheduleDPC (void * dpc, void *context);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL100_registerDriver(nNIKAL100_tDriver *driver);
nNIKAL100_cc void nNIKAL100_unregisterDriver(nNIKAL100_tDriver *driver);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL200_registerDriver(nNIKAL100_tDriver *driver);
nNIKAL100_cc void nNIKAL200_unregisterDriver(nNIKAL100_tDriver *driver);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL200_registerSimulatedDeviceDriver(nNIKAL100_tDriver *driver,
                                                                       size_t numDevices);
nNIKAL100_cc void nNIKAL200_unregisterSimulatedDeviceDriver(nNIKAL100_tDriver *driver);

nNIKAL100_cc const nNIKAL100_tText **nNIVersion_NIKAL_getVersionStringArray(void);

nNIKAL100_cc nNIKAL100_tUPtr nNIKAL120_checkStackUsage(void);
nNIKAL100_cc nNIKAL100_tU32 nNIKAL170_getCallStack(nNIKAL100_tU32 callingLevel, nNIKAL100_tU32 startLevel, void **retAddrBuffer);

/* nNIKAL220_[create/destroy]SimulatedDevice for a driver are NOT thread safe! */
nNIKAL100_cc void* nNIKAL200_createSimulatedDevice(nNIKAL100_tDriver *driver, const char* devName, void* creationContext);
nNIKAL100_cc void nNIKAL200_destroySimulatedDevice(nNIKAL100_tDriver *driver, void *kalDevice);

/* This function is provided for use by the processmodule.sh codegen */
nNIKAL100_cc int nNIKAL1500_helperACPIProbe(void *dev, nNIKAL100_tDriver *driver);

nNIKAL100_cc void *nNIKAL1500_registerACPIDriver(void* inAcpiDriver, void *acpiProbe);
nNIKAL100_cc void nNIKAL1500_unregisterACPIDriver(void *acpiDriverData);
nNIKAL100_cc int nNIKAL1500_callACPIMethodGetInteger(const void *osDevObject,
                                                    const char *method,
                                                    nNIKAL100_tU64 *value);

nNIKAL100_cc int nNIKAL1500_callACPIMethod(const void *osDevObject,
                                           const char *method,
                                           nNIKAL100_tU8 *outbuf, nNIKAL100_tU32 size);

nNIKAL100_cc void* nNIKAL200_getDeviceExtension(void* kalDevice);
nNIKAL100_cc void* nNIKAL200_getDeviceCreationContext(void* kalDevice);

/* nNIKAL200_[un]registerDeviceInterface for one device are NOT thread safe! */
nNIKAL100_cc void *nNIKAL200_registerDeviceInterface(nNIKAL100_tDriver *driver, void *device, const char *name, size_t interfaceNum, void *interface);
nNIKAL100_cc void *nNIKAL220_registerDeviceInterface(nNIKAL100_tDriver *driver, void *device, const char *name, size_t interfaceNum, void *interface);
nNIKAL100_cc void *nNIKAL230_registerDeviceInterface(nNIKAL100_tDriver *driver, void *device, const char *name, size_t interfaceNum, void *interface);
nNIKAL100_cc void *nNIKAL240_registerDeviceInterface(nNIKAL100_tDriver *driver, void *device, const char *name, size_t interfaceNum, void *interface);
nNIKAL100_cc void nNIKAL200_unregisterDeviceInterface(void *interface);

nNIKAL100_cc void nNIKAL240_notifyUserspaceAddEvent(void *interface);

typedef enum {
   nNIKAL200_tPropertyClassDevice,
   nNIKAL200_tPropertyClassBus,
   nNIKAL200_tPropertyClassDeviceInterface
} nNIKAL200_tPropertyClass;

#define nNIKAL200_kDevicePropertiesDir           "deviceProperties"
#define nNIKAL200_kBusPropertiesDir              "busProperties"
#define nNIKAL200_kDeviceInterfacesDir           "deviceInterfaces"
#define nNIKAL200_kDeviceInterfacesPropertiesDir "properties"
#define nNIKAL200_kDeviceInterfacesPathFile      "interfacePath"

nNIKAL100_cc void *nNIKAL200_registerProperty(nNIKAL200_tPropertyClass propClass, void *context, const char *name, const void *content, size_t length);
nNIKAL100_cc void nNIKAL200_unregisterProperty(void *handle);

#define nNIKAL220_kPageListContiguous       (1UL<<0)
#define nNIKAL220_kPageList32BitAddressable (1UL<<1)
#define nNIKAL220_kPageListZeroed           (1UL<<2)
#define nNIKAL220_kPageListAccessModeRead   (1UL<<3)
#define nNIKAL220_kPageListAccessModeWrite  (1UL<<4)
typedef struct nNIKAL220_tPageList nNIKAL220_tPageList;

nNIKAL100_cc nNIKAL220_tPageList *nNIKAL220_tPageList_createFromAllocation(size_t pages, unsigned long flags);
nNIKAL100_cc nNIKAL220_tPageList *nNIKAL220_tPageList_createFromUser(void __user *pointer,
                                                                     size_t pages,
                                                                     unsigned long write);
nNIKAL100_cc void                 nNIKAL220_tPageList_destroy(nNIKAL220_tPageList *list);

nNIKAL100_cc void *nNIKAL220_tPageList_mapToKernel    (nNIKAL220_tPageList *list, size_t offsetInPages, size_t sizeInPages);
nNIKAL100_cc void  nNIKAL220_tPageList_unmapFromKernel(nNIKAL220_tPageList *list, void *pointer);
nNIKAL100_cc void *nNIKAL220_tPageList_mapOnePageToKernel(nNIKAL220_tPageList *list, size_t pageIndex);
nNIKAL100_cc void  nNIKAL220_tPageList_unmapOnePageFromKernel(nNIKAL220_tPageList *list, size_t pageIndex);

nNIKAL100_cc void   nNIKAL220_tPageList_getPhysicalAddress(const nNIKAL220_tPageList *list,
                                                           size_t offset,
                                                           nNIKAL100_tU64 *address,
                                                           size_t *size);

nNIKAL100_cc nNIKAL100_tStatus nNIKAL220_tUserMemMap_mapPageList(nNIKAL220_tUserMemMap *vma,
                                                          nNIKAL220_tPageList *list,
                                                          size_t offsetInPages,
                                                          size_t sizeInPages);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL220_tUserMemMap_mapPhysicalAddress(nNIKAL220_tUserMemMap *vma,
                                                                 nNIKAL100_tU64 address,
                                                                 size_t size);

#define nNIKAL230_kPhysMemFlagDefault       0UL
#define nNIKAL230_kPhysMemFlagWriteCombined (1UL << 0)
#define nNIKAL1750_kPhysMemFlagCache        (1UL << 1)
nNIKAL100_cc nNIKAL100_tStatus nNIKAL230_tUserMemMap_mapPhysicalAddress(nNIKAL220_tUserMemMap *vma,
                                                                 nNIKAL100_tU64 address,
                                                                 size_t size,
                                                                 unsigned long flags);
nNIKAL100_cc void __iomem * nNIKAL230_mapPhysicalToKernel (nNIKAL100_tU64 physicalAddress,
                                                           size_t size,
                                                           unsigned long flags);
nNIKAL100_cc void nNIKAL230_unmapPhysicalFromKernel (void __iomem * memoryArea, unsigned long flags);

nNIKAL100_cc void*    nNIKAL220_tUserMemMap_getVirtualAddress(nNIKAL220_tUserMemMap *vma);
nNIKAL100_cc size_t   nNIKAL220_tUserMemMap_getSize(nNIKAL220_tUserMemMap *vma);

#define nNIKAL220_kUserMemMapAccessModeRead  (1UL<<0)
#define nNIKAL220_kUserMemMapAccessModeWrite (1UL<<1)
#define nNIKAL220_kUserMemMapAccessModeExec  (1UL<<2)
nNIKAL100_cc nNIKAL100_tU32 nNIKAL220_tUserMemMap_getAccessMode(nNIKAL220_tUserMemMap *vma);

/* NI-KAL DMA abstraction */

typedef enum
{
   nNIKAL220_kDMADirectionHostToDevice,
   nNIKAL220_kDMADirectionDeviceToHost
} nNIKAL220_tDMADirection;

typedef struct nNIKAL220_tSGL nNIKAL220_tSGL;

nNIKAL100_cc nNIKAL100_tStatus nNIKAL220_pciSetDMAMask(void *osDevice, nNIKAL100_tU64 mask);
nNIKAL100_cc nNIKAL220_tSGL *nNIKAL220_tSGL_createForPCI(void *osDevice, size_t maxPages);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL220_tSGL_map(nNIKAL220_tSGL *sgl, nNIKAL220_tPageList *pageList, size_t offsetInBytes, size_t sizeInBytes, nNIKAL220_tDMADirection dir);
nNIKAL100_cc nNIKAL100_tU32 nNIKAL220_tSGL_numSegments(nNIKAL220_tSGL *sgl);
nNIKAL100_cc void nNIKAL220_tSGL_iterate(nNIKAL220_tSGL *sgl);
nNIKAL100_cc void nNIKAL220_tSGL_nextSegment(nNIKAL220_tSGL *sgl);
nNIKAL100_cc nNIKAL100_tU64 nNIKAL220_tSGL_getSegmentAddress(nNIKAL220_tSGL *sgl);
nNIKAL100_cc size_t nNIKAL220_tSGL_getSegmentSize(nNIKAL220_tSGL *sgl);
nNIKAL100_cc void nNIKAL220_tSGL_prepareForDMA(nNIKAL220_tSGL *sgl);
nNIKAL100_cc void nNIKAL220_tSGL_completeDMA(nNIKAL220_tSGL *sgl);
nNIKAL100_cc void nNIKAL220_tSGL_unmap(nNIKAL220_tSGL *sgl);
nNIKAL100_cc void nNIKAL220_tSGL_destroy(nNIKAL220_tSGL *sgl);

nNIKAL100_cc nNIKAL100_tU32 nNIKAL230_getCurrentEffectiveUserID(void);

nNIKAL100_cc void* nNIKAL250_createKernelSocketBuffer(size_t sizeInBytes);
nNIKAL100_cc void nNIKAL250_releaseKernelSocketBuffer(void *socketBuffer);
nNIKAL100_cc nNIKAL100_tStatus nNIKAL250_kernelSocketSend(void *socketBuffer,
                                                          nNIKAL100_tU32 pid,
                                                          nNIKAL100_tU16 attribute,
                                                          const void *message,
                                                          size_t sizeInBytes);

/* NI-KAL Serial Core functionality */
nNIKAL100_cc void *nNIKAL100_registerSerialCoreDriver(nNIKAL100_tSerialCoreDriverData *driverData);
nNIKAL100_cc void nNIKAL100_unregisterSerialCoreDriver(void *driver);
nNIKAL100_cc void *nNIKAL100_addOneSerialPort(void *driver, nNIKAL100_tSerialCorePortData *portData, nNIKAL100_tSerialCoreTxBuffer *txBuffer);
nNIKAL100_cc void nNIKAL100_removeOneSerialPort(void *driver, void *port);
nNIKAL100_cc void nNIKAL100_insertCharToSerialRxBuffer(void *port, char ch, nNIKAL100_tSerialCoreError errorFlag, bool ignore);
nNIKAL100_cc void nNIKAL100_pushSerialRxBufferToUser(void *port);
nNIKAL100_cc void nNIKAL100_serialCoreTxWakeup(void *port);
nNIKAL100_cc void nNIKAL100_serialCoreHandleModemChange(void *port, unsigned int msr);
nNIKAL100_cc nNIKAL100_tU32 nNIKAL100_serialCoreLockPort(void *port);
nNIKAL100_cc void nNIKAL100_serialCoreUnlockPort(void *port, nNIKAL100_tU32 flags);
nNIKAL100_cc nNIKAL100_tBoolean nNIKAL1500_serialCoreIsPortLockedOnWiremodeConfig(void);

nNIKAL100_cc void nNIKAL250_halt(const char *component, const char *file, int line, const char *message);
nNIKAL100_cc void *nNIKAL1500_registerSysctlInt_debugKALOnly(const char *rootName, const char *sysctlName, int *sysctlVal, unsigned short accessMode);
nNIKAL100_cc void nNIKAL1500_unregisterSysctl_debugKALOnly(void *sysctlObj);

#ifdef __cplusplus
}
#endif

#endif
