#include "GlobalVariable.h"

using namespace DirectX;

const XMVECTOR GDirection::GDefaultForward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
const XMVECTOR GDirection::GDefaultUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
const XMVECTOR GDirection::GDefaultRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);