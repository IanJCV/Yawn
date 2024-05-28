cbuffer constants : register(b0)
{
    float4x4 model;
    float4x4 mvp;
    float4 col;
};

struct vs_in
{
    float3 positionWS : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};


struct vs_out
{
    float4 clipPos : SV_Position;
    float3 normal : NORMAL;
    float3 normalWS : NORMAL1;
    float4 color : COLOR;
    float4 color2 : COLOR2;
    float2 texcoord : TEXCOORD;
    float orient : VFACE;
};

vs_out vs_main(vs_in input)
{
  vs_out output = (vs_out)0;
  
  output.clipPos = mul(mvp, float4(input.positionWS * 0.2, 1.0));
  output.color = input.color;
  output.color2 = col;
  output.normal = input.normal;
  output.normal = mul(model, input.normal);
  output.texcoord = input.texcoord;

  return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    float ndotl = pow(saturate(dot(input.normal, float3(-0.5, 0.5, 0))), 1);
    float ambient = 0.15;
    float4 light = max(ndotl * input.color2, ambient.rrrr);
    return float4(light);
}