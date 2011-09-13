#define _WIN32_WINNT 0x0500
#include "drkpvr.h"
#if _DEBUG
#define D3D_DEBUG_INFO
#endif

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>

#include "nullRend.h"
#include <algorithm>
#include "d3dRend11.h"
#include "windows.h"
#include "regs.h"
#include <vector>
//#include <xmmintrin.h>

#if REND_API == REND_D3D11
#pragma comment(lib, "d3d11.lib") 
#pragma comment(lib, "d3dx11.lib") 

volatile bool render_restart;

#define MODVOL 1
#define _float_colors_
//#define _HW_INT_
//#include <D3dx9shader.h>

using namespace TASplitter;



D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11VertexShader*     g_pVertexShader = NULL;
ID3D11PixelShader*      g_pPixelShader = NULL;
ID3D11InputLayout*      g_pVertexLayout = NULL;
ID3D11Buffer*           g_pVertexBuffer = NULL;
ID3D11Texture2D*        g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;	
ID3D11DepthStencilState* g_pDepthStencilState;
ID3D11RasterizerState* g_pRasterizerState;


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

#define PS_SHADER_COUNT (384*4)

	f32 vrf(u32 addr)
	{
		return *(f32*)&params.vram[vramlock_ConvOffset32toOffset64(addr)];
	}
	u32 vri(u32 addr)
	{
		return *(u32*)&params.vram[vramlock_ConvOffset32toOffset64(addr)];
	}

	struct VertexDecoder;
	FifoSplitter<VertexDecoder> TileAccel;


	u32 FrameNumber=0;
	u32 fb_FrameNumber=0;


	//x=emulation mode
	//y=filter mode
	//result = {d3dmode,shader id}
	void HandleEvent(u32 evid,void* p)
	{
		
	}

	void SetRenderRect(float* rect,bool do_clear)
	{
		
	}
	void SetFBScale(float x,float y)
	{
		
	}

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

	void VramLockedWrite(vram_block* bl)
	{

	}

	u32 vri(u32 addr);

	void DrawOSD();
	void VBlank()
	{
		FrameNumber++;

		//todo: FB emulation

		// Present the information rendered to the back buffer to the front buffer (the screen)
		g_pSwapChain->Present( 0, 0 );
	}

	//Vertex storage types
	//64B
	struct Vertex
	{
		//64
		float x,y,z;

		float col[4];
		float spc[4];

		float u,v;
	};
	Vertex* BGPoly;
	
	struct PolyParam
	{
		u32 first;		//entry index , holds vertex/pos data
		u32 count;

		//lets see what more :)
		
		TSP tsp;
		TCW tcw;
		PCW pcw;
		ISP_TSP isp;
		u32 tileclip;
	};

		struct PolyParamAA
	{
		u32 first;		//entry index , holds vertex/pos data
		u32 count;

		//lets see what more :)
		
		TSP tsp;
		TCW tcw;
		PCW pcw;
		ISP_TSP isp;
		u32 tileclip;
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
	
	void DrawOSD()
	{
		
	}

	template <u32 Type>
	__forceinline
	void RendStrips(PolyParam* gp)
	{
			if (gp->count>2)//we need 2+ vertexes for a valid polygon
			{		
				g_pImmediateContext->Draw(gp->count,gp->first);
			}
	}

	template <u32 Type>
	void RendPolyParamList(List<PolyParam>& gpl)
	{
		if (gpl.used==0)
			return;
		//we want at least 1 PParam

		for (u32 i=0;i<gpl.used;i++)
		{		
			RendStrips<Type>(&gpl.data[i]);
		}
	}
	
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
		//vert_packed_color_(cv->col,col);
		if (isp.Offset)
		{
			//Intesity color (can be missing too ;p)
			u32 col=vri(ptr);ptr+=4;
			//vert_packed_color_(cv->spc,col);
		}
	}

	void DoRender()
	{
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

		{
			pvrrc.verts.Finalise();
			u32 sz=pvrrc.verts.used*sizeof(Vertex);

			D3D11_MAPPED_SUBRESOURCE oMappedRes;
			g_pImmediateContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMappedRes);

			pvrrc.verts.Copy(oMappedRes.pData,sz);

			g_pImmediateContext->Unmap(g_pVertexBuffer,0);
		}

		// Clear the back buffer 
		float ClearColor[4] = { 0.0f, 0.125f, rand()/(float)RAND_MAX, 1.0f }; // red,green,blue,alpha
		g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

		g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 0, 0 );

		g_pImmediateContext->RSSetState(g_pRasterizerState);
		g_pImmediateContext->OMSetDepthStencilState(g_pDepthStencilState,0);
		// Render a triangle
		// Render a triangle
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


		RendPolyParamList<ListType_Opaque>(pvrrc.global_param_op);

		RendPolyParamList<ListType_Punch_Through>(pvrrc.global_param_pt);

		RendPolyParamList<ListType_Translucent>(pvrrc.global_param_tr);
	}
	
	void ListModes(void(* callback)(u32 w,u32 h,u32 rr))
	{
	
	}

	void StartRender()
	{
		SetCurrentPVRRC(PARAM_BASE);
		VertexCount+= pvrrc.verts.used;
		
		render_end_pending_cycles=500000;
		render_end_pending_cycles+=pvrrc.verts.used*25;
		
		

		DoRender();
	}

	void EndRender()
	{
		
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

	//--------------------------------------------------------------------------------------
	// Helper for compiling shaders with D3DX11
	//--------------------------------------------------------------------------------------
	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* pErrorBlob;
		hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
			dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
		if( FAILED(hr) )
		{
			if( pErrorBlob != NULL )
				puts( (char*)pErrorBlob->GetBufferPointer() );
			if( pErrorBlob ) pErrorBlob->Release();
			return hr;
		}
		if( pErrorBlob ) pErrorBlob->Release();

		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	// Create Direct3D device and swap chain
	//--------------------------------------------------------------------------------------
	HRESULT InitDevice()
	{
		HRESULT hr = S_OK;

		RECT rc;
		GetClientRect( (HWND)emu.GetRenderTarget(), &rc );
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;

		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE( driverTypes );

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE( featureLevels );

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory( &sd, sizeof( sd ) );
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = (HWND)emu.GetRenderTarget();
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
		{
			g_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
			if( SUCCEEDED( hr ) )
				break;
		}
		if( FAILED( hr ) )
			return hr;

		// Create a render target view
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
		if( FAILED( hr ) )
			return hr;

		hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
		pBackBuffer->Release();
		if( FAILED( hr ) )
			return hr;

		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory( &descDepth, sizeof(descDepth) );
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
		if( FAILED( hr ) )
			return hr;

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory( &descDSV, sizeof(descDSV) );
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
		if( FAILED( hr ) )
			return hr;

		g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );


		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		g_pImmediateContext->RSSetViewports( 1, &vp );

		// Compile the vertex shader
		ID3DBlob* pVSBlob = NULL;
		hr = CompileShaderFromFile( L"d3d11.fx", "VS", "vs_4_0", &pVSBlob );
		if( FAILED( hr ) )
		{
			MessageBox( NULL,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
			return hr;
		}

		// Create the vertex shader
		hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
		if( FAILED( hr ) )
		{	
			pVSBlob->Release();
			return hr;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE( layout );

		// Create the input layout
		hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(), &g_pVertexLayout );
		pVSBlob->Release();
		if( FAILED( hr ) )
			return hr;

		// Set the input layout
		g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

		
		// Compile the pixel shader
		ID3DBlob* pPSBlob = NULL;
		hr = CompileShaderFromFile( L"Tutorial03.fx", "PS", "ps_4_0", &pPSBlob );
		if( FAILED( hr ) )
		{
			MessageBox( NULL,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
			return hr;
		}

		// Create the pixel shader
		hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
		pPSBlob->Release();
		if( FAILED( hr ) )
			return hr;
		
		// Create vertex buffer
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof( Vertex)* 1024 * 1024 ;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE ;
		hr = g_pd3dDevice->CreateBuffer( &bd, 0, &g_pVertexBuffer );
		if( FAILED( hr ) )
			return hr;


		// Set vertex buffer
		UINT stride = sizeof( Vertex );
		UINT offset = 0;
		g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

		// Set primitive topology
		g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		D3D11_DEPTH_STENCIL_DESC dsd;
		dsd.DepthEnable=true;
		dsd.DepthFunc=D3D11_COMPARISON_GREATER_EQUAL;
		dsd.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;
		dsd.StencilEnable=false;
		g_pd3dDevice->CreateDepthStencilState( &dsd, &g_pDepthStencilState);


		D3D11_RASTERIZER_DESC rastDesc;
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FrontCounterClockwise = true;
		rastDesc.DepthBias = false;
		rastDesc.DepthBiasClamp = 0;
		rastDesc.SlopeScaledDepthBias = 0;
		rastDesc.DepthClipEnable = false;
		rastDesc.ScissorEnable = false;
		rastDesc.MultisampleEnable = false;
		rastDesc.AntialiasedLineEnable = false;

		g_pd3dDevice->CreateRasterizerState(&rastDesc, &g_pRasterizerState);

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Clean up the objects we've created
	//--------------------------------------------------------------------------------------
	void CleanupDevice()
	{
		if( g_pImmediateContext ) g_pImmediateContext->ClearState();

		if( g_pVertexBuffer ) g_pVertexBuffer->Release();
		if( g_pVertexLayout ) g_pVertexLayout->Release();
		if( g_pVertexShader ) g_pVertexShader->Release();
		if( g_pPixelShader ) g_pPixelShader->Release();
		if( g_pDepthStencil ) g_pDepthStencil->Release();
		if( g_pDepthStencilView ) g_pDepthStencilView->Release();
		if( g_pRenderTargetView ) g_pRenderTargetView->Release();
		if( g_pSwapChain ) g_pSwapChain->Release();
		if( g_pImmediateContext ) g_pImmediateContext->Release();
		if( g_pd3dDevice ) g_pd3dDevice->Release();
		if (g_pDepthStencilState) g_pDepthStencilState->Release();
	}





	bool InitRenderer()
	{
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

		InitDevice();

		return TileAccel.Init();
	}

	void TermRenderer()
	{
		for (u32 i=0;i<rcnt.size();i++)
		{
			rcnt[i].Free();
		}
		rcnt.clear();

		CleanupDevice();

		TileAccel.Term();
	}

	void ResetRenderer(bool Manual)
	{
		TileAccel.Reset(Manual);
		VertexCount=0;
		FrameCount=0;
	}

	bool ThreadStart()
	{

		return true;
	}

	void ThreadEnd()
	{
		
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
