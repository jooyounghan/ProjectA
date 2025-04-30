#pragma once
#include "IDrawable.h"
#include "ISerializable.h"
#include "ShapedVectorProperty.h"

#include "Interpolater.h"
#include "ShapedVectorSelector.h"
#include "ControlPointGridView.h"
#include "InterpolaterSelector.h"
#include "GPUInterpPropertyManager.h"

#include <vector>
#include <memory>
#include <functional>

namespace D3D11
{
	class CDynamicBuffer;
}

class ARuntimeSpawnProperty : public IDrawable, public IUpdatable, public ISerializable
{
public:
	ARuntimeSpawnProperty(
		const std::function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
		const std::function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
	);
	~ARuntimeSpawnProperty() override = default;

protected:
	float m_currentLifeTime;

protected:
	struct  
	{
		union 
		{
			SShapedVectorProperty shapedPositionVectorProperty;
			struct
			{
				char padding1[88];
				float maxLife;
				float spriteIndex;
			};
		};
		union
		{
			SShapedVectorProperty shapedSpeedVectorProperty;
			struct
			{
				char padding3[88];
				DirectX::XMFLOAT2 xyScale;
			};
		};
		DirectX::XMVECTOR color;
	} m_runtimeSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_runtimeSpawnPropertyGPU;
	bool m_isRuntimeSpawnPropertyChanged;

public:
	ID3D11Buffer* GetParticleSpawnPropertyBuffer() const noexcept { return m_runtimeSpawnPropertyGPU->GetBuffer(); }

protected:
	EShapedVector m_positionShapedVector;
	std::unique_ptr<CShapedVectorSelector> m_positionShapedVectorSelector;
	DirectX::XMFLOAT3 m_positionOrigin;
	DirectX::XMVECTOR m_positionUpVector;

protected:
	EShapedVector m_speedShapedVector;
	std::unique_ptr<CShapedVectorSelector> m_speedShapedVectorSelector;
	DirectX::XMFLOAT3 m_speedOrigin;
	DirectX::XMVECTOR m_speedUpVector;

protected:
	SControlPoint<4> m_colorInitControlPoint;
	SControlPoint<4> m_colorFinalControlPoint;
	std::vector<SControlPoint<4>> m_colorControlPoints;
	EInterpolationMethod m_colorInterpolationMethod;
	std::unique_ptr<IInterpolater<4>> m_colorInterpolater;

protected:
	bool m_checkGPUColorInterpolater;
	UINT m_gpuColorInterpolaterID;

protected:
	std::function<void(bool, EInterpolationMethod, IInterpolater<4>*)> m_onGpuColorInterpolaterSelected;
	std::function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)> m_onGpuColorInterpolaterUpdated;

public:
	inline void SetGPUColorInterpolaterID(UINT gpuColorInterpolaterID) noexcept { m_gpuColorInterpolaterID = gpuColorInterpolaterID; }

protected:
	std::unique_ptr<CControlPointGridView<4>> m_colorControlPointGridView;
	std::unique_ptr<CInterpolaterSelectPlotter<4>> m_colorInterpolationSelectPlotter;

protected:
	virtual void AdjustControlPointsFromLife();

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

protected:
	virtual void UpdateImpl(ID3D11DeviceContext* deviceContext, float dt);

public:
	virtual void DrawUI() override final;

protected:
	virtual void DrawUIImpl() override;

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;
};

