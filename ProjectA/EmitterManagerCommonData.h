#pragma once
#include "EmitterTypeDefinition.h"
#include "DispatchIndirectStructure.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

namespace D3D11
{
	class CComputeShader;
	class CVertexShader;
	class CGeometryShader;
	class CPixelShader;
	class CGraphicsPSOObject;

	class CConstantBuffer;
	class CStructuredBuffer;
	class CAppendBuffer;
}

class CEmitterManagerCommonData
{
public:
	static void Intialize(ID3D11Device* device);

#pragma region ���� �ʱ�ȭ ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GInitializeParticleSetCS[EmitterTypeCount];
#pragma endregion

#pragma region ���� ���� ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GParticleInitialSourceCS[EmitterTypeCount];
	static std::unique_ptr<D3D11::CComputeShader> GParticleRuntimeSourceCS[EmitterTypeCount];
#pragma endregion

#pragma region Indirect ���� ��� ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GCalcualteIndirectArgCS;
	static std::unique_ptr<D3D11::CComputeShader> GCalcualteRadixIndirectArgCS;
#pragma endregion


#pragma region ���� �ùķ��̼� ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GCaculateParticleForceCS[EmitterTypeCount];
#pragma endregion

#pragma region ��������Ʈ ���� ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GSpriteSetLocalHistogramCS;
#pragma endregion

#pragma region ���� �׸��� ���� PSO
public:
	static std::unique_ptr<D3D11::CVertexShader> GParticleDrawVS[EmitterTypeCount];
	static std::unique_ptr<D3D11::CGeometryShader> GParticleDrawGS[EmitterTypeCount];
	static std::unique_ptr<D3D11::CPixelShader> GParticleDrawPS[EmitterTypeCount];
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawParticlePSO[EmitterTypeCount];
#pragma endregion

#pragma region �̹��� �׸��� PSO
public:
	static const std::vector<DirectX::XMFLOAT3> GEmitterBoxPositions;
	static const std::vector<DirectX::XMFLOAT3> GEmitterBoxNormals;
	static const std::vector<UINT> GEmitterBoxIndices;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterPositionBuffer;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterNormalBuffer;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterIndexBuffer;
	static std::unique_ptr<D3D11::CVertexShader> GEmitterDrawVS;
	static std::unique_ptr<D3D11::CPixelShader> GEmitterDrawPS[EmitterTypeCount];
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawEmitterPSO[EmitterTypeCount];
#pragma endregion
};



