#pragma once
#include "imgui.h"

#include <DirectXMath.h>
#include <string>
#include <unordered_map>
#include <concepts>

template<typename T>
concept IsEnumClass = std::is_enum_v<T> && !std::is_convertible_v<T, int>;


template<IsEnumClass T>
class BaseSelector
{
public:
	static void SelectEnums(const std::string& selectorName, const std::unordered_map<T, std::string>& stringMaps, T& enumResult);
};

template<IsEnumClass T>
void BaseSelector<T>::SelectEnums(const std::string& selectorName, const std::unordered_map<T, std::string>& stringMaps, T& enumResult)
{
	if (ImGui::BeginCombo(selectorName.c_str(), stringMaps.at(enumResult).c_str()))
	{
		for (auto& stringMap : stringMaps)
		{
			T enumType = stringMap.first;
			const char* enumString = stringMap.second.c_str();

			const bool isSelected = (enumResult == enumType);
			if (ImGui::Selectable(enumString, isSelected))
				enumResult = enumType;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}
