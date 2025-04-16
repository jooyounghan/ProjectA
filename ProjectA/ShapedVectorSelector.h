#pragma once
#include <unordered_map>
#include <string>

enum class EShapedVector
{
	None,
	Sphere,
	HemiSphere,
	Cone
};

class ShapedVectorSelector
{
protected:
	static std::unordered_map<EShapedVector, std::string> ShapedVectorStringMap;
	
public:
	static void SelectShapedVector(EShapedVector& result);
};

