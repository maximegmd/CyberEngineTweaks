#pragma once

struct Type
{
    struct Descriptor
    {
        std::string name{ "Unknown" };
        std::vector<std::string> functions;
        std::vector<std::string> staticFunctions;
        std::vector<std::string> properties;

        std::string ToString() const;
    };

    Type(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::IRTTIType* apClass);
    virtual ~Type(){};

    sol::object Index(const std::string& acName, sol::this_environment aThisEnv);
    sol::object NewIndex(const std::string& acName, sol::object aParam);

    virtual sol::object Index_Impl(const std::string& acName, sol::this_environment aThisEnv);
    virtual sol::object NewIndex_Impl(const std::string& acName, sol::object aParam);

    std::string GetName() const;
    virtual Descriptor Dump(bool aWithHashes) const;
    std::string GameDump();

    std::string FunctionDescriptor(RED4ext::CBaseFunction* apFunc, bool aWithHashes) const;
    sol::variadic_results Execute(RED4ext::CBaseFunction* apFunc, const std::string& acName, sol::variadic_args args, std::string& aReturnMessage);

protected:
    virtual RED4ext::ScriptInstance GetHandle() { return nullptr; }

    RED4ext::IRTTIType* m_pType{ nullptr };

    friend struct Scripting;

    TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref m_lua;
    std::unordered_map<std::string, sol::object> m_properties;
};

struct ClassType : Type
{
    ClassType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::IRTTIType* apClass);
    virtual ~ClassType(){};

    Descriptor Dump(bool aWithHashes) const override;
    sol::object Index_Impl(const std::string& acName, sol::this_environment aThisEnv) override;
    sol::object NewIndex_Impl(const std::string& acName, sol::object aParam) override;
};

struct UnknownType : Type
{
    UnknownType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView,
                RED4ext::IRTTIType* apClass,
                RED4ext::ScriptInstance apInstance);

    Descriptor Dump(bool aWithHashes) const override;
    RED4ext::ScriptInstance GetHandle()  override { return m_pInstance.get(); }

private:
    std::unique_ptr<uint8_t[]> m_pInstance;
};