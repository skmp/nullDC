/*
	Glue code for sh4 interface stuff
*/
#include "types.h"
#include "sh4_if.h"
#include "sh4_interpreter.h"
#include "rec_v1\driver.h"

//sh4 exeption saved stack pointer :)
u32* sh4_exept_ssp;
//sh4 next opcode execute
u32* sh4_exept_next;
//sh4 exeption raised bool , set to true when an exeption was raised , and its not handled
bool sh4_exept_raised=false;

u8 GetSingleFromDouble(u8 dbl)
{
	if (dbl>=dr_0 && dbl<=dr_7)
	{
		u8 res=(u8)(dbl-dr_0);
		return (u8)(fr_0+(res<<1));
	}
	else if (dbl>=xd_0 && dbl<=xd_7)
	{
		u8 res=(u8)(dbl-xd_0);
		return (u8)(xf_0+(res<<1));
	}

	log("GetSingleFromDouble : WRONG ID %X\n",dbl);

	return reg_xmtrx;//error :P
}

bool IsReg64(Sh4RegType reg)
{
	if (reg>=dr_0 && reg<=dr_7)
		return true;

	if (reg>=xd_0 && reg<=xd_7)
		return true;

	return false;
}
//what to put here ?
//Ahh i know i know :P

//Get an interface to sh4 interpreter
sh4_if* Get_Sh4Interpreter()
{
	sh4_if* rv=(sh4_if*)malloc(sizeof(sh4_if));
	
	rv->Run=Sh4_int_Run;
	rv->Stop=Sh4_int_Stop;
	rv->Step=Sh4_int_Step;
	rv->Skip=Sh4_int_Skip;
	rv->Reset=Sh4_int_Reset;
	rv->Init=Sh4_int_Init;
	rv->Term=Sh4_int_Term;
	rv->IsCpuRunning=Sh4_int_IsCpuRunning;
	rv->GetRegister=Sh4_int_GetRegister;
	rv->SetRegister=Sh4_int_SetRegister;
	//rv->RaiseInterrupt=RaiseInterrupt_;
	rv->RaiseExeption=sh4_int_RaiseExeption;
	rv->ResetCache=0;
	return rv;
}

//Get an interface to sh4 dynarec
sh4_if* Get_Sh4Recompiler()
{
	sh4_if* rv=(sh4_if*)malloc(sizeof(sh4_if));
	
	rv->Run=rec_Sh4_int_Run;
	rv->Stop=rec_Sh4_int_Stop;
	rv->Step=rec_Sh4_int_Step;
	rv->Skip=rec_Sh4_int_Skip;
	rv->Reset=rec_Sh4_int_Reset;
	rv->Init=rec_Sh4_int_Init;
	rv->Term=rec_Sh4_int_Term;
	rv->IsCpuRunning=rec_Sh4_int_IsCpuRunning;
	rv->GetRegister=Sh4_int_GetRegister;
	rv->SetRegister=Sh4_int_SetRegister;
	//rv->RaiseInterrupt=RaiseInterrupt_;
	rv->RaiseExeption=rec_sh4_int_RaiseExeption;
	rv->ResetCache=rec_sh4_ResetCache;
	return rv;
}

void Release_Sh4If(sh4_if* cpu)
{
	free(cpu);
}