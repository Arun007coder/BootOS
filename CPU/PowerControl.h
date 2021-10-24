#ifndef __POWERCONTROL_H
#define __POWERCONTROL_H

#include "../Includes/types.h"
#include "../Drivers/IOPorts.h"

class PowerControl
{
public:
    //To reboot the computer
    void reboot();
    //To halt the CPU
    void halt();
    //If running in virtualBox. This function will Stop virtualBox
    void StopVirtualBox();
};

#endif