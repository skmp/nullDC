#include "types.h"

int serial_cmdl(wchar** arg,int cl);
int slave_cmdl(wchar** arg,int cl);
//void PrintSerialIPUsage(int argc, char *argv[]);
void WriteSerial(u8 data);
bool PendingSerialData();
s32 ReadSerial();
float GetRxSpeed();
float GetTxSpeed();
//this is a fake dma handler
//to initiate it write size to 0
//pointer to 4
//0xdeadc0de to 8
void WriteBlockSerial(u8* blk,u32 size,u8* sum);