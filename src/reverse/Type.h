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

    Type(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::rtti::IType* apClass);
    virtual ~Type() = default;

    RED4ext::rtti::IType* GetType() const { return m_pType; }
    virtual void* GetHandle() const { return nullptr; }
    virtual RED4ext::rtti::IType* GetValueType() const { return m_pType; }
    virtual void* GetValuePtr() const { return nullptr; }

    sol::object Index(const std::string& acName, sol::this_environment aThisEnv);
    sol::object NewIndex(const std::string& acName, sol::optional<sol::object> aParam);

    virtual sol::object Index_Impl(const std::string& acName, sol::this_environment aThisEnv);
    virtual sol::object NewIndex_Impl(const std::string& acName, sol::object aParam);

    std::string GetName() const;
    virtual Descriptor Dump(bool aWithHashes) const;
    std::string GameDump() const;

    std::string FunctionDescriptor(RED4ext::CBaseFunction* apFunc, bool aWithHashes) const;

protected:
    RED4ext::rtti::IType* m_pType{nullptr};

    friend struct Scripting;

    TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref m_lua;
    // NOTE: this single unordered_map cannot be changed to TP::Map due to map corruption in function NewIndex_Impl
    std::unordered_map<std::string, sol::object> m_properties;
};

struct ClassType : Type
{
    ClassType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::rtti::IType* apClass);
    ~ClassType() override = default;

    Descriptor Dump(bool aWithHashes) const override;
    sol::object Index_Impl(const std::string& acName, sol::this_environment aThisEnv) override;
    sol::object NewIndex_Impl(const std::string& acName, sol::object aParam) override;

    RED4ext::CClass* GetClass() const { return reinterpret_cast<RED4ext::CClass*>(m_pType); }
};

struct UnknownType : Type
{
    UnknownType(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aView, RED4ext::rtti::IType* apType, void* apInstance);
    UnknownType(UnknownType&& aOther) noexcept;
    ~UnknownType() override;

    Descriptor Dump(bool aWithHashes) const override;
    void* GetHandle() const override;

private:
    void* m_pInstance;
};