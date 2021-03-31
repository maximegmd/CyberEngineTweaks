#pragma once

struct GameOption
{
    virtual ~GameOption();

    static void* s_integerVtable;
    static void* s_booleanVtable;
    static void* s_floatVtable;
    static void* s_stringVtable;
    static void* s_colorVtable;

    const char* pName;
    const char* pCategory;
    uint64_t unk18;
    uint64_t unk20;
    uint8_t unk28;
    uint8_t flag;
    uint8_t pad2A[0x30 - 0x2A];
    union
    {
        bool Boolean;
        struct
        {
            int32_t Value;
            int32_t Min;
            int32_t Max;
            int32_t Default;
        } Integer;
        struct
        {
            float Value;
            float Min;
            float Max;
            float Default;
        } Float;

        RED4ext::CString String;
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

    bool IsA(void* apVtable) const;

    bool Toggle();
};

static_assert(offsetof(GameOption, unk28) == 0x28);
static_assert(offsetof(GameOption, Boolean) == 0x30);

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
