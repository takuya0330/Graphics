#pragma once

#include "D3D12App.h"

class D3D12GPUTimerApp : public D3D12App
{
public:
	D3D12GPUTimerApp(LPCWSTR title, UINT width, UINT height);

	virtual ~D3D12GPUTimerApp() override = default;

protected:
	virtual bool OnInitialize() override;

	virtual void OnFinalize() override;

	virtual void OnUpdate() override;

	virtual void OnRender() override;

	virtual LRESULT CALLBACK OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

protected:
	void beginCapture();

	void endCapture();

	float getTime();

private:
	uint32_t getQueryIndex(bool is_begin) const;

private:
	ComPtr<ID3D12QueryHeap> m_query_heap;
	ComPtr<ID3D12Resource>  m_readback_buffer;
	float                   m_gpu_time;

	ComPtr<ID3D12DescriptorHeap> m_imgui_heap;
};
