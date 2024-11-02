//----------------------------------------------------------------------------
//!	@file	ps_filter_fade.fx
//!	@brief	フェードフィルター(モザイクエフェクトつき)
//----------------------------------------------------------------------------
#include "dxlib_ps.h"

// DxLibでは定数バッファはb4以降が自由に利用できる
cbuffer FilterFadeParameter : register(b4)
{
    float4	resolution_;	//!< 解像度 [幅, 高さ, 1.0f/幅, 1.0f/高さ]
    float	alpha_;			//!< フェードアルファ値(0.0f-1.0f)
    uint	mosaicWidth_;	//!< モザイクのピクセル幅
};


//----------------------------------------------------------------------------
// メイン関数
//----------------------------------------------------------------------------
PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT	output;

	// ピクセル位置
	uint2 position = uint2(input.position_.xy);
	
	//-----------------------------------------------------------
	// 軽量モザイク加工
	//-----------------------------------------------------------
	position -= position % mosaicWidth_;
	position += mosaicWidth_ / 2;		// ブロックをセンタリング

	// テクスチャカラーの読み込み
	float2	rcpResolution = resolution_.zw;		// 解像度の逆数
	float2	uv = float2(position) * rcpResolution;
	float4	color = DiffuseTexture.Sample(DiffuseSampler, uv);

	// フェードアウトで暗くする
	color.rgb *= alpha_;

	output.color0_ = color;

	// 出力パラメータを返す
	return output;
}
