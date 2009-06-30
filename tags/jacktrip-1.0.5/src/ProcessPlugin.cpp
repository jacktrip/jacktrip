//#include "ProcessPlugin.h"

/*
//----------------------------------------------------------------------------
// Jack Callbacks 
//----------------------------------------------------------------------------

int srate(jack_nframes_t nframes, void *arg)
{
  printf("the sample rate is now %u/sec\n", nframes);
  return 0;
}

void jack_shutdown(void *arg)
{
  std::cout << "" << std::endl; 
  std::exit(1);
}

int process (jack_nframes_t nframes, void *arg)
{
  for (int i = 0; i < gNumInChans; i++) {
    gInChannel[i] = (float *)jack_port_get_buffer(input_ports[i], nframes);
  }
  for (int i = 0; i < gNumOutChans; i++) {
    gOutChannel[i] = (float *)jack_port_get_buffer(output_ports[i], nframes);
  }
  DSP.compute(nframes, gInChannel, gOutChannel);
  return 0;
}
*/
