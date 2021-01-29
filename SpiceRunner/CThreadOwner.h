#pragma once

class CThreadParent
{
public:
    CThreadParent() {}
    virtual void SetProperty(ULONG prop, UINT_PTR val) = 0;
};

class CThreadOwner
{
public:
    CThreadOwner(CThreadParent *parent = NULL) :
        m_Parent(parent) {}
    ~CThreadOwner()
    {
        StopThread(true);
        while (IsThreadRunning())
        {
            Sleep(10);
        }
    }
    bool IsThreadRunning() const { return m_ThreadHandle != NULL; }
    enum tThreadState { tsNotRunning, tsRunning, tsAborted, tsDeleting };
    bool StartThread()
    {
        if (IsThreadRunning())
        {
            return false;
        }
        InterlockedCompareExchange(&m_State, tsRunning, tsNotRunning);
        m_ThreadHandle = (HANDLE)_beginthread(_ThreadProc, 0, this);
        if (IsThreadRunning())
        {
            return true;
        }
        else
        {
            InterlockedCompareExchange(&m_State, tsNotRunning, tsRunning);
            return false;
        }
    }
    virtual tThreadState StopThread(bool bDeleting)
    {
        LONG val;
        if (bDeleting)
        {
            val = InterlockedCompareExchange(&m_State, tsDeleting, tsRunning);
        }
        else
        {
            val = InterlockedCompareExchange(&m_State, tsAborted, tsRunning);
        }
        return (tThreadState)val;
    }
protected:
    tThreadState ThreadState() { return (tThreadState)m_State; }
    HANDLE m_ThreadHandle = NULL;
    CThreadParent *m_Parent;
    // return false if the object deleted inside the thread
    virtual bool ThreadProc() = 0;
    virtual void ThreadTerminated(tThreadState previous)
    {
        UNREFERENCED_PARAMETER(previous);
    }
    static void __cdecl _ThreadProc(PVOID param)
    {
        CThreadOwner *pOwner = (CThreadOwner *)param;
        if (pOwner->ThreadProc())
        {
            //CloseHandle(pOwner->m_ThreadHandle);
            pOwner->m_ThreadHandle = NULL;
            LONG val = InterlockedExchange(&pOwner->m_State, tsNotRunning);
            pOwner->ThreadTerminated((tThreadState)val);
        }
    }
private:
    LONG   m_State = tsNotRunning;
};
