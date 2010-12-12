
#include "types.h"
#include <windows.h>
#include <assert.h>
#include <iostream>

#define sipcver "0.3 -spdma -buffered"

HANDLE readp=NULL;
HANDLE writep=NULL;
#define buff_size (512*1024)
u8 ReadBuff[buff_size];

u32 readind=0;
u32 ReadSz=0;
u32 serial_RxBytes=0;
u32 serial_TxBytes=0;
u32 last_serial_RxBytes=0;
u32 last_serial_TxBytes=0;
u32 last_serial_Rx_tick=0;
u32 last_serial_Tx_tick=0;

void StartGDBSession();

int serial_cmdl(wchar** arg,int cl)
{
	if (cl<1)
		printf("-serial : too few params.-serial <FILE> must be used\n");

	if (writep)
		printf("-serial : -serial/-slave allready used ...\n");

	wprintf(_T("Serial port to \"%s\" redirection version %s\n"),arg[1],sipcver);
	writep = CreateFile(arg[1],GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL ,NULL );
	if (writep==INVALID_HANDLE_VALUE)
	{
		wprintf(_T("Unable to open \"%\".The file must exist \n"),arg[1]);
		writep=NULL;
		return 0;
	}
	DCB dcbConfig;

	if(GetCommState(writep, &dcbConfig))
	{
		dcbConfig.BaudRate = 115200;
		dcbConfig.ByteSize = 8;
		dcbConfig.Parity = NOPARITY;
		dcbConfig.StopBits = ONESTOPBIT;
		dcbConfig.fBinary = TRUE;
		dcbConfig.fParity = TRUE;
	}

	SetCommState(writep, &dcbConfig);

	COMMTIMEOUTS to;
	GetCommTimeouts(writep,&to);
	to.ReadIntervalTimeout=MAXDWORD;
	to.ReadTotalTimeoutConstant=0;
	to.ReadTotalTimeoutMultiplier=0;

	to.WriteTotalTimeoutConstant=1000000;
	to.WriteTotalTimeoutMultiplier=5;

	SetCommTimeouts(writep,&to);

	readp=writep;
	
	return 1;
}
int slave_cmdl(wchar** arg,int cl)
{
	if (cl<2)
		printf("-slave : too few params.-slave <piperead> <pipewrite> must be used\n");

	//TODO : how to do on 64b compatable code ?
	HANDLE laddWrSlave = (HANDLE)(u64)_wtoi(arg[1]);
	HANDLE laddRdSlave = (HANDLE)(u64)_wtoi(arg[2]);
	if (laddWrSlave ==0 || laddRdSlave==0)
	{
		printf("Invalid param @ slave, pipe cant be 0/non number\n");
		return 0;
	}
	log("Value of write handle to pipe1: %p\n",laddWrSlave);
	log("Value of read handle to pipe2 : %p\n",laddRdSlave);

	//this warning can't be fixed can it ?
	writep = laddWrSlave;
	readp = laddRdSlave;

	return 2;
}
void PrintSerialIPUsage(int argc, wchar *argv[])
{
	/*
	if (argc==1)
	{
		printf("If you want to use serial port ipc redirection use -slave piperead pipewrite/-serial FILE_NAME \n");
		//StartGDBSession();
		return;
	}

	if (argc!=4 && argc!=3)
	{
		printf("Serial port pipe redirection version %s\n",sipcver);
		printf("Wrong number of parameters , expecting  nulldc -slave piperead pipewrite/-serial FILE_NAME \n");
		printf("redirection disabled");
		//StartGDBSession();
		return;
	}
*/
}
void WriteBlockSerial(u8* blk,u32 size,u8* sum)
{
	serial_TxBytes+=size;
	*sum=0; 
	for (u32 i=0;i<size;i++)
			*sum^=blk[i];
	if (!writep)
	{
		for (u32 i=0;i<size;i++)
			putc(blk[i],stdout);
		return;
	}

	//printf("Write IPC not implemented");
	DWORD dwWritten=0;
	
	if (!WriteFile(writep,blk,size,&dwWritten,NULL))
		log("IPC error");

	if (dwWritten!=size)
		log("IPC error");
}
void WriteSerial(u8 data)
{
	serial_TxBytes++;

	putc(data,stdout);

	if (!writep)	
		return;

	//log("Write IPC not implemented");
	DWORD dwWritten=0;
	
	if (!WriteFile(writep,&data,1,&dwWritten,NULL))
		log("IPC error");

	if (dwWritten!=1)
		log("IPC error");
}

int pend_temp=0;
bool PendingSerialData()
{
	if(!readp)
		return false;

	if (readind==0)
	{
		/*pend_temp++;
		if ((pend_temp%8)!=0)
			return false;*/
		
		/*
		
		if (((i++)%256)==0)
			PeekNamedPipe(readp,NULL,NULL,NULL,&tba,NULL);
		*/

		DWORD tba=0;
		//ReadFile(readp, &ReadBuff, 0, &tba, NULL);
		if(ReadFile(readp, &ReadBuff, buff_size, &tba, NULL) != 0)
        {
            if(tba > 0)
            {
				ReadSz=tba;
				readind=tba;
            }
        }
		/*
		if (tba!=0)
		{
			readind=tba;
			if (readind>(buff_size)) readind=buff_size;
			ReadFile(readp,ReadBuff,readind,&tba,NULL);
			ReadSz=tba;
			if (readind!=tba)
				log("IPC ERROR \n");
		}
		*/
		return tba!=0;
	}
	else
	{
		return true;
	}
}
s32 ReadSerial()
{
	serial_RxBytes++;
	if (!readp)
		return -1;

	//u8 read_data;
	//DWORD dwRead;
	if (readind<=0)
	{
		log("IPC error");
		return -1;
	}
	
	u8 rv= ReadBuff[ReadSz-readind];
	readind--;
	return rv;
/*
	if (!ReadFile(readp,&read_data,1,&dwRead,NULL))
		log("IPC error");
	if (dwRead!=1)
		log("IPC error");
	//log("Read IPC not implemented");
	return read_data;*/
	
}



float GetRxSpeed()
{
	double tp=((double)timeGetTime()-(double)last_serial_Rx_tick)/1000.0;
	float rv=(float)((serial_RxBytes-last_serial_RxBytes)/tp);
	last_serial_RxBytes=serial_RxBytes;
	last_serial_Rx_tick=timeGetTime();
	return rv;
}

float GetTxSpeed()
{
	double tp=((double)timeGetTime()-(double)last_serial_Tx_tick)/1000.0;
	float rv=(float)((serial_TxBytes-last_serial_TxBytes)/tp);
	last_serial_TxBytes=serial_TxBytes;
	last_serial_Tx_tick=timeGetTime();
	return rv;
}