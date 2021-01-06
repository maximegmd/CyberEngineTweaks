#pragma once

struct Type
{
    struct Descriptor
    {
        std::string name{"Unknown"};
        std::vector<std::string> functions;
        std::vector<std::string> staticFunctions;
        std::vector<std::string> properties;

        std::string ToString() const;
    };
    
    Type(sol::state_view aView, RED4ext::CClass* apClass);

    sol::object Index(const std::string& acName);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::protected_function InternalIndex(const std::string& acName);
    std::string GetName() const;
    Descriptor Dump(bool aWithHashes) const;
    std::string GameDump();
    std::string FunctionDescriptor(RED4ext::CBaseFunction* apFunc, bool aWithHashes) const;
    
    sol::variadic_results Execute(RED4ext::CClassFunction* apFunc, const std::string& acName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage);

protected:
    virtual RED4ext::ScriptInstance GetHandle() { return nullptr; }

    RED4ext::CClass* m_pType{ nullptr };

    friend struct Scripting;
    
private:
    sol::state_view m_lua;
    std::unordered_map<std::string, sol::object> m_properties;
};

