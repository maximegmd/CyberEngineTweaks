#pragma once
#include <RED4ext/CName.hpp>

class RTTIExtender
{
    static void CreateSingleton(RED4ext::CName aTypeName);
    static void AddFunctionalTests();

public:
    static void Initialize();
};