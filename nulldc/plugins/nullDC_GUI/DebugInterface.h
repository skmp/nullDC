#pragma once


class DebugInterface
{
public:
	virtual wchar *disasm(unsigned int address) {return L"NODEBUGGER";}
	virtual int getInstructionSize(int instruction) {return 1;}

	virtual bool isBreakpoint(unsigned int address) {return false;}
	virtual void setBreakpoint(unsigned int address){}
	virtual void clearBreakpoint(unsigned int address){}
	virtual void clearAllBreakpoints() {}
	virtual void toggleBreakpoint(unsigned int address){}
	virtual unsigned int readMemory(unsigned int address){return 0;}
	virtual unsigned int getPC() {return 0;}
	virtual void setPC(unsigned int address) {}
	virtual void step() {}
	virtual void runToBreakpoint() {}
	virtual int getColor(unsigned int address){return 0xFFFFFFFF;}
	virtual wchar *getDescription(unsigned int address) {return L"";}
};
