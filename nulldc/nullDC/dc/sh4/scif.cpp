/*
	Dreamcast serial port.
	This is missing most of the functionality, but works for KOS (And thats all that uses it)
*/
#include "types.h"
#include "dc/mem/sh4_internal_reg.h"
#include "scif.h"
#include "serial_ipc/serial_ipc_client.h"

//SCIF SCSMR2 0xFFE80000 0x1FE80000 16 0x0000 0x0000 Held Held Pclk
SCSMR2_type SCIF_SCSMR2;

//SCIF SCBRR2 0xFFE80004 0x1FE80004 8 0xFF 0xFF Held Held Pclk
u8 SCIF_SCBRR2;

//SCIF SCSCR2 0xFFE80008 0x1FE80008 16 0x0000 0x0000 Held Held Pclk
SCSCR2_type SCIF_SCSCR2;

//SCIF SCFTDR2 0xFFE8000C 0x1FE8000C 8 Undefined Undefined Held Held Pclk
u8 SCIF_SCFTDR2;

//SCIF SCFSR2 0xFFE80010 0x1FE80010 16 0x0060 0x0060 Held Held Pclk
SCSCR2_type SCIF_SCFSR2;

//SCIF SCFRDR2 0xFFE80014 0x1FE80014 8 Undefined Undefined Held Held Pclk
//Read OLNY
u8 SCIF_SCFRDR2;

//SCIF SCFCR2 0xFFE80018 0x1FE80018 16 0x0000 0x0000 Held Held Pclk
SCFCR2_type SCIF_SCFCR2;

//Read OLNY
//SCIF SCFDR2 0xFFE8001C 0x1FE8001C 16 0x0000 0x0000 Held Held Pclk
SCFDR2_type SCIF_SCFDR2;

//SCIF SCSPTR2 0xFFE80020 0x1FE80020 16 0x0000 0x0000 Held Held Pclk
SCSPTR2_type SCIF_SCSPTR2;

//SCIF SCLSR2 0xFFE80024 0x1FE80024 16 0x0000 0x0000 Held Held Pclk
SCLSR2_type SCIF_SCLSR2;


void SerialWrite(u32 data)
{
	WriteSerial((u8)data);
	//putc(data,stdout);
}

//SCIF_SCFSR2 read
u32 ReadSerialStatus()
{
	if (PendingSerialData())
	{
		return 0x60 | 2;
	}
	else
	{
		return 0x60| 0;
	}
	/*
	//TODO : Add status for serial input
	return 0x60;//hackish but works !
	*/
}

//SCIF_SCFDR2 - 16b
u32 Read_SCFDR2()
{
	return 0;
}
//SCIF_SCFRDR2
u32 ReadSerialData()
{
	s32 rd=ReadSerial();
	return (u8)rd ;
}

//Init term res
void scif_Init()
{
	//SCIF SCSMR2 0xFFE80000 0x1FE80000 16 0x0000 0x0000 Held Held Pclk
	SCIF[(SCIF_SCSMR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	SCIF[(SCIF_SCSMR2_addr&0xFF)>>2].readFunction=0;
	SCIF[(SCIF_SCSMR2_addr&0xFF)>>2].writeFunction=0;
	SCIF[(SCIF_SCSMR2_addr&0xFF)>>2].data16=&SCIF_SCSMR2.full;

	//SCIF SCBRR2 0xFFE80004 0x1FE80004 8 0xFF 0xFF Held Held Pclk
	SCIF[(SCIF_SCBRR2_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	SCIF[(SCIF_SCBRR2_addr&0xFF)>>2].readFunction=0;
	SCIF[(SCIF_SCBRR2_addr&0xFF)>>2].writeFunction=0;
	SCIF[(SCIF_SCBRR2_addr&0xFF)>>2].data8=&SCIF_SCBRR2;

	//SCIF SCSCR2 0xFFE80008 0x1FE80008 16 0x0000 0x0000 Held Held Pclk
	SCIF[(SCIF_SCSCR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	SCIF[(SCIF_SCSCR2_addr&0xFF)>>2].readFunction=0;
	SCIF[(SCIF_SCSCR2_addr&0xFF)>>2].writeFunction=0;
	SCIF[(SCIF_SCSCR2_addr&0xFF)>>2].data16=&SCIF_SCSCR2.full;

	//Write olny 
	//SCIF SCFTDR2 0xFFE8000C 0x1FE8000C 8 Undefined Undefined Held Held Pclk
	SCIF[(SCIF_SCFTDR2_addr&0xFF)>>2].flags=REG_8BIT_READWRITE;//call the write callback
	SCIF[(SCIF_SCFTDR2_addr&0xFF)>>2].readFunction=0;
	SCIF[(SCIF_SCFTDR2_addr&0xFF)>>2].writeFunction=SerialWrite;
	SCIF[(SCIF_SCFTDR2_addr&0xFF)>>2].data8=&SCIF_SCFTDR2;

	//SCIF SCFSR2 0xFFE80010 0x1FE80010 16 0x0060 0x0060 Held Held Pclk
	SCIF[(SCIF_SCFSR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_WRITE_DATA;
	SCIF[(SCIF_SCFSR2_addr&0xFF)>>2].readFunction=ReadSerialStatus;
	SCIF[(SCIF_SCFSR2_addr&0xFF)>>2].writeFunction=0;
	SCIF[(SCIF_SCFSR2_addr&0xFF)>>2].data16=&SCIF_SCFSR2.full;

	//READ OLNY
	//SCIF SCFRDR2 0xFFE80014 0x1FE80014 8 Undefined Undefined Held Held Pclk
	SCIF[(SCIF_SCFRDR2_addr&0xFF)>>2].flags=REG_8BIT_READWRITE ;
	SCIF[(SCIF_SCFRDR2_addr&0xFF)>>2].readFunction=ReadSerialData;
	SCIF[(SCIF_SCFRDR2_addr&0xFF)>>2].writeFunction=0;
	SCIF[(SCIF_SCFRDR2_addr&0xFF)>>2].data8=&SCIF_SCFRDR2;

	//SCIF SCFCR2 0xFFE80018 0x1FE80018 16 0x0000 0x0000 Held Held Pclk
	SCIF[(SCIF_SCFCR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	SCIF[(SCIF_SCFCR2_addr&0xFF)>>2].readFunction=0;
	SCIF[(SCIF_SCFCR2_addr&0xFF)>>2].writeFunction=0;
	SCIF[(SCIF_SCFCR2_addr&0xFF)>>2].data16=&SCIF_SCFCR2.full;

	//Read OLNY
	//SCIF SCFDR2 0xFFE8001C 0x1FE8001C 16 0x0000 0x0000 Held Held Pclk
	SCIF[(SCIF_SCFDR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE;
	SCIF[(SCIF_SCFDR2_addr&0xFF)>>2].readFunction=Read_SCFDR2;
	SCIF[(SCIF_SCFDR2_addr&0xFF)>>2].writeFunction=0;
	SCIF[(SCIF_SCFDR2_addr&0xFF)>>2].data16=&SCIF_SCFDR2.full;

	//SCIF SCSPTR2 0xFFE80020 0x1FE80020 16 0x0000 0x0000 Held Held Pclk
	SCIF[(SCIF_SCSPTR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	SCIF[(SCIF_SCSPTR2_addr&0xFF)>>2].readFunction=0;
	SCIF[(SCIF_SCSPTR2_addr&0xFF)>>2].writeFunction=0;
	SCIF[(SCIF_SCSPTR2_addr&0xFF)>>2].data16=&SCIF_SCSPTR2.full;

	//SCIF SCLSR2 0xFFE80024 0x1FE80024 16 0x0000 0x0000 Held Held Pclk
	SCIF[(SCIF_SCLSR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	SCIF[(SCIF_SCLSR2_addr&0xFF)>>2].readFunction=0;
	SCIF[(SCIF_SCLSR2_addr&0xFF)>>2].writeFunction=0;
	SCIF[(SCIF_SCLSR2_addr&0xFF)>>2].data16=&SCIF_SCLSR2.full;
}
void scif_Reset(bool Manual)
{
	/*
	SCIF SCSMR2 H'FFE8 0000 H'1FE8 0000 16 H'0000 H'0000 Held Held Pclk
	SCIF SCBRR2 H'FFE8 0004 H'1FE8 0004 8 H'FF H'FF Held Held Pclk
	SCIF SCSCR2 H'FFE8 0008 H'1FE8 0008 16 H'0000 H'0000 Held Held Pclk
	SCIF SCFTDR2 H'FFE8 000C H'1FE8 000C 8 Undefined Undefined Held Held Pclk
	SCIF SCFSR2 H'FFE8 0010 H'1FE8 0010 16 H'0060 H'0060 Held Held Pclk
	SCIF SCFRDR2 H'FFE8 0014 H'1FE8 0014 8 Undefined Undefined Held Held Pclk
	SCIF SCFCR2 H'FFE8 0018 H'1FE8 0018 16 H'0000 H'0000 Held Held Pclk
	SCIF SCFDR2 H'FFE8 001C H'1FE8 001C 16 H'0000 H'0000 Held Held Pclk
	SCIF SCSPTR2 H'FFE8 0020 H'1FE8 0020 16 H'0000*2 H'0000*2 Held Held Pclk
	SCIF SCLSR2 H'FFE8 0024 H'1FE8 0024 16 H'0000 H'0000 Held Held Pclk
	*/
	SCIF_SCSMR2.full=0x0000;
	SCIF_SCBRR2=0xFF;
	SCIF_SCFSR2.full=0x000;
	SCIF_SCFCR2.full=0x000;
	SCIF_SCFDR2.full=0x000;
	SCIF_SCSPTR2.full=0x000;
	SCIF_SCLSR2.full=0x000;
}
void scif_Term()
{
}

