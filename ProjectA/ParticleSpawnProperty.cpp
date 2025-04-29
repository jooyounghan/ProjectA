#include "ParticleSpawnProperty.h"
#include "MacroUtilities.h"

using namespace std;
using namespace ImGui;

ParticleSpawnProperty::ParticleSpawnProperty(
	const function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
)
	: 
	ARuntimeSpawnProperty(gpuColorInterpolaterSelectedHandler, gpuColorInterpolaterUpdatedHandler)
{

}

void ParticleSpawnProperty::DrawPropertyUIImpl()
{
	ARuntimeSpawnProperty::DrawPropertyUIImpl();

	SeparatorText("��ƼŬ ũ��");
	if (DragFloat2("��ƼŬ X-Y ũ��", &m_runtimeSpawnPropertyCPU.xyScale.x, 0.01f, 0.f, 10.f, "%.2f"))
	{
		m_isRuntimeSpawnPropertyChanged = true;
	}
}
