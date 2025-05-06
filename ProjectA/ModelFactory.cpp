#include "ModelFactory.h"
#include <Windows.h>

using namespace std;
using namespace DirectX;

vector<XMFLOAT3> ModelFactory::CreateBoxPositions(const XMVECTOR& scale)
{
    vector<XMFLOAT3> result;
    XMFLOAT3 tmpPos[4];
   
    // ����
    XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, -1.0f, 1.f), scale));
    result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// �Ʒ���
	XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, 1.0f, 1.f), scale));
	result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// �ո�
	XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, -1.0f, 1.f), scale));
	result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// �޸�
	XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(1.0f, -1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(1.0f, 1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, 1.0f, 1.f), scale));
	result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// ����
	XMStoreFloat3(&tmpPos[0], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[1], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, 1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[2], XMVectorMultiply(XMVectorSet(-1.0f, 1.0f, -1.0f, 1.f), scale));
	XMStoreFloat3(&tmpPos[3], XMVectorMultiply(XMVectorSet(-1.0f, -1.0f, -1.0f, 1.f), scale));
	result.emplace_back(tmpPos[0]);
	result.emplace_back(tmpPos[1]);
	result.emplace_back(tmpPos[2]);
	result.emplace_back(tmpPos[3]);

	// ������
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

	// ����
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// �Ʒ���
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// �ո�
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// �޸�
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// ����
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	// ������
	result.emplace_back(0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f);
	result.emplace_back(1.0f, 1.0f);
	result.emplace_back(0.0f, 1.0f);

	return result;
}

vector<XMFLOAT3> ModelFactory::CreateBoxNormals()
{
	vector<XMFLOAT3> result;

	// ����
	result.emplace_back(0.0f, 1.0f, 0.0f);
	result.emplace_back(0.0f, 1.0f, 0.0f);
	result.emplace_back(0.0f, 1.0f, 0.0f);
	result.emplace_back(0.0f, 1.0f, 0.0f);

	// �Ʒ���
	result.emplace_back(0.0f, -1.0f, 0.0f);
	result.emplace_back(0.0f, -1.0f, 0.0f);
	result.emplace_back(0.0f, -1.0f, 0.0f);
	result.emplace_back(0.0f, -1.0f, 0.0f);

	// �ո�
	result.emplace_back(0.0f, 0.0f, -1.0f);
	result.emplace_back(0.0f, 0.0f, -1.0f);
	result.emplace_back(0.0f, 0.0f, -1.0f);
	result.emplace_back(0.0f, 0.0f, -1.0f);

	// �޸�
	result.emplace_back(0.0f, 0.0f, 1.0f);
	result.emplace_back(0.0f, 0.0f, 1.0f);
	result.emplace_back(0.0f, 0.0f, 1.0f);
	result.emplace_back(0.0f, 0.0f, 1.0f);

	// ����
	result.emplace_back(-1.0f, 0.0f, 0.0f);
	result.emplace_back(-1.0f, 0.0f, 0.0f);
	result.emplace_back(-1.0f, 0.0f, 0.0f);
	result.emplace_back(-1.0f, 0.0f, 0.0f);

	// ������
	result.emplace_back(1.0f, 0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f, 0.0f);
	result.emplace_back(1.0f, 0.0f, 0.0f);

	return result;
}

vector<UINT> ModelFactory::CreateBoxIndices()
{
	return {
		0,  1,  2,  0,  2,  3,  // ����
		4,  5,  6,  4,  6,  7,  // �Ʒ���
		8,  9,  10, 8,  10, 11, // �ո�
		12, 13, 14, 12, 14, 15, // �޸�
		16, 17, 18, 16, 18, 19, // ����
		20, 21, 22, 20, 22, 23  // ������
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
