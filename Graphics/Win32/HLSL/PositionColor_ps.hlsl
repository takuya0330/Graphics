struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(VS_OUT pin) : SV_TARGET
{
    return pin.color;
}
