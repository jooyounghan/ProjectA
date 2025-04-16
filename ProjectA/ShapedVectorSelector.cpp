#include "ShapedVectorSelector.h"
#include "imgui.h"

using namespace std;
using namespace ImGui;

unordered_map<EShapedVector, string> ShapedVectorSelector::ShapedVectorStringMap
{
	{ EShapedVector::None, "선택 안함" },
	{ EShapedVector::Sphere, "구" },
	{ EShapedVector::HemiSphere, "반구" },
	{ EShapedVector::Cone, "콘" }
};

void ShapedVectorSelector::SelectShapedVector(EShapedVector& result)
{
    PushID("Test");
	BeginCombo("벡터 모양", ShapedVectorStringMap[result].c_str());
	{
        for (auto& shapedVector : ShapedVectorStringMap)
        {
            EShapedVector shapedVectorType = shapedVector.first;
            const char* shapedVectorString = shapedVector.second.c_str();

            const bool is_selected = (result == shapedVectorType);
            if (ImGui::Selectable(shapedVector.second.c_str(), is_selected))
                result = shapedVectorType;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
	}
    PopID();
}
