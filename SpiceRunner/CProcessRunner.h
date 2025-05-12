#pragma once

class CProcessRunner
{
public:
    CProcessRunner()
    {
        Clean();
    }
    void Terminate()
    {
        if (pi.hProcess)
        {
            TerminateProcess(pi.hProcess, 0);
        }
    }
    void RunProcess(CString& CommandLine)
    {
        Clean();
        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOW;
        si.dwFlags = STARTF_USESHOWWINDOW;
        if (CreateProcess(NULL, CommandLine.GetBuffer(), NULL, NULL, FALSE, 0, NULL, _T("."), &si, &pi))
        {
            ULONG waitTime = m_WaitTime ? m_WaitTime * 1000 : INFINITE;
            Log("Started %S", CommandLine.GetBuffer());
            while (WaitForSingleObject(pi.hProcess, waitTime) == WAIT_TIMEOUT)
            {
                if (ShouldTerminate())
                {
                    Terminate();
                }
            }
            Log("Finished %S", CommandLine.GetBuffer());
        }
        if (pi.hProcess) CloseHandle(pi.hProcess);
        if (pi.hThread) CloseHandle(pi.hThread);
        Clean();
    }
    static const UINT DefaultWaitTime = 0;
    UINT m_WaitTime = DefaultWaitTime;
protected:
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    void Clean()
    {
        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
    }
    virtual bool ShouldTerminate() { return false; }
};

