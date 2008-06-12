#ifndef _PROCESS_PLUGIN_H
#define _PROCESS_PLUGIN_H

#include "plugin.h"

/**
 * @brief Virtual function declarations for process plugins to Stream
 */

class ProcessPlugin : public Plugin
{
	public:
		ProcessPlugin(const char *name)  { this->setName(name); }
		ProcessPlugin()  { this->setName("no name process :("); }
			
		virtual int process(char *buf) = 0;
		void run() {}
};

#endif
