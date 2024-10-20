struct VS_IN
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct VS_CB
{
    matrix world;
    matrix view;
    matrix proj;
};
ConstantBuffer<VS_CB> cb : register(b0);

VS_OUT main(VS_IN vin)
{
    VS_OUT vout = (VS_OUT) 0;

    float4 wpos = mul(cb.world, float4(vin.position, 1));
    float4 vpos = mul(cb.view, wpos);
    vout.position = mul(cb.proj, vpos);
    vout.color = vin.color;

    return vout;
}
