#include "aica_hax.h"
#include "aica_hle.h"

u8 *aica_reg;
u8 *aica_ram;

u32 FASTCALL ReadMem_reg(u32 addr,u32 size)
{
	ReadMemArrRet(aica_reg,addr&0x7FFF,size);

	//should never come here
	return 0;
}

void FASTCALL  WriteMem_reg(u32 addr,u32 data,u32 size)
{
	if ((addr & 0xFFFF) == 0x2c00)
	{
		printf("Write to ARM reset, value= %x\n",data);
		if ((data&1)==0)
		{
			ARM_Katana_Driver_Info();
			arm7_on=true;
		}
		else
		{
			arm7_on=false;
		}
	}
	WriteMemArrRet(aica_reg,addr&0x7FFF,data,size);

	//should never come here
}

u32 ReadMem_ram(u32 addr,u32 size)
{
	//TODO : Add Warn
	return 0;
}

void WriteMem_ram(u32 addr,u32 data,u32 size)
{
	if (size==1)
		aica_ram[addr&AICA_MEM_MASK]=(u8)data;
	else if (size==2)
		*(u16*)&aica_ram[addr&AICA_MEM_MASK]=(u16)data;
	else if (size==4)
		*(u32*)&aica_ram[addr&AICA_MEM_MASK]=data;
}
int calls=0;
void FASTCALL UpdateAICA(u32 Cycles)
{
	calls=0;
	if (calls & 0x100 && HleEnabled())
	{
		//if ( (addr& AICA_MEM_MASK) > (aud_drv->cmdBuffer & AICA_MEM_MASK))
		{
			//if ((addr& AICA_MEM_MASK) < ((aud_drv->cmdBuffer +0x8000) & AICA_MEM_MASK))
			//	return 0xFFFFFF;
			memset(&aica_ram[aud_drv->cmdBuffer& AICA_MEM_MASK],0xFFFFFF,0x8000);
		}
	}

		*(u32*)&aica_ram[((0x80FFC0-0x800000)&0x1FFFFF)]=*(u32*)&aica_ram[((0x80FFC0-0x800000)&0x1FFFFF)]?0:3;
	//return 0x3;			//hack snd_dbg

	//the kos command list is 0x810000 to 0x81FFFF
	//here we hack the first and last comands
	//seems to fix everything ^^
	*(u32*)&aica_ram[0x1000C]=0x1;

	//aica_ram[0x1000C]+=0x1;

	//here we hack the first and last comands
	//seems to fix everything ^^
	*(u32*)&aica_ram[0x1FFFC]=1;
		//return 0x1;			//hack kos command que
	//aica_ram[0x1FFFC]+=0x1;

	//crazy taxi / doa2 /*
	//if (addr>0x800100 && addr<0x800300)
	//	return 0x800000;			//hack katana command que
	for (int i=0x100;i<0x300;i+=4)
	{
		*(u32*)&aica_ram[i]=0x800000;
	}
	//if (calls & 0x10)
		//memset(&aica_ram[0x100],0x800000,16);
	*(u32*)&aica_ram[0x32040]=0xFFFFFF;
		//return 0xFFFFFF;			//hack katana command que
	*(u32*)&aica_ram[0x32060]=0xFFFFFF;
		//return 0xFFFFFF;			//hack katana command que

	//trickstyle 
	//another position for command queue ?
	//if (addr>=0x813400 && addr<=0x813900)
	{
		//what to return what to return ??
		//this not works good
		//return 0x1;
	}
	//memset(a&ica_ram[13400],0x800000,0x400);

	//hack Katana wait
	//Waits for something , then writes to the value returned here
	//Found in Roadsters[also in Toy cmd]
	//it seems it is somehow rlated to a direct mem read after that 
	// im too tired to debug and find the actualy relation but it is
	*(u32*)&aica_ram[0x000EC]=0x80000;
		//return 0x80000;
	*(u32*)&aica_ram[0x0005C]=0x80000;
		//return 0x80000;
	/*Rez, not realy working tho*/
	/*static u32 tempyy=0xDEAD;
	if (addr==0x8000F8)
		return (tempyy++)^(tempyy^=-1);*/
	//*(u32*)&aica_ram[0x000EC]=0x1;

	//same as above , doa2/le
	*(u32*)&aica_ram[0x000E8]=0x80000;
		//return 0x80000;
	//*(u32*)&aica_ram[0x000E8]=0x1;

	//addr == 0x008000f8 -> recv , locks while reading from it ;)



	//some status bits/queue , hacked of course :p
	//if ((addr>0x8014e0) && (addr<0x8015e0))
	//{
	//	return 0xFFFFFF;
	//}

	for (int i=0x014e0;i<0x015e0;i+=4)
	{
		*(u32*)&aica_ram[i]=0xFFFFFF;
	}
	//if((addr&0xFF)==0x5C)
	//	return 1;			// hack naomi aica ram check ?

	//kos/katana
	//*(u32*)&aica_ram[((0x80FFC0-0x800000)&0x1FFFFF)]=*(u32*)&aica_ram[((0x80FFC0-0x800000)&0x1FFFFF)]?0:3;
	//return 0x3;			//hack snd_dbg

	//the kos command list is 0x810000 to 0x81FFFF
	//here we hack the first and last comands
	//seems to fix everything ^^
	//if (addr==0x81000C)
		//return 0x1;			//hack kos command que
	//aica_ram[0x1000C]+=0x1;

	//here we hack the first and last comands
	//seems to fix everything ^^
	//if (addr==0x81FFFC)
	//	return 0x1;			//hack kos command que
	//aica_ram[0x1FFFC]+=0x1;

	//crazy taxi / doa2 /*
	//if (addr>0x800100 && addr<0x800300)
		//return 0x800000;			//hack katana command que
	//if (calls & 0x10)
	//	memset(&aica_ram[0x100],0x800000,16);

	//trickstyle 
	//another position for command queue ?
	//if (addr>=0x813400 && addr<=0x813900)
	//{
		//what to return what to return ??
		//this not works good
		//return 0x1;
	//}
	//memset(a&ica_ram[13400],0x800000,0x400);

	//hack Katana wait
	//Waits for something , then writes to the value returned here
	//Found in Roadsters[also in Toy cmd]
	//it seems it is somehow rlated to a direct mem read after that 
	// im too tired to debug and find the actualy relation but it is
	//if (addr==0x8000EC)
	//	return 0x80000;
	//*(u32*)&aica_ram[0x000EC]=0x1;

	//same as above , doa2/le
	//if (addr==0x8000E8)
		//return 0x80000;
	//*(u32*)&aica_ram[0x000E8]=0x1;

	//addr == 0x008000f8 -> recv , locks while reading from it ;)



	//some status bits/queue , hacked of course :p
	//if ((addr>0x8014e0) && (addr<0x8015e0))
	//{
	//	return 0xFFFFFF;
	//}
}
void init_mem()
{
	//aica_ram=(u8*)malloc(AICA_MEM_SIZE);
	aica_reg=(u8*)malloc(0x8000);
	memset(aica_ram,0,AICA_MEM_SIZE);
	memset(aica_reg,0,0x8000);
}

void term_mem()
{
	free(aica_reg);
	//free(aica_ram);
}