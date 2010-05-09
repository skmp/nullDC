/*
	sh4 base core
	most of it is (very) old
	could use many cleanups, lets hope someone does them
*/

// sub <REG_M>,<REG_N>           
 sh4op(i0011_nnnn_mmmm_1000)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	//rn=(s32)r[n];
	//rm=(s32)r[m];
	r[n] -=r[m];
	//r[n]=(u32)rn;
}

//add <REG_M>,<REG_N>           
 sh4op(i0011_nnnn_mmmm_1100)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] +=r[m];
}

 //
// 7xxx

//add #<imm>,<REG_N>
 sh4op(i0111_nnnn_iiii_iiii)
{
	u32 n = GetN(op);
	s32 stmp1 = GetSImm8(op);
	r[n] +=stmp1;
}
