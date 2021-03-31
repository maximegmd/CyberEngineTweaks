#include <stdafx.h>

#include "GameOptions.h"

static std::vector<GameOption*> s_gameOptions;

void* GameOption::s_booleanVtable{nullptr};
void* GameOption::s_integerVtable{nullptr};
void* GameOption::s_floatVtable{nullptr};
void* GameOption::s_stringVtable{nullptr};
void* GameOption::s_colorVtable{nullptr};

std::string GameOption::GetInfo()
{
    std::stringstream ret;

    if (pCategory)
        ret << pCategory << "/";

    if (pName)
        ret << pName;

    ret << " = ";
    ret << GetString();

    return ret.str();
}

std::string GameOption::GetString()
{
    std::stringstream ret;

    if (IsA(s_booleanVtable))
    {
        ret << (Boolean ? "true" : "false");
    }
    else if (IsA(s_integerVtable))
    {
        ret << Integer.Value;
    }
    else if (IsA(s_floatVtable))
    {
        ret << std::to_string(Float.Value);
    }
    else if(IsA(s_stringVtable))
    {
        ret << "\"" << String.c_str() << "\"";
    }
    else if(IsA(s_colorVtable))
    {
        ret << "0x" << std::hex << Integer.Value << std::dec;
    }

    return ret.str();
}

bool GameOption::GetBool(bool& retval)
{
    retval = false;
    if (!IsA(s_booleanVtable))
        return false;

    retval = Boolean;
    return true;
}

bool GameOption::GetInt(int& retval)
{
    retval = 0;
    if (!IsA(s_integerVtable) && !IsA(s_colorVtable))
        return false;

    retval = Integer.Value;
    return true;
}

bool GameOption::GetFloat(float& retval)
{
    retval = 0.f;
    if (!IsA(s_floatVtable))
        return false;

    retval = Float.Value;
    return true;
}

bool GameOption::GetColor(int& retval)
{
    retval = 0;
    if (!IsA(s_colorVtable))
        return false;

    retval = Integer.Value;
    return true;
}

bool GameOption::Set(const std::string& value)
{
    if (IsA(s_booleanVtable))
    {
        return SetBool(stricmp(value.c_str(), "true") == 0 || stricmp(value.c_str(), "1") == 0);
    }
    if (IsA(s_integerVtable))
    {
        return SetInt(std::stoi(value, nullptr, 0));
    }
    if (IsA(s_floatVtable))
    {
        return SetFloat(std::stof(value, nullptr));
    }
    if (IsA(s_stringVtable))
    {
        return SetString(value);
    }
    if (IsA(s_colorVtable))
    {
        return SetColor(std::stoi(value, nullptr, 0));
    }

    return false;
}

bool GameOption::SetBool(bool value)
{
    if (!IsA(s_booleanVtable))
        return false;

    Boolean = value;

    return true;
}

bool GameOption::SetInt(int value)
{
    if (!IsA(s_integerVtable) && !IsA(s_colorVtable))
        return false;

    Integer.Value = value;

    if (IsA(s_colorVtable))
        return true; // no min/max checks for colors

    if (Integer.Min == Integer.Max == 0.f)
        Integer.Value = value;
    else
        Integer.Value = std::clamp(value, Integer.Min, Integer.Max);

    return true;
}

bool GameOption::SetFloat(float value)
{
    if (!IsA(s_floatVtable))
        return false;

    if (Float.Min == Float.Max == 0.f)
        Float.Value = value;
    else
        Float.Value = std::clamp(value, Float.Min, Float.Max);

    return true;
}

bool GameOption::SetString(const std::string& value)
{
    return false; // Not sure how to edit REDString properly atm, probably need to call games REDString::Set function
}

bool GameOption::SetColor(int value)
{
    if (!IsA(s_colorVtable))
        return false;

    Integer.Value = value;

    return true;
}

bool GameOption::IsA(void* apVtable) const
{
    return *(void**)this == apVtable;
}

bool GameOption::Toggle()
{
    if (IsA(s_booleanVtable))
        return false;

    Boolean = !Boolean;

    return true;
}

GameOption* GameOptions::Find(const std::string& category, const std::string& name)
{
    auto option = std::find_if(
        s_gameOptions.begin(), s_gameOptions.end(),
        [&category, &name](GameOption* x)
        {
            return stricmp(x->pCategory, category.c_str()) == 0 && stricmp(x->pName, name.c_str()) == 0;
        });

    if (option == s_gameOptions.end())
    {
        spdlog::get("scripting")->info("Failed to find game option '{}/{}'!", category, name);
        return nullptr;;
    }

    return *option;
}

void GameOptions::Print(const std::string& category, const std::string& name)
{
    auto* option = Find(category, name);
    if (!option)
        return;
    
    spdlog::get("scripting")->info(option->GetInfo());
}

std::string GameOptions::Get(const std::string& category, const std::string& name)
{
    auto* option = Find(category, name);
    if (!option)
        return "";

    return option->GetString();
}

bool GameOptions::GetBool(const std::string& category, const std::string& name)
{
    auto* option = Find(category, name);
    if (!option)
        return false;

    bool value = false;
    bool result = option->GetBool(value);
    if (!result)
    {
        spdlog::get("scripting")->info("Failed to read game option '{}/{}', not a boolean?", category, name);
        return false;
    }

    return value;
}

int GameOptions::GetInt(const std::string& category, const std::string& name)
{
    auto* option = Find(category, name);
    if (!option)
        return false;

    int value = false;
    bool result = option->GetInt(value);
    if (!result)
    {
        spdlog::get("scripting")->info("Failed to read game option '{}/{}', not an integer/color?", category, name);
        return 0;
    }

    return value;
}

float GameOptions::GetFloat(const std::string& category, const std::string& name)
{
    auto* option = Find(category, name);
    if (!option)
        return false;

    float value = false;
    bool result = option->GetFloat(value);
    if (!result)
    {
        spdlog::get("scripting")->info("Failed to read game option '{}/{}', not a float?", category, name);
        return 0.f;
    }

    return value;
}

void GameOptions::Set(const std::string& category, const std::string& name, const std::string& value)
{
    auto* option = Find(category, name);
    if (!option)
        return;
    
    auto consoleLogger = spdlog::get("scripting");
    if (option->Set(value))
        consoleLogger->info(option->GetInfo());
    else
    {
        if (option->IsA(GameOption::s_stringVtable))
            consoleLogger->error("Failed to set game option '{}/{}', can't set string options right now.", category, name);
        else
            consoleLogger->error("Failed to set game option '{}/{}' due to an error (missing pointer?).", category, name);
    }
}

void GameOptions::SetBool(const std::string& category, const std::string& name, bool value)
{
    auto* option = Find(category, name);
    if (!option)
        return;
    
    auto consoleLogger = spdlog::get("scripting");
    if (option->SetBool(value))
        consoleLogger->info(option->GetInfo());
    else
    {
        if (!option->IsA(GameOption::s_booleanVtable))
            consoleLogger->error("Failed to set game option '{}/{}', not a boolean.", category, name);
        else
            consoleLogger->error("Failed to set game option '{}/{}' due to an error (missing pointer?).", category, name);
    }
}

void GameOptions::SetInt(const std::string& category, const std::string& name, int value)
{
    auto* option = Find(category, name);
    if (!option)
        return;
    
    auto consoleLogger = spdlog::get("scripting");
    if (option->SetInt(value))
        consoleLogger->info(option->GetInfo());
    else
    {
        if (!option->IsA(GameOption::s_integerVtable) && option->IsA(GameOption::s_colorVtable))
            consoleLogger->error("Failed to set game option '{}/{}', not an integer.", category, name);
        else
            consoleLogger->error("Failed to set game option '{}/{}' due to an error (missing pointer?).", category, name);
    }
}

void GameOptions::SetFloat(const std::string& category, const std::string& name, float value)
{
    auto* option = Find(category, name);
    if (!option)
        return;
    
    auto consoleLogger = spdlog::get("scripting");
    if (option->SetFloat(value))
        consoleLogger->info(option->GetInfo());
    else
    {
        if (!option->IsA(GameOption::s_floatVtable))
            consoleLogger->error("Failed to set game option '{}/{}', not a float.", category, name);
        else
            consoleLogger->error("Failed to set game option '{}/{}' due to an error (missing pointer?).", category, name);
    }
}

void GameOptions::Toggle(const std::string& category, const std::string& name)
{
    auto* option = Find(category, name);
    if (!option)
        return;

    auto consoleLogger = spdlog::get("scripting");
    if (option->Toggle())
        consoleLogger->info(option->GetInfo());
    else
    {
        if (option->IsA(GameOption::s_booleanVtable))
            consoleLogger->error("Failed to set game option '{}/{}', not a boolean.", category, name);
        else
            consoleLogger->error("Failed to set game option '{}/{}' due to an error (missing pointer?).", category, name);
    }
}

void GameOptions::Dump()
{
    for (auto option : s_gameOptions)
        spdlog::info(option->GetInfo());
    
    spdlog::get("scripting")->info("Dumped {} options to cyber_engine_tweaks.log", s_gameOptions.size());
}

void GameOptions::List(const std::string& category)
{
    auto consoleLogger = spdlog::get("scripting");

    int count = 0;
    auto iter = s_gameOptions.begin();
    while (iter != s_gameOptions.end())
    {
        iter = std::find_if(
            iter, s_gameOptions.end(),
            [&category](GameOption* x)
            {
                if (!category.length() || category.at(0) == '*')
                    return true;

                return stricmp(x->pCategory, category.c_str()) == 0;
            });

        if (iter != s_gameOptions.end())
        {
            consoleLogger->info((*iter)->GetInfo());
            iter++;
            count++;
        }
    }

    consoleLogger->info("Found {} options", count);
}

std::vector<GameOption*>& GameOptions::GetList()
{
    return s_gameOptions;
}
