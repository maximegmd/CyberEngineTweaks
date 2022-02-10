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
            Item(name='DoInternal', pattern='48 89 5C 24 08 48 89 6C  24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 30  8B 01 41 8B F8 4C 8B 35', expected=1)
        ])
    ]
