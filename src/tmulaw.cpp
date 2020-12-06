#include "mulaw.h"
#include <iostream>

typedef short byte;

int main() {

  for (int i=0; i<32768; i++) {
    byte mli = LinearToMuLawSample(i);
    short ih = MuLawToLinearSample(mli);
    byte mlmi = LinearToMuLawSample(-i);
    short mih = MuLawToLinearSample(mlmi);
    char mlistr[5]; sprintf(mlistr, "0x%02x",mli);
    char mlmistr[5]; sprintf(mlmistr, "0x%02x",mlmi);
    
    std::cout << "mulaw[" << i << "] == " << mlistr << " -> " << ih << "\t|\tmulaw[" << -i << "] == " << mlmistr << " -> " << mih << "\n";
  }

}
