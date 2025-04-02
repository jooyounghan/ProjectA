#pragma once

namespace App
{
#ifdef APPMANAGER_EXPORTS
#define APPMANAGER_API __declspec(dllexport)
#else
#define APPMANAGER_API __declspec(dllimport)
#endif
}