cbuffer constants : register(b0)
{
    float4x4 model;
    float4x4 mvp;
    float4 col;
    float3 viewPos;

    float pad;
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
    float4 positionCS : SV_Position;
    float3 positionWS : POSITION1;
    float3 normal : NORMAL;
    float3 normalWS : NORMAL1;
    float4 color : COLOR;
    float4 color2 : COLOR2;
    float2 texcoord : TEXCOORD0;
    float3 cameraPos : TEXCOORD1;
    float orient : VFACE;
};

vs_out vs_main(vs_in input)
{
  vs_out output = (vs_out)0;
  
  output.positionCS = mul(mvp, float4(input.positionWS, 1.0));
  output.color = input.color;
  output.color2 = col;
  output.normal = input.normal;
  output.normalWS = mul(model, input.normal);
  output.texcoord = input.texcoord;
  output.cameraPos = viewPos;

  return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    float ndotl = pow(saturate(dot(input.normal, float3(-0.5, 0.5, 0))), 0.2);
    float ambient = 0.15;
    float3 light = max(ndotl * input.color.rgb, ambient.rrr);
    float fresnel = saturate(dot(input.positionWS - input.cameraPos, input.normalWS));
    fresnel = pow(fresnel, 0.1);
    return float4(light + (fresnel * float3(1.0, 0.0, 0.0)), 1.0);
}