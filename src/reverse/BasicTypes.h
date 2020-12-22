#pragma once
#include <string>

namespace RED4ext {
	namespace REDreverse {
		namespace Scripting {
			struct IScriptable;
		}
	}
}

struct Quaternion
{
	float x;
	float y;
	float z;
	float w;

	std::string ToString() const noexcept;
};

// - But yamyam these two are exactly the same why you do this?
// - We have to implement different destructors but we are lazy
struct StrongHandle
{
	RED4ext::REDreverse::Scripting::IScriptable* handle;
	uint32_t* refCount;
};

struct WeakHandle
{
	RED4ext::REDreverse::Scripting::IScriptable* handle;
	uint32_t* refCount;
};

struct CName
{
	uint64_t hash;

	std::string ToString() const noexcept;
};
