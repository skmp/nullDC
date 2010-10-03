//-----------------------------------------------------------------------------
// Effect File Variables
//-----------------------------------------------------------------------------

float4   WindowSize:register(c0);
float4   TextureSize:register(c1);


sampler tex:register(s0);
sampler texflt:register(s1);

//-----------------------------------------------------------------------------
// Vertex Definitions
//-----------------------------------------------------------------------------

struct VS_INPUT 
{
  float4 position : POSITION;
  float4 color	  : COLOR0;
  float2 texcoord : TEXCOORD0;
};

struct VS_OUTPUT 
{
  float4 hposition : POSITION;
  float4 color	   : COLOR0;
  float2 texcoord  : TEXCOORD0;
  float2 position  : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// Simple Vertex Shader
//-----------------------------------------------------------------------------

VS_OUTPUT VertexMain(VS_INPUT inp) 
{
  VS_OUTPUT outp;
  
  outp.hposition.xy=(inp.position.xy-WindowSize.zw+float2(0.5,0.5))*WindowSize.xy;
  outp.hposition.zw=inp.position.zw;
  outp.position=inp.position.xy;
  outp.color=inp.color;
  outp.texcoord=inp.texcoord;

  return outp;
}

//-----------------------------------------------------------------------------
// Simple Pixel Shaders
//-----------------------------------------------------------------------------

float4 ps_DrawRGBA(VS_OUTPUT inp) : COLOR
{
  float4  rv = tex2D(texflt, inp.texcoord)*inp.color;

  return rv;
}

float4 ps_DrawRGB(VS_OUTPUT inp) : COLOR
{
  float4  rv = tex2D(texflt, inp.texcoord)*inp.color;
          rv.a = inp.color.a;

  return rv;
}

float4 ps_DrawA(VS_OUTPUT inp) : COLOR
{
  float4  rv = inp.color;
  rv.a*=tex2D(texflt, inp.texcoord).a;

  return rv;
}

float4 ps_DrawFBz(VS_OUTPUT inp) : COLOR
{
  float4  rv = tex2D(texflt, inp.texcoord)*inp.color;
          rv.a = inp.color.a;

  return rv;
}

float4 GetSubSum(float2 tc,float x,float y)
{
	tc+=float2(x/640,y/480);
	return min(tex2D(texflt, tc),float4(1.2,1.2,1.2,1.2))*0.2f;
}
//AA!
float4 ps_DrawFB(VS_OUTPUT inp) : COLOR
{
  float2 tc= inp.texcoord;
  float4  rv = smoothstep(float4(0,0,0,0),float4(1,1,1,1),tex2D(texflt, inp.texcoord));
  
  rv.a = inp.color.a;

  return rv;
}