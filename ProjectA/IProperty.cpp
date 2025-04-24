#include "IProperty.h"

APropertyOnEmitterTimeline::APropertyOnEmitterTimeline(
	float& emitterCurrentTime, 
	float& emitterLoopTime
)
	: m_emitterCurrentTime(emitterCurrentTime),
	m_emitterLoopTime(emitterLoopTime)
{
}
