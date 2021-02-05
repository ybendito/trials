#pragma once

#include "CThreadOwner.h"
#include "CConnectionTester.h"
#include "CProcessRunner.h"

class CServerItem : public CThreadOwner, public CProcessRunner
{
public:
    enum eServerAction { esaFirst, esaConnect = esaFirst, esaDisconnect, esaDelete, esaEdit, esaInvalid };

    CServerItem() : CProcessRunner(1000)
    {
        for (UINT i = esaFirst; i < esaInvalid; ++i)
        {
            m_Id[i] = IDC_STATIC;
        }
    }
    ~CServerItem()
    {
        Disconnect();
    }
    CServerItem(const CServerItem& from)
    {
        m_WaitTime = from.m_WaitTime;
        m_HostName = from.m_HostName;
        m_Port = from.m_Port;
        m_ConnectNow = from.m_ConnectNow;
        for (UINT i = esaFirst; i < esaInvalid; ++i)
        {
            m_Id[i] = from.m_Id[i];
        }
    }

    bool CheckId(UINT id, eServerAction& which)
    {
        UINT i;
        for (i = esaFirst; i < esaInvalid; ++i)
        {
            if (m_Id[i] == id)
            {
                which = (eServerAction)i;
                return true;
            }
        }
        return false;
    }
    BOOL IsDisconnected(BOOL& bConnected) const
    {
        bConnected = m_Available;
        return !IsThreadRunning();
    }
    void Connect()
    {
        StartThread();
    }
    void Disconnect()
    {
        StopThread(false);
        Terminate();
    }
    CString Name() const
    {
        CString s;
        s.Format(_T("%s:%d"), m_HostName.GetString(), m_Port);
        return s;
    }
    CString m_HostName;
    UINT    m_Port = 10400;
    BOOL    m_ConnectNow = false;
    INT     m_Id[esaInvalid];
protected:
    bool m_Available;
    bool ThreadProc() override
    {
        AfxSocketInit();
        m_Available = false;
        while (ThreadState() == tsRunning)
        {
            CConnectionTester m_Tester("Initial", true);
            m_Available = m_Tester.Update(m_HostName, m_Port, 2000);
            if (!m_Available)
                continue;
            StartViewer();
        }
        m_Available = false;
        return true;
    }
    //CProcessRunner
    bool ShouldTerminate() override
    {
        CConnectionTester m_Tester("Intermediate", true);
        m_Available = m_Tester.Update(m_HostName, m_Port, 1000);
        return !m_Available;
    }
    void StartViewer()
    {
        CString cmdLine;
        CString exename = Profile.m_Binary;
        cmdLine.Format(_T("\"%s\" spice://%s:%d"), exename.GetString(), m_HostName.GetString(), m_Port);
        RunProcess(cmdLine);
    }
};

typedef CArray<CServerItem, CServerItem> CServerItemsArray;
