/*
	Original drkpvr code was based on opengl
	this hasn't been worked on since 2005(?) or so .. most likely won't compile ~
*/
#include "oglRend.h"


#if REND_API == REND_OGL

#include <windows.h>

#include "glew.h"
#include "wglew.h"

#include <gl\gl.h>
#include "regs.h"
#include "vector"

using namespace std;
#pragma comment(lib, "opengl32.lib") 
#pragma comment(lib, "glu32.lib") 
#pragma comment(lib, "glaux.lib") 
#pragma comment(lib, "glew32-drkpvr.lib") 


#include <gl\glaux.h>
using namespace TASplitter;

namespace OpenGLRenderer
{
	void SetFpsText(wchar* text)
	{
		SetWindowText((HWND)emu.GetRenderTarget(), text);
	}

	u32 texFormat[8]=
	{
		GL_RGB5_A1 ,//0	1555 value: 1 bit; RGB values: 5 bits each
		GL_RGB5    ,//1	565	 R value: 5 bits; G value: 6 bits; B value: 5 bits
		GL_RGBA4   ,//2	4444 value: 4 bits; RGB values: 4 bits each
		GL_RGB5    ,//3	YUV422 32 bits per 2 pixels; YUYV values: 8 bits each
		GL_RGBA	   ,//4	Bump Map	16 bits/pixel; S value: 8 bits; R value: 8 bits
		GL_RGBA	   ,//5	4 BPP Palette	Palette texture with 4 bits/pixel
		GL_RGBA		,//6	8 BPP Palette	Palette texture with 8 bits/pixel
		GL_RGB5_A1//7	Reserved	Regarded as 1555
	};

	char texFormatName[8][30]=
	{
		"1555",
		"565",
		"4444",
		"YUV422",
		"Bump Map",
		"8 BPP Palette",
		"8 BPP Palette",
		"Reserved	, 1555"
	};
	u32 temp_tex_buffer[1024*1024*4];

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
						argb##format##to8888_VQ(&pbt,(u8*)&params.vram[sa],w,h);\
					}\
					else\
					{\
						if (tcw.NO_PAL.MipMapped)\
							sa+=MipPoint[tsp.TexU]<<3;\
						argb##format##to8888_TW(&pbt,(u8*)&params.vram[sa],w,h);\
					}

	//Texture Cache :)
	struct TextureCacheData
	{
		u32 Start;
		GLuint texID;
		u32 Lookups;
		u32 Updates;

		TSP tsp;TCW tcw;

		u32 w,h;
		u32 size;
		bool dirty;
		vram_block* lock_block;

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

			printf(" %dx%d @ 0x%X",8<<tsp.TexU,8<<tsp.TexV,tcw.NO_PAL.TexAddr<<3);
			printf("\n");
		}
		void Update()
		{
			verify(dirty);
			verify(lock_block==0);

			Updates++;
			dirty=false;
			glBindTexture(GL_TEXTURE_2D, texID);

			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			u32 sa=Start;

			PixelBuffer pbt;
			pbt.p_buffer_start=pbt.p_current_line=temp_tex_buffer;
			pbt.pixels_per_line=w;

			switch (tcw.NO_PAL.PixelFmt)
			{
			case 0:
			case 7:
				//0	1555 value: 1 bit; RGB values: 5 bits each
				//7	Reserved	Regarded as 1555
				if (tcw.NO_PAL.ScanOrder)
				{
					verify(tcw.NO_PAL.VQ_Comp==0);
					argb1555to8888(&pbt,(u8*)&params.vram[sa],w,h);
				}
				else
				{
					//verify(tsp.TexU==tsp.TexV);
					twidle_tex(1555);
				}
				break;

				//1	565	 R value: 5 bits; G value: 6 bits; B value: 5 bits
			case 1:
				if (tcw.NO_PAL.ScanOrder)
				{
					//verify(tcw.NO_PAL.VQ_Comp==0);
					argb565to8888(&pbt,(u8*)&params.vram[sa],w,h);
				}
				else
				{
					//verify(tsp.TexU==tsp.TexV);
					twidle_tex(565);
				}
				break;

				//2	4444 value: 4 bits; RGB values: 4 bits each
			case 2:
				if (tcw.NO_PAL.ScanOrder)
				{
					//verify(tcw.NO_PAL.VQ_Comp==0);
					argb4444to8888(&pbt,(u8*)&params.vram[sa],w,h);
				}
				else
				{
					twidle_tex(4444);
				}

				break;
				//3	YUV422 32 bits per 2 pixels; YUYV values: 8 bits each
				//4	Bump Map	16 bits/pixel; S value: 8 bits; R value: 8 bits
				//5	4 BPP Palette	Palette texture with 4 bits/pixel
				//6	8 BPP Palette	Palette texture with 8 bits/pixel
			default:
				printf("Unhandled texture\n");
				memset(temp_tex_buffer,0xFFFFFFFF,w*h*4);
			}

			//PrintTextureName();
			u32 ea=sa+w*h*2;
			if (ea>=(8*1024*1024))
			{
				ea=(8*1024*1024)-1;
			}
			//(u32 start_offset64,u32 end_offset64,void* userdata);
			lock_block = params.vram_lock_64(sa,ea,this);

			glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA8 , w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, temp_tex_buffer);

			//printf("Updated Texture @ 0x%X\n",tcw.NO_PAL.TexAddr<<3);
		}
	};

	TexCacheList<TextureCacheData> TexCache;


	TextureCacheData* __fastcall GenText(TSP tsp,TCW tcw,TextureCacheData* tf)
	{
		//generate texture
		tf->Start=(tcw.NO_PAL.TexAddr<<3) & 0x7FFFFF;
		glGenTextures(1,&tf->texID);
		tf->w=8<<tsp.TexU;
		tf->h=8<<tsp.TexV;
		tf->tsp=tsp;
		tf->tcw=tcw;
		tf->dirty=true;
		tf->lock_block=0;
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
	GLuint RenderToTextureTex;
	GLuint __fastcall GetTexture(TSP tsp,TCW tcw)
	{	
		u32 addr=tcw.NO_PAL.TexAddr<<3;
		if (addr==RenderToTextureAddr)
			return RenderToTextureTex;

		TextureCacheData* tf = TexCache.Find(tcw.full,tsp.full);
		if (tf)
		{
			if (tf->dirty)
			{
				if ((tf->tsp.full==tsp.full) && (tf->tcw.full==tcw.full))
					tf->Update();
				else
				{
					glDeleteTextures(1,&tf->texID);
					GenText(tsp,tcw,tf);
				}
			}
			tf->Lookups++;
			return tf->texID;
		}
		else
		{
			tf = GenText(tsp,tcw);
			return tf->texID;
		}
		return 0;
	}
	
	void VramLockedWrite(vram_block* bl)
	{
		TextureCacheData* tcd = (TextureCacheData*)bl->userdata;
		tcd->dirty=true;
		tcd->lock_block=0;
		params.vram_unlock(bl);
	}

	//OpenGl Init/Term
	HDC   hdc1;
	HGLRC    hglrc1;
	/*GLvoid ReSizeGLScene(GLsizei width, GLsizei height)				// Resize And Initialize The GL Window
	{
		if (height==0)								// Prevent A Divide By Zero By
		{
			height=1;							// Making Height Equal One
		}

		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glLoadIdentity();							// Reset The Projection Matrix
	}*/

	void Resize();
	void EnableOpenGL(HWND hWnd, HDC& hDC, HGLRC& hRC)
	{
		GLuint	PixelFormat;

		static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
		{
			sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
			1,											// Version Number
			PFD_DRAW_TO_WINDOW |						// Format Must Support Window
			PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
			PFD_DOUBLEBUFFER,							// Must Support Double Buffering
			PFD_TYPE_RGBA,								// Request An RGBA Format
			32,											// Select Our Color Depth
			0, 0, 0, 0, 0, 0,							// Color Bits Ignored
			0,											// No Alpha Buffer
			0,											// Shift Bit Ignored
			0,											// No Accumulation Buffer
			0, 0, 0, 0,									// Accumulation Bits Ignored
			24,											// 16Bit Z-Buffer (Depth Buffer)  
			0,											// No Stencil Buffer
			0,											// No Auxiliary Buffer
			PFD_MAIN_PLANE,								// Main Drawing Layer
			0,											// Reserved
			0, 0, 0										// Layer Masks Ignored
		};

		if(!(hDC=GetDC(hWnd)))
		{
			MessageBoxA(hWnd,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
			//Term();	return false;
		}
		if( !(PixelFormat=ChoosePixelFormat(hDC,&pfd)))
		{
			MessageBoxA(hWnd,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
			//		Term();	return false;
		}
		if( !SetPixelFormat(hDC,PixelFormat,&pfd))
		{
			MessageBoxA(hWnd,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
			//		Term();	return false;
		}
		if(!(hRC=wglCreateContext(hDC)) || !wglMakeCurrent(hDC,hRC))
		{
			MessageBoxA(hWnd,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
			//		Term();	return false;
		}
		/*if (GLEW_OK != glewInit())
		{
		MessageBoxA(hWnd,"Couldn't Initialize GLEW.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		//		Term();	return false;
		}*/


		//	if( gfx_opts.wireframe == TRUE )
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//		else
		{	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	}

		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);


		glShadeModel(GL_SMOOTH);
		glAlphaFunc(GL_GREATER, 0.f);

		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearDepth(0.f);	// 0 ? wtf .. thX F|RES

		glClearColor(0.07f, 0.25f, 0.12f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable( GL_CULL_FACE );

		Resize();
	}
	void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( hRC );
		ReleaseDC( hWnd, hDC );
	}
	//use that someday
	void VBlank()
	{
	}

	//u32 VertexCount;
	//u32 FrameCount;

	//Vertex storage types
	//64B
	struct Vertex
	{
		//0
		float xyz[3];

		//12
		u32 nil;//padding

		//16
		float col[4];

		//32
		//tex cords if texture
		float uv[4];

		//48
		//offset color
		float col_offset[4];

		//64
	};

	//8B
	struct VertexList
	{
		u32 first;
		u32 sz;
	};

		bool gl_Text2D_enabled;
	bool gl_TextCord_enabled;
		const static u32 SrcBlendGL[] =
	{
		GL_ZERO,
		GL_ONE,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA
	};

	const static u32 DstBlendGL[] =
	{
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA
	};
	//8B
	struct PolyParam
	{
		u32 first;		//entry index , holds vertex/pos data
		u32 vlc;

		//lets see what more :)
		TCW tcw;	//0xFFFFFFFF if no texture
		TSP tsp;

		void SetRenderMode_Op()
		{
			if (first&0x80000000)
			{
				GLuint texID=GetTexture(tsp,tcw);
			
				glEnable(GL_TEXTURE_2D);
				

				glBindTexture(GL_TEXTURE_2D,texID);

				glEnableClientState( GL_TEXTURE_COORD_ARRAY );
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			}
			else
			{
				glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				glDisable(GL_TEXTURE_2D);
			}
		}
		void SetRenderMode_Tr()
		{
			if (first&0x80000000)
			{
				GLuint texID=GetTexture(tsp,tcw);
				//verify(glIsTexture(texID));
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,texID);
				glEnableClientState( GL_TEXTURE_COORD_ARRAY );
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			}
			else
			{
				glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				glDisable(GL_TEXTURE_2D);
			}
			
			if(tsp.UseAlpha)
			{
				glEnable(GL_BLEND);
				glBlendFunc(SrcBlendGL[tsp.SrcInstr], DstBlendGL[tsp.DstInstr]);
			}
			else
				glDisable(GL_BLEND);
		}
		void SetRenderMode_Pt()
		{
			if (first&0x80000000)
			{
				GLuint texID=GetTexture(tsp,tcw);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,texID);
				glEnableClientState( GL_TEXTURE_COORD_ARRAY );
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			}
			else
			{
				glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				glDisable(GL_TEXTURE_2D);
			}
		}
	};


	//vertex lists
	struct TA_context
	{
		u32 Address;
		u32 LastUsed;
		f32 invW_min;
		f32 invW_max;
		List<Vertex> verts;
		List<VertexList> vertlists;
		List<PolyParam> global_param_op;
		List<PolyParam> global_param_pt;
		List<PolyParam> global_param_tr;

		
		void Init()
		{
			verts.Init();
			vertlists.Init();
			global_param_op.Init();
			global_param_pt.Init();
			global_param_tr.Init();
		}
		void Clear()
		{
			verts.Clear();
			vertlists.Clear();
			global_param_op.Clear();
			global_param_pt.Clear();
			global_param_tr.Clear();
			invW_min= 1000000.0f;
			invW_max=-1000000.0f;
		}
		void Free()
		{
			verts.Free();
			vertlists.Free();
			global_param_op.Free();
			global_param_pt.Free();
			global_param_tr.Free();
		}
	};

	
	TA_context tarc;
	TA_context pvrrc;
	
	float near_z=1;
	float far_z=1000;


# define InvSrcBlendGL(x) SrcBlendGL[7-x]
# define InvDstBlendGL(x) DstBlendGL[7-x]


	void Resize(u32 w,u32 h,bool inv=false)
	{
		static u32 ow=INFINITE,oh=INFINITE,oinv=INFINITE;
		if (!((w==ow) && (h==oh) && (oinv==(inv?1:0))))
		{
			ow=w;
			oh=h;
			oinv=(inv?1:0);
			glViewport( 0,0, w, h );
		}

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//verify(near_z<=far_z);
		if (inv)
		{
			glOrtho(0,640,0,480,near_z ,  far_z );
			//glViewport( w/2,h/2, w, h );
		}
		else
			glOrtho(0,640,480,0,near_z,  far_z);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	void Resize()
	{
		RECT rClient;
		GetClientRect((HWND)emu.GetRenderTarget(),&rClient);

		Resize((u32)(rClient.right-rClient.left), (u32)(rClient.bottom-rClient.top) );
	}


	template <u32 Type>
	__forceinline
	void RendStrips(PolyParam* gp)
	{
		u32 vlc=gp->vlc;
		VertexList* v=&pvrrc.vertlists.data[gp->first];
		
		if (Type==0)
			gp->SetRenderMode_Op();
		else if (Type==1)
			gp->SetRenderMode_Pt();
		else
			gp->SetRenderMode_Tr();

		while(vlc--)
		{
			glDrawArrays(GL_TRIANGLE_STRIP, v->first&0x7FFFFFFF, v->sz);
			v++;//next vertex list
		}
	}

	template <u32 Type>
	void RendPolyParamList(List<PolyParam>& gpl)
	{
		for (u32 i=0;i<gpl.used;i++)
		{		
			RendStrips<Type>(&gpl.data[i]);
		}
	}
	__forceinline float clamp01(float v)
	{
		v=v>1?1:v;
		v=v<0?0:v;
		return v;
	}
	
	//split into 2 regions
	void FixZValues_1(List<Vertex>& verts,f32 z_max,f32 z_min)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearDepth(0.f);	// 0 ? wtf .. thX F|RES

		f32 z_diff=z_max;
		z_max*=1.01f;
		z_diff=z_max-z_diff;
		f32 z_min2,z_max2;

		//fixup z_min
		if (z_min==0)
			z_min=0.0000001f;
		if (z_min<=1)
		{
			z_min2=3.6f+(1/(z_min));
		}
		else
		{
			//z_max>=invW
			z_min2=z_max-z_min;	//smaller == closer to screen
			z_min2/=z_min;		//scale from 0 to 1
			z_min2=2.1f+z_min;	//its in front of other Z :)
		}

		//fixup z_max
		if (z_max<=1)
		{
			z_max2=3.4f+(1/(z_max));
		}
		else
		{
			//z_max>=invW
			z_max2=z_diff;	//smaller == closer to screen
			z_max2/=z_max;		//scale from 0 to 1
			z_max2=1.9f+z_max;	//its in front of other Z :)
		}

		f32 z_scale=(z_min2-z_max2); //yay ?

		Vertex* cv = &verts.data[0];
		for (u32 i=0;i<verts.used;i++)
		{
			//cv is 0 for infinitevly away , <0 for closer
			f32 invW=cv->xyz[2];
			if (invW==0)
				invW=0.0000002f;

			if (invW<=1)
			{
				invW=3.5f+(1/(invW));
			}
			else
			{
				//z_max is >=invW
				invW=z_max-invW;	//smaller == closer to screen
				invW/=z_max;		//scale from 0 to 1
				invW=2+invW;		//its in front of other Z :)
			}

			cv->xyz[2]=clamp01((invW-z_max2)/z_min2);	//scale from 0 to 1

			//now , the larger z , the further away things are now
			cv++;
		}

		far_z=-1;
		near_z=1;
	}
	//split into 3 regions
	void FixZValues_2(List<Vertex>& verts,f32 z_max,f32 z_min)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearDepth(0.f);	// 0 ? wtf .. thX F|RES

		f32 z_diff=z_max;
		z_max*=1.01f;
		z_diff=z_max-z_diff;
		f32 z_min2,z_max2;

		//fixup z_min
		if (z_min==0)
			z_min=0.0000001f;
		if (z_min<=1)
		{
			z_min2=3.6f+(1/(z_min));
		}
		else
		{
			//z_max>=invW
			z_min2=z_max-z_min;	//smaller == closer to screen
			z_min2/=z_min;		//scale from 0 to 1
			z_min2=2.1f+z_min;	//its in front of other Z :)
		}

		//fixup z_max
		if (z_max<=1)
		{
			z_max2=3.4f+(1/(z_max));
		}
		else
		{
			//z_max>=invW
			z_max2=z_diff;	//smaller == closer to screen
			z_max2/=z_max;		//scale from 0 to 1
			z_max2=1.9f+z_max;	//its in front of other Z :)
		}

		f32 z_scale=(z_min2-z_max2); //yay ?

		Vertex* cv = &verts.data[0];
		for (u32 i=0;i<verts.used;i++)
		{
			//cv is 0 for infinitevly away , <0 for closer
			f32 invW=cv->xyz[2];
			if (invW==0)
				invW=0.0000002f;

			if (invW<=0.0001f)
			{
				invW=3.1f+(1/(invW));
			}
			else if (invW<=1)
			{
				invW=3.5f+(1/(invW));
			}
			else
			{
				//z_max is >=invW
				invW=z_max-invW;	//smaller == closer to screen
				invW/=z_max;		//scale from 0 to 1
				invW=2+invW;		//its in front of other Z :)
			}

			cv->xyz[2]=clamp01((invW-z_max2)/z_min2);	//scale from 0 to 1

			//now , the larger z , the further away things are now
			cv++;
		}
		far_z=-1;
		near_z=1;
	}
	//rescale to 0...1
	void FixZValues_3(List<Vertex>& verts,f32 z_max,f32 z_min)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearDepth(0.f);	// 0 ? wtf .. thX F|RES

		z_max*=1.01f;
		
	
		Vertex* cv = &verts.data[0];
		for (u32 i=0;i<verts.used;i++)
		{
			//cv is 0 for infinitevly away , <0 for closer
			f32 invW=cv->xyz[2];

			invW=z_max-invW;	//smaller == closer to screen
			invW/=z_max;

			cv->xyz[2]=clamp01(invW);

			//now , the larger z , the further away things are now
			cv++;
		}

		
		far_z=-1;
		near_z=1;
	}
	//split to 2 regions , rescale 0...0.8 first region , 0.8 to 1 second
	void FixZValues_4(List<Vertex>& verts,f32 z_max,f32 z_min)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearDepth(0.f);	// 0 ? wtf .. thX F|RES

		z_max*=1.01f;
		
	
		Vertex* cv = &verts.data[0];
		for (u32 i=0;i<verts.used;i++)
		{
			//cv is 0 for infinitevly away , <0 for closer
			f32 invW=cv->xyz[2];

			invW=z_max-invW;	//smaller == closer to screen
			invW*=0.8f;
			//invW/=z_max;
			if (invW >=0.8f)
			{
				cv->xyz[2]=0.99f - 0.14f/invW;
			}
			else
				cv->xyz[2]=invW;

			//now , the larger z , the further away things are now
			cv++;
		}

		far_z=-1;
		near_z=1;
	}
	
	//Here is a new idea ! ~~ 

	void FixZValues_5(List<Vertex>& verts,f32 invW_max,f32 invW_min)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearDepth(0);

		//GFX thinks we give it a normal Z value
/*		z_max*=1.01f;
		
		if (z_min==0)
			z_min=0.0000001f;
*/	/*
		float z_max,z_min;
		if (invW_max>1000000)
			invW_max=1000000;
		if (invW_max==0)
			near_z=888800000000.0f; //some realy big value , its infinately away ...
		else
			near_z=1/invW_max;//restore W

		if (invW_min==0)
			far_z=888800000000.0f; //some realy big value , its infinately away ...
		else
			far_z=1/invW_min;//restore W
*/
		far_z=10000;
		near_z=-10000;
		/*Vertex* cv = &verts.data[0];
		for (u32 i=0;i<verts.used;i++)
		{
			//cv is 0 for infinitevly away , <0 for closer
			f32 invW=cv->xyz[2];

			if (invW>invW_max)
				invW=invW_max;

			float W;
			if (invW==0)
				W=888800000000.0f; //some realy big value , its infinately away ...
			else
				W=1/invW;//restore W

			//now , the larger z , the further away things are now
			//cv->xyz[2]=W;
			cv++;
		}*/

		//constants were edited a bit , so z_min & z_max can be used as clipping planes :)

		//near cliping is z_max (closest point to screen) and far cliping is z_min
	}
	//

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
		//verify(Rendering==false);
		//return;
	//	printf("SetCurrentPVRRC:0x%X\n",addr);
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

	bool render=true;
	int zfix=1;

	bool running=true;
	cResetEvent rs(false,true);
	cResetEvent re(false,true);
	GLuint VertexBufferObject;
	void DoRender();
	bool InitGL()
	{
		//start open gl !
		EnableOpenGL((HWND)emu.GetRenderTarget(),hdc1,hglrc1);
		GLenum err = glewInit();
		if (err!=GLEW_OK || !GLEW_EXT_gpu_shader4 || !GLEW_EXT_geometry_shader4 || !GLEW_NV_depth_buffer_float)
		{
			printf("This plugin needs GLEW_EXT_gpu_shader4,GLEW_EXT_geometry_shader4,GLEW_NV_depth_buffer_float\n");
			DisableOpenGL((HWND)emu.GetRenderTarget(),hdc1,hglrc1);
			return false;
		}
		//glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32F_NV, 640, 480);
		
		GLboolean db3;
		glGetBooleanv(GL_DEPTH_BUFFER_FLOAT_MODE_NV,&db3);

		glGenTextures(1,&RenderToTextureTex);
		glBindTexture(GL_TEXTURE_2D, RenderToTextureTex);			// Bind The Texture
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 1024, 1024, 0,GL_RGBA, GL_UNSIGNED_BYTE, temp_tex_buffer);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		
		glGenBuffersARB(1,&VertexBufferObject);

		running=true;
		return true;
	}

	volatile bool OpenGlSucceeded=false;
	u32 THREADCALL RenderThead(void* param)
	{
		bool bext=InitGL();
		OpenGlSucceeded=bext;
		re.Set();
		if (!bext)
			return 0;
		

		while(1)
		{
			rs.Wait();
			if (!running)
				break;
			//render
			DoRender();
			re.Set();
		}

		//terminate opengl :)
		glDeleteTextures(1,&RenderToTextureTex);
		DisableOpenGL((HWND)emu.GetRenderTarget(),hdc1,hglrc1);
		return 0;
	}

	cThread rth(RenderThead,0);

	void DoRender()
	{
		
		if (GetAsyncKeyState('1'))
			zfix=0;
		if (GetAsyncKeyState('2'))
			zfix=1;
		if (GetAsyncKeyState('3'))
			zfix=2;
		if (GetAsyncKeyState('4'))
			zfix=3;
		if (GetAsyncKeyState('5'))
			zfix=4;
		if (GetAsyncKeyState('8'))
			zfix=-1;

		if (GetAsyncKeyState('9'))
			render=false;
		if (GetAsyncKeyState('0'))
			render=true;

		

		
		if (render)
		{
			//fixeszz
			if (zfix==0)
			{
				FixZValues_1(pvrrc.verts,pvrrc.invW_max,pvrrc.invW_min);
			}
			else if (zfix==1)
			{
				FixZValues_2(pvrrc.verts,pvrrc.invW_max,pvrrc.invW_min);
			}
			else if (zfix==2)
			{
				FixZValues_3(pvrrc.verts,pvrrc.invW_max,pvrrc.invW_min);
			}
			else if (zfix ==3)
			{
				FixZValues_4(pvrrc.verts,pvrrc.invW_max,pvrrc.invW_min);
			}
			else if (zfix ==4)
			{
				FixZValues_5(pvrrc.verts,pvrrc.invW_max,pvrrc.invW_min);
			}
		}

		if ((FB_W_SOF1 & 0x1000000)!=0)
		{
			return;
			//yay render to texture yay
			Resize(2048,2048,true);
		}
		else
		{
			Resize();
			FrameCount++;
		}

		if (render)
		{

			if (GetAsyncKeyState('Q'))
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			if (GetAsyncKeyState('W'))
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindBufferARB( GL_ARRAY_BUFFER_ARB, VertexBufferObject );
			glBufferDataARB(GL_ARRAY_BUFFER,pvrrc.verts.used*sizeof(Vertex),pvrrc.verts.data,GL_STREAM_DRAW);

			Vertex* pvtx0=0;
			glColorPointer(4, GL_FLOAT, sizeof(Vertex),  pvtx0->col);
			glTexCoordPointer(4, GL_FLOAT, sizeof(Vertex), pvtx0->uv);
			glVertexPointer(3, GL_FLOAT, sizeof(Vertex), pvtx0->xyz);
			

			glEnableClientState( GL_COLOR_ARRAY );
			glEnableClientState( GL_VERTEX_ARRAY );


			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);

			RendPolyParamList<0>(pvrrc.global_param_op);

			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GEQUAL, (float)(PT_ALPHA_REF &0xFF)/255.f);

			RendPolyParamList<1>(pvrrc.global_param_pt);

			glAlphaFunc(GL_GREATER, 0.f);
			RendPolyParamList<2>(pvrrc.global_param_tr);
		}
		if ((FB_W_SOF1 & 0x1000000)!=0)
		{
			glBindTexture(GL_TEXTURE_2D,RenderToTextureTex);
//			CheckErrorsGL("display: glBindTexture");
			//CopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0           , 0            , 0            , 0      ,0       , 1024, 1024);
//			CheckErrorsGL("display: glCopyTexImage2D");
			RenderToTextureAddr=FB_W_SOF1 & 0x7FFFFF; ///bwhahaha
		}
		else
		{
			SwapBuffers(hdc1);
			RenderToTextureAddr=0xFFFFFFFF;
		}
	}
	void StartRender()
	{
		SetCurrentPVRRC(PARAM_BASE);
		VertexCount+= pvrrc.verts.used;
		render_end_pending_cycles=pvrrc.verts.used*170+100000;
		//Rendering=true;
		rs.Set();
	}

	void EndRender()
	{
		re.Wait();
		//Rendering=false;
	}
	using namespace TASplitter;
	VertexList* CurrentVL=0;
	PolyParam* CurrentPP=0;
	List<PolyParam>* CurrentPPlist;

	f32 f16(u16 v)
	{
		u32 z=v<<16;
		return *(f32*)&z;
	}
	
	struct VertexDecoder
	{
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

		}
		__forceinline
		static void EndList(u32 ListType)
		{
			CurrentPPlist=0;
		}

		//Polys
#define glob_param_bdc \
		PolyParam* d_pp =CurrentPPlist->Append(); \
		\
		CurrentPP=d_pp;\
		\
		d_pp->first=tarc.vertlists.used;\
		d_pp->vlc=0;\
		if (pp->pcw.Texture)\
			d_pp->first|=0x80000000;\
			\
		d_pp->tcw=pp->tcw;\
		d_pp->tsp=pp->tsp;

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
		}
		__forceinline
		static void fastcall AppendPolyParam2A(TA_PolyParam2A* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void fastcall AppendPolyParam2B(TA_PolyParam2B* pp)
		{
			
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
			
		}

		//Poly Strip handling
		__forceinline
		static void StartPolyStrip()
		{
			CurrentVL= tarc.vertlists.Append();
			CurrentVL->first=tarc.verts.used;
			CurrentVL->sz=0;
		}
		__forceinline
		static void EndPolyStrip()
		{
			CurrentPP->vlc++;
			CurrentVL = 0;
		}

		//Poly Vertex handlers

#define vert_cvt_base \
	Vertex* cv=tarc.verts.Append();\
	CurrentVL->sz++;\
	cv->xyz[0]=vtx->xyz[0];\
	cv->xyz[1]=vtx->xyz[1];\
	f32 invW=vtx->xyz[2];\
	cv->xyz[2]=invW;\
	if (tarc.invW_min>invW)\
		tarc.invW_min=invW;\
	if (tarc.invW_max<invW)\
		tarc.invW_max=invW;

#define vert_uv_32(u_name,v_name) \
		cv->uv[0]	=	(vtx->u_name)*invW;\
		cv->uv[1]	=	(vtx->v_name)*invW;\
		cv->uv[2]	=	0; \
		cv->uv[3]	=	invW; 

#define vert_uv_16(u_name,v_name) \
		cv->uv[0]	=	f16(vtx->u_name)*invW;\
		cv->uv[1]	=	f16(vtx->v_name)*invW;\
		cv->uv[2]	=	0; \
		cv->uv[3]	=	invW; 

		//(Non-Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
		{
			vert_cvt_base;
			cv->col[0]	= (255 & (vtx->BaseCol >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol >> 24)) / 255.f;
		}

		//(Non-Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
		{
			vert_cvt_base;
			cv->col[0]	= vtx->BaseR;
			cv->col[1]	= vtx->BaseG;
			cv->col[2]	= vtx->BaseB;
			cv->col[3]	= vtx->BaseA;
		}

		//(Non-Textured, Intensity)
		__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= vtx->BaseInt;
			cv->col[1]	= vtx->BaseInt;
			cv->col[2]	= vtx->BaseInt;
			cv->col[3]	= vtx->BaseInt;
		}

		//(Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= (255 & (vtx->BaseCol >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol >> 24)) / 255.f;

			vert_uv_32(u,v);
		}

		//(Textured, Packed Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= (255 & (vtx->BaseCol >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol >> 24)) / 255.f;

			vert_uv_16(u,v);
		}

		//(Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= 1;//vtx->BaseR;
			cv->col[1]	= 1;//vtx->BaseG;
			cv->col[2]	= 1;//vtx->BaseB;
			cv->col[3]	= 1;//vtx->BaseA;

			vert_uv_32(u,v);
		}
		__forceinline
		static void AppendPolyVertex5B(TA_Vertex5B* vtx)
		{

		}

		//(Textured, Floating Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex6A(TA_Vertex6A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= 1;//vtx->BaseR;
			cv->col[1]	= 1;//vtx->BaseG;
			cv->col[2]	= 1;//vtx->BaseB;
			cv->col[3]	= 1;//vtx->BaseA;

			vert_uv_16(u,v);
		}
		__forceinline
		static void AppendPolyVertex6B(TA_Vertex6B* vtx)
		{

		}

		//(Textured, Intensity)
		__forceinline
		static void AppendPolyVertex7(TA_Vertex7* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= vtx->BaseInt;
			cv->col[1]	= vtx->BaseInt;
			cv->col[2]	= vtx->BaseInt;
			cv->col[3]	= vtx->BaseInt;

			vert_uv_32(u,v);
		}

		//(Textured, Intensity, 16bit UV)
		__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= 1.0f;//vtx->BaseInt;
			cv->col[1]	= 1.0f;//vtx->BaseInt;
			cv->col[2]	= 1.0f;//vtx->BaseInt;
			cv->col[3]	= 1.0f;//vtx->BaseInt;

			vert_uv_16(u,v);
		}

		//(Non-Textured, Packed Color, with Two Volumes)
		__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= (255 & (vtx->BaseCol0 >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol0 >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol0 >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol0 >> 24)) / 255.f;
		}

		//(Non-Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= 1.0f;//vtx->BaseInt0;
			cv->col[1]	= 1.0f;//vtx->BaseInt0;
			cv->col[2]	= 1.0f;//vtx->BaseInt0;
			cv->col[3]	= 1.0f;//vtx->BaseInt0;
		}

		//(Textured, Packed Color,	with Two Volumes)	
		__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= (255 & (vtx->BaseCol0 >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol0 >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol0 >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol0 >> 24)) / 255.f;

			vert_uv_32(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex11B(TA_Vertex11B* vtx)
		{
			
		}

		//(Textured, Packed Color, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex12A(TA_Vertex12A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= (255 & (vtx->BaseCol0 >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol0 >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol0 >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol0 >> 24)) / 255.f;

			vert_uv_16(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex12B(TA_Vertex12B* vtx)
		{

		}

		//(Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex13A(TA_Vertex13A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= 1.0f;//vtx->BaseInt0;
			cv->col[1]	= 1.0f;//vtx->BaseInt0;
			cv->col[2]	= 1.0f;//vtx->BaseInt0;
			cv->col[3]	= 1.0f;//vtx->BaseInt0;

			vert_uv_32(u0,v0);			
		}
		__forceinline
		static void AppendPolyVertex13B(TA_Vertex13B* vtx)
		{

		}

		//(Textured, Intensity, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex14A(TA_Vertex14A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= 1.0f;//vtx->BaseInt0;
			cv->col[1]	= 1.0f;//vtx->BaseInt0;
			cv->col[2]	= 1.0f;//vtx->BaseInt0;
			cv->col[3]	= 1.0f;//vtx->BaseInt0;

			vert_uv_16(u0,v0);	
		}
		__forceinline
		static void AppendPolyVertex14B(TA_Vertex14B* vtx)
		{

		}

		//Sprites
		__forceinline
		static void AppendSpriteParam(TA_SpriteParam* spr)
		{
			//printf("Sprite\n");
		}

		//Sprite Vertex Handlers
		__forceinline
		static void AppendSpriteVertexA(TA_Sprite1A* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertexB(TA_Sprite1B* sv)
		{

		}


		//ModVolumes
		__forceinline
		static void AppendModVolParam(TA_ModVolParam* modv)
		{

		}

		//ModVol Strip handling
		__forceinline
		static void ModVolStripStart()
		{

		}
		__forceinline
		static void ModVolStripEnd()
		{

		}

		//Mod Volume Vertex handlers
		__forceinline
		static void AppendModVolVertexA(TA_ModVolA* mvv)
		{

		}
		__forceinline
		static void AppendModVolVertexB(TA_ModVolB* mvv)
		{

		}
		static void StartModVol(TA_ModVolParam* param)
		{
		}
		__forceinline
		static void SetTileClip(u32 xmin,u32 ymin,u32 xmax,u32 ymax)
		{
			
		}
		__forceinline
		static void TileClipMode(u32 mode)
		{
		
		}

		//Misc
		__forceinline
		static void ListCont()
		{
			//printf("LC : TA OL base = 0x%X\n",TA_OL_BASE);
		}
		__forceinline
		static void ListInit()
		{
			//printf("LI : TA OL base = 0x%X\n",TA_OL_BASE);
			SetCurrentTARC(TA_ISP_BASE);
			tarc.Clear();
		}
		__forceinline
		static void SoftReset()
		{
		}
	};

	FifoSplitter<VertexDecoder> TileAccel;

	bool InitRenderer()
	{
		for (int i=0;i<rcnt.size();i++)
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
		for (int i=0;i<rcnt.size();i++)
		{
			rcnt[i].Free();
		}
		rcnt.clear();
		TileAccel.Term();
	}

	void ResetRenderer(bool Manual)
	{
		TileAccel.Reset(Manual);
		FrameCount=0;
		VertexCount=0;
		RenderToTextureAddr=0xFFFFFFFF;
	}

	bool ThreadStart()
	{
		rth.Start();
		re.Wait(INFINITE);
		
		return OpenGlSucceeded;
	}

	void ThreadEnd()
	{
		running=false;
		rs.Set();
		rth.WaitToEnd(0xFFFFFFFF);
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
}
#endif