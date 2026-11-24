"""Offline source contracts only; not COM lifetime or live compatibility tests."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]


def source(name):
    return (ROOT / 'src' / name).read_text(encoding='utf-8-sig')


class ShutdownContracts(unittest.TestCase):
    def test_lua_callbacks_precede_terminal_cleanup(self):
        text = source('scripting/LuaVM_Hooks.cpp')
        start = text.index('GameMainThread::Get().AddShutdownTask(')
        shutdown = text[start:]
        self.assertLess(shutdown.index('UnloadAllMods();'),
                        shutdown.index('m_d3d12.BeginShutdown(D3D12::ShutdownOrigin::AfterLuaUnload);'))

    def test_terminal_transition_is_idempotent_before_reset(self):
        text = source('d3d12/D3D12.cpp').split('void D3D12::BeginShutdown(', 1)[1]
        self.assertLess(text.index('std::lock_guard'), text.index('if (m_shutdownStarted)'))
        self.assertLess(text.index('if (m_shutdownStarted)'), text.index('m_shutdownStarted = true'))
        self.assertLess(text.index('m_shutdownStarted = true'), text.index('ResetState(true)'))
        self.assertIn('default_logger()->flush()', text)

    def test_gpu_completion_precedes_cleanup(self):
        text = source('d3d12/D3D12.cpp').split('void D3D12::BeginShutdown(', 1)[1]
        self.assertLess(text.index('ShutdownCompletion::Wait('), text.index('ResetState(true)'))
        self.assertIn('completion != ShutdownCompletion::Result::Complete', text)
        self.assertIn('[] { Sleep(1); }, 1, 2000)', text)
        self.assertNotIn('INFINITE', text)

    def test_fallback_has_distinct_origin(self):
        self.assertIn('BeginShutdown(ShutdownOrigin::RendererFallback)', source('d3d12/D3D12_Hooks.cpp'))

    def test_process_exit_is_distinct_from_dynamic_unload(self):
        text = source('dllmain.cpp').split('case DLL_PROCESS_DETACH:', 1)[1].split('default:', 1)[0]
        self.assertIn('SelectAction(aReserved != nullptr)', text)
        self.assertIn('CET::OnProcessTermination();', text)
        self.assertIn('else\n            Shutdown();', text)

    def test_process_exit_does_not_destruct_wait_or_log(self):
        text = source('CET.cpp').split('void CET::OnProcessTermination()', 1)[1].split('\n}', 1)[0]
        self.assertIn('LeaveForOperatingSystem(s_pInstance)', text)
        self.assertNotIn('.reset(', text)
        self.assertNotIn('Log::', text)
        self.assertNotIn('WaitFor', text)
        self.assertNotIn('std::lock_guard', text)

    def test_present_checks_shutdown_before_context_access(self):
        text = source('d3d12/D3D12_Hooks.cpp').split('void* D3D12::CRenderGlobal_Resize', 1)[0]
        self.assertLess(text.index('std::lock_guard'), text.index('if (!d3d12.m_shutdownStarted)'))
        self.assertLess(text.index('if (!d3d12.m_shutdownStarted)'), text.index('RenderContext::GetInstance()'))

    def test_render_and_cleanup_use_same_lifecycle_lock(self):
        text = source('d3d12/D3D12_Functions.cpp')
        for signature in ('bool D3D12::ResetState', 'bool D3D12::Initialize',
                          'void D3D12::PrepareUpdate', 'void D3D12::Update'):
            body = text.split(signature, 1)[1].split('\n}', 1)[0]
            self.assertIn('std::lock_guard', body)
            self.assertIn('m_imguiLock', body)

    def test_no_reference_leak_or_exception_swallowing(self):
        text = source('d3d12/D3D12_Functions.cpp') + source('d3d12/D3D12_Hooks.cpp')
        self.assertNotIn('.Detach(', text)
        self.assertNotIn('__except', text)
        self.assertIn('m_pdxgiSwapChain.Reset();', text)


if __name__ == '__main__':
    unittest.main()
