////////////////////////////////////////////////////////////////////////////////////////////

#include "nullDC_GUI.h"

//#include "dc/mem/memutil.h"
//#include "dc/mem/sh4_mem.h"
//#include "plugins/plugin_manager.h"
//#include "dc/maple/maple_if.h"
//#include "dc/sh4/sh4_cst.h"
//#include "dc/mem/Elf.h"

#include "DebugInterface.h"
#include "SH4DbgInterface.h"

////////////////////////
#include <vector>
using namespace std;

 #define BPT_ENABLED		0x0001

typedef struct
{
	u32 dwAddr;	// Address to break on
	u16 wFlags;	// 
	u16 wSaved;	// Saved SH4 Instruction (use this for ptr address on another arch)

} Bpt, BreakPoint;


vector<Bpt> bpt;

int Invector(u32 dwAddr, vector<Bpt> vec)
{
	for (u32 indx = 0; indx < vec.size(); indx++)
		if(vec[indx].dwAddr == dwAddr)
			return indx;
	return -1;
}


//[22:57] <@CalcMe> josuttis = The C++ Standard Library, by Josuttis, ISBN (0-201-37926-0)
//is arguably the best current tutorial on proper usage of the ANSI Standard C++ Library (See also: Effective STL)
////////////////////////

////////////////////
wchar szDisBuf[256];////////
wchar *SH4DebugInterface::disasm(unsigned int address) 
{
		u16 dwOp = ReadMem16(address);// *FIXME*
		//DisSH4_Instr[nyb0(dwOp)]( dwOp, address, szDisBuf );

		DissasembleOpcode(dwOp,address,szDisBuf);
		return szDisBuf;
}
unsigned int SH4DebugInterface::readMemory(unsigned int address)
{
	return ReadMem32(address);//MemReadAddr(address);
//	return CMemory::ReadUnchecked_U32(address);
}


bool SH4DebugInterface::isBreakpoint(unsigned int address) 
{
	int i = Invector(address, bpt);

	if( i >= 0 )
		return (bpt[i].wFlags&BPT_ENABLED) ? true : false ;

	return false;
}


void SH4DebugInterface::clearAllBreakpoints() {}

void SH4DebugInterface::setBreakpoint(unsigned int address) {
	toggleBreakpoint(address);	//CBreakPoints::AddBreakPoint(address);
}
void SH4DebugInterface::clearBreakpoint(unsigned int address) {
	toggleBreakpoint(address);	//CBreakPoints::RemoveBreakPoint(address);
}


#include <windows.h>
extern HWND hDebugger;
void RefreshDebugger(HWND hWnd);



void SH4DebugInterface::toggleBreakpoint(unsigned int address)
{
	int i = Invector(address, bpt);

	if( i >= 0 )
	{
		if(bpt[i].wFlags&BPT_ENABLED)
		{
			bpt[i].wFlags &= ~BPT_ENABLED;
			WriteMem16(bpt[i].dwAddr, bpt[i].wSaved);

			printf("BPT: Changing %X to disabled %X\n", bpt[i].dwAddr, bpt[i].wFlags);
		}
		else
		{
			bpt[i].wFlags |= BPT_ENABLED;
			bpt[i].wSaved = ReadMem16(bpt[i].dwAddr);
			WriteMem16(bpt[i].dwAddr, BPT_OPCODE);

			printf("BPT: Changing %X to enabled %X\n", bpt[i].dwAddr, bpt[i].wFlags);
		}
	//	bpt.erase(iter);	//	bpt.remove(brk);	// 
	}
	else
	{
		Bpt brk;
		brk.dwAddr = address;
		brk.wFlags = BPT_ENABLED;
		brk.wSaved = ReadMem16(brk.dwAddr);
		WriteMem16(brk.dwAddr, BPT_OPCODE);	// must make rom writeable for this, else no brom bpts

		printf("BPT: Adding %X to vector\n", brk.dwAddr);

		bpt.push_back(brk);
	}


	PostMessage(hDebugger,WM_MOVE,NULL,NULL);
//	RefreshDebugger(hDebugger);	// why doesn't this work 

	// TODO get debugger to refresh here ;)
	//CBreakPoints::IsAddressBreakPoint(address)?CBreakPoints::RemoveBreakPoint(address):CBreakPoints::AddBreakPoint(address);
}

int curcol=0;
int SH4DebugInterface::getColor(unsigned int address)
{
	int colors[7] = {0xe0FFFF,0xFFe0e0,0xe8e8FF,0xFFe0FF,0xe0FFe0,0xFFFFe0,0xAeE0FF};
//	int n=Debugger_GetSymbolNum(address);
//	if (n==-1) return 0xFFFFFF;
	return colors[curcol&3];//address>>1&3]; // colors[n%6];
}

////////////////////
////////////////////


/*extern u8 symtab[];
extern u8 strtab[];

extern u32 symindex;
extern bool bElfLoaded;
extern Elf32_Sym * pSymTab;*/

static wchar szDesc[256];
wchar *SH4DebugInterface::getDescription(unsigned int address) 
{
	static u32 last=0;
	if( last==address )
		return szDesc;
	else
		last=address;

	u32 dwOp = ReadMem16(address);
	u32 sz=swprintf(szDesc, L"0x%04X      ", dwOp);
	GetSymbName(address,&szDesc[sz],false);
	/*
	if(!bElfLoaded || ((symindex>>16)==0) )
	{
		u32 dwOp = ReadMem16(address);//MemRead16(address);	// *FIXME*
		sprintf(szDesc, "0x%04X", dwOp);
	} else
	{
		for(u32 i=0; i<(symindex>>16); i++)	{
			if( (address&0xFFFFFF) == (pSymTab[i].st_value&0xFFFFFF) )
			{
				curcol = (curcol++==6) ? 0 : curcol;
				sprintf(szDesc, "%s v:%X", (char*)(strtab+pSymTab[i].st_name), pSymTab[i].st_value);
				return szDesc;
			}
		}
		sprintf(szDesc, "\"");
	}*/
	return szDesc;//Debugger_GetDescription(address);
}

unsigned int SH4DebugInterface::getPC() 
{
	return Sh4GetRegister(reg_pc);//PowerPC::PC;
}

void SH4DebugInterface::setPC(unsigned int address) 
{
	// *FIXME* this should prob. make sure delay slot is 0 too
	Sh4SetRegister(reg_pc,address);
	//pc = address;
//	PowerPC::PC=address;
}

void SH4DebugInterface::runToBreakpoint() 
{}