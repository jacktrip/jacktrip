#include "jamlink.h"

void printBinary(const unsigned short val)
{  
  for(int i = 15; i >= 0; i--)
    if(val & (1 << i))
      std::cout << "1";
    else
      std::cout << "0";
}



void printBinaryChar(const char* val) 
{
  for(int i = 7; i >= 0; i--)
    if(*val & (1 << i))
      std::cout << "1";
    else
      std::cout << "0";
}


//-------------------------------------------------------------------------------
/*! \brief Function to byteswap a char array
 *
 */
//-------------------------------------------------------------------------------
void byteSwap(char* bswp, int len) 
{
  char tmp;
  for (int i=0; i<(len/2); i++){
    tmp = bswp[2*i];
    bswp[2*i] = bswp[2*i+1];
    bswp[2*i+1] = tmp;
  }
}

