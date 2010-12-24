#include "Devices.h"

#ifdef BUILD_NAOMI

_NaomiState State;

char Joy_strName[64] = "NAOMI Controller\0";
char Joy_strBrand_2[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";

char EEPROM[0x100];
bool EEPROM_loaded=false;

void printState(u32 cmd,u32* buffer_in,u32 buffer_in_len)
{
	printf("Command : 0x%X",cmd);
	if (buffer_in_len>0)
		printf(",Data : %d bytes\n",buffer_in_len);
	else
		printf("\n");
	buffer_in_len>>=2;
	while(buffer_in_len-->0)
	{
		printf("%08X ",*buffer_in++);
		if (buffer_in_len==0)
			printf("\n");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// NAOMI JAMMA DMA
// ---------------
u32 FASTCALL ControllerDMA_NAOMI(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	u8*buffer_out_b=(u8*)buffer_out;
	u8*buffer_in_b=(u8*)buffer_in;
	buffer_out_len=0;

	u16 kcode[4]={0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

	if(joysticks[0].enabled)
		GetJoyStatus(0);
	else
		memset(joystate[0].control, 0, sizeof(joystate[0].control));

	if(joysticks[1].enabled)	
		GetJoyStatus(1);
	else
		memset(joystate[1].control, 0, sizeof(joystate[1].control));

	switch (Command)
	{
	case 0x86:
		{
			u32 subcode=*(u8*)buffer_in;

			switch(subcode)
			{
			case 0x15:
			case 0x33:
				{
					buffer_out[0]=0xffffffff;
					buffer_out[1]=0xffffffff;					

					if(joystate[0].control[CTLN_SERVICE2] || 
					   joystate[1].control[CTLN_SERVICE2])	buffer_out[0]&=~(1<<0x1b);
					
					if(joystate[0].control[CTLN_TEST2] || 
					   joystate[1].control[CTLN_TEST2])		buffer_out[0]&=~(1<<0x1a);
					
					if(State.Mode==0 && subcode!=0x33)	//Get Caps
					{
						buffer_out_b[0x11+1]=0x8E;	//Valid data check
						buffer_out_b[0x11+2]=0x01;
						buffer_out_b[0x11+3]=0x00;
						buffer_out_b[0x11+4]=0xFF;
						buffer_out_b[0x11+5]=0xE0;
						buffer_out_b[0x11+8]=0x01;

						switch(State.Cmd)
						{
							//Reset, in : 2 bytes, out : 0
							case 0xF0:
								break;

							//Find nodes?
							//In addressing Slave address, in : 2 bytes, out : 1
							case 0xF1:
								{
									buffer_out_len=4*4;
								}
								break;

							//Speed Change, in : 2 bytes, out : 0
							case 0xF2:
								break;

							//Name
							//"In the I / O ID" "Reading each slave ID data"
							//"NAMCO LTD.; I / O PCB-1000; ver1.0; for domestic only, no analog input"
							//in : 1 byte, out : max 102
							case 0x10:
								{
									static char ID1[102]="nullDC Team; I/O Plugin-1; ver0.2; for nullDC or other emus";
									buffer_out_b[0x8+0x10]=(BYTE)strlen(ID1)+3;
									for(int i=0;ID1[i]!=0;++i)
									{
										buffer_out_b[0x8+0x13+i]=ID1[i];
									}
								}
								break;

							//CMD Version
							//REV in command|Format command to read the (revision)|One|Two 
							//in : 1 byte, out : 2 bytes
							case 0x11:
								{
									buffer_out_b[0x8+0x13]=0x13;
								}
								break;

							//JVS Version
							//In JV REV|JAMMA VIDEO standard reading (revision)|One|Two 
							//in : 1 byte, out : 2 bytes
							case 0x12:
								{
									buffer_out_b[0x8+0x13]=0x30;
								}
								break;

							//COM Version
							//VER in the communication system|Read a communication system compliant version of |One|Two
							//in : 1 byte, out : 2 bytes
							case 0x13:
								{
									buffer_out_b[0x8+0x13]=0x10;
								}
								break;

							//Features
							//Check in feature |Each features a slave to read |One |6 to
							//in : 1 byte, out : 6 + (?)
							case 0x14:
								{
									unsigned char *FeatPtr=buffer_out_b+0x8+0x13;
									buffer_out_b[0x8+0x9+0x3]=0x0;
									buffer_out_b[0x8+0x9+0x9]=0x1;
									#define ADDFEAT(Feature,Count1,Count2,Count3)	*FeatPtr++=Feature; *FeatPtr++=Count1; *FeatPtr++=Count2; *FeatPtr++=Count3;
									ADDFEAT(1,2,12,0);	//Feat 1=Digital Inputs.  2 Players. 10 bits
									ADDFEAT(2,2,0,0);	//Feat 2=Coin inputs. 2 Inputs
									ADDFEAT(3,2,0,0);	//Feat 3=Analog. 2 Chans

									ADDFEAT(0,0,0,0);	//End of list
								}
								break;

							default:
								printf("unknown CAP %X\n",State.Cmd);
						}
						buffer_out_len=4*4;
					}
					else if(State.Mode==1 || State.Mode==2 || subcode==0x33)	//Get Data
					{
						unsigned char glbl=0x00;
						unsigned char p1_1=0x00;
						unsigned char p1_2=0x00;
						unsigned char p2_1=0x00;
						unsigned char p2_2=0x00;	
						static unsigned char LastKey[256];
						static unsigned short coin1=0x0000;
						static unsigned short coin2=0x0000;	
						
						if(joystate[0].control[CTLN_TEST1] || 
						   joystate[1].control[CTLN_TEST1])   glbl|=0x80;

						if(joystate[0].control[CTLN_SERVICE1])	  p1_1|=0x40;																										
						if(joystate[0].control[CTLN_START])   p1_1|=0x80; 
						
						if(joystate[0].control[CTLN_D_UP]   ) p1_1|=0x20;
						if(joystate[0].control[CTLN_D_DOWN] ) p1_1|=0x10;
						if(joystate[0].control[CTLN_D_LEFT] ) p1_1|=0x08;
						if(joystate[0].control[CTLN_D_RIGHT]) p1_1|=0x04;						
												
						if(joystate[0].control[CTLN_BUTTON1]) p1_1|=0x02;
						if(joystate[0].control[CTLN_BUTTON2]) p1_1|=0x01;
						if(joystate[0].control[CTLN_BUTTON3]) p1_2|=0x80;
						if(joystate[0].control[CTLN_BUTTON4]) p1_2|=0x40;																			
						if(joystate[0].control[CTLN_BUTTON5]) p1_2|=0x20;
						if(joystate[0].control[CTLN_BUTTON6]) p1_2|=0x10;

						// Player 2
						if(joystate[1].control[CTLN_SERVICE1])	  p2_1|=0x40;																										
						if(joystate[1].control[CTLN_START])   p2_1|=0x80; 
						
						if(joystate[1].control[CTLN_D_UP]   ) p2_1|=0x20;
						if(joystate[1].control[CTLN_D_DOWN] ) p2_1|=0x10;
						if(joystate[1].control[CTLN_D_LEFT] ) p2_1|=0x08;
						if(joystate[1].control[CTLN_D_RIGHT]) p2_1|=0x04;						
												
						if(joystate[1].control[CTLN_BUTTON1]) p2_1|=0x02;
						if(joystate[1].control[CTLN_BUTTON2]) p2_1|=0x01;
						if(joystate[1].control[CTLN_BUTTON3]) p2_2|=0x80;
						if(joystate[1].control[CTLN_BUTTON4]) p2_2|=0x40;																			
						if(joystate[1].control[CTLN_BUTTON5]) p2_2|=0x20;
						if(joystate[1].control[CTLN_BUTTON6]) p2_2|=0x10;
							
						static bool old_coin1 = false;
						static bool old_coin2 = false;

						if(!old_coin1 && joystate[0].control[CTLN_COIN]) // Coin key
						{
							coin1++;
							old_coin1 = true;
						}	
						else if(old_coin1 && !joystate[0].control[CTLN_COIN]) // Coin key
							old_coin1 = false;
						
						if(!old_coin2 && joystate[1].control[CTLN_COIN]) // Coin key
						{
							coin2++;
							old_coin2 = true;
						}
						else if(old_coin2 && !joystate[1].control[CTLN_COIN]) // Coin key													
							old_coin2 = false;
						

						buffer_out_b[0x11+0]=0x00;
						buffer_out_b[0x11+1]=0x8E;	//Valid data check
						buffer_out_b[0x11+2]=0x01;
						buffer_out_b[0x11+3]=0x00;
						buffer_out_b[0x11+4]=0xFF;
						buffer_out_b[0x11+5]=0xE0;
						buffer_out_b[0x11+8]=0x01;

						//memset(OutData+8+0x11,0x00,0x100);

						buffer_out_b[8+0x12+0]=1;
						buffer_out_b[8+0x12+1]=glbl;
						buffer_out_b[8+0x12+2]=p1_1;
						buffer_out_b[8+0x12+3]=p1_2;						
						buffer_out_b[8+0x12+4]=p2_1;
						buffer_out_b[8+0x12+5]=p2_2;	
						buffer_out_b[8+0x12+6]=1;
						buffer_out_b[8+0x12+7]=coin1>>8;
						buffer_out_b[8+0x12+8]=coin1&0xff;						
						buffer_out_b[8+0x12+9]=coin2>>8;
						buffer_out_b[8+0x12+10]=coin2&0xff;						
						buffer_out_b[8+0x12+11]=1;
						buffer_out_b[8+0x12+12]=0x00;
						buffer_out_b[8+0x12+13]=0x00;
						buffer_out_b[8+0x12+14]=0x00;
						buffer_out_b[8+0x12+15]=0x00;
						buffer_out_b[8+0x12+16]=0x00;
						buffer_out_b[8+0x12+17]=0x00;
						buffer_out_b[8+0x12+18]=0x00;
						buffer_out_b[8+0x12+19]=0x00;
						buffer_out_b[8+0x12+20]=0x00;						

						if(State.Mode==1)
						{
							buffer_out_b[0x11+0x7]=19;
							buffer_out_b[0x11+0x4]=19+5;
						}
						else
						{
							buffer_out_b[0x11+0x7]=17;
							buffer_out_b[0x11+0x4]=17-1;
						}

						//OutLen=8+0x11+16;
						buffer_out_len=8+0x12+20;
					}
				}
				return 8;
					
			case 0x17:	//Select Subdevice
				{
					State.Mode=0;
					State.Cmd=buffer_in_b[8];
					State.Node=buffer_in_b[9];
					buffer_out_len=0;
				}
				return (7);
				
			case 0x27:	//Transfer request
				{
					State.Mode=1;
					State.Cmd=buffer_in_b[8];
					State.Node=buffer_in_b[9];
					buffer_out_len=0;
				}
				return (7);
			case 0x21:		//Transfer request with repeat
				{
					State.Mode=2;
					State.Cmd=buffer_in_b[8];
					State.Node=buffer_in_b[9];
					buffer_out_len=0;
				}
				return (7);

			case 0x0B:	//EEPROM write
				{
					int address=buffer_in_b[1];
					int size=buffer_in_b[2];
					//printf("EEprom write %08X %08X\n",address,size);
					//printState(Command,buffer_in,buffer_in_len);
					memcpy(EEPROM+address,buffer_in_b+4,size);

					wchar eeprom_file[512];
					host.ConfigLoadStr(L"emu",L"gamefile",eeprom_file,L"");
					wcscat_s(eeprom_file,L".eeprom");
					FILE* f;
					_wfopen_s(&f,eeprom_file,L"wb");
					if (f)
					{
						fwrite(EEPROM,1,0x80,f);
						fclose(f);
						wprintf(L"SAVED EEPROM to %s\n",eeprom_file);
					}
				}
				return (7);
			case 0x3:	//EEPROM read
				{
					if (!EEPROM_loaded)
					{
						EEPROM_loaded=true;
						wchar eeprom_file[512];
						host.ConfigLoadStr(L"emu",L"gamefile",eeprom_file,L"");
						wcscat_s(eeprom_file,L".eeprom");
						FILE* f;
						_wfopen_s(&f,eeprom_file,L"rb");
						if (f)
						{
							fread(EEPROM,1,0x80,f);
							fclose(f);
							wprintf(L"LOADED EEPROM from %s\n",eeprom_file);
						}
					}
					//printf("EEprom READ ?\n");
					int address=buffer_in_b[1];
					//printState(Command,buffer_in,buffer_in_len);
					memcpy(buffer_out,EEPROM+address,0x80);
					buffer_out_len=0x80;
				}
				return 8;
				//IF I return all FF, then board runs in low res
			case 0x31:
				{
					buffer_out[0]=0xffffffff;
					buffer_out[1]=0xffffffff;
				}
				return (8);
								
			default:
				printf("Unknown 0x86 : SubCommand 0x%X - State: Cmd 0x%X Mode :  0x%X Node : 0x%X\n",subcode,State.Cmd,State.Mode,State.Node);
				printState(Command,buffer_in,buffer_in_len);
			}

			return 8;//MAPLE_RESPONSE_DATATRF
		}
		break;
	case 0x82:
		{
			const char *ID="315-6149    COPYRIGHT SEGA E\x83\x00\x20\x05NTERPRISES CO,LTD.  ";
			memset(buffer_out_b,0x20,256);
			memcpy(buffer_out_b,ID,0x38-4);
			buffer_out_len=256;
			return (0x83);
		}
	/*typedef struct {
		DWORD		func;//4
		DWORD		function_data[3];//3*4
		u8		area_code;//1
		u8		connector_direction;//1
		char		product_name[30];//30*1
		char		product_license[60];//60*1
		WORD		standby_power;//2
		WORD		max_power;//2
	} maple_devinfo_t;*/
	case 1: 
		{
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;


			//caps
			//4
			w32(1 << 24);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				if (Joy_strName[i]!=0)
				{
					w8((u8)Joy_strName[i]);
				}
				else
				{
					w8(0x20);
				}
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				if (Joy_strBrand_2[i]!=0)
				{
					w8((u8)Joy_strBrand_2[i]);
				}
				else
				{
					w8(0x20);
				}
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0xAE01); 

			//2
			w16(0xF401); 
		}
		return 5;

	/* controller condition structure 
	typedef struct {//8 bytes
	WORD buttons;			///* buttons bitfield	/2
	u8 rtrig;			///* right trigger			/1
	u8 ltrig;			///* left trigger 			/1
	u8 joyx;			////* joystick X 			/1
	u8 joyy;			///* joystick Y				/1
	u8 joy2x;			///* second joystick X 		/1
	u8 joy2y;			///* second joystick Y 		/1
	} cont_cond_t;*/
	case 9:
		{
			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32(1 << 24);
			//struct data
			//2
			w16(kcode[0] | 0xF901); 
				
			//trigger			
			w8(0x0);			
			w8(0x0); 
			
			// Analog XY
			w8(0x80); 			
			w8(0x80); 
			
			// XY2
			w8(0x80); 			
			w8(0x80); 

			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
		}
		return 8;

	default:
		printf("unknown MAPLE Frame\n");
		printState(Command,buffer_in,buffer_in_len);
		return 7;		
	}
}
#endif