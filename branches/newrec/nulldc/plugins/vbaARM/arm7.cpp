#include "arm7.h"
#include "mem.h"

#define C_CORE

//#define CPUReadHalfWordQuick(addr) arm_ReadMem16(addr & 0x7FFFFF)
#define CPUReadMemoryQuick(addr) arm_ReadMem32(addr & ARAM_MASK)
#define CPUReadByte arm_ReadMem8
#define CPUReadMemory arm_ReadMem32
#define CPUReadHalfWord arm_ReadMem16
#define CPUReadHalfWordSigned(addr) ((s16)arm_ReadMem16(addr))

#define CPUWriteMemory arm_WriteMem32
#define CPUWriteHalfWord arm_WriteMem16
#define CPUWriteByte arm_WriteMem8


#define reg arm_Reg
#define armNextPC arm_ArmNextPC


#define CPUUpdateTicksAccesint(a) 1
#define CPUUpdateTicksAccessSeq32(a) 1
#define CPUUpdateTicksAccesshort(a) 1
#define CPUUpdateTicksAccess32(a) 1
#define CPUUpdateTicksAccess16(a) 1


enum
{
	R13_IRQ=18,
	R14_IRQ=19,
	SPSR_IRQ =20,
	R13_USR=26,
	R14_USR=27,
	R13_SVC=28,
	R14_SVC=29,
	SPSR_SVC =30,
	R13_ABT=31,
	R14_ABT=32,
	SPSR_ABT =33,
	R13_UND=34,
	R14_UND=35,
	SPSR_UND =36,
	R8_FIQ= 37,
	R9_FIQ= 38,
	R10_FIQ=39,
	R11_FIQ=40,
	R12_FIQ=41,
	R13_FIQ=42,
	R14_FIQ=43,
	SPSR_FIQ =44,
};

typedef union {
	struct {
		u8 B0;
		u8 B1;
		u8 B2;
		u8 B3;
	} B;
	struct {
		u16 W0;
		u16 W1;
	} W;
	u32 I;
} reg_pair;

//bool arm_FiqPending; -- not used , i use the input directly :)
//bool arm_IrqPending;

u32 arm_ArmNextPC;

reg_pair arm_Reg[45];

void CPUSwap(u32 *a, u32 *b)
{
	u32 c = *b;
	*b = *a;
	*a = c;
}


bool N_FLAG;
bool Z_FLAG;
bool C_FLAG;
bool V_FLAG;
bool armIrqEnable;
bool armFiqEnable;
//bool armState;
int armMode;

bool Arm7Enabled=false;

BYTE cpuBitsSet[256];

u16 IE;
u16 IF;
u16 IME;

int cpuSavedTicks;
int* extCpuLoopTicks = NULL;
int* extClockTicks = NULL;
int* extTicks = NULL;

bool intState = false;
bool stopState = false;
bool holdState = false;

#define CPU_BREAK_LOOP \
	cpuSavedTicks = cpuSavedTicks - *extCpuLoopTicks;\
	*extCpuLoopTicks = *extClockTicks;

#define CPU_BREAK_LOOP_2 \
	cpuSavedTicks = cpuSavedTicks - *extCpuLoopTicks;\
	*extCpuLoopTicks = *extClockTicks;\
	*extTicks = *extClockTicks;

void CPUSwitchMode(int mode, bool saveState, bool breakLoop=true);
void CPUUpdateCPSR();
void CPUUpdateFlags(bool breakLoop = true);
void CPUSoftwareInterrupt(int comment);
void CPUUndefinedException();
void CPUInterrupt();
void CPUFiq();

//void CreateTables();
void arm_Init()
{
	//CreateTables();
	arm_Reset();
	
	int i;
	for(i = 0; i < 256; i++) 
	{
		int count = 0;
		int j;
		for(j = 0; j < 8; j++)
			if(i & (1 << j))
				count++;
		cpuBitsSet[i] = count;
	}
}

void CPUSwitchMode(int mode, bool saveState, bool breakLoop)
{
	CPUUpdateCPSR();

	switch(armMode) {
	case 0x10:
	case 0x1F:
		reg[R13_USR].I = reg[13].I;
		reg[R14_USR].I = reg[14].I;
		reg[17].I = reg[16].I;
		break;
	case 0x11:
		CPUSwap(&reg[R8_FIQ].I, &reg[8].I);
		CPUSwap(&reg[R9_FIQ].I, &reg[9].I);
		CPUSwap(&reg[R10_FIQ].I, &reg[10].I);
		CPUSwap(&reg[R11_FIQ].I, &reg[11].I);
		CPUSwap(&reg[R12_FIQ].I, &reg[12].I);
		reg[R13_FIQ].I = reg[13].I;
		reg[R14_FIQ].I = reg[14].I;
		reg[SPSR_FIQ].I = reg[17].I;
		break;
	case 0x12:
		reg[R13_IRQ].I  = reg[13].I;
		reg[R14_IRQ].I  = reg[14].I;
		reg[SPSR_IRQ].I =  reg[17].I;
		break;
	case 0x13:
		reg[R13_SVC].I  = reg[13].I;
		reg[R14_SVC].I  = reg[14].I;
		reg[SPSR_SVC].I =  reg[17].I;
		break;
	case 0x17:
		reg[R13_ABT].I  = reg[13].I;
		reg[R14_ABT].I  = reg[14].I;
		reg[SPSR_ABT].I =  reg[17].I;
		break;
	case 0x1b:
		reg[R13_UND].I  = reg[13].I;
		reg[R14_UND].I  = reg[14].I;
		reg[SPSR_UND].I =  reg[17].I;
		break;
	}

	u32 CPSR = reg[16].I;
	u32 SPSR = reg[17].I;

	switch(mode) {
	case 0x10:
	case 0x1F:
		reg[13].I = reg[R13_USR].I;
		reg[14].I = reg[R14_USR].I;
		reg[16].I = SPSR;
		break;
	case 0x11:
		CPUSwap(&reg[8].I, &reg[R8_FIQ].I);
		CPUSwap(&reg[9].I, &reg[R9_FIQ].I);
		CPUSwap(&reg[10].I, &reg[R10_FIQ].I);
		CPUSwap(&reg[11].I, &reg[R11_FIQ].I);
		CPUSwap(&reg[12].I, &reg[R12_FIQ].I);
		reg[13].I = reg[R13_FIQ].I;
		reg[14].I = reg[R14_FIQ].I;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_FIQ].I;
		break;
	case 0x12:
		reg[13].I = reg[R13_IRQ].I;
		reg[14].I = reg[R14_IRQ].I;
		reg[16].I = SPSR;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_IRQ].I;
		break;
	case 0x13:
		reg[13].I = reg[R13_SVC].I;
		reg[14].I = reg[R14_SVC].I;
		reg[16].I = SPSR;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_SVC].I;
		break;
	case 0x17:
		reg[13].I = reg[R13_ABT].I;
		reg[14].I = reg[R14_ABT].I;
		reg[16].I = SPSR;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_ABT].I;
		break;    
	case 0x1b:
		reg[13].I = reg[R13_UND].I;
		reg[14].I = reg[R14_UND].I;
		reg[16].I = SPSR;
		if(saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_UND].I;
		break;    
	default:
		printf("Unsupported ARM mode %02x\n", mode);
		die("Arm error..");
		break;
	}
	armMode = mode;
	CPUUpdateFlags(breakLoop);
	CPUUpdateCPSR();
}

void CPUUpdateCPSR()
{
	u32 CPSR = reg[16].I & 0x40;
	if(N_FLAG)
		CPSR |= 0x80000000;
	if(Z_FLAG)
		CPSR |= 0x40000000;
	if(C_FLAG)
		CPSR |= 0x20000000;
	if(V_FLAG)
		CPSR |= 0x10000000;
	/*if(!armState)
		CPSR |= 0x00000020;*/
	if (!armFiqEnable)
		CPSR |= 0x40;
	if(!armIrqEnable)
		CPSR |= 0x80;
	CPSR |= (armMode & 0x1F);
	reg[16].I = CPSR;
}

void CPUUpdateFlags(bool breakLoop)
{
	u32 CPSR = reg[16].I;

	N_FLAG = (CPSR & 0x80000000) ? true: false;
	Z_FLAG = (CPSR & 0x40000000) ? true: false;
	C_FLAG = (CPSR & 0x20000000) ? true: false;
	V_FLAG = (CPSR & 0x10000000) ? true: false;
	//armState = (CPSR & 0x20) ? false : true;
	armIrqEnable = (CPSR & 0x80) ? false : true;
	armFiqEnable = (CPSR & 0x40) ? false : true;
	if(breakLoop) {
		if(armIrqEnable && (IF & IE) && (IME & 1)) {
			CPU_BREAK_LOOP_2;
		}
	}
}

void CPUSoftwareInterrupt(int comment)
{
	u32 PC = reg[15].I;
	//bool savedArmState = armState;
	CPUSwitchMode(0x13, true, false);
	reg[14].I = PC - 4;
	reg[15].I = 0x08;
	
	armIrqEnable = false;
	armNextPC = 0x08;
	reg[15].I += 4;
}

void CPUUndefinedException()
{
	u32 PC = reg[15].I;
	CPUSwitchMode(0x1b, true, false);
	reg[14].I = PC - 4;
	reg[15].I = 0x04;
	armIrqEnable = false;
	armNextPC = 0x04;
	reg[15].I += 4;  
}

void arm_Reset()
{
	Arm7Enabled = false;
	// clean registers
	memset(&arm_Reg[0], 0, sizeof(arm_Reg));

	IE       = 0x0000;
	IF       = 0x0000;
	IME      = 0x0000;

	armMode = 0x1F;

	reg[13].I = 0x03007F00;
	reg[15].I = 0x0000000;
	reg[16].I = 0x00000000;
	reg[R13_IRQ].I = 0x03007FA0;
	reg[R13_SVC].I = 0x03007FE0;
	armIrqEnable = true;      
	armFiqEnable = false;

	//armState = true;
	C_FLAG = V_FLAG = N_FLAG = Z_FLAG = false;

	// disable FIQ
	reg[16].I |= 0x40;

	CPUUpdateCPSR();

	armNextPC = reg[15].I;
	reg[15].I += 4;

	//arm_FiqPending = false; 
}

void CPUInterrupt()
{
	u32 PC = reg[15].I;
	//bool savedState = armState;
	CPUSwitchMode(0x12, true, false);
	reg[14].I = PC;
	//if(!savedState)
	//	reg[14].I += 2;
	reg[15].I = 0x18;
	//armState = true;
	armIrqEnable = false;

	armNextPC = reg[15].I;
	reg[15].I += 4;
}

void CPUFiq()
{
	u32 PC = reg[15].I;
	//bool savedState = armState;
	CPUSwitchMode(0x11, true, false);
	reg[14].I = PC;
	//if(!savedState)
	//	reg[14].I += 2;
	reg[15].I = 0x1c;
	//armState = true;
	armIrqEnable = false;
	armFiqEnable = false;

	armNextPC = reg[15].I;
	reg[15].I += 4;
}


/*
	--Seems like aica has 3 interrupt controllers actualy (damn lazy sega ..)
	The "normal" one (the one that exists on scsp) , one to emulate the 68k intc , and , 
	of course , the arm7 one

	The output of the sci* bits is input to the e68k , and the output of e68k is inputed into the FIQ
	pin on arm7
*/

void arm_Run(u32 CycleCount)
{
  if (!Arm7Enabled)
	 return;

	u32 clockTicks=0;
	while (clockTicks<CycleCount)
	{
		if (armFiqEnable && e68k_out)
		{
			CPUFiq();
		}

		#include "arm-new.h"
	}


}

void arm_SetEnabled(bool enabled)
{
	if(!Arm7Enabled && enabled)
			arm_Reset();
	
	Arm7Enabled=enabled;
}