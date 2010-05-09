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

//00000000Å007FFFFF @DRAM_AREA* 
//00800000Å008027FF @CHANNEL_DATA 
//00802800Å00802FFF @COMMON_DATA 
//00803000Å00807FFF @DSP_DATA 
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

	//00000000Å 007FFFFF @DRAM_AREA* 
	//00800000Å 008027FF @CHANNEL_DATA 
	//00802800Å 00802FFF @COMMON_DATA 
	//00803000Å 00807FFF @DSP_DATA 
	/*
	u32 total_map=0x800000;//8 mb
	while(total_map)
	{
		total_map-=AICA_RAM_SIZE;
		_vmem_map_block(aica_ram,(total_map)>>16,(total_map+AICA_RAM_MASK)>>16);
		printf("Mapped 0x%08X to 0x%08X to ram \n",total_map,total_map+AICA_RAM_MASK);
	}
	_vmem_handler reg_read= _vmem_register_handler_Template(arm_ReadReg,arm_WriteReg);
	_vmem_map_handler(reg_read,0x0080,0x0081);
	printf("Mapped 0x%08X to 0x%08X to register funcions \n",0x00800000,0x00810000);

	
	for (u32 i=1;i<0x100;i++)
	{
		_vmem_mirror_mapping((i<<24)>>16,0,(1<<24)>>16);
	}*/
}
//kill mem map & free used mem ;)
void term_mem()
{
	//_vmem_term();
}


template<int sz,typename T>
u32 ReadMemArm(u32 addr)
{
	addr&=0x00FFFFFF;
	if (addr<0x800000)
	{
		return *(T*)&aica_ram[addr&ARAM_MASK];
	}
	else
	{
		return arm_ReadReg<sz,u32>(addr);
	}
}


template<int sz,typename T>
void WriteMemArm(u32 addr,T data)
{
	addr&=0x00FFFFFF;
	if (addr<0x800000)
	{
		*(T*)&aica_ram[addr&ARAM_MASK]=data;
	}
	else
	{
		arm_WriteReg<sz,T>(addr,data);
	}
}

template u32 ReadMemArm<8,u8>(u32 adr);
template u32 ReadMemArm<16,u16>(u32 adr);
template u32 ReadMemArm<32,u32>(u32 adr);

template void WriteMemArm<8>(u32 adr,u8 data);
template void WriteMemArm<16>(u32 adr,u16 data);
template void WriteMemArm<32>(u32 adr,u32 data);