#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

struct audiodevice;

struct audiodevice *audiodevice_create(io_connect_t port, int index);

#endif /* AUDIODEVICE_H */