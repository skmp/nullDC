#include "codespan.h"
#include "dc/mem/sh4_mem.h"

//start page , olny valid if in ram
u32 CodeSpan::page_start()
{
	return GetRamPageFromAddress(start);
}
//end page , olny valid if in ram
u32 CodeSpan::page_end()
{
	return GetRamPageFromAddress(end);
}
//True if block is on ram :)
bool CodeSpan::OnRam()
{
	//code can't start and end on diff mem areas , so olny check start :)
	return IsOnRam(start);
}
//Retuns the size of the code span (in bytes)
u32 CodeSpan::Size()
{
	return end-start+2;
}

//Returns the opcode count of the code span
u32 CodeSpan::OpcodeCount()
{
	return Size()>>1;
}


//Returns the Page count of the code span [olny valid if in vram]
u32 CodeSpan::PageCount()
{
	verify(OnRam());
	return page_end()-page_start()+1;
}
//Checks if this CodeSpan contains an adress.Valid olny if on block is on ram and address is on ram :)
bool CodeSpan::Contains(u32 address,u32 sz)
{
	verify(OnRam() && IsOnRam(address));
	u32 r_address=address&RAM_MASK;
	u32 r_start=start&RAM_MASK;
	u32 r_end=end&RAM_MASK;

	return r_start<=r_address && (r_address+sz)<=r_end;
}

//checks if this CodeSpan contains another Codespan
bool CodeSpan::Contains(CodeSpan* to)
{
	verify(OnRam() && to->OnRam());
	return Contains(to->start,2) && Contains(to->end,2);
}

//Checks if this CodeSpan contains an adress , counting in page units.
bool CodeSpan::ContainsPage(u32 address)
{
	verify(OnRam() && IsOnRam(address));
	u32 r_address=GetRamPageFromAddress(address);
	u32 r_start=page_start();
	u32 r_end=page_end();

	return r_start<=r_address && r_address<=r_end;
}

//Checks if this CodeSpan contains another code span , counting in page units.
bool CodeSpan::ContainsPage(CodeSpan* to)
{
	verify(OnRam() && to->OnRam());
	return ContainsPage(to->start) && ContainsPage(to->end);
}

//Creates a Union w/ the CodeSpan , assuming this CodeSpan is on ram.The to codespan contains decoded ram offsets
void CodeSpan::Union(CodeSpan* to)
{
	verify(OnRam());
	to->start=min(to->start&RAM_MASK,start&RAM_MASK);
	to->start=max(to->end&RAM_MASK,end&RAM_MASK);
}

//Checks if this CodeSpan Intersects w/ another , assuming both are on ram
bool CodeSpan::Intersect(CodeSpan* to)
{
	verify(OnRam() && to->OnRam());
	u32 cr_start=start&RAM_MASK;
	u32 cr_end=end&RAM_MASK;

	u32 tr_start=to->start&RAM_MASK;
	u32 tr_end=to->end&RAM_MASK;

	//in order not to inersect , the block must be before  (this-> start < to-> end) or after (this->end <to->start)
	return  !(cr_start<tr_end || cr_end<tr_start);
}