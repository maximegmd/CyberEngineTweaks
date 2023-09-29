from typing import List

class Item:
    name: str
    pattern: str
    expected: int
    index: int
    offset: int

    def __init__(self, pattern: str, name: str = '', expected: int = 1, index: int = 0, offset: int = 0) -> None:
        self.name = name
        self.pattern = pattern
        self.expected = expected
        self.index = index
        self.offset = offset

class Group:
    name: str
    pointers: List[Item]
    functions: List[Item]

    def __init__(self, name: str, pointers: List[Item] = [], functions: List[Item] = []) -> None:
        self.name = name
        self.pointers = pointers
        self.functions = functions

def get_groups() -> List[Group]:
    # Add new patterns here, please try to keep the groups ordering alphabetized.
    return [
        Group(name='CRenderGlobal', pointers=[
            # instance offset is used by CRenderNode_Present_DoInternal
            Item(name='InstanceOffset', pattern='48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 4C 89 48 20 57 41 56 41 57 48 83 EC 30 8B 01 45 8B', expected=1, offset=36), # ok
            Item(name='_DoNotUse_RenderQueueOffset', pattern='39 72 24 74 5B 48 8B 4A 18 4C 8D 8C 24 88 00 00 00 8B 42 24 44 8B C7 48 8B 95 ? ? ? ?', expected=1) # ok
        ], functions=[
            Item(name='Resize', pattern='48 8B C4 44 88 48 20 44 89 40 18 89 50 10 89 48 08 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68 88 48 81 EC 38 01 00 00', expected=1),
            Item(name='Shutdown', pattern='40 53 48 83 EC 20 48 8B D9 48 8D 05 ? ? ? ? 48 81 C1 98 00 00 00 48 89 01 E8', expected=1)
        ]),
        Group(name='CRenderNode_Present', functions=[
            Item(name='DoInternal', pattern='48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 4C 89 48 20 57 41 56 41 57 48 83 EC 30 8B 01 45 8B', expected=1) # ok
        ]),
        Group(name='CScript', functions=[
            Item(name='RunPureScript', pattern='40 55 48 81 EC D0 00 00 00 48 8D 6C 24 40 8B', expected=1), # ok
            Item(name='AllocateFunction', pattern='40 53 48 83 EC 30 BA B8 00 00 00 48 8D 4C 24 20 E8', expected=2, index=0), # ok
            Item(name='Log', pattern='48 8B C4 53 48 83 EC 70 48 83 60 C0 00 48 8D 48 C8 83 60 BC 00', expected=3, index=0), # ok
            Item(name='LogError', pattern='48 8B C4 53 48 83 EC 70 48 83 60 C0 00 48 8D 48 C8 83 60 BC 00', expected=3, index=1), # ok
            Item(name='LogWarning', pattern='48 8B C4 53 48 83 EC 70 48 83 60 C0 00 48 8D 48 C8 83 60 BC 00', expected=3, index=2), # ok
            Item(name='ToStringDEBUG', pattern='48 89 5C 24 08 57 48 83 EC 20 83 64 24 38 00 4C 8D 15 ? ? ? ? FE 42 62 33 C0', expected=4, index=1), # ok
            Item(name='LogChannel', pattern='48 89 5C 24 08 48 89 74 24 18 55 48 8B EC 48 83 EC 70 48 8B 02 48 8D 35 ? ? ? ? 48 83 65 18 00 4C 8D 45 18 48 83 62 30 00 45 33 C9 48 83 62 38 00', expected=2, index=0), # ok
            Item(name='LogChannelWarning', pattern='48 89 5C 24 08 48 89 74 24 18 55 48 8B EC 48 83 EC 70 48 8B 02 48 8D 35 ? ? ? ? 48 83 65 18 00 4C 8D 45 18 48 83 62 30 00 45 33 C9 48 83 62 38 00', expected=2, index=1), # ok
            Item(name='TDBIDConstructorDerive', pattern='48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 45 33 C9 48 8B FA', expected=1), # ok
            Item(name='TranslateBytecode', pattern='48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B 1A 48 8B E9 8B 42 0C', expected=2), # ok
            Item(name='TweakDBLoad', pattern='48 89 5C 24 10 48 89 7C 24 18 55 48 8B EC 48 83 EC 70 48 8B F9 48 8B DA 48 8B 0D ? ? ? ? 48 8B 01 FF 90 B8 00 00 00', expected=1), # ok
            Item(name='RegisterMemberFunction', pattern='40 53 48 83 EC 20 49 8B C1 4D 8B D0 44 8B 4C 24 58 4C 8B DA 41 83 C9 03 4C 8B C0 49 8B D2 48 8B D9 E8', expected=1) # ok
        ]),
        Group(name='CWinapi', functions=[
            Item(name='ClipToCenter', pattern='48 89 5C 24 08 55 48 8B EC 48 83 EC 30 48 8B D9 48 8B 89 68 01 00 00', expected=1) # ok
        ]),
        Group(name='gameIGameSystem', functions=[
            Item(name='Initialize', pattern='48 89 5C 24 08 57 48 83 EC 30 48 8B 42 78 4C 8B CA 48 8B D9', expected=1), # ok
            Item(name='UnInitialize', pattern=' 48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B F9 48 8D 51 42', expected=2, index=1), # ok 
            Item(name='Spawn', pattern='48 89 5C 24 10 48 89 74 24 18 55 57 41 56 48 8D 6C 24 B0 48 81 EC 50 01 00 00 48 83 79 50 00 49 8B D9 4D 8B F0', expected=1), # ok  
            Item(name='Despawn', pattern='48 8B C4 48  89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41  56 48 83 EC 40 48 8B E9 0F 57 C0 48 83 C1 41 48 8B F2 F3 0F 7F 40 D8 E8', expected=1), # ok 
            Item(name='SpawnCallback', pattern='48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 60  48 8B F1 48 8B FA 48 83 C1 48 E8', expected=1) # ok 
        ]),
        Group(name='CPhotoMode', functions=[
            Item(name='SetRecordID', pattern='48 89 5C 24 10 48 89 4C 24 08 55 48 8B EC 48 83 EC 40 48 8B DA 48 8D 4D E0 48 8D 55 10 E8', expected=1) # ok
        ]),
        Group(name='CPatches', functions=[
            Item(name='BoundaryTeleport', pattern='48 8B C4 48 89 58 10 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 F8 FE FF FF 48 81 EC D0 01 00 00 0F 29 78 B8 48 8D 51 48', expected=1),
            Item(name='IntroMovie', pattern='48 89 5C 24 08 57 48 83 EC 20 48 8B 44 24 50 48 8B D9 48 89 41 08', expected=1), # ok
            Item(name='Vignette', pattern='33 C0 48 39  41 68 74 11', expected=1),
            #Item(name='MinimapFlicker', pattern='44 0F 29 98 78 FF FF FF 44  0F 29 A0 68 FF FF FF 45 85 F6 75 56', expected=1),
            Item(name='OptionsInit', pattern='48 89 5C 24 08 55 48 8B EC 48 83 EC 70 48 83 65 F8 00 48 8B D9 83 65 F4 00', expected=1),
            #Item(name='SkipStartScreen', pattern='74 5F E8 ? ? ? ? 48 8D 4C 24 20 8B D8 E8 ? ? ? ? 48 8B C8 8B D3 E8', expected=2, index=1),
        ]),
        Group(name='CGame', functions=[
            Item(name='Main', pattern='48 89 5C 24 10 55 56 57 48 8B EC 48 81 EC 80 00 00 00 48 8B F9 0F 29 74 24 70 0F 29 7C 24 60 48 8D 4D C0', expected=1) # ok
        ]),
        Group(name='CBaseInitializationState', functions=[
            Item(name='OnTick', pattern='40 53 48 83 EC 20 48 8B 05 ? ? ? ? 33 DB 4C 8B C2 48 85 C0 ? ? ? ?', expected=1) # ok
        ]),
        Group(name='CInitializationState', functions=[
            Item(name='OnTick', pattern='40 53 48 83 EC 30 48 8B 05 ? ? ? ? 33 DB 4C 8B C2 8B 88 08 01 00 00', expected=1) # ok
        ]),
        Group(name='CRunningState', functions=[
            Item(name='OnTick', pattern='40 53 48 83 EC 30 83 64 24 28 00 48 8D 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B DA', expected=1) # ok
        ]),
        Group(name='CShutdownState', functions=[
            Item(name='OnTick', pattern='40 53 48 83 EC 20 48 8B DA E8 ? ? ? ? 48 8B CB 89 83 B0 02 00 00 ', expected=1) # ok
        ]),
        Group(name='PlayerSystem', functions=[
            Item(name='OnPlayerSpawned', pattern='48 89 5C 24 18 48 89 74 24 20 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC 50 48 8B DA 48 8B F9', expected=1)
        ]),
    ]

