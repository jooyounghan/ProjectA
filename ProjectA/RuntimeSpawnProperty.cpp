#include "RuntimeSpawnProperty.h"
#include "MacroUtilities.h"
#include "InitialPropertyDefinition.h"

#include "DynamicBuffer.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;		


#define SelectedGPUColorInterp()		\
m_onGpuColorInterpolaterSelected(		\
	m_checkGPUColorInterpolater, 		\
	m_colorInterpolationMethod, 		\
	m_colorInterpolater.get()			\
)										\

#define UpdatedGPUColorInterp()			\
m_onGpuColorInterpolaterUpdated(		\
	m_checkGPUColorInterpolater, 		\
	m_runtimeSpawnPropertyCPU.maxLife,	\
	m_colorInterpolationMethod, 		\
	m_colorInterpolater.get()			\
)										\


CRuntimeSpawnProperty::CRuntimeSpawnProperty(
	const function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
)
	: IDrawable(),
	m_currentLifeTime(0.f),
	m_isRuntimeSpawnPropertyChanged(false),
	m_positionShapedVector(InitPositionShapedVector),
	m_positionOrigin(InitOrigin),
	m_positionUpVector(InitUpVector),
	m_speedShapedVector(InitSpeedShapedVector),
	m_speedOrigin(InitOrigin),
	m_speedUpVector(InitUpVector),
	m_colorInitControlPoint{ 0.f, MakeArray(1.f, 0.f, 0.f, 1.f) },
	m_colorFinalControlPoint{ InitLife, MakeArray(0.f, 0.f ,1.f, 1.f) },
	m_colorInterpolationMethod(EInterpolationMethod::Linear),
	m_checkGPUColorInterpolater(false),
	m_onGpuColorInterpolaterSelected(gpuColorInterpolaterSelectedHandler),
	m_onGpuColorInterpolaterUpdated(gpuColorInterpolaterUpdatedHandler)
{
	ZeroMem(m_runtimeSpawnPropertyCPU);

	m_runtimeSpawnPropertyCPU.color = InitColor;
	m_runtimeSpawnPropertyCPU.maxLife = InitLife;
	m_runtimeSpawnPropertyCPU.xyScale = InitXYScale;

	m_positionShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"생성 위치 벡터", "생성 반지름",
		m_positionOrigin, m_positionUpVector, m_positionCenterAngle,
		m_runtimeSpawnPropertyCPU.shapedPositionVectorProperty
	);

	m_speedShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"생성 속도 벡터", "생성 속도",
		m_speedOrigin, m_speedUpVector, m_speedCenterAngle,
		m_runtimeSpawnPropertyCPU.shapedSpeedVectorProperty
	);

	m_colorControlPointGridView = make_unique<CControlPointGridView<4>>(
		"시간",
		array<string, 4>{ "R", "G", "B", "A"},
		"색상값",
		0.01f, 0.f, 1.f,
		m_colorInitControlPoint,
		m_colorFinalControlPoint,
		m_colorControlPoints
	);

	m_colorInterpolationSelectPlotter = make_unique<CInterpolaterSelectPlotter<4>>(
		"파티클 색상 보간 방법",
		"Color Control Points",
		array<string, 4>{ "R", "G", "B", "A" },
		m_colorInitControlPoint,
		m_colorFinalControlPoint,
		m_colorControlPoints
	);

	m_colorInterpolationSelectPlotter->CreateInterpolater(
		m_colorInterpolationMethod, 
		m_colorInterpolater
	);
	m_colorInterpolationSelectPlotter->ResetXYScale();

	AdjustControlPointsFromLife();
}


void CRuntimeSpawnProperty::AdjustControlPointsFromLife()
{
	const float& maxLife = m_runtimeSpawnPropertyCPU.maxLife;
	m_colorFinalControlPoint.x = maxLife;
	m_colorControlPoints.erase(
		std::remove_if(m_colorControlPoints.begin(), m_colorControlPoints.end(),
			[&](const SControlPoint<4>& p)
			{
				return p.x > maxLife;
			}),
		m_colorControlPoints.end()
	);

	m_colorInterpolater->UpdateCoefficient();
	m_colorInterpolationSelectPlotter->ResetXYScale();

	UpdatedGPUColorInterp();
	m_isRuntimeSpawnPropertyChanged = true;
}


void CRuntimeSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_runtimeSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_runtimeSpawnPropertyCPU));
	m_runtimeSpawnPropertyGPU->InitializeBuffer(device);

	SelectedGPUColorInterp();
	UpdatedGPUColorInterp();
}

void CRuntimeSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	m_currentLifeTime += dt;
	const float& life = m_runtimeSpawnPropertyCPU.maxLife;
	if (m_currentLifeTime > life)
	{
		m_currentLifeTime = max(m_currentLifeTime - life, 0.f);
	}

	UpdateImpl(deviceContext, dt);

	if (m_isRuntimeSpawnPropertyChanged)
	{
		m_runtimeSpawnPropertyGPU->Stage(deviceContext);
		m_runtimeSpawnPropertyGPU->Upload(deviceContext);
		m_isRuntimeSpawnPropertyChanged = false;
	}
}

void CRuntimeSpawnProperty::UpdateImpl(ID3D11DeviceContext* deviceContext, float dt)
{
	if (!m_checkGPUColorInterpolater)
	{
		array<float, 4> interpolatedColor = m_colorInterpolater->GetInterpolated(m_currentLifeTime);
		m_runtimeSpawnPropertyCPU.color = XMVectorSet(interpolatedColor[0], interpolatedColor[1], interpolatedColor[2], interpolatedColor[3]);
		m_isRuntimeSpawnPropertyChanged = true;
	}
}

void CRuntimeSpawnProperty::DrawUI()
{
	if (!ImGui::CollapsingHeader("파티클 생성 프로퍼티"))
		return;

	DrawUIImpl();
}

void CRuntimeSpawnProperty::DrawUIImpl()
{
	SeparatorText("파티클 생성 위치 설정");
	m_isRuntimeSpawnPropertyChanged |=
		m_positionShapedVectorSelector->SelectEnums(m_positionShapedVector) |
		m_positionShapedVectorSelector->SetShapedVectorProperty(m_positionShapedVector);

	SeparatorText("파티클 생성 속도 설정");
	m_isRuntimeSpawnPropertyChanged |=
		m_speedShapedVectorSelector->SelectEnums(m_speedShapedVector) |
		m_speedShapedVectorSelector->SetShapedVectorProperty(m_speedShapedVector);

	SeparatorText("파티클 생명 주기 설정");
	if (DragFloat("파티클 생명 주기", &m_runtimeSpawnPropertyCPU.maxLife, 0.1f, 0.f, 10.f, "%.1f"))
	{
		AdjustControlPointsFromLife();
	}

	SeparatorText("파티클 색상 설정");
	EInterpolationMethod currnetColorInterpolateKind = m_colorInterpolationMethod;
	m_colorInterpolationSelectPlotter->SelectEnums(currnetColorInterpolateKind);
	if (m_colorInterpolationMethod != currnetColorInterpolateKind)
	{
		m_colorInterpolationMethod = currnetColorInterpolateKind;
		m_colorInterpolationSelectPlotter->CreateInterpolater(
			m_colorInterpolationMethod, 
			m_colorInterpolater
		);

		UpdatedGPUColorInterp();
	}

	if (Checkbox("GPU 기반 색상 보간", &m_checkGPUColorInterpolater))
	{
		if (m_checkGPUColorInterpolater)
		{
			m_runtimeSpawnPropertyCPU.color = XMVectorSet(0.f, 0.f, 0.f, 0.f);
			m_isRuntimeSpawnPropertyChanged = true;
		}

		m_colorInterpolationSelectPlotter->CreateInterpolater(
			m_colorInterpolationMethod,
			m_colorInterpolater
		);

		SelectedGPUColorInterp();
		UpdatedGPUColorInterp();
	}

	if (m_colorControlPointGridView->DrawControlPointGridView())
	{
		m_colorInterpolater->UpdateCoefficient();
		m_colorInterpolationSelectPlotter->ResetXYScale();

		UpdatedGPUColorInterp();
	}

	m_colorInterpolationSelectPlotter->ViewInterpolatedPlots();
}

void CRuntimeSpawnProperty::Serialize(std::ofstream& ofs)
{
	SerializeHelper::SerializeElement<decltype(m_runtimeSpawnPropertyCPU)>(ofs, m_runtimeSpawnPropertyCPU);

	SerializeHelper::SerializeElement<EShapedVector>(ofs, m_positionShapedVector);
	SerializeHelper::SerializeElement<XMFLOAT3>(ofs, m_positionOrigin);
	SerializeHelper::SerializeElement<XMVECTOR>(ofs, m_positionUpVector);
	SerializeHelper::SerializeElement<float>(ofs, m_positionCenterAngle);

	SerializeHelper::SerializeElement<EShapedVector>(ofs, m_speedShapedVector);
	SerializeHelper::SerializeElement<XMFLOAT3>(ofs, m_speedOrigin);
	SerializeHelper::SerializeElement<XMVECTOR>(ofs, m_speedUpVector);
	SerializeHelper::SerializeElement<float>(ofs, m_speedCenterAngle);

	SerializeHelper::SerializeElement<SControlPoint<4>>(ofs, m_colorInitControlPoint);
	SerializeHelper::SerializeElement<SControlPoint<4>>(ofs, m_colorFinalControlPoint);
	SerializeHelper::SerializeVector<SControlPoint<4>>(ofs, m_colorControlPoints);
	SerializeHelper::SerializeElement<EInterpolationMethod>(ofs, m_colorInterpolationMethod);

	SerializeHelper::SerializeElement<bool>(ofs, m_checkGPUColorInterpolater);
}

void CRuntimeSpawnProperty::Deserialize(std::ifstream& ifs)
{
	m_runtimeSpawnPropertyCPU = SerializeHelper::DeserializeElement<decltype(m_runtimeSpawnPropertyCPU)>(ifs);

	m_positionShapedVector = SerializeHelper::DeserializeElement<EShapedVector>(ifs);
	m_positionOrigin = SerializeHelper::DeserializeElement<XMFLOAT3>(ifs);
	m_positionUpVector = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);
	m_positionCenterAngle = SerializeHelper::DeserializeElement<float>(ifs);

	m_speedShapedVector = SerializeHelper::DeserializeElement<EShapedVector>(ifs);
	m_speedOrigin = SerializeHelper::DeserializeElement<XMFLOAT3>(ifs);
	m_speedUpVector = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);
	m_speedCenterAngle = SerializeHelper::DeserializeElement<float>(ifs);

	m_colorInitControlPoint = SerializeHelper::DeserializeElement<SControlPoint<4>>(ifs);
	m_colorFinalControlPoint = SerializeHelper::DeserializeElement<SControlPoint<4>>(ifs);
	m_colorControlPoints = SerializeHelper::DeserializeVector<SControlPoint<4>>(ifs);
	m_colorInterpolationMethod = SerializeHelper::DeserializeElement<EInterpolationMethod>(ifs);

	m_checkGPUColorInterpolater = SerializeHelper::DeserializeElement<bool>(ifs);

	m_colorInterpolationSelectPlotter->CreateInterpolater(
		m_colorInterpolationMethod,
		m_colorInterpolater
	);
	m_colorInterpolationSelectPlotter->ResetXYScale();

	m_isRuntimeSpawnPropertyChanged = true;
}