//
//  PAUserClient.h
//

#ifndef PAUSERCLIENT_H
#define PAUSERCLIENT_H

#include <IOKit/IOUserClient.h>

#define PAUserClient org_pulseaudio_iouserclient

class PAUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(PAUserClient)

private:
	PADevice	*device;

// IOUserClient interface
public:
	IOReturn	clientMemoryForType(UInt32 type, UInt32 *flags, IOMemoryDescriptor **memory);
	IOReturn	message(UInt32 type, IOService *provider,  void *argument = 0);
	IOReturn	clientClose(void);

	void		stop(IOService * provider);
	bool		start(IOService * provider);
	bool		initWithTask(task_t owningTask, void * securityID, UInt32 type);
	bool		finalize(IOOptionBits options);
	bool		terminate(IOOptionBits options);
};

#endif // PAUSERCLIENT_H