//
//  CleanAtherosUser.h
//  CleanAtheros
//
//  Created by Abel Espinosa on 2/8/15.
//  Copyright (c) 2015 Abel Espinosa. All rights reserved.
//
#include <IOKit/IOService.h>
#include <IOKit/usb/IOUSBHostDevice.h>
#include <IOKit/usb/USB.h>

/*
 typedef struct {
 UInt16 bInterfaceClass;
 UInt16 bInterfaceSubClass;
 UInt16 bInterfaceProtocol;
 UInt16 bAlternateSetting;
 } IOUSBFindInterfaceRequest;
 */
typedef struct
{
    UInt8	bFunctionLength;
    UInt8 	bDescriptorType;
    UInt8 	bDescriptorSubtype;
} FunctionalDescriptorHeader;

typedef struct
{
    UInt8	bFunctionLength;
    UInt8 	bDescriptorType;
    UInt8 	bDescriptorSubtype;
    UInt8 	bcdCDC1;
    UInt8 	bcdCDC2;
} HDRFunctionalDescriptor;

typedef struct
{
    UInt8	bFunctionLength;
    UInt8 	bDescriptorType;
    UInt8 	bDescriptorSubtype;
    UInt8 	bcdCDC1;
    UInt8 	bcdCDC2;
} WHCMFunctionalDescriptor;

typedef struct
{
    UInt8	bFunctionLength;
    UInt8 	bDescriptorType;
    UInt8 	bDescriptorSubtype;
    UInt8 	bMasterInterface;
    UInt8	bSlaveInterface[];
} UnionFunctionalDescriptor;

enum
{
    CS_INTERFACE		= 0x24,
    Header_FunctionalDescriptor	= 0x00,
    Union_FunctionalDescriptor	= 0x06,
    WCM_FunctionalDescriptor	= 0x11,
    kCDCPowerOffState	= 0,
    kCDCPowerOnState	= 1,
    kNumCDCStates	= 2
};
// Globals

static IOPMPowerState gOurPowerStates[kNumCDCStates] =
{
    {1,0,0,0,0,0,0,0,0,0,0,0},
    {1,IOPMDeviceUsable,IOPMPowerOn,IOPMPowerOn,0,0,0,0,0,0,0,0}
};

class com_gubawang_AR9271_CleanAtheros : public IOService
{
    OSDeclareDefaultStructors(com_gubawang_AR9271_CleanAtheros)
private:
    UInt8			fInterfaceNumber;			// My interface number
    UInt8			fSubClass;				// Interface subclass
    UInt16			fControlLen;			// Subordinate interface length
    UInt8			*fControlMap;			// Subordinate interface numbers
    UInt8			fPowerState;				// Ordinal for power management
public:
    IOUSBHostDevice		*fInterface;
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    virtual IOService *probe(IOService *provider, SInt32 *score) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    bool 			configureDevice(void);
    bool			getFunctionalDescriptors(void);
    bool 			allocateResources(void);
    bool			initForPM(IOService *provider);
};
