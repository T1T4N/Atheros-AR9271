//
//  CleanAtherosUser.h
//  CleanAtheros
//
//  Created by Abel Espinosa on 2/8/15.
//  Copyright (c) 2015 Abel Espinosa. All rights reserved.
//
#include <IOKit/IOUserClient.h>
#include "CleanAtheros.h"

class com_gubawang_AR9271_CleanAtherosUser : public IOUserClient
{
    OSDeclareDefaultStructors(com_gubawang_AR9271_CleanAtherosUser)
private:
    task_t                          m_task;
    com_gubawang_AR9271_CleanAtheros* m_driver;
public:
    virtual bool init(OSDictionary *dictionary = 0);
    virtual void free(void);
    virtual IOService *probe(IOService *provider, SInt32 *score);
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
};