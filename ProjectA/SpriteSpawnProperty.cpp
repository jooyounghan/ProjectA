#include "SpriteSpawnProperty.h"
#include "MacroUtilities.h"
#include "InitialPropertyDefinition.h"

using namespace std;
using namespace DirectX;
using namespace ImGui;

SpriteSpawnProperty::SpriteSpawnProperty(
	const std::function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const std::function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
	const std::function<void(bool, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterSelectedHandler,
	const std::function<void(bool, float, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler
)
	: 
	ARuntimeSpawnProperty(gpuColorInterpolaterSelectedHandler, gpuColorInterpolaterUpdatedHandler),
	m_onGpuSpriteSizeInterpolaterSelected(gpuSpriteSizeInterpolaterSelectedHandler),
	m_onGpuSpriteSizeInterpolaterUpdated(gpuSpriteSizeInterpolaterUpdatedHandler),
	m_checkGPUSpriteSizeInterpolater(false),
	m_spriteSizeInitControlPoint{ 0.f, MakeArray(0.f, 0.f) },
	m_spriteSizeFinalControlPoint{ InitLife, MakeArray(10.f, 10.f) },
	m_spriteSizeInterpolationMethod(EInterpolationMethod::Linear)
{
	m_spriteSizeControlPointGridView = make_unique<CControlPointGridView<2>>(
		"시간",
		array<string, 2>{ "X 크기", "Y 크기" },
		"스프라이트 크기",
		0.01f, 0.f, 10.f,
		m_spriteSizeInitControlPoint,
		m_spriteSizeFinalControlPoint,
		m_spriteSizeControlPoints
	);

	m_spriteSizeInterpolationSelectPlotter = make_unique<CInterpolaterSelectPlotter<2>>(
		"스프라이트 크기 보간 방법",
		"Sprite Size Control Points",
		array<string, 2>{ "X 크기", "Y 크기" },
		m_spriteSizeInitControlPoint,
		m_spriteSizeFinalControlPoint,
		m_spriteSizeControlPoints
	);

	m_spriteSizeInterpolationSelectPlotter->CreateInterpolater(
		m_spriteSizeInterpolationMethod, 
		m_spriteSizeInterpolater
	);
	m_spriteSizeInterpolationSelectPlotter->ResetXYScale();

	AdjustControlPointsFromLife();
}

void SpriteSpawnProperty::AdjustControlPointsFromLife()
{
	ARuntimeSpawnProperty::AdjustControlPointsFromLife();

	const float& maxLife = m_baseParticleSpawnPropertyCPU.maxLife;
	m_spriteSizeFinalControlPoint.x = maxLife;
	m_spriteSizeControlPoints.erase(
		std::remove_if(m_spriteSizeControlPoints.begin(), m_spriteSizeControlPoints.end(),
			[&](const SControlPoint<2>& p)
			{
				return p.x > maxLife;
			}),
		m_spriteSizeControlPoints.end()
	);

	m_spriteSizeInterpolater->UpdateCoefficient();
	m_spriteSizeInterpolationSelectPlotter->ResetXYScale();

	m_onGpuSpriteSizeInterpolaterUpdated(
		m_checkGPUSpriteSizeInterpolater, m_baseParticleSpawnPropertyCPU.maxLife,
		m_spriteSizeInterpolationMethod, m_spriteSizeInterpolater.get()
	);
}


void SpriteSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	ARuntimeSpawnProperty::Initialize(device, deviceContext);

	m_onGpuSpriteSizeInterpolaterSelected(
		m_checkGPUSpriteSizeInterpolater, m_spriteSizeInterpolationMethod, m_spriteSizeInterpolater.get()
	);

	m_onGpuSpriteSizeInterpolaterUpdated(
		m_checkGPUSpriteSizeInterpolater, m_baseParticleSpawnPropertyCPU.maxLife, 
		m_spriteSizeInterpolationMethod, m_spriteSizeInterpolater.get()
	);
}

void SpriteSpawnProperty::UpdateImpl(ID3D11DeviceContext* deviceContext, float dt)
{
	if (!m_checkGPUSpriteSizeInterpolater)
	{
		array<float, 2> interpolatedXYScale = m_spriteSizeInterpolater->GetInterpolated(m_currentLifeTime);
		m_baseParticleSpawnPropertyCPU.xyScale = XMFLOAT2(interpolatedXYScale[0], interpolatedXYScale[1]);
		m_isParticleSpawnPropertyChanged = true;
	}
}

void SpriteSpawnProperty::DrawPropertyUIImpl()
{
	ARuntimeSpawnProperty::DrawPropertyUIImpl();

	SeparatorText("스프라이트 크기 설정");
	EInterpolationMethod currnetSpriteSizeInterpolateKind = m_spriteSizeInterpolationMethod;
	m_spriteSizeInterpolationSelectPlotter->SelectEnums(currnetSpriteSizeInterpolateKind);
	if (m_spriteSizeInterpolationMethod != currnetSpriteSizeInterpolateKind)
	{
		m_spriteSizeInterpolationMethod = currnetSpriteSizeInterpolateKind;
		m_spriteSizeInterpolationSelectPlotter->CreateInterpolater(
			m_spriteSizeInterpolationMethod, 
			m_spriteSizeInterpolater
		);

		m_onGpuSpriteSizeInterpolaterUpdated(
			m_checkGPUSpriteSizeInterpolater, m_baseParticleSpawnPropertyCPU.maxLife,
			m_spriteSizeInterpolationMethod, m_spriteSizeInterpolater.get()
		);
	}

	if (Checkbox("GPU 기반 스프라이트 크기 보간", &m_checkGPUSpriteSizeInterpolater))
	{
		m_spriteSizeInterpolationSelectPlotter->CreateInterpolater(
			m_spriteSizeInterpolationMethod,
			m_spriteSizeInterpolater
		);

		m_onGpuSpriteSizeInterpolaterSelected(
			m_checkGPUSpriteSizeInterpolater, m_spriteSizeInterpolationMethod, m_spriteSizeInterpolater.get()
		);

		m_onGpuSpriteSizeInterpolaterUpdated(
			m_checkGPUSpriteSizeInterpolater, m_baseParticleSpawnPropertyCPU.maxLife,
			m_spriteSizeInterpolationMethod, m_spriteSizeInterpolater.get()
		);
	}

	if (m_spriteSizeControlPointGridView->DrawControlPointGridView())
	{
		m_spriteSizeInterpolater->UpdateCoefficient();
		m_spriteSizeInterpolationSelectPlotter->ResetXYScale();

		m_onGpuSpriteSizeInterpolaterUpdated(
			m_checkGPUSpriteSizeInterpolater, m_baseParticleSpawnPropertyCPU.maxLife,
			m_spriteSizeInterpolationMethod, m_spriteSizeInterpolater.get()
		);
	}

	m_spriteSizeInterpolationSelectPlotter->ViewInterpolatedPlots();
}

void SpriteSpawnProperty::Serialize(std::ofstream& ofs)
{
	ARuntimeSpawnProperty::Serialize(ofs);

	SerializeHelper::SerializeElement<SControlPoint<2>>(ofs, m_spriteSizeInitControlPoint);
	SerializeHelper::SerializeElement<SControlPoint<2>>(ofs, m_spriteSizeFinalControlPoint);
	SerializeHelper::SerializeVector<SControlPoint<2>>(ofs, m_spriteSizeControlPoints);
	SerializeHelper::SerializeElement<EInterpolationMethod>(ofs, m_spriteSizeInterpolationMethod);

	SerializeHelper::SerializeElement<bool>(ofs, m_checkGPUSpriteSizeInterpolater);
}

void SpriteSpawnProperty::Deserialize(std::ifstream& ifs)
{
	ARuntimeSpawnProperty::Deserialize(ifs);

	m_spriteSizeInitControlPoint = SerializeHelper::DeserializeElement<SControlPoint<2>>(ifs);
	m_spriteSizeFinalControlPoint = SerializeHelper::DeserializeElement<SControlPoint<2>>(ifs);
	m_spriteSizeControlPoints = SerializeHelper::DeserializeVector<SControlPoint<2>>(ifs);
	m_spriteSizeInterpolationMethod = SerializeHelper::DeserializeElement<EInterpolationMethod>(ifs);

	m_checkGPUSpriteSizeInterpolater = SerializeHelper::DeserializeElement<bool>(ifs);

	m_spriteSizeInterpolationSelectPlotter->CreateInterpolater(
		m_spriteSizeInterpolationMethod,
		m_spriteSizeInterpolater
	);
	m_spriteSizeInterpolationSelectPlotter->ResetXYScale();

	m_onGpuSpriteSizeInterpolaterSelected(
		m_checkGPUSpriteSizeInterpolater, m_spriteSizeInterpolationMethod, m_spriteSizeInterpolater.get()
	);

	m_onGpuSpriteSizeInterpolaterUpdated(
		m_checkGPUSpriteSizeInterpolater, m_baseParticleSpawnPropertyCPU.maxLife,
		m_spriteSizeInterpolationMethod, m_spriteSizeInterpolater.get()
	);
}
