#define _WIN32_WINNT 0x0500
#include "drkpvr.h"
#if _DEBUG
#define D3D_DEBUG_INFO
#endif
#include <d3dx9.h>

#include "pvr_algo/pvr_algo.hpp"

#include "nullRend.h"
#include <algorithm>
#include "d3dRend.h"
#include "windows.h"
//#include "gl\gl.h"
#include "regs.h"
#include <vector>
//#include <xmmintrin.h>

#if REND_API == REND_D3D
#pragma comment(lib, "d3d9.lib") 
#pragma comment(lib, "d3dx9.lib") 

#define MODVOL 1
#define _float_colors_
//#define _HW_INT_
//#include <D3dx9shader.h>

using namespace TASplitter;

pvr_algo::pvr_state_manager_c g_pvr_states_manager;

volatile bool render_restart = false;
bool UseSVP=false;
bool UseFixedFunction=false;
bool dosort=false;
#if _DEBUG
	#define SHADER_DEBUG D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION
#else
	#define SHADER_DEBUG 0 /*D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION*/
#endif
/*
#define DEV_CREATE_FLAGS D3DCREATE_HARDWARE_VERTEXPROCESSING
//#define DEV_CREATE_FLAGS D3DCREATE_SOFTWARE_VERTEXPROCESSING

#if DEV_CREATE_FLAGS==D3DCREATE_HARDWARE_VERTEXPROCESSING
	#define VB_CREATE_FLAGS 0
#else
	#define VB_CREATE_FLAGS D3DUSAGE_SOFTWAREPROCESSING
#endif
*/
#define scale_type_1



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
//these can be used to force a profile
#define FORCE_SW_VERTEX_SHADERS (0)
#define FORCE_FIXED_FUNCTION    (0)
//#define D3DXGetPixelShaderProfile(x) "ps_2_0"
//#define D3DXGetVertexShaderProfile(x) "vs_2_0"

#define PS_SHADER_COUNT (384*4)
	bool RenderWasStarted=false;
	volatile bool d3d_init_done=false;
	volatile bool d3d_do_resize=false;
	volatile bool d3d_do_restart=false;
	IDirect3D9* d3d9;
	IDirect3DDevice9* dev;
	IDirect3DVertexBuffer9* vb;
	IDirect3DVertexShader9* compiled_vs;
	IDirect3DPixelShader9* compiled_ps[PS_SHADER_COUNT]={0};
	IDirect3DPixelShader9* ShadeColPixelShader=0;
	IDirect3DPixelShader9* ZPixelShader=0;
	
	IDirect3DTexture9* pal_texture=0;
	IDirect3DTexture9* fb_texture8888=0,* fb_texture565=0,*fb_texture1555=0;
	IDirect3DTexture9* fog_texture=0;
	IDirect3DTexture9* rtt_texture[2]={0,0};
	u32 rtt_index;
	IDirect3DTexture9* fb_texture=0;
	u32 rtt_address=0;
	u32 rtt_FrameNumber=0xFFFFFFFF;
	IDirect3DSurface9* fb_surf=0,* rtt_surf[2]={0,0},* fb_surface8888=0,* fb_surface565=0,* fb_surface1555=0;
	IDirect3DSurface9* backbuffer=0;
	D3DSURFACE_DESC fb_surf_desc;
	ID3DXFont* font;
	ID3DXConstantTable* shader_consts;
	RECT window_rect;
	D3DPRESENT_PARAMETERS ppar;
	u32 ppar_BackBufferWidth,ppar_BackBufferHeight;

	CRITICAL_SECTION d3d_lock;
	
	struct 
	{
		IDirect3DVertexShader9* vs;
		IDirect3DPixelShader9* ps_DrawFB;
		IDirect3DPixelShader9* ps_DrawA;
		IDirect3DPixelShader9* ps_DrawRGB;
		IDirect3DPixelShader9* ps_DrawRGBA;
	} Composition;
	

	IDirect3DVertexDeclaration9* vdecl;
	IDirect3DVertexDeclaration9* vdecl_mod;
	IDirect3DVertexDeclaration9* vdecl_osd;

	struct VertexDecoder;
	FifoSplitter<VertexDecoder> TileAccel;

	struct { bool needs_resize;NDC_WINDOW_RECT new_size;u32 rev;} resizerq;

	u32 clear_rt=0;
	u32 last_ps_mode=0xFFFFFFFF;
	float current_scalef[4];
	//CRITICAL_SECTION tex_cache_cs;
	
	u32 FrameNumber=0;
	u32 fb_FrameNumber=0;

	u32 frameStart = 0;
	u32 frameRate = 0;
	DWORD timer, timeStart = GetTickCount();

	wchar fps_text[512];
	float res_scale[4]={0,0,320,-240};
	float fb_scale[2]={1,1};

	u32 ZBufferCF=D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER;
	u32 ZBufferMode=0;
	const char* ZBufferModeName[]=
	{
		"Float Z Buffering (D24FS8)",
		"Float Z Buffering Emulation (D24S8+FPE)",
		"Z Scale mode 1 (D24S8)",
		"Z Scale mode 2 (D24S8)",
		"Z Scale mode 42 (D24yada)",
	};

	HRESULT r_set_texture_stage_state(DWORD s,u32 t,DWORD v) {
		u64 trans;
		if (g_pvr_states_manager.cached(s,t,v)) {
			return D3D_OK;
		}

		g_pvr_states_manager.translate(t,trans);
		return dev->SetTextureStageState(s,(D3DTEXTURESTAGESTATETYPE)trans,v);
	}

	HRESULT r_set_texture(DWORD s,IDirect3DBaseTexture9* t) {
		//Todo : Make sure to flush dirty textures!
		/*if (g_pvr_states_manager.cached(s,R_EXT_OP_TEXTURE_BASE,(u64)t)) {
			return D3D_OK;
		}*/
		return dev->SetTexture(s,t);
	}

	HRESULT r_set_state(u32 s,DWORD v) { //Temporary(single pass) states
		u64 trans;
		if (g_pvr_states_manager.cached(R_NO_SAMPLER,s,v)) {
			return D3D_OK;
		}
		g_pvr_states_manager.translate(s,trans);
		return dev->SetRenderState((D3DRENDERSTATETYPE)trans,v);
	}

	HRESULT r_set_sampler_state(DWORD s,u32 t,DWORD v) {

		u64 trans;
		if (g_pvr_states_manager.cached(s,t,v)) {
			return D3D_OK;
		}

		g_pvr_states_manager.translate(t,trans);
		return dev->SetSamplerState(s,(D3DSAMPLERSTATETYPE)trans,v);
	}

	//x=emulation mode
	//y=filter mode
	//result = {d3dmode,shader id}
	void HandleEvent(u32 evid,void* p)
	{
		if (evid == NDE_GUI_RESIZED)
		{
			resizerq.needs_resize=true;
			if (p)
				memcpy((void*)&resizerq.new_size,p,sizeof(NDC_WINDOW_RECT));
			resizerq.rev++;
		}
	}
	void SetRenderRect(float* rect,bool do_clear)
	{
		res_scale[0]=rect[0];
		res_scale[1]=rect[1];

		res_scale[2]=rect[2]/2;
		res_scale[3]=-rect[3]/2;

		if(do_clear)
			clear_rt|=1;
	}
	void SetFBScale(float x,float y)
	{
		fb_scale[0]=x;
		fb_scale[1]=y;
	}
	const static u32 CullMode[]= 
	{
		
		D3DCULL_NONE,	//0	No culling	no culling
		D3DCULL_NONE,	//1	Cull if Small	Cull if	( |det| < fpu_cull_val )

		//wtf ?
		D3DCULL_CCW /*D3DCULL_CCW*/,		//2	Cull if Negative	Cull if 	( |det| < 0 ) or
						//( |det| < fpu_cull_val )
		D3DCULL_CW /*D3DCULL_CW*/,		//3	Cull if Positive	Cull if 	( |det| > 0 ) or
						//( |det| < fpu_cull_val )
		
		
		D3DCULL_NONE,	//0	No culling	no culling
		D3DCULL_NONE,	//1	Cull if Small	Cull if	( |det| < fpu_cull_val )

		//wtf ?
		D3DCULL_CW /*D3DCULL_CCW*/,		//2	Cull if Negative	Cull if 	( |det| < 0 ) or
						//( |det| < fpu_cull_val )
		D3DCULL_CCW /*D3DCULL_CW*/,		//3	Cull if Positive	Cull if 	( |det| > 0 ) or
						//( |det| < fpu_cull_val )
		
	};
	const static u32 Zfunction[]=
	{
		//This bit is used in combination with the Z Write Disable bit, and 
		//supports compare processing, which is required for OpenGL and D3D 
		//versus Z buffer updates.  It is important to note that, because the
		//value of either 1/z or 1/w is referenced for the Z value, the closer
		//that the polygon is, the larger that the Z value will be.
		
		//This setting is ignored for Translucent polygons in Auto-sort 
		//mode; the comparison must be made on a "Greater or Equal" basis.  
		//This setting is also ignored for Punch Through polygons in HOLLY2; 
		//the comparison must be made on a "Less or Equal" basis.


		D3DCMP_NEVER,				//0	Never
		D3DCMP_LESS/*EQUAL*/,		//1	Less
		D3DCMP_EQUAL,				//2	Equal
		D3DCMP_LESSEQUAL,			//3	Less Or Equal
		D3DCMP_GREATER/*EQUAL*/,	//4	Greater
		D3DCMP_NOTEQUAL,			//5	Not Equal
		D3DCMP_GREATEREQUAL,		//6	Greater Or Equal
		D3DCMP_ALWAYS,				//7	Always

	};

	/*
	0	Zero	(0, 0, 0, 0)
	1	One	(1, 1, 1, 1)
	2	‘Other’ Color	(OR, OG, OB, OA) 
	3	Inverse ‘Other’ Color	(1-OR, 1-OG, 1-OB, 1-OA)
	4	SRC Alpha	(SA, SA, SA, SA)
	5	Inverse SRC Alpha	(1-SA, 1-SA, 1-SA, 1-SA)
	6	DST Alpha	(DA, DA, DA, DA)
	7	Inverse DST Alpha	(1-DA, 1-DA, 1-DA, 1-DA)
	*/

	const static u32 DstBlendGL[] =
	{
		D3DBLEND_ZERO,
		D3DBLEND_ONE,
		D3DBLEND_SRCCOLOR,
		D3DBLEND_INVSRCCOLOR,
		D3DBLEND_SRCALPHA,
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_DESTALPHA,
		D3DBLEND_INVDESTALPHA
	};

	const static u32 SrcBlendGL[] =
	{
		D3DBLEND_ZERO,
		D3DBLEND_ONE,
		D3DBLEND_DESTCOLOR,
		D3DBLEND_INVDESTCOLOR,
		D3DBLEND_SRCALPHA,
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_DESTALPHA,
		D3DBLEND_INVDESTALPHA
	};

	char texFormatName[8][30]=
	{
		"1555",
		"565",
		"4444",
		"YUV422",
		"Bump Map",
		"4 BPP Palette",
		"8 BPP Palette",
		"Reserved	, 1555"
	};

	float unkpack_bgp_to_float[256];

	f32 f16(u16 v)
	{
		u32 z=v<<16;
		return *(f32*)&z;
	}
	const u32 MipPoint[8] =
	{
		0x00006,//8
		0x00016,//16
		0x00056,//32
		0x00156,//64
		0x00556,//128
		0x01556,//256
		0x05556,//512
		0x15556//1024
	};


#define twidle_tex(format)\
						if (tcw.NO_PAL.VQ_Comp)\
					{\
						vq_codebook=(u8*)&params.vram[sa];\
						if (tcw.NO_PAL.MipMapped)\
							sa+=MipPoint[tsp.TexU];\
						##format##to8888_VQ(&pbt,(u8*)&params.vram[sa],w,h);\
					}\
					else\
					{\
						if (tcw.NO_PAL.MipMapped)\
							sa+=MipPoint[tsp.TexU]<<3;\
						##format##to8888_TW(&pbt,(u8*)&params.vram[sa],w,h);\
					}
#define norm_text(format) \
	u32 sr;\
	if (tcw.NO_PAL.StrideSel)\
					{sr=(TEXT_CONTROL&31)*32;}\
					else\
					{sr=w;}\
					format(&pbt,(u8*)&params.vram[sa],sr,h);

	typedef void fastcall texture_handler_FP(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);

	/*
	texture_handler_FP* texture_handlers[8] = 
	{
		,//0	1555 value: 1 bit; RGB values: 5 bits each
		,//1	565	 R value: 5 bits; G value: 6 bits; B value: 5 bits
		,//3	YUV422 32 bits per 2 pixels; YUYV values: 8 bits each
		,//2	4444 value: 4 bits; RGB values: 4 bits each
		,//4	Bump Map	16 bits/pixel; S value: 8 bits; R value: 8 bits
		,//5	4 BPP Palette	Palette texture with 4 bits/pixel
		,//6	8 BPP Palette	Palette texture with 8 bits/pixel
		,//7 -> undefined , handled as 0
	};

	u32 texture_format[8]
	{
		D3DFMT_A1R5G5B5,//0	1555 value: 1 bit; RGB values: 5 bits each
		D3DFMT_R5G6B5,//1	565	 R value: 5 bits; G value: 6 bits; B value: 5 bits
		D3DFMT_UYVY,//3	YUV422 32 bits per 2 pixels; YUYV values: 8 bits each
		D3DFMT_A4R4G4B4,//2	4444 value: 4 bits; RGB values: 4 bits each
		D3DFMT_UNKNOWN,//4	Bump Map	16 bits/pixel; S value: 8 bits; R value: 8 bits
		D3DFMT_A8R8G8B8,//5	4 BPP Palette	Palette texture with 4 bits/pixel
		D3DFMT_A8R8G8B8,//6	8 BPP Palette	Palette texture with 8 bits/pixel
		D3DFMT_A1R5G5B5,//7 -> undefined , handled as 0
	};
	*/
	struct TextureCacheData;
	std::vector<TextureCacheData*> lock_list;
	//Texture Cache :)
	struct TextureCacheData
	{
		TCW tcw;TSP tsp;
		IDirect3DTexture9* Texture;
		u32 Lookups;
		u32 Updates;
		u32 LastUsed;
		u32 w,h;
		u32 size;
		bool dirty;
		u32 pal_rev;
		vram_block* lock_block;

		//Releases any resources , EXEPT the texture :)
		void Destroy()
		{
			if (lock_block)
				params.vram_unlock(lock_block);
			lock_block=0;
		}
		//Called when texture entry is reused , resets any texture type info (dynamic/static)
		void Reset()
		{
			Lookups=0;
			Updates=0;
		}
		void PrintTextureName()
		{
			printf(texFormatName[tcw.NO_PAL.PixelFmt]);
	
			if (tcw.NO_PAL.VQ_Comp)
				printf(" VQ");

			if (tcw.NO_PAL.ScanOrder==0)
				printf(" TW");

			if (tcw.NO_PAL.MipMapped)
				printf(" MM");

			if (tcw.NO_PAL.StrideSel)
				printf(" Stride");

			if (tcw.NO_PAL.StrideSel)
				printf(" %d[%d]x%d @ 0x%X",(TEXT_CONTROL&31)*32,8<<tsp.TexU,8<<tsp.TexV,tcw.NO_PAL.TexAddr<<3);
			else
				printf(" %dx%d @ 0x%X",8<<tsp.TexU,8<<tsp.TexV,tcw.NO_PAL.TexAddr<<3);
			printf("\n");
		}
		void Update()
		{
//			verify(dirty);
//			verify(lock_block==0);

			LastUsed=FrameNumber;
			Updates++;
			dirty=false;

			u32 sa=(tcw.NO_PAL.TexAddr<<3) & VRAM_MASK;

			if (Texture==0)
			{
				if (tcw.NO_PAL.MipMapped && (!(settings.Emulation.PaletteMode>=2 && (tcw.NO_PAL.PixelFmt==5 || tcw.NO_PAL.PixelFmt==6))) )
				{
					verifyc(dev->CreateTexture(w,h,0,D3DUSAGE_AUTOGENMIPMAP,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&Texture,0));
				}
				else
				{
					verifyc(dev->CreateTexture(w,h,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&Texture,0));
				}
			}
			D3DLOCKED_RECT rect;

			verifyc(Texture->LockRect(0,&rect,NULL,0 /* | D3DLOCK_DISCARD*/));
			

			PixelBuffer pbt; 
			pbt.init(rect.pBits,rect.Pitch);
			
			switch (tcw.NO_PAL.PixelFmt)
			{
			case 0:
			case 7:
				//0	1555 value: 1 bit; RGB values: 5 bits each
				//7	Reserved	Regarded as 1555
				if (tcw.NO_PAL.ScanOrder)
				{
					//verify(tcw.NO_PAL.VQ_Comp==0);
					norm_text(argb1555to8888);
					//argb1555to8888(&pbt,(u16*)&params.vram[sa],w,h);
				}
				else
				{
					//verify(tsp.TexU==tsp.TexV);
					twidle_tex(argb1555);
				}
				break;

				//redo_argb:
				//1	565	 R value: 5 bits; G value: 6 bits; B value: 5 bits
			case 1:
				if (tcw.NO_PAL.ScanOrder)
				{
					//verify(tcw.NO_PAL.VQ_Comp==0);
					norm_text(argb565to8888);
					//(&pbt,(u16*)&params.vram[sa],w,h);
				}
				else
				{
					//verify(tsp.TexU==tsp.TexV);
					twidle_tex(argb565);
				}
				break;

				
				//2	4444 value: 4 bits; RGB values: 4 bits each
			case 2:
				if (tcw.NO_PAL.ScanOrder)
				{
					//verify(tcw.NO_PAL.VQ_Comp==0);
					//argb4444to8888(&pbt,(u16*)&params.vram[sa],w,h);
					norm_text(argb4444to8888);
				}
				else
				{
					twidle_tex(argb4444);
				}

				break;
				//3	YUV422 32 bits per 2 pixels; YUYV values: 8 bits each
			case 3:
				if (tcw.NO_PAL.ScanOrder)
				{
					norm_text(YUV422to8888);
					//norm_text(ANYtoRAW);
				}
				else
				{
					//it cant be VQ , can it ?
					//docs say that yuv can't be VQ ...
					//HW seems to support it ;p
					twidle_tex(YUV422);
				}
				break;
				//4	Bump Map	16 bits/pixel; S value: 8 bits; R value: 8 bits
			case 5:
				//5	4 BPP Palette	Palette texture with 4 bits/pixel
				verify(tcw.PAL.VQ_Comp==0);
				if (tcw.NO_PAL.MipMapped)
							sa+=MipPoint[tsp.TexU]<<1;
				palette_index = tcw.PAL.PalSelect<<4;
				pal_rev=pal_rev_16[tcw.PAL.PalSelect];
				if (settings.Emulation.PaletteMode<2)
				{
					PAL4to8888_TW(&pbt,(u8*)&params.vram[sa],w,h);
				}
				else
				{
					PAL4toX444_TW(&pbt,(u8*)&params.vram[sa],w,h);
				}

				break;
			case 6:
				//6	8 BPP Palette	Palette texture with 8 bits/pixel
				verify(tcw.PAL.VQ_Comp==0);
				if (tcw.NO_PAL.MipMapped)
							sa+=MipPoint[tsp.TexU]<<2;
				palette_index = (tcw.PAL.PalSelect<<4)&(~0xFF);
				pal_rev=pal_rev_256[tcw.PAL.PalSelect>>4];
				if (settings.Emulation.PaletteMode<2)
				{
					PAL8to8888_TW(&pbt,(u8*)&params.vram[sa],w,h);
				}
				else
				{
					PAL8toX444_TW(&pbt,(u8*)&params.vram[sa],w,h);
				}
				break;
			default:
				printf("Unhandled texture\n");
				//memset(temp_tex_buffer,0xFFEFCFAF,w*h*4);
			}


			//done , unlock texture !
			Texture->UnlockRect(0);
			//PrintTextureName();
			if (!lock_block)
				lock_list.push_back(this);
			/*
			char file[512];

			sprintf(file,"g:\\textures\\0x%08x_%08x_%s_%d_VQ%d_TW%d_MM%d_.png",tcw.full,tsp.full,texFormatName[tcw.NO_PAL.PixelFmt],Lookups
			,tcw.NO_PAL.VQ_Comp,tcw.NO_PAL.ScanOrder,tcw.NO_PAL.MipMapped);
			D3DXSaveTextureToFileA( file,D3DXIFF_PNG,Texture,0);
			*/
		}
		void LockVram()
		{
			u32 sa=(tcw.NO_PAL.TexAddr<<3) & VRAM_MASK;
			u32 ea=sa+w*h*2;
			if (ea>VRAM_MASK)
			{
				ea=VRAM_MASK;
			}
			lock_block = params.vram_lock_64(sa,ea,this);
		}
	};

	TexCacheList<TextureCacheData> TexCache;
	TexCacheList<TextureCacheData> TexCache_Discard;

	TextureCacheData* __fastcall GenText(TSP tsp,TCW tcw,TextureCacheData* tf)
	{
		//generate texture	
		tf->w=8<<tsp.TexU;
		tf->h=8<<tsp.TexV;
		tf->tsp=tsp;
		tf->tcw=tcw;
		tf->dirty=true;
		tf->lock_block=0;
		tf->Texture=0;
		tf->Reset();
		tf->Update();
		return tf;
	}

	TextureCacheData* __fastcall GenText(TSP tsp,TCW tcw)
	{
		//add new entry to tex cache
		TextureCacheData* tf = &TexCache.Add(0)->data;
		//Generate texture 
		return GenText(tsp,tcw,tf);
	}

	u32 RenderToTextureAddr;

	inline bool is_fmt_supported(TSP tsp,TCW tcw) {
			const s32 sw = (s32)((u32)(8<<tsp.TexU));
			const s32 sh = (s32)((u32)(8<<tsp.TexV));
			if (( (sw + sh) << 1) > VRAM_SIZE) { //Minimum of 2components
				printf("Unsupported texture(OUT OF RANGE) w %u h %u fmt %u vq %u\n",8<<tsp.TexU,8<<tsp.TexV,tcw.NO_PAL.VQ_Comp);
				return false;
			}

			switch (tcw.NO_PAL.PixelFmt) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 7:
				if (tcw.NO_PAL.ScanOrder) {
					return (tcw.NO_PAL.VQ_Comp == 0);
				} 
				return true;

				case 4:
					return false;

				case 5:
					return (tcw.NO_PAL.VQ_Comp==0);
				case 6:
					return (tcw.NO_PAL.VQ_Comp==0);
			}
		return false;
	}

	IDirect3DTexture9* __fastcall GetTexture(TSP tsp,TCW tcw)
	{	
		if (!is_fmt_supported(tsp,tcw)) {
			return 0;
		}

		u32 addr=(tcw.NO_PAL.TexAddr<<3) & VRAM_MASK;
		if (addr==rtt_address)
		{
			rtt_FrameNumber=FrameNumber;
			//printf("Reading to %d\n",rtt_index);
			return rtt_texture[rtt_index];
		}

		//EnterCriticalSection(&tex_cache_cs);
		TextureCacheData* tf = TexCache.Find(tcw.full,tsp.full);
		if (tf)
		{
			tf->LastUsed=FrameNumber;
			if (tf->dirty)
			{
				tf->Update();
			}
			if (settings.Emulation.PaletteMode==1)
			{
				if (tcw.PAL.PixelFmt==5)
				{				
					if (tf->pal_rev!=pal_rev_16[tcw.PAL.PalSelect])
						tf->Update();
				}
				else if (tcw.PAL.PixelFmt==6)
				{
					if (tf->pal_rev!=pal_rev_256[tcw.PAL.PalSelect>>4])
						tf->Update();
				}
			}
			tf->Lookups++;
			//LeaveCriticalSection(&tex_cache_cs);
			return tf->Texture;
		}
		else
		{
			tf = GenText(tsp,tcw);
			//LeaveCriticalSection(&tex_cache_cs);
			return tf->Texture;
		}
		return 0;
	}
	
	void VramLockedWrite(vram_block* bl)
	{
		//EnterCriticalSection(&tex_cache_cs);
		TextureCacheData* tcd = (TextureCacheData*)bl->userdata;
		tcd->dirty=true;
		tcd->lock_block=0;
		/*
		if (tcd->Updates==0)
		{
			tcd->Texture->Release();
			tcd->Texture=0;
		}*/
		params.vram_unlock(bl);
		//LeaveCriticalSection(&tex_cache_cs);
	}
	extern cThread rth;

	u32 vri(u32 addr);
	//use that someday
	/*
	Vertex FullScreenQuad[4] = 
	{
		{0,0,0.5 ,0,0,0,0 ,0,0,0,0 },
		{0,0,0.5 ,0,0,0,0 ,0,0,0,0 },
		{0,0,0.5 ,0,0,0,0 ,0,0,0,0 },
		{0,0,0.5 ,0,0,0,0 ,0,0,0,0 },
	};
	*/
	void DrawOSD();
	void VBlank()
	{
		FrameNumber++;
		
		u32 field=0;//default from field 1
		u32 interlc=SPG_CONTROL.interlace;
		switch (VO_CONTROL.field_mode)
		{
			//From SPG
		case 0:
			field=SPG_STATUS.fieldnum;
			break;

			//From ~SPG
		case 1:
			field=1^SPG_STATUS.fieldnum;
			break;

			//From field 1
		case 2:
			field=0;
			break;

			//From field 2
		case 3:
			field=1;
			break;

			//From field 1 when HSYNC && VSYNC match
		case 4:
			field=0;	//uh. ...
			break;

			//From field 2 when HSYNC && VSYNC match
		case 5:
			field=1;	//uh. ...
			break;

			//From field 1 when HSYNC -> 1 in VSYNC
		case 6:
			field=0;	//uh. ...
			break;

			//From field 2 when HSYNC -> 1 in VSYNC
		case 7:
			field=1;	//uh. ...
			break;

			//Inverted when VSYNC -> 1
		case 8:
			//what ?
			break;

		default:
			break;
		}

		u32 src;
		//select input ..
		if (field)
			src=FB_R_SOF2;
		else
			src=FB_R_SOF1;

		u32 addr1=vramlock_ConvOffset32toOffset64(src);
		u32* ptest=(u32*)&params.vram[addr1];

		EnterCriticalSection(&d3d_lock);
		if (d3d_init_done)
		{
			IDirect3DTexture9* tex=fb_texture;
			IDirect3DSurface9* surf=fb_surf;
			
			//assume rect is same as front buffer
			RECT rs={0,0,fb_surf_desc.Width,fb_surf_desc.Height};
			
			dev->ColorFill(backbuffer,0,D3DCOLOR_ARGB(255,VO_BORDER_COL.Red,VO_BORDER_COL.Green,VO_BORDER_COL.Blue));

			if ((FB_R_CTRL.fb_enable && !VO_CONTROL.blank_video) || DC_PLATFORM==DC_PLATFORM_ATOMISWAVE)
			{
				if ( *ptest!=0xDEADC0DE)
				{
					//use proper FB rect size
					rs.right=640;
					rs.bottom=480;

					D3DLOCKED_RECT lr;
					u32 bpp;
					switch(FB_R_CTRL.fb_depth)
					{
					case fbde_0555:
						bpp=2;
						surf=fb_surface1555;
						tex=fb_texture1555;
						break;
					case fbde_565:
						bpp=2;
						surf=fb_surface565;
						tex=fb_texture565;
						break;
					case fbde_C888:
						bpp=4;
						surf=fb_surface8888;
						tex=fb_texture8888;
						break;
					case fbde_888:
						bpp=3;
						surf=fb_surface8888;
						tex=fb_texture8888;
						break;
					}

					u32 PixelsPerLine=640;

					verifyc(surf->LockRect(&lr,0,0));


					u32 out_field=SPG_STATUS.fieldnum;

					u32 fb_skip=FB_R_SIZE.fb_modulus;
					u32 line_double=FB_R_CTRL.fb_line_double;
					u32 pixel_double=VO_CONTROL.pixel_double;

					if (pixel_double)
						PixelsPerLine/=2;
					//neat trick to detect single framebuffer interlacing
					if (interlc==1)
					{
						int diff=FB_R_SOF2-FB_R_SOF1;
						if (diff<0)
							diff=-diff;
						if (line_double)
							diff/=2;
						if ((diff/4)==(fb_skip-1))
						{
							src=FB_R_SOF1;
							fb_skip=1;
							interlc=0;
						}
					}
					else
					{
						//half lines needed on non-interlaced mode on non VGA streams
						if (!FB_R_CTRL.vclk_div)
						{
							rs.bottom/=2;
						}
					}

					u32* read=(u32*)&params.vram[vramlock_ConvOffset32toOffset64(src)];
					u32* read_line=read;

					u32* write=(u32*)lr.pBits;
					u32* line=(u32*)lr.pBits;

					if (pixel_double)
						rs.right/=2;
					if (line_double)
						rs.bottom/=2;

					u32 DWordsPerLine=PixelsPerLine*bpp/4;

					for (u32 y=0;y<(u32)rs.bottom;y+=1)
					{
						if (!interlc || out_field==(y&1))
						{
							if (bpp==3)
							{
								u8* br=(u8*)read;
								u8* bw=(u8*)write;
								for (u32 x=0;x<PixelsPerLine;x+=1)
								{
									*bw++=*br++;
									if (((u32)br&3)==0)
										br+=4;
									*bw++=*br++;
									if (((u32)br&3)==0)
										br+=4;
									*bw++=*br++;
									if (((u32)br&3)==0)
										br+=4;

									*bw++=0;	//skip A
								}
							}
							else
							{
								for (u32 x=0;x<DWordsPerLine;x+=1)
								{
									*write++=*read;
									read+=2;//skip the 'other' bank
								}
							}
							read_line+=(DWordsPerLine-1)*2;//*2 to skip the 'other' bank.-1 so that it points to the last pixel of the last line
							read_line+=fb_skip*2;//*2 to skip the 'other' bank
							read=read_line;
						}

						line+=lr.Pitch/4;
						write=line;
					}

					verifyc(surf->UnlockRect());
				}
			}
			else
			{
				tex=0;
				surf=0;
			}

			//if 0, then simply colorfill (fb is off)
			if (surf!=0)
			{
				//perform aspect ratio matching here ..
				//this is wayyyy wayy way way way more complicated than it needs, its what psy calls razimaths(tm)
				//it actually works
				void CalcRect(float* dst,float* src);
				float rdf[2]={(float)rs.right,(float)rs.bottom};
				float rsf[2]={(float)ppar.BackBufferWidth,(float)ppar.BackBufferHeight};
				RECT rd;
				CalcRect(rdf,rsf);
				rd.right=(LONG)(0.5f+rs.right/rdf[0]*rsf[0]);
				rd.bottom=(LONG)(0.5f+rs.bottom/rdf[1]*rsf[1]);
				rd.left=(LONG)(abs((LONG)ppar.BackBufferWidth-rd.right)/2);
				rd.top=(LONG)(abs((LONG)ppar.BackBufferHeight-rd.bottom)/2);
				
				rd.right+=rd.left;
				rd.bottom+=rd.top;
				if (rd.right>(LONG)ppar.BackBufferWidth)
					rd.right=(LONG)ppar.BackBufferWidth;

				if (rd.bottom>(LONG)ppar.BackBufferHeight)
					rd.bottom=(LONG)ppar.BackBufferHeight;

				dev->StretchRect(surf,&rs,backbuffer,&rd, D3DTEXF_LINEAR);	//add an option for D3DTEXF_POINT for pretty pixels?
			}

			dev->SetRenderTarget(0,backbuffer);
			r_set_texture(0,tex);
			r_set_texture(1,tex);
			dev->SetVertexShader(Composition.vs);
			dev->SetPixelShader(Composition.ps_DrawFB);
			
			float fscale[8]=
			{
				1/320.f,-1.0f/240.f,320,240,
				1/320.f,-1.0f/240.f,320,240
			};
			dev->SetVertexShaderConstantF(0,fscale,2);
			dev->SetPixelShaderConstantF(0,fscale,2);

			f32 fsq[] = 
			{
				0		,0		,0.5,1,		1,1,1,1,	0,0,
				0		,480	,0.5,1,		1,1,1,1,	0,1,
				640		,0		,0.5,1,		1,1,1,1,	1,0,
				640		,480	,0.5,1,		1,1,1,1,	1,1,
			};

			r_set_state(R_CULLMODE,D3DCULL_NONE);
			verifyc(dev->SetVertexDeclaration(vdecl_osd));
			
			r_set_state(R_ALPHABLENDENABLE,FALSE);
			r_set_state(R_ZENABLE,FALSE);

			//(dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,fsq,10*4));

			DrawOSD();
			(dev->Present(0,0,0,0));
		}
		LeaveCriticalSection(&d3d_lock);
	}

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
	const D3DVERTEXELEMENT9 vertelem_mv[] =
	{
		{0, 0,  D3DDECLTYPE_FLOAT3		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,0},
		D3DDECL_END()
	};
	const D3DVERTEXELEMENT9 vertelem_osd[] =
	{
		{0, 0,  D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,0},
		{0, 16,  D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,0},
		{0, 32,  D3DDECLTYPE_FLOAT2		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},
		D3DDECL_END()
	};
	const D3DVERTEXELEMENT9 vertelem[] =
	{
		{0, 0,  D3DDECLTYPE_FLOAT3		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,0},
		#ifdef _float_colors_
			{0, 12, D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},	//Base color
			{0, 28, D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1},	//Specular(offset) color
			#ifdef _HW_INT_
				{0, 44, D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},	//u,v,base intesity , offset intesity
			#else
				{0, 44, D3DDECLTYPE_FLOAT2		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},	//u,v,base intesity , offset intesity
			#endif
		#else
			{0, 12, D3DDECLTYPE_D3DCOLOR	, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},	//Base color
			{0, 16, D3DDECLTYPE_D3DCOLOR	, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1},	//Specular color
			{0, 20, D3DDECLTYPE_FLOAT4		, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},	//u,v,base intesity , offset intesity
		#endif
		
		D3DDECL_END()
	};

	

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
	struct ModParam
	{
		u32 first;		//entry index , holds vertex/pos data
		u32 count;
	};
	struct ModTriangle
	{
		f32 x0,y0,z0,x1,y1,z1,x2,y2,z2;
	};


	static Vertex* vert_reappend;

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
	
	
	bool UsingAutoSort()
	{
		if (((FPU_PARAM_CFG>>21)&1) == 0)
			return ((ISP_FEED_CFG&1)==0);
		else
			return ( (vri(REGION_BASE)>>29) & 1) == 0;
	}

	TA_context tarc;
	TA_context pvrrc;
 
	
bool operator<(const PolyParam &left, const PolyParam &right)
{
/* put any condition you want to sort on here */
	return left.zvZ<right.zvZ;
	//return left.zMin<right.zMax;
}
	void SortPParams()
	{
		if (pvrrc.verts.allocate_list_sz->size()==0)
			return;

		u32 base=0;
		u32 csegc=0;
		u32 cseg=-1;
		Vertex* bptr=0;
 

		//g_pvr_sort_module
		for (u32 i=0;i<pvrrc.global_param_tr.used;i++)
		{
			u32 s=pvrrc.global_param_tr.data[i].first;
			u32 c=pvrrc.global_param_tr.data[i].count;

			
			//float zmax=-66666666666,zmin=66666666666;
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
				/*if (zmax<bptr[j].z)
					zmax=bptr[j].z;
				if (zmin>bptr[j].z)
					zmin=bptr[j].z;*/
					
			}
			pvrrc.global_param_tr.data[i].zvZ=zv/c;
			/*pvrrc.global_param_tr.data[i].zMax=zmax;
			pvrrc.global_param_tr.data[i].zMin=zmin;*/
		}

		 
		std::stable_sort(pvrrc.global_param_tr.data,pvrrc.global_param_tr.data+pvrrc.global_param_tr.used);
	}

	std::vector<TA_context> rcnt;
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


	PolyParam* CurrentPP=0;
	List<PolyParam>* CurrentPPlist;
	
	template<u32 state>
	void SetTexMode(u32 clamp,u32 mirror)
	{
		if (clamp)
			r_set_sampler_state(0,state,D3DTADDRESS_CLAMP);
		else 
		{
			if (mirror)
				r_set_sampler_state(0,state,D3DTADDRESS_MIRROR);
			else
				r_set_sampler_state(0,state,D3DTADDRESS_WRAP);
		}
		
	}

	TSP cache_tsp;
	TCW cache_tcw;
	//PCW cache_pcw;
	ISP_TSP cache_isp;
	u32 cache_clipmode=0xFFFFFFFF;
	bool cache_clip_alpha_on_zero=true;
	u32 cache_texture_enabled=0;
	u32 cache_stencil_modvol_on=false;
	void GPstate_cache_reset(PolyParam* gp)
	{
		cache_tsp.full = ~gp->tsp.full;
		cache_tcw.full = ~gp->tcw.full;
		//cache_pcw.full = ~gp->pcw.full;
		cache_isp.full = ~gp->isp.full;
		cache_clipmode=0xFFFFFFFF;
		cache_clip_alpha_on_zero=true;
		cache_texture_enabled=~gp->pcw.Texture;
		cache_stencil_modvol_on=0;
		last_ps_mode=-1;
	}
	//for fixed pipeline
	__forceinline
	void SetGPState_fp(PolyParam* gp)
	{
		if (gp->pcw.Texture)
		{
			r_set_state(R_SPECULARENABLE,gp->pcw.Offset );

			//if (gp->tsp.ShadInstr!=cache_tsp.ShadInstr ||  ( gp->tsp.UseAlpha != cache_tsp.UseAlpha) )
			{
				switch(gp->tsp.ShadInstr)	// these should be correct, except offset
				{
					//PIXRGB = TEXRGB + OFFSETRGB
					//PIXA    = TEXA
				case 0:	// Decal
					r_set_texture_stage_state(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
					r_set_texture_stage_state(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

					r_set_texture_stage_state(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

					if (gp->tsp.IgnoreTexA)
					{
						//a=1
						r_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
					}
					else
					{
						//a=tex.a
						r_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					}
					break;

					//The texture color value is multiplied by the Shading Color value.  
					//The texture  value is substituted for the Shading a value.
					//PIXRGB = COLRGB x TEXRGB + OFFSETRGB
					//PIXA   = TEXA
				case 1:	// Modulate
					r_set_texture_stage_state(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
					r_set_texture_stage_state(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					r_set_texture_stage_state(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

					r_set_texture_stage_state(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

					if (gp->tsp.IgnoreTexA)
					{
						//a=1
						r_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
					}
					else
					{
						//a=tex.a
						r_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					}
					break;
					//The texture color value is blended with the Shading Color 
					//value according to the texture a value.
					//PIXRGB = (TEXRGB x TEXA) +
					//(COLRGB x (1- TEXA) ) +
					//OFFSETRGB
					//PIXA   = COLA
				case 2:	// Decal Alpha
					if (gp->tsp.IgnoreTexA)
					{
						//Tex.a=1 , so Color = Tex
						r_set_texture_stage_state(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
					}
					else
					{
						r_set_texture_stage_state(0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA);
					}
					r_set_texture_stage_state(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					r_set_texture_stage_state(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

					r_set_texture_stage_state(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
					if(gp->tsp.UseAlpha)
					{
						r_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
					}
					else
					{
						r_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
					}
					break;

					//The texture color value is multiplied by the Shading Color value. 
					//The texture a value is multiplied by the Shading a value.
					//PIXRGB= COLRGB x  TEXRGB + OFFSETRGB
					//PIXA   = COLA  x TEXA
				case 3:	// Modulate Alpha
					r_set_texture_stage_state(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
					r_set_texture_stage_state(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					r_set_texture_stage_state(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

					if(gp->tsp.UseAlpha)
					{
						if (gp->tsp.IgnoreTexA)
						{
							//a=Col.a
							r_set_texture_stage_state(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
						}
						else
						{
							//a=Text.a*Col.a
							r_set_texture_stage_state(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
						}
					}
					else
					{
						if (gp->tsp.IgnoreTexA)
						{
							//a= 1
							r_set_texture_stage_state(0, D3DTSS_ALPHAOP,   D3DTA_TFACTOR);
						}
						else
						{
							//a= Text.a*1
							r_set_texture_stage_state(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
						}
					}
					r_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					r_set_texture_stage_state(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

					break;
				}
			}

		}
		else
		{
			//Offset color is enabled olny if Texture is enabled ;)
			r_set_state(R_SPECULARENABLE,FALSE);
			r_set_texture(0,NULL);
		}
	}
	//fox pixel shaders

	//Texture -> 1 if texture is enabled , 0 if its not
	//Offset -> 1 if offset is enabled , 0 if its not (only valid when texture is enabled)
	//ShadInstr -> 0 to 3 , see pvr docs , valid only when texture is enabled
	//IgnoreTexA -> 1 if on  0 if off , valid only w/ textures on
	//UseAlpha -> 1 if on  0 if off , works when no textures are used too ?

	#define idx_pp_Texture 0
	#define idx_pp_Offset 1
	#define idx_pp_ShadInstr 2
	#define idx_pp_IgnoreTexA 3
	#define idx_pp_UseAlpha 4
	#define idx_pp_TextureLookup 5
	#define idx_ZBufferMode 6
	#define idx_pp_FogCtrl 7

	D3DXMACRO ps_macros[]=
	{
		{"pp_Texture","0"},
		{"pp_Offset","0"},
		{"pp_ShadInstr","0"},
		{"pp_IgnoreTexA","0"},
		{"pp_UseAlpha","0"},
		{"TextureLookup","0"},	//use shader to emulate pals
		{"ZBufferMode","0"},		//Z mode. 0 -> D24FS8, 1 -> D24S8 + FPemu, 2 -> D24S8 + scaling
		{"pp_FogCtrl","0"},
		{0,0}	//end of list
	};
	// -> function to do projected lookup
// -> function to use for texture lookup.One of TextureLookup_Normal,TextureLookup_Palette,TextureLookup_Palette_Bilinear
	char* ps_macro_numers[] =
	{
		"0",
		"1",
		"2",
		"3",
		"4",
	};
	char* ps_macro_TLUM[]=
	{
		"TextureLookup_Normal",
		"TextureLookup_Palette",
		"TextureLookup_Palette_Bilinear",
	};

	void WirteCacheShader(FILE* f,DWORD* shader,u32 mode,u32 len)
	{
		if (!f)
			return;

		fwrite(&mode,4,1,f);
		fwrite(&len,4,1,f);
		if (len!=-1)
			fwrite(shader,len,1,f);
	}

	void CompilePS(u32 mode,const char* profile,FILE* f)
	{
		verify(mode<(PS_SHADER_COUNT));
		if (compiled_ps[mode]!=0)
			return;
		ID3DXBuffer* perr;
		ID3DXBuffer* shader;
		ID3DXConstantTable* consts;

		D3DXCompileShaderFromFileA("ps_hlsl.fx"
			,ps_macros,NULL,"PixelShader_main",profile,SHADER_DEBUG/*D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION*/,&shader,&perr,&consts);
		if (perr)
		{
			char* text=(char*)perr->GetBufferPointer();
			printf("%s\n",text);
		}
		verifyc(dev->CreatePixelShader((DWORD*)shader->GetBufferPointer(),&compiled_ps[mode]));
		WirteCacheShader(f,(DWORD*)shader->GetBufferPointer(),mode,shader->GetBufferSize());
		if (perr)
			perr->Release();
		shader->Release();
		consts->Release();
	}
	/*
		Cache file format :
		"drkpvr ps cache 0"
		u8[16] file hash; -> hash(stcat(file,ps_profile,extramodes))
		u32 mode;
		u32 size;
		u8[size] data; 

		if mode is -1 then this is the last item on the file :)
	*/
	bool hash_file(const char* shaderfile,const char* profile,const char* extra,u8* hash)
	{
		u8* pfile;
		FILE* fshade=fopen(shaderfile,"rb");
		if (!fshade)
			return false;
		
		fseek(fshade,0,SEEK_END);
		u32 fsize=ftell(fshade);
		fseek(fshade,0,SEEK_SET);
		u32 buffsize=fsize+strlen(profile)+strlen(extra)+2+1;
		pfile = (u8*)malloc(buffsize);
		fread(pfile,1,fsize,fshade);
		fclose(fshade);

		pfile[fsize]=0;
		strcat((char*)pfile,";");
		strcat((char*)pfile,profile);
		strcat((char*)pfile,";");
		strcat((char*)pfile,extra);
		if(strlen((char*)pfile)!=(buffsize-1))
			die("Error on hashing file ...");
		memset(hash,0,16);
		
		for (u32 i=0;i<buffsize;i++)
		{
			hash[i&15]+=pfile[i];
			hash[i&15]^=(pfile[i]+1);
			u32 sp=(pfile[i]+hash[~i&15])&15;
			hash[sp]^=hash[15-sp]*7;		//7 is the most powerful magic number, no ;)
			hash[15-sp]^=hash[sp]/7;		//i hope it is !
		}
		free(pfile);
		return true;
	}
	bool LoadFileCache(char* cache,u8* hash)
	{
		FILE* f;
		char* tag ="drkpvr ps cache 0";
		char temp[128];
		memset(compiled_ps,0,sizeof(compiled_ps));

		f=fopen(cache,"rb");
		if (!f)
			return false;
		
		//check file tag
		if (fread(temp,strlen(tag),1,f)!=1)
			goto __error_out;
		if (memcmp(temp,tag,strlen(tag)))
			goto __error_out;//GOTO is evil, right ? >:~

		//check hash value
		if (fread(temp,16,1,f)!=1)
			goto __error_out;
		if (memcmp(temp,hash,16))
			goto __error_out;

		int chunk_size;
		u32 chunk_mode;

		for(;;)
		{
			if (fread(&chunk_mode,4,1,f)!=1)
				goto __error_out;
			if (fread(&chunk_size,4,1,f)!=1)
				goto __error_out;
			if (chunk_size<0)
				break;
			u8* databuf=(u8*)malloc(chunk_size);
			if (fread(databuf,chunk_size,1,f)!=1)
				goto __error_out;
			if (FAILED(dev->CreatePixelShader((DWORD*)databuf,&compiled_ps[chunk_mode])))
				goto __error_out;
			free(databuf);
		}

		fclose(f);
		return true;

__error_out:
		for (u32 i=0;i<PS_SHADER_COUNT;i++)
		{
			if (compiled_ps[i])
				compiled_ps[i]->Release();
			compiled_ps[i]=0;
		}
		fclose(f);
		return false;
	}
	FILE* CreateCacheShader(const char* name,const u8* hash)
	{
		FILE* rv=fopen(name,"wb");
		if (!rv)
			return 0;
		char* tag ="drkpvr ps cache 0";
		fwrite(tag,strlen(tag),1,rv);
		fwrite(hash,16,1,rv);
		return rv;
	}
	void FinishCacheShader(FILE *f)
	{
		u32 md=0;
		s32 sz=-1;
		fwrite(&md,4,1,f);
		fwrite(&sz,4,1,f);
		fclose(f);
	}
	void PrecompilePS()
	{
		char temp[30];
		strcpy(temp,D3DXGetPixelShaderProfile(dev));
		//printf(&temp[3]);

		ps_macros[idx_ZBufferMode].Definition=ps_macro_numers[ZBufferMode];

		const char * profile=D3DXGetPixelShaderProfile(dev);

		u8 file_hash[16];
		if (hash_file("ps_hlsl.fx",profile,ps_macros[idx_ZBufferMode].Definition,file_hash))
		{
			printf("ps_hlsl.fx hash : ");
			for (int i=0;i<16;i++)
				printf("%02X",file_hash[i]);
			printf("\n");
			if (LoadFileCache("ps_hlsl.fxc",file_hash))
			{
				printf("Using cached ps_hlsl.fx\n");
				return;
			}
		}
		printf("ps_hlsl.fx cache invalid, rebuilding ..\n");
		
		FILE*cache = CreateCacheShader("ps_hlsl.fxc",file_hash);

		#define forl(n,s,e) for (u32 n=s;n<=e;n++)
		forl(Texture,0,1)
		{
			ps_macros[idx_pp_Texture].Definition=ps_macro_numers[Texture];

			forl(UseAlpha,0,1)
			{
				ps_macros[idx_pp_UseAlpha].Definition=ps_macro_numers[UseAlpha];
				forl(FogCtrl,0,3)
				{
					ps_macros[idx_pp_FogCtrl].Definition=ps_macro_numers[FogCtrl];
					if (Texture)
					{
						forl(pal_tex,0,2)
						{
							ps_macros[idx_pp_TextureLookup].Definition=ps_macro_TLUM[pal_tex];
							forl(Offset,0,1)
							{
								ps_macros[idx_pp_Offset].Definition=ps_macro_numers[Offset];
								forl(ShadInstr,0,3)
								{
									ps_macros[idx_pp_ShadInstr].Definition=ps_macro_numers[ShadInstr];
									forl(IgnoreTexA,0,1)
									{
										ps_macros[idx_pp_IgnoreTexA].Definition=ps_macro_numers[IgnoreTexA];

										u32 mode=0;			
										mode|=pal_tex;
										mode<<=1;
										mode|=Offset;
										mode<<=2;
										mode|=ShadInstr;
										mode<<=1;
										mode|=IgnoreTexA;
										mode<<=1;
										mode|=Texture;
										mode<<=1;
										mode|=UseAlpha;
										mode<<=2;
										mode|=FogCtrl;

										CompilePS(mode,profile,cache);
									}
								}
							}
						}
					}
					else
					{
						ps_macros[idx_pp_Offset].Definition=ps_macro_numers[0];
						ps_macros[idx_pp_ShadInstr].Definition=ps_macro_numers[0];
						ps_macros[idx_pp_IgnoreTexA].Definition=ps_macro_numers[0];

						u32 mode=0;			
						mode|=Texture;
						mode<<=1;
						mode|=UseAlpha;
						mode<<=2;
						mode|=FogCtrl;

						CompilePS(mode,profile,cache);
					}
				}
			}
		}
		#undef forl
		FinishCacheShader(cache);
	}
	void FASTCALL ResrotePS()
	{
		dev->SetPixelShader(compiled_ps[last_ps_mode]);
	}
	void FASTCALL SetPS(u32 mode)
	{
		if (last_ps_mode!=mode)
		{
			last_ps_mode=mode;
			dev->SetPixelShader(compiled_ps[mode]);
		}
	}
	float cur_pal_index[4]={0,0,0,1};
	__forceinline
	void SetGPState_ps(PolyParam* gp)
	{
		u32 mode=0;
		if (gp->pcw.Texture)
		{
			if (settings.Emulation.PaletteMode>1)
			{
				u32 pal_mode=settings.Emulation.PaletteMode-1;
				if (pal_mode==2 && gp->tsp.FilterMode==0)
					pal_mode=1;//no filter .. ugh
					
				u32 pf=gp->tcw.PAL.PixelFmt;
				if (pf==5)
				{
					cur_pal_index[1]=gp->tcw.PAL.PalSelect/64.0f;
					mode|=pal_mode;
					dev->SetPixelShaderConstantF(0,cur_pal_index,1);
				}
				else if (pf==6)
				{
					cur_pal_index[1]=(gp->tcw.PAL.PalSelect&~0xF)/64.0f;
					mode|=pal_mode;
					dev->SetPixelShaderConstantF(0,cur_pal_index,1);
				}
			}
			
			mode<<=1;
			mode|=gp->pcw.Offset;

			mode<<=2;
			mode|=gp->tsp.ShadInstr;

			mode<<=1;
			mode|=gp->tsp.IgnoreTexA;			
		}

		mode<<=1;
		mode|=gp->pcw.Texture;
		

		mode<<=1;
		mode|=gp->tsp.UseAlpha;

		mode<<=2;
		mode|=gp->tsp.FogCtrl;

		SetPS(mode);
	}

	//realy only uses bit0, destroys all of em atm :]

	void SetTileClip(u32 val)
	{
		if (cache_clipmode==val)
			return;
		cache_clipmode=val;

		u32 clipmode=val>>28;

		if (clipmode<2 ||clipmode&1 )
		{
			r_set_state(R_CLIPPLANEENABLE,0);
		}
		else
		{
			clipmode&=1; 
			/*if (clipmode&1)
				r_set_state(R_CLIPPLANEENABLE,3);
			else*/
				r_set_state(R_CLIPPLANEENABLE,15);

			float x_min=(float)(val&63);
			float x_max=(float)((val>>6)&63);
			float y_min=(float)((val>>12)&31);
			float y_max=(float)((val>>17)&31);
			x_min=x_min*32;
			x_max=x_max*32 +31.999f;	//+31.999 to do [min,max)
			y_min=y_min*32;
			y_max=y_max*32 +31.999f;
			
			x_min+=current_scalef[0];
			x_max+=current_scalef[0];
			y_min+=current_scalef[1];
			y_max+=current_scalef[1];

			x_min/=current_scalef[2];
			x_max/=current_scalef[2];
			y_min/=current_scalef[3];
			y_max/=current_scalef[3];



			//Ax + By + Cz + Dw
			float v[4];
			float clips=1.0f-clipmode*2;
			v[0]=clips;
			v[1]=0;
			v[2]=0;
			v[3]=-(x_min-1)*clips ;
			verifyc(dev->SetClipPlane(0,v));

			v[0]=-clips;
			v[1]=0;
			v[2]=0;
			v[3]=(x_max-1)*clips ;
			verifyc(dev->SetClipPlane(1,v));

			v[0]=0;
			v[1]=-clips;
			v[2]=0;
			v[3]=(1+y_min)*clips;
			verifyc(dev->SetClipPlane(2,v));

			v[0]=0;
			v[1]=clips;
			v[2]=0;
			v[3]=-(1+y_max)*clips ;
			verifyc(dev->SetClipPlane(3,v));
		}
		
	}
	const float TextureSizes[8][2]=
	{
		8.f,1/8.f,
		16.f,1/16.f,
		32.f,1/32.f,
		64.f,1/64.f,
		128.f,1/128.f,
		256.f,1/256.f,
		512.f,1/512.f,
		1024.f,1/1024.f,
	};
	//
	template <u32 Type,bool FFunction,bool df,bool SortingEnabled>
	__forceinline
	void SetGPState(PolyParam* gp,u32 cflip=0)
	{	/*
		if (gp->tsp.DstSelect ||
			gp->tsp.SrcSelect)
			printf("DstSelect  DstSelect\n"); */

		SetTileClip(gp->tileclip);
		//has to preserve cache_tsp/cache_isp
		//can freely use cache_tcw
		if (FFunction)
		{
			SetGPState_fp(gp);
		}
		else
		{
			SetGPState_ps(gp);
		}

		const u32 stencil=(gp->pcw.Shadow!=0)?0x80:0;
		if (cache_stencil_modvol_on!=stencil && settings.Emulation.ModVolMode==MVM_NormalAndClip)
		{
			cache_stencil_modvol_on=stencil;
			r_set_state(R_STENCILREF,stencil);						//Clear/Set bit 7 (Clear for non 2 volume stuff)
		}
		

		if ((gp->tcw.full != cache_tcw.full) || (gp->tsp.full!=cache_tsp.full) || (cache_texture_enabled!= gp->pcw.Texture))
		{
			cache_tcw=gp->tcw;
			cache_texture_enabled = gp->pcw.Texture;

			if ( gp->tsp.FilterMode == 0 || (settings.Emulation.PaletteMode>1 && ( gp->tcw.PAL.PixelFmt==5|| gp->tcw.PAL.PixelFmt==6) ))
			{
				r_set_sampler_state(0, R_MINFILTER, D3DTEXF_POINT);
				r_set_sampler_state(0, R_MAGFILTER, D3DTEXF_POINT);
				r_set_sampler_state(0, R_MIPFILTER, D3DTEXF_POINT);	//_NONE ? this disables mipmapping alltogether ?
			}
			else
			{
				r_set_sampler_state(0, R_MINFILTER, D3DTEXF_LINEAR);
				r_set_sampler_state(0, R_MAGFILTER, D3DTEXF_LINEAR);
				r_set_sampler_state(0, R_MIPFILTER, D3DTEXF_LINEAR);	//LINEAR for Trilinear filtering
			}

			if (gp->tsp.full!=cache_tsp.full)
			{
				cache_tsp=gp->tsp;

				if (Type==ListType_Translucent)
				{
					r_set_state(R_SRCBLEND, SrcBlendGL[gp->tsp.SrcInstr]);
					r_set_state(R_DESTBLEND, DstBlendGL[gp->tsp.DstInstr]);
					bool clip_alpha_on_zero=gp->tsp.SrcInstr==4 && (gp->tsp.DstInstr==1 || gp->tsp.DstInstr==5);
					if (clip_alpha_on_zero!=cache_clip_alpha_on_zero)
					{
						cache_clip_alpha_on_zero=clip_alpha_on_zero;
						r_set_state(R_ALPHATESTENABLE,clip_alpha_on_zero);
					}
				}

				SetTexMode<R_ADDRESSV>(gp->tsp.ClampV,gp->tsp.FlipV);
				SetTexMode<R_ADDRESSU>(gp->tsp.ClampU,gp->tsp.FlipU);
			}

			if (gp->pcw.Texture)
			{
				IDirect3DTexture9* tex=GetTexture(gp->tsp,gp->tcw);
				r_set_texture(0,tex);
				float tsz[4];
				tsz[0]=TextureSizes[gp->tsp.TexU][0];
				tsz[2]=TextureSizes[gp->tsp.TexU][1];
				tsz[1]=TextureSizes[gp->tsp.TexV][0];
				tsz[3]=TextureSizes[gp->tsp.TexV][1];

				dev->SetPixelShaderConstantF(1,tsz,1);
				dev->SetVertexShaderConstantF(3,tsz,1);
			}
		}

		if (df)
		{
			r_set_state(R_CULLMODE,CullMode[gp->isp.CullMode+cflip]);
		}
		if (gp->isp.full!= cache_isp.full)
		{
			cache_isp.full=gp->isp.full;
			//set cull mode !
			if (!df)
				r_set_state(R_CULLMODE,CullMode[gp->isp.CullMode]);
			//set Z mode !
			if (Type==ListType_Opaque)
			{
				r_set_state(R_ZFUNC,Zfunction[gp->isp.DepthMode]);
			}
			else if (Type==ListType_Translucent)
			{
				if (SortingEnabled)
					r_set_state(R_ZFUNC,Zfunction[6]); // : GEQ
				else
					r_set_state(R_ZFUNC,Zfunction[gp->isp.DepthMode]);
			}
			else
			{
				//gp->isp.DepthMode=6;
				r_set_state(R_ZFUNC,Zfunction[6]); //PT : LEQ //GEQ ?!?! wtf ? seems like the docs ARE wrong on this one =P
			}

			r_set_state(R_ZWRITEENABLE,gp->isp.ZWriteDis==0);
		}
	}
	template <u32 Type,bool FFunction,bool SortingEnabled>
	__forceinline
	void RendStrips(PolyParam* gp)
	{
			SetGPState<Type,FFunction,false,SortingEnabled>(gp);
			if (gp->count>2)//0 vert polys ? why does games even bother sending em  ? =P
			{		
				dev->DrawPrimitive(D3DPT_TRIANGLESTRIP,gp->first ,
					gp->count-2);
			}
	}

	template <u32 Type,bool FFunction,bool SortingEnabled>
	void RendPolyParamList(List<PolyParam>& gpl)
	{
		if (gpl.used==0)
			return;
		//we want at least 1 PParam

		//reset the cache state
		GPstate_cache_reset(&gpl.data[0]);

		for (u32 i=0;i<gpl.used;i++)
		{		
			RendStrips<Type,FFunction,SortingEnabled>(&gpl.data[i]);
		}
	}
	
	struct SortTrig
	{
		f32 z;
		u32 id;
		PolyParam* pparam;
	};

	bool operator<(const SortTrig &left, const SortTrig &right)
	{
		/* put any condition you want to sort on here */
		return left.z<right.z;
		//return left.zMin<right.zMax;
	}

	 

	pvr_algo::pvr_sort_c g_pvr_sort_algo;
	vector<pvr_algo::poly_indice_pair_t> pvr_srt_tmp0;
	vector<pvr_algo::poly_indice_pair_t> pvr_srt_tmp1;

 
	//sort and render , only for alpha blended stuff
	template <bool FFunction>
	void SortRendPolyParamList(List<PolyParam>& gpl)
	{
		if ( (gpl.used==0) || (pvrrc.verts.allocate_list_sz->size()==0)) {
			return;
		}
		
		pvr_srt_tmp0.clear();

		u32 base=0;
		u32 csegc=0;
		u32 cseg=-1;
		Vertex* bptr=0;
	//	f32 t1=0;

		pvr_algo::poly_indice_pair_t poly_sort_ref;

		for (u32 i=0;i<pvrrc.global_param_tr.used;i++)
		{
			PolyParam* gp=&gpl.data[i];
			if (gp->count<3)
				continue;
			u32 s=gp->first;
			u32 c=gp->count-2;

			//float *p1=&t1,*p2=&t1;
			for (u32 j=s;j<(s+c);j++)
			{
				while (j>=csegc)
				{
					cseg++;
					bptr=(Vertex*)((*pvrrc.verts.allocate_list_ptr)[cseg]);
					bptr-=csegc;
					csegc+=(*pvrrc.verts.allocate_list_sz)[cseg]/sizeof(Vertex);
				}

				poly_sort_ref.z = bptr[j].z;
				poly_sort_ref.id = j;
				poly_sort_ref.gp = i;
	 
				pvr_srt_tmp0.push_back(poly_sort_ref);
			}
		}

		if (pvr_srt_tmp0.empty()) {
			return;
		}

		if (pvr_srt_tmp0.size() > pvr_srt_tmp1.size()) {
			pvr_srt_tmp1.clear();
			pvr_srt_tmp1.resize(pvr_srt_tmp0.size());
		} 

		g_pvr_sort_algo.sort(&pvr_srt_tmp0[0],&pvr_srt_tmp1[0],(u32)pvr_srt_tmp0.size());
 
		//reset the cache state
		GPstate_cache_reset( &gpl.data[ pvr_srt_tmp0[0].gp ]);

		for (u32 i=0,j = (u32)pvr_srt_tmp0.size();i<j;i++) {
			PolyParam* gp = &gpl.data[ pvr_srt_tmp0[i].gp ];
			u32 id = pvr_srt_tmp0[i].id;
			u32 fl=((id - gp->first)&1)<<2;
			SetGPState<ListType_Translucent,FFunction,true,true>(gp,fl);
			dev->DrawPrimitive(D3DPT_TRIANGLESTRIP,id,1); //FIXME
		}
		//printf("%d Render calls\n",pvr_srt_tmp0.size());
		pvr_srt_tmp0.clear();
	}

	void DrawOSD()
	{
		//r_set_state(R_ZFUNC,D3DCMP_ALWAYS);
		r_set_state(R_SRCBLEND, D3DBLEND_SRCALPHA);
		r_set_state(R_DESTBLEND, D3DBLEND_INVSRCALPHA);
		r_set_state(R_ALPHABLENDENABLE,TRUE);
		
		r_set_state( R_ALPHATESTENABLE,  TRUE );
		r_set_state( R_ALPHAREF,         0x08 );
		r_set_state( R_ALPHAFUNC,  D3DCMP_GREATEREQUAL );
		r_set_state( R_FILLMODE,   D3DFILL_SOLID );
		r_set_state( R_CULLMODE,   D3DCULL_CCW );
		r_set_state( R_STENCILENABLE,    FALSE );
		r_set_state( R_CLIPPING,         TRUE );
		r_set_state( R_CLIPPLANEENABLE,  FALSE );
		r_set_state( R_VERTEXBLEND,      D3DVBF_DISABLE );
		r_set_state( R_INDEXEDVERTEXBLENDENABLE, FALSE );
		r_set_state( R_FOGENABLE,        FALSE );
		r_set_state( R_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_RED  | D3DCOLORWRITEENABLE_GREEN |
		D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA );

		r_set_state( R_ZENABLE,FALSE);
		
		if (settings.OSD.ShowFPS)
		{
			// Create a rectangle to indicate where on the screen it should be drawn						
			
			timer = GetTickCount() - timeStart;
			
			if ( timer > 250 )
			{				
				timeStart = GetTickCount();

				frameRate = (u32)((FrameNumber - frameStart) * 1000.0f / timer);
				frameStart = FrameNumber;			
			}
			
			// frameRate

			wsprintf(fps_text, L"%i", frameRate);
			
			RECT rct;

			rct.left=10;
			rct.right=64;
			rct.top=10;
			rct.bottom=rct.top+30;
			

			//font->
			// Draw some text
			//i need to set a new PS/FP state here .. meh ... 
			font->DrawText(NULL, fps_text, -1, &rct, DT_LEFT, D3DCOLOR_ARGB(150,0x00,0xFF,0x00) );
		}
		if (settings.OSD.ShowStats)
		{
			wchar text[512];
			wchar* cpath_vs[2]={L"H/W VS1.1+",L"Emulated VS3"};
			wchar* cpath_ps[2]={L"PS2+",L"Fixed-Function"};

			wsprintf(text,
				L"Config : %s;%s" L"\n"
				L"Texture Cache : %d textures" L"\n",cpath_vs[UseSVP],cpath_ps[UseFixedFunction],TexCache.textures);
			
			RECT rct;
			rct.left=10;
			rct.right=780;
			rct.top=40;
			rct.bottom=rct.top+300;

			//font->
			// Draw some text
			//i need to set a new PS/FP state here .. meh ...
			font->DrawText(NULL, text, -1, &rct, DT_LEFT, D3DCOLOR_ARGB(150,0x00,0x00,0xFF)  );
		}
	}
	//
	void UpdatePaletteTexure()
	{
		palette_update();
		if (pal_texture==0)
			return;

		D3DLOCKED_RECT rect;
		//pal is organised as 16x64 texture
		pal_texture->LockRect(0,&rect,NULL,D3DLOCK_DISCARD);

		u8* tex=(u8*)rect.pBits;
		u8* src=(u8*)palette_lut;

		for (int i=0;i<64;i++)
		{
			memcpy(tex,src,16*4);
			tex+=rect.Pitch;
			src+=16*4;
		}
		pal_texture->UnlockRect(0);
	}
	void UpdateFogTableTexure()
	{
		if (fog_texture==0)
			return;
		
		D3DLOCKED_RECT rect;
		//fog is 128x1 texure
		//.bg -> .rg
		//ARGB 8888 -> B G R A -> B=7:0 aka '1', G=15:8 aka '0'
		//ARGB 8888 -> B G R A -> R=7:0 aka '1', G=15:8 aka '0'
		fog_texture->LockRect(0,&rect,NULL,D3DLOCK_DISCARD);

		u8* tex=(u8*)rect.pBits;

		//could just memcpy ;p
		u8* fog_table=(u8*)FOG_TABLE;
		for (int i=0;i<128;i++)
		{
			const register u32 offs = i << 2;
			tex[offs+0]=0;//B
			tex[offs+1]=fog_table[offs+0];//G
			tex[offs+2]=fog_table[offs+1];//R
			tex[offs+3]=0;//A
		}
		fog_texture->UnlockRect(0);
	}
	void SetMVS_Mode(u32 mv_mode,ISP_Modvol ispc)
	{
		if (mv_mode==0)	//normal trigs
		{
			//set states
			verifyc(r_set_state(R_ZENABLE,TRUE));
			verifyc(r_set_state(R_STENCILWRITEMASK,2));	//write bit 1
			verifyc(r_set_state(R_STENCILFUNC,D3DCMP_ALWAYS));	//allways pass
			verifyc(r_set_state(R_STENCILPASS,D3DSTENCILOP_INVERT));	//flip bit 1
			verifyc(r_set_state(R_STENCILZFAIL,D3DSTENCILOP_KEEP));		//else keep it
			r_set_state(R_CULLMODE,CullMode[ispc.CullMode]); //-> needs to be properly set
		}
		else
		{
			//1 (last in) or 2 (last out)
			//each trinagle forms the last of a volume
			if (mv_mode==1)
			{
				//res : old : final 
				//0   : 0      : 00
				//0   : 1      : 01
				//1   : 0      : 01
				//1   : 1      : 01

				//if !=0 -> set to 10
				verifyc(r_set_state(R_STENCILFUNC,D3DCMP_LESSEQUAL));	//if (st>=1) st=1; else st=0;
				verifyc(r_set_state(R_STENCILREF,1));					
				verifyc(r_set_state(R_STENCILPASS,D3DSTENCILOP_REPLACE));
				verifyc(r_set_state(R_STENCILFAIL,D3DSTENCILOP_ZERO));
			}
			else
			{
				printf("*************************************************************************\n");
				printf("**mv_mode!=1. THIS IS PROLLY BUGGED, REPORT IF NOT ON THE ISSUE TRACKER**\n");
				printf("*************************************************************************\n");
				//res : old : final 
				//0   : 0   : 00
				//0   : 1   : 00
				//1   : 0   : 00
				//1   : 1   : 01

				verifyc(r_set_state(R_STENCILFUNC,D3DCMP_GREATER));			//if (st>2) then st=1; else st=0
				verifyc(r_set_state(R_STENCILREF,2));						
				verifyc(r_set_state(R_STENCILPASS,D3DSTENCILOP_REPLACE));
				verifyc(r_set_state(R_STENCILFAIL,D3DSTENCILOP_ZERO));
			}

			//common states :)
			verifyc(r_set_state(R_ZENABLE,FALSE));	//no Z testing, we just want to sum up all of the modvol area ...
			verifyc(r_set_state(R_STENCILWRITEMASK,3));	//write 2 lower bits
			verifyc(r_set_state(R_STENCILMASK,3));		//read 2 lower ones
		}
	}
	//
	void DoRender()
	{
		dosort=UsingAutoSort();

		bool rtt=(FB_W_SOF1 & 0x1000000)!=0;

		void* ptr;
		if (FrameNumber-rtt_FrameNumber >60)
		{
			rtt_address=0xFFFFFFFF;
		}
		if (rtt)
		{
			rtt_FrameNumber=FrameNumber;
			rtt_address=FB_W_SOF1&VRAM_MASK;
			//write to the 'other' rtt buffer, read from the last writen one
			verifyc(dev->SetRenderTarget(0,rtt_surf[rtt_index^1]));
			//printf("Writing to %d\n",rtt_index^1);
		}
		else
		{
			fb_FrameNumber=FrameNumber;
			verifyc(dev->SetRenderTarget(0,fb_surf));
		}

		// Clear the backbuffer to a blue color
		//All of the screen is allways filled w/ smth , no need to clear the color buffer
		//gives a nice speedup on large resolutions
		r_set_state(R_SCISSORTESTENABLE, FALSE); 
		if (rtt || clear_rt==0)
		{
			verifyc(dev->Clear( 0, NULL, ZBufferCF  , D3DCOLOR_XRGB(0,0,0), 0.0f, 0 ));
		}
		else
		{
			verifyc(dev->Clear( 0, NULL, D3DCLEAR_TARGET | ZBufferCF , D3DCOLOR_XRGB(0,0,0), 0.0f, 0 ));
			if(clear_rt)
				clear_rt--;
		}

		pvrrc.verts.Finalise();
		u32 sz=pvrrc.verts.used*sizeof(Vertex);

		verifyc(vb->Lock(0,sz,&ptr,D3DLOCK_DISCARD));

		pvrrc.verts.Copy(ptr,sz);

		verifyc(vb->Unlock());

		//memset(pvrrc.verts.data,0xFEA345FD,pvrrc.verts.size*sizeof(Vertex));

		UpdatePaletteTexure();
		UpdateFogTableTexure();

		// Begin the scene
		if( SUCCEEDED( dev->BeginScene() ) )
		{	
			if (ppar.MultiSampleType!=D3DMULTISAMPLE_NONE)
				r_set_state(R_MULTISAMPLEANTIALIAS, true);
			/*
				Pal texture stuff
			*/
			if (pal_texture!=0)
			{
				verifyc(r_set_texture(1,pal_texture));
				verifyc(r_set_sampler_state(1, R_MINFILTER, D3DTEXF_POINT));
				verifyc(r_set_sampler_state(1, R_MAGFILTER, D3DTEXF_POINT));
			}

			if (fog_texture!=0)
			{
				verifyc(r_set_texture(2,fog_texture));
				verifyc(r_set_sampler_state(2, R_MINFILTER, D3DTEXF_POINT));
				verifyc(r_set_sampler_state(2, R_MAGFILTER, D3DTEXF_POINT));
			}

			//Init stuff
			dev->SetVertexShader(compiled_vs);

#define clamp(minv,maxv,x) min(maxv,max(minv,x))
			float bg=*(float*)&ISP_BACKGND_D; 

#ifdef scale_type_1
			float c0[4]={clamp(0,10000000.0f,pvrrc.invW_min)};
			float c1[4]={clamp(0.0000001f,10000000.0f,pvrrc.invW_max)};
			c0[0]*=0.99f;
			c1[0]*=1.01f;
//			printf("ZMAE: %f %f\n",c0[0],c1[0]);
			dev->SetVertexShaderConstantF(0,c0,1);
			dev->SetVertexShaderConstantF(1,c1,1);
#endif
			/*
				Set constants !
			*/

			//VERT and RAM constants
			u8* fog_colvert_bgra=(u8*)&FOG_COL_VERT;
			u8* fog_colram_bgra=(u8*)&FOG_COL_RAM;
			float ps_FOG_COL_VERT[4]={fog_colvert_bgra[2]/255.0f,fog_colvert_bgra[1]/255.0f,fog_colvert_bgra[0]/255.0f,1};
			float ps_FOG_COL_RAM[4]={fog_colram_bgra[2]/255.0f,fog_colram_bgra[1]/255.0f,fog_colram_bgra[0]/255.0f,1};

			dev->SetPixelShaderConstantF(2,ps_FOG_COL_VERT,1);
			dev->SetPixelShaderConstantF(3,ps_FOG_COL_RAM,1);

			//Fog density constant
			u8* fog_density=(u8*)&FOG_DENSITY;
			float fog_den_mant=fog_density[1]/128.0f;		//bit 7 -> x. bit, so [6:0] -> fraction -> /128
			s32 fog_den_exp=(s8)fog_density[0];
			float fog_den_float=fog_den_mant*pow(2.0f,fog_den_exp);

			float ps_FOG_DENSITY[4]= { fog_den_float,0,0,1 };
			dev->SetPixelShaderConstantF(4,ps_FOG_DENSITY,1);
			/*
				Setup initial render states
			*/
			r_set_state(R_ZENABLE,D3DZB_TRUE);

			dev->SetVertexDeclaration(vdecl);
			dev->SetStreamSource(0,vb,0,sizeof(Vertex));

			r_set_sampler_state(0, R_MINFILTER, D3DTEXF_LINEAR);
			r_set_sampler_state(0, R_MAGFILTER, D3DTEXF_LINEAR);

			r_set_texture_stage_state(0, D3DTSS_TEXTURETRANSFORMFLAGS,	D3DTTFF_COUNT4 | D3DTTFF_PROJECTED);
			
			r_set_state(R_TEXTUREFACTOR, 0xFFFFFFFF);

			//Opaque
			r_set_state(R_ALPHABLENDENABLE,FALSE);
			r_set_state(R_ALPHATESTENABLE,FALSE);

			//Scale values have the sync values
			//adjust em here for FB/AA/Stuff :)
			float scale_x=fb_scale[0];
			float scale_y=fb_scale[1];
			if (VO_CONTROL.pixel_double)
				scale_x*=0.5;
			else
				scale_x*=1;

			float x_scale_coef_aa=2.0;
			if (SCALER_CTL.hscale)
			{
				scale_x*=2;//2x resolution on X (AA)
				x_scale_coef_aa=1.0;
			}
			/*			float yscalef=SCALER_CTL.vscalefactor/1024.0f;
			scale_y*=1/yscalef;
			*/
			if (rtt)
			{
				current_scalef[0]=-(float)(FB_X_CLIP.min*scale_x);
				current_scalef[1]=-(float)(FB_Y_CLIP.min/**scale_y*/);
				current_scalef[2]=(float)(FB_X_CLIP.max+1)*0.5f*scale_x;
				current_scalef[3]=-(float)((FB_Y_CLIP.max+1)*0.5f)/**scale_y*/;
				r_set_state(R_SCISSORTESTENABLE, FALSE); 
			}
			else
			{
				current_scalef[0]=res_scale[0]*scale_x;
				current_scalef[1]=res_scale[1]*scale_y;
				current_scalef[2]=res_scale[2]*scale_x;
				current_scalef[3]=res_scale[3]*scale_y;

				//if widescreen mode == keep AR,Render extra
				if (settings.Enhancements.AspectRatioMode==2 && FB_Y_CLIP.min==0 && FB_Y_CLIP.max==479 && FB_X_CLIP.min==0 && FB_X_CLIP.max==639)
				{
					//rendering to frame buffer and not scissoring anything [yes this is a hack to allow widescreen hack]
					r_set_state(R_SCISSORTESTENABLE, FALSE); 
				}
				else
				{
					RECT srect;
					srect.top=(int)(0.5f+(current_scalef[1]/(-current_scalef[3]*2)*fb_surf_desc.Height+FB_Y_CLIP.min*fb_surf_desc.Height/(-current_scalef[3]*2)));
					srect.bottom=(int)(0.5f+(current_scalef[1]/(-current_scalef[3]*2)*fb_surf_desc.Height+((FB_Y_CLIP.max+1)*fb_surf_desc.Height/(-current_scalef[3]*2))));

					srect.left=(int)(0.5f+(current_scalef[0]/(current_scalef[2]*x_scale_coef_aa)*fb_surf_desc.Width+FB_X_CLIP.min*fb_surf_desc.Width/(current_scalef[2]*x_scale_coef_aa)));
					srect.right=(int)(0.5f+(current_scalef[0]/(current_scalef[2]*x_scale_coef_aa)*fb_surf_desc.Width+((FB_X_CLIP.max+1)*fb_surf_desc.Width/(current_scalef[2]*x_scale_coef_aa))));

					dev->SetScissorRect(&srect);
					r_set_state(R_SCISSORTESTENABLE, TRUE); 
				}
			}

			dev->SetVertexShaderConstantF(2,current_scalef,1);

			float res_align_offs[4]={ (-1.0f/fb_surf_desc.Width)-1.0f,(1.0f/fb_surf_desc.Height)+1.0f };
			dev->SetVertexShaderConstantF(4,res_align_offs,1);


			//stencil modes
			if (settings.Emulation.ModVolMode==MVM_NormalAndClip)
			{
				verifyc(r_set_state(R_STENCILENABLE,TRUE));
				verifyc(r_set_state(R_STENCILWRITEMASK,0xFF));				//write bit 7.I set em all here as a speed optimisation to minimise RMW operations
				verifyc(r_set_state(R_STENCILFUNC,D3DCMP_ALWAYS));			//allways pass
				verifyc(r_set_state(R_STENCILPASS,D3DSTENCILOP_REPLACE));	//flip bit 1
				verifyc(r_set_state(R_STENCILZFAIL,D3DSTENCILOP_KEEP));		//else keep it
				
				verifyc(r_set_state(R_STENCILREF,0x00));					//Clear/Set bit 7 (Clear for non 2 volume stuff)
			}
			else
			{
				verifyc(r_set_state(R_STENCILENABLE,FALSE));
			}
			

			//OPAQUE
			if (!GetAsyncKeyState(VK_F1))
			{
				if (UseFixedFunction)
				{
					RendPolyParamList<ListType_Opaque,true,false>(pvrrc.global_param_op);
				}
				else
				{
					RendPolyParamList<ListType_Opaque,false,false>(pvrrc.global_param_op);
				}
			}

			//Punch Through
			r_set_state(R_ALPHATESTENABLE,TRUE);

			r_set_state(R_ALPHAFUNC,D3DCMP_GREATEREQUAL);

			r_set_state(R_ALPHAREF,PT_ALPHA_REF &0xFF);
			
			verifyc(r_set_state(R_STENCILREF,0x00));					//Clear/Set bit 7 (Clear for non 2 volume stuff)

			if (!GetAsyncKeyState(VK_F2))
			{
				if (UseFixedFunction)
				{
					RendPolyParamList<ListType_Punch_Through,true,false>(pvrrc.global_param_pt);
				}
				else
				{
					RendPolyParamList<ListType_Punch_Through,false,false>(pvrrc.global_param_pt);
				}
			}

			
			//OP mod vols
			if (settings.Emulation.ModVolMode!=MVM_Off && pvrrc.modtrig.used>0)
			{
				if(ZBufferCF & D3DCLEAR_STENCIL && (settings.Emulation.ModVolMode==MVM_Normal || settings.Emulation.ModVolMode==MVM_NormalAndClip ))
				{
					/*
					mode :
					normal trig : flip
					last *in*   : flip, merge*in* &clear from last merge
					last *out*  : flip, merge*out* &clear from last merge
					*/
					r_set_state(R_ALPHABLENDENABLE,TRUE); //->BUG on nvdrivers (163 && 169 tested so far)
					r_set_state(R_ALPHATESTENABLE,FALSE);
					r_set_state(R_COLORWRITEENABLE,0);

					r_set_state(R_ZWRITEENABLE,FALSE);
					verifyc(r_set_state(R_ZFUNC,D3DCMP_GREATER));
					
					//TODO: Find out what ZPixelShader was supposed to be doing.
					verifyc(dev->SetPixelShader(ZPixelShader));
					verifyc(r_set_state(R_STENCILENABLE,TRUE));

					//we WANT stencil to have all 1's here for bit 1
					//set it as needed here :) -> not realy , we want em 0'd
					
					f32 fsq[] = {-640*8,-480*8,pvrrc.invW_min, -640*8,480*8,pvrrc.invW_min, 640*8,-480*8,pvrrc.invW_min, 640*8,480*8,pvrrc.invW_min};
					/*
					verifyc(r_set_state(R_ZENABLE,FALSE));						//Z doesnt matter
					verifyc(r_set_state(R_STENCILFUNC,D3DCMP_ALWAYS));			//allways pass
					verifyc(r_set_state(R_STENCILWRITEMASK,3));					//write bit 1
					verifyc(r_set_state(R_STENCILPASS,D3DSTENCILOP_REPLACE));	//Set to reference (2)
					verifyc(r_set_state(R_STENCILREF,2));						//reference value(2)

					verifyc(dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,fsq,3*4));
					*/
					//set correct declaration
					verifyc(dev->SetVertexDeclaration(vdecl_mod));

					u32 mod_base=0;	//cur base
					u32 mod_last=0; //last merge

					u32 cmv_count=(pvrrc.global_param_mvo.used-1);
					//ISP_Modvol
					for (u32 cmv=0;cmv<cmv_count;cmv++)
					{
						u32 sz=pvrrc.global_param_mvo.data[cmv+1].id;
						
						ISP_Modvol ispc=pvrrc.global_param_mvo.data[cmv];
						mod_base=ispc.id;
						sz-=mod_base;
						
						u32 mv_mode = ispc.DepthMode;
						
						//We read from Z buffer, but dont write :)
						verifyc(r_set_state(R_ZENABLE,TRUE));
						//enable stenciling, and set bit 1 for mod vols that dont pass the Z test as closed ones (not even count of em)


						if (mv_mode==0)	//normal trigs
						{
							SetMVS_Mode(0,ispc);
							//Render em (counts intersections)
							verifyc(dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST,sz,pvrrc.modtrig.data+mod_base,3*4));
						}
						else if (mv_mode<3)
						{
							while(sz)
							{
								//merge and clear all the prev. stencil bits
								
								//Count Intersections (last poly)
								SetMVS_Mode(0,ispc);
								verifyc(dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST,1,pvrrc.modtrig.data+mod_base,3*4));
								//Sum the area
								SetMVS_Mode(mv_mode,ispc);
								verifyc(dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST,mod_base-mod_last+1,pvrrc.modtrig.data+mod_last,3*4));

								//update pointers
								mod_last=mod_base+1;
								sz--;
								mod_base++;
							}
						}
						else
						{
							//die("Not supported mv_mode\n");
						}
					}

					//black out any stencil with '1'
					r_set_state(R_ALPHABLENDENABLE,TRUE);
					r_set_state(R_SRCBLEND, D3DBLEND_SRCALPHA);
					r_set_state(R_DESTBLEND, D3DBLEND_INVSRCALPHA); 

					r_set_state(R_COLORWRITEENABLE,0xF);
					verifyc(r_set_state(R_STENCILFUNC,D3DCMP_EQUAL));	//only the odd ones are 'in'
					
					u32 RefValue=0x01;	//'in'

					if (settings.Emulation.ModVolMode==MVM_NormalAndClip)
					{
						RefValue|=0x80;	//stencil volume mask
					}

					verifyc(r_set_state(R_STENCILREF,RefValue));	//allways (stencil volume mask && 'in')
					verifyc(r_set_state(R_STENCILMASK,RefValue));	//allways (as above)

					verifyc(r_set_state(R_STENCILWRITEMASK,0));	//dont write to stencil

					verifyc(r_set_state(R_ZENABLE,FALSE));

					verifyc(dev->SetPixelShader(ShadeColPixelShader));
					//render a fullscreen quad

					verifyc(dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,fsq,3*4));

					verifyc(r_set_state(R_STENCILENABLE,FALSE));	//turn stencil off ;)
				}
				else if (settings.Emulation.ModVolMode==MVM_Volume)
				{
					//TODO: Find out what ZPixelShader was supposed to be doing.
					verifyc(dev->SetPixelShader(ZPixelShader));
					r_set_state(R_ALPHABLENDENABLE,TRUE);
					r_set_state(R_SRCBLEND, D3DBLEND_SRCALPHA);
					r_set_state(R_DESTBLEND, D3DBLEND_INVSRCALPHA); 
					r_set_state(R_ALPHATESTENABLE,FALSE);

					r_set_state(R_ZENABLE,TRUE);
					r_set_state(R_ZWRITEENABLE,FALSE);
					r_set_state(R_CULLMODE,D3DCULL_NONE);

					verifyc(dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST,pvrrc.modtrig.used,pvrrc.modtrig.data,3*4));

					
				}

				r_set_state(R_ZWRITEENABLE,TRUE);
				r_set_state(R_ZENABLE,TRUE);
				r_set_state(R_ALPHATESTENABLE,TRUE); 

				dev->SetVertexDeclaration(vdecl);
				dev->SetStreamSource(0,vb,0,sizeof(Vertex));
				//bypass the ps cache and force it to set one :)
				SetPS(0);
				SetPS(1);
			}

			r_set_state(R_ALPHABLENDENABLE,TRUE);
			r_set_state(R_ALPHAFUNC,D3DCMP_GREATER);
			r_set_state(R_ALPHAREF,0);
			
			//Disable stencil (we don't support it on transcl. stuff anyway)
			verifyc(r_set_state(R_STENCILENABLE,FALSE));
			if (!GetAsyncKeyState(VK_F3))
			{
				if (dosort && settings.Emulation.AlphaSortMode==1)
					SortPParams();

				if (dosort && settings.Emulation.AlphaSortMode == 2)
				{
					if (UseFixedFunction)
					{
						SortRendPolyParamList<true>(pvrrc.global_param_tr);
					}
					else
					{
						SortRendPolyParamList<false>(pvrrc.global_param_tr);
					}	
				}
				else
				{
					if (UseFixedFunction)
					{
						if (dosort)
							RendPolyParamList<ListType_Translucent,true,true>(pvrrc.global_param_tr);
						else
							RendPolyParamList<ListType_Translucent,true,false>(pvrrc.global_param_tr);
					}
					else
					{
						if (dosort)
							RendPolyParamList<ListType_Translucent,false,true>(pvrrc.global_param_tr);
						else
							RendPolyParamList<ListType_Translucent,false,false>(pvrrc.global_param_tr);
					}
				}
			}
			

			// End the scene
			dev->EndScene();
			g_pvr_states_manager.invalidate_sampler(R_NO_SAMPLER);
		}

		if (rtt)
		{
			//swap RTT target for next time !
			rtt_index^=1;
		}
	}
	//
	volatile bool running=false;
	cResetEvent rs(false,true);
	cResetEvent re(false,true);
	D3DXMACRO vs_macros[]=
	{
		{"ZBufferMode",0},		//Z mode. 0 -> D24FS8, 1 -> D24S8 + FPemu, 2 -> D24S8 + scaling, 3 -> more scale, 4-> more, linear, scale
		{"FixedFunction",0},
		{0,0}	//end of list
	};
	
	void ListModes(void(* callback)(u32 w,u32 h,u32 rr))
	{
		IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
		D3DDISPLAYMODE mode;
		for (u32 i=0;i<d3d9->GetAdapterModeCount(D3DADAPTER_DEFAULT,D3DFMT_X8R8G8B8);i++)
		{
			d3d9->EnumAdapterModes(D3DADAPTER_DEFAULT,D3DFMT_X8R8G8B8,i,&mode);
			callback(mode.Width,mode.Height,mode.RefreshRate);
		}
		d3d9->Release();
	}

	IDirect3DVertexShader9* CompileVS(const char* file, const char* function,const char* vsp,const D3DXMACRO* pDefines)
	{
		IDirect3DVertexShader9* rv=0;
		ID3DXBuffer* perr;
		ID3DXBuffer* shader;
		
		verifyc(D3DXCompileShaderFromFileA(file,pDefines,NULL,function,vsp , SHADER_DEBUG, &shader,&perr,&shader_consts));
		
		if (perr)
		{
			char* text=(char*)perr->GetBufferPointer();
			printf("%s\n",text);
		}
		
		verifyc(dev->CreateVertexShader((DWORD*)shader->GetBufferPointer(),&rv));
		
		shader->Release();
		if (shader_consts) 
			shader_consts->Release();
		shader_consts=0;
		shader=0;

		return rv;
	}

	IDirect3DPixelShader9* CompilePS(const char* file, const char* function,const D3DXMACRO* pDefines)
	{
		IDirect3DPixelShader9* rv=0;
		ID3DXBuffer* perr;
		ID3DXBuffer* shader;
		ID3DXConstantTable* consts;

		D3DXCompileShaderFromFileA(file
			,pDefines,NULL,function,D3DXGetPixelShaderProfile(dev),SHADER_DEBUG,&shader,&perr,&consts);
		if (perr)
		{
			char* text=(char*)perr->GetBufferPointer();
			printf("%s\n",text);
		}
		verifyc(dev->CreatePixelShader((DWORD*)shader->GetBufferPointer(),&rv));
		if (perr)
			perr->Release();
		shader->Release();
		consts->Release();

		return rv;
	}

	#define safe_release(d) {if (d) {(d->Release()==0);d=0;}}
	#define safe_release2(d) {if (d) {verify(d->Release()==0);d=0;}}
	void Medidate_fb()
	{
		RECT cr;
		GetClientRect((HWND)emu.GetRenderTarget(),&cr);
		
		int win_width=cr.right-cr.left;
		int win_height=cr.bottom-cr.top;
		switch(settings.Video.ResolutionMode)
		{
		case 0:
		default:
			win_height=-1;
			win_width=-1;
			break;

		case 1: //HD(1280x800) or window
			if (win_height*win_width>1024000)
			{
				float rz=sqrtf(1024000.0f/(win_height*win_width));
				win_height = (int)(win_height * rz);
				win_width = (int)(win_width * rz);
			}
			break;
		
		case 2: //640x480
			{
				float ar=((float)win_width/(float)win_height);
				win_width=(s32)(480*ar);
				win_height=480;
			}
			break;
		
		case 3: //Half pixels
			win_height = (int)(win_height / sqrtf(2));
			win_width = (int)(win_width/ sqrtf(2));
			break;

		case 4: //Quarter pixels
			win_height/=2;
			win_width/=2;
			break;
		}
		
		ppar_BackBufferWidth=ppar_BackBufferHeight=-1;

		if (win_height >0 && win_width>0)
		{
			ppar_BackBufferWidth=win_width;
			ppar_BackBufferHeight=win_height;
		}

		
		if (ppar_BackBufferWidth==-1 && ppar_BackBufferHeight==-1)
		{
			ppar_BackBufferWidth=ppar.BackBufferWidth;
			ppar_BackBufferHeight=ppar.BackBufferHeight;
		}

		printf("drkpvr: Render target size %dx%d\n",ppar_BackBufferWidth,ppar_BackBufferHeight);

		u32 h=1;
		for(;h<ppar_BackBufferHeight;h<<=1)
			;
		h>>=1;
		u32 w=1;
		for(;w<ppar_BackBufferWidth;w<<=1)
			;
		w>>=1;

		for (int rtid=0;rtid<2;rtid++)
		{
			verifyc(dev->CreateTexture(w,h,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&rtt_texture[rtid],NULL));
			rtt_texture[rtid]->GetSurfaceLevel(0,&rtt_surf[rtid]);
		}

		verifyc(dev->CreateTexture(ppar_BackBufferWidth,ppar_BackBufferHeight,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&fb_texture,NULL));
		fb_texture->GetSurfaceLevel(0,&fb_surf);
		fb_surf->GetDesc(&fb_surf_desc);
		dev->GetRenderTarget(0,&backbuffer);
	}

	u32 THREADCALL RenderThead_internal(void* param)
	{
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);

		LARGE_INTEGER freq,InitStart,InitEnd;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&InitStart);
		render_restart=false;
		d3d_do_restart=false;
		d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
		// char temp[2][30]; // Unreferenced
		memset(&ppar,0,sizeof(ppar));

		LoadSettings();
		ZBufferMode=settings.Emulation.ZBufferMode;
		bool FZB= SUCCEEDED(d3d9->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_REF,D3DFMT_X8B8G8R8,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,D3DFMT_D24FS8));
		if (FZB)
			printf("Device Supports D24FS8\n");

		if (ZBufferMode==0 && !FZB)
		{
			printf("Cant use D24FS8, falling back to D24S8+FPE\n");
			ZBufferMode=1;
		}

		D3DCAPS9 caps;
		d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,&caps);

		printf("Device caps... VS : %X ; PS : %X\n",caps.VertexShaderVersion,caps.PixelShaderVersion);

		if (caps.VertexShaderVersion<D3DVS_VERSION(1, 0) || FORCE_SW_VERTEX_SHADERS)
		{
			UseSVP=true;
		}
		if (caps.PixelShaderVersion<D3DPS_VERSION(2, 0)|| FORCE_FIXED_FUNCTION)
		{
			UseFixedFunction=true;
		}
		else
		{
			UseFixedFunction=false;
		}

		printf("Will use %s\n",ZBufferModeName[ZBufferMode]);
		printf(UseSVP?"Will use SVP\n":"Will use Vertex Shaders\n");

		if (UseFixedFunction)
		{
			if (settings.Emulation.PaletteMode>1)
			{
				printf("Palette Mode that needs pixel shaders is selected, but no shaders are avaialbe\nReverting to VPT mode\n");
				settings.Emulation.PaletteMode=1;
			}
			if (ZBufferMode==1)
			{
				ZBufferMode=FZB?0:2;
				printf("Fixed function does not support %s, switching to %s\n",ZBufferModeName[1],ZBufferModeName[ZBufferMode]);
			}
		}

		ppar.SwapEffect = D3DSWAPEFFECT_DISCARD;
		ppar.PresentationInterval=settings.Video.VSync?D3DPRESENT_INTERVAL_ONE:D3DPRESENT_INTERVAL_IMMEDIATE;
		ppar.Windowed =   TRUE;

		ppar.EnableAutoDepthStencil=TRUE;
		ppar.AutoDepthStencilFormat = ZBufferMode==0 ? D3DFMT_D24FS8 : D3DFMT_D24S8;

		ppar.MultiSampleType = (D3DMULTISAMPLE_TYPE)settings.Enhancements.MultiSampleCount;
		ppar.MultiSampleQuality = settings.Enhancements.MultiSampleQuality;

		printf("drkpvr: Initialising windowed ");
		printf(" AA:%dx%x\n",ppar.MultiSampleType,ppar.MultiSampleQuality);

		//ppar.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		
		if (UseSVP || FAILED(d3d9->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,(HWND)emu.GetRenderTarget(),/*D3DCREATE_MULTITHREADED|*/
			D3DCREATE_HARDWARE_VERTEXPROCESSING,&ppar,&dev)))
		{
			if (!UseSVP)
				printf("We had to use SVP after all ...");

			UseSVP=true;
			verifyc(d3d9->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,(HWND)emu.GetRenderTarget(),/*D3DCREATE_MULTITHREADED|*/
				D3DCREATE_SOFTWARE_VERTEXPROCESSING,&ppar,&dev));
		}

		Medidate_fb();

		LPCSTR vsp= D3DXGetVertexShaderProfile(dev);
		if (vsp==0)
		{
			vsp="vs_3_0";
			printf("Strange , D3DXGetVertexShaderProfile(dev) failed , defaulting to \"vs_3_0\"\n");
		}

		if (UseSVP)
			vsp="vs_3_sw";

		printf(UseSVP?"Using SVP/%s\n":"Using Vertex Shaders/%s\n",vsp);
		printf(UseFixedFunction?"Using Fixed Function\n":"Using Pixel Shaders/%s\n",D3DXGetPixelShaderProfile(dev));
		
		vs_macros[0].Definition=ps_macro_numers[ZBufferMode];
		vs_macros[1].Definition=ps_macro_numers[UseFixedFunction?1:0];

		//yay , 20 mb -_- =P
		verifyc(dev->CreateVertexBuffer(20*1024*1024,D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | (UseSVP?D3DUSAGE_SOFTWAREPROCESSING:0),0,D3DPOOL_DEFAULT,&vb,0));
		
		verifyc(dev->CreateVertexDeclaration(vertelem,&vdecl));
		verifyc(dev->CreateVertexDeclaration(vertelem_mv,&vdecl_mod));
		verifyc(dev->CreateVertexDeclaration(vertelem_osd,&vdecl_osd));
		

		compiled_vs=CompileVS("vs_hlsl.fx","VertexShader_main",vsp,vs_macros);

		Composition.vs=CompileVS("composition.fx","VertexMain",vsp,0);

		verifyc(dev->CreateTexture(640,480,1,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&fb_texture8888,0));
		verifyc(fb_texture8888->GetSurfaceLevel(0,&fb_surface8888));

		verifyc(dev->CreateTexture(640,480,1,D3DUSAGE_DYNAMIC,D3DFMT_R5G6B5,D3DPOOL_DEFAULT,&fb_texture565,0));
		verifyc(fb_texture565->GetSurfaceLevel(0,&fb_surface565));

		verifyc(dev->CreateTexture(640,480,1,D3DUSAGE_DYNAMIC,D3DFMT_A1R5G5B5,D3DPOOL_DEFAULT,&fb_texture1555,0));
		verifyc(fb_texture1555->GetSurfaceLevel(0,&fb_surface1555));

		if (!UseFixedFunction)
		{
			verifyc(dev->CreateTexture(16,64,1,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&pal_texture,0));
			verifyc(dev->CreateTexture(128,1,1,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&fog_texture,0));
			LARGE_INTEGER ps_compile_start,ps_compile_end;
			QueryPerformanceCounter(&ps_compile_start);
			PrecompilePS();
			QueryPerformanceCounter(&ps_compile_end);
			
			printf("Compiling and loading shaders took %.2f ms\n",(ps_compile_end.QuadPart-ps_compile_start.QuadPart)/(freq.QuadPart/1000.0));

#if MODVOL
			ShadeColPixelShader=CompilePS("ps_hlsl.fx","PixelShader_ShadeCol",ps_macros);
			ZPixelShader=CompilePS("ps_hlsl.fx","PixelShader_Z",ps_macros);
#endif
			Composition.ps_DrawA=CompilePS("composition.fx","ps_DrawA",0);
			Composition.ps_DrawFB=CompilePS("composition.fx","ps_DrawFB",0);
			Composition.ps_DrawRGB=CompilePS("composition.fx","ps_DrawRGB",0);
			Composition.ps_DrawRGBA=CompilePS("composition.fx","ps_DrawRGBA",0);
		}

		D3DXCreateFont( dev, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, 
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &font );

		/*
			Reset Render stuff here
		*/
		clear_rt=5;
		rtt_address=-1;
		rtt_FrameNumber=0;
		d3d_init_done=true;

		QueryPerformanceCounter(&InitEnd);
			
		printf("Initialising 3D Renderer took %.2f ms\n",(InitEnd.QuadPart-InitStart.QuadPart)/(freq.QuadPart/1000.0));
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
		while(1)
		{
			rs.Wait();
			EnterCriticalSection(&d3d_lock);
			if (!running)
				break;
			HRESULT hr;
			hr=dev->TestCooperativeLevel();
			if (FAILED(hr) )
			{
				goto nl;
			}
			//render
			DoRender();
			//if (d3d_do_resize)
			//{
			//	verifyc(dev->Reset(&ppar));
			//	Medidate_fb();
			//	d3d_do_resize=false;
			//}
nl:
			re.Set();
			LeaveCriticalSection(&d3d_lock);
		}
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);

		//*NOTE* we still have the critical section in here ..
		d3d_init_done=false;
		LeaveCriticalSection(&d3d_lock);
		
		// IntelliSense complained... now it should SEE the definition.
		#define safe_release(d) {if (d) {(d->Release()==0);d=0;}}
		
		safe_release(Composition.vs);
		safe_release(Composition.ps_DrawFB);
		safe_release(Composition.ps_DrawA);
		safe_release(Composition.ps_DrawRGB);
		safe_release(Composition.ps_DrawRGBA);

		safe_release(vb);
		safe_release(compiled_vs);

		safe_release(vdecl);
		safe_release(vdecl_mod);
		safe_release(vdecl_osd);
		

		for(int i=0;i<PS_SHADER_COUNT;i++)
			safe_release(compiled_ps[i]);

		safe_release(ShadeColPixelShader);
		safe_release(ZPixelShader);
		
		safe_release(fb_surf);
		safe_release(backbuffer);
		safe_release(rtt_surf[0]);
		safe_release(rtt_surf[1]);
		safe_release(fb_surface8888);
		safe_release(fb_surface1555);
		safe_release(fb_surface565);

		safe_release(fb_texture8888);
		safe_release(fb_texture1555);
		safe_release(fb_texture565);
		safe_release(pal_texture);
		safe_release(fog_texture);
		safe_release(rtt_texture[0]);
		safe_release(rtt_texture[1]);
		safe_release(fb_texture);

		
		safe_release(font);
		safe_release(shader_consts);

		//kill texture cache
		TexCacheList<TextureCacheData>::TexCacheEntry* ptext= TexCache.plast;
		while(ptext)
		{
			ptext->data.Destroy();
			ptext->data.Texture->Release();
			TexCacheList<TextureCacheData>::TexCacheEntry* pprev;
			pprev=ptext->prev;
			TexCache.Remove(ptext);
			//free it !
			delete ptext;
			ptext=pprev;
		}

		safe_release2(dev);
		safe_release2(d3d9);


		#undef safe_release

		return 0;
	}

	u32 THREADCALL RenderThead(void* param)
	{
		do
		{
			RenderThead_internal(param);
			if (d3d_do_restart)
				running=true;
		} while(d3d_do_restart);
		return 0;
	}
	cThread rth(RenderThead,0);


	union _ISP_BACKGND_T_type
	{
		struct
		{
			u32 tag_offset:3;
			u32 tag_address:21;
			u32 skip:3;
			u32 shadow:1;
			u32 cache_bypass:1;
		};
		u32 full;
	};
	union _ISP_BACKGND_D_type
	{
		u32 i;
		f32 f;
	};

	//functions to read data :p
	f32 vrf(u32 addr)
	{
		return *(f32*)&params.vram[vramlock_ConvOffset32toOffset64(addr)];
	}
	u32 vri(u32 addr)
	{
		return *(u32*)&params.vram[vramlock_ConvOffset32toOffset64(addr)];
	}
	void decode_pvr_vertex(u32 base,u32 ptr,Vertex* to);
	int old_pal_mode;
	void StartRender()
	{
		SetCurrentPVRRC(PARAM_BASE);
		VertexCount+= pvrrc.verts.used;
		render_end_pending_cycles= pvrrc.verts.used*45;
		//if (render_end_pending_cycles<500000)
			render_end_pending_cycles+=500000;

		if (!d3d_init_done)
		{
			RenderWasStarted=false;
			//printf("Render didnt start ..\n");
			return;
		}

		if (old_pal_mode!=settings.Emulation.PaletteMode)
		{
			//mark pal texures dirty
			TexCacheList<TextureCacheData>::TexCacheEntry* ptext= TexCache.plast;
			while(ptext)
			{
				if ((ptext->data.tcw.PAL.PixelFmt == 5) || (ptext->data.tcw.PAL.PixelFmt == 6))
				{
					ptext->data.dirty=true;
					//Force it to recreate the texture
					if (ptext->data.Texture!=0)
					{
						ptext->data.Texture->Release();
						ptext->data.Texture=0;
					}
				}
				ptext=ptext->prev;
			}
			old_pal_mode=settings.Emulation.PaletteMode;
		}

		//--BG poly
		u32 param_base=PARAM_BASE & 0xF00000;
		_ISP_BACKGND_D_type bg_d; 
		_ISP_BACKGND_T_type bg_t;

		bg_d.i=ISP_BACKGND_D & ~(0xF);
		bg_t.full=ISP_BACKGND_T;
		
		PolyParam* bgpp=&pvrrc.global_param_op.data[0];
		Vertex* cv=BGPoly;

		bool PSVM=(FPU_SHAD_SCALE&0x100)!=0; //double parameters for volumes

		//Get the strip base
		u32 strip_base=(param_base + bg_t.tag_address*4)&0x7FFFFF;	//this is *not* VRAM_MASK on purpose.It fixes naomi bios and quite a few naomi games
																	//i have *no* idea why that happens, they manage to set the render target over there as well
																	//and that area is *not* writen by the games (they instead write the params on 000000 instead of 800000)
																	//could be a h/w bug ? param_base is 400000 and tag is 100000*4
		//Calculate the vertex size
		u32 strip_vs=3 + bg_t.skip;
		u32 strip_vert_num=bg_t.tag_offset;

		if (PSVM && bg_t.shadow)
		{
			strip_vs+=bg_t.skip;//2x the size needed :p
		}
		strip_vs*=4;
		//Get vertex ptr
		u32 vertex_ptr=strip_vert_num*strip_vs+strip_base +3*4;
		//now , all the info is ready :p

		bgpp->isp.full=vri(strip_base);
		bgpp->tsp.full=vri(strip_base+4);
		bgpp->tcw.full=vri(strip_base+8);
		bgpp->count=4;
		bgpp->first=0;
		bgpp->tileclip=0;//disabled ! HA ~

		bgpp->isp.DepthMode=7;// -> this makes things AWFULLY slow .. sometimes
		bgpp->isp.CullMode=0;// -> so that its not culled, or somehow else hiden !
		bgpp->tsp.FogCtrl=2;
		//Set some pcw bits .. i should realy get rid of pcw ..
		bgpp->pcw.UV_16bit=bgpp->isp.UV_16b;
		bgpp->pcw.Gouraud=bgpp->isp.Gouraud;
		bgpp->pcw.Offset=bgpp->isp.Offset;
		bgpp->pcw.Texture=bgpp->isp.Texture;

		float scale_x= (SCALER_CTL.hscale) ? 2.f:1.f;	//if AA hack the hacked pos value hacks
		for (int i=0;i<3;i++)
		{
			decode_pvr_vertex(strip_base,vertex_ptr,&cv[i]);
			vertex_ptr+=strip_vs;
		}

		float ZV=0;

		cv[0].x=0;
		cv[0].y=0;
		cv[0].z=bg_d.f;

		cv[1].x=640*scale_x;
		cv[1].y=0;
		cv[1].z=bg_d.f;

		cv[2].x=0;
		cv[2].y=480;
		cv[2].z=bg_d.f;

		cv[3]=cv[2];
		cv[3].x=640*scale_x;
		cv[3].y=480;
		cv[3].z=bg_d.f;
		
		//this is really suboptimal for precition .. but whatever works, right ?
		//(i don't really keep track of min, i just use bg_d)
		//if (pvrrc.invW_min<bg_d.f)
		pvrrc.invW_min=bg_d.f;
		if (pvrrc.invW_max<bg_d.f)
			pvrrc.invW_max=bg_d.f;

		RenderWasStarted=true;
		rs.Set();
		FrameCount++;
		
	}

	void Write32BitVram(u32 adr,u32 data)
	{
		*(u32*)&params.vram[vramlock_ConvOffset32toOffset64(adr)]=data;
	}
	void EndRender()
	{
		if (!RenderWasStarted)
		{
			//printf("Render was not started ..\n");
			return;
		}
		re.Wait();
		if (!(FB_W_SOF1 & 0x1000000))
		{
			Write32BitVram(FB_W_SOF1,0xDEADC0DE);
			Write32BitVram(FB_W_SOF1+0x280,0xDEADC0DE);
			Write32BitVram(FB_W_SOF1+0x500,0xDEADC0DE);
			Write32BitVram(FB_W_SOF1+0x780,0xDEADC0DE);
			Write32BitVram(FB_W_SOF1+0xA00,0xDEADC0DE);
			Write32BitVram(FB_W_SOF1+0x1400,0xDEADC0DE);
			
			Write32BitVram(FB_W_SOF2,0xDEADC0DE);
			Write32BitVram(FB_W_SOF2+0x280,0xDEADC0DE);
			Write32BitVram(FB_W_SOF2+0x500,0xDEADC0DE);
			Write32BitVram(FB_W_SOF2+0x780,0xDEADC0DE);
			Write32BitVram(FB_W_SOF2+0xA00,0xDEADC0DE);
			Write32BitVram(FB_W_SOF2+0x1400,0xDEADC0DE);
		}
		/*
		if (FB_W_SOF1 & 0x1000000)
		{
			D3DLOCKED_RECT lr;
			HRESULT rv = sysmems->LockRect(&lr,NULL,D3DLOCK_READONLY);
			u32* pixel=(u32*)lr.pBits;
			u32 stride=lr.Pitch/4;

			pixel+=stride*FB_Y_CLIP.min;
			u32 dest=FB_W_SOF1&VRAM_MASK;
			u32 pvr_stride=(FB_W_LINESTRIDE&0xFF)*8;
			for (u32 y=FB_Y_CLIP.min;y<FB_Y_CLIP.max;y++)
			{
				u32 cp=dest;
				for (u32 x=FB_X_CLIP.min;x<FB_X_CLIP.max;x++)
				{
					params.vram[cp]  =pixel[x];
					params.vram[cp+1]=pixel[x]>>16;
					cp+=2;
				}
				pixel+=stride;
				dest+=pvr_stride;
			}

			sysmems->UnlockRect();
		}
*/
		

		for (size_t i=0;i<lock_list.size();i++)
		{
			TextureCacheData* tcd=lock_list[i];
			if (tcd->lock_block==0 && tcd->dirty==false)
				tcd->LockVram();
			
		}
		lock_list.clear();
		
		TexCacheList<TextureCacheData>::TexCacheEntry* ptext= TexCache.plast;
		while(ptext && ((FrameNumber-ptext->data.LastUsed)>60))
		{
			TexCacheList<TextureCacheData>::TexCacheEntry* pprev;
			pprev=ptext->prev;

			if (settings.Emulation.TexCacheMode==0 || ptext->data.dirty==true)
			{
				ptext->data.Destroy();
				ptext->data.Texture->Release();
				TexCache.Remove(ptext);
				//free it !
				delete ptext;
			}
			ptext=pprev;
		}

		int old_rev;
		static NDC_WINDOW_RECT nwr;
		bool do_resize;
		static s32 old_res_mode=-1;
		do
		{
			old_rev = resizerq.rev;
			bool same_res=memcmp(&nwr,&resizerq.new_size,sizeof(NDC_WINDOW_RECT))==0 && settings.Video.ResolutionMode==old_res_mode;
			memcpy(&nwr,(void*)&resizerq.new_size,sizeof(NDC_WINDOW_RECT));
			do_resize=resizerq.needs_resize && !same_res;

		} while(old_rev!=resizerq.rev);

		resizerq.needs_resize=false;

		//resize renderer
		if (do_resize)
		{
			
		}

		if (do_resize || render_restart)
		{
			//resize
			old_res_mode=settings.Video.ResolutionMode;
			d3d_do_resize=true;
			//Restart renderer
			render_restart=false;
			d3d_do_restart=true;
			running=false;
			d3d_init_done=false;
			rs.Set();
		}
	}

	__declspec(align(16)) static f32 FaceBaseColor[4];
	__declspec(align(16)) static f32 FaceOffsColor[4];
	__declspec(align(16)) static f32 SFaceBaseColor[4];
	__declspec(align(16)) static f32 SFaceOffsColor[4];

#ifdef MODVOL
	ModTriangle* lmr=0;
	//s32 lmr_count=0;
#endif
	u32 tileclip_val=0;
	struct VertexDecoder
	{

		__forceinline
		static void SetTileClip(u32 xmin,u32 ymin,u32 xmax,u32 ymax)
		{
			u32 rv=tileclip_val & 0xF0000000;
			rv|=xmin; //6 bits
			rv|=xmax<<6; //6 bits
			rv|=ymin<<12; //5 bits
			rv|=ymax<<17; //5 bits
			tileclip_val=rv;
		}
		__forceinline
		static void TileClipMode(u32 mode)
		{
			tileclip_val=(tileclip_val&(~0xF0000000)) | (mode<<28);
		}
		//list handling
		__forceinline
		static void StartList(u32 ListType)
		{
			if (ListType==ListType_Opaque)
				CurrentPPlist=&tarc.global_param_op;
			else if (ListType==ListType_Punch_Through)
				CurrentPPlist=&tarc.global_param_pt;
			else if (ListType==ListType_Translucent)
				CurrentPPlist=&tarc.global_param_tr;
			
			CurrentPP=0;
			vert_reappend=0;
		}
		__forceinline
		static void EndList(u32 ListType)
		{
			vert_reappend=0;
			CurrentPP=0;
			CurrentPPlist=0;
			if (ListType==ListType_Opaque_Modifier_Volume)
			{
				ISP_Modvol p;
				p.id=tarc.modtrig.used;
				*tarc.global_param_mvo.Append()=p;
			}
		}

		/*
			if (CurrentPP==0 || CurrentPP->pcw.full!=pp->pcw.full || \
		CurrentPP->tcw.full!=pp->tcw.full || \
		CurrentPP->tsp.full!=pp->tsp.full || \
		CurrentPP->isp.full!=pp->isp.full	) \
		*/
		//Polys  -- update code on sprites if that gets updated too --
#define glob_param_bdc \
		{\
			PolyParam* d_pp =CurrentPPlist->Append(); \
			CurrentPP=d_pp;\
			d_pp->first=tarc.verts.used; \
			d_pp->count=0; \
			vert_reappend=0; \
			d_pp->isp=pp->isp; \
			d_pp->tsp=pp->tsp; \
			d_pp->tcw=pp->tcw; \
			d_pp->pcw=pp->pcw; \
			d_pp->tileclip=tileclip_val;\
		}

#define poly_float_color_(to,a,r,g,b) \
	to[0] = r;	\
	to[1] = g;	\
	to[2] = b;	\
	to[3] = a;

#define sat(x) (x<0?0:x>1?1:x)
#define poly_float_color(to,src) \
	poly_float_color_(to,sat(pp->src##A),sat(pp->src##R),sat(pp->src##G),sat(pp->src##B))

	//poly param handling
	__forceinline
		static void fastcall AppendPolyParam0(TA_PolyParam0* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam1(TA_PolyParam1* pp)
		{
			glob_param_bdc;
			poly_float_color(FaceBaseColor,FaceColor);
		}
		__forceinline
		static void fastcall AppendPolyParam2A(TA_PolyParam2A* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam2B(TA_PolyParam2B* pp)
		{
			poly_float_color(FaceBaseColor,FaceColor);
			poly_float_color(FaceOffsColor,FaceOffset);
		}
		__forceinline
		static void fastcall AppendPolyParam3(TA_PolyParam3* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam4A(TA_PolyParam4A* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam4B(TA_PolyParam4B* pp)
		{
			poly_float_color(FaceBaseColor,FaceColor0);
		}

		//Poly Strip handling
		//We unite Strips together by dupplicating the [last,first].On odd sized strips
		//a second [first] vert is needed to make sure Culling works fine :)
		__forceinline
		static void StartPolyStrip()
		{
			if (vert_reappend)
			{
				Vertex* old=((Vertex*)tarc.verts.ptr);
				if (CurrentPP->count&1)
				{
					Vertex* cv=tarc.verts.Guarantee(4,3);//4
					cv[1].x=cv[0].x=old[-1].x;
					cv[1].y=cv[0].y=old[-1].y;
					cv[1].z=cv[0].z=old[-1].z;
				}
				else
				{
					Vertex* cv=tarc.verts.Guarantee(3,2);//3
					cv[0].x=old[-1].x;//dup prev
					cv[0].y=old[-1].y;//dup prev
					cv[0].z=old[-1].z;//dup prev
				}
				vert_reappend=(Vertex*)tarc.verts.ptr;
			}
		}
		__forceinline
		static void EndPolyStrip()
		{
			if (vert_reappend)
			{
				Vertex* vert=vert_reappend;
				vert[-1].x=vert[0].x;
				vert[-1].y=vert[0].y;
				vert[-1].z=vert[0].z;
			}
			vert_reappend=(Vertex*)1;
			CurrentPP->count=tarc.verts.used - CurrentPP->first;
		}

		//Poly Vertex handlers
#ifdef scale_type_1
#define z_update(zv) \
	/*if (tarc.invW_min>zv)\
		tarc.invW_min=zv;*/\
	if (((u32&)zv)<0x41000000 && tarc.invW_max<zv)\
		tarc.invW_max=zv;
#else
	#define z_update(zv)
#endif

		//if ((*(u32*)&invW)==0x7F800000) return;\
	//Append vertex base
#define vert_cvt_base \
	f32 invW=vtx->xyz[2];\
	Vertex* cv=tarc.verts.Append();\
	cv->x=vtx->xyz[0];\
	cv->y=vtx->xyz[1];\
	cv->z=invW;\
	z_update(invW);

	//Resume vertex base (for B part)
#define vert_res_base \
	Vertex* cv=((Vertex*)tarc.verts.ptr)-1;

	//uv 16/32
#define vert_uv_32(u_name,v_name) \
		cv->u	=	(vtx->u_name);\
		cv->v	=	(vtx->v_name);

#define vert_uv_16(u_name,v_name) \
		cv->u	=	f16(vtx->u_name);\
		cv->v	=	f16(vtx->v_name);

	//Color convertions
#ifdef _float_colors_
	#define vert_packed_color_(to,src) \
	{ \
	u32 t=src; \
		to[2]	= unkpack_bgp_to_float[(u8)(t)];t>>=8;\
		to[1]	= unkpack_bgp_to_float[(u8)(t)];t>>=8;\
		to[0]	= unkpack_bgp_to_float[(u8)(t)];t>>=8;\
		to[3]	= unkpack_bgp_to_float[(u8)(t)];	\
	}

	#define vert_float_color_(to,a,r,g,b) \
			to[0] = r;	\
			to[1] = g;	\
			to[2] = b;	\
			to[3] = a;
#else
	#error OLNY floating color is supported for now
#endif

	//Macros to make thins easyer ;)
#define vert_packed_color(to,src) \
	vert_packed_color_(cv->to,vtx->src);

#define vert_float_color(to,src) \
	vert_float_color_(cv->to,vtx->src##A,vtx->src##R,vtx->src##G,vtx->src##B)

	//Intesity handling
#ifdef _HW_INT_
	//Hardware intesinty handling , we just store the int value
	#define vert_int_base(base) \
		cv->base_int = vtx->base;

	#define vert_int_offs(offs) \
		cv->offset_int = vtx->offs;

	#define vert_int_no_base() \
		cv->base_int = 1;

	#define vert_int_no_offs() \
		cv->offset_int = 1;

	#define vert_face_base_color(baseint) \
		vert_float_color_(cv->col,FaceBaseColor[3],FaceBaseColor[0],FaceBaseColor[1],FaceBaseColor[2]);	 \
		vert_int_base(baseint);

	#define vert_face_offs_color(offsint) \
		vert_float_color_(cv->spc,FaceOffsColor[3],FaceOffsColor[0],FaceOffsColor[1],FaceOffsColor[2]);	 \
		vert_int_offs(offsint);
#else
	//Notes:
	//Alpha doesn't get intensity
	//Intesity is clamped before the mul, as well as on face color to work the same as the hardware. [Fixes red dog]

	//Notes:
	//Alpha doesn't get intensity
	//Intesity is clamped before the mul, as well as on face color to work the same as the hardware. [Fixes red dog]

	#define vert_face_base_color(baseint) \
		{ float satint=sat(vtx->baseint); \
		vert_float_color_(cv->col,FaceBaseColor[3],FaceBaseColor[0]*satint,FaceBaseColor[1]*satint,FaceBaseColor[2]*satint); }

	#define vert_face_offs_color(offsint) \
		{ float satint=sat(vtx->offsint); \
		vert_float_color_(cv->spc,FaceOffsColor[3],FaceOffsColor[0]*satint,FaceOffsColor[1]*satint,FaceOffsColor[2]*satint); }

	#define vert_int_no_base()
	#define vert_int_no_offs()
#endif



		//(Non-Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol);

			vert_int_no_base();
		}

		//(Non-Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
		{
			vert_cvt_base;

			vert_float_color(col,Base);

			vert_int_no_base();
		}

		//(Non-Textured, Intensity)
		__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
		{
			vert_cvt_base;
			
			vert_face_base_color(BaseInt);
		}

		//(Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
		{
			vert_cvt_base;
			
			vert_packed_color(col,BaseCol);
			vert_packed_color(spc,OffsCol);

			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_32(u,v);
		}

		//(Textured, Packed Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol);
			vert_packed_color(spc,OffsCol);

			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_16(u,v);
		}

		//(Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
		{
			vert_cvt_base;

			//Colors are on B
			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_32(u,v);
		}
		__forceinline
		static void AppendPolyVertex5B(TA_Vertex5B* vtx)
		{
			vert_res_base;

			vert_float_color(col,Base);
			vert_float_color(spc,Offs);
		}

		//(Textured, Floating Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex6A(TA_Vertex6A* vtx)
		{
			vert_cvt_base;

			//Colors are on B
			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_16(u,v);
		}
		__forceinline
		static void AppendPolyVertex6B(TA_Vertex6B* vtx)
		{
			vert_res_base;

			vert_float_color(col,Base);
			vert_float_color(spc,Offs);
		}

		//(Textured, Intensity)
		__forceinline
		static void AppendPolyVertex7(TA_Vertex7* vtx)
		{
			vert_cvt_base;

			vert_face_base_color(BaseInt);
			vert_face_offs_color(OffsInt);

			vert_uv_32(u,v);
		}

		//(Textured, Intensity, 16bit UV)
		__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
		{
			vert_cvt_base;

			vert_face_base_color(BaseInt);
			vert_face_offs_color(OffsInt);

			vert_uv_16(u,v);
			
		}

		//(Non-Textured, Packed Color, with Two Volumes)
		__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol0);

			vert_int_no_base();
		}

		//(Non-Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
		{
			vert_cvt_base;
			
			vert_face_base_color(BaseInt0);
		}

		//(Textured, Packed Color,	with Two Volumes)	
		__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol0);
			vert_packed_color(spc,OffsCol0);

			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_32(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex11B(TA_Vertex11B* vtx)
		{
			vert_res_base;

		}

		//(Textured, Packed Color, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex12A(TA_Vertex12A* vtx)
		{
			vert_cvt_base;

			vert_packed_color(col,BaseCol0);
			vert_packed_color(spc,OffsCol0);

			vert_int_no_base();
			vert_int_no_offs();

			vert_uv_16(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex12B(TA_Vertex12B* vtx)
		{
			vert_res_base;

		}

		//(Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex13A(TA_Vertex13A* vtx)
		{
			vert_cvt_base;

			vert_face_base_color(BaseInt0);
			vert_face_offs_color(OffsInt0);

			vert_uv_32(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex13B(TA_Vertex13B* vtx)
		{
			vert_res_base;

		}

		//(Textured, Intensity, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex14A(TA_Vertex14A* vtx)
		{
			vert_cvt_base;

			vert_face_base_color(BaseInt0);
			vert_face_offs_color(OffsInt0);

			vert_uv_16(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex14B(TA_Vertex14B* vtx)
		{
			vert_res_base;

		}

		//Sprites
		__forceinline
		static void AppendSpriteParam(TA_SpriteParam* spr)
		{
			//printf("Sprite\n");
			PolyParam* d_pp =CurrentPPlist->Append(); 
			CurrentPP=d_pp;
			d_pp->first=tarc.verts.used; 
			d_pp->count=0; 
			vert_reappend=0; 
			d_pp->isp=spr->isp; 
			d_pp->tsp=spr->tsp; 
			d_pp->tcw=spr->tcw; 
			d_pp->pcw=spr->pcw; 
			d_pp->tileclip=tileclip_val;

			vert_packed_color_(SFaceBaseColor,spr->BaseCol);
			vert_packed_color_(SFaceOffsColor,spr->OffsCol);
		}

#define append_sprite(indx) \
	vert_float_color_(cv[indx].col,SFaceBaseColor[3],SFaceBaseColor[0],SFaceBaseColor[1],SFaceBaseColor[2])\
	vert_float_color_(cv[indx].spc,SFaceOffsColor[3],SFaceOffsColor[0],SFaceOffsColor[1],SFaceOffsColor[2])
	//cv[indx].base_int=1;\
	//cv[indx].offset_int=1;

#define append_sprite_yz(indx,set,st2) \
	cv[indx].y=sv->y##set; \
	cv[indx].z=sv->z##st2; \
	z_update(sv->z##st2);

#define sprite_uv(indx,u_name,v_name) \
		cv[indx].u	=	f16(sv->u_name);\
		cv[indx].v	=	f16(sv->v_name);
		//Sprite Vertex Handlers
		__forceinline
		static void AppendSpriteVertexA(TA_Sprite1A* sv)
		{
			if (CurrentPP->count)
			{
				Vertex* old=((Vertex*)tarc.verts.ptr);
				Vertex* cv=tarc.verts.Guarantee(6,2);
				cv[0].x=old[-1].x;//dup prev
				cv[0].y=old[-1].y;//dup prev
				cv[0].z=old[-1].z;//dup prev
				vert_reappend=(Vertex*)tarc.verts.ptr;
			}

			Vertex* cv = tarc.verts.Append(4);
			
			//Fill static stuff
			append_sprite(0);
			append_sprite(1);
			append_sprite(2);
			append_sprite(3);

			cv[2].x=sv->x0;
			cv[2].y=sv->y0;
			cv[2].z=sv->z0;
			z_update(sv->z0);

			cv[3].x=sv->x1;
			cv[3].y=sv->y1;
			cv[3].z=sv->z1;
			z_update(sv->z1);

			cv[1].x=sv->x2;
		}
		static void CaclulateSpritePlane(Vertex* base)
		{
			const Vertex& A=base[2];
			const Vertex& B=base[3];
			const Vertex& C=base[1];
			      Vertex& P=base[0];
			//Vector AB = B-A;
            //Vector AC = C-A;
            //Vector AP = P-A;
			float AC_x=C.x-A.x,AC_y=C.y-A.y,AC_z=C.z-A.z,
				  AB_x=B.x-A.x,AB_y=B.y-A.y,AB_z=B.z-A.z,
				  AP_x=P.x-A.x,AP_y=P.y-A.y;

			float P_y=P.y,P_x=P.x,P_z=P.z,A_x=A.x,A_y=A.y,A_z=A.z;

			float AB_v=B.v-A.v,AB_u=B.u-A.u,
				  AC_v=C.v-A.v,AC_u=C.u-A.u;

			float /*P_v,P_u,*/A_v=A.v,A_u=A.u;

            float k3 = (AC_x * AB_y - AC_y * AB_x);
 
            if (k3 == 0)
            {
                //throw new Exception("WTF?!");
            }
 
            float k2 = (AP_x * AB_y - AP_y * AB_x) / k3;
 
            float k1 = 0;
 
            if (AB_x == 0)
            {
                //if (AB_y == 0)
				//	;
                //    //throw new Exception("WTF?!");
 
                k1 = (P_y - A_y - k2 * AC_y) / AB_y;
            }
            else
            {
                k1 = (P_x - A_x - k2 * AC_x) / AB_x;
            }
 
			P.z = A_z + k1 * AB_z + k2 * AC_z;
            P.u = A_u + k1 * AB_u + k2 * AC_u;
			P.v = A_v + k1 * AB_v + k2 * AC_v;
		}
		__forceinline
		static void AppendSpriteVertexB(TA_Sprite1B* sv)
		{
			vert_res_base;
			cv-=3;

			cv[1].y=sv->y2;
			cv[1].z=sv->z2;
			z_update(sv->z2);

			cv[0].x=sv->x3;
			cv[0].y=sv->y3;
			//cv[0].z=sv->z2; //temp , gota calc. 4th Z properly :p


			sprite_uv(2, u0,v0);
			sprite_uv(3, u1,v1);
			sprite_uv(1, u2,v2);
			//sprite_uv(0, u0,v2);//or sprite_uv(u2,v0); ?

			CaclulateSpritePlane(cv);

			z_update(cv[0].z);

			if (CurrentPP->count)
			{
				Vertex* vert=vert_reappend;
				vert[-1].x=vert[0].x;
				vert[-1].y=vert[0].y;
				vert[-1].z=vert[0].z;
				CurrentPP->count+=2;
			}
			
			CurrentPP->count+=4;
		}

		//ModVolumes

		//Mod Volume Vertex handlers
		static void StartModVol(TA_ModVolParam* param)
		{
			if (TileAccel.CurrentList!=ListType_Opaque_Modifier_Volume)
				return;
			ISP_Modvol p;
			p.full=param->isp.full;
			p.VolumeLast=param->pcw.Volume;
			p.id=tarc.modtrig.used;

			*tarc.global_param_mvo.Append()=p;
			/*
			printf("MOD VOL %d - 0x%08X 0x%08X 0x%08X \n",tarc.modtrig.used,param->pcw.Volume,param->isp.DepthMode,param->isp.CullMode);
			
			if (param->pcw.Volume || param->isp.DepthMode)
			{
				//if (lmr_count)
				//{
					*tarc.modsz.Append()=lmr_count+1;
					lmr_count=-1;
				//}
			}
			*/
		}
		__forceinline
		static void AppendModVolVertexA(TA_ModVolA* mvv)
		{
		#ifdef MODVOL
			if (TileAccel.CurrentList!=ListType_Opaque_Modifier_Volume)
				return;
			lmr=tarc.modtrig.Append();

			lmr->x0=mvv->x0;
			lmr->y0=mvv->y0;
			lmr->z0=mvv->z0;
			lmr->x1=mvv->x1;
			lmr->y1=mvv->y1;
			lmr->z1=mvv->z1;
			lmr->x2=mvv->x2;

			z_update(mvv->z1);
			z_update(mvv->z0);
			//lmr_count++;
		#endif	
		}
		__forceinline
		static void AppendModVolVertexB(TA_ModVolB* mvv)
		{
		#ifdef MODVOL
			if (TileAccel.CurrentList!=ListType_Opaque_Modifier_Volume)
				return;
			lmr->y2=mvv->y2;
			lmr->z2=mvv->z2;
			z_update(mvv->z2);
		#endif
		}

		//Misc
		__forceinline
		static void ListCont()
		{
			//printf("LC : TA OL base = 0x%X\n",TA_OL_BASE);
			SetCurrentTARC(TA_ISP_BASE);
		}
		__forceinline
		static void ListInit()
		{
			//printf("LI : TA OL base = 0x%X\n",TA_OL_BASE);
			SetCurrentTARC(TA_ISP_BASE);
			tarc.Clear();

			//allocate storage for BG poly
			tarc.global_param_op.Append();
			BGPoly=tarc.verts.Append(4);
		}
		__forceinline
		static void SoftReset()
		{
		}
	};

	//decode a vertex in the native pvr format
	//used for bg poly
	void decode_pvr_vertex(u32 base,u32 ptr,Vertex* cv)
	{
		//ISP
		//TSP
		//TCW
		ISP_TSP isp;
		TSP tsp;
		TCW tcw;

		isp.full=vri(base);
		tsp.full=vri(base+4);
		tcw.full=vri(base+8);

		//XYZ
		//UV
		//Base Col
		//Offset Col

		//XYZ are _allways_ there :)
		cv->x=vrf(ptr);ptr+=4;
		cv->y=vrf(ptr);ptr+=4;
		cv->z=vrf(ptr);ptr+=4;

		if (isp.Texture)
		{	//Do texture , if any
			if (isp.UV_16b)
			{
				u32 uv=vri(ptr);
				cv->u	=	f16((u16)uv);
				cv->v	=	f16((u16)(uv>>16));
				ptr+=4;
			}
			else
			{
				cv->u=vrf(ptr);ptr+=4;
				cv->v=vrf(ptr);ptr+=4;
			}
		}

		//Color
		u32 col=vri(ptr);ptr+=4;
		vert_packed_color_(cv->col,col);
		if (isp.Offset)
		{
			//Intesity color (can be missing too ;p)
			u32 col=vri(ptr);ptr+=4;
			vert_packed_color_(cv->spc,col);
		}
	}


	bool InitRenderer()
	{
		InitializeCriticalSection(&d3d_lock);
		for (u32 i=0;i<256;i++)
		{
			unkpack_bgp_to_float[i]=i/255.0f;
		}
		for (u32 i=0;i<rcnt.size();i++)
		{
			rcnt[i].Free();
		}
		rcnt.clear();
		tarc.Address=0xFFFFFFFF;
		tarc.Init();
		//pvrrc needs no init , it is ALLWAYS copied from a valid tarc :)

		return TileAccel.Init();
	}

	void TermRenderer()
	{
		DeleteCriticalSection(&d3d_lock); 
		for (u32 i=0;i<rcnt.size();i++)
		{
			rcnt[i].Free();
		}
		rcnt.clear();

		TileAccel.Term();
		//free all textures
		verify(TexCache.pfirst==0);
	}

	void ResetRenderer(bool Manual)
	{
		TileAccel.Reset(Manual);
		VertexCount=0;
		FrameCount=0;
	}

	bool ThreadStart()
	{
		running=true;
		rth.Start();
		return true;
	}

	void ThreadEnd()
	{
		printf("ThreadEnd\n");
		if (running)
		{
			verify(!rth.ended);
			running=false;
			//if (!rth.ended)
			{
				rs.Set();
				rth.WaitToEnd(0xFFFFFFFF);
			}
		}
	}

	

	void ListCont()
	{
		TileAccel.ListCont();
	}
	void ListInit()
	{
		TileAccel.ListInit();
	}
	void SoftReset()
	{
		TileAccel.SoftReset();
	}
#endif
