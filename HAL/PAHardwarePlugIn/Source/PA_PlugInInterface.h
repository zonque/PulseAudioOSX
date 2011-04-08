#include <CoreAudio/AudioHardwarePlugIn.h>
#include "PA_Plugin.h"

class PA_PlugInInterface
{
public:
	PA_PlugInInterface();
	~PA_PlugInInterface();
	
	PA_Plugin *plugin;
	static AudioHardwarePlugInInterface staticInterface;
};
