// Written by Juan-Pablo Caceres
// Soundwire Group, 2008

//JamLink UDP Header:
/************************************************************************/
/* values for the UDP stream type                                       */
/* streamType is a 16-bit value at the head of each UDP stream          */
/* Its bit map is as follows:  (b15-msb)                                */
/* B15:reserved, B14:extended header, B13 Stereo, B12 not 16-bit        */
/* B11-B9: 0-48 Khz, 1-44 Khz, 2-32 Khz, 3-24 Khz,                      */
/*         4-22 Khz, 5-16 Khz, 6-11 Khz, 7-8 Khz                        */      
/* B8-0: Samples in packet                                              */
/************************************************************************/
#define ETX_RSVD (0<<15)
#define ETX_XTND (1<<14)
#define ETX_STEREO (1<<13)
#define ETX_MONO (0<<13)
#define ETX_16BIT (0<<12)
#define ETX_RATE_MASK(a) (a&(0x7<<9))
#define ETX_48KHZ (0<<9)
#define ETX_44KHZ (1<<9)
#define ETX_32KHZ (2<<9)
#define ETX_24KHZ (3<<9)
#define ETX_22KHZ (4<<9)
#define ETX_16KHZ (5<<9)
#define ETX_11KHZ (6<<9)
#define ETX_8KHZ  (7<<9)
#define ETX_SPP(a) (a&0x01FF) /* able to express up to 512 SPP */

/* Note that the extended header is likely to have 6 more bytes: */
/* 2 for the sequence number, and 4 for a timestamp */

//Instpired on Thinking in C++
//http://www.linuxtopia.org/online_books/programming_books/thinking_in_c++/Chapter03_045.html

// Display a unsigned short (2 bytes) in binary
void printBinary(const unsigned short val);
#include <iostream>

#define PR(STR, EXPR) \
  cout << STR; printBinary(EXPR); cout << endl;

void printBinary(const unsigned short val) {
  for(int i = 15; i >= 0; i--)
    if(val & (1 << i))
      std::cout << "1";
    else
          std::cout << "0";
}

