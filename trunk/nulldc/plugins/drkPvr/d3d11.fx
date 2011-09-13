
struct VS_INPUT
{
    float3 pos : POS;
	float4 col0 : COL0;
	float4 col1 : COL1;
    float2 uv : UV;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col0 : COLOR0;
	float4 col1 : COLOR1;
	float3 uv : TEXCOORD0;
};


// W/perspective is done in PS ...
PS_INPUT VS( VS_INPUT vi )
{
    PS_INPUT vo = (PS_INPUT)0;
    vo.pos.xyz = vi.pos;
    

	vo.pos.xy+=float2(-320,-240);
	vo.pos.xy/=float2(320,-240);
	
	vo.col0=vi.col0*vi.pos.z;
	vo.col1=vi.col1*vi.pos.z;
	vo.uv.xy=vi.uv*vi.pos.z;
	vo.uv.z=vo.pos.z;

	vo.pos.z/=65536;
	vo.pos.w=1;

    return vo;
}


float4 PS( PS_INPUT s) : SV_Target
{
		
    return s.col0/s.uv.z;
}