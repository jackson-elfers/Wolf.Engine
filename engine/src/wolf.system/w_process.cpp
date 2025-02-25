#include "w_system_pch.h"
#include "w_process.h"
#include <stdlib.h>

//maximum class name size
#define MAX_CLASS_NAME_SIZE 256

#ifdef __WIN32
#include <tlhelp32.h>//for checking process
#include <psapi.h>//list all processes
#endif

using namespace wolf;
using namespace wolf::system;

static std::wstringstream s_processes_list;
static std::vector<w_process_info*> s_processes_handles;

static BOOL CALLBACK EnumWindowsProc(HWND pHWND, LPARAM pLParam)
{
	DWORD _process_id;
	TCHAR _class_name[MAX_CLASS_NAME_SIZE];
	TCHAR _title[MAX_CLASS_NAME_SIZE];

	GetClassNameW(pHWND, _class_name, sizeof(_class_name));
	GetWindowTextW(pHWND, _title, sizeof(_title));
	GetWindowThreadProcessId(pHWND, &_process_id);

	auto _process_info = new w_process_info();
	if (!_process_info)
	{
		logger.error("could not allocate memory for w_process_info. trace info w_process::EnumWindowsProc");
		return FALSE;
	}

	_process_info->class_name = _class_name;
	_process_info->title_name = _title;
	_process_info->info.dwProcessId = _process_id;
	_process_info->info.dwThreadId = 0;
	_process_info->info.hProcess = NULL;
	_process_info->info.hThread = 0;
	_process_info->handle = pHWND;
	_process_info->process_name = w_process::get_process_name_by_ID(_process_id);

	s_processes_handles.push_back(_process_info);

	return TRUE;
}

W_RESULT w_process::kill_process_by_ID(_In_ const unsigned long& pProcessID)
{
	W_RESULT _hr = W_PASSED;

#ifdef __WIN32

	// Get a handle to the process.
	HANDLE _hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pProcessID);

	// Get the process name.
	if (_hProcess != NULL)
	{
		if (!TerminateProcess(_hProcess, 0))
		{
			logger.error(L"Error on killing process id: {}", pProcessID);
			_hr = W_FAILED;
		}
	}

	// Release the handle to the process.
	CloseHandle(_hProcess);
#else

#endif

	return _hr;
}

std::wstring w_process::get_process_name_by_ID(_In_ const unsigned long& pProcessID)
{
	std::wstring _name;
#ifdef __WIN32
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	// Get a handle to the process.
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pProcessID);

	// Get the process name.
	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
			&cbNeeded))
		{
			GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
		}
	}

	_name = szProcessName;
	
	// Release the handle to the process.
	CloseHandle(hProcess);
#else

#endif

	return _name;
}

const std::wstring w_process::enum_all_processes()
{
#ifdef __WIN32
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) return L"";

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);

	// Print the name and process identifier for each process.
	s_processes_list.str(L"");
	s_processes_list.clear();
	s_processes_list << L"Process Name : Process ID\n";

	std::wstring _process_name;
	for (i = 0; i < cProcesses; i++)
	{
		if (aProcesses[i] != 0)
		{
			_process_name = get_process_name_by_ID(aProcesses[i]);
			// Print the process name and identifier.
			s_processes_list << _process_name << L" " << aProcesses[i] << "\n";
		}
	}

	s_processes_list << "\0";
	std::wstring _result = s_processes_list.str();
	
	s_processes_list.str(L"");
	s_processes_list.clear();

	return _result;
#endif
}

void w_process::enum_all_processes(_Inout_ std::vector<w_process_info*>& pProcessHandles)
{
	s_processes_handles.clear();
	EnumWindows(EnumWindowsProc, NULL);
	pProcessHandles.assign(s_processes_handles.begin(), s_processes_handles.end());
}

bool w_process::check_for_number_of_running_instances_from_process(_In_z_ const wchar_t* pProcessName, 
	_In_ size_t pNumberOfRunningInstnacesToBeChecked)
{
	bool _result = false;

#ifdef __WIN32

	size_t _exists = 0;
	PROCESSENTRY32 _entry;
	_entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE _snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(_snapshot, &_entry))
	{
		while (Process32Next(_snapshot, &_entry))
		{
			if (!_wcsicmp(_entry.szExeFile, pProcessName))
			{
				_exists++;
				if (_exists == pNumberOfRunningInstnacesToBeChecked)
				{
					_result = true;
					break;
				}
			}
		}
	}
	CloseHandle(_snapshot);

#else


#endif

	return _result;
}

size_t w_process::get_number_of_running_instances_from_process(_In_z_ const wchar_t* pProcessName)
{
#ifdef __WIN32

	size_t _exists = 0;
	PROCESSENTRY32 _entry;
	_entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE _snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(_snapshot, &_entry))
	{
		while (Process32Next(_snapshot, &_entry))
		{
			if (!_wcsicmp(_entry.szExeFile, pProcessName))
			{
				_exists++;
			}
		}
	}
	CloseHandle(_snapshot);

#elif defined __APPLE__

	pid_t pids[2048];
         std::wstring _input_name, _compare_name;
         _input_name = std::wstring(name);
         size_t size = 0;

	int bytes = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
	int n_proc = bytes / sizeof(pids[0]);
	int i;
	for (i = 0; i < n_proc; i++) 
	{
		struct proc_bsdinfo proc;
		int st = proc_pidinfo(pids[i], PROC_PIDTBSDINFO, 0, &proc, sizeof(proc));
		if (st == sizeof(proc)) 
		{
                	size = std::strlen(proc.pbi_name);
                	if (size > 0) 
			{
                    		_compare_name.resize(size);
                    		std::mbstowcs(&_compare_name[0], proc.pbi_name, size);
                	}

                if (!wcscmp(_input_name.data(), _compare_name.data())) 
		{
			_exists++;
		}
	}
#elif defined __linux
	 DIR* dir;
         struct dirent* ent;
         char* endptr;
         char buf[512];
         std::wstring _input_name, _compare_name;
         _input_name = std::wstring(name);
         size_t size = 0;

         if (!(dir = opendir("/proc"))) {
             perror("can't open /proc");
              return num;
         }

         while((ent = readdir(dir)) != NULL)
         {
             /* if endptr is not a null character, the directory is not
              * entirely numeric, so ignore it */
             long lpid = strtol(ent->d_name, &endptr, 10);
             if (*endptr != '\0')
             {
                 continue;
             }

             /* try to open the cmdline file */
             snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
             FILE* fp = fopen(buf, "r");

             if (fp)
             {
                 if (fgets(buf, sizeof(buf), fp) != NULL)
                 {
                     /* check the first token in the file, the program name */
                     char* first = strtok(buf, " ");

                     size = std::strlen(buf);
                      if (size > 0)
                      {
                          _compare_name.resize(size);
                          std::mbstowcs(&_compare_name[0], buf, size);
                      }


                     auto _tmp = std::string(buf);
                     auto pos = _compare_name.find_last_of('/');
                     std::wstring _file = _compare_name;
                     if(pos != std::wstring::npos)
                      _file = _compare_name.substr(pos+1);
                     if (!wcscmp((const wchar_t*)_file.data(), (const wchar_t*)_input_name.data()))
                     {
                        _exists++;
                     }
                 }
                 fclose(fp);
             }

         }

         closedir(dir);
#endif

	return _exists;
}

#ifdef __WIN32
w_process_info* w_process::create_process(
	_In_z_ const wchar_t* pPathtoProcess,
	_In_z_ const wchar_t* pCmdsArg,
	_In_z_ const wchar_t* pCurrentDirectoryPath,
	_In_  const long long pWaitAfterRunningProcess,
	_In_ DWORD pCreationFlags)
{
	STARTUPINFO _startup_info;
	ZeroMemory(&_startup_info, sizeof(_startup_info));

	PROCESS_INFORMATION _process_info;
	ZeroMemory(&_process_info, sizeof(_process_info));

	if (CreateProcess(
		pPathtoProcess,
		LPWSTR(pCmdsArg),
		NULL,
		NULL,
		FALSE,
		pCreationFlags,
		NULL,
		pCurrentDirectoryPath,
		&_startup_info,
		&_process_info))
	{
		WaitForSingleObject(_process_info.hProcess, pWaitAfterRunningProcess);

		auto _w_process_info = new w_process_info();
		_w_process_info->info = _process_info;
		
		DWORD _err_val = GetLastError();
		_w_process_info->error_code = std::error_code(_err_val, std::system_category());

		return _w_process_info;
	}
	else
	{
		logger.error(L"Process ID: \"{}\" could not run with arg \"{}\" . Error code : \"{}\"", pPathtoProcess, pCmdsArg, GetLastError());
		return nullptr;
	}
}

bool w_process::kill_process(_In_ w_process_info* pProcessInfo)
{
	if (!pProcessInfo) return false;
	
	if (!TerminateProcess(pProcessInfo->info.hProcess, 0))
	{
		logger.error(L"Error on killing process id: {}", pProcessInfo->info.dwProcessId);
		return false;
	}

	if (!CloseHandle(pProcessInfo->info.hThread))
	{
		logger.error("Error on closing thread id: {} for process id: {}", pProcessInfo->info.dwThreadId, pProcessInfo->info.dwProcessId);
	}
	if (!CloseHandle(pProcessInfo->info.hProcess))
	{
		logger.error("Error on closing process id: {}", pProcessInfo->info.dwProcessId);
		return false;
	}

	return true;
}

bool w_process::kill_all_processes(_In_ std::initializer_list<const wchar_t*> pProcessNames)
{
	W_RESULT _hr = W_PASSED;

#ifdef __WIN32
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) return L"";

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);

	DWORD _process_id = 0;
	std::wstring _process_name;
	for (i = 0; i < cProcesses; i++)
	{
		_process_id = aProcesses[i];
		if (_process_id != 0)
		{
			_process_name = get_process_name_by_ID(_process_id);
			for (auto _name : pProcessNames)
			{
				if (wcscmp(_process_name.c_str(), _name) == 0)
				{
					if (kill_process_by_ID(_process_id) == W_FAILED)
					{
						_hr = W_FAILED;
					}
				}
			}

		}
	}

#endif

	return _hr == W_PASSED;
}

bool w_process::force_kill_process_by_name(_In_z_ const std::wstring pProcessName,
	_In_ const bool pTerminateChildProcesses)
{
	std::wstring _cmd = L"taskkill /IM " + pProcessName + L" /F";
	if (pTerminateChildProcesses)
	{
		_cmd += L" /T";
	}
	_wsystem(_cmd.c_str());
	return true;
}

bool w_process::force_kill_process_by_name_as_admin(
	_In_z_ const std::wstring pProcessName,
	_In_z_ const std::wstring pUserNameName,
	_In_z_ const std::wstring pPassword,
	_In_ const bool pTerminateChildProcesses)
{
	std::wstring _cmd = L"taskkill /IM " + pProcessName +
		L" /U " + pUserNameName +
		L" /P " + pPassword +
		L" /F";
	if (pTerminateChildProcesses)
	{
		_cmd += L" /T";
	}
	_wsystem(_cmd.c_str());
	return true;
}

bool w_process::force_kill_process(_In_z_ const DWORD pProcessID, 
	_In_ const bool pTerminateChildProcesses)
{
	std::wstring _cmd = L"taskkill /PID " + std::to_wstring(pProcessID) + L" /F";
	if (pTerminateChildProcesses)
	{
		_cmd += L" /T";
	}
	_wsystem(_cmd.c_str());
	return true;
}

bool w_process::force_kill_process_by_name_as_admin(
	_In_ const DWORD pProcessID,
	_In_z_ const std::wstring pUserNameName,
	_In_z_ const std::wstring pPassword,
	_In_ const bool pTerminateChildProcesses)
{
	std::wstring _cmd = L"taskkill /PID " + std::to_wstring(pProcessID) +
		L" /U " + pUserNameName +
		L" /P " + pPassword +
		L" /F";
	if (pTerminateChildProcesses)
	{
		_cmd += L" /T";
	}
	_wsystem(_cmd.c_str());
	return true;
}

#endif
