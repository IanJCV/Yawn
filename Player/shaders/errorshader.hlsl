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
    float3 positionWS : POSITION1;
};

vs_out vs_main(vs_in input)
{
    vs_out output = (vs_out) 0;
  
    output.positionCS = mul(mvp, float4(input.positionOS, 1.0));
    output.positionWS = mul(model, float4(input.positionOS, 1.0));

    return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    float3 magenta = float3(1, 0, 1);
    float3 black = float3(0, 0, 0);
    
    float3 uv = floor(2 * input.positionWS / 2);
    float a = abs((uv.x + uv.y + uv.z) % 2);
    return float4(magenta * a, 1);
    
    return float4(lerp(magenta, black, distance(input.positionWS, float3(0, 0, 0)) % 2), 1);
}