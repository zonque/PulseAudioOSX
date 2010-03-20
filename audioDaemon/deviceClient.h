#ifndef DEVICE_CLIENT_H
#define DEVICE_CLIENT_H

extern CFMutableArrayRef deviceArray;

IOReturn deviceClientStart(void);
void deviceClientStop(void);

#endif /* DEVICE_CLIENT_H */
