#pragma once
#include "imgui.h"

#include <DirectXMath.h>
#include <string>
#include <unordered_map>
#include <concepts>

template<typename T>
concept IsEnumClass = std::is_enum_v<T> && !std::is_convertible_v<T, int>;


template<IsEnumClass T>
class CBaseSelector
{
public:
	CBaseSelector(const std::string& selectorName, const std::unordered_map<T, std::string>& stringMaps);
	virtual ~CBaseSelector() = default;

protected:
	std::string m_selectorName;
	std::unordered_map<T, std::string> m_stringMaps;

public:
	bool SelectEnums(T& enumResult);
};

template<IsEnumClass T>
inline CBaseSelector<T>::CBaseSelector(
	const std::string& selectorName, 
	const std::unordered_map<T, std::string>& stringMaps
)
	: m_selectorName(selectorName), m_stringMaps(stringMaps)
{
}

template<IsEnumClass T>
bool CBaseSelector<T>::SelectEnums(T& enumResult)
{
	bool result = false;
	if (ImGui::BeginCombo(m_selectorName.c_str(), m_stringMaps.at(enumResult).c_str()))
	{
		for (auto& stringMap : m_stringMaps)
		{
			T enumType = stringMap.first;
			const char* enumString = stringMap.second.c_str();

			const bool isSelected = (enumResult == enumType);
			if (ImGui::Selectable(enumString, isSelected))
			{
				enumResult = enumType;
				result = true;
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return result;
}
