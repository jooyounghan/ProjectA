#include "ShapedVectorSelector.h"
#include "imgui.h"

using namespace std;
using namespace ImGui;

unordered_map<EShapedVector, string> ShapedVectorSelector::ShapedVectorStringMap
{
	{ EShapedVector::None, "���� ����" },
	{ EShapedVector::Sphere, "��" },
	{ EShapedVector::HemiSphere, "�ݱ�" },
	{ EShapedVector::Cone, "��" }
};

void ShapedVectorSelector::SelectShapedVector(EShapedVector& result)
{
    PushID("Test");
	BeginCombo("���� ���", ShapedVectorStringMap[result].c_str());
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
