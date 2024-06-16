struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

Texture2D image : register(t0);
SamplerState linear_sampler : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    return image.Sample(linear_sampler, pin.texcoord);
}
