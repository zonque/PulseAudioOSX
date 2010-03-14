#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include "../kext/PAVirtualDeviceUserClientTypes.h"
#include "../kext/PADriverUserClientTypes.h"
#include "../kext/PAUserClientCommonTypes.h"

struct audioDevice;

struct audioDevice *audioDeviceCreate(io_connect_t port, struct PAVirtualDeviceInfo *info);
void audioDeviceRemove(struct audioDevice *dev);

int audioDeviceGetIndex(struct audioDevice *dev);
io_connect_t audioDeviceGetPort(struct audioDevice *dev);

#endif /* AUDIODEVICE_H */