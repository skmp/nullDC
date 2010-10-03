#include "pvr_renderer_d3d1x.h"
#include "pvr_impl.h"
#include "helper_classes.h"
#include "ta.h"
#include "regs.h"
#include "TexCache.h"
#include "mudpvr.h"
#include <algorithm>
#include <vector>

using namespace TASplitter;
//
// Globals
//
struct { bool needs_resize;NDC_WINDOW_RECT new_size;u32 rev;} resizerq;
struct PolyParam
{
	u32 first;		//entry index , holds vertex/pos data
	u32 count;

	//lets see what more :)

	TSP tsp;
	TCW tcw;
	PCW pcw;
	ISP_TSP isp;
	float zvZ;
	u32 tileclip;
	//float zMin,zMax;
};
//Vertex storage types
//64B
struct Vertex
{
	//64
	float x,y,z;

#ifdef _float_colors_
	float col[4];
	float spc[4];
#else
	u32 col;
	u32 spc;
#endif
	float u,v;
	/*float p1,p2,p3;	*/ //pad to 64 bytes (for debugging purposes)
#ifdef _HW_INT_
	float base_int,offset_int;
#endif
};
Vertex* BGPoly;
static Vertex* vert_reappend;

struct ModParam
{
	u32 first;		//entry index , holds vertex/pos data
	u32 count;
};
struct ModTriangle
{
	f32 x0,y0,z0,x1,y1,z1,x2,y2,z2;
};
union ISP_Modvol
{
	struct
	{
		u32 id:26;
		u32 VolumeLast:1;
		u32	CullMode	: 2;
		u32	DepthMode	: 3;
	};
	u32 full;
};
//vertex lists
struct TA_context
{
	u32 Address;
	u32 LastUsed;
	f32 invW_min;
	f32 invW_max;
	List2<Vertex> verts;
	List<ModTriangle>	modtrig;
	List<ISP_Modvol>	global_param_mvo;
	List<PolyParam> global_param_op;
	List<PolyParam> global_param_pt;
	List<PolyParam> global_param_tr;
	void Init()
	{
		verts.Init();
		global_param_op.Init();
		global_param_pt.Init();
		global_param_mvo.Init();
		global_param_tr.Init();

		modtrig.Init();
	}
	void Clear()
	{
		verts.Clear();
		global_param_op.Clear();
		global_param_pt.Clear();
		global_param_tr.Clear();
		modtrig.Clear();
		global_param_mvo.Clear();
		invW_min= 1000000.0f;
		invW_max=-1000000.0f;
	}
	void Free()
	{
		verts.Free();
		global_param_op.Free();
		global_param_pt.Free();
		global_param_tr.Free();
		modtrig.Free();
		global_param_mvo.Free();
	}
};


TA_context tarc;
TA_context pvrrc;
std::vector<TA_context> rcnt;
PolyParam* CurrentPP=0;
List<PolyParam>* CurrentPPlist;
extern cThread RenderThread;


//----------------------------------------------------------------------------------------------------------------------------------
//D3D1x Renderer
//----------------------------------------------------------------------------------------------------------------------------------

bool rend_init()
{
	return true;

}
void rend_term()
{

}
void rend_reset(bool Manual)
{

}
bool rend_thread_start()
{

	return true;

}
void rend_thread_end  ()
{

}
void rend_vblank      ()
{

}
void rend_start_render ()
{

}
void rend_end_render   ()
{

}
void rend_handle_event (u32 evid,void* p)
{
	if (evid == NDE_GUI_RESIZED)
	{
		resizerq.needs_resize=true;
		if (p)
			memcpy((void*)&resizerq.new_size,p,sizeof(NDC_WINDOW_RECT));
		resizerq.rev++;
	}
}

void rend_list_cont()
{
	//tile list continue

}
void rend_list_init()
{
	//tile list init

}
void rend_list_softreset()
{
	//tile list softreset

}

void rend_text_invl (vram_block* bl){

	params.vram_unlock(bl);
}
void rend_set_render_rect (float* rect,bool  drc){}
void rend_set_fb_scale (float x,float y){}
void rend_list_modes(void(* callback)(u32 w,u32 h,u32 rr)){}

//----------------------------------------------------------------------------------------------------------------------------------
// CORE RASTERIZER
//----------------------------------------------------------------------------------------------------------------------------------
//Convert offset32 to offset64
u32 vramlock_ConvOffset32toOffset64(u32 offset32)
{
	//64b wide bus is archevied by interleaving the banks every 32 bits
	//so bank is Address<<3
	//bits <4 are <<1 to create space for bank num
	//bank 0 is mapped at 400000 (32b offset) and after
	const u32 bank_bit=VRAM_MASK-(VRAM_MASK/2);
	const u32 static_bits=(VRAM_MASK-(bank_bit*2-1))|3;
	const u32 moved_bits=VRAM_MASK-static_bits-bank_bit;

	u32 bank=(offset32&bank_bit)/bank_bit*4;//bank will be used as uper offset too
	u32 lv=offset32&static_bits; //these will survive
	offset32&=moved_bits;
	offset32<<=1;
	//       |inbank offset    |       bank id        | lower 2 bits (not changed)
	u32 rv=  offset32 + bank                  + lv;

	return rv;
}

bool operator<(const PolyParam &left, const PolyParam &right)
{
	return left.zvZ<right.zvZ;
}
	void SortPParams()
	{
		if (pvrrc.verts.allocate_list_sz->size()==0)
			return;
		u32 base=0;
		u32 csegc=0;
		u32 cseg=-1;
		Vertex* bptr=0;
		for (u32 i=0;i<pvrrc.global_param_tr.used;i++)
		{
			u32 s=pvrrc.global_param_tr.data[i].first;
			u32 c=pvrrc.global_param_tr.data[i].count;
			float zv=0;
			for (u32 j=s;j<(s+c);j++)
			{
				while (j>=csegc)
				{
					cseg++;
					bptr=(Vertex*)((*pvrrc.verts.allocate_list_ptr)[cseg]);
					bptr-=csegc;
					csegc+=(*pvrrc.verts.allocate_list_sz)[cseg]/sizeof(Vertex);
				}
				zv+=bptr[j].z;
			}
			pvrrc.global_param_tr.data[i].zvZ=zv/c;
		}

		std::stable_sort(pvrrc.global_param_tr.data,pvrrc.global_param_tr.data+pvrrc.global_param_tr.used);
}

u32 fastcall FindRC(u32 addr)
	{
		for (u32 i=0;i<rcnt.size();i++)
		{
			if (rcnt[i].Address==addr)
			{
				return i;
			}
		}
		return 0xFFFFFFFF;
}
	
void fastcall SetCurrentTARC(u32 addr)
	{
		addr&=0xF00000;
		//return;
		//printf("SetCurrentTARC:0x%X\n",addr);
		if (addr==tarc.Address)
			return;//nothing to do realy

		//save old context
		u32 found=FindRC(tarc.Address);
		if (found!=0xFFFFFFFF)
			memcpy(&rcnt[found],&tarc,sizeof(TA_context));

		//switch to new one
		found=FindRC(addr);
		if (found!=0xFFFFFFFF)
		{
			memcpy(&tarc,&rcnt[found],sizeof(TA_context));
		}
		else
		{
			//add one :p
			tarc.Address=addr;
			tarc.Init();
			tarc.Clear();
			rcnt.push_back(tarc);
		}
}
	
void fastcall SetCurrentPVRRC(u32 addr)
	{
		addr&=0xF00000;
		//return;
		//printf("SetCurrentPVRRC:0x%X\n",addr);
		if (addr==tarc.Address)
		{
			memcpy(&pvrrc,&tarc,sizeof(TA_context));
			return;
		}

		u32 found=FindRC(addr);
		if (found!=0xFFFFFFFF)
		{
			memcpy(&pvrrc,&rcnt[found],sizeof(TA_context));
			return;
		}

		printf("WARNING : Unable to find a PVR rendering context\n");
		memcpy(&pvrrc,&tarc,sizeof(TA_context));
}
