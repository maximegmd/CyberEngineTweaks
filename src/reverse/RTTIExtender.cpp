#include "RTTIExtender.h"

#include <RED4ext/SharedMutex.hpp>
#include <RED4ext/Scripting/Natives/Generated/Transform.hpp>
#include <RED4ext/Scripting/Natives/Generated/WorldTransform.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/Entity.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/EntityID.hpp>
#include <RED4ext/Scripting/Natives/Generated/game/IGameSystem.hpp>

#include "RED4ext/Scripting/Utils.hpp"

template <typename T> struct GameCall
{
    GameCall(uintptr_t aAddress, const int32_t acOffset = 0)
    {
        const RED4ext::RelocPtr<uint8_t> addr(aAddress);
        auto* pLocation = addr.GetAddr();
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
    RED4ext::TweakDBID DONOTUSE_recordDBID{};
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
        static GameCall<TFunc> func(CyberEngineTweaks::Addresses::CPhotoMode_SetRecordID);

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
        static GameCall<TFunc> func(CyberEngineTweaks::Addresses::gameIGameSystem_Initialize);
        func(this, apGameInstance);
    }

    void UnInitialize()
    {
        using TFunc = void (*)(TEMP_Spawner*);
        static GameCall<TFunc> func(CyberEngineTweaks::Addresses::gameIGameSystem_UnInitialize);
        func(this);
    }

    RED4ext::ent::EntityID Spawn(const RED4ext::CName acEntityPath, TEMP_SpawnSettings& aSettings)
    {
        // REDSmartPtr<TEMP_PendingEntity::Unk00> TEMP_Spawner::func(this, TEMP_SpawnSettings&, RED4ext::CName&)
        using TFunc = void (*)(TEMP_Spawner*, REDSmartPtr<TEMP_PendingEntity::Unk00>*, TEMP_SpawnSettings&, const RED4ext::CName);
        static GameCall<TFunc> func(CyberEngineTweaks::Addresses::gameIGameSystem_Spawn);

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
        static GameCall<TFunc> func(CyberEngineTweaks::Addresses::gameIGameSystem_Despawn);
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
    if (pGameInstance->GetSystem(pType) != nullptr)
        return; // already init

    auto* pGameSystemType = pRTTI->GetType("gameIGameSystem");
    if (pType->IsA(pGameSystemType))
    {
        // Initialize system
        {
            auto* pGameSystem = reinterpret_cast<RED4ext::gameIGameSystem*>(apClassInstance.GetPtr());
            pGameSystem->gameInstance = pGameInstance;
            pGameSystem->OnInitialize({});
        }

        auto* pParentType = apClassInstance->GetType();
        pGameInstance->systemMap.Insert(pParentType, apClassInstance);
        pGameInstance->systemInstances.PushBack(apClassInstance);

        pParentType = pParentType->parent;
        auto* pReplicatedGameSystemType = pRTTI->GetType("gameIReplicatedGameSystem");
        while (pParentType && pParentType != pGameSystemType && pParentType != pReplicatedGameSystemType)
        {
            pGameInstance->systemImplementations.Insert(pParentType, pType);
            pParentType = pParentType->parent;
        }
    }
    else
    {
        auto* pParentType = apClassInstance->GetType();
        pGameInstance->systemMap.Insert(pParentType, apClassInstance);
        pGameInstance->systemImplementations.Insert(pParentType, pType);
    }
}

void CreateSingleton(const RED4ext::CName acTypeName)
{
    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pType = pRTTI->GetClass(acTypeName);
    auto* pGameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;
    if (pGameInstance->GetSystem(pType) != nullptr)
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
    auto* pFunctionalSystem = reinterpret_cast<FunctionalTestsGameSystem*>(pGameInstance->GetSystem(pFunctionalType));
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

struct exEntitySpawnerSystem : RED4ext::gameIGameSystem
{
    using AllocatorType = RED4ext::Memory::RTTIAllocator;

    constexpr static char NATIVE_TYPE_STR[] = "exEntitySpawner";
    constexpr static char PARENT_TYPE_STR[] = "gameIGameSystem";
    constexpr static RED4ext::CName NATIVE_TYPE = NATIVE_TYPE_STR;
    constexpr static RED4ext::CName PARENT_TYPE = PARENT_TYPE_STR;

    static exEntitySpawnerSystem* Singleton;
    static TEMP_Spawner exEntitySpawner_Spawner;

    RED4ext::CClass* GetNativeType() override
    {
        return RED4ext::CRTTISystem::Get()->GetClass(NATIVE_TYPE_STR);
    }

    void OnWorldAttached(RED4ext::world::RuntimeScene* aScene) override
    {
        exEntitySpawner_Spawner.Initialize(gameInstance);
    }

    void OnBeforeWorldDetach(RED4ext::world::RuntimeScene* aScene) override
    {
        exEntitySpawner_Spawner.UnInitialize();
    }

private:
    static void SpawnCallback(TEMP_PendingEntity::Unk00& aUnk)
    {
        using TFunc = void (*)(IScriptable*, RED4ext::ent::Entity*);
        static GameCall<TFunc> func(CyberEngineTweaks::Addresses::gameIGameSystem_SpawnCallback);

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
    static void InitializeType()
    {
        auto* pRTTI = RED4ext::CRTTISystem::Get();

        RED4ext::CClass::Flags classFlags{};
        classFlags.isNative = 1;
        RED4ext::CNamePool::Add(NATIVE_TYPE_STR);
        pRTTI->CreateScriptedClass(NATIVE_TYPE, classFlags, pRTTI->GetClass(PARENT_TYPE));

        auto* pClass = pRTTI->GetClass(NATIVE_TYPE);
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

    static void InitializeSingleton()
    {
        auto* pRTTI = RED4ext::CRTTISystem::Get();
        auto* pSystemType = pRTTI->GetClass(NATIVE_TYPE);
        auto* pGameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;

        if (pGameInstance->GetSystem(pSystemType))
            return; // already init

        auto systemInstance = RED4ext::MakeHandle<exEntitySpawnerSystem>();
        systemInstance->gameInstance = pGameInstance;

        pGameInstance->systemMap.Insert(pSystemType, systemInstance);
        pGameInstance->systemInstances.PushBack(systemInstance);

        Singleton = systemInstance.instance;
    }

    static void Spawn(IScriptable*, RED4ext::CStackFrame* apFrame, RED4ext::ent::EntityID* apOut, int64_t)
    {
        RED4ext::CName entityPath; // <- raRef
        RED4ext::WorldTransform worldTransform{};
        RED4ext::CName appearance = "default";
        RED4ext::TweakDBID recordDBID{};

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
        RED4ext::TweakDBID recordDBID{};
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
    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pClass = pRTTI->GetClass("WorldFunctionalTests");
    if (pClass != nullptr)
    {
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

static void AddToInventory(RED4ext::IScriptable*, RED4ext::CStackFrame* apFrame, void*, int64_t)
{
    RED4ext::CString recordPath;
    int32_t quantity{1};

    RED4ext::GetParameter(apFrame, &recordPath);
    RED4ext::GetParameter(apFrame, &quantity);
    apFrame->code++; // skip ParamEnd

    RED4ext::ScriptGameInstance gameInstance;
    RED4ext::Handle<RED4ext::IScriptable> player;
    RED4ext::ExecuteGlobalFunction("GetPlayer;GameInstance", &player, gameInstance);

    bool result;
    RED4ext::TweakDBID itemID(recordPath.c_str());
    RED4ext::ExecuteFunction("gameTransactionSystem", "GiveItemByTDBID", &result, player, itemID, quantity);
}

void RTTIExtender::InitializeTypes()
{
    exEntitySpawnerSystem::InitializeType();

    AddFunctionalTests(); // This is kept for backward compatibility with mods

    {
        auto pFunction = RED4ext::CGlobalFunction::Create("AddToInventory", "AddToInventory", AddToInventory);
        pFunction->AddParam("String", "itemID");
        pFunction->AddParam("Int32", "quantity", false, true);
        pFunction->flags.isNative = true;
        pFunction->flags.isExec = true;

        auto pRTTI = RED4ext::CRTTISystem::Get();
        pRTTI->RegisterFunction(pFunction);
    }
}

void RTTIExtender::InitializeSingletons()
{
    exEntitySpawnerSystem::InitializeSingleton();

    CreateSingleton("WorldFunctionalTests");
    CreateSingleton("FunctionalTestsGameSystem");

    {
        auto* pRTTI = RED4ext::CRTTISystem::Get();
        auto* pClass = pRTTI->GetClass("WorldFunctionalTests");

        struct WorldFunctionalTests
        {
            uint8_t unk00[0x40];
            RED4ext::Handle<RED4ext::ISerializable> unk40;
        };
        RED4EXT_ASSERT_OFFSET(WorldFunctionalTests, unk40, 0x40);

        auto* pGameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;
        auto* pWorld = reinterpret_cast<WorldFunctionalTests*>(pGameInstance->GetSystem(pClass));

        // whatever this is, it's only used to get gameInstance
        // passing cpPlayerSystem (or any system) will work
        pWorld->unk40 = pGameInstance->GetSystem(pRTTI->GetType("cpPlayerSystem"))->ref.Lock();
    }
}
