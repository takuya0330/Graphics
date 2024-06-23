#pragma once

#include <cstdint>

#include "ForwardDeclaration.h"

namespace RHI {

//! \brief デバイスクラスが管理するオブジェクトのインターフェース
class IDeviceObject
{
	friend class DeviceObjectDisposer;

public:
	virtual ~IDeviceObject() = default;

	//! \brief 参照カウントをインクリメントします
	virtual void AddRef() noexcept = 0;

	//! \brief 参照カウントをデクリメントします
	virtual void Release() noexcept = 0;

	//! \brief 現在の参照カウントを取得します
	//!
	//! \retval 参照カウント
	virtual uint32_t GetRefCount() const noexcept = 0;

	//! \brief デバイスを取得します
	//!
	//! \retval true 取得成功
	//! \retval false 取得失敗
	virtual bool GetDevice(IDevice** device) const = 0;

protected:
	//! \brief オブジェクトを破棄します
	//!
	//! \note この関数は DeviceObjectDisposer によって、適切なタイミングで呼び出されます
	virtual void dispose() noexcept = 0;
};

} // namespace RHI
