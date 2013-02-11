#include "audiostream.h"
#include "sdl_audiostream.h"
#include "ds_audiostream.h"


/*
WriteSample_FP* WriteSample;
WriteSamples1_FP* WriteSamples1;
WriteSamples2_FP* WriteSamples2;
*/
cResetEvent speed_limit(true,true);

struct SoundSample { s16 l;s16 r; };
SoundSample* RingBuffer;
SoundSample* TempBuffer;
u32 RingBufferByteSize;
u32 RingBufferSampleCount;

volatile u32 WritePtr;	//last WRITEN sample
volatile u32 ReadPtr;	//next sample to read
u32 BufferByteSz;
u32 BufferSampleCount;

u32 gen_samples=0;
LARGE_INTEGER time_now,time_last;
LARGE_INTEGER time_diff;

#ifdef LOG_SOUND
WaveWriter rawout("d:\\aica_out.wav");
#endif
void InitAudBuffers(u32 buff_samples)
{
	BufferSampleCount=buff_samples;
	BufferByteSz=BufferSampleCount*sizeof(s16)*2;
	RingBufferSampleCount=(settings.BufferCount+2)*BufferSampleCount;
	RingBufferByteSize=RingBufferSampleCount*sizeof(s16)*2;
	WritePtr=0;
	ReadPtr=0;
	RingBuffer=(SoundSample*)malloc(RingBufferByteSize);
	TempBuffer=(SoundSample*)malloc(RingBufferByteSize);

	//first samlpe is actualy used, so make sure its 0 ...
	RingBuffer[0].r=0;
	RingBuffer[0].l=0;
}
u32 asRingUsedCount()
{
	if (WritePtr>=ReadPtr)
		return WritePtr-ReadPtr;
	else
		return RingBufferSampleCount-(ReadPtr-WritePtr);
	//s32 sz=(WritePtr+1)%RingBufferSampleCount-ReadPtr;
	//return sz<0?sz+RingBufferSampleCount:sz;
}
u32 asRingFreeCount()
{
	return RingBufferSampleCount-asRingUsedCount()-1;
}
//Return read buffer , 0 if none
bool asRingRead(u8* dst,u32 sz)
{
	if (sz==0)
		sz=BufferSampleCount;
	if (asRingUsedCount()>=sz)
	{
		const u32 ptr=ReadPtr;
		if ((ReadPtr+sz)<=RingBufferSampleCount)
		{
			//R ... W, just copy the sz :)
			memcpy(dst,&RingBuffer[ptr],sz*sizeof(s16)*2);
		}
		else
		{
			//...W R...., just copy the sz :)
			const u32 szhi=RingBufferSampleCount-ptr;
			const u32 szlow=sz-szhi;

			memcpy(dst,&RingBuffer[ptr],szhi*sizeof(s16)*2);
			dst+=szhi*sizeof(s16)*2;
			memcpy(dst,&RingBuffer[0],szlow*sizeof(s16)*2);
		}
		ReadPtr=(ptr+sz)%RingBufferSampleCount;
		
		speed_limit.Set();
		return true;
	}
	else
	{
		//printf("ONLY %d used, rd=%d, wt=%d\n",asRingUsedCount(),ReadPtr,WritePtr);
		return false;
	}
}

bool asRingStretchPitchChange(u8* dst,u32 outcount,u32 usedcount)
{
	bool rv=asRingRead((u8*)TempBuffer,usedcount);
	verify(rv==true);//can't be false ...
	u32 steps=usedcount*outcount*2;

	//this does change pitch, but for a few samples it won't matter much ...
	for (u32 cs=outcount-1;cs<steps;cs+=usedcount*2)
	{
		u32 srcidx=cs/(outcount*2) ;
		verify(srcidx<usedcount);

		*(SoundSample*)dst=TempBuffer[srcidx];
		dst+=sizeof(SoundSample);
	}
	return true;
}
/*
void DWTInv(SampleType* pout,SampleType* pin,u32 samples)
{
}
void DWTFwd(SampleType* pout,SampleType* pin,u32 samples)
{
    //This function assumes input.length=2^n, n>1
    //int[] output = new int[input.length];
 
    for (int length = samples >> 1,u32 odd=samples&1; ; length >>= 1) 
	{
        //length=2^n, WITH DECREASING n
        for (int i = 0; i < length; i++) 
		{
            int sum = input[i*2]+input[i*2+1];
            int difference = input[i*2]-input[i*2+1];
            output[i] = sum;
            output[length+i] = difference;
        }
		if (odd)
			output[length+length]=output[length+length-1];
        if (length == 1) 
            return;
		odd=length&1;
 
        //Swap arrays to do next iteration
        System.arraycopy(output, 0, input, 0, length<<1);
    }
}
*/

//0 returns 0, 2^31 returns 0 (overflow), works for all else values
//can be implemented like
//mov eax,ecx; or pop eax;
//bsr ecx,eax;
//bsf edx,eax;
//jz _nothing_set;
//cmp edx,ecx	//zero (no carry) if equal, negative (since edx<=ecx) otherwise
//adc ecx,0;	//add the carry bit;
//
//mov eax,1;	//generate the next number 1<<cl
//shl eax,cl;	
//_nothing_set:	//eax = 0 (input value) or 1<<cl
//ret
// ;)
u32 next_pow2(u32 s)
{
	unsigned long idxR,idxF;
	_BitScanReverse(&idxR,s);
	if (0==_BitScanReverse(&idxF,s))
		return s;	//its 0 anyway ...
	else
	{
		//idxR==idxF -> single bit set, power of 2
		//idxR>idxF -> more bits set, round up by next idxR
		idxR+=((s32)idxF-(s32)idxR)>=0?0:1;
	}
	return 1<<idxR;
}
/*
SoundSample DWTTemp[4096];
bool asRingStretchDWT(u8* dst,u32 outcount,u32 usedcount)
{
	u32 strech_count=next_pow2(usedcount);
	asRingStretchPitchChange((u8*)DWTTemp,strech_count,usedcount);
	//do DWT
	//shift the frequencys
	//do iDWT

	return true;
}
*/
bool asRingStretchRead(u8* dst,u32 sz)
{
	if (sz==1)
		return false; //not supported ...

	if (sz==0)
		sz=BufferSampleCount;

	u32 used=asRingUsedCount();
	//used=min(used,sz*3/6);
	if (used==0)
		return false;

	if (settings.LimitFPS && used>=sz)
	{
		return asRingRead(dst,sz);
	}
	else
	{
		return asRingStretchPitchChange(dst,sz,used);
	}
}

void WriteSample(s16 r, s16 l)
{
	#ifdef LOG_SOUND
	rawout.Write(l,r);
	#endif

	if (!asRingFreeCount())
	{
		if (settings.LimitFPS)
		{
			speed_limit.Reset();
			speed_limit.Wait();
		}
		else
			return;
	}

	gen_samples++;
	//while limit on, 128 samples done, there is a buffer ready to be service AND speed is too fast then wait ;p
	while (settings.LimitFPS==1 && gen_samples>128 && asRingUsedCount()>BufferSampleCount && QueryPerformanceCounter(&time_now) && (time_now.QuadPart-time_last.QuadPart)<=time_diff.QuadPart )
	{
		__noop;
	}

	if (settings.LimitFPS==1 && gen_samples>128)
	{
		gen_samples=0;
		QueryPerformanceCounter(&time_last);
	}

	const u32 ptr=(WritePtr+1)%RingBufferSampleCount;
	RingBuffer[ptr].r=r;
	RingBuffer[ptr].l=l;
	WritePtr=ptr;
	//if (0==(WritePtr&255))
	//printf("write done %d %d \n",ReadPtr,WritePtr);
}
void WriteSamples1(s16* r , s16* l , u32 sample_count)
{
	for (u32 i=0;i<sample_count;i++)
		WriteSample(r[i],l[i]);
}
void WriteSamples2(s16* rl , u32 sample_count)
{
	for (u32 i=0;i<(sample_count*2);i+=2)
		WriteSample(rl[i|0],rl[i|1]);
}

void InitAudio()
{
	QueryPerformanceFrequency(&time_diff);
	time_diff.QuadPart*=128;
	time_diff.QuadPart/=44100;

	InitAudBuffers(settings.BufferSize);
	if (settings.SoundRenderer)
	{
		//DSound
		ds_InitAudio();
	}
	else
	{
		//SDL
		sdl_InitAudio();
	}
}
void TermAudio()
{
	if (settings.SoundRenderer)
	{
		ds_TermAudio();
	}
	else
	{
		sdl_TermAudio();
	}
}