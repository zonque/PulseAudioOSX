#ifndef DRIVER_CLIENT_H
#define DRIVER_CLIENT_H

extern mach_port_t driver_async_port;
extern io_connect_t driver_data_port;

IOReturn addDeviceFromInfo (struct PAVirtualDeviceInfo *info);
IOReturn driverClientStart(void);

#endif /* DRIVER_CLIENT_H */