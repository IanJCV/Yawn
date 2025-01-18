Texture2D diffuse1 : register(t0);
SamplerState sampler_diffuse1 : register(s0);

TextureCube skybox : register(t1);

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
    float3 cameraPos : POSITION2;
    float3 cameraDir : POSITION3;
    float3 normal : NORMAL;
    float3 normalWS : NORMAL1;
    float4 color : COLOR;
    float4 color2 : COLOR2;
    float2 texcoord : TEXCOORD0;
    float orient : VFACE;
};

vs_out vs_main(vs_in input)
{
    vs_out output = (vs_out) 0;
  
    output.positionCS = mul(mvp, float4(input.positionOS, 1.0));
    output.positionWS = mul(model, float4(input.positionOS, 1.0));
    output.normalWS = mul(model, float4(input.normal, 1.0));
    output.cameraPos = viewPos;
    output.cameraDir = viewDir;
    output.color = input.color;
    output.color2 = col;
    output.normal = input.normal;
    output.texcoord = input.texcoord;
    
    return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    float ndotl = pow(saturate(dot(input.normal, float3(-0.5, 0.5, 0))), 1);
    float ambient = 0.15;
    float3 light = max(ndotl, ambient.rrr);
    
    float3 cameraDir = normalize(input.positionWS - input.cameraPos);
    float bias = -0.8;
    float scale = 2;
    float power = 1.25;
    float fresnel = max(0, min(1, bias + scale * (1 + dot(cameraDir, input.normalWS) * power)));
    float4 tex = diffuse1.Sample(sampler_diffuse1, input.texcoord);
    
    float3 reflection = reflect(cameraDir, input.normalWS);
    
    float3 finalLight = light;
    float3 finalReflection = (skybox.Sample(sampler_diffuse1, reflection).rgb * fresnel * 0.25);
    
    return float4(tex.rgb * (finalLight + finalReflection), 1);
}