#include "D3D11App.h"

D3D11App::D3D11App(LPCWSTR title, UINT width, UINT height)
    : Win32App(title, width, height)
{
}

bool D3D11App::OnInitialize()
{
	return true;
}

void D3D11App::OnFinalize()
{
}

void D3D11App::OnUpdate()
{
}

void D3D11App::OnRender()
{
}
