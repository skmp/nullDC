#pragma once
#include "nullAICA.h"

void WriteSample(s16 r, s16 l);
void WriteSamples1(s16* r , s16* l , u32 sample_count);
void WriteSamples2(s16* rl , u32 sample_count);

void InitAudio();
void TermAudio();

class WaveWriter
{
public:
	FILE* f;
	WaveWriter(char* file)
	{
		f=fopen(file,"wb");
		Write("RIFF",4);
		Write("NULL",4); //will replaced with file size later on
		Write("WAVE",4);

		Write("fmt ",4);
		Write32(16);		//Chunk size, 16 for pcm
		Write16(1);			//uncompressed
		Write16(2);			//2 channels
		Write32(44100);		//Sample rate
		Write32(44100*2*2); //Byte rate
		Write16(4);			//block aligment
		Write16(16);			//Bits per sample

		Write("data",4);
		Write("NULL",4); //will replaced with file size later on

	}
	void Write(void* ptr,u32 sz)
	{
		fwrite(ptr,1,sz,f);
	}
	void Write8(u8 l)
	{
		Write(&l,1);
	}
	void Write16(u16 l)
	{
		Write(&l,2);
	}
	void Write32(u32 l)
	{
		Write(&l,4);
	}

	void Write(s16 l,s16 r)
	{
		Write(&l,2);
		Write(&r,2);
	}
	~WaveWriter()
	{
		int sz=ftell(f);
		fseek(f,4,SEEK_SET);
		Write32(sz-8);
		fseek(f,40,SEEK_SET);
		Write32(sz-44);
		fclose(f);
	}
};