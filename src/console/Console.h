#pragma once

using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0* pThis);

struct Image;
struct Console
{
	static void Initialize();
	static void Shutdown();
	static Console& Get();

	~Console();

	void Toggle();
	bool IsEnabled() const;
	
	void Log(const std::string& pText);
	void GameLog(const std::string& pText) { if (!m_disabledGameLog) Log(pText); }
	
	void Update();

	LRESULT OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	
	void Hook();
	
	static BOOL ClipToCenter(RED4ext::CGameEngine::UnkC0* pThis);

private:

	Console();
	
	std::recursive_mutex m_outputLock;
	std::vector<std::string> m_outputLines;
	bool m_outputShouldScroll{ true };
	bool m_outputScroll{ false };
	bool m_inputClear{ true };
	bool m_disabledGameLog{ true };
	
	TClipToCenter* m_realClipToCenter{ nullptr };
	
	bool m_enabled{ false };
	bool m_focusConsoleInput{ false };
};
