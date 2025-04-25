#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////
// ++delayload

#include <delayimp.h>

BOOL _G_msvcp_win;

FARPROC WINAPI DliHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	switch (dliNotify)
	{
	case dliNotePreLoadLibrary:
		if (_G_msvcp_win && !strcmp("MSVCP140.dll", pdli->szDll))
		{
			pdli->szDll = "msvcp_win.dll";
		}
		break;
	}

	return 0;
}

const PfnDliHook __pfnDliNotifyHook2 = DliHook;

// --delayload
//////////////////////////////////////////////////////////////////////////

int _G_dwTlsIndex;

void _Do_call()
{
	TlsSetValue(_G_dwTlsIndex, (PVOID)(ULONG_PTR)MB_ICONINFORMATION);
}

ULONG WINAPI tf(Concurrency::details::_ContextCallback* ctx)
{
	if (0 <= CoInitializeEx(0, COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED))
	{
		ctx->_CallInContext(_Do_call, false);

		CoUninitialize();
	}

	delete ctx;

	return 0;
}

void WINAPI ep(void*)
{
	if (0 <= CoInitializeEx(0, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED))
	{
		if (0 <= (_G_dwTlsIndex = TlsAlloc()))
		{
			for (;;)
			{
				switch (MessageBoxW(0, L"msvcp_win ( Yes )\r\nor\r\nmsvcp140 ( No ) ?",
					L"which dll is use ?", MB_ICONQUESTION | MB_YESNOCANCEL))
				{
				case IDYES:
					_G_msvcp_win = TRUE;
					break;
				case IDNO:
					_G_msvcp_win = FALSE;
					break;
				default:
					goto __exit;
				}

				if (Concurrency::details::_ContextCallback* ctx = new 
					Concurrency::details::_ContextCallback( Concurrency::details::_ContextCallback::_CaptureCurrent() ))
				{
					TlsSetValue(_G_dwTlsIndex, (PVOID)(ULONG_PTR)MB_ICONHAND);

					if (HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)tf, ctx, 0, 0))
					{
					__loop:
						switch (MsgWaitForMultipleObjectsEx(1, &hThread, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE))
						{
						default:
							__debugbreak();
						case WAIT_OBJECT_0:
							break;
						case WAIT_OBJECT_0 + 1:
							MSG msg;
							while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
							{
								DispatchMessageW(&msg);
							}
							goto __loop;
						}
						CloseHandle(hThread);
						MessageBoxW(0, 0, L"", (ULONG)(ULONG_PTR)TlsGetValue(_G_dwTlsIndex));
					}
					else
					{
						delete ctx;
					}
				}

				__FUnloadDelayLoadedDLL2("MSVCP140.dll");
			}
		__exit:
			TlsFree(_G_dwTlsIndex);
		}

		CoUninitialize();
	}

	ExitProcess(0);
}