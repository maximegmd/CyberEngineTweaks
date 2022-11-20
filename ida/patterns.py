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
            Item(name='InstanceOffset', pattern='48 89 5C 24 08 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 30 8B 01 41 8B F8 4C 8B 35', expected=1, offset=32)
        ], functions=[
            Item(name='Resize', pattern='44 88 4C 24 20 44 89 44 24 18 89 54 24 10 89 4C', expected=1)
        ]),
        Group(name='CRenderNode_Present', functions=[
            Item(name='DoInternal', pattern='48 89 5C 24 08 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 30 8B 01 41 8B F8 4C 8B 35', expected=1)
        ]),
        Group(name='CScript', functions=[
            Item(name='AllocateFunction', pattern='BA B8 00 00 00 48 8D 4D D7 E8', expected=3, index=0, offset=10),
            Item(name='Log', pattern='40 53 48 83 EC ? 48 8D 4C 24 20 48 8B DA E8 ? ? ? ? 33 D2 48 8D 4C  24 40 E8', expected=1),
            Item(name='ToStringDEBUG', pattern='48 89 5C 24 08 57 48 83  EC 20 FE 42 62 4C 8D 15 ? ? ? ? 33 C9 33 C0', expected=4, index=2),
            Item(name='LogChannel', pattern='4C 8B DC 49 89 5B 08 49  89 73 18 57 48 83 EC 70 48 8B 02 ? ? ? ? ? ? ? FE 42 62 4D 8D 43 10 33 FF 45 33 C9 49 89  7B 10 48 8B DA 48 89 7A', expected=1),
            Item(name='TDBIDConstructorDerive', pattern='40 53 48 83 EC 30 33 C0 4C 89 44 24 20 48 8B DA', expected=1),
            Item(name='TranslateBytecode', pattern='4C 8B DC 55 53 57 41 55 49 8D 6B A1 48 81 EC 98 00 00 00 48 8B 1A 4C 8B E9 8B 42 0C 48 8D 3C C3'),
            Item(name='TweakDBLoad', pattern='48 89 5C 24 18 55 57 41 56 48 8B EC 48 83 EC 70 48 8B D9 45 33 F6 48 8D', expected=1),
            Item(name='RegisterMemberFunction', pattern='48 89 5C 24 08 57 48 83 EC 20 49 8B C1 4D 8B D0 44 8B 4C 24 58 48 8B DA 41 83 C9 03', expected=1)
        ]),
        Group(name='CWinapi', functions=[
            Item(name='ClipToCenter', pattern='48 89 5C 24 08 57 48 83 EC 30 48 8B 99 ? 01 00 00 48 8B F9 FF', expected=1)
        ]),
        Group(name='gameIGameSystem', functions=[
            Item(name='Constructor', pattern='48 8B D9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 C7 43 40 00 00 00 00', expected=2, index=0),
            Item(name='Initialize', pattern='48 89 5C 24 18 48 89 6C 24 20 57 48 83 EC 30 48 8B 42 78', expected=1),
            Item(name='UnInitialize', pattern='40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 33 C0 48 89 43 50 48 89 43 48', expected=1),
            Item(name='Spawn', pattern='48 89 5C 24 18 55 56 41 54 41 56 41 57 48 8D 6C 24 90 48 81 EC 70 01 00 00 48 83 79 50 00 49 8B', expected=1),
            Item(name='Despawn', pattern='48 89 5C 24 10 48 89 6C  24 18 56 57 41 54 41 56 41 57 48 83 EC 50 4C 8B F9 0F 57 C0 48 83 C1 41', expected=1),
            Item(name='SpawnCallback', pattern='48 89 5C 24 18 48 89 6C  24 20 56 57 41 56 48 83 EC 70 48 8B F1 48 8B EA  48 83 C1 48 E8', expected=1)
        ]),
        Group(name='CPhotoMode', functions=[
            Item(name='SetRecordID', pattern='48 8B C4 55 57 48 8D 68 A1 48 81 EC 98 00 00 00 48 89 58 08 48 8B D9 48 89 70 18 48 8D 4D 27 48', expected=1)
        ]),
        Group(name='CPatches', functions=[
            Item(name='BoundaryTeleport', pattern='48 8B C4 55 53 41 54 48  8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 89 70 10 48 8D 59 48', expected=1),
            Item(name='IntroMovie', pattern='48 89 5C 24 08 57 48 83 EC 20 48 8B 44 24 50 48 8B D9 48 89 41 08', expected=1),
            Item(name='Vignette', pattern='48 8B 41 30 48 83 78 68 00 74', expected=1),
            Item(name='MinimapFlicker', pattern='83 79 2C 00 48 8B F2 4C', expected=1),
            Item(name='OptionsInit', pattern='40 53 48 83 EC 40 48 8B D9 48 8D 4C 24 20 E8 ? ? ? ? E8 ? ? ? ? 4C 8B 43 08', expected=1),
            Item(name='SkipStartScreen', pattern='74 5F E8 ? ? ? ? 48 8D 4C 24 20 8B D8 E8 ? ? ? ? 48 8B C8 8B D3 E8', expected=2, index=1),
            Item(name='AmdSMT', pattern='75 2D 33 C9 B8 01 00 00 00 0F A2 8B C8 C1 F9 08', expected=1)
        ]),
        Group(name='CGame', functions=[
            Item(name='Main', pattern='40 57 48 83 EC 70 48 8B F9 0F 29 7C 24 50 48 8D 4C 24 38', expected=1)
        ]),
        Group(name='CGameApplication', functions=[
            Item(name='Run', pattern='48 89 5C 24 08 57 48 83 EC 20 48 8B D9 33 FF 90 E8 ? ? ? ? 84 C0', expected=1)
        ]),
        Group(name='CBaseInitializationState', functions=[
            Item(name='OnTick', pattern='48 83 EC 28 48 8B 05 ? ? ? ? 4C 8B C2 48 85 C0 75 12 8D 50 03 49 8B C8 E8 ? ? ? ?', expected=1)
        ]),
        Group(name='CInitializationState', functions=[
            Item(name='OnTick', pattern='48 83 EC 28 48 8B 05 ? ? ? ? 4C 8B C2 8B 88 F8 00 00 00 85 C9 74 3D 83 E9 02 74 25 83 E9 01', expected=1)
        ]),
        Group(name='CRunningState', functions=[
            Item(name='OnTick', pattern='40 53 48 83 EC 20 48 8B 0D ? ? ? ? 48 8B DA E8 ? ? ? ? 84 C0', expected=1)
        ]),
        Group(name='CShutdownState', functions=[
            Item(name='OnTick', pattern='48 89 6C 24 18 56 48 83 EC 30 48 8B 0D ? ? ? ?', expected=1)
        ]),
        Group(name='PlayerSystem', functions=[
            Item(name='OnPlayerSpawned', pattern='48 8B C4 4C 89 48 20 55 56 57 48 8B EC 48 81 EC 80 00 00 00', expected=1)
        ])
    ]

