//---------------------------------------------------------------------------
//! @file   Shadowmap.cpp
//! @brief  シャドウマップ管理クラス
//---------------------------------------------------------------------------
#include "Shadowmap.h"

struct ShadowInfo {
    matrix mat_light_view_;
    matrix mat_light_proj_;
};

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool Shadowmap::Init()
{
    //----------------------------------------------------------
    // シャドウマップテクスチャを作成
    //----------------------------------------------------------
    u32 resolution   = shadow_resolution_;
    shadowmap_depth_ = std::make_shared<Texture>(resolution, resolution, DXGI_FORMAT_D32_FLOAT);
    shadowmap_color_ = std::make_shared<Texture>(resolution, resolution, DXGI_FORMAT_R32_FLOAT);

    //----------------------------------------------------------
    // シャドウマップのバッファへの描画初期化を登録
    //----------------------------------------------------------
    auto beginShadow = [this]() { begin(); };
    auto endShadow   = [this]() { end(); };

    SetProc("Shadowmap::begin", beginShadow, ProcTiming::Shadow, ProcPriority::HIGHEST);
    SetProc("Shadowmap::end", endShadow, ProcTiming::Shadow, ProcPriority::LOWEST);

    //----------------------------------------------------------
    // 定数バッファを作成
    //----------------------------------------------------------
    cb_shadow_info_ = DxLib::CreateShaderConstantBuffer(sizeof(ShadowInfo));

    return Super::Init();
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void Shadowmap::Update()
{
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void Shadowmap::Exit()
{
    // 定数バッファを解放
    DxLib::DeleteShaderConstantBuffer(cb_shadow_info_);
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void Shadowmap::GUI()
{
    Super::GUI();
}

//---------------------------------------------------------------------------
//! シャドウ描画開始
//---------------------------------------------------------------------------
void Shadowmap::begin()
{
    //----------------------------------------------------------
    // シャドウ用のデプスバッファを設定
    //----------------------------------------------------------
    // 描画先をシャドウバッファに変更
    // ※DxLibの仕様でカラーバッファとデプスバッファを両方設定しておかなければならない
    //   カラーバッファはダミーで設定しておく
    SetRenderTarget(shadowmap_color_.get(), shadowmap_depth_.get());

    // デプスバッファをクリア
    ClearDepth(1.0f);    // 1.0f = 無限遠

    //----------------------------------------------------------
    // カメラを光源位置から光線方向に向いて設定
    //----------------------------------------------------------
    float range  = 50.0f * 2;     // 影を撮影する範囲 (±20.0m)
    float height = 100.0f;    // カメラの高度 (100.0m)

    ////////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////////
    float3 center_position = float3(0.0f, 0.0f, 0.0f);    // カメラのLookAt

    auto cameraWk = Scene::GetCurrentCamera();
    auto camera   = cameraWk.lock();
    if(camera) {
        float3 cameraPos      = camera->GetPosition();    //shadow camera look at is updated into the main camera's back
        float3 camera_look_at = camera->GetTarget();

        float3 front = normalize(camera_look_at - cameraPos);

        //center_position = camera_look_at;
        center_position = cameraPos + front * range;
    }

    // ビュー行列 (方向と位置)
    float3 look_at  = center_position;
    float3 position = look_at + light_dir_ * height;    // 高さぶん、上に移動した場所

    matrix mat_view = matrix::lookAtLH(position, look_at);

    // 投影行列 (影の範囲)
    // 平行投影
    //              /       LIGHT       /
    //             /<-range--●-------->/----
    //            /         /position /   ^
    //           /         /         /    | height
    //          /         /         /     |
    //         /         /         /      v
    // -------/=========v=========/---------
    //       /      look_at      /        ↓
    //      /                   /    height_margin
    //
    float height_margin = 100.0f;    // 奥行に余分に延ばす距離

    float scale_x = 1.0f / range;
    float scale_y = 1.0f / range;
    float scale_z = 1.0f / (height + height_margin);    // 奥行は十分に奥まで地面に刺さる程度の距離があれば問題ない

    matrix mat_proj = matrix::scale(float3(1.0f / range, 1.0f / range, 1.0f / (height + height_margin)));

    // 設定
    DxLib::SetCameraViewMatrix(mat_view);
    DxLib::SetupCamera_ProjectionMatrix(mat_proj);

    // 定数バッファに反映
    {
        void* p = DxLib::GetBufferShaderConstantBuffer(cb_shadow_info_);
        {
            auto* info            = reinterpret_cast<ShadowInfo*>(p);
            info->mat_light_view_ = mat_view;
            info->mat_light_proj_ = mat_proj;
        }

        // 定数バッファワークメモリをGPU側へ転送
        DxLib::UpdateShaderConstantBuffer(cb_shadow_info_);
    }
}

//---------------------------------------------------------------------------
//! シャドウ描画終了
//---------------------------------------------------------------------------
void Shadowmap::end()
{
    //----------------------------------------------------------
    // [DxLib] デプスバッファをカラーバッファにコピーする
    //----------------------------------------------------------
    // DxLibの仕様でデプスバッファをテクスチャとして利用できない仕様がある
    // 但し、DirectX11のAPIでデプスバッファ→カラーバッファへコピーは出来る
    // コピー先のカラーバッファはDxLibで利用できる
    {
        auto* d3d_context = GetD3DDeviceContext();    // DirectX11の描画コマンドを登録

        auto* src_texture = shadowmap_depth_->d3dResource();
        auto* dst_texture = shadowmap_color_->d3dResource();

        // コピー実行
        // - 解像度が同じであること
        // - フォーマットが同じであること
        d3d_context->CopyResource(dst_texture, src_texture);
    }

    //----------------------------------------------------------
    // RenderTargetを元に戻す
    //----------------------------------------------------------
    SetRenderTarget(GetHdrBuffer(), GetDepthStencil());

    // 定数バッファを設定 (b9)
    DxLib::SetShaderConstantBuffer(cb_shadow_info_, DX_SHADERTYPE_PIXEL, 9);
}

//---------------------------------------------------------------------------
//! シャドウテクスチャを取得
//---------------------------------------------------------------------------
Texture* Shadowmap::getShadowTexture()
{
    return shadowmap_color_.get();
}
