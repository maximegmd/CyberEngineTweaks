#pragma once

enum class GameOptionType : uint8_t
{
    Boolean,
    Integer,
    Float,
    String,
    Color // stored as int32
};

struct GameOption
{
    virtual ~GameOption();

    const char* pName;
    const char* pCategory;
    uint64_t unk18;
    uint64_t unk20;
    GameOptionType type;
    uint8_t flag;
    uint8_t pad2A[0x30 - 0x2A];
    union
    {
        bool* pBoolean;
        int32_t* pInteger;
        float* pFloat;
        RED4ext::CString* pString;
    };

    union
    {
        int32_t* pIntegerMin;
        float* pFloatMin;
    };

    union
    {
        int32_t* pIntegerMax;
        float* pFloatMax;
    };

    std::string GetInfo();

    std::string GetString();

    bool GetBool(bool& retval);
    bool GetInt(int& retval);
    bool GetFloat(float& retval);
    bool GetColor(int& retval);

    bool Set(const std::string& value);
    bool SetBool(bool value);
    bool SetInt(int value);
    bool SetFloat(float value);
    bool SetString(const std::string& value);
    bool SetColor(int value);

    bool Toggle();
};

static_assert(offsetof(GameOption, type) == 0x28);
static_assert(offsetof(GameOption, pBoolean) == 0x30);

// Struct to expose to Lua to allow these functions to be called at runtime
struct GameOptions
{
private:
    static GameOption* Find(const std::string& category, const std::string& name);

public:
    static void Print(const std::string& category, const std::string& name);

    static std::string Get(const std::string& category, const std::string& name);
    static bool GetBool(const std::string& category, const std::string& name);
    static int GetInt(const std::string& category, const std::string& name);
    static float GetFloat(const std::string& category, const std::string& name);

    static void Set(const std::string& category, const std::string& name, const std::string& value);
    static void SetBool(const std::string& category, const std::string& name, bool value);
    static void SetInt(const std::string& category, const std::string& name, int value);
    static void SetFloat(const std::string& category, const std::string& name, float value);

    static void Toggle(const std::string& category, const std::string& name);

    static void Dump();
    static void List(const std::string& category);

    static std::vector<GameOption*>& GetList();

};
