#pragma once
#include "types.h"

struct CodeSpan
{
public:
	//start pc
	u32 start;

	//end pc
	u32 end;

	//True if block is on ram :)
	bool OnRam();

	//Retuns the size of the code span (in bytes)
	u32 Size();
	//Returns the opcode count of the code span
	u32 OpcodeCount();
	//Returns the Page count of the code span [olny valid if in vram]
	u32 PageCount();
	//start page , olny valid if in ram
	u32 page_start();
	//end page , olny valid if in ram
	u32 page_end();

	//Checks if this CodeSpan contains an adress.Valid olny if on block is on ram and address is on ram :)
	bool Contains(u32 address,u32 sz);
	//checks if this CodeSpan contains another Codespan
	bool Contains(CodeSpan* to);
	//Checks if this CodeSpan contains an adress , counting in page units.
	bool ContainsPage(u32 address);
	//Checks if this CodeSpan contains another code span , counting in page units.
	bool ContainsPage(CodeSpan* to);

	//Creates a Union w/ the CodeSpan , assuming this CodeSpan is on ram.The to codespan contains decoded ram offsets
	void Union(CodeSpan* to);
	//Checks if this CodeSpan Intersects w/ another , assuming both are on ram
	bool Intersect(CodeSpan* to);
};

struct CodeRegion : public CodeSpan
{
public:
	//cycle count
	u32 cycles;
};
