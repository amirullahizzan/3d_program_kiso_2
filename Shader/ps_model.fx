//----------------------------------------------------------------------------
//!	@file	ps_model.fx
//!	@brief	MV1モデルピクセルシェーダー
//----------------------------------------------------------------------------
#include "dxlib_ps.h"

float PI = 3.141592;

//--------------------------------------------------------------
// 定数バッファ
//--------------------------------------------------------------
cbuffer CameraInfo : register(b10)
{
    matrix mat_view_; //!< ビュー行列
    matrix mat_proj_; //!< 投影行列
    float3 eye_position_; //!< カメラの位置
};

TextureCube iblDiffuseTexture : register(t14);
TextureCube iblSpecularTexture : register(t15);

SamplerState iblDiffuseSampler : register(s14);
SamplerState iblSpecularSampler : register(s15);

cbuffer ShadowInfo : register(b9)
{
    matrix mat_light_view_; //!< ライト用のビュー行列
    matrix mat_light_proj_; //!< ライト用の投影行列
};

Texture2D ShadowTexture : register(t9);
SamplerState ShadowSampler : register(s9);

float rand(float2 co)
{
    return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453);
}

static const float W = 11.2;
float3 Uncharted2Tonemap(float3 x)
{
    static const float A = 0.15;
    static const float B = 0.50;
    static const float C = 0.10;
    static const float D = 0.20;
    static const float E = 0.02;
    static const float F = 0.30;
    static const float W = 11.2;

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 ACESFilm(float3 x)
{
    x *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}


float D_GGX_Alpha(float roughness)
{
#if 0
	// call of duty: WWII GGX gloss parameterization method
	// https://www.activision.com/cdn/research/siggraph_2018_opt.pdf
	// provides a wider roughness range
    float gloss = 1.0f - roughness;
    return sqrt(2.0) / (1.0 + 218.0 * gloss);
#else
    float gloss = 1.0 - roughness;
    float denominator = (1.0 + pow(2, 18 * gloss));
    return sqrt(2.0 / denominator);
#endif
}

//---------------------------------------------------------------------------
// Cook-Torrance D項 : Trowbridge-Reitz (GGX)モデル	
//---------------------------------------------------------------------------
float D_GGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness; // α = ラフネスの2乗
//  float alpha = D_GGX_Alpha(roughness * 2);	// x2 をすると近くなる？

    float numerator = alpha * alpha; // 分子
    float denominator = NdotH * NdotH * (alpha * alpha - 1.0) + 1.0; // 分母
    denominator *= denominator * PI;
	
    return numerator / denominator;
}


float2 VogelDiskSampling(int index, int totalSampleCount, float thetaOffset = 0.0)
{
    static const float GOLDEN_ANGLE = 2.4;
    float theta = GOLDEN_ANGLE * float(index) + thetaOffset;
    float radius = sqrt(float(index) + 0.5) / sqrt(float(totalSampleCount));
    float2 offset = radius * float2(cos(theta), sin(theta));

    return offset;
}

//---------------------------------------------------------------------------
// Cook-Torrance G項 : Smith
//---------------------------------------------------------------------------
float G_Smith(float roughness, float NdotV, float NdotL)
{
    float a = max(0.0001, roughness * roughness); // ラフネスの2乗
    float k = a * 0.5;
    float GV = NdotV / (NdotV * (1.0 - k) + k);
    float GL = NdotL / (NdotL * (1.0 - k) + k);
	
    return GV * GL;
}

//---------------------------------------------------------------------------
// Cook-Torrance F項 : Schlickのフレネル近似式
//---------------------------------------------------------------------------
float3 F_Schlick(float3 specularColor, float NdotV)
{
    float3 f0 = max(specularColor, 0.04); // 反射率基準値は4%
    return f0 + (1.0 - f0) * pow(1.0 - NdotV, 5.0);
}


//----------------------------------------------------------------------------
// メイン関数
//----------------------------------------------------------------------------
PS_OUTPUT main(PS_INPUT_MODEL input)
{
    PS_OUTPUT output;

    float2 uv = input.uv0_;
    float3 N = normalize(input.normal_); // 法線
    float3 V = normalize(eye_position_ - input.worldPosition_);
	
	//------------------------------------------------------------
	// 影
	//------------------------------------------------------------
    float shadow = 1.0; // 影の遮蔽項 (0:完全に影 1:影ではない)
    {
        //-------------
        // (1) シャドウ生成時の座標変換を再現する
        float4 light_view_position = mul(float4(input.worldPosition_, 1.0), mat_light_view_);
        float4 light_screen_position = mul(light_view_position, mat_light_proj_);
        
        //-------------
        // (2) シャドウ用のデプスバッファを参照するUV座標を求める
        // Screen空間をUV空間に変更
        float2 shadow_uv = light_screen_position.xy * float2(0.5, -0.5) + 0.5;
        
        //-------------
        // (3) 影判定

#if false
        float shadow_z = ShadowTexture.Sample(ShadowSampler, shadow_uv).r;
        const float SHADOW_BIAS = 0.001;
        if (shadow_z + SHADOW_BIAS < light_screen_position.z)
        {
            shadow = 0;
        }

     //   for (int y = -2; y <= 2; y++)
     //   {
     //       for (int x = -2; x <= 2; x++)
     //       {
     //           float2 uv_offset = float2(x, y) * 0.001;
     //       
     //           float shadow_z = ShadowTexture.Sample(ShadowSampler, shadow_uv + uv_offset).r;
     //           const float SHADOW_BIAS = 0.001 * 4;
     //           if (shadow_z + SHADOW_BIAS < light_screen_position.z)
     //           {
     //               shadow -= 1.0 / 25.0;
     //           }
     //
     //       }
     //   }
#else
        const int SAMPLE_COUNT = 25;
        
        float randomized_radian = rand(input.position_.xy);
        for (int i = 0; i < SAMPLE_COUNT; i++)
        {
            float2 uv_offset = VogelDiskSampling(i, SAMPLE_COUNT, randomized_radian) * 0.001;
            
            float shadow_z = ShadowTexture.Sample(ShadowSampler, shadow_uv + uv_offset).r;
            const float SHADOW_BIAS = 0.001 * 4;
            if (shadow_z + SHADOW_BIAS < light_screen_position.z)
            {
                shadow -= 1.0 / SAMPLE_COUNT;
            }

        }
        
#endif
    }
    //using lambert to cancel out shadow sillhouette / outline
    //影の輪郭を
    float3 LightDir = normalize(float3(1,1,-1)); //Use buffer later to connect light_dir_
    float lambert = saturate(dot(N, LightDir) * 2.4); 
//    shadow = lambert;
    
    shadow = min(shadow,lambert);
    
    
	//------------------------------------------------------------
	// 法線マップ
	//------------------------------------------------------------
    N = Normalmap(N, input.worldPosition_, uv);

	//------------------------------------------------------------
	// テクスチャカラーを読み込み
	//------------------------------------------------------------
    float4 textureColor = DiffuseTexture.Sample(DiffuseSampler, uv);

	// リニア化
    float3 albedo = pow(textureColor.rgb, 2.2);
	
	
    // アルファテスト
    if (textureColor.a < 0.5)
        discard;

//	output.color0_ = textureColor * input.diffuse_;

	
    float roughness = 0.3; // ラフさ            (つるつる) 0.0 ～ 1.0 (ざらざら)
    float metalness = 0.0; // 金属度合い metallic (非金属) 0.0 ～ 1.0 (金属)
	
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);
	
	//----------------------------------------------------------
	// 拡散反射光 Diffuse
	//----------------------------------------------------------
    float3 L = normalize(float3(1, 1, -1)); // 光源の方向 L

	// Lambertモデル
	//    diffuse = max( dot(N, L), 0 )
	//
	// saturate = 数値を0.0～1.0の間に収める。 clamp(x, 0, 1) と同義
	// GPUは saturate と abs は 0サイクル コストなしで実行できる
	
	// 光量を 1/π 倍することでエネルギー保存則を満たす計算になる

    float Kd = 1.0 / PI;
    float diffuse = saturate(dot(N, L)) * Kd;
	
	//----------------------------------------------------------
	// 鏡面反射光 Specular
	//----------------------------------------------------------
    float3 H = normalize(L + V);
	
    float NdotH = saturate(dot(N, H));
	
	
#if 0	
	// Blinn-Phongモデル
	//   specular = pow( saturate( dot(N, H) ), specularPower )
	//   specularPowerは数値を上げると鋭いシルエットになる。
    float	specularPower = 250;
    float	Ks = (specularPower + 9.0) / (PI * 9.0); // 分子と分母を交換して逆数にしている
    float	specular = pow(saturate(dot(N, H)), specularPower) * Ks;
#else
	// Trowbridge-Reitz (GGX)モデル	
    float alpha = roughness * roughness; // α = ラフネスの2乗
    float numerator = alpha * alpha; // 分子
    float denominator = NdotH * NdotH * (alpha * alpha - 1.0) + 1.0; // 分母
    denominator *= denominator * PI;
	
    float3 specular = numerator / denominator;
#endif
	// フレネル反射
	// Schlick の近似式
    float f0 = 0.04; // 4%
    float fresnel = f0 + (1.0 - f0) * pow(1.0 - dot(N, V), 5.0);
	
    float3 color;
    float3 lightColor = float3(1, 1, 1) * 4;
	
	// Diffuse
	//color = (diffuse < 0.1) ? float3(0.7, 0.7, 0.7) : float3(1, 1, 1);
    color = diffuse * albedo * lightColor;

	// Specular
	// CookTorranceモデル
    float NdotL = saturate(dot(N, L));
    float LdotH = saturate(dot(L, H));

#if 1

	// 高速化近似バージョン SIGGRAPH 2015 Optimizing PBR
    float a = roughness * roughness;
    numerator = a * a;
    denominator = NdotH * NdotH * (numerator - 1.0) + 1.0;
    denominator *= denominator * 4.0 * PI * LdotH * LdotH * (roughness + 0.5);
    specular = numerator / denominator * specularColor;
#else

    float D = D_GGX(NdotH, roughness);
    float G = G_Smith(roughness, dot(N, V), NdotL);
    float3 F = F_Schlick(specularColor, dot(N, V));

    specular =              (D * G * F)
				/ //--------------------------------
			        (4.0 * dot(N, V) * NdotL + 0.00001);

    specular *= NdotL;
    
#endif
    color += lightColor * specular; // * specularColor;

    //==============
    // 影
    //==============
   // Define a dark blue color
    float3 darkBlue = float3(0.1, 0.1, 0.2); // Adjust RGB values for the desired shade

// Combine the shadow with the dark blue color
    float3 shadowColor = lerp(darkBlue, float3(1.0, 1.0, 1.0), shadow); // Blend with white based on shadow intensity

// Apply the shadow effect
    color.rgb *= shadowColor;
    
    //----------------------------------------------------------
    // Image Based Lighting	
    //----------------------------------------------------------
    if (1)
    {
        float iblScale = 0.25;
        
        float3 iblDiffuse = iblDiffuseTexture.Sample(iblDiffuseSampler, N).rgb * (1.0 / PI);
        iblDiffuse *= iblScale;
	    // 反射ベクトルRを計算する
	    // R = reflect(入射ベクトル, 法線ベクトル);
        float3 R = reflect(-V, N);
   
        float3 environmentBrdf = pow(1.0 - max(roughness, dot(N, V)), 3.0) + specularColor;
	
        float mip = 8.0 * roughness;
        float3 iblSpecular = iblSpecularTexture.SampleLevel(iblSpecularSampler, R, mip).rgb;
        iblSpecular *= environmentBrdf;
        iblSpecular *= iblScale;
	
        color += iblDiffuse * albedo * (1.0 - metalness) + iblSpecular;
    }
	

	 
    if ((int(input.position_.y) & 7) >> 2 == 0)
    {
//         color *= 0.5;
    }
	
	
	// 輝度が1.0を超えないようにする

#if 0
	// Reinhard Tonemapping
    color.rgb = color.rgb / (color.rgb + 1.0);
#elif 0
	// Uncharted2 Tonemapping
    float ExposureBias = 2.0f;
    float3 curr = Uncharted2Tonemap(ExposureBias * color.rgb);

    float3 whiteScale = 1.0f / Uncharted2Tonemap(W);
    color.rgb = curr * whiteScale;
#elif 1
	// ACES Filmic Tonemapping
    color.rgb = ACESFilm(color.rgb);
#endif
    
    
	// sRGBへ変換
    color.rgb = pow(saturate(color.rgb), 1.0 / 2.2);
	
    output.color0_ = float4(color, 1);

	// 出力パラメータを返す
    return output;
}
