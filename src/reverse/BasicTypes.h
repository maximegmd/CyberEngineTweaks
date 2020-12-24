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

struct Vector4
{
	Vector4(float aX = 0.f, float aY = 0.f, float aZ = 0.f, float aW = 0.f)
		: x(aX), y(aY), z(aZ), w(aW)
	{}
	
	float x;
	float y;
	float z;
	float w;

	std::string ToString() const noexcept;
};

struct EulerAngles
{
	EulerAngles(float aX = 0.f, float aY = 0.f, float aZ = 0.f)
		: x(aX), y(aY), z(aZ)
	{}
	
	float x;
	float y;
	float z;
	
	std::string ToString() const noexcept;
};

struct Quaternion : Vector4
{
	Quaternion(float aX = 0.f, float aY = 0.f, float aZ = 0.f, float aW = 0.f)
		: Vector4(aX, aY, aZ, aW)
	{}
	
	float x;
	float y;
	float z;
	float w;

	std::string ToString() const noexcept;
};

uint32_t crc32(const char* buf, size_t len, uint32_t seed);

// - But yamyam these two are exactly the same why you do this?
// - We have to implement different destructors but we are lazy
struct StrongHandle
{
	RED4ext::REDreverse::Scripting::IScriptable* handle{nullptr};
	uint32_t* refCount{nullptr};
};

struct WeakHandle
{
	RED4ext::REDreverse::Scripting::IScriptable* handle{nullptr};
	uint32_t* refCount{nullptr};
};

struct CName
{
	CName(uint64_t aHash = 0) : hash(aHash){}
	CName(const std::string& aName) : hash(RED4ext::FNV1a(aName.c_str())){}
	uint64_t hash;

	std::string ToString() const noexcept;
};

#pragma pack(push, 1)
struct TweakDBID
{
	TweakDBID() = default;

	TweakDBID(uint32_t name_hash, uint8_t name_length, uint16_t unk5, uint8_t unk7)
	{
		this->name_hash = name_hash;
		this->name_length = name_length;
		this->unk5 = unk5;
		this->unk7 = unk7;
	}

	TweakDBID(uint64_t value)
	{
		this->value = value;
	}

	TweakDBID(const std::string_view aName)
	{
		name_hash = crc32(aName.data(), aName.size(), 0);
		name_length = aName.size();
		unk5 = unk7 = 0;
	}

	TweakDBID(const TweakDBID& base, const std::string_view aName)
	{
		name_hash = crc32(aName.data(), aName.size(), base.name_hash);
		name_length = aName.size() + base.name_length;
		unk5 = unk7 = 0;
	}

	std::string ToString() const noexcept;
	
	union
	{
		uint64_t value{0};
		struct
		{
			uint32_t name_hash;
			uint8_t name_length;
			uint16_t unk5;
			uint8_t unk7;
		};
	};
};

static_assert(sizeof(TweakDBID) == 8);

struct ItemID
{
	ItemID() = default;
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