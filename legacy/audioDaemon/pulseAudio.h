#ifndef _AD_PULSEAUDIO_H
#define _AD_PULSEAUDIO_H

int pulseAudioClientStart(void);
void pulseAudioClientStop(void);

pa_mainloop_api *pulseAudioAPI(void);

#endif /* _AD_PULSEAUDIO_H */
