#pragma once
#include <d3d11.h>

class IUpdatable
{
public:
	virtual ~IUpdatable() = 0;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) = 0;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) = 0;
};

inline IUpdatable::~IUpdatable() = default;