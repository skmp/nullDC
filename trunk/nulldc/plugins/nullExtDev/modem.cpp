/*
	This is just wasting space on your hard drive
	Nothing really emulated
*/
#include "modem.h"
#pragma pack(1)
union modemreg_t
{
	u8 ptr[0x21];
	struct
	{
		//00 Receive Data Buffer (RBUFFER)/Voice Receive Data Buffer (VBUFR)
		u8 reg00;
		//01 VOLUME VPAUSE - - TXHF RXHF RXP
		struct
		{
			u8 RXP:1;
			u8 RXHF:1;
			u8 TXHF:1;
			u8 nil2:2;
			u8 VPAUSE:1;
			u8 VOLUME:2;
		} reg01;
		//02 TDE SQDIS S511 - RTSDE V54TE V54AE V54PE 
		//                     DCDEN CDEN - - CODBITS
		struct
		{
			union
			{
				struct 
				{
					u8 V54PE:1;
					u8 V54AE:1;
					u8 V54TE:1;
					u8 RTSDE:1;
					u8 nil1:1;
					u8 S511:1;
				} v0;
				struct 
				{
					u8 CODBITS:2;
					u8 nil2:2;
					u8 CDEN:1;
					u8 DCDEN:1;
				} v1;
				struct
				{
					u8 nila:6;
					u8 TDE:1;
					u8 SQDIS:1;
				};
			};
		} reg02;
		//03 EPT SEPT SRCEN RLSDE - - GTE GTS
		struct
		{
			u8 GTS:1;
			u8 GTE:1;
			u8 nil2:2;
			u8 RLSDE:1;
			u8 SRCEN:1;
			u8 SEPT:1;
			u8 EPT:1;
		} reg03;
		//04 RB - - FIFOEN - NRZIEN/VAGC TOD STRN
		struct
		{
			u8 STRN:1;
			u8 TOD:1;
			u8 NRZIEN_VAGC:1;
			u8 nil1_1:1;
			u8 FIFOEN:1;
			u8 nil1:2;
			u8 RB:1;
		} reg04;
		//05 - - - TXSQ CEQ - STOFF -
		struct
		{
			u8 nil1_2:1;
			u8 STOFF:1;
			u8 nil1_1:1;
			u8 CEQ:1;
			u8 TXSQ:1;
			u8 nil1:3;
		} reg05;
		//06 - EXOS - HDLC PEN STB WDSZ/DECBITS
		struct
		{
			u8 WDSZ_DECBITS:2;
			u8 STB:1;
			u8 PEN:1;
			u8 HDLC:1;
			u8 nil1_1:1;
			u8 EXOS:1;
			u8 nil1:1;
		} reg06;
		//07 RDLE RDL L2ACT - L3ACT - RA MHLD
		struct
		{
			u8 MHLD:1;
			u8 RA:1;
			u8 nil1_1:1;
			u8 L3ACT:1;
			u8 nil1:1;
			u8 L2ACT:1;
			u8 RDL:1;
			u8 RDLE:1;
		} reg07;
		//08 ASYN TPDM V21S V54T V54A V54P RTRN RTS
		struct
		{
			u8 RTS:1;
			u8 RTRN:1;
			u8 V54P:1;
			u8 V54A:1;
			u8 V54T:1;
			u8 V21S:1;
			u8 TPDM:1;
			u8 ASYN:1;
		} reg08;
		//09 NV25 CC DTMF ORG LL DATA RRTSE DTR
		struct
		{
			u8 DTR:1;
			u8 RRTSE:1;
			u8 DATA:1;
			u8 LL:1;
			u8 ORG:1;
			u8 DTMF:1;
			u8 CC:1;
			u8 NV25:1;
		} reg09;
		//0A PNSUC FLAGDT PE FE OE CRCS FLAGS SYNCD
		struct
		{
			u8 SYNCD:1;
			u8 FLAGS:1;
			u8 CRCS:1;
			u8 OE:1;
			u8 FE:1;
			u8 PE:1;
			u8 FLAGDT:1;
			u8 PNSUC:1;
		} reg0a;
		//0B TONEA TONEB TONEC ATV25 ATBEL - DISDET EQMAT
		struct
		{
			u8 EQMAT:1;
			u8 DISDET:1;
			u8 nil:1;
			u8 ATBEL:1;
			u8 ATV25:1;
			u8 TONEC:1;
			u8 TONEB:1;
			u8 TONEA:1;
		} reg0b;
		//0C AADET ACDET CADET CCDET SDET SNDET RXFNE RSEQ
		struct
		{
			u8 RSEQ:1;
			u8 RXFNE:1;
			u8 SNDET:1;
			u8 SDET:1;
			u8 CCDET:1;
			u8 CADET:1;
			u8 ACDET:1;
			u8 AADET:1;
		} reg0c;
		//0D P2DET PNDET S1DET SCR1 U1DET - TXFNF - 
		struct
		{
			u8 nil2:1;
			u8 TXFNF:1;
			u8 nil1:1;
			u8 U1DET:1;
			u8 SCR1:1;
			u8 S1DET:1;
			u8 PNDET:1;
			u8 P2DET:1;
		} reg0d;
		//0E RTDET BRKD RREDT SPEED
		struct
		{
			u8 SPEED:5;
			u8 RREDT:1;
			u8 BRKD:1;
			u8 RTDET:1;
		} reg0e;
		//0F RLSD FED CTS DSR RI TM RTSDT V54DT
		struct
		{
			u8 V54DT:1;
			u8 RTSDT:1;
			u8 TM:1;
			u8 RI:1;
			u8 DSR:1;
			u8 CTS:1;
			u8 FED:1;
			u8 RLSD:1;
		} reg0f;
		//10 Transmit Data Buffer (TBUFFER)/Voice Transmit Buffer (VBUFT)
		u8 reg10;
		//11 BRKS PARSL TXV RXV V23HDX TEOF TXP
		struct
		{
			u8 TXP:1;
			u8 TEOF:1;
			u8 V23HDX:1;
			u8 RXV:1;
			u8 TXV:1;
			u8 PARSL:2;
			u8 BRKS:1;
		} reg11;
		//12 Configuration (CONF)
		u8 reg12;
		//13 TLVL RTH TXCLK
		struct
		{
			u8 TLVL:4;
			u8 RTH:2;
			u8 TXCLK:2;
		} reg13;
		//14 ABCODE
		u8 reg14;
		//15 SLEEP - RDWK HWRWK AUTO RREN EXL3 EARC
		struct
		{
			u8 EARC:1;
			u8 EXL3:1;
			u8 RREN:1;
			u8 AUTO:1;
			u8 HWRWK:1;
			u8 RDWK:1;
			u8 nil:1;
			u8 SLEEP:1;
		} reg15;
		//16 Secondary Receive Data Buffer/V.34 Receive Status (SECRXB)
		u8 reg16;
		//17 Secondary Transmit Data Buffer/V.34 Transmit Status(SECTXB)
		u8 reg17;
		//18 Memory Access Data LSB B7-B0 (MEDAL)
		//19 Memory Access Data MSB B15-B8 (MEDAM)
		u16 reg18_19;
		//1A SFRES RIEN RION DMAE - SCOBF SCIBE SECEN
		struct
		{
			u8 SECEN:1;
			u8 SCIBE:1;
			u8 SCOBF:1;
			u8 nil:1;
			u8 DMAE:1;
			u8 RION:1;
			u8 RIEN:1;
			u8 SFRES:1;
		} reg1a;
		//1B EDET DTDET OTS DTMFD DTMFW
		struct
		{
			u8 DTMFW:4;
			u8 DTMFD:1;
			u8 OTS:1;
			u8 DTDET:1;
			u8 EDET:1;
		} reg1b;
		//1C Memory Access Address Low B7-B0 (MEADDL)
		//1D MEACC - MEMW MEMCR Memory Access Address High B11-B8 (MEADDH)
		struct
		{
			u8 MEMADD_l:8;	//anything else exept u8 breaks it ? WTFH ?
			u8 MEMADD_h:4;
			u8 MEMCR:1;
			u8 MEMW:1;
			u8 nil:1;
			u8 MEACC:1;
		} reg1c_1d;
		//1E TDBIA RDBIA TDBIE - TDBE RDBIE - RDBF
		struct
		{
			u8 RDBF:1;
			u8 nil:1;
			u8 RDBIE:1;
			u8 TDBE:1;
			u8 nil2:1;
			u8 TDBIE:1;
			u8 RDBIA:1;
			u8 TDBIA:1;
		} reg1e;
		//1F NSIA NCIA - NSIE NEWS NCIE - NEWC
		struct
		{
			u8 NEWC:1;
			u8 nil:1;
			u8 NCIE:1;
			u8 NEWS :1;
			u8 NSIE:1;
			u8 nil2:1;
			u8 NCIA:1;
			u8 NSIA:1;
		} reg1f;
	};
};
#define MODEM_COUNTRY_RES 0
#define MODEM_COUNTRY_JAP 1
#define MODEM_COUNTRY_USA 2

#define MODEM_MAKER_SEGA 0
#define MODEM_MAKER_ROCKWELL 1

#define MODEM_DEVICE_TYPE_336K 0


u32 MODEM_ID[2] = 
{
	MODEM_COUNTRY_RES,
	(MODEM_MAKER_ROCKWELL<<4) | (MODEM_DEVICE_TYPE_336K),
};

modemreg_t modem_regs;

enum ModemStates
{
	MS_INVALID,				//needs reset
	MS_RESET,				//reset is low
	MS_RESETING,			//reset is hi
	MS_ST_CONTROLER,		//Controller self test
	MS_ST_DSP,				//DSP self test
	MS_NORMAL,	 			//Normal operation

};
void SetBits(u32 reg,u32 v)
{
	modem_regs.ptr[reg]|=v;
}
void ResetBits(u32 reg,u32 v)
{
	modem_regs.ptr[reg]&=v;
}
void SetMask(u32 reg,u32 v,u32 m)
{
	ResetBits(reg,m);
	SetBits(reg,v);
}
ModemStates state=MS_INVALID;

void NormalDefaultRegs()
{
	verify(state==MS_NORMAL);
	//det default values for normal state
}
void DSPTestEnd()
{
	verify(state==MS_ST_DSP);
	state=MS_NORMAL;

	printf("DSPTestEnd\n");
	printf("Tests Ended -- starting normal operation\n");
	NormalDefaultRegs();
}
void DSPTestStart()
{
	verify(state==MS_ST_DSP);
	state=MS_ST_DSP;
	printf("DSPTestStart\n");
	//Set valid DSP test values sometime
	modem_regs.reg1e.TDBE=1;

	SetUpdateCallback(DSPTestEnd,5);
}
void ControllerTestEnd()
{
	verify(state==MS_ST_CONTROLER);
	state=MS_ST_DSP;

	warn(modem_regs.reg1e.TDBE==0);//This _has_ to be done
	
	printf("ControllerTestEnd\n");
	SetUpdateCallback(DSPTestStart,5);
}

#define SetReg16(rh,rl,v) {modem_regs.ptr[rh]=(v)>>8;modem_regs.ptr[rl]=(v)&0xFF; }
//End the reset and start internal tests
void ControllerTestStart()
{
	verify(state==MS_RESETING);
	//Set Self test values :)
	state=MS_ST_CONTROLER;
	//k, lets set values
	
	//1E:3 -> set
	modem_regs.reg1e.TDBE=1;

/*
	RAM1 Checksum  = 0xEA3C or 0x451
	RAM2 Checksum  = 0x5536 or 0x49A5
	ROM1 Checksum  = 0x5F4C
	ROM2 Checksum  = 0x3835 or 0x3836
	Timer/ROM/RAM  = 0x801 or 0xDE00
	Part Number    = 0x3730 or 0x3731
	Revision Level = 0x4241
*/
	SetReg16(0x1D,0x1C,0xEA3C);
	SetReg16(0x1B,0x1A,0x5536);
	SetReg16(0x19,0x18,0x5F4C);
	SetReg16(0x17,0x16,0x3835);
	SetReg16(0x15,0x14,0x801);
	SetReg16(0x13,0x12,0x3730);
	SetReg16(0x11,0x0,0x4241);

	printf("ControllerTestStart\n");
	SetUpdateCallback(ControllerTestEnd,5);
}

void modem_reset(u32 v)
{
	if (v==0)
	{
		memset(&modem_regs,0,sizeof(modem_regs));
		state=MS_RESET;
		printf("Modem reset start ...\n");
	}
	else
	{
		memset(&modem_regs,0,sizeof(modem_regs));
		state=MS_RESETING;
		modem_regs.ptr[0x20]=1;
		SetUpdateCallback(ControllerTestStart,10);
		printf("Modem reset end ...\n");

	}
}

void ModemNormalWrite(u32 reg,u32 data)
{
	u32 old=modem_regs.ptr[reg];
	modem_regs.ptr[reg]=data;
	switch(reg)
	{
		//Data Write Regs, for transfers to DSP
	case 0x18:
	case 0x19:
		break;

		//Address low 
	case 0x1C:
		break;

	case 0x1D:
		if (modem_regs.reg1c_1d.MEACC)
		{
			modem_regs.reg1c_1d.MEACC=0;
			printf("DSP write %0X=%0X\n",modem_regs.reg1c_1d.MEMADD_l || (modem_regs.reg1c_1d.MEMADD_h<<8),modem_regs.reg18_19 );
		}
		break;
	case 0x1F:
		if (modem_regs.reg1f.NEWC)
		{
			if(modem_regs.reg1a.SFRES)
			{
				printf("Soft Reset SET && NEWC, executing reset and init\n");
				modem_reset(1);
			}
			else
			{
				modem_regs.reg1f.NEWC=0;//accept the settings anyway
				printf("NEWC w/o Reset ?!\n");
			}
		}
		break;
	default:
		printf("ModemNormalWrite : undef %03X=%X\n",reg,data);
	}
}
void ModemCRSTWrite(u32 reg,u32 data)
{
	switch(reg)
	{
	default:
		printf("ModemCRSTWrite : undef %03X=%X\n",reg,data);
	}
}

u32 FASTCALL ModemReadMem_A0_006(u32 addr ,u32 size)
{
	u32 reg=addr&0x7FF;
	verify((reg&3)==0);
	reg>>=2;

	if (reg<0x100)
	{
		verify(reg<=1);
		return MODEM_ID[reg];
	}
	else
	{
		reg-=0x100;
		if (reg<0x21)
		{
			if (state==MS_NORMAL)
			{
				//if (reg==0xF)
				{
					modem_regs.reg0f.CTS=1;
					modem_regs.reg0b.ATBEL=1;
					modem_regs.reg0b.TONEA=1;
					modem_regs.reg0b.TONEB=1;
					modem_regs.reg0b.TONEC=1;
					modem_regs.reg0d.TXFNF=1;
					modem_regs.reg0c.RXFNE=1;


				}
				return modem_regs.ptr[reg];
			}
			else if (state==MS_ST_CONTROLER || state==MS_ST_DSP)
			{
				if (reg==0x10)
				{
					modem_regs.reg1e.TDBE=0;
					return 0;
				}
				else
				{
					//printf("modem reg %03X read -- reset/test state\n",reg);
					return modem_regs.ptr[reg];
				}
			}
			else if (state==MS_RESETING)
			{
				return 0; //still reset
			}
			else
			{
				printf("modem reg %03X read -- undef state %d\n",reg,state);
				return 0xDEADC0DE; 
			} 
		}
		else
		{
			printf("modem reg %03X read -- wtf is it ?\n",reg);
			return 0;
		}
	}
}
void FASTCALL ModemWriteMem_A0_006(u32 addr,u32 data,u32 size)
{
	u32 reg=addr&0x7FF;
	verify((reg&3)==0);
	reg>>=2;

	

	if (reg<0x100)
	{
		verify(reg<=1);
		printf("modem reg %03X write -- MODEM ID?!\n",reg);
	}
	else
	{
		reg-=0x100;
		if (reg<0x20)
		{
			if (state==MS_NORMAL)
			{
				ModemNormalWrite(reg,data);
			}
			else
			{
				printf("modem reg %03X write %X -- undef state?\n",reg,data);
			}
			return;
		}
		else if (reg==0x20)
		{
			//Hard reset (?)
			modem_reset(data);
		}
		else
		{
			printf("modem reg %03X write %X -- wtf is it?\n",reg,data);
			return;
		}
	}
} 

