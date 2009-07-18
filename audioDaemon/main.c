#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CFMachPort.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CoreFoundation.h>

__BEGIN_DECLS
#include <mach/mach.h>
#include <IOKit/iokitmig.h>
__END_DECLS

#include "audiodevice.h"

#define PADriverClassName "org_pulseaudio_ioaudiodevice"

static void 
deviceAdded (void *refCon, io_iterator_t iterator)
{
    io_service_t        serviceObject;

    while ((serviceObject = IOIteratorNext(iterator)))
		audiodevice_create(serviceObject);
}

static void 
deviceRemoved (void *refCon, io_iterator_t iterator)
{
    io_service_t        serviceObject;
	
    while ((serviceObject = IOIteratorNext(iterator))) {
		// TODO
	}
}

void listenToService(char *name, mach_port_t masterPort)
{
    OSStatus				ret;
    CFMutableDictionaryRef	classToMatch;
    IONotificationPortRef	gNotifyPort;
    io_iterator_t			gNewDeviceAddedIter;
    io_iterator_t			gNewDeviceRemovedIter;
	CFRunLoopSourceRef		runLoopSource;

    classToMatch = IOServiceMatching(name);
    if (!classToMatch) {
        printf("listenToService: IOServiceMatching returned a NULL dictionary.\n");
        return;
    }

    // increase the reference count by 1 since die dict is used twice.
    classToMatch = (CFMutableDictionaryRef) CFRetain(classToMatch);
    
    gNotifyPort = IONotificationPortCreate(masterPort);
    runLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
	
    ret = IOServiceAddMatchingNotification(gNotifyPort,
                                           kIOFirstMatchNotification,
                                           classToMatch,
                                           deviceAdded,
                                           NULL,
                                           &gNewDeviceAddedIter);
    
    // Iterate once to get already-present devices and arm the notification
    deviceAdded(NULL, gNewDeviceAddedIter);
	
    ret = IOServiceAddMatchingNotification(gNotifyPort,
                                           kIOTerminatedNotification,
                                           classToMatch,
                                           deviceRemoved,
                                           NULL,
                                           &gNewDeviceRemovedIter);
	
    // Iterate once to get already-present devices and arm the notification
    deviceRemoved(NULL, gNewDeviceRemovedIter);
}

int main (int argc, const char * argv[]) {

    mach_port_t		masterPort;
    kern_return_t	kernResult;

	kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
	
    if (kernResult != kIOReturnSuccess) {
        printf("IOMasterPort returned %d\n", kernResult);
        return false;
    }

	listenToService(PADriverClassName, masterPort);
	mach_port_deallocate(mach_task_self(), masterPort);

	CFRunLoopRun();
	
	return 0;
}
