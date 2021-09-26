#include "../Includes/types.h"
#include "gdt.h"
#include"../CPU/Interrupts.h"
#include "../Drivers/Keyboard.h"
#include "../Drivers/Mouse.h"
#include "../Drivers/Driver.h"

static uint8_t cursory;
static uint8_t cursorx;
static bool isMouse = false;

void printf(char* str)
{
    static uint8_t x=0, y=0;
    uint8_t curx;
    uint8_t cury;

    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        cursorx = x;

        switch(str[i])
        {
            case '\n':
                y++;
                x = 0;
            break;
            case '\5':
                for(y = 0; y < 25; y++)
                    for(x = 0; x < 80; x++)
                        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';

                x = 0;
                y = 0;
            break;
            case '\3':
                for(x = cursorx - 0x02; x == cursorx +2 ; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | 'a';
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
            break;
        }
        

        if (x >= 80)
        {
            y++;
            x = 0;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';

            x = 0;
            y = 0;
        }
        
    }
}

void printfchar(char st)
{
    static uint8_t x=0, y=0;

    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

        switch(st)
        {
            case '\n':
                y++;
                x = 0;
            break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | st;
                x++;
            break;
        }
        

        if (x >= 80)
        {
            y++;
            x = 0;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';

            x = 0;
            y = 0;
        }
}

void printHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}


class PrintKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:

    MouseToConsole()
    {
    }

    /*
    virtual void OnMouseUp(uint8_t button)
    {
        if (button == 0x01)
            printf("you clicked");
        else if (button == 0x03)
            printf("hhh");
    }
    */

    virtual void OnActivate()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }

    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }

};


typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}



extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Welcome to SectorOS Monolithic kernel \n");

    GlobalDescriptorTable gdt;

    InterruptManager interrupts(0x20, &gdt);

    DriverManager drvmgr;
    printf("\nSYSMSG: Initializing Hardwares [Stage 1]...");
    PrintKeyboardEventHandler kbhandler;
    KeyboardDriver KeyboardDriver(&interrupts, &kbhandler);
    drvmgr.AddDriver(&KeyboardDriver);

    MouseToConsole msmgr;
    MouseDriver MouseDriver(&interrupts, &msmgr);
    drvmgr.AddDriver(&MouseDriver);

    printf("\nSYSMSG: Initializing Hardwares [Stage 2]...");
    drvmgr.activateall();

    printf("\nSYSMSG: Initializing Hardwares [Stage 3]...\n \n");
    interrupts.Activate();

    printf("$: ");
    while(1);
}