#include "BaseParticleSpawnProperty.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "DynamicBuffer.h"

#include "ShapedVectorSelector.h"
#include "ControlPointGridView.h"
#include "InterpolaterSelector.h"


using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;		

#define InitLife 1.f

CBaseParticleSpawnProperty::CBaseParticleSpawnProperty(
	const function<void(float)>& lifeChangedHandler,
	const function<void(uint32_t, uint32_t)>& colorInterpolationChangedHandler
)
	: IProperty(),
	m_currentLifeTime(0.f),
	m_onLifeChanged(lifeChangedHandler),
	m_onColorInterpolationChanged(colorInterpolationChangedHandler),
	m_isParticleSpawnPropertyChanged(false),
	m_positionShapedVector(EShapedVector::Sphere),
	m_positionOrigin(XMFLOAT3(0.f, 0.f, 0.f)),
	m_positionUpVector(XMVectorSet(0.f, 1.f, 0.f, 0.f)),
	m_speedShapedVector(EShapedVector::None),
	m_speedOrigin(XMFLOAT3(0.f, 0.f, 0.f)),
	m_speedUpVector(XMVectorSet(0.f, 1.f, 0.f, 0.f)),
	m_colorInitControlPoint{ 0.f, MakeArray(1.f, 0.f, 0.f, 1.f)},
	m_colorFinalControlPoint{ InitLife, MakeArray(0.f, 0.f ,1.f, 1.f)},
	m_colorInterpolationMethod(EInterpolationMethod::Linear),
	m_useGPUColorInterpolater(false)
{
	AutoZeroMemory(m_baseParticleSpawnPropertyCPU);

	m_baseParticleSpawnPropertyCPU.life = InitLife;

	m_positionShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"���� ��ġ ����", "���� ������",
		m_positionOrigin, m_positionUpVector,
		m_baseParticleSpawnPropertyCPU.shapedPositionVectorProperty
	);

	m_speedShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"���� �ӵ� ����", "���� �ӵ�",
		m_speedOrigin, m_speedUpVector,
		m_baseParticleSpawnPropertyCPU.shapedSpeedVectorProperty
	);

	m_colorControlPointGridView = make_unique<CControlPointGridView<4>>(
		"�ð�",
		array<string, 4>{ "R", "G", "B", "A"},
		"����",
		0.01f, 0.f, 1.f,
		m_colorInitControlPoint,
		m_colorFinalControlPoint,
		m_colorControlPoints
	);

	m_colorInterpolationSelectPlotter = make_unique<CInterpolaterSelectPlotter<4>>(
		"��ƼŬ ���� ���� ���",
		"Color Control Points",
		array<string, 4>{ "R", "G", "B", "A" },
		m_colorInitControlPoint,
		m_colorFinalControlPoint,
		m_colorControlPoints
	);

	m_colorInterpolationSelectPlotter->CreateInterpolater(m_useGPUColorInterpolater, m_colorInterpolationMethod, m_colorInterpolater);
	m_colorInterpolationSelectPlotter->RedrawSelectPlotter();

	m_onLifeChanged(InitLife);
	m_onColorInterpolationChanged(m_colorInterpolater->GetInterpolaterID(), m_colorInterpolater->GetCoefficientCount());
}

ID3D11Buffer* CBaseParticleSpawnProperty::GetParticleSpawnPropertyBuffer() const noexcept { return m_baseParticleSpawnPropertyGPU->GetBuffer(); }

void CBaseParticleSpawnProperty::OnCheckGPUColorInterpolater()
{
	if (m_useGPUColorInterpolater)
	{
		m_baseParticleSpawnPropertyCPU.color = XMVectorZero();
		m_isParticleSpawnPropertyChanged = true;
	}
	else
	{

	}

	m_colorInterpolationSelectPlotter->CreateInterpolater(m_useGPUColorInterpolater, m_colorInterpolationMethod, m_colorInterpolater);
	m_onColorInterpolationChanged(m_colorInterpolater->GetInterpolaterID(), m_colorInterpolater->GetCoefficientCount());
}

void CBaseParticleSpawnProperty::AdjustControlPointsFromLife()
{
	const float& life = m_baseParticleSpawnPropertyCPU.life;
	m_colorFinalControlPoint.x = life;
	m_colorControlPoints.erase(
		std::remove_if(m_colorControlPoints.begin(), m_colorControlPoints.end(),
			[&](const SControlPoint<4>& p)
			{
				return p.x > life;
			}),
		m_colorControlPoints.end()
	);

	m_colorInterpolater->UpdateCoefficient();
	m_colorInterpolationSelectPlotter->RedrawSelectPlotter();
	m_onLifeChanged(life);
}


void CBaseParticleSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_baseParticleSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_baseParticleSpawnPropertyCPU));
	m_baseParticleSpawnPropertyGPU->InitializeBuffer(device);
}

void CBaseParticleSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	m_currentLifeTime += dt;
	const float& life = m_baseParticleSpawnPropertyCPU.life;
	if (m_currentLifeTime > life)
	{
		m_currentLifeTime = max(m_currentLifeTime - life, 0.f);
	}

	if (!m_useGPUColorInterpolater)
	{
		array<float, 4> interpolatedColor = m_colorInterpolater->GetInterpolated(m_currentLifeTime);
		m_baseParticleSpawnPropertyCPU.color = XMVectorSet(interpolatedColor[0], interpolatedColor[1], interpolatedColor[2], interpolatedColor[3]);
		m_isParticleSpawnPropertyChanged = true;
	}

	if (m_isParticleSpawnPropertyChanged)
	{
		m_baseParticleSpawnPropertyGPU->Stage(deviceContext);
		m_baseParticleSpawnPropertyGPU->Upload(deviceContext);
		m_isParticleSpawnPropertyChanged = false;
	}
}

void CBaseParticleSpawnProperty::DrawPropertyUI()
{
	if (!ImGui::CollapsingHeader("��ƼŬ ���� ������Ƽ"))
		return;

	SeparatorText("��ƼŬ ���� ��ġ ����");
	m_positionShapedVectorSelector->SelectEnums(m_positionShapedVector);
	if (m_positionShapedVectorSelector->SetShapedVectorProperty(m_positionShapedVector))
	{
		m_isParticleSpawnPropertyChanged = true;
	}

	SeparatorText("��ƼŬ ���� �ӵ� ����");
	m_speedShapedVectorSelector->SelectEnums(m_speedShapedVector);
	if (m_speedShapedVectorSelector->SetShapedVectorProperty(m_speedShapedVector))
	{
		m_isParticleSpawnPropertyChanged = true;
	}

	SeparatorText("��ƼŬ ���� �ֱ� ����");
	if (DragFloat("��ƼŬ ���� �ֱ�", &m_baseParticleSpawnPropertyCPU.life, 0.1f, 0.f, 10.f, "%.1f"))
	{
		AdjustControlPointsFromLife();
		m_isParticleSpawnPropertyChanged = true;
	}

	SeparatorText("��ƼŬ ���� ����");
	EInterpolationMethod currnetColorInterpolateKind = m_colorInterpolationMethod;
	m_colorInterpolationSelectPlotter->SelectEnums(currnetColorInterpolateKind);
	if (m_colorInterpolationMethod != currnetColorInterpolateKind)
	{
		m_colorInterpolationMethod = currnetColorInterpolateKind;
		m_colorInterpolationSelectPlotter->CreateInterpolater(m_useGPUColorInterpolater, m_colorInterpolationMethod, m_colorInterpolater);
		m_onColorInterpolationChanged(m_colorInterpolater->GetInterpolaterID(), m_colorInterpolater->GetCoefficientCount());
	}

	if (Checkbox("GPU ��� ���� ����", &m_useGPUColorInterpolater))
	{
		OnCheckGPUColorInterpolater();
	}

	if (m_colorControlPointGridView->DrawControlPointGridView())
	{
		m_colorInterpolater->UpdateCoefficient();
		m_colorInterpolationSelectPlotter->RedrawSelectPlotter();
	}
	m_colorInterpolationSelectPlotter->ViewInterpolatedPlots();
}

void CBaseParticleSpawnProperty::Serialize(std::ofstream& ofs)
{
	SerializeHelper::SerializeElement<decltype(m_baseParticleSpawnPropertyCPU)>(ofs, m_baseParticleSpawnPropertyCPU);

	SerializeHelper::SerializeElement<EShapedVector>(ofs, m_positionShapedVector);
	SerializeHelper::SerializeElement<XMFLOAT3>(ofs, m_positionOrigin);
	SerializeHelper::SerializeElement<XMVECTOR>(ofs, m_positionUpVector);

	SerializeHelper::SerializeElement<EShapedVector>(ofs, m_speedShapedVector);
	SerializeHelper::SerializeElement<XMFLOAT3>(ofs, m_speedOrigin);
	SerializeHelper::SerializeElement<XMVECTOR>(ofs, m_speedUpVector);

	SerializeHelper::SerializeElement<SControlPoint<4>>(ofs, m_colorInitControlPoint);
	SerializeHelper::SerializeElement<SControlPoint<4>>(ofs, m_colorFinalControlPoint);
	SerializeHelper::SerializeVector<SControlPoint<4>>(ofs, m_colorControlPoints);
	SerializeHelper::SerializeElement<EInterpolationMethod>(ofs, m_colorInterpolationMethod);

	SerializeHelper::SerializeElement<bool>(ofs, m_useGPUColorInterpolater);
}

void CBaseParticleSpawnProperty::Deserialize(std::ifstream& ifs)
{
	m_baseParticleSpawnPropertyCPU = SerializeHelper::DeserializeElement<decltype(m_baseParticleSpawnPropertyCPU)>(ifs);

	m_positionShapedVector = SerializeHelper::DeserializeElement<EShapedVector>(ifs);
	m_positionOrigin = SerializeHelper::DeserializeElement<XMFLOAT3>(ifs);
	m_positionUpVector = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);

	m_speedShapedVector = SerializeHelper::DeserializeElement<EShapedVector>(ifs);
	m_speedOrigin = SerializeHelper::DeserializeElement<XMFLOAT3>(ifs);
	m_speedUpVector = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);

	m_colorInitControlPoint = SerializeHelper::DeserializeElement<SControlPoint<4>>(ifs);
	m_colorFinalControlPoint = SerializeHelper::DeserializeElement<SControlPoint<4>>(ifs);
	m_colorControlPoints = SerializeHelper::DeserializeVector<SControlPoint<4>>(ifs);
	m_colorInterpolationMethod = SerializeHelper::DeserializeElement<EInterpolationMethod>(ifs);

	m_useGPUColorInterpolater = SerializeHelper::DeserializeElement<bool>(ifs);

	OnCheckGPUColorInterpolater();
	AdjustControlPointsFromLife();

	m_isParticleSpawnPropertyChanged = true;
}

