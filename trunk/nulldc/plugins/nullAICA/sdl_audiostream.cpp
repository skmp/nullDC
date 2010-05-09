
#include "audiostream_rif.h"
#include "audiostream.h"

#ifdef _INCLUDE_SDL
#include <SDL.h>
#pragma comment(lib, "sdl.lib") 

/* Prototype of our callback function */
void sdl_fill_audio(void *userdata, Uint8 *stream, int len);
void sdl_InitAudio()
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE);
	
	SDL_AudioSpec desired;

	//44.1k
	desired.freq=44100;
	//16-bit signed audio
	desired.format=AUDIO_S16LSB;
	//stereo
	desired.channels=2;
	//Large audio buffer reduces risk of dropouts but increases response time
	desired.samples=settings.BufferSize;

	/* Our callback function */
	desired.callback=sdl_fill_audio;

	desired.userdata=NULL;

	/* Open the audio device */
	if ( SDL_OpenAudio(&desired, 0) < 0 ){
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}
	//sdl_InitAudBuffers(desired.samples);
	SDL_PauseAudio(0);
}
void sdl_TermAudio()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}

void sdl_fill_audio(void *userdata, Uint8 *stream, int len)
{
	u8* rb=GetReadBuffer();
	speed_limit.Set();
	if (rb)
	{
		memcpy(stream,rb,len);
	}
	else
	{
		memset(stream,0,len);
		//printf("SDLAudioStream : out of buffers ...\n");
	}

	//if (len!=settings.BufferSize*4)
	//	printf("Error [sdl_fill_audio]: len!=buffersz\n");
}
#endif