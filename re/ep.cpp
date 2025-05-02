#include "stdafx.h"

#define label(x) _CRT_CONCATENATE(x, __LINE__)

#define BEGIN_PRIVILEGES(name, n) static const union { TOKEN_PRIVILEGES name;\
struct { ULONG PrivilegeCount; LUID_AND_ATTRIBUTES Privileges[n];} label(_) = { n, {

#define LAA(se) {{se}, SE_PRIVILEGE_ENABLED }

#define END_PRIVILEGES }};};

NTSTATUS WINAPI AdjustPrivileges(_In_ const TOKEN_PRIVILEGES* ptp)
{
	NTSTATUS status;
	HANDLE hToken;

	if (0 <= (status = NtOpenProcessToken(NtCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)))
	{
		status = NtAdjustPrivilegesToken(hToken, FALSE, const_cast<PTOKEN_PRIVILEGES>(ptp), 0, 0, 0);
		NtClose(hToken);
	}

	return status;
};

NTSTATUS GetToken(_Out_ PHANDLE TokenHandle, _In_ PVOID buf, _In_ const TOKEN_PRIVILEGES* RequiredSet)
{
	NTSTATUS status;

	union {
		PVOID pv;
		PBYTE pb;
		PSYSTEM_PROCESS_INFORMATION pspi;
	};

	pv = buf;
	ULONG NextEntryOffset = 0;

	OBJECT_ATTRIBUTES oa = { sizeof(oa) };

	do
	{
		pb += NextEntryOffset;

		HANDLE hProcess, hToken, hNewToken;

		CLIENT_ID ClientId = { pspi->UniqueProcessId };

		if (ClientId.UniqueProcess)
		{
			if (0 <= NtOpenProcess(&hProcess, PROCESS_QUERY_LIMITED_INFORMATION, &oa, &ClientId))
			{
				status = NtOpenProcessToken(hProcess, TOKEN_DUPLICATE, &hToken);

				NtClose(hProcess);

				if (0 <= status)
				{
					status = NtDuplicateToken(hToken, 
						TOKEN_ADJUST_PRIVILEGES | TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY,
						&oa, FALSE, TokenPrimary, &hNewToken);

					NtClose(hToken);

					if (0 <= status)
					{
						status = NtAdjustPrivilegesToken(hNewToken, FALSE, const_cast<PTOKEN_PRIVILEGES>(RequiredSet), 0, 0, 0);

						if (STATUS_SUCCESS == status)
						{
							*TokenHandle = hNewToken;
							return STATUS_SUCCESS;
						}

						NtClose(hNewToken);

					}
				}
			}
		}

	} while (NextEntryOffset = pspi->NextEntryOffset);

	return STATUS_UNSUCCESSFUL;
}

NTSTATUS GetToken(_Out_ PHANDLE TokenHandle, _In_ const TOKEN_PRIVILEGES* RequiredSet)
{
	NTSTATUS status;

	ULONG cb = 0x40000;

	do
	{
		status = STATUS_INSUFFICIENT_RESOURCES;

		if (PBYTE buf = new BYTE[cb += 0x1000])
		{
			if (0 <= (status = NtQuerySystemInformation(SystemProcessInformation, buf, cb, &cb)))
			{
				status = GetToken(TokenHandle, buf, RequiredSet);

				if (status == STATUS_INFO_LENGTH_MISMATCH)
				{
					status = STATUS_UNSUCCESSFUL;
				}
			}

			delete[] buf;
		}

	} while (status == STATUS_INFO_LENGTH_MISMATCH);

	return status;
}

HRESULT GetLastHrEx(ULONG dwError = GetLastError())
{
	NTSTATUS status = RtlGetLastNtStatus();
	return RtlNtStatusToDosErrorNoTeb(status) == dwError ? HRESULT_FROM_NT(status) : HRESULT_FROM_WIN32(dwError);
}

NTSTATUS Exec(PWSTR lpCommandLine)
{
	static const TOKEN_PRIVILEGES tp_se = { 1, { { {SE_SYSTEM_ENVIRONMENT_PRIVILEGE}, SE_PRIVILEGE_ENABLED } } };
	
	BEGIN_PRIVILEGES(tp_ai, 2)
		LAA(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE),
		LAA(SE_INCREASE_QUOTA_PRIVILEGE),
	END_PRIVILEGES;

	HANDLE hToken;
	NTSTATUS status;
	if (STATUS_SUCCESS == (status = AdjustPrivileges(&tp_ai)))
	{
		if (STATUS_SUCCESS == (status = GetToken(&hToken, &tp_se)))
		{
			STARTUPINFOW si = { sizeof(si) };
			PROCESS_INFORMATION pi;
			
			if (CreateProcessAsUserW(hToken, 0, lpCommandLine, 0, 0, 0, 0, 0, 0, &si, &pi))
			{
				NtClose(pi.hThread);
				NtClose(pi.hProcess);
			}
			else
			{
				status = GetLastHrEx();
			}
			NtClose(hToken);
		}
	}

	return status;
}

void WINAPI ep(PWSTR lpCommandLine)
{
	NTSTATUS status = STATUS_INVALID_PARAMETER;

	if (lpCommandLine = wcschr(GetCommandLineW(), '*'))
	{
		status = Exec(lpCommandLine + 1);
	}

	ExitProcess(status);
}

