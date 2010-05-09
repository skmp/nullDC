
#pragma once

class SH4DebugInterface : public DebugInterface
{
public:
	SH4DebugInterface(){} 
	virtual wchar *disasm(unsigned int address);
	virtual int getInstructionSize(int instruction) {return 2;}
	virtual bool isBreakpoint(unsigned int address);
	virtual void setBreakpoint(unsigned int address);
	virtual void clearBreakpoint(unsigned int address);
	virtual void clearAllBreakpoints();
	virtual void toggleBreakpoint(unsigned int address);
	virtual unsigned int readMemory(unsigned int address);
	virtual unsigned int getPC();
	virtual void setPC(unsigned int address);
	virtual void step() {}
	virtual void runToBreakpoint();
	virtual int getColor(unsigned int address);
	virtual wchar *getDescription(unsigned int address);
};


extern SH4DebugInterface di;