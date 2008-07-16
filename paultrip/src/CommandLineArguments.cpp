
#include "CommandLineArguments.h"

#include <iostream>
using namespace std;

void CommandLineArguments::printUsage()
{
  cout << "PaulTrip: A System for High-Quality Audio Network Performance" << endl;
  cout << "over the Internet" << endl;
  cout << "Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe." << endl;
  cout << "SoundWIRE group at CCRMA, Stanford University" << endl;
  cout << "" << endl;
  cout << "Usage: " << endl;
  cout << "--------------------------------------------" << endl;
  cout << " -h  --help                    Prints this help" << endl;
  cout << "" << endl;
}
