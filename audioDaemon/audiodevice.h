/*
 *  audiodevice.h
 *  audioDaemon
 *
 *  Created by caiaq on 7/18/09.
 *  Copyright 2009 caiaq. All rights reserved.
 *
 */

struct audiodevice;

struct audiodevice *audiodevice_create(io_service_t service);
