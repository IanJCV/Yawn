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
    float3 positionOS : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

struct vs_out
{
    float4 positionCS : SV_Position;
    float4 color : COLOR;
};

vs_out vs_main(vs_in input)
{
    vs_out output = (vs_out) 0;
  
    output.positionCS = mul(mvp, float4(input.positionOS, 1.0));
    output.color = input.color;
    
    return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{    
    return input.color;
}