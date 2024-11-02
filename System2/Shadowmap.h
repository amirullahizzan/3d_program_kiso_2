//---------------------------------------------------------------------------
//! @file   Shadowmap.h
//! @brief  シャドウマップ管理クラス
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! シャドウマップ管理クラス
//===========================================================================
class Shadowmap : public Object
{
public:
    BP_OBJECT_DECL(Shadowmap, u8"シャドウマップ管理")

    //-------------------------------------------------
    //! @name Object継承クラス
    //-------------------------------------------------
    //!@{

    virtual bool Init() override;      //!< 初期化
    virtual void Update() override;    //!< 更新
    virtual void Exit() override;      //!< 終了
    virtual void GUI() override;       //!< GUI表示

    //!@}

    //! シャドウ描画開始
    void begin();

    //! シャドウ描画終了
    void end();

    //! シャドウテクスチャを取得
    static Texture* getShadowTexture();

private:
    u32    shadow_resolution_ = 2048 * 4;                                   //!< シャドウ解像度
    float3 light_dir_         = normalize(float3(1.0f, 1.0f, -1.0f));    // 光源の方向

    static inline std::shared_ptr<Texture> shadowmap_depth_;    //!< シャドウマップ用デプステクスチャー
    static inline std::shared_ptr<Texture> shadowmap_color_;    //!< シャドウマップ用カラーテクスチャー

    int cb_shadow_info_ = -1;    //!< [DxLib] シャドウ用定数バッファ
};
