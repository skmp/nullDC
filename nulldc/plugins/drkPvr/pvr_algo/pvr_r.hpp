#ifndef _pvr_r_hpp_
#define _pvr_r_hpp_

//Can be anything really;)
#include "d3d9types.h" 

	static const u32 R_NO_SAMPLER = 8;
	static const u32 k_max_pvr_samplers = 7 + 2;
	
	//All render states
	enum {
	   R_ZENABLE   = 0                , 
		R_FILLMODE                 ,
		R_SHADEMODE               ,
		R_ZWRITEENABLE           ,
		R_ALPHATESTENABLE      ,
		R_LASTPIXEL                , 
		R_SRCBLEND                , 
		R_DESTBLEND                 ,   /* D3DBLEND */
		R_CULLMODE                  ,   /* D3DCULL */
		R_ZFUNC                     ,   /* D3DCMPFUNC */
		R_ALPHAREF                  ,   /* D3DFIXED */
		R_ALPHAFUNC                 ,   /* D3DCMPFUNC */
		R_DITHERENABLE              ,   /* TRUE to enable dithering */
		R_ALPHABLENDENABLE          ,   /* TRUE to enable alpha blending */
		R_FOGENABLE                 ,   /* TRUE to enable fog blending */
		R_SPECULARENABLE            ,   /* TRUE to enable specular */
		R_FOGCOLOR                  ,   /* D3DCOLOR */
		R_FOGTABLEMODE              ,   /* D3DFOGMODE */
		R_FOGSTART                  ,   /* Fog start (for both vertex and pixel fog) */
		R_FOGEND                    ,   /* Fog end      */
		R_FOGDENSITY                ,   /* Fog density  */
		R_RANGEFOGENABLE            ,   /* Enables range-based fog */
		R_STENCILENABLE             ,   /* BOOL enable/disable stenciling */
		R_STENCILFAIL               ,   /* D3DSTENCILOP to do if stencil test fails */
		R_STENCILZFAIL              ,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
		R_STENCILPASS               ,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
		R_STENCILFUNC               ,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		R_STENCILREF                ,   /* Reference value used in stencil test */
		R_STENCILMASK               ,   /* Mask value used in stencil test */
		R_STENCILWRITEMASK          ,   /* Write mask applied to values written to stencil buffer */
		R_TEXTUREFACTOR             ,   /* D3DCOLOR used for multi-texture blend */
		R_WRAP0                     ,  /* wrap for 1st texture coord. set */
		R_WRAP1                     ,  /* wrap for 2nd texture coord. set */
		R_WRAP2                     ,  /* wrap for 3rd texture coord. set */
		R_WRAP3                     ,  /* wrap for 4th texture coord. set */
		R_WRAP4                     ,  /* wrap for 5th texture coord. set */
		R_WRAP5                     ,  /* wrap for 6th texture coord. set */
		R_WRAP6                     ,  /* wrap for 7th texture coord. set */
		R_WRAP7                     ,  /* wrap for 8th texture coord. set */
		R_CLIPPING                  ,
		R_LIGHTING                  ,
		R_AMBIENT                   ,
		R_FOGVERTEXMODE             ,
		R_COLORVERTEX               ,
		R_LOCALVIEWER               ,
		R_NORMALIZENORMALS          ,
		R_DIFFUSEMATERIALSOURCE     ,
		R_SPECULARMATERIALSOURCE    ,
		R_AMBIENTMATERIALSOURCE     ,
		R_EMISSIVEMATERIALSOURCE    ,
		R_VERTEXBLEND               ,
		R_CLIPPLANEENABLE           ,
		R_POINTSIZE                 ,   /* float point size */
		R_POINTSIZE_MIN             ,   /* float point size min threshold */
		R_POINTSPRITEENABLE         ,   /* BOOL point texture coord control */
		R_POINTSCALEENABLE          ,   /* BOOL point size scale enable */
		R_POINTSCALE_A              ,   /* float point attenuation A value */
		R_POINTSCALE_B              ,   /* float point attenuation B value */
		R_POINTSCALE_C              ,   /* float point attenuation C value */
		R_MULTISAMPLEANTIALIAS      ,  // BOOL - set to do FSAA with multisample buffer
		R_MULTISAMPLEMASK           ,  // DWORD - per-sample enable/disable
		R_PATCHEDGESTYLE            ,  // Sets whether patch edges will use float style tessellation
		R_DEBUGMONITORTOKEN         ,  // DEBUG ONLY - token to debug monitor
		R_POINTSIZE_MAX             ,   /* float point size max threshold */
		R_INDEXEDVERTEXBLENDENABLE  ,
		R_COLORWRITEENABLE          ,  // per-channel write enable
		R_TWEENFACTOR               ,   // float tween factor
		R_BLENDOP                   ,   // D3DBLENDOP setting
		R_POSITIONDEGREE            ,   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
		R_NORMALDEGREE              ,   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC
		R_SCISSORTESTENABLE         ,
		R_SLOPESCALEDEPTHBIAS       ,
		R_ANTIALIASEDLINEENABLE     ,
		R_MINTESSELLATIONLEVEL      ,
		R_MAXTESSELLATIONLEVEL      ,
		R_ADAPTIVETESS_X            ,
		R_ADAPTIVETESS_Y            ,
		R_ADAPTIVETESS_Z            ,
		R_ADAPTIVETESS_W            ,
		R_ENABLEADAPTIVETESSELLATION ,
		R_TWOSIDEDSTENCILMODE       ,   /* BOOL enable/disable 2 sided stenciling */
		R_CCW_STENCILFAIL           ,   /* D3DSTENCILOP to do if ccw stencil test fails */
		R_CCW_STENCILZFAIL          ,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
		R_CCW_STENCILPASS           ,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */
		R_CCW_STENCILFUNC           ,   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		R_COLORWRITEENABLE1         ,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		R_COLORWRITEENABLE2         ,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		R_COLORWRITEENABLE3         ,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		R_BLENDFACTOR               ,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
		R_SRGBWRITEENABLE           ,   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
		R_DEPTHBIAS                 ,
		R_WRAP8                     ,   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
		R_WRAP9                     ,
		R_WRAP10                    ,
		R_WRAP11                    ,
		R_WRAP12                    ,
		R_WRAP13                    ,
		R_WRAP14                    ,
		R_WRAP15                    ,
		R_SEPARATEALPHABLENDENABLE  ,  /* TRUE to enable a separate blending function for the alpha channel */
		R_SRCBLENDALPHA             ,  /* SRC blend factor for the alpha channel when R_SEPARATEDESTALPHAENABLE is TRUE */
		R_DESTBLENDALPHA            ,  /* DST blend factor for the alpha channel when R_SEPARATEDESTALPHAENABLE is TRUE */
		R_BLENDOPALPHA              ,  /* Blending operation for the alpha channel when R_SEPARATEDESTALPHAENABLE is TRUE */
		
		R_ADDRESSU       ,  /* D3DTEXTUREADDRESS for U coordinate */
		R_ADDRESSV       ,  /* D3DTEXTUREADDRESS for V coordinate */
		R_ADDRESSW       ,  /* D3DTEXTUREADDRESS for W coordinate */
		R_BORDERCOLOR   ,  /* D3DCOLOR */
		R_MAGFILTER      ,  /* D3DTEXTUREFILTER filter to use for magnification */
		R_MINFILTER      ,  /* D3DTEXTUREFILTER filter to use for minification */
		R_MIPFILTER      ,  /* D3DTEXTUREFILTER filter to use between mipmaps during minification */
		R_MIPMAPLODBIAS  ,  /* float Mipmap LOD bias */
		R_MAXMIPLEVEL    ,  /* DWORD 0..(n-1) LOD index of largest map to use (0 == largest) */
		R_MAXANISOTROPY  , /* DWORD maximum anisotropy */
		R_SRGBTEXTURE    , /* Default = 0 (which means Gamma 1.0,
									   no correction required.) else correct for
									   Gamma = 2.2 */
		R_ELEMENTINDEX   , /* When multi-element texture is assigned to sampler, this
										indicates which element index to use.  Default = 0.  */
		R_DMAPOFFSET     , /* Offset in vertices in the pre-sampled displacement map.
										Only valid for D3DDMAPSAMPLER sampler  */
	 
		/*Extended OPS*/
		R_EXT_OP_TEXTURE_BASE,

		/*Length*/
		R_STATES
	};
	
	static const u64 k_r_states_tbl[] = {
		D3DRS_ZENABLE                   , 
		D3DRS_FILLMODE                 ,
		D3DRS_SHADEMODE               ,
		D3DRS_ZWRITEENABLE           ,
		D3DRS_ALPHATESTENABLE      ,
		D3DRS_LASTPIXEL                , 
		D3DRS_SRCBLEND                , 
		D3DRS_DESTBLEND                 ,   /* D3DBLEND */
		D3DRS_CULLMODE                  ,   /* D3DCULL */
		D3DRS_ZFUNC                     ,   /* D3DCMPFUNC */
		D3DRS_ALPHAREF                  ,   /* D3DFIXED */
		D3DRS_ALPHAFUNC                 ,   /* D3DCMPFUNC */
		D3DRS_DITHERENABLE              ,   /* TRUE to enable dithering */
		D3DRS_ALPHABLENDENABLE          ,   /* TRUE to enable alpha blending */
		D3DRS_FOGENABLE                 ,   /* TRUE to enable fog blending */
		D3DRS_SPECULARENABLE            ,   /* TRUE to enable specular */
		D3DRS_FOGCOLOR                  ,   /* D3DCOLOR */
		D3DRS_FOGTABLEMODE              ,   /* D3DFOGMODE */
		D3DRS_FOGSTART                  ,   /* Fog start (for both vertex and pixel fog) */
		D3DRS_FOGEND                    ,   /* Fog end      */
		D3DRS_FOGDENSITY                ,   /* Fog density  */
		D3DRS_RANGEFOGENABLE            ,   /* Enables range-based fog */
		D3DRS_STENCILENABLE             ,   /* BOOL enable/disable stenciling */
		D3DRS_STENCILFAIL               ,   /* D3DSTENCILOP to do if stencil test fails */
		D3DRS_STENCILZFAIL              ,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
		D3DRS_STENCILPASS               ,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
		D3DRS_STENCILFUNC               ,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		D3DRS_STENCILREF                ,   /* Reference value used in stencil test */
		D3DRS_STENCILMASK               ,   /* Mask value used in stencil test */
		D3DRS_STENCILWRITEMASK          ,   /* Write mask applied to values written to stencil buffer */
		D3DRS_TEXTUREFACTOR             ,   /* D3DCOLOR used for multi-texture blend */
		D3DRS_WRAP0                     ,  /* wrap for 1st texture coord. set */
		D3DRS_WRAP1                     ,  /* wrap for 2nd texture coord. set */
		D3DRS_WRAP2                     ,  /* wrap for 3rd texture coord. set */
		D3DRS_WRAP3                     ,  /* wrap for 4th texture coord. set */
		D3DRS_WRAP4                     ,  /* wrap for 5th texture coord. set */
		D3DRS_WRAP5                     ,  /* wrap for 6th texture coord. set */
		D3DRS_WRAP6                     ,  /* wrap for 7th texture coord. set */
		D3DRS_WRAP7                     ,  /* wrap for 8th texture coord. set */
		D3DRS_CLIPPING                  ,
		D3DRS_LIGHTING                  ,
		D3DRS_AMBIENT                   ,
		D3DRS_FOGVERTEXMODE             ,
		D3DRS_COLORVERTEX               ,
		D3DRS_LOCALVIEWER               ,
		D3DRS_NORMALIZENORMALS          ,
		D3DRS_DIFFUSEMATERIALSOURCE     ,
		D3DRS_SPECULARMATERIALSOURCE    ,
		D3DRS_AMBIENTMATERIALSOURCE     ,
		D3DRS_EMISSIVEMATERIALSOURCE    ,
		D3DRS_VERTEXBLEND               ,
		D3DRS_CLIPPLANEENABLE           ,
		D3DRS_POINTSIZE                 ,   /* float point size */
		D3DRS_POINTSIZE_MIN             ,   /* float point size min threshold */
		D3DRS_POINTSPRITEENABLE         ,   /* BOOL point texture coord control */
		D3DRS_POINTSCALEENABLE          ,   /* BOOL point size scale enable */
		D3DRS_POINTSCALE_A              ,   /* float point attenuation A value */
		D3DRS_POINTSCALE_B              ,   /* float point attenuation B value */
		D3DRS_POINTSCALE_C              ,   /* float point attenuation C value */
		D3DRS_MULTISAMPLEANTIALIAS      ,  // BOOL - set to do FSAA with multisample buffer
		D3DRS_MULTISAMPLEMASK           ,  // DWORD - per-sample enable/disable
		D3DRS_PATCHEDGESTYLE            ,  // Sets whether patch edges will use float style tessellation
		D3DRS_DEBUGMONITORTOKEN         ,  // DEBUG ONLY - token to debug monitor
		D3DRS_POINTSIZE_MAX             ,   /* float point size max threshold */
		D3DRS_INDEXEDVERTEXBLENDENABLE  ,
		D3DRS_COLORWRITEENABLE          ,  // per-channel write enable
		D3DRS_TWEENFACTOR               ,   // float tween factor
		D3DRS_BLENDOP                   ,   // D3DBLENDOP setting
		D3DRS_POSITIONDEGREE            ,   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
		D3DRS_NORMALDEGREE              ,   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC
		D3DRS_SCISSORTESTENABLE         ,
		D3DRS_SLOPESCALEDEPTHBIAS       ,
		D3DRS_ANTIALIASEDLINEENABLE     ,
		D3DRS_MINTESSELLATIONLEVEL      ,
		D3DRS_MAXTESSELLATIONLEVEL      ,
		D3DRS_ADAPTIVETESS_X            ,
		D3DRS_ADAPTIVETESS_Y            ,
		D3DRS_ADAPTIVETESS_Z            ,
		D3DRS_ADAPTIVETESS_W            ,
		D3DRS_ENABLEADAPTIVETESSELLATION ,
		D3DRS_TWOSIDEDSTENCILMODE       ,   /* BOOL enable/disable 2 sided stenciling */
		D3DRS_CCW_STENCILFAIL           ,   /* D3DSTENCILOP to do if ccw stencil test fails */
		D3DRS_CCW_STENCILZFAIL          ,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
		D3DRS_CCW_STENCILPASS           ,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */
		D3DRS_CCW_STENCILFUNC           ,   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		D3DRS_COLORWRITEENABLE1         ,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		D3DRS_COLORWRITEENABLE2         ,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		D3DRS_COLORWRITEENABLE3         ,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		D3DRS_BLENDFACTOR               ,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
		D3DRS_SRGBWRITEENABLE           ,   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
		D3DRS_DEPTHBIAS                 ,
		D3DRS_WRAP8                     ,   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
		D3DRS_WRAP9                     ,
		D3DRS_WRAP10                    ,
		D3DRS_WRAP11                    ,
		D3DRS_WRAP12                    ,
		D3DRS_WRAP13                    ,
		D3DRS_WRAP14                    ,
		D3DRS_WRAP15                    ,
		D3DRS_SEPARATEALPHABLENDENABLE  ,  /* TRUE to enable a separate blending function for the alpha channel */
		D3DRS_SRCBLENDALPHA             ,  /* SRC blend factor for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
		D3DRS_DESTBLENDALPHA            ,  /* DST blend factor for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
		D3DRS_BLENDOPALPHA              ,  /* Blending operation for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
		
		D3DSAMP_ADDRESSU       ,  /* D3DTEXTUREADDRESS for U coordinate */
		D3DSAMP_ADDRESSV       ,  /* D3DTEXTUREADDRESS for V coordinate */
		D3DSAMP_ADDRESSW       ,  /* D3DTEXTUREADDRESS for W coordinate */
		D3DSAMP_BORDERCOLOR   ,  /* D3DCOLOR */
		D3DSAMP_MAGFILTER      ,  /* D3DTEXTUREFILTER filter to use for magnification */
		D3DSAMP_MINFILTER      ,  /* D3DTEXTUREFILTER filter to use for minification */
		D3DSAMP_MIPFILTER      ,  /* D3DTEXTUREFILTER filter to use between mipmaps during minification */
		D3DSAMP_MIPMAPLODBIAS  ,  /* float Mipmap LOD bias */
		D3DSAMP_MAXMIPLEVEL    ,  /* DWORD 0..(n-1) LOD index of largest map to use (0 == largest) */
		D3DSAMP_MAXANISOTROPY  , /* DWORD maximum anisotropy */
		D3DSAMP_SRGBTEXTURE    , /* Default = 0 (which means Gamma 1.0,
									   no correction required.) else correct for
									   Gamma = 2.2 */
		D3DSAMP_ELEMENTINDEX   , /* When multi-element texture is assigned to sampler, this
										indicates which element index to use.  Default = 0.  */
		D3DSAMP_DMAPOFFSET     , /* Offset in vertices in the pre-sampled displacement map.
										Only valid for D3DDMAPSAMPLER sampler  */
 
	};
#endif

