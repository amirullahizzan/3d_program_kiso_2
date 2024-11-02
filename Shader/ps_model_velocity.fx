//----------------------------------------------------------------------------
//!	@file	ps_model_velocity.fx
//!	@brief	速度バッファを生成 ピクセルシェーダー
//----------------------------------------------------------------------------
#include "dxlib_ps.h"

//----------------------------------------------------------------------------
// メイン関数
//----------------------------------------------------------------------------
PS_OUTPUT main(PS_INPUT_MODEL input)
{
	PS_OUTPUT	output;

	//----------------------------------------------------------
	// スクリーン座標を計算するためにWで除算
	// -1～+1 のスクリーン空間の座標を求める
	//----------------------------------------------------------
	float2	curr_position = input.currPosition_.xy / input.currPosition_.w;
	float2	prev_position = input.prevPosition_.xy / input.prevPosition_.w;

	// UV空間の移動量を計算
	float2	velocity = (curr_position - prev_position) * float2(0.5, -0.5);

	// カラーで縦横の速度成分に着色して出力
	output.color0_.rg = velocity * 100;
	output.color0_.b  = 0.0;
	output.color0_.a  = 1.0;

	// 出力パラメータを返す
	return output;
}
