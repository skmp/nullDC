/*
	sh4 base core
	most of it is (very) old
	could use many cleanups, lets hope someone does them
*/

//Bitwise logical operations
//

//and <REG_M>,<REG_N>           
 sh4op(i0010_nnnn_mmmm_1001)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] &= r[m];
}

//xor <REG_M>,<REG_N>           
 sh4op(i0010_nnnn_mmmm_1010)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] ^= r[m];
}

//or <REG_M>,<REG_N>            
 sh4op(i0010_nnnn_mmmm_1011)
{
	u32 n = GetN(op);
	u32 m = GetM(op);	
	r[n] |= r[m];
}


//shll2 <REG_N>                 
 sh4op(i0100_nnnn_0000_1000)
{
	u32 n = GetN(op);
	r[n] <<= 2;
}


//shll8 <REG_N>                 
 sh4op(i0100_nnnn_0001_1000)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	r[n] <<= 8;
}


//shll16 <REG_N>                
 sh4op(i0100_nnnn_0010_1000)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	r[n] <<= 16;
}


//shlr2 <REG_N>                 
 sh4op(i0100_nnnn_0000_1001)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	r[n] >>= 2;
}


//shlr8 <REG_N>                 
 sh4op(i0100_nnnn_0001_1001)
{
	//iNimp("shlr8 <REG_N>");
	u32 n = GetN(op);
	r[n] >>= 8;
}


//shlr16 <REG_N>                
 sh4op(i0100_nnnn_0010_1001)
{
	u32 n = GetN(op);
	r[n] >>= 16;
}

// and #<imm>,R0                 
 sh4op(i1100_1001_iiii_iiii)
{//ToDo : Check This [26/4/05]
	//iNimp("and #<imm>,R0");
	u32 imm = GetImm8(op);
	r[0] &= imm;
}


// xor #<imm>,R0                 
 sh4op(i1100_1010_iiii_iiii)
{
	//iNimp("xor #<imm>,R0");
	u32  imm  = GetImm8(op);
	r[0] ^= imm;
}


// or #<imm>,R0                  
 sh4op(i1100_1011_iiii_iiii)
{//ToDo : Check This [26/4/05]
	//iNimp("or #<imm>,R0");
	u32 imm = GetImm8(op);
	r[0] |= imm;
} 


 
//TODO : move it somewhere better
//nop                           
sh4op(i0000_0000_0000_1001)
{
	//no operation xD XD .. i just love this opcode ..
	//what ? you expected something fancy or smth ?
} 

