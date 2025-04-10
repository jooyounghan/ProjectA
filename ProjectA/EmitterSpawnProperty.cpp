#include "EmitterSpawnProperty.h"
#include "BufferMacroUtilities.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;

CEmitterSpawnProperty::CEmitterSpawnProperty(
	const XMFLOAT2& minInitRadians, 
	const XMFLOAT2& maxInitRadians, 
	const XMFLOAT2& minMaxRadius, 
	UINT initialParticleCount
)
	: m_emitterSpawnPropertyCPU{ minInitRadians, maxInitRadians, minMaxRadius, initialParticleCount, NULL }
{

}

void CEmitterSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterSpawnPropertyCPU));
	m_emitterSpawnPropertyGPU->InitializeBuffer(device);
}

void CEmitterSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	// Do Nothing
}
