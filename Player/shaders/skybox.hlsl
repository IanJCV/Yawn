TextureCube cubeTexture : register(t0);
SamplerState sampler_cubeTexture : register(s0);

cbuffer constants : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float4x4 mvp;
    float4 col;
    float3 viewPos;
    float3 viewDir;

    float2 pad;
};


struct vs_in
{
    float3 positionWS : SV_Position;
};

struct vs_out
{
    float4 positionCS : SV_Position;
    float3 texcoords : POSITION1;
};

vs_out vs_main(vs_in input)
{
    vs_out output = (vs_out) 0;
  
    output.positionCS = mul(mul(projection, view), float4(input.positionWS * 100000, 1.0)).xyww;
    output.texcoords = input.positionWS;
    
    return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{       
    float4 col = cubeTexture.Sample(sampler_cubeTexture, input.texcoords);
    return col;
}