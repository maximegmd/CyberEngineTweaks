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
    
    Type(sol::state_view aView, RED4ext::IRTTIType* apClass);

    sol::object Index(const std::string& acName);
    sol::object NewIndex(const std::string& acName, sol::object aParam);

	virtual sol::object Index_Impl(const std::string& acName);
	virtual sol::object NewIndex_Impl(const std::string& acName, sol::object aParam);

    std::string GetName() const;
    virtual Descriptor Dump(bool aWithHashes) const;
    std::string GameDump();
    
	std::string FunctionDescriptor(RED4ext::CBaseFunction* apFunc, bool aWithHashes) const;
	sol::variadic_results Execute(RED4ext::CBaseFunction* apFunc, const std::string& acName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage);

protected:
    virtual RED4ext::ScriptInstance GetHandle() { return nullptr; }

    RED4ext::IRTTIType* m_pType{ nullptr };

    friend struct Scripting;
    
    sol::state_view m_lua;
    std::unordered_map<std::string, sol::object> m_properties;
};

struct ClassType : Type
{
    ClassType(sol::state_view aView, RED4ext::IRTTIType* apClass);

	virtual Descriptor Dump(bool aWithHashes) const;
	virtual sol::object Index_Impl(const std::string& acName);
    virtual sol::object NewIndex_Impl(const std::string& acName, sol::object aParam);
};

struct UnknownType : Type
{
    UnknownType(sol::state_view aView, RED4ext::IRTTIType* apClass, RED4ext::ScriptInstance apInstance);

	virtual Descriptor Dump(bool aWithHashes) const;
    virtual RED4ext::ScriptInstance GetHandle() { return m_pInstance.get(); }

private:
    std::unique_ptr<uint8_t[]> m_pInstance;
};