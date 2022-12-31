#include "stdafx.h"

#include "GameOptions.h"

static TiltedPhoques::Vector<GameOption*> s_gameOptions;

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

    if (GetType() == kBoolean)
    {
        ret << (Boolean ? "true" : "false");
    }
    else if (GetType() == kInteger)
    {
        ret << Integer.Value;
    }
    else if (GetType() == kFloat)
    {
        ret << std::to_string(Float.Value);
    }
    else if (GetType() == kString)
    {
        ret << "\"" << String.c_str() << "\"";
    }
    else if (GetType() == kColor)
    {
        ret << "0x" << std::hex << Integer.Value << std::dec;
    }

    return ret.str();
}

bool GameOption::GetBool(bool& retval)
{
    return Get(&retval, kBoolean);
}

bool GameOption::GetInt(int& retval)
{
    return Get(&retval, kInteger);
}

bool GameOption::GetFloat(float& retval)
{
    return Get(&retval, kFloat);
}

bool GameOption::GetColor(int& retval)
{
    return Get(&retval, kColor);
}

bool GameOption::Set(const std::string& value)
{
    if (GetType() == kBoolean)
    {
        return SetBool(_stricmp(value.c_str(), "true") == 0 || _stricmp(value.c_str(), "1") == 0);
    }
    if (GetType() == kInteger)
    {
        return SetInt(std::stoi(value, nullptr, 0));
    }
    if (GetType() == kFloat)
    {
        return SetFloat(std::stof(value, nullptr));
    }
    if (GetType() == kString)
    {
        return SetString(value);
    }
    if (GetType() == kColor)
    {
        return SetColor(std::stoi(value, nullptr, 0));
    }

    return false;
}

bool GameOption::SetBool(bool value)
{
    return Set(&value, kBoolean);
}

bool GameOption::SetInt(int value)
{
    return Set(&value, kInteger);
}

bool GameOption::SetFloat(float value)
{
    return Set(&value, kFloat);
}

bool GameOption::SetString(const std::string& value)
{
    RED4ext::CString str(value.c_str());
    return Set(&str, kString);
}

bool GameOption::SetColor(int value)
{
    return Set(&value, kColor);
}

bool GameOption::Toggle()
{
    if (GetType() != kBoolean)
        return false;

    Boolean = !Boolean;

    return true;
}

GameOption* GameOptions::Find(const std::string& category, const std::string& name)
{
    const auto option = std::ranges::find_if(
        s_gameOptions, [&category, &name](const GameOption* x) { return _stricmp(x->pCategory, category.c_str()) == 0 && _stricmp(x->pName, name.c_str()) == 0; });

    if (option == s_gameOptions.end())
    {
        spdlog::get("scripting")->info("Failed to find game option '{}/{}'!", category, name);
        return nullptr;
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
    const bool result = option->GetBool(value);
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
    const bool result = option->GetInt(value);
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
    const bool result = option->GetFloat(value);
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

    const auto consoleLogger = spdlog::get("scripting");
    if (option->Set(value))
        consoleLogger->info(option->GetInfo());
    else
    {
        if (option->GetType() == GameOption::kString)
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

    const auto consoleLogger = spdlog::get("scripting");
    if (option->SetBool(value))
        consoleLogger->info(option->GetInfo());
    else
    {
        if (option->GetType() != GameOption::kBoolean)
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

    const auto consoleLogger = spdlog::get("scripting");
    if (option->SetInt(value))
        consoleLogger->info(option->GetInfo());
    else
    {
        if (option->GetType() != GameOption::kInteger && option->GetType() != GameOption::kColor)
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

    const auto consoleLogger = spdlog::get("scripting");
    if (option->SetFloat(value))
        consoleLogger->info(option->GetInfo());
    else
    {
        if (option->GetType() != GameOption::kFloat)
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

    const auto consoleLogger = spdlog::get("scripting");
    if (option->Toggle())
        consoleLogger->info(option->GetInfo());
    else
    {
        if (option->GetType() != GameOption::kBoolean)
            consoleLogger->error("Failed to set game option '{}/{}', not a boolean.", category, name);
        else
            consoleLogger->error("Failed to set game option '{}/{}' due to an error (missing pointer?).", category, name);
    }
}

void GameOptions::Dump()
{
    for (const auto option : s_gameOptions)
        Log::Info(option->GetInfo());

    spdlog::get("scripting")->info("Dumped {} options to cyber_engine_tweaks.log", s_gameOptions.size());
}

void GameOptions::List(const std::string& category)
{
    const auto consoleLogger = spdlog::get("scripting");

    int count = 0;
    auto iter = s_gameOptions.begin();
    while (iter != s_gameOptions.end())
    {
        iter = std::find_if(
            iter, s_gameOptions.end(),
            [&category](const GameOption* x)
            {
                if (!category.length() || category.at(0) == '*')
                    return true;

                return _stricmp(x->pCategory, category.c_str()) == 0;
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

TiltedPhoques::Vector<GameOption*>& GameOptions::GetList()
{
    return s_gameOptions;
}
