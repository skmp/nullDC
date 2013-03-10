/*
**	Elf.cpp * Executable & Linkable Format (ELF) Support *
*	*****************************************************
*	ZeZu[2/11/04] Rev.1 ZeZu[8/14/05]
*/

#include "types.h"

#include <memory.h>
#include <string.h>
#include <stdio.h>

#include "sh4_mem.h"
#include "Elf.h"


u8 shstrtab[0x800];
u8 symtab[0x80000];
u8 strtab[0x80000];

u32 symindex=0;
Elf32_Sym * pSymTab = (Elf32_Sym *)symtab;
bool bSymbolInfo;

//address : Address to get teh sumbol name (if any) ,szDesc a char* ptr to store the data , returns string size
int GetSymbName(u32 address,char *szDesc,bool bUseUnkAddress)
{
	address&=0x1fffffff;
	if (((address>>26)&0x7)!=3)//not on Ram area , no sumbols :P
	{
		if (bUseUnkAddress)
			return sprintf(szDesc, "%x",address);
		else
			return 0;
	}

	if(!bSymbolInfo || ((symindex>>16)==0) )
	{
		//if no info
		if (bUseUnkAddress)
			return sprintf(szDesc, "%x",address);
		else
			return 0;
	} 
	else
	{
		for(u32 i=0; i<(symindex>>16); i++)	{
			if( (address&RAM_MASK) == (pSymTab[i].st_value&RAM_MASK) )
			{
				return sprintf(szDesc, "%s v:%X", (char*)(strtab+pSymTab[i].st_name), pSymTab[i].st_value);
			}
		}
	}
	return sprintf(szDesc, "%x",address);
}

bool LoadELF( wchar* szFileName )
{
	FILE *fElf = _tfopen( szFileName, L"rb" );
	if( !fElf )
		return false;

	fseek(fElf, 0, SEEK_END);
	u32 flen = ftell(fElf);
	fseek(fElf, 0, SEEK_SET);

	if( (flen < sizeof(Elf32_Ehdr)) || (flen > 0x400000) )	// >4MB not likely
	{
		fclose(fElf);
		return false;
	}

	unsigned char * pElf = new unsigned char [ flen ] ;
	if(!pElf)
	{
		fclose(fElf);
		return false;
	}

	if( fread(pElf, 1, flen, fElf) != flen )
	{
		fclose(fElf);
		delete[] pElf;
		return false;
	}

	fclose(fElf);

	///////////////////////////////////////////////////////////////////////////////////////////

	log("-> ELF Load(): Opened: %s (%d bytes)\n", szFileName, flen );

	Elf32_Ehdr *pEhdr = (Elf32_Ehdr *)pElf;

	if( (pEhdr->e_ident[0] != 0x7F) || (pEhdr->e_ident[1] != 0x45) ||	// 0x74 | 'E'
		(pEhdr->e_ident[2] != 0x4C) || (pEhdr->e_ident[3] != 0x46) ||	// 'L'  | 'F'
		(pEhdr->e_ident[EI_DATA] != ELFDATA2LSB)	||
		(pEhdr->e_ident[EI_CLASS] != ELFCLASS32)	||  
		(pEhdr->e_ident[EI_VERSION] != EV_CURRENT)	|| 
		(pEhdr->e_type != ET_EXEC)	||
		(pEhdr->e_machine != EM_SH)	||
		(pEhdr->e_version != EV_CURRENT) ) {
		log("-> ELF ERROR: This ELF File is NOT for Hitachi SuperH Series Processors! (Dreamcast)\n");
		delete [] pElf;
		return false;
	}

	if( (pEhdr->e_shentsize != sizeof(Elf32_Shdr)) ||
		(pEhdr->e_phentsize != sizeof(Elf32_Phdr)) ||
		(flen<(pEhdr->e_shoff+(pEhdr->e_shentsize*pEhdr->e_shnum))) ||
		(flen<(pEhdr->e_phoff+(pEhdr->e_phentsize*pEhdr->e_phnum))) )
	{
		delete [] pElf;
		return false;
	}

	u32 i=0;
	Elf32_Shdr *pShdr = (Elf32_Shdr *)pElf + pEhdr->e_shoff;
	//Elf32_Phdr *pPhdr = (Elf32_Phdr *)pElf + pEhdr->e_phoff; -> not used for now

	log("ELF: entry:%X shnum:%X shoff:%X \n", pEhdr->e_entry, pEhdr->e_shnum, pEhdr->e_shoff);


	//	take care of symtab first
	//
	for( i=0; i<pEhdr->e_shnum; i++ )
	{
		pShdr = (Elf32_Shdr *)((u8*)pElf + pEhdr->e_shoff + pEhdr->e_shentsize * i);

		if( SHT_SYMTAB == pShdr->sh_type )
		{
			symindex = pShdr->sh_link;
			symindex |= ((pShdr->sh_size / pShdr->sh_entsize) << 16);

			if(pShdr->sh_size < 0x80000)
				memcpy(symtab,(pElf+pShdr->sh_offset),pShdr->sh_size);
			bSymbolInfo=true;
		}
	}

	//	take care of strtab second
	//
	for( i=0; i<pEhdr->e_shnum; i++ )
	{
		pShdr = (Elf32_Shdr *)((u8*)pElf + pEhdr->e_shoff + pEhdr->e_shentsize * i);

		if( SHT_STRTAB == pShdr->sh_type )
		{
			if( (symindex&0xFFFF) == i ) 
			{
				if(pShdr->sh_size < 0x80000) 
				{
					log("ELF: Found .strtab!\n");
					memcpy(strtab,(pElf+pShdr->sh_offset),pShdr->sh_size);
				}
			} else
			{
				// can have more than one strtab, other than symtab/strtab too so check to see if its index into itself is ".shstrtab"
				char *szT = (char*)(pElf + pShdr->sh_offset + pShdr->sh_name);

				if(0==strcmp(szT,".shstrtab"))
					if(pShdr->sh_size < 0x800) 
					{
						log("ELF: Found .shstrtab!\n");
						memcpy(shstrtab,(pElf+pShdr->sh_offset),pShdr->sh_size);
					}
			}
		}
	}

	//	take care of the rest
	//
	for( i=0; i<pEhdr->e_shnum; i++ )
	{
		pShdr = (Elf32_Shdr *)((u8*)pElf + pEhdr->e_shoff + pEhdr->e_shentsize * i);

		log("Segment[%i]: \"%s\" type:%X flags:%X offs:%X size:%X \n", i, 
			(char*)(shstrtab+pShdr->sh_name), pShdr->sh_type, pShdr->sh_flags, pShdr->sh_offset, pShdr->sh_size);

		switch( pShdr->sh_type )
		{
		case SHT_NULL:		break;
		case SHT_NOBITS:	break;
		case SHT_STRTAB:	break;
		case SHT_SYMTAB:	break;

		case SHT_PROGBITS:
			if( pShdr->sh_flags & SHF_ALLOC )	// WRITE=1 | ALLOC=2 | EXECINSTR=4
			{
				log("->\tLoaded To %X size:%X\n", pShdr->sh_addr, pShdr->sh_size);

				if( (pShdr->sh_addr & 0x0F000000) == 0x0C000000 )
					for( u32 x=0; x<=pShdr->sh_size; x++ )	// +=2
						mem_b[(pShdr->sh_addr&RAM_MASK)+x] = pElf[pShdr->sh_offset+x];
						//WriteMem16((pShdr->sh_addr+(x*2)), pElf[(pShdr->sh_offset+(x*2))]);
			}
		break;

		case SHT_NOTE:
		case SHT_RELA:
		case SHT_HASH:
		case SHT_DYNAMIC:
		case SHT_REL:
		case SHT_SHLIB:
		case SHT_DYNSYM:
		case SHT_INIT_ARRAY:
		case SHT_FINI_ARRAY:
		case SHT_PREINIT_ARRAY:
		case SHT_GROUP:
		case SHT_SYMTAB_SHNDX:

		default:
		//	if( (pShdr->sh_type >= SHT_LOOS) && (pShdr->sh_type <= SHT_HIOS) ) { break; }
		//	if( (pShdr->sh_type >= SHT_LOPROC) && (pShdr->sh_type <= SHT_HIPROC) ) { break; }
		//	if( (pShdr->sh_type >= SHT_LOUSER) && (pShdr->sh_type <= SHT_HIUSER) ) { break; }

			log("ELF File Unknown SecHead[%x].type:%X \n", i, pShdr->sh_type);
		break;
		}
	}

/*
	FILE *fp = fopen("SymTab.txt","wt");
	if(!fp) goto done;

	for(i=0; i<(symindex>>16); i++)
		fprintf(fp,
		"SymTab[%04X]\n"
		"{\n"
		"\tst_name:%X \"%s\"\n"
		"\tst_value:%X\n"
		"\tst_size:%X\n"
		"\tst_info:%X\n"
		"\tst_other:%X\n"
		"\tst_shndx:%X\n"
		"}\n\n",
		i,
		pSymTab[i].st_name, (char*)(strtab+pSymTab[i].st_name),
		pSymTab[i].st_value,
		pSymTab[i].st_size,
		pSymTab[i].st_info,
		pSymTab[i].st_other,
		pSymTab[i].st_shndx );

	fclose(fp);

done:
*/	delete [] pElf;	// free memory loaded for file 
	return true;
}

