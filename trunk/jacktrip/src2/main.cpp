/*
 * main.h
 * author: Juan-Pablo Caceres
 * date: June 2008
 */

#include <iostream>
#include <unistd.h>

#include "JackAudioInterface.h"

using namespace std;

int main(int argc, char** argv)
{
  JackAudioInterface test(10,10);
  cout << "SR: " << test.getSampleRate() << endl;
  cout << "Buffer Size: " << test.getBufferSize() << endl;
  
  usleep(100000000);
  return 0;
}

