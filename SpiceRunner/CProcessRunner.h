#pragma once

class CProcessRunner
{
public:
    CProcessRunner(ULONG WaitTime = INFINITE) :
        m_WaitTime(WaitTime)
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
            while (WaitForSingleObject(pi.hProcess, m_WaitTime) == WAIT_TIMEOUT)
            {
                if (ShouldTerminate())
                {
                    Terminate();
                }
            }
        }
        if (pi.hProcess) CloseHandle(pi.hProcess);
        if (pi.hThread) CloseHandle(pi.hThread);
        Clean();
    }
protected:
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    void Clean()
    {
        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
    }
    ULONG m_WaitTime;
    virtual bool ShouldTerminate() { return false; }
};

