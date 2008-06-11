#ifndef _OUTPUT_PLUGIN_H
#define _OUTPUT_PLUGIN_H

#include "plugin.h"

class Stream;

/**
 * @brief Virtual function declarations for outputs from Stream
 */

class OutputPlugin : public Plugin
{
  protected:
    int    key;
    Stream *stream;
  public:
    OutputPlugin(const char *name) : key(-1)
        {
            this->setName(name);
			this->dontRun = false;
        }
    virtual int send(char *buf) = 0;
    virtual void stop() = 0;
    void setReadKey(int newKey)
        {
            key = newKey;
        }
    int getReadKey()
        {
            return key;
        }
    void setStream( Stream *str )
        {
            stream = str;
        }
    
};

#endif
