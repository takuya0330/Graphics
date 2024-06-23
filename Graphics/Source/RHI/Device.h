#pragma once

#include "ForwardDeclaration.h"
#include "Platform.h"

namespace RHI {

//! \brief デバイス作成情報
struct DeviceDesc
{
};

//! \brief プラットフォーム固有のデバイス
struct RawDevice
{
#if _D3D11
	MSWRL::ComPtr<ID3D11Device> d3d11_device;
#elif _D3D12
	MSWRL::ComPtr<ID3D12Device> d3d12_device;
#else
#error Not supported.
#endif
};

//! \brief デバイス基底クラス
class IDevice
{
public:
	virtual ~IDevice() = default;

	//! \brief デバイスクラスが管理するオブジェクトを解放します
	//!
	//! \param[in] object オブジェクト
	//!
	//! \note この関数は IDeviceObject::Release() で参照カウントが 0 のときに呼び出されます
	virtual void Release(IDeviceObject* object) = 0;

	//! \brief コマンドバッファを作成します
	//!
	//! \param[in] command_buffer_desc コマンドバッファ作成情報
	//! \param[out] command_buffer コマンドバッファ
	//!
	//! \retval true 作成成功
	//! \retval false 作成失敗
	virtual bool CreateCommandBuffer(const CommandBufferDesc* command_buffer_desc, ICommandBuffer** command_buffer) = 0;

	//! \brief コマンドキューを作成します
	//!
	//! \param[in] command_queue_desc コマンドキュー作成情報
	//! \param[out] command_queue コマンドキュー
	//!
	//! \retval true 作成成功
	//! \retval false 作成失敗
	//virtual bool CreateCommandQueue(const CommandQueueDesc* command_queue_desc, ICommandQueue** command_queue) = 0;

	//! \brief スワップチェインを作成します
	//!
	//! \param[in] swap_chain_desc スワップチェイン作成情報
	//! \param[out] swap_chain スワップチェイン
	//!
	//! \retval true 作成成功
	//! \retval false 作成失敗
	virtual bool CreateSwapChain(const SwapChainDesc* swap_chain_desc, ISwapChain** swap_chain) = 0;

	//! \brief カラーターゲットを作成します
	//!
	//! \param[in] color_target_desc カラーターゲット作成情報
	//! \param[out] color_target カラーターゲット
	//!
	//! \retval true 作成成功
	//! \retval false 作成失敗
	//virtual bool CreateColorTarget(const ColorTargetDesc* color_target_desc, IColorTarget** color_target) = 0;

	//! \brief 深度ターゲットを作成します
	//!
	//! \param[in] depth_target_desc 深度ターゲット作成情報
	//! \param[out] depth_target 深度ターゲット
	//!
	//! \retval true 作成成功
	//! \retval false 作成失敗
	//virtual bool CreateDepthTarget(const DepthTargetDesc* depth_target_desc, IDepthTarget** depth_target) = 0;

	//! \brief プラットフォーム固有のデバイスを取得します
	//!
	//! \param[out] raw_device プラットフォーム固有のデバイスが格納された構造体
	//!
	//! \retval true 取得成功
	//! \retval false 取得失敗
	virtual bool GetRawDevice(RawDevice* raw_device) const = 0;
};

//! \brief デバイスを作成します
//!
//! \param[in] device_desc デバイス作成情報
//! \param[out] device デバイス
//!
//! \retval true 作成成功
//! \retval false 作成失敗
bool CreateDevice(const DeviceDesc* device_desc, IDevice** device);

} // namespace RHI
