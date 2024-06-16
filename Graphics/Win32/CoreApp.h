#pragma once

#define APP_WIN32    1
#define APP_D3D11    1
#define APP_D3D12    1
#define APP_IMGUI    1
#define APP_TRIANGLE 1
#define APP_TEXTURE  1

class CoreApp
{
public:
	virtual ~CoreApp() = default;

	virtual int Run() { return 0; }
};
