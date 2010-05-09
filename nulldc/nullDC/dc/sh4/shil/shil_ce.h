#pragma once
#include "types.h"
#include "dc\sh4\sh4_if.h"
#include "dc\sh4\sh4_if.h"
#include "dc\sh4\rec_v1\BasicBlock.h"

struct shil_ce_info_entry
{
	bool is_const;
	bool can_be_const;
	u32 value;
};
struct shil_ce_info
{
	shil_ce_info_entry reginfo[sh4_reg_count];
	void Init()
	{
		memset(reginfo,0,sizeof(reginfo));
		for (u32 i=0;i<sh4_reg_count;i++)
			reginfo[i].can_be_const=true;
	}
	void Merge(shil_ce_info* other)
	{
		for (u32 i=0;i<sh4_reg_count;i++)
		{
			if (reginfo[i].can_be_const && other->reginfo[i].can_be_const)
			{
				if (other->reginfo[i].is_const)
				{
					if (reginfo[i].is_const)
					{
						if (reginfo[i].value!=other->reginfo[i].value)
						{
							reginfo[i].is_const=false;
							reginfo[i].can_be_const=false;
						}
					}
					else
					{
						reginfo[i].is_const=true;
						reginfo[i].value=other->reginfo[i].value;
					}
				}
				else
				{
					if (reginfo[i].is_const)
					{
						reginfo[i].is_const=false;
						reginfo[i].can_be_const=false;
					}
				}
			}
			else
			{
				reginfo[i].can_be_const=false;
			}
		}
	}
};

void shil_optimise_pass_ce_driver(BasicBlock* bb);