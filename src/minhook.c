#include "config.h"

#if defined(USE_MINHOOK) && (USE_MINHOOK == 1)

#pragma warning(push)
#pragma warning(disable : 4214 4244 4310)
#include "minhook/src/hook.c"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4244)
#include "minhook/src/trampoline.c"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4201)
#include "minhook/src/buffer.c"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4701 4706)
#include "minhook/src/hde/hde32.c"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4701 4706)
#include "minhook/src/hde/hde64.c"
#pragma warning(pop)

#endif
