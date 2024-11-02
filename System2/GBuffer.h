//---------------------------------------------------------------------------
//! @file   GBuffer.h
//! @brief  GBuffer管理クラス
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! GBuffer管理クラス
//===========================================================================
class GBuffer : public Object
{
public:
    BP_OBJECT_DECL(GBuffer, u8"GBuffer管理")

    //-------------------------------------------------
    //! @name Object継承クラス
    //-------------------------------------------------
    //!@{

    virtual bool Init() override;      //!< 初期化
    virtual void Update() override;    //!< 更新
    virtual void Exit() override;      //!< 終了
    virtual void GUI() override;       //!< GUI表示

    //!@}

private:
    static constexpr u32     GBUFFER_COUNT = 8;          //!< GBufferの数
    std::shared_ptr<Texture> gbuffer_[GBUFFER_COUNT];    //!< GBufferテクスチャー
};
