#pragma once

class CoreApp
{
public:
	virtual ~CoreApp() = default;

	virtual int Run() { return 0; }
};
