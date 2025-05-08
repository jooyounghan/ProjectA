#include "ParticleRuntimeSpawnProperty.h"
#include "MacroUtilities.h"

using namespace std;
using namespace ImGui;

CParticleRuntimeSpawnProperty::CParticleRuntimeSpawnProperty(
	const function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
)
	: 
	CRuntimeSpawnProperty(gpuColorInterpolaterSelectedHandler, gpuColorInterpolaterUpdatedHandler)
{

}

void CParticleRuntimeSpawnProperty::DrawUIImpl()
{
	CRuntimeSpawnProperty::DrawUIImpl();

	SeparatorText("파티클 크기");
	if (DragFloat2("파티클 X-Y 크기", &m_runtimeSpawnPropertyCPU.xyScale.x, 0.01f, 0.f, 10.f, "%.2f"))
	{
		m_isRuntimeSpawnPropertyChanged = true;
	}
}
