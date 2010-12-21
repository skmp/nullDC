//Pixel Shader
//Pvr emulation using a pixel shader .. bahh :p
//pp_Texture -> 1 if texture is enabled , 0 if its not
//pp_Offset -> 1 if offset is enabled , 0 if its not (only valid when texture is enabled)
//pp_ShadInstr -> 0 to 3 , see pvr docs , valid only when texture is enabled
//pp_IgnoreTexA -> 1 if on  0 if off , valid only w/ textures on
//pp_UseAlpha -> 1 if on  0 if off , works when no textures are used too ?
//pp_FogCtrl
//misc #defines :
//ZBufferMode -> z buffer mode :p
//ZBufferMode : 0 -> fp fixup (nop)
//ZBufferMode : 1 -> fp Z emu (emulate fp on matnissa bits)
//ZBufferMode : 2 -> rescale  (nop)

//TextureLookup -> function to use for texture lookup.One of TextureLookup_Normal,TextureLookup_Palette,TextureLookup_Palette_Bilinear
struct pixel 
{
	float4 col : TEXCOORD1;
	
	#if pp_Texture==1
		#if pp_Offset==1
			float4 offs : TEXCOORD2;
		#endif
		//uv is now allways passed
	#endif
	
	float4 uv : TEXCOORD0;
};
 
sampler2D samplr : register(s0);
sampler2D tex_pal : register(s1);
sampler1D fog_table: register(s2);

float4 current_pal: register(c0);
float4 texture_size: register(c1);
float4 FOG_COL_VERT:register(c2);
float4 FOG_COL_RAM :register(c3);
float4 FOG_DENSITY :register(c4);

float4 TextureLookup_Normal(float4 uv)
{
	return tex2Dproj( samplr, uv);
}

//utility function for pal. lookups :)
float4 PalleteLookup(float4 pos)
{
	//xyzw -> x=index , y=bank
	float4 texcol=tex2D(tex_pal,pos.rg+current_pal.xy);
	return texcol;
}

float4 TextureLookup_Palette(float4 uv)
{
	float4 pal_color=TextureLookup_Normal(uv);
	
	return PalleteLookup(pal_color);
}

/*
	Bilinear filtering
	Screen space perspective -> Texture space linear (*W)
	Texture space quad, filtered
	This also takes dx/dy/mipmaps into account correctly, but drkpvr so far does not generate pal. texture index mipmaps so i commented it out for now
*/
float4 TextureLookup_Palette_Nproj(float2 uv/*,float2 dx,float2 dy*/)
{
	float4 pal_color=tex2D/*grad*/(samplr,uv/*,dx,dy*/);
	
	return PalleteLookup(pal_color);
}

float4 TextureLookup_Palette_Bilinear(float4 uv)
{
	float2 tcpoj=uv.xy/uv.w;			//Project texture to 2d tc space
	/*
	float2 dx=ddx(tcpoj.xy);			//Get x derivatives for mip mapping
	float2 dy=ddy(tcpoj.xy);			//Get y derivatives for mip mapping
	*/
	float2 Img=tcpoj*texture_size.xy-float2(0.5,0.5);	//to image space to get the frac/ceil
	float2 lt=floor(Img)/texture_size.xy;
	float2 weight=frac(Img);
	
	float ltx=lt.x;
	float4 top_left = TextureLookup_Palette_Nproj( lt /*,dx,dy*/);
	lt.x+=1/texture_size.x;
	float4 top_right = TextureLookup_Palette_Nproj( lt /*,dx,dy*/);
	lt.y+=1/texture_size.y;
	float4 bot_right = TextureLookup_Palette_Nproj( lt /*,dx,dy*/); 
	lt.x=ltx;
	float4 bot_left = TextureLookup_Palette_Nproj( lt/*,dx,dy*/);
	
	
	float4 top = lerp( top_left, top_right, weight.x );	//.x=0 -> left, .x=1 -> right
	float4 bot = lerp( bot_left, bot_right, weight.x );
	float4 final = lerp( top, bot, weight.y );			//.y=0 -> top , .y=1 -> bottom
	return final;
}

float4 TextureLookup_Palette_Bilinear_ko(float4 uv)
{
	float2 tcpoj=uv.xy/uv.w;			//Project texture to 2d tc space
	/*
	float2 dx=ddx(tcpoj.xy);			//Get x derivatives for mip mapping
	float2 dy=ddy(tcpoj.xy);			//Get y derivatives for mip mapping
	*/
	float2 Img=tcpoj*texture_size.xy-float2(0.5,0.5);	//to image space to get the frac/ceil
	float4 ltrb=float4(floor(Img),ceil(Img))*texture_size.zwzw;//zw=1/xy
	float2 weight=frac(Img);
	
	float4 top_left = TextureLookup_Palette_Nproj( ltrb.xy /*,dx,dy*/);
	float4 top_right = TextureLookup_Palette_Nproj( ltrb.zy /*,dx,dy*/);
	float4 bot_left = TextureLookup_Palette_Nproj( ltrb.xw /*,dx,dy*/);
	float4 bot_right = TextureLookup_Palette_Nproj( ltrb.zw /*,dx,dy*/); 
	
	float4 top = lerp( top_left, top_right, weight.x );	//.x=0 -> left, .x=1 -> right
	float4 bot = lerp( bot_left, bot_right, weight.x );
	float4 final = lerp( top, bot, weight.y );			//.y=0 -> top , .y=1 -> bottom
	return final;
}

//same as below, but uses fewer sm2 opcodes so that the damn shader can fit on 2_0 cards
float fdecp(float flt,out float e)
{
	float lg2=log2(flt);	//ie , 2.5
	float frc=frac(lg2);	//ie , 0.5
	e=lg2-frc;				//ie , 2.5-0.5=2 (exp)
	return pow(2,frc);		//2^0.5 (manitsa)
}
/*
float fdecp(float flt,out float e)
{
	//float fogexp=floor(log2(foginvW));				//0 ... 7
	//float fogexp_pow=pow(2,fogexp);					//0 ... 128
	//float fogman=(foginvW/fogexp_pow);				//[1,2) mantissa bits. that is 1.m

	e=floor(log2(flt));
	float powe=pow(2,e);
	return (w/powx);
}
*/
//compress Z to D{s6e18}S8
float CompressZ(float w)
{
	float x;
	float y=fdecp(w,x);
	x=clamp(x-16,-63,0);	//s6e18, max : 2^16*(2^18-1)/2(^18) , min : 2^-47*(2^18-1)/2(^18)
	x+=62;					//bias to positive, +1 more is done by the add below.x_max =62,x_min = -1 (63;0)
	//y						//mantissa bits, allways in [1..2) range as 0 is not a valid input :)
	return (x+y)/64.0f;		//Combine and save the exp + mantissa at the mantissa field.Min value is 0 (-1+1), max value is 63 +(2^18-1)/2(^18).
							//Normalised by 64 so that it falls in the [0..1) range :)
}
float fog_mode2(float invW)
{
	//pixel z* scale, scale is on m1.7es8 format,result is
	//1.m0.7eu3, with clamping (min val = 1<<0 -> 1, max value = 1.1111111<<7=11111111.0 -> 255.0
	//FOG lookup uses idx=eu3:m[6:3] -> 0, .. 127 [128 values pairs]
	//Then it interpolates lerp(FOG[idx][0],FOG[idx][1],0.m[2:0])
	float foginvW=FOG_DENSITY.x*invW;
	foginvW=clamp(foginvW,1,255);

	float fogexp;									//0 ... 7
	float fogman=fdecp(foginvW, fogexp);			//[1,2) mantissa bits. that is 1.m
	
	float fogman_hi=fogman*16-16;					//[16,32) -16 -> [0,16)
	float fogman_idx=floor(fogman_hi);					//[0,15]
	float fogman_blend=frac(fogman_hi);				//[0,1) -- can also be fogman_idx-fogman_idx !
	float fog_idx_fr=fogexp*16+fogman_idx;			//[0,127]
	//D3D9 texture mapping rule : [0.0, 1.0] (0.0 to 1.0, inclusive) to an integer texel space value ranging from [ - 0.5, n - 0.5]
	//0 (0/255) -> 0.5 BEFORE 1st texture pixel (#0)
	//1 (256/256)-> 0.5 AFTER LAST texture pixel (#255)
	//? (0.5/255) -> EXACTLY 1st pixel (#0)
	//? (255.5/256) -> EXACTLY LAST pixel (#255)
	//on an 256x1 texture , pixel 0 is at 0, 1 at 1 .. 255 at 255
	//idx select index *2.idx=0 -> pixel 0(0.5) ,idx=1 -> pixel 2 (2.5), .. idx=127-> pixel 254 (254.5)
	//fraction then blends betwen the idx pixel, and the next pixel
	//I'l use bilinear filter for that work, so idx=127 is  [254.5,255.4999)  -> 254.5 (idx*2+0.5) + blend factor
	// -> actualy, i can use .r and .g to store the cooefs and do the lerp manualy !
	// -> so, [0.5, 127.5]
	float fog_idx_pixel_fr=fog_idx_fr+0.5f;
	float fog_idx_pixel_n=fog_idx_pixel_fr/128;//normalise to [0.5/128,127.5/128) coordinates ;p

	//fog is 128x1 texure
	//ARGB 8888 -> B G R A -> B=7:0 aka '1', G=15:8 aka '0'
	float2 fog_coefs=tex1D(fog_table,fog_idx_pixel_n).rg;
	//frexp(foginvW,out fogexp);	//for exp .. 0 .. 7

	float fog_coef=lerp(fog_coefs.r,fog_coefs.g,fogman_blend);
	
	return fog_coef;
}
struct PSO
{
	float4 col:COLOR0;
	#if ZBufferMode==1
	float  z  :DEPTH;
	#endif
};
//pvr only supports ARGB8888 colors, but they are pre-clamped on the vertex shader (no need to do it here)
PSO PixelShader_main(in pixel s )
{ 
	float4 color=s.col/s.uv.w;
	clip(s.uv.z);
	
	//For non-old ATI cards, fixes noised cars in Jet Grind Radio for example.
	//color=round(color*255)/255;
	
	#if pp_UseAlpha==0
		color.a=1;
	#endif
	
	#if pp_FogCtrl==3
		//color.a=;
		
		color=float4(FOG_COL_RAM.rgb,fog_mode2(s.uv.w));
	#endif
	
	#if pp_Texture==1
		
		//get texture color
		float4 texcol=TextureLookup(s.uv);
		
		//apply modifiers
		#if pp_IgnoreTexA==1
			texcol.a=1;	
		#endif
		
		//OFFSETRGB is allways added after that (if enabled)
		#if pp_ShadInstr==0
			//PIXRGB = TEXRGB + OFFSETRGB
			color.rgb=texcol.rgb;
			//PIXA    = TEXA
			color.a=texcol.a;
		#elif  pp_ShadInstr==1
			//PIXRGB = COLRGB x TEXRGB + OFFSETRGB
			color.rgb*=texcol.rgb;
			//PIXA   = TEXA
			color.a=texcol.a;
		#elif  pp_ShadInstr==2
			//PIXRGB = (TEXRGB x TEXA) + (COLRGB x (1- TEXA) ) + OFFSETRGB
			color.rgb=(texcol.rgb*texcol.a) + (color.rgb * (1-texcol.a));
			//PIXA   = COLA
			//color.a remains the same
		#elif  pp_ShadInstr==3
			//PIXRGB= COLRGB x  TEXRGB + OFFSETRGB
			color.rgb*=texcol.rgb;
			//PIXA   = COLA  x TEXA
			color.a*=texcol.a;
		#endif
	
		//if offset is enabled , add it :)
		#if pp_Offset==1
			float4 offscol=s.offs/s.uv.w;
			color.rgb+=offscol.rgb;
			
			#if pp_FogCtrl==1
				color.rgb=lerp(color.rgb,FOG_COL_VERT.rgb,offscol.a);
			#endif
		#endif
		
	#else
		//we don't realy have anything to do here -- just return the color ...
	#endif
	
	#if pp_FogCtrl==0
		float fog_blend=fog_mode2(s.uv.w);
		
		color.rgb=lerp(color.rgb,FOG_COL_RAM.rgb,fog_blend);
	#endif
	
	PSO rv;
	rv.col=color;
	#if ZBufferMode==1
	rv.z=CompressZ(s.uv.w);
	#endif
	
	return rv; 
}

PSO PixelShader_Z(float4 uv : TEXCOORD0)
{
	PSO rv;
	rv.col=float4(0,0,0,0.5f);
	
	#if ZBufferMode==1
	rv.z=CompressZ(uv.w);
	#endif
	
	return rv;
}

float4 PixelShader_ShadeCol() :COLOR0
{
	return  float4(0,0,0,0.5f);
}

float4 PixelShader_Framebuffer(float4 uv : TEXCOORD0) :COLOR0
{
	//samplr
	return tex2D(samplr,uv.xy);
}