#include "RTTIExtender.h"
#include <RED4ext/Types/generated/WorldTransform.hpp>
#include <RED4ext/Types/generated/ent/EntityID.hpp>

void GetParameter(RED4ext::CStackFrame* apFrame, void* apInstance)
{
    apFrame->unk30 = 0;
    apFrame->unk38 = 0;
    const auto opcode = *(apFrame->code++);
    RED4ext::OpcodeHandlers::Run(opcode, apFrame->context, apFrame, apInstance, nullptr);
}

#pragma region WorldFunctionalTests

void WorldFunctionalTests_SpawnEntity(RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame,
                                      RED4ext::ent::EntityID* apOut, int64_t a4)
{
    struct UnkStuff
    {
        struct UnkStuff2
        {
            uint8_t unk[0x110];
            RED4ext::ent::EntityID entityID;
        };

        UnkStuff2* unk00;
        uint64_t unk08;
        uint64_t unk18;
        uint64_t unk20;
        uint64_t unk28;
    };
    RED4EXT_ASSERT_SIZE(UnkStuff, 5 * 8);
    RED4EXT_ASSERT_OFFSET(UnkStuff::UnkStuff2, entityID, 0x110);

    struct FunctionalTestsGameSystem
    {
        uint8_t unk[0xC8];
        RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>> spawnedEntities;
        RED4ext::DynArray<UnkStuff> pendingEntities;
    };
    RED4EXT_ASSERT_OFFSET(FunctionalTestsGameSystem, spawnedEntities, 0xC8);
    RED4EXT_ASSERT_OFFSET(FunctionalTestsGameSystem, pendingEntities, 0xD8);

    RED4ext::CString entityPath;
    RED4ext::WorldTransform worldTransform;
    RED4ext::CString unknown;

    GetParameter(apFrame, &entityPath);
    GetParameter(apFrame, &worldTransform);
    GetParameter(apFrame, &unknown);
    apFrame->code++; // skip ParamEnd

    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pGameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;
    auto* pFunctionalType = pRTTI->GetType("FunctionalTestsGameSystem");
    auto* pFunctionalSystem = reinterpret_cast<FunctionalTestsGameSystem*>(pGameInstance->GetInstance(pFunctionalType));
    uint32_t oldSize = pFunctionalSystem->pendingEntities.size;

    RED4ext::ExecuteFunction("WorldFunctionalTests", "Internal_SpawnEntity", nullptr, entityPath, worldTransform,
                             unknown);

    // if any entity was spawned
    uint32_t newSize = pFunctionalSystem->pendingEntities.size;
    if (oldSize != newSize)
    {
        auto& pending = pFunctionalSystem->pendingEntities[newSize - 1];
        *apOut = pending.unk00->entityID;
    }
}

void WorldFunctionalTests_DespawnEntity(RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame, void* apOut,
                                        int64_t a4)
{
    RED4ext::Handle<RED4ext::IScriptable> entity;

    GetParameter(apFrame, &entity);
    apFrame->code++; // skip ParamEnd

    RED4ext::ExecuteFunction("WorldFunctionalTests", "Internal_DespawnEntity", nullptr, entity);
}

#pragma endregion

void RTTIExtender::CreateSingleton(RED4ext::CName aTypeName)
{
    struct IUpdatableSystem : RED4ext::IScriptable
    {
        virtual void sub_110(uint64_t) = 0; // probably Update
    };

    struct gameIGameSystem : IUpdatableSystem
    {
        RED4ext::GameInstance* gameInstance;

        virtual void sub_118() = 0;
        virtual void sub_120() = 0;
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
        virtual void OnInitializeAsync(void*) = 0;   // 198
        virtual void OnUnInitializeAsync(void*) = 0; // 1A0
    };
    RED4EXT_ASSERT_OFFSET(gameIGameSystem, gameInstance, 0x40);

    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pType = reinterpret_cast<RED4ext::CClass*>(pRTTI->GetType(aTypeName));
    auto* pGameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;
    if (pGameInstance->GetInstance(pType) != nullptr)
        return; // already init

    auto* pClassInstance = pType->AllocInstance();
    RED4ext::Handle<RED4ext::IScriptable> handle(pClassInstance);

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
            static_assert(sizeof(systemInitParams) == 40);

            auto* pGameSystem = reinterpret_cast<gameIGameSystem*>(pClassInstance);
            pGameSystem->gameInstance = pGameInstance;
            pGameSystem->OnInitializeAsync(&systemInitParams);
        }

        auto* pParentType = reinterpret_cast<RED4ext::CClass*>(pClassInstance->GetParentType());
        pGameInstance->unk08.Insert(pParentType, handle);
        pGameInstance->unk38.PushBack(handle);

        pParentType = pParentType->parent;
        auto* pReplicatedGameSystemType = pRTTI->GetType("gameIReplicatedGameSystem");
        while (pParentType && pParentType != pGameSystemType && pParentType != pReplicatedGameSystemType)
        {
            // pGameInstance->unk48.Insert(pParentType, pType);
            pGameInstance->unk40.Insert((uintptr_t)pParentType, pType);
            pParentType = pParentType->parent;
        }
    }
    else
    {
        auto* pParentType = pClassInstance->GetParentType();
        pGameInstance->unk08.Insert(pParentType, handle);
        // pGameInstance->unk48.Insert(pParentType, pType);
        pGameInstance->unk40.Insert((uintptr_t)pParentType, pType);
    }
}

void RTTIExtender::AddFunctionalTests()
{
    CreateSingleton("WorldFunctionalTests");
    CreateSingleton("FunctionalTestsGameSystem");

    auto* pRTTI = RED4ext::CRTTISystem::Get();
    auto* pClass = reinterpret_cast<RED4ext::CClass*>(pRTTI->GetType("WorldFunctionalTests"));
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
            flags.isStatic = true;
            pFunction = RED4ext::CClassStaticFunction::Create(pClass, "SpawnEntity", "SpawnEntity",
                                                              WorldFunctionalTests_SpawnEntity, flags);
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
            flags.isStatic = true;
            pFunction = RED4ext::CClassStaticFunction::Create(pClass, "DespawnEntity", "DespawnEntity",
                                                              WorldFunctionalTests_DespawnEntity, flags);
            pFunction->AddParam("handle:entEntity", "entity");
            pClass->RegisterFunction(pFunction);
        }
    }
}

void RTTIExtender::Initialize()
{
    AddFunctionalTests();
}
