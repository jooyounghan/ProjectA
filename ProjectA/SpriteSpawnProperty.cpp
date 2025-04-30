#include "SpriteSpawnProperty.h"
#include "MacroUtilities.h"
#include "InitialPropertyDefinition.h"

//#include "stb_image.h"

using namespace std;
using namespace DirectX;
using namespace ImGui;

#define SelectedGPUSpriteSizeInterp()		\
m_onGpuSpriteSizeInterpolaterSelected(		\
	m_checkGPUSpriteSizeInterpolater, 		\
	m_spriteSizeInterpolationMethod, 		\
	m_spriteSizeInterpolater.get()			\
)											\

#define UpdateGPUSpriteSizeInterp()			\
m_onGpuSpriteSizeInterpolaterUpdated(		\
	m_checkGPUSpriteSizeInterpolater,		\
	m_runtimeSpawnPropertyCPU.maxLife,		\
	m_spriteSizeInterpolationMethod,		\
	m_spriteSizeInterpolater.get()			\
)											\

#define SelectedGpuSpriteIndexInterp()		\
m_onGpuSpriteIndexInterpolaterSelected(		\
	m_checkGPUSpriteIndexInterpolater, 		\
	m_spriteIndexInterpolationMethod, 		\
	m_spriteIndexInterpolater.get()			\
)											\

#define UpdateGPUSpriteIndexInterp()		\
m_onGpuSpriteIndexInterpolaterUpdated(		\
m_checkGPUSpriteIndexInterpolater,			\
m_runtimeSpawnPropertyCPU.maxLife,			\
m_spriteTextureCount,						\
m_spriteIndexInterpolationMethod,			\
m_spriteIndexInterpolater.get()				\
)											\

SpriteSpawnProperty::SpriteSpawnProperty(
	const std::function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const std::function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
	const std::function<void(bool, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterSelectedHandler,
	const std::function<void(bool, float, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler,
	const std::function<void(bool, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterSelectedHandler,
	const std::function<void(bool, float, UINT, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterUpdatedHandler
)
	: 
	ARuntimeSpawnProperty(gpuColorInterpolaterSelectedHandler, gpuColorInterpolaterUpdatedHandler),
	m_onGpuSpriteSizeInterpolaterSelected(gpuSpriteSizeInterpolaterSelectedHandler),
	m_onGpuSpriteSizeInterpolaterUpdated(gpuSpriteSizeInterpolaterUpdatedHandler),
	m_onGpuSpriteIndexInterpolaterSelected(gpuSpriteIndexInterpolaterSelectedHandler),
	m_onGpuSpriteIndexInterpolaterUpdated(gpuSpriteIndexInterpolaterUpdatedHandler),
	m_checkGPUSpriteSizeInterpolater(false),
	m_spriteSizeInitControlPoint{ 0.f, MakeArray(0.f, 0.f) },
	m_spriteSizeFinalControlPoint{ InitLife, MakeArray(10.f, 10.f) },
	m_spriteSizeInterpolationMethod(EInterpolationMethod::Linear),
	m_spriteTextureCount(1),
	m_checkGPUSpriteIndexInterpolater(false),
	m_spriteIndexInitControlPoint{ 0.f, 0.f },
	m_spriteIndexFinalControlPoint{ InitLife, 10.f },
	m_spriteIndexInterpolationMethod(EInterpolationMethod::Linear)
{
	CreateSpriteSizeInterpolaterUI();
	CreateSpriteIndexInterpolaterUI();
	AdjustControlPointsFromLife();
	AdjustControlPointsFromTextureCount();
}

void SpriteSpawnProperty::CreateSpriteSizeInterpolaterUI()
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
}

void SpriteSpawnProperty::CreateSpriteIndexInterpolaterUI()
{
	m_spriteIndexControlPointGridView = make_unique<CControlPointGridView<1>>(
		"시간",
		array<string, 1>{ "스프라이트 인덱스" },
		"스프라이트 텍스쳐 인덱스",
		1.f, 0.f, 10.f,
		m_spriteIndexInitControlPoint,
		m_spriteIndexFinalControlPoint,
		m_spriteIndexControlPoints
	);

	m_spriteIndexInterpolationSelectPlotter = make_unique<CInterpolaterSelectPlotter<1>>(
		"스프라이트 인덱스 방법",
		"Sprite Index",
		array<string, 1>{ "인덱스" },
		m_spriteIndexInitControlPoint,
		m_spriteIndexFinalControlPoint,
		m_spriteIndexControlPoints
	);

	m_spriteIndexInterpolationSelectPlotter->CreateInterpolater(
		m_spriteIndexInterpolationMethod,
		m_spriteIndexInterpolater
	);
	m_spriteIndexInterpolationSelectPlotter->ResetXYScale();
}

void SpriteSpawnProperty::AdjustControlPointsFromLife()
{
	ARuntimeSpawnProperty::AdjustControlPointsFromLife();

	const float& maxLife = m_runtimeSpawnPropertyCPU.maxLife;
	
	m_spriteSizeFinalControlPoint.x = maxLife;
	m_spriteSizeControlPoints.erase(
		std::remove_if(m_spriteSizeControlPoints.begin(), m_spriteSizeControlPoints.end(),
			[&](const SControlPoint<2>& p)
			{
				return p.x > maxLife;
			}),
		m_spriteSizeControlPoints.end()
	);

	m_spriteIndexFinalControlPoint.x = maxLife;
	m_spriteIndexControlPoints.erase(
		std::remove_if(m_spriteIndexControlPoints.begin(), m_spriteIndexControlPoints.end(),
			[&](const SControlPoint<1>& p)
			{
				return p.x > maxLife;
			}),
		m_spriteIndexControlPoints.end()
	);

	m_spriteSizeInterpolater->UpdateCoefficient();
	m_spriteSizeInterpolationSelectPlotter->ResetXYScale();

	m_spriteIndexInterpolater->UpdateCoefficient();
	m_spriteIndexInterpolationSelectPlotter->ResetXYScale();

	UpdateGPUSpriteSizeInterp();
	UpdateGPUSpriteIndexInterp();
}


void SpriteSpawnProperty::AdjustControlPointsFromTextureCount()
{
	float maxSpriteTextureIndex = (m_spriteTextureCount - 1.f);
	m_spriteIndexInitControlPoint.y[0] = min(m_spriteIndexInitControlPoint.y[0], float(maxSpriteTextureIndex));
	m_spriteIndexFinalControlPoint.y[0] = min(m_spriteIndexFinalControlPoint.y[0], float(maxSpriteTextureIndex));
	for (auto& spriteIndexControlPoint : m_spriteIndexControlPoints)
	{
		spriteIndexControlPoint.y[0] = min(spriteIndexControlPoint.y[0], float(maxSpriteTextureIndex));
	}

	m_spriteIndexInterpolater->UpdateCoefficient();
	m_spriteIndexInterpolationSelectPlotter->ResetXYScale();

	UpdateGPUSpriteIndexInterp();
}

void SpriteSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	ARuntimeSpawnProperty::Initialize(device, deviceContext);

	m_spriteSizeInterpolationSelectPlotter->CreateInterpolater(
		m_spriteSizeInterpolationMethod,
		m_spriteSizeInterpolater
	);
	m_spriteSizeInterpolationSelectPlotter->ResetXYScale();

	SelectedGPUSpriteSizeInterp();
	UpdateGPUSpriteSizeInterp();

	m_spriteIndexInterpolationSelectPlotter->CreateInterpolater(
		m_spriteIndexInterpolationMethod,
		m_spriteIndexInterpolater
	);
	m_spriteIndexInterpolationSelectPlotter->ResetXYScale();

	SelectedGpuSpriteIndexInterp();
	UpdateGPUSpriteIndexInterp();
}

void SpriteSpawnProperty::UpdateImpl(ID3D11DeviceContext* deviceContext, float dt)
{
	ARuntimeSpawnProperty::UpdateImpl(deviceContext, dt);

	if (!m_checkGPUSpriteSizeInterpolater)
	{
		array<float, 2> interpolatedXYScale = m_spriteSizeInterpolater->GetInterpolated(m_currentLifeTime);
		m_runtimeSpawnPropertyCPU.xyScale = XMFLOAT2(interpolatedXYScale[0], interpolatedXYScale[1]);
		m_isRuntimeSpawnPropertyChanged = true;
	}

	if (!m_checkGPUSpriteIndexInterpolater)
	{
		array<float, 1> interpolatedIndex = m_spriteIndexInterpolater->GetInterpolated(m_currentLifeTime);
		m_runtimeSpawnPropertyCPU.spriteIndex = interpolatedIndex[0];
		m_isRuntimeSpawnPropertyChanged = true;
	}
}

void SpriteSpawnProperty::DrawUIImpl()
{
	ARuntimeSpawnProperty::DrawUIImpl();
	DrawSpriteSizeSetting();
	DrawSpriteIndexSetting();
}

void SpriteSpawnProperty::DrawSpriteSizeSetting()
{
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

		UpdateGPUSpriteSizeInterp();
	}

	if (Checkbox("GPU 기반 스프라이트 크기 보간", &m_checkGPUSpriteSizeInterpolater))
	{
		m_spriteSizeInterpolationSelectPlotter->CreateInterpolater(
			m_spriteSizeInterpolationMethod,
			m_spriteSizeInterpolater
		);

		SelectedGPUSpriteSizeInterp();
		UpdateGPUSpriteSizeInterp();
	}

	if (m_spriteSizeControlPointGridView->DrawControlPointGridView())
	{
		m_spriteSizeInterpolater->UpdateCoefficient();
		m_spriteSizeInterpolationSelectPlotter->ResetXYScale();

		UpdateGPUSpriteSizeInterp();
	}

	m_spriteSizeInterpolationSelectPlotter->ViewInterpolatedPlots();
}

void SpriteSpawnProperty::DrawSpriteIndexSetting()
{
	SeparatorText("스프라이트 텍스쳐 설정");
	if (DragInt("텍스쳐 개수", (int*)&m_spriteTextureCount, 1.f, 0, 100, "%d"))
	{
		AdjustControlPointsFromTextureCount();
		UpdateGPUSpriteIndexInterp();
	}

	EInterpolationMethod currnetSpriteIndexInterpolateKind = m_spriteIndexInterpolationMethod;
	m_spriteIndexInterpolationSelectPlotter->SelectEnums(currnetSpriteIndexInterpolateKind);
	if (m_spriteIndexInterpolationMethod != currnetSpriteIndexInterpolateKind)
	{
		m_spriteIndexInterpolationMethod = currnetSpriteIndexInterpolateKind;
		m_spriteIndexInterpolationSelectPlotter->CreateInterpolater(
			m_spriteIndexInterpolationMethod,
			m_spriteIndexInterpolater
		);

		UpdateGPUSpriteIndexInterp();
	}

	if (Checkbox("GPU 기반 스프라이트 텍스쳐 인덱스 보간", &m_checkGPUSpriteIndexInterpolater))
	{
		m_spriteIndexInterpolationSelectPlotter->CreateInterpolater(
			m_spriteIndexInterpolationMethod,
			m_spriteIndexInterpolater
		);

		SelectedGpuSpriteIndexInterp();
		UpdateGPUSpriteIndexInterp();
	}

	if (m_spriteIndexControlPointGridView->DrawControlPointGridView())
	{
		m_spriteIndexInterpolater->UpdateCoefficient();
		m_spriteIndexInterpolationSelectPlotter->ResetXYScale();

		UpdateGPUSpriteIndexInterp();
	}

	m_spriteIndexInterpolationSelectPlotter->ViewInterpolatedPlots();
}

void SpriteSpawnProperty::Serialize(std::ofstream& ofs)
{
	ARuntimeSpawnProperty::Serialize(ofs);

	SerializeHelper::SerializeElement<SControlPoint<2>>(ofs, m_spriteSizeInitControlPoint);
	SerializeHelper::SerializeElement<SControlPoint<2>>(ofs, m_spriteSizeFinalControlPoint);
	SerializeHelper::SerializeVector<SControlPoint<2>>(ofs, m_spriteSizeControlPoints);
	SerializeHelper::SerializeElement<EInterpolationMethod>(ofs, m_spriteSizeInterpolationMethod);
	SerializeHelper::SerializeElement<bool>(ofs, m_checkGPUSpriteSizeInterpolater);

	SerializeHelper::SerializeElement<UINT>(ofs, m_spriteTextureCount);
	SerializeHelper::SerializeElement<SControlPoint<1>>(ofs, m_spriteIndexInitControlPoint);
	SerializeHelper::SerializeElement<SControlPoint<1>>(ofs, m_spriteIndexFinalControlPoint);
	SerializeHelper::SerializeVector<SControlPoint<1>>(ofs, m_spriteIndexControlPoints);
	SerializeHelper::SerializeElement<EInterpolationMethod>(ofs, m_spriteIndexInterpolationMethod);
	SerializeHelper::SerializeElement<bool>(ofs, m_checkGPUSpriteIndexInterpolater);
}

void SpriteSpawnProperty::Deserialize(std::ifstream& ifs)
{
	ARuntimeSpawnProperty::Deserialize(ifs);

	m_spriteSizeInitControlPoint = SerializeHelper::DeserializeElement<SControlPoint<2>>(ifs);
	m_spriteSizeFinalControlPoint = SerializeHelper::DeserializeElement<SControlPoint<2>>(ifs);
	m_spriteSizeControlPoints = SerializeHelper::DeserializeVector<SControlPoint<2>>(ifs);
	m_spriteSizeInterpolationMethod = SerializeHelper::DeserializeElement<EInterpolationMethod>(ifs);
	m_checkGPUSpriteSizeInterpolater = SerializeHelper::DeserializeElement<bool>(ifs);

	m_spriteTextureCount = SerializeHelper::DeserializeElement<UINT>(ifs);
	m_spriteIndexInitControlPoint = SerializeHelper::DeserializeElement<SControlPoint<1>>(ifs);
	m_spriteIndexFinalControlPoint = SerializeHelper::DeserializeElement<SControlPoint<1>>(ifs);
	m_spriteIndexControlPoints = SerializeHelper::DeserializeVector<SControlPoint<1>>(ifs);
	m_spriteIndexInterpolationMethod = SerializeHelper::DeserializeElement<EInterpolationMethod>(ifs);
	m_checkGPUSpriteIndexInterpolater = SerializeHelper::DeserializeElement<bool>(ifs);


}
