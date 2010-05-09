/*
	sh4 base core
	most of it is (very) old
	could use many cleanups, lets hope someone does them
*/

#pragma once
#include "types.h"
#include "sh4_interpreter.h"

//fadd <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0000);
//fsub <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0001);
//fmul <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0010);
//fdiv <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0011);
//fcmp/eq <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0100);
//fcmp/gt <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0101);
//fmov.s @(R0,<REG_M>),<FREG_N>
sh4op(i1111_nnnn_mmmm_0110);
//fmov.s <FREG_M>,@(R0,<REG_N>)
sh4op(i1111_nnnn_mmmm_0111);
//fmov.s @<REG_M>,<FREG_N> 
sh4op(i1111_nnnn_mmmm_1000);
//fmov.s @<REG_M>+,<FREG_N>
sh4op(i1111_nnnn_mmmm_1001);
//fmov.s <FREG_M>,@<REG_N>
sh4op(i1111_nnnn_mmmm_1010);
//fmov.s <FREG_M>,@-<REG_N>
sh4op(i1111_nnnn_mmmm_1011);
//fmov <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_1100);
//fabs <FREG_N>            
sh4op(i1111_nnnn_0101_1101);
// FSCA FPUL, DRn//F0FD//1111_nnnn_1111_1101
sh4op(i1111_nnn0_1111_1101);
//fcnvds <DR_N>,FPUL       
sh4op(i1111_nnnn_1011_1101);
//fcnvsd FPUL,<DR_N>       
sh4op(i1111_nnnn_1010_1101);
//fipr <FV_M>,<FV_N>            
sh4op(i1111_nnmm_1110_1101);
//fldi0 <FREG_N>           
sh4op(i1111_nnnn_1000_1101);
//fldi1 <FREG_N>           
sh4op(i1111_nnnn_1001_1101);
//flds <FREG_N>,FPUL       
sh4op(i1111_nnnn_0001_1101);
//float FPUL,<FREG_N>      
sh4op(i1111_nnnn_0010_1101);
//fneg <FREG_N>            
sh4op(i1111_nnnn_0100_1101);
//frchg                    
sh4op(i1111_1011_1111_1101);
//fschg                    
sh4op(i1111_0011_1111_1101);
//fsqrt <FREG_N>                
sh4op(i1111_nnnn_0110_1101);
//ftrc <FREG_N>, FPUL      
sh4op(i1111_nnnn_0011_1101);
//fsts FPUL,<FREG_N>       
sh4op(i1111_nnnn_0000_1101);
//fmac <FREG_0>,<FREG_M>,<FREG_N> 
sh4op(i1111_nnnn_mmmm_1110);
//ftrv xmtrx,<FV_N>       
sh4op(i1111_nn01_1111_1101);
//FSRRA
sh4op(i1111_nnnn_0111_1101);