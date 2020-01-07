#ifndef __TESTRINGBUFFER__
#define __TESTRINGBUFFER__

#include "RingBuffer.h"
#include <QThread>
#include <iostream>

static RingBuffer rb(2,100);

class TestRingBufferWrite : public QThread
{
public:

    void run()
    {
        int8_t* writeSlot;
        writeSlot = new int8_t[2];
        writeSlot[0] = *"a";
        writeSlot[1] = *"b";
        while (true) {
            //std::cout << "writing BEFORE" << std::endl;
            rb.insertSlotBlocking(writeSlot);
            //std::cout << "writing AFTER" << std::endl;
        }
    }

};


class TestRingBufferRead : public QThread
{
public:

    void run()
    {
        int8_t* readSlot;
        readSlot = new int8_t[2];
        while (true) {
            //std::cout << "reading BEFORE" << std::endl;
            rb.readSlotBlocking(readSlot);
            //std::cout << "reading AFTER" << std::endl;
            //std::cout << *(readSlot) << std::endl;
            //std::cout << *(readSlot+1) << std::endl;
        }
    }
};

#endif
