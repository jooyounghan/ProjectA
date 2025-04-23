#pragma once
#include "D3D11DllHelper.h"

struct D3D11MANAGER_API D3D11_DISPATCH_INDIRECT_ARGS
{
	unsigned int threadGroupCountX = 1;
	unsigned int threadGroupCountY = 1;
	unsigned int threadGroupCountZ = 1;
	const unsigned int dummy = 0;
};