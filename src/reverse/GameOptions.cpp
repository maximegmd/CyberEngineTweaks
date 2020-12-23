#include <string>
#include <spdlog/spdlog.h>
#include <sstream>

#include "GameOptions.h"
#include "overlay/Overlay.h"
#include "RED4ext/REDreverse/CString.hpp"

static std::vector<GameOption*> s_gameOptions;

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

    switch (type)
    {
    case GameOptionType::Boolean:
        if (pBoolean)
            ret << (*pBoolean ? "true" : "false");
        break;
    case GameOptionType::Integer:
        if (pInteger)
            ret << *pInteger;
        break;
    case GameOptionType::Float:
        if (pFloat)
            ret << std::to_string(*pFloat);
        break;
    case GameOptionType::String:
        if (pString)
            ret << "\"" << pString->ToString() << "\"";
        break;
    case GameOptionType::Color:
        if (pInteger)
            ret << "0x" << std::hex << *pInteger << std::dec;
        break;
    }

    return ret.str();
}

bool GameOption::GetBool(bool& retval)
{
    retval = false;
    if (type != GameOptionType::Boolean)
        return false;
    if (!pBoolean)
        return false;

    retval = *pBoolean;
    return true;
}

bool GameOption::GetInt(int& retval)
{
    retval = 0;
    if (type != GameOptionType::Integer && type != GameOptionType::Color)
        return false;
    if (!pInteger)
        return false;

    retval = *pInteger;
    return true;
}

bool GameOption::GetFloat(float& retval)
{
    retval = 0.f;
    if (type != GameOptionType::Float)
        return false;
    if (!pFloat)
        return false;

    retval = *pFloat;
    return true;
}

bool GameOption::GetColor(int& retval)
{
    retval = 0;
    if (type != GameOptionType::Color)
        return false;
    if (!pInteger)
        return false;

    retval = *pInteger;
    return true;
}

bool GameOption::Set(const std::string& value)
{
    switch (this->type)
    {
    case GameOptionType::Boolean:
        return SetBool(stricmp(value.c_str(), "true") == 0 || stricmp(value.c_str(), "1") == 0);

    case GameOptionType::Integer:
        return SetInt(std::stoi(value, nullptr, 0));

    case GameOptionType::Float:
        return SetFloat(std::stof(value, nullptr));

    case GameOptionType::String:
        return SetString(value);

    case GameOptionType::Color:
        return SetColor(std::stoi(value, nullptr, 0));

    }

    return false;
}

bool GameOption::SetBool(bool value)
{
    if (type != GameOptionType::Boolean)
        return false;

    if (!pBoolean)
        return false;

    *pBoolean = value;

    return true;
}

bool GameOption::SetInt(int value)
{
    if (type != GameOptionType::Integer && type != GameOptionType::Color)
        return false;

    if (!pInteger)
        return false;

    *pInteger = value;

    if (type == GameOptionType::Color)
        return true; // no min/max checks for colors

    if (pIntegerMin && *pIntegerMin > *pInteger)
        *pInteger = *pIntegerMin;

    if (pIntegerMax && *pIntegerMax < *pInteger)
        *pInteger = *pIntegerMax;

    return true;
}

bool GameOption::SetFloat(float value)
{
    if (type != GameOptionType::Float)
        return false;

    if (!pFloat)
        return false;

    *pFloat = value;

    if (pFloatMin && *pFloatMin > *pFloat)
        *pFloat = *pFloatMin;

    if (pFloatMax && *pFloatMax < *pFloat)
        *pFloat = *pFloatMax;

    return true;
}

bool GameOption::SetString(const std::string& value)
{
    return false; // Not sure how to edit REDString properly atm, probably need to call games REDString::Set function
}

bool GameOption::SetColor(int value)
{
    if (type != GameOptionType::Color)
        return false;

    if (!pInteger)
        return false;

    *pInteger = value;

    return true;
}

bool GameOption::Toggle()
{
    if (type != GameOptionType::Boolean)
        return false;

    if (!pBoolean)
        return false;

    *pBoolean = !*pBoolean;

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
        Overlay::Get().Log("Failed to find game option '" + category + "/" + name + "'!");
        return nullptr;;
    }

    return *option;
}

void GameOptions::Print(const std::string& category, const std::string& name)
{
    auto* option = Find(category, name);
    if (!option)
        return;

    Overlay::Get().Log(option->GetInfo());
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
        Overlay::Get().Log("Failed to read game option '" + category + "/" + name + "', not a boolean?");
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
        Overlay::Get().Log("Failed to read game option '" + category + "/" + name + "', not an integer/color?");
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
        Overlay::Get().Log("Failed to read game option '" + category + "/" + name + "', not a float?");
        return 0.f;
    }

    return value;
}

void GameOptions::Set(const std::string& category, const std::string& name, const std::string& value)
{
    auto* option = Find(category, name);
    if (!option)
        return;

    if (option->Set(value))
        Overlay::Get().Log(option->GetInfo());
    else
    {
        if (option->type == GameOptionType::String)
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "', can't set string options right now.");
        else
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "' due to an error (missing pointer?).");
    }
}

void GameOptions::SetBool(const std::string& category, const std::string& name, bool value)
{
    auto* option = Find(category, name);
    if (!option)
        return;

    if (option->SetBool(value))
        Overlay::Get().Log(option->GetInfo());
    else
    {
        if (option->type != GameOptionType::Boolean)
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "', not a boolean.");
        else
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "' due to an error (missing pointer?).");
    }
}

void GameOptions::SetInt(const std::string& category, const std::string& name, int value)
{
    auto* option = Find(category, name);
    if (!option)
        return;

    if (option->SetInt(value))
        Overlay::Get().Log(option->GetInfo());
    else
    {
        if (option->type != GameOptionType::Integer && option->type != GameOptionType::Color)
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "', not an integer.");
        else
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "' due to an error (missing pointer?).");
    }
}

void GameOptions::SetFloat(const std::string& category, const std::string& name, float value)
{
    auto* option = Find(category, name);
    if (!option)
        return;

    if (option->SetFloat(value))
        Overlay::Get().Log(option->GetInfo());
    else
    {
        if (option->type != GameOptionType::Float)
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "', not a float.");
        else
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "' due to an error (missing pointer?).");
    }
}

void GameOptions::Toggle(const std::string& category, const std::string& name)
{
    auto* option = Find(category, name);
    if (!option)
        return;

    if (option->Toggle())
        Overlay::Get().Log(option->GetInfo());
    else
    {
        if (option->type != GameOptionType::Boolean)
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "', not a boolean.");
        else
            Overlay::Get().Log("Failed to set game option '" + category + "/" + name + "' due to an error (missing pointer?).");
    }
}

void GameOptions::Dump()
{
    for (auto option : s_gameOptions)
    {
        spdlog::info(option->GetInfo());
    }

    Overlay::Get().Log("Dumped " + std::to_string(s_gameOptions.size()) + " options to cyber_engine_tweaks.log");
}

void GameOptions::List(const std::string& category)
{
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
            Overlay::Get().Log((*iter)->GetInfo());
            iter++;
            count++;
        }
    }

    Overlay::Get().Log("Found " + std::to_string(count) + " options");
}

std::vector<GameOption*>& GameOptions::GetList()
{
    return s_gameOptions;
}
