#include "jamlink.h"

void printBinary(const unsigned short val) {
  for(int i = 15; i >= 0; i--)
    if(val & (1 << i))
      std::cout << "1";
    else
      std::cout << "0";
}
