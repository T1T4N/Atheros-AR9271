//
//  CleanAtherosUser.h
//  CleanAtheros
//
//  Created by Abel Espinosa on 2/8/15.
//  Copyright (c) 2015 Abel Espinosa. All rights reserved.
//
#include <IOKit/IOLib.h>
#include "CleanAtheros.h"

// Define the driver's superclass.
#define super IOService
#define DEBUG_NAME "CleanAtheros"
#define VersionNumber   "1.0"

// This required macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(com_gubawang_AR9271_CleanAtheros, IOService)

bool com_gubawang_AR9271_CleanAtheros::init(OSDictionary *dict)
{
    bool result = super::init(dict);
    if (!result)
    {
        IOLog("CleanAtheros::Error Initializing\n");
        return false;
    }
    else
    {
        IOLog("CleanAtheros::Initializing\n");
    }
    return result;
}

void com_gubawang_AR9271_CleanAtheros::free(void)
{
    IOLog("CleanAtheros::Freeing\n");
    super::free();
}

IOService *com_gubawang_AR9271_CleanAtheros::probe(IOService *provider,
                                                SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog("CleanAtheros::Probing\n");
    return result;
}

bool com_gubawang_AR9271_CleanAtheros::start(IOService *provider)
{
    bool result = super::start(provider);
    if(!result)
    {
        IOLog("CleanAtheros::Starting Failed!\n");
        return false;
    }
    else
    {
        IOLog("CleanAtheros::Starting\n");
        fInterface = OSDynamicCast(IOUSBDevice, provider);
        if(!fInterface)
        {
            IOLog("CleanAtheros::Starting - Provider Invalid\n");
            return false;
        }
        if (!configureDevice())
        {
            IOLog("CleanAtheros::Starting - Device Configuration Failed\n");
            return false;
        }
        if (!allocateResources())
        {
            IOLog("CleanAtheros::Starting - Resource Allocation Failed\n");
            return false;
        }
        
        if (!initForPM(provider))
        {
            IOLog("CleanAtheros::Starting - Power Manager Initialization Failed\n");
            return false;
        }
        
        fInterface->retain();
        
        registerService();
        
        IOLog("CleanAtheros::Starting - Successful\n");
        IOLog(DEBUG_NAME ": Version number - %s\n", VersionNumber);
        return true;
    }
    return result;
}

void com_gubawang_AR9271_CleanAtheros::stop(IOService *provider)
{
    IOLog("CleanAtheros::Stopping\n");
    super::stop(provider);
}

bool com_gubawang_AR9271_CleanAtheros::configureDevice()
{
    bool	configOK = false;
    
    IOLog("CleanAtheros::Configuring Device\n");
    
    fInterfaceNumber = fInterface->GetDeviceClass();
    fSubClass = fInterface->GetDeviceSubClass();
    IOLog("CleanAtheros::Configuring Device - Subclass and interface number\n");
    
    IOLog("CleanAtheros::Configuring Device - Comm interface number\n");
    
    if (!getFunctionalDescriptors())
    {
        IOLog("CleanAtheros::Configuring Device - Getting Functional Descriptors Failed\n");
        configOK = false;
    }
    else
    {
        IOLog("CleanAtheros::Configuring Device - Getting Functional Descriptors OK\n");
        configOK = true;
    }
    
    if (!configOK)
    {
        IOLog("CleanAtheros::Configuring Device - Failed\n");
        return false;
    }
    
    return true;
    
}/* end configureDevice */

bool com_gubawang_AR9271_CleanAtheros::getFunctionalDescriptors()
{
    bool				gotDescriptors = false;
    UInt16				vers;
    UInt16				*chkVers;
    const FunctionalDescriptorHeader 	*funcDesc = NULL;
    HDRFunctionalDescriptor		*HDRFDesc;		// header functional descriptor
    WHCMFunctionalDescriptor		*WCMFDesc;		// whcm functional descriptor
    UnionFunctionalDescriptor		*UNNFDesc;		// union functional descriptor
    
    IOLog("CleanAtheros::Configuring Device - Getting Functional Descriptors\n");
    
    do
    {
        IOLog("CleanAtheros::No descriptors\n");
        funcDesc = (const FunctionalDescriptorHeader *)fInterface->FindNextDescriptor((void*)funcDesc, CS_INTERFACE);
        
        if (!funcDesc)
        {
            IOLog("CleanAtheros::No descriptors\n");
            gotDescriptors = true;				// We're done
        } else {
            IOLog("CleanAtheros::Descriptor found \n");
            switch (funcDesc->bDescriptorSubtype)
            {
                case Header_FunctionalDescriptor:
                    HDRFDesc = (HDRFunctionalDescriptor *)funcDesc;
                    IOLog("CleanAtheros::Configuring Device - Getting Header Functional Descriptor\n");
                    chkVers = (UInt16 *)&HDRFDesc->bcdCDC1;
                    vers = USBToHostWord(*chkVers);
                    if (vers > kUSBRel11)
                    {
                        IOLog("CleanAtheros::Configuring Device - Header descriptor version number is incorrect\n");
                    }
                    break;
                case Union_FunctionalDescriptor:
                    UNNFDesc = (UnionFunctionalDescriptor *)funcDesc;
                    IOLog("CleanAtheros::Configuring Device - Getting Union Functional Descriptor\n");
                    
                    if (UNNFDesc->bFunctionLength > sizeof(FunctionalDescriptorHeader))
                    {
                        if (fInterfaceNumber != UNNFDesc->bMasterInterface)
                        {
                            IOLog("CleanAtheros::Configuring Device - Master interface is incorrect\n");
                        }
                        else
                        {
                            fControlLen = UNNFDesc->bFunctionLength - sizeof(FunctionalDescriptorHeader);
                            fControlLen -= sizeof(UNNFDesc->bMasterInterface);	// Step over master as it's us and we've already checked it
                            fControlMap = (UInt8 *)IOMalloc(fControlLen);
                            bcopy(&UNNFDesc->bSlaveInterface, fControlMap, fControlLen);		// Just save them for now...
                            IOLog("CleanAtheros::Configuring Device - Map and length\n");
                        }
                    } else {
                        IOLog("CleanAtheros::Configuring Device - Union descriptor length error\n");
                    }
                    break;
                case WCM_FunctionalDescriptor:
                    WCMFDesc = (WHCMFunctionalDescriptor *)funcDesc;
                    IOLog("CleanAtheros::Configuring Device - WHCM Functional Descriptor\n");
                    
                    chkVers = (UInt16 *)&WCMFDesc->bcdCDC1;
                    vers = USBToHostWord(*chkVers);
                    if (vers > kUSBRel10)
                    {
                        IOLog("CleanAtheros::Configuring Device - WHCM descriptor version number is incorrect\n");
                    }
                    break;
                default:
                    IOLog("CleanAtheros::Configuring Device - unknown Functional Descriptor\n");
                    break;
            }
        }
    } while(!gotDescriptors);
    return true;
    
}/* end getFunctionalDescriptors */

bool com_gubawang_AR9271_CleanAtheros::allocateResources()
{
    IOLog("CleanAtheros::Configuring Device - Allocating Resources\n");
   
    // Open the interface
    
    if (!fInterface->open(this))
    {
        IOLog("CleanAtheros::Configuring Device - Opening Comm Interface Failed\n");
        return false;
    }
    
    return true;
    
}/* end allocateResources */

bool com_gubawang_AR9271_CleanAtheros::initForPM(IOService *provider)
{
    IOLog("CleanAtheros::Configuring Device - Initializing Power Manager\n");
    
    fPowerState = kCDCPowerOnState;				// init our power state to be 'on'
    PMinit();							// init power manager instance variables
    provider->joinPMtree(this);					// add us to the power management tree
    if (pm_vars != NULL)
    {
        
        // register ourselves with ourself as policy-maker
        
        registerPowerDriver(this, gOurPowerStates, kNumCDCStates);
        return true;
    } else {
        IOLog("CleanAtheros::Configuring Device - Initializing Power Manager Failed\n");
    }
    
    return false;
    
}/* end initForPM */

IOUSBInterface* com_gubawang_AR9271_CleanAtheros::FindNextInterface(IOUSBInterface* current,IOUSBFindInterfaceRequest* request)
{
    /*
     enum {
     kUSBCompositeClass          	= 0,
     kUSBCommClass               	= 2,		// Deprecated
     kUSBCommunicationClass			= 2,
     kUSBHubClass                	= 9,
     kUSBDataClass               	= 10,
     kUSBPersonalHealthcareClass		= 15,
     kUSBDiagnosticClass				= 220,
     kUSBWirelessControllerClass 	= 224,
     kUSBMiscellaneousClass			= 239,
     kUSBApplicationSpecificClass 	= 254,
     kUSBVendorSpecificClass     	= 255
     };
     */
    enum {
        kIOUSBVendorIDAppleComputer		= 0x05AC,
        kIOUSBVendorIDApple             = 0x05AC
    };

    
    IOUSBFindInterfaceRequest req;
    IOUSBInterface*           fCommInterface = NULL;
    req.bInterfaceClass =    kUSBVendorSpecificClass;
    req.bInterfaceSubClass = kUSBVendorSpecificClass;
    req.bInterfaceProtocol = kUSBVendorSpecificClass;
    req.bAlternateSetting =  kIOUSBFindInterfaceDontCare;
    
    //fCommInterface = fpDevice->FindNextInterface(NULL, &req);
    if (!fCommInterface)
    {
        // not found
    }
    
    
    
    return NULL;
}









