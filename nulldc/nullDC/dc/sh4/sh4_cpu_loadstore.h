/*
	sh4 base core
	most of it is (very) old
	could use many cleanups, lets hope someone does them
*/

//this file contains (almost) all load/stores !

//mov.b @(R0,<REG_M>),<REG_N>   
sh4op(i0000_nnnn_mmmm_1100)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ReadMemBOS8(r[n],r[0],r[m]);
} 

//mov.w @(R0,<REG_M>),<REG_N>   
sh4op(i0000_nnnn_mmmm_1101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ReadMemBOS16(r[n],r[0],r[m]);
} 

//mov.l @(R0,<REG_M>),<REG_N>   
sh4op(i0000_nnnn_mmmm_1110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ReadMemBOU32(r[n],r[0],r[m]);
} 

//mov.b <REG_M>,@(R0,<REG_N>)   
sh4op(i0000_nnnn_mmmm_0100)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	WriteMemBOU8(r[0],r[n], r[m]);
}

//mov.w <REG_M>,@(R0,<REG_N>)   
sh4op(i0000_nnnn_mmmm_0101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	WriteMemBOU16(r[0] , r[n], r[m]);
}

//mov.l <REG_M>,@(R0,<REG_N>)   
sh4op(i0000_nnnn_mmmm_0110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	WriteMemBOU32(r[0], r[n], r[m]);
}

//
// 1xxx

//mov.l <REG_M>,@(<disp>,<REG_N>)
sh4op(i0001_nnnn_mmmm_iiii)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	u32 disp = GetImm4(op);

	WriteMemBOU32(r[n] , (disp <<2), r[m]);
}

//
//	2xxx

//mov.b <REG_M>,@<REG_N>        
sh4op(i0010_nnnn_mmmm_0000)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	WriteMemU8(r[n],r[m] );
}

// mov.w <REG_M>,@<REG_N>        
sh4op(i0010_nnnn_mmmm_0001)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	WriteMemU16(r[n],r[m]);
}

// mov.l <REG_M>,@<REG_N>        
sh4op(i0010_nnnn_mmmm_0010)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	WriteMemU32(r[n],r[m]);
}

// mov.b <REG_M>,@-<REG_N>       
sh4op(i0010_nnnn_mmmm_0100)
{
	//iNimp("mov.b <REG_M>,@-<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	WriteMemBOU8(r[n],(u32)-1,r[m]);
	r[n]--;
}

//mov.w <REG_M>,@-<REG_N>       
sh4op(i0010_nnnn_mmmm_0101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	WriteMemBOU16(r[n],(u32)-2,r[m]);
	r[n] -= 2;
}

//mov.l <REG_M>,@-<REG_N>       
sh4op(i0010_nnnn_mmmm_0110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	WriteMemBOU32(r[n],(u32)-4,r[m]);
	r[n] -= 4;
}

//
// 4xxx

//sts.l FPUL,@-<REG_N>          
sh4op(i0100_nnnn_0101_0010)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4,fpul);
	r[n] -= 4;
}

//sts.l MACH,@-<REG_N>          
sh4op(i0100_nnnn_0000_0010)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4,mac.h);
	r[n] -= 4;
}

//sts.l MACL,@-<REG_N>          
sh4op(i0100_nnnn_0001_0010)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4, mac.l);
	r[n] -= 4;
}

//sts.l PR,@-<REG_N>            
sh4op(i0100_nnnn_0010_0010)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4,pr);
	r[n] -= 4;
}

//sts.l DBR,@-<REG_N>            
sh4op(i0100_nnnn_1111_0010)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4,dbr);
	r[n] -= 4;
}

//stc.l GBR,@-<REG_N>           
sh4op(i0100_nnnn_0001_0011)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4, gbr);
	r[n] -= 4;
}

//stc.l VBR,@-<REG_N>           
sh4op(i0100_nnnn_0010_0011)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4, vbr);
	r[n] -= 4;
}

//stc.l SSR,@-<REG_N>           
sh4op(i0100_nnnn_0011_0011)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4, ssr);
	r[n] -= 4;
}

//stc.l SGR,@-<REG_N>           
sh4op(i0100_nnnn_0011_0010)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4, sgr);
	r[n] -= 4;
}

//stc.l SPC,@-<REG_N>           
sh4op(i0100_nnnn_0100_0011)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4, spc);
	r[n] -= 4;
}

//stc RM_BANK,@-<REG_N>         
sh4op(i0100_nnnn_1mmm_0011)
{
	u32 n = GetN(op);
	u32 m = GetM(op) & 0x07;
	
	WriteMemBOU32(r[n],(u32)-4, r_bank[m]);
	r[n] -= 4;
}

//lds.l @<REG_N>+,MACH
sh4op(i0100_nnnn_0000_0110)
{
	u32 n = GetN(op);
	
	ReadMemU32(mac.h,r[n]);
	r[n] += 4;
}

//lds.l @<REG_N>+,MACL
sh4op(i0100_nnnn_0001_0110)
{
	u32 n = GetN(op);
	
	ReadMemU32(mac.l,r[n]);	
	r[n] += 4;
}

//lds.l @<REG_N>+,PR            
sh4op(i0100_nnnn_0010_0110)
{
	u32 n = GetN(op);
	
	ReadMemU32(pr,r[n]);
	r[n] += 4;
}


//lds.l @<REG_N>+,FPUL          
sh4op(i0100_nnnn_0101_0110)
{
	u32 n = GetN(op);	
	
	ReadMemU32(fpul,r[n]);
	r[n] += 4;
}

//lds.l @<REG_N>+,DBR         
sh4op(i0100_nnnn_1111_0110)
{
	u32 n = GetN(op);
	
	ReadMemU32(dbr,r[n]);
	r[n] += 4;
}


//ldc.l @<REG_N>+,GBR           
sh4op(i0100_nnnn_0001_0111)
{
	u32 n = GetN(op);	
	
	ReadMemU32(gbr,r[n]);
	r[n] += 4;
}


//ldc.l @<REG_N>+,VBR           
sh4op(i0100_nnnn_0010_0111)
{
	u32 n = GetN(op);	
	
	ReadMemU32(vbr,r[n]);
	r[n] += 4;
}


//ldc.l @<REG_N>+,SSR           
sh4op(i0100_nnnn_0011_0111)
{
	u32 n = GetN(op);	
	
	ReadMemU32(ssr,r[n]);
	r[n] += 4;
}

//ldc.l @<REG_N>+,SGR           
sh4op(i0100_nnnn_0011_0110)
{
	u32 n = GetN(op);	
	
	ReadMemU32(sgr,r[n]);
	r[n] += 4;
}

//ldc.l @<REG_N>+,SPC           
sh4op(i0100_nnnn_0100_0111)
{
	u32 n = GetN(op);	
	
	ReadMemU32(spc,r[n]);
	r[n] += 4;
}


//ldc.l @<REG_N>+,RM_BANK       
sh4op(i0100_nnnn_1mmm_0111)
{
	u32 n = GetN(op);
	u32 m = GetM(op) & 7;
	
	ReadMemU32(r_bank[m],r[n]);
	r[n] += 4;
}

//
// 5xxx

//mov.l @(<disp>,<REG_M>),<REG_N>
sh4op(i0101_nnnn_mmmm_iiii)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	u32 disp = GetImm4(op) << 2;
	
	ReadMemBOU32(r[n],r[m],disp);
}

//
// 6xxx

//mov.b @<REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_0000)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	ReadMemS8(r[n],r[m]);
} 

//mov.w @<REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_0001)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ReadMemS16(r[n] ,r[m]);
} 

//mov.l @<REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_0010)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	ReadMemU32(r[n],r[m]);
} 

//mov.b @<REG_M>+,<REG_N>       
sh4op(i0110_nnnn_mmmm_0100)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	ReadMemS8(r[n],r[m]);
	if (n != m) 
		r[m] += 1;
} 

//mov.w @<REG_M>+,<REG_N>       
sh4op(i0110_nnnn_mmmm_0101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ReadMemS16(r[n],r[m]);
	if (n != m) 
		r[m] += 2;
} 

//mov.l @<REG_M>+,<REG_N>       
sh4op(i0110_nnnn_mmmm_0110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ReadMemU32(r[n],r[m]);
	if (n != m)
		r[m] += 4;
} 

//
//8xxx

//mov.b R0,@(<disp>,<REG_M>)    
sh4op(i1000_0000_mmmm_iiii)
{
	u32 n = GetM(op);
	u32 disp = GetImm4(op);

	WriteMemBOU8(r[n],disp,r[0]);
}

// mov.w R0,@(<disp>,<REG_M>)    
sh4op(i1000_0001_mmmm_iiii)
{
	u32 disp = GetImm4(op);
	u32 m = GetM(op);

	WriteMemBOU16(r[m] , (disp << 1),r[0]);
}

// mov.b @(<disp>,<REG_M>),R0    
 sh4op(i1000_0100_mmmm_iiii)
{
	u32 disp = GetImm4(op);
	u32 m = GetM(op);

	ReadMemBOS8(r[0] ,r[m] , disp);
}

// mov.w @(<disp>,<REG_M>),R0    
 sh4op(i1000_0101_mmmm_iiii)
{
	u32 disp = GetImm4(op);
	u32 m = GetM(op);

	ReadMemBOS16(r[0],r[m] , (disp << 1));
}

//
// 9xxx

//mov.w @(<disp>,PC),<REG_N>   
 sh4op(i1001_nnnn_iiii_iiii)
{
	u32 n = GetN(op);
	u32 disp = (GetImm8(op));

	ReadMemS16(r[n],(disp<<1) + pc + 4);
}

//
// Cxxx
// mov.b R0,@(<disp>,GBR)        
sh4op(i1100_0000_iiii_iiii)
{
	u32 disp = GetImm8(op); 

	WriteMemBOU8(gbr, disp, r[0]);
}

// mov.w R0,@(<disp>,GBR)        
sh4op(i1100_0001_iiii_iiii)
{
	u32 disp = GetImm8(op);

	WriteMemBOU16(gbr , (disp << 1), r[0]);
}

// mov.l R0,@(<disp>,GBR)        
sh4op(i1100_0010_iiii_iiii)
{
	u32 disp = (GetImm8(op));

	WriteMemBOU32(gbr,(disp << 2), r[0]);
}

// mov.b @(<disp>,GBR),R0        
sh4op(i1100_0100_iiii_iiii)
{
	u32 disp = GetImm8(op);

	ReadMemBOS8(r[0],gbr,disp);
}

// mov.w @(<disp>,GBR),R0        
sh4op(i1100_0101_iiii_iiii)
{
	u32 disp = GetImm8(op);

	ReadMemBOS16(r[0],gbr,(disp<<1));
}

// mov.l @(<disp>,GBR),R0        
sh4op(i1100_0110_iiii_iiii)
{
	u32 disp = GetImm8(op);
	
	ReadMemBOU32(r[0],gbr,(disp<<2));
}

//
// Dxxx

// mov.l @(<disp>,PC),<REG_N>    
sh4op(i1101_nnnn_iiii_iiii)
{
	u32 n = GetN(op);
	u32 disp = (GetImm8(op));
	
	ReadMemU32(r[n],(disp<<2) + (pc & 0xFFFFFFFC) + 4);
}

//
// Exxx

//movca.l R0, @<REG_N>          
sh4op(i0000_nnnn_1100_0011)
{
	u32 n = GetN(op);

	WriteMemU32(r[n],r[0]);
}

#ifdef _INTERPRETER_COMPILE_
//All memory opcodes are here
//fmov.s @(R0,<REG_M>),<FREG_N>
sh4op(i1111_nnnn_mmmm_0110)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		ReadMemU32(fr_hex[n],r[m] + r[0]);
	}
	else
	{
		u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			ReadMemU64(fr_hex[n],r[m] + r[0]);
		}
		else
		{
			ReadMemU64(xf_hex[n],r[m] + r[0]);
		}
	}
}

//fmov.s <FREG_M>,@(R0,<REG_N>)
sh4op(i1111_nnnn_mmmm_0111)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		WriteMem32(r[0] + r[n], fr_hex[m]);
	}
	else
	{
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;
		if (((op >> 4) & 0x1) == 0)
		{
			WriteMemU64(r[n] + r[0],fr_hex[m]);
		}
		else
		{
			WriteMemU64(r[n] + r[0],xf_hex[m]);
		}
	}
}

//fmov.s @<REG_M>,<FREG_N> 
sh4op(i1111_nnnn_mmmm_1000)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		ReadMemU32(fr_hex[n],r[m]);
	}
	else
	{
		u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			ReadMemU64(fr_hex[n],r[m]);
		}
		else
		{
			ReadMemU64(xf_hex[n],r[m]);
		}
	}
}

//fmov.s @<REG_M>+,<FREG_N>
sh4op(i1111_nnnn_mmmm_1001)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		ReadMemU32(fr_hex[n],r[m]);
		r[m] += 4;
	}
	else 
	{
		u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			ReadMemU64(fr_hex[n],r[m]);
		}
		else
		{
			ReadMemU64(xf_hex[n],r[m] );
		}
		r[m] += 8;
	}
}

//fmov.s <FREG_M>,@<REG_N>
sh4op(i1111_nnnn_mmmm_1010)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		WriteMemU32(r[n], fr_hex[m]);
	}
	else
	{
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;

		if (((op >> 4) & 0x1) == 0)
		{
			WriteMemU64(r[n], fr_hex[m]);
		}
		else
		{
			WriteMemU64(r[n], xf_hex[m]);
		}
		
	}
}

//fmov.s <FREG_M>,@-<REG_N>
sh4op(i1111_nnnn_mmmm_1011)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		u32 ea = r[n] - 4;

		WriteMemU32(ea, fr_hex[m]);

		r[n] = ea;
	}
	else
	{
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;

		u32 ea = r[n] - 8;
		if (((op >> 4) & 0x1) == 0)
		{
			WriteMemU64(ea , fr_hex[m]);
		}
		else
		{
			WriteMemU64(ea , xf_hex[m]);
		}

		r[n] = ea;
	}
}

#endif