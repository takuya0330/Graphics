RaytracingAccelerationStructure g_scene : register(t0);
RWTexture2D<float4> g_output : register(u0);

struct Payload
{
    float3 color;
};

struct Attribute
{
    float2 barys;
};

[shader("raygeneration")]
void MainRGS()
{
    uint2 index = DispatchRaysIndex().xy;
    float2 dimentions = float2(DispatchRaysDimensions().xy);
    float2 screen = (index + 0.5f) / dimentions * 2.0f - 1.0f;

    RayDesc ray_desc;
    ray_desc.Origin = float3(screen.x, -screen.y, 1);
    ray_desc.Direction = float3(0, 0, -1);
    ray_desc.TMin = 0;
    ray_desc.TMax = 100000;

    Payload payload;
    payload.color = 0;

    RAY_FLAG ray_flag = RAY_FLAG_NONE;
    uint ray_mask = 0xFF;
    
    TraceRay(g_scene, ray_flag, ray_mask, 0, 1, 0, ray_desc, payload);

    float3 color = payload.color;
    g_output[index] = float4(color, 1);
}

[shader("miss")]
void MainMS(inout Payload payload)
{
    payload.color = float3(0.2f, 0.2f, 0.2f);
}

[shader("closesthit")]
void MainCHS(inout Payload payload, in Attribute attr)
{
    float3 color = 0;
    color.xy = attr.barys;
    color.z = 1.0f - attr.barys.x - attr.barys.y;
    payload.color = color;
}
