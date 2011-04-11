#include <CoreAudio/AudioHardwarePlugIn.h>
#include "PA_Plugin.h"

class PA_PlugInInterface
{
private:
	AudioHardwarePlugInInterface *staticInterface;
	
public:
	PA_PlugInInterface(CFAllocatorRef inAllocator);
	~PA_PlugInInterface();
	
	PA_Plugin *plugin;
	
	AudioHardwarePlugInRef GetInterface() { return &staticInterface; }

};
