struct VS_IN
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

VS_OUT main(VS_IN vin)
{
    VS_OUT vout = (VS_OUT) 0;
    
    vout.position = float4(vin.position, 1);
    vout.texcoord = vin.texcoord;

    return vout;
}
