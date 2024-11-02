//---------------------------------------------------------------------------
//!	@file	dxlib_ps.h
//! @brief	DxLib用ピクセルシェーダー定義
//---------------------------------------------------------------------------

//===========================================================================
// ピクセルシェーダーの入力(2D)
//===========================================================================
struct PS_INPUT
{
	float4	position_      : SV_Position;
	float4	diffuse_       : COLOR0;			// Diffuseカラー

#if DXLIB_VERSION >= 0x324d	// [DxLib] Ver 3.24dから仕様が変更されている
	float4	specular_      : COLOR1;			// Specularカラー
	float2	uv0_           : TEXCOORD0;
	float2	uv1_           : TEXCOORD1;
#else
	float2	uv0_           : TEXCOORD0;
#endif
};

//===========================================================================
// ピクセルシェーダーの入力(3D)
//===========================================================================
struct PS_INPUT_3D
{
	float4	position_      : SV_Position;
	float3	worldPosition_ : WORLD_POSITION;	// ワールド座標
	float3	normal_        : NORMAL0;			// 法線
	float4	diffuse_       : COLOR0;			// Diffuseカラー
	float2	uv0_           : TEXCOORD0;			// テクスチャ座標
};

//===========================================================================
// ピクセルシェーダーの入力(3Dモデル)
//===========================================================================
struct PS_INPUT_MODEL
{
	float4	position_      : SV_Position;		//!< 座標       (スクリーン空間)
    float4  currPosition_  : CURR_POSITION;    //!< 現在の座標 (スクリーン空間)
	float3	worldPosition_ : WORLD_POSITION;	//!< ワールド座標
	float3	normal_        : NORMAL0;			//!< 法線
	float4	diffuse_       : COLOR0;			//!< Diffuseカラー
	float2	uv0_           : TEXCOORD0;		//!< テクスチャ座標
	//--------
	float4	prevPosition_  : PREV_POSITION;	//! 1フレーム前の座標 (スクリーン空間) GSで合成される
};

//===========================================================================
// ピクセルシェーダーの出力
//===========================================================================
struct PS_OUTPUT
{
	float4	color0_ : SV_Target0;	// 色
};


// [DxLib] マテリアルパラメータ
struct DX_D3D11_CONST_MATERIAL
{
	float4		Diffuse;				// ディフューズカラー
	float4		Specular;				// スペキュラカラー
	float4		Ambient_Emissive;		// マテリアルエミッシブカラー + マテリアルアンビエントカラー * グローバルアンビエントカラー

	float		Power;					// スペキュラの強さ
	float		TypeParam0;				// マテリアルタイプパラメータ0
	float		TypeParam1;				// マテリアルタイプパラメータ1
	float		TypeParam2;				// マテリアルタイプパラメータ2
};

// [DxLib] フォグパラメータ
struct DX_D3D11_VS_CONST_FOG
{
	float		LinearAdd;				// フォグ用パラメータ end / ( end - start )
	float		LinearDiv;				// フォグ用パラメータ -1  / ( end - start )
	float		Density;				// フォグ用パラメータ density
	float		E;						// フォグ用パラメータ 自然対数の低

	float4		Color;					// カラー
};

// [DxLib] ライトパラメータ
struct DX_D3D11_CONST_LIGHT
{
	int			Type;					// ライトタイプ( DX_LIGHTTYPE_POINT など )
	int3		Padding1;				// パディング１

	float3		Position;				// 座標( ビュー空間 )
	float		RangePow2;				// 有効距離の２乗

	float3		Direction;				// 方向( ビュー空間 )
	float		FallOff;				// スポットライト用FallOff

	float3		Diffuse;				// ディフューズカラー
	float		SpotParam0;				// スポットライト用パラメータ０( cos( Phi / 2.0f ) )

	float3		Specular;				// スペキュラカラー
	float		SpotParam1;				// スポットライト用パラメータ１( 1.0f / ( cos( Theta / 2.0f ) - cos( Phi / 2.0f ) ) )

	float4		Ambient;				// アンビエントカラーとマテリアルのアンビエントカラーを乗算したもの

	float		Attenuation0;			// 距離による減衰処理用パラメータ０
	float		Attenuation1;			// 距離による減衰処理用パラメータ１
	float		Attenuation2;			// 距離による減衰処理用パラメータ２
	float		Padding2;				// パディング２
};

// [DxLib] ピクセルシェーダー・頂点シェーダー共通パラメータ
struct DX_D3D11_CONST_BUFFER_COMMON
{
	DX_D3D11_CONST_LIGHT	Light[6];	// ライトパラメータ
	DX_D3D11_CONST_MATERIAL	Material;	// マテリアルパラメータ
	DX_D3D11_VS_CONST_FOG	Fog;		// フォグパラメータ
};

// [DxLib] 定数バッファピクセルシェーダー基本パラメータ
struct DX_D3D11_PS_CONST_BUFFER_BASE
{
	float4		factorColor_;			// アルファ値等

	float		preMultipliedAlpha_;	// カラーにアルファ値を乗算するかどうか(0.0f:乗算しない  1.0f:乗算する)
	float		alphaTestRef_;			// アルファテストで使用する比較値
	float2		padding0_;

	int			alphaTestCmpMode_;		// アルファテスト比較モード(DX_CMP_NEVER など)
	int3		padding1_;

	float4		IgnoreTextureColor;		// テクスチャカラー無視処理用カラー
};

//===========================================================================
//	定数バッファ
//===========================================================================

// 頂点シェーダー・ピクセルシェーダー共通パラメータ
cbuffer cbD3D11_CONST_BUFFER_COMMON : register(b0)
{
	DX_D3D11_CONST_BUFFER_COMMON	DxLib_Common;
};

// 基本パラメータ
cbuffer cbD3D11_CONST_BUFFER_PS_BASE : register(b1)
{
	DX_D3D11_PS_CONST_BUFFER_BASE	DxLib_Base;
};

//===========================================================================
//	テクスチャ
//===========================================================================
SamplerState	DiffuseSampler     : register(s0);	// Diffuseテクスチャサンプラー
Texture2D		DiffuseTexture     : register(t0);	// Diffuseテクスチャ


SamplerState	NormalSampler   : register(s1);	// 法線マップテクスチャ
Texture2D		NormalTexture   : register(t1);	// 法線マップテクスチャ

SamplerState	SpecularMapSampler : register(s2);	// スペキュラマップテクスチャ
Texture2D		SpecularMapTexture : register(t2);	// スペキュラマップテクスチャ

// t3～t6はトゥーン用テクスチャ

SamplerState	SubSampler         : register(s7);	// サブテクスチャ
Texture2D		SubTexture         : register(t7);	// サブテクスチャ

SamplerState	ShadowMap0Sampler  : register(s8);	// シャドウマップ０テクスチャ
Texture2D		ShadowMap0Texture  : register(t8);	// シャドウマップ０テクスチャ

SamplerState	ShadowMap1Sampler  : register(s9);	// シャドウマップ１テクスチャ
Texture2D		ShadowMap1Texture  : register(t9);	// シャドウマップ１テクスチャ

SamplerState	ShadowMap2Sampler  : register(s10);	// シャドウマップ２テクスチャ
Texture2D		ShadowMap2Texture  : register(t10);	// シャドウマップ２テクスチャ


//===========================================================================
//	PixelShader ユーティリティー
//===========================================================================

//----------------------------------------------------------------------------
//!	Tangent/Binormalを必要としない接空間変換行列作成
//!	@param	[in]	N	法線
//!	@param	[in]	p	ワールド座標
//!	@param	[in]	uv	テクスチャ座標
//----------------------------------------------------------------------------
float3x3 calcCotangentFrame(float3 N, float3 p, float2 uv)
{
	// 隣接ピクセルの勾配を取得
	float3	dp1  = ddx(p);
	float3	dp2  = ddy(p);
	float2	duv1 = ddx(uv);
	float2	duv2 = ddy(uv);

	float3	dp1_p = cross(N, dp1);
	float3	dp2_p = cross(dp2, N);

	// 連立一次方程式を解いてTangentとBinormalを求める
	float3	T = dp2_p * duv1.x + dp1_p * duv2.x;
	float3	B = dp2_p * duv1.y + dp1_p * duv2.y;

	float	inv_max = rsqrt( max(dot(T,T), dot(B,B)) );
	return float3x3(T * inv_max, B * inv_max, N);
}

//----------------------------------------------------------------------------
//!	法線マッピングを適用します
//!	@param	[in]	N	法線
//!	@param	[in]	p	ワールド座標
//!	@param	[in]	uv	テクスチャ座標
//----------------------------------------------------------------------------
float3 Normalmap(float3 N, float3 p, float2 uv)
{
	// 法線マップテクスチャを読み込み
	float3	texture_normal = NormalTexture.Sample(NormalSampler, uv).rgb;
	// デコード
	texture_normal = texture_normal * 2 - 1;

	// タンジェント空間を変換
	float3x3	TBN = calcCotangentFrame(N, p, uv);
	return normalize( mul( texture_normal, TBN) );
}
