//
//  PAUserClient.h
//

#ifndef PAUSERCLIENT_H
#define PAUSERCLIENT_H

#include <IOKit/IOUserClient.h>

#define PAUserClient org_pulseaudio_iouserclient

// keep this enum in sync with userspace applications
enum {
	kPAUserClientGetNumSampleFrames,
	kPAUserClientSetSampleRate,
	kPAUserClientNumberOfMethods
};

class PAUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(PAUserClient)

private:
	PADevice	*device;

	static const IOExternalMethodDispatch	sMethods[kPAUserClientNumberOfMethods];

// IOUserClient interface
public:
	IOReturn	externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
							   IOExternalMethodDispatch *dispatch, OSObject *target, void *reference);
	IOReturn	clientMemoryForType(UInt32 type, UInt32 *flags, IOMemoryDescriptor **memory);
	IOReturn	message(UInt32 type, IOService *provider,  void *argument = 0);
	IOReturn	clientClose(void);

	void		stop(IOService * provider);
	bool		start(IOService * provider);
	bool		initWithTask(task_t owningTask, void * securityID, UInt32 type);
	bool		finalize(IOOptionBits options);
	bool		terminate(IOOptionBits options);

	static IOReturn		setSampleRate(PAUserClient *target, void *reference, IOExternalMethodArguments *arguments);
	static IOReturn		getNumSampleFrames(PAUserClient *target, void *reference, IOExternalMethodArguments *arguments);
};

#endif // PAUSERCLIENT_H