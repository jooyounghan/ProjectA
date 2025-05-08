#pragma once
#include "BaseSelector.h"
#include "EmitterTypeDefinition.h"

#include <memory>

class AEmitter;
class CInitialSpawnProperty;
class CEmitterUpdateProperty;
class CRuntimeSpawnProperty;
class ForceUpdateProperty;

class CEmitterSelector : public CBaseSelector<EEmitterType>
{
public:
	CEmitterSelector(const std::string& selectorName);
	~CEmitterSelector() override = default;
};

