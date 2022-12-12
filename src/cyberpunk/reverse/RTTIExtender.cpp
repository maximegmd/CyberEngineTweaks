#include "RTTIExtender.h"

#include <RED4ext/SharedMutex.hpp>
#include <RED4ext/Scripting/Natives/Generated/Transform.hpp>
#include <RED4ext/Scripting/Natives/Generated/WorldTransform.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/Entity.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/EntityID.hpp>

template <typename T> struct GameCall
{
    GameCall(uintptr_t aAddress, const int32_t acOffset = 0)
    {
        const RelocPtr<uint8_t> addr(aAddress);
        const auto* pLocation = addr.GetAddr();
        m_address = pLocation ? reinterpret_cast<T>(pLocation + acOffset) : nullptr;
    }

    operator T() const
    {
        assert(m_address != nullptr);
        return m_address;
    }

private:
    T m_address;
};

#pragma region To be moved to RED4Ext.SDK once reversed

// what? why not Handle? what to call this?
// T must have AddRef and DecRef
template <typename T> struct REDSmartPtr
{
    REDSmartPtr(T* aData = nullptr)
        : data(aData)
    {
        AddRef();
    }

    REDSmartPtr(const REDSmartPtr& acOther)
        : data(acOther.data)
    {
        AddRef();
    }

    REDSmartPtr(REDSmartPtr&& aOther) noexcept
        : data(aOther.data)
    {
        aOther.data = nullptr;
    }

    ~REDSmartPtr() { Reset(); }

    void Reset()
    {
        if (data != nullptr)
        {
            data->DecRef();
            data = nullptr;
        }
    }

    REDSmartPtr& operator=(const REDSmartPtr& acOther)
    {
        if (this == &acOther)
            return *this;

        Reset();
        data = acOther.data;
        AddRef();

        return *this;
    }

    REDSmartPtr& operator=(REDSmartPtr&& aOther) noexcept
    {
        if (this == &aOther)
            return *this;

        Reset();
        data = aOther.data;
        aOther.data = nullptr;

        return *this;
    }

    [[nodiscard]] T* operator->() { return data; }

    [[nodiscard]] const T* operator->() const { return data; }

    explicit operator bool() const noexcept { return data != nullptr; }

private:
    void AddRef()
    {
        if (data != nullptr)
        {
            data->AddRef();
        }
    }

public:
    T* data;
};

struct IUpdatableSystem : RED4ext::IScriptable
{
    virtual void sub_110(uint64_t) = 0; // probably Update
};

struct gameIGameSystem : IUpdatableSystem
{
    RED4ext::GameInstance* gameInstance = nullptr;

    static void CallConstructor(void* apAddress)
    {
        // expected: 2, index: 0
        using TFunc = void (*)(void*);
        static GameCall<TFunc> func(Game::Addresses::gameIGameSystem_Constructor, -6);
        func(apAddress); // gameIGameSystem::ctor()
    }

    virtual void OnInitialize() = 0; // 118
    virtual void OnShutdown() = 0;   // 120
    virtual void sub_128() = 0;
    virtual void sub_130() = 0;
    virtual void sub_138() = 0;
    virtual void sub_140() = 0;
    virtual void sub_148() = 0;
    virtual void sub_150() = 0;
    virtual void sub_158() = 0;
    virtual void sub_160() = 0;
    virtual void sub_168() = 0;
    virtual void sub_170() = 0;
    virtual void sub_178() = 0;
    virtual void sub_180() = 0;
    virtual void sub_188() = 0;
    virtual void sub_190() = 0;
    virtual void OnSystemInitializeAsync(void*) = 0;   // 198
    virtual void OnSystemUnInitializeAsync(void*) = 0; // 1A0
};
RED4EXT_ASSERT_OFFSET(gameIGameSystem, gameInstance, 0x40);

struct TEMP_PendingEntity
{
    struct Unk00
    {
        // TEMP_SpawnSettings without the first 8 bytes???
        uint8_t unk08[0x100];
        RED4ext::CName entityPath;
        RED4ext::ent::EntityID entityID;
        RED4ext::ent::Entity* entity;
        // ...

        virtual void sub_00() = 0;
        virtual ~Unk00() = 0;
        virtual void sub_10() = 0;
        virtual void sub_18() = 0;
        virtual void sub_20() = 0;
        virtual void DecRef() = 0;
        virtual void AddRef() = 0;
    };
    RED4EXT_ASSERT_OFFSET(Unk00, entityID, 0x110);
    RED4EXT_ASSERT_OFFSET(Unk00, entity, 0x118);

    Unk00* unk00;
    uint64_t unk08;
    uint64_t unk18;
    uint64_t unk20;
    uint64_t unk28;
};
RED4EXT_ASSERT_SIZE(TEMP_PendingEntity, 5 * 8);

struct TEMP_SpawnSettings
{
    uintptr_t unk00 = 0;
    uintptr_t unk08 = 0;
    RED4ext::Transform transform{};
    // uint8_t unk30[0x38]{};
    // uintptr_t unk68 = 0;
    std::function<void(TEMP_PendingEntity::Unk00&)> callback;
    uint8_t unk70[0x18]{};
    uintptr_t unk88 = 0;
    uintptr_t unk90 = 0;
    uintptr_t unk98 = 0;
    RED4ext::Handle<RED4ext::IScriptable> unkA0;
    RED4ext::Handle<RED4ext::IScriptable> unkB0;
    RED4ext::CName appearance = "default";
    uintptr_t unkC8 = 0;
    uintptr_t unkD0 = 0;
    uintptr_t unkD8 = 0;
    RED4ext::TweakDBID DONOTUSE_recordDBID = 0;
    uint32_t unkE8 = 0x10101FF;

    void SetTransform(const RED4ext::WorldTransform& acWorldTransform)
    {
        constexpr auto FixedToFloat = [](const int32_t acValue)
        {
            return acValue * (1.f / (2 << 16));
        };

        transform.position.X = FixedToFloat(acWorldTransform.Position.x.Bits);
        transform.position.Y = FixedToFloat(acWorldTransform.Position.y.Bits);
        transform.position.Z = FixedToFloat(acWorldTransform.Position.z.Bits);
        transform.position.W = 0.0f;

        // ------------------------------

        // Normalize Quats
        const float quatLength = sqrtf(
            acWorldTransform.Orientation.i * acWorldTransform.Orientation.i + acWorldTransform.Orientation.j * acWorldTransform.Orientation.j +
            acWorldTransform.Orientation.k * acWorldTransform.Orientation.k + acWorldTransform.Orientation.r * acWorldTransform.Orientation.r);
        if (quatLength != 0.0f)
        {
            transform.orientation.i = acWorldTransform.Orientation.i / quatLength;
            transform.orientation.j = acWorldTransform.Orientation.j / quatLength;
            transform.orientation.k = acWorldTransform.Orientation.k / quatLength;
            transform.orientation.r = acWorldTransform.Orientation.r / quatLength;
        }
    }

    void SetRecordID(const RED4ext::TweakDBID acTweakDBID)
    {
        // Copied from the function photomode uses to spawn 3rd person puppet
        using TFunc = void (*)(const RED4ext::TweakDBID, RED4ext::Handle<RED4ext::IScriptable>&);
        static GameCall<TFunc> func(Game::Addresses::CPhotoMode_SetRecordID);

        DONOTUSE_recordDBID = acTweakDBID;
        func(acTweakDBID, unkB0);
    }
};
RED4EXT_ASSERT_OFFSET(TEMP_SpawnSettings, unk70, 0x70);
RED4EXT_ASSERT_OFFSET(TEMP_SpawnSettings, DONOTUSE_recordDBID, 0xE0);
RED4EXT_ASSERT_SIZE(TEMP_SpawnSettings, 0xF0); // Guessed. ctor is inlined

struct TEMP_Spawner
{
    RED4ext::DynArray<void*> unk00;
    RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>> spawnedEntities;
    RED4ext::DynArray<TEMP_PendingEntity> pendingEntities;
    RED4ext::DynArray<void*> unk30;
    uint8_t unk40 = 0;                       // most likely a mutex
    RED4ext::SharedMutex entitiesMtx;        // used in DespawnEntity
    RED4ext::SharedMutex pendingEntitiesMtx; // used in SpawnEntity
    uintptr_t unk48 = 0;
    uintptr_t unk50 = 0;
    uintptr_t unk58 = 0;
    uintptr_t unk60 = 0;

    TEMP_Spawner(RED4ext::Memory::IAllocator* apAllocator = nullptr)
        : unk00(apAllocator)
        , spawnedEntities(apAllocator)
        , pendingEntities(apAllocator)
        , unk30(apAllocator)
    {
    }

    void Initialize(RED4ext::GameInstance* apGameInstance)
    {
        using TFunc = void (*)(TEMP_Spawner*, RED4ext::GameInstance*);
        static GameCall<TFunc> func(Game::Addresses::gameIGameSystem_Initialize);
        func(this, apGameInstance);
    }

    void UnInitialize()
    {
        using TFunc = void (*)(TEMP_Spawner*);
        static GameCall<TFunc> func(Game::Addresses::gameIGameSystem_UnInitialize);
        func(this);
    }

    RED4ext::ent::EntityID Spawn(const RED4ext::CName acEntityPath, TEMP_SpawnSettings& aSettings)
    {
        // REDSmartPtr<TEMP_PendingEntity::Unk00> TEMP_Spawner::func(this, TEMP_SpawnSettings&, RED4ext::CName&)
        using TFunc = void (*)(TEMP_Spawner*, REDSmartPtr<TEMP_PendingEntity::Unk00>*, TEMP_SpawnSettings&, const RED4ext::CName);
        static GameCall<TFunc> func(Game::Addresses::gameIGameSystem_Spawn);

        REDSmartPtr<TEMP_PendingEntity::Unk00> pendingEntity;
        func(this, &pendingEntity, aSettings, acEntityPath);

        RED4ext::ent::EntityID entityID{};
        entityID.hash = pendingEntity ? pendingEntity->entityID.hash : 0;
        return entityID;
    }

    RED4ext::ent::EntityID SpawnRecord(const RED4ext::TweakDBID acRecordDBID, TEMP_SpawnSettings& aSettings)
    {
        auto* pTDB = RED4ext::TweakDB::Get();

        RED4ext::CName entityPath;
        if (!pTDB->TryGetValue(RED4ext::TweakDBID(acRecordDBID, ".entityTemplatePath"), entityPath))
        {
            RED4ext::ent::EntityID entityID{};
            entityID.hash = 0;
            return entityID;
        }

        // if not set by user, use the default one from TweakDB
        RED4ext::CName defaultAppearance;
        if (pTDB->TryGetValue(RED4ext::TweakDBID(acRecordDBID, ".appearanceName"), defaultAppearance) && aSettings.appearance == "default")
        {
            // ... as long as it's not empty.
            if (!defaultAppearance.IsNone() && defaultAppearance != "")
            {
                aSettings.appearance = defaultAppearance;
            }
        }

        aSettings.SetRecordID(acRecordDBID);
        return Spawn(entityPath, aSettings);
    }

    void Despawn(const RED4ext::Handle<RED4ext::IScriptable>& aEntity)
    {
        using TFunc = void (*)(TEMP_Spawner*, RED4ext::IScriptable*);
        static GameCall<TFunc> func(Game::Addresses::gameIGameSystem_Despawn);
        func(this, aEntity.GetPtr());
    }
};
RED4EXT_ASSERT_OFFSET(TEMP_Spawner, unk48, 0x48);
RED4EXT_ASSERT_SIZE(TEMP_Spawner, 0x68);

#pragma endregion

void CreateSingleton(const RED4ext::Handle<RED4ext::IScriptable>& apClassInstance)
{
    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pType = apClassInstance->GetNativeType();
    auto* pGameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;
    if (pGameInstance->GetInstance(pType) != nullptr)
        return; // already init

    auto* pGameSystemType = pRTTI->GetType("gameIGameSystem");
    if (pType->IsA(pGameSystemType))
    {
        // Initialize system
        {
            struct
            {
                uint64_t unk00 = 0;
                const char* unk08 = "";
                uint64_t unk10 = 0;
                uint32_t unk18 = 0;
                uint32_t unk1C = 1;
                union
                {
                    uint32_t fullValue = 0xFF000200;
#pragma pack(push, 1)
                    struct
                    {
                        uint8_t unk00;
                        uint16_t unk01;
                        uint8_t unk03;
                    };
#pragma pack(pop)
                } unk20;
                uint32_t unk24 = 0;

            } systemInitParams;
            RED4EXT_ASSERT_SIZE(systemInitParams, 40);

            auto* pGameSystem = reinterpret_cast<gameIGameSystem*>(apClassInstance.GetPtr());
            pGameSystem->gameInstance = pGameInstance;
            pGameSystem->OnSystemInitializeAsync(&systemInitParams);
        }

        auto* pParentType = apClassInstance->GetType();
        pGameInstance->unk08.Insert(pParentType, apClassInstance);
        pGameInstance->unk38.PushBack(apClassInstance);

        pParentType = pParentType->parent;
        auto* pReplicatedGameSystemType = pRTTI->GetType("gameIReplicatedGameSystem");
        while (pParentType && pParentType != pGameSystemType && pParentType != pReplicatedGameSystemType)
        {
            pGameInstance->unk48.Insert(pParentType, pType);
            pParentType = pParentType->parent;
        }
    }
    else
    {
        auto* pParentType = apClassInstance->GetType();
        pGameInstance->unk08.Insert(pParentType, apClassInstance);
        pGameInstance->unk48.Insert(pParentType, pType);
    }
}

void CreateSingleton(const RED4ext::CName acTypeName)
{
    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pType = pRTTI->GetClass(acTypeName);
    auto* pGameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;
    if (pGameInstance->GetInstance(pType) != nullptr)
        return; // already init

    auto* pClassInstance = static_cast<RED4ext::IScriptable*>(pType->CreateInstance());
    RED4ext::Handle handle(pClassInstance);

    CreateSingleton(handle);
}

#pragma region WorldFunctionalTests

// This is kept for backward compatibility.
// Use exEntitySpawner

void WorldFunctionalTests_SpawnEntity(RED4ext::IScriptable*, RED4ext::CStackFrame* apFrame, RED4ext::ent::EntityID* apOut, int64_t)
{
    struct FunctionalTestsGameSystem
    {
        uint8_t unk[0xB8];
        TEMP_Spawner spawner;
    };

    RED4ext::CString entityPath;
    RED4ext::WorldTransform worldTransform{};
    RED4ext::CString unknown;

    GetParameter(apFrame, &entityPath);
    GetParameter(apFrame, &worldTransform);
    GetParameter(apFrame, &unknown);
    apFrame->code++; // skip ParamEnd

    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pGameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;
    auto* pFunctionalType = pRTTI->GetType("FunctionalTestsGameSystem");
    auto* pFunctionalSystem = reinterpret_cast<FunctionalTestsGameSystem*>(pGameInstance->GetInstance(pFunctionalType));
    uint32_t oldSize = pFunctionalSystem->spawner.pendingEntities.size;

    ExecuteFunction("WorldFunctionalTests", "Internal_SpawnEntity", nullptr, entityPath, worldTransform, unknown);

    // if any entity was spawned
    uint32_t newSize = pFunctionalSystem->spawner.pendingEntities.size;
    if (oldSize != newSize)
    {
        auto& pending = pFunctionalSystem->spawner.pendingEntities[newSize - 1];
        *apOut = pending.unk00->entityID;
    }
}

void WorldFunctionalTests_DespawnEntity(RED4ext::IScriptable*, RED4ext::CStackFrame* apFrame, void*, int64_t)
{
    RED4ext::Handle<RED4ext::IScriptable> entity;

    GetParameter(apFrame, &entity);
    apFrame->code++; // skip ParamEnd

    ExecuteFunction("WorldFunctionalTests", "Internal_DespawnEntity", nullptr, entity);
}

#pragma endregion

#pragma region EntitySpawner

struct exEntitySpawnerSystem : gameIGameSystem
{
    constexpr static char NATIVE_TYPE_STR[] = "exEntitySpawner";
    constexpr static char PARENT_TYPE_STR[] = "gameIGameSystem";
    constexpr static RED4ext::CName NATIVE_TYPE = NATIVE_TYPE_STR;
    constexpr static RED4ext::CName PARENT_TYPE = PARENT_TYPE_STR;

    static exEntitySpawnerSystem* Singleton;
    static TEMP_Spawner exEntitySpawner_Spawner;

private:
    static exEntitySpawnerSystem* CreateNew(void* apAddress)
    {
        constexpr int32_t VftableSize = 0x1A8;
        static uintptr_t* ClonedVftable = nullptr;

        CallConstructor(apAddress);

        if (ClonedVftable == nullptr)
        {
            ClonedVftable = static_cast<uintptr_t*>(malloc(VftableSize));
            memcpy(ClonedVftable, *static_cast<uintptr_t**>(apAddress), VftableSize);

            ClonedVftable[0] = reinterpret_cast<uintptr_t>(&HOOK_GetNativeType);
            ClonedVftable[0x118 / 8] = reinterpret_cast<uintptr_t>(&HOOK_OnInitialize);
            ClonedVftable[0x120 / 8] = reinterpret_cast<uintptr_t>(&HOOK_OnShutdown);
        }

        // overwrite vftable with our clone
        *static_cast<uintptr_t**>(apAddress) = ClonedVftable;

        return static_cast<exEntitySpawnerSystem*>(apAddress);
    }

    static RED4ext::CBaseRTTIType* HOOK_GetNativeType(exEntitySpawnerSystem*)
    {
        auto* pRTTI = RED4ext::CRTTISystem::Get();
        return pRTTI->GetClass(NATIVE_TYPE_STR);
    }

    static void HOOK_OnInitialize(exEntitySpawnerSystem* apThis) { exEntitySpawner_Spawner.Initialize(apThis->gameInstance); }

    static void HOOK_OnShutdown(exEntitySpawnerSystem*) { exEntitySpawner_Spawner.UnInitialize(); }

    static void SpawnCallback(TEMP_PendingEntity::Unk00& aUnk)
    {
        using TFunc = void (*)(IScriptable*, RED4ext::ent::Entity*);
        static GameCall<TFunc> func(Game::Addresses::gameIGameSystem_SpawnCallback);

        struct GameInstance_78_Unk
        {
            uint8_t unk00[0xD0];
            IScriptable* worldRuntimeEntityRegistry;
        };

        struct GameInstance_78
        {
            GameInstance_78_Unk* unk00;
        };

        auto* pGameInstance = Singleton->gameInstance;

        if (aUnk.entity)
            func(reinterpret_cast<GameInstance_78*>(pGameInstance->unk78)->unk00->worldRuntimeEntityRegistry, aUnk.entity);
    }

public:
    static void InitializeSingleton()
    {
        // should only be called once
        assert(Singleton == nullptr);
        auto* pRTTI = RED4ext::CRTTISystem::Get();

        RED4ext::CClass::Flags classFlags{};
        classFlags.isNative = 1;
        RED4ext::CNamePool::Add(NATIVE_TYPE_STR);
        pRTTI->CreateScriptedClass(NATIVE_TYPE, classFlags, pRTTI->GetClass(PARENT_TYPE));

        auto* pAllocator = pRTTI->GetClass(PARENT_TYPE)->GetAllocator();
        Singleton = CreateNew(pAllocator->AllocAligned(sizeof(exEntitySpawnerSystem), alignof(exEntitySpawnerSystem)).memory);
        CreateSingleton(RED4ext::Handle<IScriptable>(Singleton));
    }

    static void Spawn(IScriptable*, RED4ext::CStackFrame* apFrame, RED4ext::ent::EntityID* apOut, int64_t)
    {
        RED4ext::CName entityPath; // <- raRef
        RED4ext::WorldTransform worldTransform{};
        RED4ext::CName appearance = "default";
        RED4ext::TweakDBID recordDBID = 0;

        GetParameter(apFrame, &entityPath);
        GetParameter(apFrame, &worldTransform);
        GetParameter(apFrame, &appearance);
        GetParameter(apFrame, &recordDBID);
        apFrame->code++; // skip ParamEnd

        if (appearance == RED4ext::CName(""))
        {
            // doesn't work for vehicles
            appearance = "default";
        }

        TEMP_SpawnSettings settings;
        settings.appearance = appearance;
        settings.callback = SpawnCallback;
        settings.SetRecordID(recordDBID);
        settings.SetTransform(worldTransform);

        const auto entityID = exEntitySpawner_Spawner.Spawn(entityPath, settings);

        if (apOut != nullptr)
        {
            *apOut = entityID;
        }
    }

    static void SpawnRecord(IScriptable*, RED4ext::CStackFrame* apFrame, RED4ext::ent::EntityID* apOut, int64_t)
    {
        RED4ext::TweakDBID recordDBID = 0;
        RED4ext::WorldTransform worldTransform{};
        RED4ext::CName appearance = "default";
        GetParameter(apFrame, &recordDBID);
        GetParameter(apFrame, &worldTransform);
        GetParameter(apFrame, &appearance);
        apFrame->code++; // skip ParamEnd

        if (appearance == RED4ext::CName(""))
        {
            // doesn't work for vehicles
            appearance = "default";
        }

        TEMP_SpawnSettings settings;
        settings.appearance = appearance;
        settings.callback = SpawnCallback;
        settings.SetTransform(worldTransform);

        const auto entityID = exEntitySpawner_Spawner.SpawnRecord(recordDBID, settings);

        if (apOut != nullptr)
        {
            *apOut = entityID;
        }
    }

    static void Despawn(IScriptable*, RED4ext::CStackFrame* apFrame, void*, int64_t)
    {
        RED4ext::Handle<IScriptable> entity;

        GetParameter(apFrame, &entity);
        apFrame->code++; // skip ParamEnd

        exEntitySpawner_Spawner.Despawn(entity);
    }
};
exEntitySpawnerSystem* exEntitySpawnerSystem::Singleton = nullptr;
TEMP_Spawner exEntitySpawnerSystem::exEntitySpawner_Spawner;

#pragma endregion

void RTTIExtender::AddFunctionalTests()
{
    CreateSingleton("WorldFunctionalTests");
    CreateSingleton("FunctionalTestsGameSystem");

    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pClass = pRTTI->GetClass("WorldFunctionalTests");
    if (pClass != nullptr)
    {
        {
            struct WorldFunctionalTests
            {
                uint8_t unk00[0x40];
                RED4ext::Handle<RED4ext::ISerializable> unk40;
            };
            RED4EXT_ASSERT_OFFSET(WorldFunctionalTests, unk40, 0x40);

            auto* pGameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;
            auto* pWorld = reinterpret_cast<WorldFunctionalTests*>(pGameInstance->GetInstance(pClass));

            // whatever this is, it's only used to get gameInstance
            // passing cpPlayerSystem (or any system) will work
            pWorld->unk40 = pGameInstance->GetInstance(pRTTI->GetType("cpPlayerSystem"))->ref.Lock();
        }

        auto* pFunction = pClass->GetFunction("SpawnEntity");
        if (pFunction != nullptr && pFunction->params.size == 0)
        {
            RED4ext::CNamePool::Add("Internal_SpawnEntity");
            pFunction->fullName = pFunction->shortName = "Internal_SpawnEntity";
            pFunction->AddParam("String", "entityPath");
            pFunction->AddParam("WorldTransform", "worldTransform");
            pFunction->AddParam("String", "unknown");

            RED4ext::CBaseFunction::Flags flags{};
            flags.isNative = true;
            pFunction = RED4ext::CClassStaticFunction::Create(pClass, "SpawnEntity", "SpawnEntity", WorldFunctionalTests_SpawnEntity, flags);
            pFunction->AddParam("String", "entityPath");
            pFunction->AddParam("WorldTransform", "worldTransform");
            pFunction->AddParam("String", "unknown");
            pFunction->SetReturnType("entEntityID");
            pClass->RegisterFunction(pFunction);
        }

        pFunction = pClass->GetFunction("DespawnEntity");
        if (pFunction != nullptr && pFunction->params.size == 0)
        {
            RED4ext::CNamePool::Add("Internal_DespawnEntity");
            pFunction->fullName = pFunction->shortName = "Internal_DespawnEntity";
            pFunction->AddParam("handle:entEntity", "entity");

            RED4ext::CBaseFunction::Flags flags{};
            flags.isNative = true;
            pFunction = RED4ext::CClassStaticFunction::Create(pClass, "DespawnEntity", "DespawnEntity", WorldFunctionalTests_DespawnEntity, flags);
            pFunction->AddParam("handle:entEntity", "entity");
            pClass->RegisterFunction(pFunction);
        }
    }
}

void RTTIExtender::AddEntitySpawner()
{
    exEntitySpawnerSystem::InitializeSingleton();

    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pClass = pRTTI->GetClass(exEntitySpawnerSystem::NATIVE_TYPE);
    if (pClass)
    {
        RED4ext::CBaseFunction::Flags flags{};
        flags.isNative = true;
        RED4ext::CClassStaticFunction* pFunction;

        // -------------

        pFunction = RED4ext::CClassStaticFunction::Create(pClass, "Spawn", "Spawn", exEntitySpawnerSystem::Spawn, flags);
        pFunction->AddParam("raRef:CResource", "entityPath");
        pFunction->AddParam("WorldTransform", "worldTransform");
        pFunction->AddParam("CName", "appearance", false, true);
        pFunction->AddParam("TweakDBID", "recordID", false, true);
        pFunction->SetReturnType("entEntityID");
        pClass->RegisterFunction(pFunction);

        // -------------

        pFunction = RED4ext::CClassStaticFunction::Create(pClass, "SpawnRecord", "SpawnRecord", exEntitySpawnerSystem::SpawnRecord, flags);
        pFunction->AddParam("TweakDBID", "recordID");
        pFunction->AddParam("WorldTransform", "worldTransform");
        pFunction->AddParam("CName", "appearance", false, true);
        pFunction->SetReturnType("entEntityID");
        pClass->RegisterFunction(pFunction);

        // -------------

        pFunction = RED4ext::CClassStaticFunction::Create(pClass, "Despawn", "Despawn", exEntitySpawnerSystem::Despawn, flags);
        pFunction->AddParam("handle:entEntity", "entity");
        pClass->RegisterFunction(pFunction);
    }
}

void RTTIExtender::Initialize()
{
    AddFunctionalTests(); // This is kept for backward compatibility with mods
    AddEntitySpawner();
}
