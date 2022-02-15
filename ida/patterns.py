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
            Item(name='InstanceOffset', pattern='48 8B 05 ? ? ? ? 8B D1 48 8B 88 18 8A 5A 01', expected=1, offset=3),
            Item(name='_DoNotUse_RenderQueueOffset', pattern='4D 8B 0E 49 39 31 0F 84 85 00 00 00 41 39 71 24 74 ? 49 8B 95', expected=1)
        ], functions=[
            Item(name='Resize', pattern='44 88 4C 24 20 44 89 44 24 18 89 54 24 10 89 4C', expected=1) 
        ]),
        Group(name='CRenderNode_Present', functions=[
            Item(name='DoInternal', pattern='48 89 5C 24 08 48 89 6C  24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 30 8B 01 41 8B F8 4C 8B 35', expected=1)
        ]),
        Group(name='CScript', functions=[
            Item(name='RunPureScript', pattern='40 55 48 81 EC D0 00 00 00 48 8D 6C 24 40 8B', expected=1),
            Item(name='CreateFunction', pattern='48 89 5C 24 08 57 48 83 EC 40 8B F9 48 8D 54 24 30 48 8B 0D ? ? ? ? 41 B8 B8 00 00 00', expected=1),
            Item(name='Log', pattern='40 53 48 83 EC ? 48 8D 4C 24 20 48 8B DA E8 ? ? ? ? 33 D2 48 8D 4C  24 40 E8', expected=1),
            Item(name='LogChannel', pattern='40 53 48 83 EC ? 48 8D 4C 24 20 48 8B DA E8 ? ? ? ? 33 D2 48 8D 4C 24 40 E8', expected=1),
            Item(name='TDBIDConstructorDerive', pattern='40 53 48 83 EC 30 33 C0 4C 89 44 24 20 48 8B DA', expected=1),
            Item(name='ProcessRunningState', pattern='40 53 48 83 EC 20 48 8B 0D ? ? ? ? 48 8B DA E8 ? ? ? ? 84 C0', expected=1),
            Item(name='TweakDBLoad', pattern='48 89 5C 24 10 48 89 7C 24 18 4C 89 74 24 20 55 48 8B EC 48 83 EC 70 48', expected=1)
        ]),
        Group(name='CWinapi', functions=[
            Item(name='ClipToCenter', pattern='48 89 5C 24 08 57 48 83 EC 30 48 8B 99 ? 01 00 00 48 8B F9 FF', expected=1)
        ]),
        Group(name='gameIGameSystem', functions=[
            Item(name='Constructor', pattern='48 8B D9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 C7 43 40 00 00 00 00', expected=2, index=0),
            Item(name='Initialize', pattern='48 89 5C 24 18 48 89 6C 24 20 57 48 83 EC 30 48 8B 42 78', expected=1),
            Item(name='UnInitialize', pattern='40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 33 C0 48 89 43 50 48 89 43 48', expected=1),
            Item(name='Spawn', pattern='FF 90 A8 01 00 00 48 8B 00 4C 8D 85 80 00 00 00', expected=1),
            Item(name='Despawn', pattern='40 55 53 56 57 41 55 41 56 41 57 48 8B EC 48 83 EC 50', expected=1),
            Item(name='SpawnCallback', pattern='41 57 48 83 EC 70 48 8B E9 4C 8B FA', expected=1)
        ]),
        Group(name='CPhotoMode', functions=[
            Item(name='SetRecordID', pattern='48 89 5C 24 08 48 89 74 24 18 55 57 41 56 48 8D 6C 24 B9 48 81 EC 90 00 00 00 48 8B F9', expected=1)
        ]),
        Group(name='CPatches', functions=[
            Item(name='BoundaryTeleport', pattern='48 8B C4 55 53 41 54 48 8D A8 78', expected=1),
            Item(name='IntroMovie', pattern='48 89 5C 24 08 57 48 83 EC 20 48 8B 44 24 50 48 8B D9 48 89 41 08', expected=1),
            Item(name='Vignette', pattern='48 8B 41 30 48 83 78 68 00 74', expected=1),
            Item(name='IsFinal', pattern='48 BB 87 C9 B1 63 33 01 15 75', expected=1),
            Item(name='CanDebugTeleport', pattern='48 BB C3 63 E3 32 7C A2 3C C1', expected=1),
            Item(name='MinimapFlicker', pattern='83 79 2C 00 48 8B F2 4C', expected=1),
            Item(name='OptionsInit', pattern='48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 48 8B F1 48 8D 4C 24 20 E8', expected=1),
            Item(name='RemovePedestrians', pattern='3B D8 0F 4E C3 8B D8 85 DB 0F 8E', expected=1),
            Item(name='SkipStartScreen', pattern='48 BB E6 F8 A5 A3 36 56 4E A7 C6 85 B0 ? ? ? 01', expected=1),
            Item(name='AmdSMT', pattern='75 2D 33 C9 B8 01 00 00 00 0F A2 8B C8 C1 F9 08', expected=1)
        ]),
        Group(name='CGame', functions=[
            Item(name='Main', pattern='40 55 57 41 57 48 81 EC', expected=1)
        ]),
    ]
