#include "mem.h"
#include "arm7.h"

u8 *aica_ram;
//Set to true when aica interrupt is pending
bool aica_interr=false;
u32 aica_reg_L=0;
//Set to true when the out of the intc is 1
bool e68k_out = false;
u32 e68k_reg_L;
u32 e68k_reg_M=0;	//constant ?

void update_e68k()
{
	if (!e68k_out && aica_interr)
	{
		//Set the pending signal
		//Is L register holded here too ?
		e68k_out=1;
		e68k_reg_L=aica_reg_L;
	}
}

void FASTCALL ArmInterruptChange(u32 bits,u32 L)
{
	aica_interr=bits!=0;
	if (aica_interr)
		aica_reg_L=L;
	update_e68k();
}

void e68k_AcceptInterrupt()
{
	e68k_out=false;
	update_e68k();
}

//Reg reads from arm side ..
template <u32 sz,class T>
inline T fastcall arm_ReadReg(u32 addr)
{
	addr&=0x7FFF;
	if (addr==REG_L)
		return e68k_reg_L;
	else if(addr==REG_M)
		return e68k_reg_M;	//shoudn't really happen
	else
		return arm_params.ReadMem_aica_reg(addr,sz);
}		
template <u32 sz,class T>
inline void fastcall arm_WriteReg(u32 addr,T data)
{
	addr&=0x7FFF;
	if (addr==REG_L)
		return;				//shoudn't really happen (read only)
	else if(addr==REG_M)
	{
		//accept interrupts
		if (data&1)
			e68k_AcceptInterrupt();
	}
	else
		return arm_params.WriteMem_aica_reg(addr,data,sz);
}
//Map using _vmem .. yay
void init_mem()
{
	aica_ram=arm_params.aica_ram;
}
//kill mem map & free used mem ;)
void term_mem()
{
	
}

//00000000~007FFFFF @DRAM_AREA* 
//00800000~008027FF @CHANNEL_DATA 
//00802800~00802FFF @COMMON_DATA 
//00803000~00807FFF @DSP_DATA 

//Force alignment for read/writes to mem
#define ACCESS_MASK (ARAM_MASK-(sz-1))

template<int sz,typename T>
T fastcall ReadMemArm(u32 addr)
{
	addr&=0x00FFFFFF;
	if (addr<0x800000)
	{
		T rv=*(T*)&aica_ram[addr&ACCESS_MASK];
		
		if (sz==4)
		{
			//32 bit misalligned reads: rotated
			u32 rot=(addr&3)<<3;
			return (rv>>rot) | (rv<<(32-rot));
		}
		else
			return rv;
	}
	else
	{
		return arm_ReadReg<sz,T>(addr);
	}
}

template<int sz,typename T>
void fastcall WriteMemArm(u32 addr,T data)
{
	addr&=0x00FFFFFF;
	if (addr<0x800000)
	{
		*(T*)&aica_ram[addr&ACCESS_MASK]=data;
	}
	else
	{
		arm_WriteReg<sz,T>(addr,data);
	}
}

template u8 ReadMemArm<1,u8>(u32 adr);
template u16 ReadMemArm<2,u16>(u32 adr);
template u32 ReadMemArm<4,u32>(u32 adr);

template void WriteMemArm<1>(u32 adr,u8 data);
template void WriteMemArm<2>(u32 adr,u16 data);
template void WriteMemArm<4>(u32 adr,u32 data);

