#pragma once

struct Type
{
    struct Descriptor
    {
        std::string name{"Unknown"};
        TiltedPhoques::Vector<std::string> functions;
        TiltedPhoques::Vector<std::string> staticFunctions;
        TiltedPhoques::Vector<std::string> properties;

        std::string ToString() const;
    };

    Type(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, red3lib::IRTTIType* apClass);
    virtual ~Type() = default;

    red3lib::IRTTIType* GetType() const { return m_pType; }
    virtual red3lib::IScriptable* GetHandle() const { return nullptr; }
    virtual red3lib::IScriptable* GetValuePtr() const { return nullptr; }

    sol::object Index(const std::string& acName, sol::this_environment aThisEnv);
    sol::object NewIndex(const std::string& acName, sol::object aParam);

    virtual sol::object Index_Impl(const std::string& acName, sol::this_environment aThisEnv);
    virtual sol::object NewIndex_Impl(const std::string& acName, sol::object aParam);

    std::string GetName() const;
    virtual Descriptor Dump(bool aWithHashes) const;
    std::string GameDump() const;

    std::string FunctionDescriptor(red3lib::CFunction* apFunc, bool aWithHashes) const;

protected:
    red3lib::IRTTIType* m_pType{nullptr};

    friend struct Scripting;

    TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref m_lua;
    // NOTE: this single unordered_map cannot be changed to TP::Map due to map corruption in function NewIndex_Impl
    std::unordered_map<std::string, sol::object> m_properties;
};

struct ClassType : Type
{
    ClassType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, red3lib::IRTTIType* apClass);
    ~ClassType() override = default;

    Descriptor Dump(bool aWithHashes) const override;
    sol::object Index_Impl(const std::string& acName, sol::this_environment aThisEnv) override;
    sol::object NewIndex_Impl(const std::string& acName, sol::object aParam) override;

    red3lib::CClass* GetClass() const { return reinterpret_cast<red3lib::CClass*>(m_pType); }
};
