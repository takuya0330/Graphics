#pragma once

#include "Platform.h"

class CoreApp
{
public:
	virtual ~CoreApp() = default;

	virtual int Run()
	{
		return 0;
	}
};
