#include "SpriteSpawnProperty.h"
#include "MacroUtilities.h"
#include "InitialPropertyDefinition.h"

using namespace std;
using namespace DirectX;
using namespace ImGui;

SpriteSpawnProperty::SpriteSpawnProperty(
	uint32_t maxEmitterCount,
	const function<void(const SSpriteInterpInformation&)>& spriteInterpInformationChangedHandler
)
	: 
	ARuntimeSpawnProperty(maxEmitterCount),
	m_onSpriteInterpInformationChanged(spriteInterpInformationChangedHandler),
	m_checkGPUSpriteSizeInterpolater(false),
	m_spriteSizeInitControlPoint{ 0.f, MakeArray(0.f, 0.f) },
	m_spriteSizeFinalControlPoint{ InitLife, MakeArray(10.f, 10.f) },
	m_spriteSizeInterpolationMethod(EInterpolationMethod::Linear)
{
	AutoZeroMemory(m_spriteSizeInterpInformation);

	m_spriteSizeControlPointGridView = make_unique<CControlPointGridView<2>>(
		"�ð�",
		array<string, 2>{ "X ũ��", "Y ũ��" },
		"��������Ʈ ũ��",
		0.01f, 0.f, 10.f,
		m_spriteSizeInitControlPoint,
		m_spriteSizeFinalControlPoint,
		m_spriteSizeControlPoints
	);

	m_spriteSizeInterpolationSelectPlotter = make_unique<CInterpolaterSelectPlotter<2>>(
		"��������Ʈ ũ�� ���� ���",
		"Sprite Size Control Points",
		array<string, 2>{ "X ũ��", "Y ũ��" },
		m_spriteSizeInitControlPoint,
		m_spriteSizeFinalControlPoint,
		m_spriteSizeControlPoints
	);

	m_spriteSizeInterpolationSelectPlotter->CreateInterpolater(
		m_d1Dim2PorpertyManager.get(),
		m_d3Dim2PorpertyManager.get(),
		m_spriteSizeInterpolationMethod, 
		m_spriteSizeInterpolater
	);
	m_spriteSizeInterpolationSelectPlotter->ResetXYScale();
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
}

void SpriteSpawnProperty::OnInterpolateInformationChagned()
{
	m_spriteSizeInterpInformation.maxLife = m_baseParticleSpawnPropertyCPU.maxLife;
	m_spriteSizeInterpInformation.colorInterpolaterID = m_colorInterpolater->GetInterpolaterID();
	m_spriteSizeInterpInformation.colorInterpolaterDegree = m_colorInterpolater->GetCoefficientCount();
	m_spriteSizeInterpInformation.spriteSizeInterpolaterID = m_spriteSizeInterpolater->GetInterpolaterID();
	m_spriteSizeInterpInformation.spriteSizeInterpolaterDegree = m_spriteSizeInterpolater->GetCoefficientCount();
	m_onSpriteInterpInformationChanged(m_spriteSizeInterpInformation);
}

void SpriteSpawnProperty::DrawPropertyUIImpl()
{
	ARuntimeSpawnProperty::DrawPropertyUIImpl();

	SeparatorText("��������Ʈ ũ�� ����");
	EInterpolationMethod currnetSpriteSizeInterpolateKind = m_spriteSizeInterpolationMethod;
	m_spriteSizeInterpolationSelectPlotter->SelectEnums(currnetSpriteSizeInterpolateKind);
	if (m_spriteSizeInterpolationMethod != currnetSpriteSizeInterpolateKind)
	{
		m_spriteSizeInterpolationMethod = currnetSpriteSizeInterpolateKind;
		m_spriteSizeInterpolationSelectPlotter->CreateInterpolater(
			m_d1Dim2PorpertyManager.get(),
			m_d3Dim2PorpertyManager.get(), 
			m_spriteSizeInterpolationMethod, 
			m_spriteSizeInterpolater
		);
		OnInterpolateInformationChagned();
	}

	if (Checkbox("GPU ��� ��������Ʈ ũ�� ����", &m_checkGPUSpriteSizeInterpolater))
	{
		m_spriteSizeInterpolationSelectPlotter->CreateInterpolater(
			m_d1Dim2PorpertyManager.get(),
			m_d3Dim2PorpertyManager.get(),
			m_spriteSizeInterpolationMethod,
			m_spriteSizeInterpolater
		);
		OnInterpolateInformationChagned();
	}

	if (m_spriteSizeControlPointGridView->DrawControlPointGridView())
	{
		m_spriteSizeInterpolater->UpdateCoefficient();
		m_spriteSizeInterpolationSelectPlotter->ResetXYScale();
	}

	m_spriteSizeInterpolationSelectPlotter->ViewInterpolatedPlots();
}
