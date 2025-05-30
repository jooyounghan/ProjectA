#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <vector>

class ModelFactory
{
public:
	static std::vector<DirectX::XMFLOAT3> CreateBoxPositions(const DirectX::XMVECTOR& scale);
	static std::vector<DirectX::XMFLOAT2> CreateBoxUVCoords();
	static std::vector<DirectX::XMFLOAT3> CreateBoxNormals();
	static std::vector<UINT> CreateBoxIndices();
	
public:
	static std::vector<DirectX::XMFLOAT3> CreateQuadPositions();
	static std::vector<DirectX::XMFLOAT2> CreateQuadUVCoords();
	static std::vector<UINT> CreateQuadIndices();
};

