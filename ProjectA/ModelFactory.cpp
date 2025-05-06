#include "ModelFactory.h"
#include <Windows.h>

using namespace std;
using namespace DirectX;

vector<XMFLOAT3> ModelFactory::CreateBoxPositions(const XMVECTOR& scale)
{
    vector<XMFLOAT3> result;
    XMFLOAT3 tmpPos[4];
   
    // À­¸é
    XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, -1.0f, 1.f), scale));
    result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// ¾Æ·§¸é
	XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, 1.0f, 1.f), scale));
	result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// ¾Õ¸é
	XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, -1.0f, 1.f), scale));
	result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// µÞ¸é
	XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, 1.0f, 1.f), scale));
	result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// ¿ÞÂÊ
	XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, -1.0f, 1.f), scale));
	result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// ¿À¸¥ÂÊ
	XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, 1.0f, 1.f), scale));
	result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	return result;
}

vector<XMFLOAT2> ModelFactory::CreateBoxUVCoords()
{
	vector<XMFLOAT2> result;

	// À­¸é
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// ¾Æ·§¸é
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// ¾Õ¸é
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// µÞ¸é
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// ¿ÞÂÊ
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// ¿À¸¥ÂÊ
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	return result;
}

vector<XMFLOAT3> ModelFactory::CreateBoxNormals()
{
	vector<XMFLOAT3> result;

	// À­¸é
	result.emplace_back(0.0f, 1.0f, 0.0f);
	result.emplace_back(0.0f, 1.0f, 0.0f);
	result.emplace_back(0.0f, 1.0f, 0.0f);
	result.emplace_back(0.0f, 1.0f, 0.0f);

	// ¾Æ·§¸é
	result.emplace_back(0.0f, -1.0f, 0.0f);
	result.emplace_back(0.0f, -1.0f, 0.0f);
	result.emplace_back(0.0f, -1.0f, 0.0f);
	result.emplace_back(0.0f, -1.0f, 0.0f);

	// ¾Õ¸é
	result.emplace_back(0.0f, 0.0f, -1.0f);
	result.emplace_back(0.0f, 0.0f, -1.0f);
	result.emplace_back(0.0f, 0.0f, -1.0f);
	result.emplace_back(0.0f, 0.0f, -1.0f);

	// µÞ¸é
	result.emplace_back(0.0f, 0.0f, 1.0f);
	result.emplace_back(0.0f, 0.0f, 1.0f);
	result.emplace_back(0.0f, 0.0f, 1.0f);
	result.emplace_back(0.0f, 0.0f, 1.0f);

	// ¿ÞÂÊ
	result.emplace_back(-1.0f, 0.0f, 0.0f);
	result.emplace_back(-1.0f, 0.0f, 0.0f);
	result.emplace_back(-1.0f, 0.0f, 0.0f);
	result.emplace_back(-1.0f, 0.0f, 0.0f);

	// ¿À¸¥ÂÊ
	result.emplace_back(1.0f, 0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f, 0.0f);

	return result;
}

vector<UINT> ModelFactory::CreateBoxIndices()
{
	return {
		0,  1,  2,  0,  2,  3,  // À­¸é
		4,  5,  6,  4,  6,  7,  // ¾Æ·§¸é
		8,  9,  10, 8,  10, 11, // ¾Õ¸é
		12, 13, 14, 12, 14, 15, // µÞ¸é
		16, 17, 18, 16, 18, 19, // ¿ÞÂÊ
		20, 21, 22, 20, 22, 23  // ¿À¸¥ÂÊ
	};
}

std::vector<DirectX::XMFLOAT3> ModelFactory::CreateQuadPositions()
{
	vector<XMFLOAT3> result;

	result.emplace_back(-1.0f, -1.0f, 0.0f);
	result.emplace_back(-1.0f, 1.0f, 0.0f);
	result.emplace_back(1.0f, -1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f, 0.0f);

	return result;
}

std::vector<DirectX::XMFLOAT2> ModelFactory::CreateQuadUVCoords()
{
	vector<XMFLOAT2> result;

	result.emplace_back(0.0f, 1.0f);
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(1.0f, 0.0f);

	return result;
}

std::vector<UINT> ModelFactory::CreateQuadIndices()
{
	return {
			0,  1,  2, 
			2,  1,  3
	};
}
