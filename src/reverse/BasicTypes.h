#pragma once
#include <string>

#include "RED4ext/REDhash.hpp"

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

uint32_t crc32(const char* buf, size_t len);

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
	CName(uint64_t aHash) : hash(aHash){}
	uint64_t hash;

	std::string ToString() const noexcept;
};

#pragma pack(push, 1)
struct TweakDBID
{
	TweakDBID(const std::string& aName)
	{
		hash = crc32(aName.c_str(), aName.size());
		string_length = aName.size();
	}

	std::string ToString() const noexcept;
	
	uint32_t hash;
	uint8_t string_length;
	uint16_t unk5{0};
	uint8_t unk7{0};
};

static_assert(sizeof(TweakDBID) == 8);

struct ItemID
{
	ItemID(const TweakDBID& aId) : id(aId) {}

	std::string ToString() const noexcept;
	
	TweakDBID id;
	uint32_t rngSeed{ 2 };
	uint16_t unkC{ 0 };
	uint8_t maybeType{ 0 };
	uint8_t padF;
};

static_assert(sizeof(ItemID) == 0x10);

#pragma pack(pop)