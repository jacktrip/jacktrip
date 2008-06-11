#ifndef _INPUT_PLUGIN_H
#define _INPUT_PLUGIN_H

#include "plugin.h"

class Stream;

/**
 * @brief Virtual function declarations for subclasses to be used as inputs to Stream
 */

class InputPlugin : public Plugin
{ 
	protected:
		int	key;
		Stream *stream;
	public:
		InputPlugin(const char *name) : key( -1 )
                    {
                        this->setName(name);
						this->dontRun = false;
                    }
		virtual int rcv(char *buf) = 0;
		virtual void stop() = 0;
                                
		void setWriteKey(int newKey)
                    {
                        key = newKey;
                    }
                int getWriteKey()
                    {
                        return key;
                    }
                void setStream( Stream *str ) 
                    {
                        stream = str;
                    }
};

#endif
