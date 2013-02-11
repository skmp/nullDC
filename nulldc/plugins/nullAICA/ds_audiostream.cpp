#include <initguid.h>
#include <dsound.h>
#include "audiostream_rif.h"
#include "audiostream.h"

u32 THREADCALL SoundThread(void* param);
#define V2_BUFFERSZ (16*1024)

IDirectSound8* dsound;
IDirectSoundBuffer8* buffer;
IDirectSoundNotify8* buffer_notify;

cThread sound_th(SoundThread,0);
HANDLE buffer_events[V2_BUFFERSZ/256];
volatile bool soundthread_running=false;

DSBPOSITIONNOTIFY not[V2_BUFFERSZ/256];
u32 WritePositions[V2_BUFFERSZ/256];

u32 wait_buffer_size;
u32 sound_buffer_count;

bool StreamV2=false;
void UpdateBuff(u8* pos)
{
	bool hasbuf=asRingRead(pos);
	if (settings.GlobalMute)
	{
		memset(pos,0,wait_buffer_size);
	}
	else
	{
		static int oob_c=0;
		if (!hasbuf)
		{
			if (oob_c++>2)
				memset(pos,0,wait_buffer_size);
			//printf("GetReadBuffer -- Out Of Buffers\n");
		}
		else
		{
			oob_c=0;
		}
	}
}
u32 THREADCALL SoundThread1(void* param)
{
	for(;;)
	{
		u32 rv = WaitForMultipleObjects(sound_buffer_count,buffer_events,FALSE,400);
		
		if (!soundthread_running)
			break;

		LPVOID p1,p2;
		DWORD s1,s2;

		rv-=WAIT_OBJECT_0;

		if(SUCCEEDED(buffer->Lock(WritePositions[rv],wait_buffer_size,&p1,&s1,&p2,&s2,0)))
		{
			UpdateBuff((u8*)p1);	
			verifyc(buffer->Unlock(p1,s1,p2,s2));
		}

	}
	return 0;
}
/*
u32 THREADCALL SoundThread2(void* param)
{
	//when play gets at 0, we write 64
	//when play gets at 64, we write 128
	//etc :)
	static u32 sound_write_pos=wait_buffer_size;//first notification is at pos = 64

	while(soundthread_running)
	{
		u32 rv = WaitForSingleObject(buffer_events[0],INFINITE);

		LPVOID p1,p2;
		DWORD s1,s2;

	
		//part 1
		verifyc(buffer->Lock(sound_write_pos,wait_buffer_size,&p1,&s1,&p2,&s2,0));

		UpdateBuff((u8*)p1);

		verifyc(buffer->Unlock(p1,s1,p2,s2));
		not[0].dwOffset=0;
			
		buffer_notify->SetNotificationPositions(1,not);
		DWORD pc,wc;
		buffer->GetCurrentPosition(&pc,&wc);
		printf("Writen @ %d, play pos @ %d\n",sound_write_pos,pc);
		sound_write_pos+=wait_buffer_size;	//Update write position :)
		if (sound_write_pos>=sound_buffer_size)
			sound_write_pos-=sound_buffer_size;
	}
	return 0;
}
*/



u32 THREADCALL SoundThread(void* param)
{
//	if (StreamV2)
//		return SoundThread2(param);
	//else
		return SoundThread1(param);
}
void ds_InitAudio()
{
	if (settings.BufferSize<2048)
	{
		StreamV2=true;
	}
	verifyc(DirectSoundCreate8(NULL,&dsound,NULL));

	verifyc(dsound->SetCooperativeLevel((HWND)eminf.GetRenderTarget(),DSSCL_PRIORITY));
	IDirectSoundBuffer* buffer_;

	WAVEFORMATEX wfx; 
	DSBUFFERDESC desc; 

	// Set up WAV format structure. 

	memset(&wfx, 0, sizeof(WAVEFORMATEX)); 
	wfx.wFormatTag = WAVE_FORMAT_PCM; 
	wfx.nChannels = 2; 
	wfx.nSamplesPerSec = 44100; 
	wfx.nBlockAlign = 4; 
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign; 
	wfx.wBitsPerSample = 16; 

	// Set up DSBUFFERDESC structure. 

	memset(&desc, 0, sizeof(DSBUFFERDESC)); 
	desc.dwSize = sizeof(DSBUFFERDESC); 
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;// _CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY; 
	
	if (!StreamV2)
		desc.dwBufferBytes = settings.BufferSize*(2)* wfx.nBlockAlign; 
	else
		desc.dwBufferBytes = V2_BUFFERSZ* wfx.nBlockAlign;	//fixed at 8k samples :)

	desc.lpwfxFormat = &wfx; 

	wait_buffer_size=settings.BufferSize*wfx.nBlockAlign;

	if (settings.HW_mixing==0)
	{
		desc.dwFlags |=DSBCAPS_LOCSOFTWARE;
	}
	else if (settings.HW_mixing==1)
	{
		desc.dwFlags |=DSBCAPS_LOCHARDWARE;
	}
	else if (settings.HW_mixing==2)
	{
		//auto
	}
	else
	{
		die("settings.HW_mixing: Invalid value");
	}

	if (settings.GlobalFocus)
		desc.dwFlags|=DSBCAPS_GLOBALFOCUS;

	verifyc(dsound->CreateSoundBuffer(&desc,&buffer_,0));
	verifyc(buffer_->QueryInterface(IID_IDirectSoundBuffer8,(void**)&buffer));
	buffer_->Release();

	verifyc(buffer->QueryInterface(IID_IDirectSoundNotify8,(void**)&buffer_notify));

	

	DWORD chunk_count;

	if (StreamV2)
	{
		chunk_count=V2_BUFFERSZ/settings.BufferSize;
		printf("DSOUND V2 : Using %d chunks of %d size\n",chunk_count,wait_buffer_size);
	}
	else
	{
		chunk_count=2;
		printf("DSOUND V1 : Using 2 chunks of %d size\n",wait_buffer_size);
	}

	sound_buffer_count=chunk_count;

	for (u32 bei=0;bei<chunk_count;bei++)
	{
		buffer_events[bei]=not[bei].hEventNotify=CreateEvent(NULL,FALSE,FALSE,NULL);

		not[bei].dwOffset=bei*wait_buffer_size;
		WritePositions[(bei+chunk_count-1)%chunk_count]=not[bei].dwOffset;
	}

	buffer_notify->SetNotificationPositions(chunk_count,not);

	//Clear the buffer with silence
	LPVOID p1=0,p2=0;
	DWORD s1=0,s2=0;

	verifyc(buffer->Lock(0,desc.dwBufferBytes,&p1,&s1,&p2,&s2,0));
	verify(p2==0);
	memset(p1,0,s1);
	verifyc(buffer->Unlock(p1,s1,p2,s2));

	//Start the thread
	soundthread_running=true;
	verify(SetThreadPriority((HANDLE)sound_th.hThread,THREAD_PRIORITY_TIME_CRITICAL));
	sound_th.Start();
	//Make sure its run before the buffer releases the event
	Sleep(0);
	//Play the buffer !
	verifyc(buffer->Play(0,0,DSBPLAY_LOOPING));
	
}
void ds_TermAudio()
{
	buffer->Stop();
	soundthread_running=false;
	SetEvent(buffer_events[0]);
	sound_th.WaitToEnd(INFINITE);

	for (u32 i=0;i<sound_buffer_count;i++)
		verify(CloseHandle(buffer_events[i]));

	buffer_notify->Release();
	buffer->Release();
	dsound->Release();
}