/*
	Command line parsing
	~yay~

	Nothing too interesting here, really
*/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "config/config.h"
#include "serial_ipc/serial_ipc_client.h"
#include "log/logging_interface.h"

wchar* trim_ws(wchar* str)
{
	if (str==0 || wcslen(str)==0)
		return 0;

	while(*str)
	{
		if (!iswspace(*str))
			break;
		str++;
	}

	size_t l=wcslen(str);
	
	if (l==0)
		return 0;

	while(l>0)
	{
		if (!iswspace(str[l-1]))
			break;
		str[l-1]=0;
		l--;
	}

	if (l==0)
		return 0;

	return str;
}

int setconfig(wchar** arg,int cl)
{
	int rv=0;
	for(;;)
	{
		if (cl<1)
		{
			printf("-config : invalid number of parameters, format is section:key=value\n");
			return rv;
		}
		
		wchar* sep=wcsstr(arg[1],L":");
		if (sep==0)
		{
			wprintf(L"-config : invalid parameter %s, format is section:key=value\n",arg[1]);
			return rv;
		}
		wchar* value=wcsstr(sep+1,L"=");
		if (value==0)
		{
			wprintf(L"-config : invalid parameter %s, format is section:key=value\n",arg[1]);
			return rv;
		}

		*sep++=0;
		*value++=0;

		wchar* sect=trim_ws(arg[1]);
		wchar* key=trim_ws(sep);
		value=trim_ws(value);

		if (sect==0 || key==0)
		{
			printf("-config : invalid parameter, format is section:key=value\n");
			return rv;
		}

		if (value==0)
			value=L"";
			
		wprintf(L"Virtual cfg %s:%s=%s\n",sect,key,value);

		cfgSetVitual(sect,key,value);
		rv++;

		if (cl>=3 && _tcsicmp(arg[2],L",")==0)
		{
			cl-=2;
			arg+=2;
			rv++;
			continue;
		}
		else
			break;
	}
	return rv;
}
int setconfigfile(wchar** arg,int cl)
{
	if (cl<1)
	{
		printf("-configfile : invalid number of parameters, format is -configfile <file>\n");
		return 0;
	}
	wcscpy(cfgPath,arg[1]);
	return 1;
}
int showhelp(wchar** arg,int cl)
{
	if (cl>=1)
	{
		if (_tcsicmp(arg[1],L"config")==0)
		{
			printf("-config	section:key=value [, ..]: add a virtual config value\n Virtual config values wont be saved to the .cfg file\n unless a different value is writen to em\nNote :\n You can specify many settings in the xx:yy=zz , gg:hh=jj , ...\n format.The spaces betwen the values and ',' are needed.");
			return 1;
		}
		else if (_tcsicmp(arg[1],L"serial")==0)
		{
			printf("-serial	<filename>              : serial redirection\nfilename:\n can be a file (ex. c:\\myfile.txt,\"c:\\ spaceses .txt\")\n or a COM port (ex. \\\\COM1)\n");
			return 1;
		}
		else if (_tcsicmp(arg[1],L"slave")==0)
		{
			printf("-slave  <piperead> <pipewrite>  : serial redirection, slave ipc mode\npiperead :\n Pipe to read serial data from\npipewrite :\n Pipe to write serial data to\n");
			return 1;
		}
		else if (_tcsicmp(arg[1],L"configfile")==0)
		{
			printf("-configfile <file>              : use <file> instead of nullDC.cfg\nfile : \n a file to use as nullDC config.If not existing\n it will be created.Any changes to the config\n will be saved there.");
			return 1;
		}
		
	}

	printf("Available commands :\n");
	
	printf("-help [config|serial|slave]     : additional for the specified command\n");
	printf("-config section:key=value [, ..]: add a virtual config value\n");
	printf("-configfile <file>              : use <file> instead of nullDC.cfg\n");
	printf("-serial	<filename>              : serial redirection\n");
	printf("-slave <piperead> <pipewrite>   : slave ipc mode\n");
	printf("-about                          : Shows info about the emu\n");

	return 0;
}
bool ParseCommandLine(int argc,wchar* argv[])
{

	int cl=argc-2;
	wchar** arg=argv+1;
	while(cl>=0)
	{
		if (_tcsicmp(*arg,L"-help")==0)
		{
			int as=showhelp(arg,cl);
			cl-=as;
			arg+=as;
			return true;
		}
		else if (_tcsicmp(*arg,L"-config")==0)
		{
			int as=setconfig(arg,cl);
			cl-=as;
			arg+=as;
		}
		else if (_tcsicmp(*arg,L"-configfile")==0)
		{
			int as=setconfigfile(arg,cl);
			cl-=as;
			arg+=as;
		}
		else if (_tcsicmp(*arg,L"-slave")==0)
		{
			int as=slave_cmdl(arg,cl);
			cl-=as;
			arg+=as;
		}
		else if (_tcsicmp(*arg,L"-serial")==0)
		{
			int as=serial_cmdl(arg,cl);
			cl-=as;
			arg+=as;
		}
		else if (_tcsicmp(*arg,L"-about")==0)
		{
			printf("nullDC, a Sega Dreamcast Emulator.Made by the nullDC team -- www.emudev.com/nullDC\n");
			wprintf(L"Version : " VER_FULLNAME L" \n");
			
			return true;
		}
		else
		{
			wprintf(L"wtf %s is suposed to do ?\n",*arg);
		}
		arg++;
		cl--;
	}
	printf("\n");
	return false;
}