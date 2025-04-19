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
	BaseSelector(const std::string& selectorName, const std::unordered_map<T, std::string>& stringMaps);
	virtual ~BaseSelector() = default;

protected:
	std::string m_selectorName;
	std::unordered_map<T, std::string> m_stringMaps;

public:
	void SelectEnums(T& enumResult);
};

template<IsEnumClass T>
inline BaseSelector<T>::BaseSelector(
	const std::string& selectorName, 
	const std::unordered_map<T, std::string>& stringMaps
)
	: m_selectorName(selectorName), m_stringMaps(stringMaps)
{
}

template<IsEnumClass T>
void BaseSelector<T>::SelectEnums(T& enumResult)
{
	if (ImGui::BeginCombo(m_selectorName.c_str(), m_stringMaps.at(enumResult).c_str()))
	{
		for (auto& stringMap : m_stringMaps)
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
