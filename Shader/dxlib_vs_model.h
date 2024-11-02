//---------------------------------------------------------------------------
//!	@file	dxlib_vs_model.h
//! @brief	DxLib用 MV1モデル頂点シェーダー定義
//---------------------------------------------------------------------------
#include "dxlib_vs.h"

//============================================================================
// シェーダーバリエーション
//============================================================================

// MV1モデルの頂点タイプ
#define DX_MV1_VERTEX_TYPE_1FRAME					(0)				// １フレームの影響を受ける頂点
#define DX_MV1_VERTEX_TYPE_4FRAME					(1)				// １～４フレームの影響を受ける頂点
#define DX_MV1_VERTEX_TYPE_8FRAME					(2)				// ５～８フレームの影響を受ける頂点
#define DX_MV1_VERTEX_TYPE_FREE_FRAME				(3)				// ９フレーム以上の影響を受ける頂点
#define DX_MV1_VERTEX_TYPE_NMAP_1FRAME				(4)				// 法線マップ用の情報が含まれる１フレームの影響を受ける頂点
#define DX_MV1_VERTEX_TYPE_NMAP_4FRAME				(5)				// 法線マップ用の情報が含まれる１～４フレームの影響を受ける頂点
#define DX_MV1_VERTEX_TYPE_NMAP_8FRAME				(6)				// 法線マップ用の情報が含まれる５～８フレームの影響を受ける頂点
#define DX_MV1_VERTEX_TYPE_NMAP_FREE_FRAME			(7)				// 法線マップ用の情報が含まれる９フレーム以上の影響を受ける頂点

// 【注意事項】
// 頂点タイプ DX_MV1_VERTEX_TYPE_FREE_FRAME と DX_MV1_VERTEX_TYPE_NMAP_FREE_FRAME は
// CPU側でスキニング計算されているため入力頂点そのものがアニメーション変形しているので注意が必要。

// スキンメッシュあり
#if (SHADER_VARIANT == DX_MV1_VERTEX_TYPE_4FRAME     ) || (SHADER_VARIANT == DX_MV1_VERTEX_TYPE_8FRAME     ) ||	\
	(SHADER_VARIANT == DX_MV1_VERTEX_TYPE_NMAP_4FRAME) || (SHADER_VARIANT == DX_MV1_VERTEX_TYPE_NMAP_8FRAME)
#define USE_SKINMESH
#endif

// 法線マップあり
#if (SHADER_VARIANT >= DX_MV1_VERTEX_TYPE_NMAP_1FRAME)
#define USE_NORMALMAP
#endif

// 8weightスキニングあり
#if (SHADER_VARIANT == DX_MV1_VERTEX_TYPE_8FRAME) || (SHADER_VARIANT == DX_MV1_VERTEX_TYPE_NMAP_8FRAME)
#define USE_BONE8
#endif

// [DxLib] MV1モデルの頂点シェーダー入力
struct VS_INPUT_MV1
{
	float3 position_      : POSITION;		// 座標(ローカル空間)
	float3 normal_        : NORMAL;			// 法線(ローカル空間)
	float4 diffuse_       : COLOR0;			// ディフューズカラー
	float4 specular_      : COLOR1;			// スペキュラカラー
	float4 uv0_           : TEXCOORD0;		// テクスチャ座標
	float4 uv1_           : TEXCOORD1;		// サブテクスチャ座標

	// 法線マップ
#if defined(USE_NORMALMAP)
	float3 tangent_       : TANGENT0;		// 接線  (ローカル空間)
	float3 binormal_      : BINORMAL0;		// 従法線(ローカル空間)
#endif

// スキニングメッシュ
#if defined(USE_SKINMESH)
	int4   blendIndices0_ : BLENDINDICES0;	// ボーン処理用配列インデックス1
	float4 blendWeight0_  : BLENDWEIGHT0;	// ボーン処理用ウエイト値0

	// 8-weight スキニング
#if defined(USE_BONE8)
	int4   blendIndices1_ : BLENDINDICES1;	// ボーン処理用配列インデックス1
	float4 blendWeight1_  : BLENDWEIGHT1;	// ボーン処理用ウエイト値1
#endif
#endif // USE_SKINMESH
};

//---------------------------------------------------------------------------
// [DxLib] ビュー行列を取得
//---------------------------------------------------------------------------
float3x4 DxLib_ViewMatrix()
{
	return DxLib_Base.ViewMatrix;
}

//---------------------------------------------------------------------------
// [DxLib] 投影行列を取得
//---------------------------------------------------------------------------
float4x4 DxLib_ProjectionMatrix()
{
	return DxLib_Base.ProjectionMatrix;
}

//---------------------------------------------------------------------------
// [DxLib] スキニング行列を取得
//!	@param	[in]	blendIndex	行列番号
//! @attention DxLibの頂点形式でインデックスが3の倍数になっているため注意
//---------------------------------------------------------------------------
float3x4	DxLib_getSkinMatrix(in int blendIndex)
{
	float4	m0 = DxLib_LocalWorldMatrix.Matrix[blendIndex + 0];
	float4	m1 = DxLib_LocalWorldMatrix.Matrix[blendIndex + 1];
	float4	m2 = DxLib_LocalWorldMatrix.Matrix[blendIndex + 2];
	return float3x4(m0, m1, m2);
}

//---------------------------------------------------------------------------
// [DxLib] スキニング計算されたワールド行列を取得
//---------------------------------------------------------------------------
float3x4 DxLib_WorldMatrix(in VS_INPUT_MV1 input)
{
	float3x4	matWorld = DxLib_Base.WorldMatrix;

#if defined(USE_SKINMESH)	// スキニングメッシュ

	matWorld =  DxLib_getSkinMatrix(input.blendIndices0_.x) * input.blendWeight0_.x;
	matWorld += DxLib_getSkinMatrix(input.blendIndices0_.y) * input.blendWeight0_.y;
	matWorld += DxLib_getSkinMatrix(input.blendIndices0_.z) * input.blendWeight0_.z;
	matWorld += DxLib_getSkinMatrix(input.blendIndices0_.w) * input.blendWeight0_.w;

#if defined(USE_BONE8)	// 8-weight スキニング

	matWorld += DxLib_getSkinMatrix(input.blendIndices1_.x) * input.blendWeight1_.x;
	matWorld += DxLib_getSkinMatrix(input.blendIndices1_.y) * input.blendWeight1_.y;
	matWorld += DxLib_getSkinMatrix(input.blendIndices1_.z) * input.blendWeight1_.z;
	matWorld += DxLib_getSkinMatrix(input.blendIndices1_.w) * input.blendWeight1_.w;

#endif

#endif // USE_SKINMESH

	return matWorld;
}
