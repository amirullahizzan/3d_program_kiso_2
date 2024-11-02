//---------------------------------------------------------------------------
//!	@file	dxlib_vs.h
//! @brief	DxLib用頂点シェーダー定義
//---------------------------------------------------------------------------

//===========================================================================
// [DxLib] 3Dプリミティブの頂点シェーダー入力
//===========================================================================
struct VS_INPUT_3D
{
	float3	position_  : POSITION0;		// 座標  (ローカル空間)
	float4	position1_ : POSITION1;		// 予備座標
	float3	normal_    : NORMAL0;		// 法線  (ローカル空間)
	float3	tangent_   : TANGENT0;		// 接線  (ローカル空間)
	float3	binormal_  : BINORMAL0;		// 従法線(ローカル空間)
	float4	diffuse_   : COLOR0;		// Diffuseカラー
	float4	specular_  : COLOR1;		// Specularカラー
	float2	uv0_       : TEXCOORD0;		// テクスチャ座標
	float2	uv1_       : TEXCOORD1;		// サブテクスチャ座標
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

// [DxLib] 基本パラメータ
struct DX_D3D11_VS_CONST_BUFFER_BASE
{
	float4x4	AntiViewportMatrix;		// アンチビューポート行列

	float4x4	ProjectionMatrix;		// 投影行列
	float3x4	ViewMatrix;				// ビュー行列
	float3x4	WorldMatrix;			// ワールド行列

	float4		ToonOutLineSize;		// トゥーンの輪郭線の大きさ
	float		DiffuseSource;			// ディフューズカラー( 0.0f:マテリアル  1.0f:頂点 )
	float		SpecularSource;			// スペキュラカラー  ( 0.0f:マテリアル  1.0f:頂点 )
	float		MulSpecularColor;		// スペキュラカラー値に乗算する値( スペキュラ無効処理で使用 )
	float		Padding;
};

// [DxLib] その他の行列
struct DX_D3D11_VS_CONST_BUFFER_OTHERMATRIX
{
	float4		ShadowMapLightViewProjectionMatrix[3][4];	// シャドウマップ用のライトビュー行列とライト射影行列を乗算したもの
	float4		TextureMatrix[3][2];						// テクスチャ座標操作用行列
};

// [DxLib] スキニングメッシュ用のワールド行列
struct DX_D3D11_VS_CONST_BUFFER_LOCALWORLDMATRIX {
	float4		Matrix[54 * 3];								// ワールド行列3x4
};

// 定数バッファはDxLibがb0～b3を使用しています。

//===========================================================================
// 定数バッファ
//===========================================================================

// 頂点シェーダー・ピクセルシェーダー共通パラメータ
cbuffer cbD3D11_CONST_BUFFER_COMMON : register(b0)
{
	DX_D3D11_CONST_BUFFER_COMMON	DxLib_Common;
};

// 基本パラメータ
cbuffer cbD3D11_CONST_BUFFER_VS_BASE : register(b1)
{
	DX_D3D11_VS_CONST_BUFFER_BASE	DxLib_Base;
};

// その他の行列
cbuffer cbD3D11_CONST_BUFFER_VS_OTHERMATRIX : register(b2)
{
	DX_D3D11_VS_CONST_BUFFER_OTHERMATRIX	DxLib_OtherMatrix;
};

// スキニングメッシュ用のローカルワールド行列
cbuffer cbD3D11_CONST_BUFFER_VS_LOCALWORLDMATRIX : register(b3)
{
	DX_D3D11_VS_CONST_BUFFER_LOCALWORLDMATRIX	DxLib_LocalWorldMatrix;
};
