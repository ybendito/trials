#pragma once

class CLastError
{
public:
    CLastError() : m_Error(0) {}
    ULONG Get() { m_Error = GetLastError(); return m_Error; }
    ULONG Value() { return m_Error; }
protected:
    ULONG m_Error;
};

#if USE_ASYNC_SOCKET

class CConnectionTester : public CAsyncSocket
{
public:
    CConnectionTester(const LPCSTR Name, bool OwnMessageLoop) :
        m_Name(Name),
        m_bOwnMessageLoop(OwnMessageLoop)
    {
    }
    bool Update(const CString& host, UINT port, INT timeout)
    {
        bool bRet = false;
        m_Host = host.GetString();
        m_Port = port;
        {
            // looks like a bug in MFC: debug build crashes
            // when different threads simulateously try
            //  to register the same window class
            CMutexSync sync(Profile.m_Mutex);
            CAsyncSocket::Create();
        }
        m_SocketState = socketConnecting;
        if (Connect(host, port))
        {
            DoLog("Connect immediately succeeded");
            bRet = true;
        }
        else
        {
            CLastError e;
            if (e.Get() != WSAEWOULDBLOCK)
            {
                DoLog("Connect failed");
            }
            else
            {
                while (m_SocketState == socketConnecting && timeout > 0)
                {
                    DoMessageLoop();
                    timeout -= 10;
                    Sleep(10);
                }
                bRet = m_SocketState == socketConnected;
                DoLog(bRet ? "Connected" : "Not connected");
            }
        }
        if (m_SocketState == socketConnected)
        {
            DoLog("Shutting down");
            while (!ShutDown())
            {
                DoMessageLoop();
                Sleep(10);
            }
            DoLog("Shutdown finished");
        }
        Close();
        m_Host = _T("Unknown");
        return bRet;
    }
protected:
    bool m_bOwnMessageLoop;
    LPCTSTR m_Host;
    UINT    m_Port;
    CStringA m_Name;
    void DoLog(LPCSTR Message)
    {
        Log("%s %S:%d %s", m_Name.GetString(), m_Host, m_Port, Message);
    }
    void DoMessageLoop()
    {
        if (!m_bOwnMessageLoop)
            return;
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    enum tSocketState
    {
        socketIdle,
        socketConnecting,
        socketConnected,
        socketClosing
    } m_SocketState = socketIdle;

    // CAsyncSocket
    void OnReceive(int nErrorCode) override
    {
        DoLog("OnReceive");
        if (nErrorCode)
        {
            m_SocketState = socketClosing;
        }
        CAsyncSocket::OnReceive(nErrorCode);
    }
    void OnSend(int nErrorCode) override
    {
        DoLog("OnSend");
        if (m_SocketState != socketConnected)
        {
            m_SocketState = socketConnected;
        }
    }
    void OnConnect(int nErrorCode) override
    {
        DoLog("OnConnect");
        __super::OnConnect(nErrorCode);
    }
    void OnClose(int nErrorCode) override
    {
        DoLog("OnClose");
        m_SocketState = socketConnecting;
        __super::OnClose(nErrorCode);
    }
};

#else

class CConnectionTester
{
public:
    CConnectionTester(const LPCSTR Name, bool Unused)
    {
        WSADATA wsaData;
        m_Name = Name;
        m_bRegistered = !WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (m_bRegistered)
        {
            m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        }
    }
    ~CConnectionTester()
    {
        if (m_Socket != INVALID_SOCKET)
        {
            closesocket(m_Socket);
        }
        if (m_bRegistered)
        {
            WSACleanup();
        }
    }
    bool Update(const CString& Host, UINT Port, INT Timeout)
    {
        if (m_Socket == INVALID_SOCKET)
        {
            return false;
        }
        m_Port = Port;
        m_Host = Host;
        CString sPort;
        timeval timeout;
        timeout.tv_sec = Timeout / 1000;
        timeout.tv_usec = (Timeout % 1000) * 1000;
        sPort.Format(L"%d", Port);
        DoLog("Connecting");
        bool b = WSAConnectByName(m_Socket, (LPTSTR)Host.GetString(), sPort.GetBuffer(), NULL, NULL, NULL, NULL, &timeout, NULL);
        DoLog(b ? "Connected" : "Not connected");
        if (!b)
        {
            Sleep(Timeout);
        }
        return b;
    }
protected:
    bool m_bRegistered;
    SOCKET m_Socket = INVALID_SOCKET;
    CString m_Name;
    LPCTSTR m_Host = _T("");
    UINT m_Port;
    void DoLog(LPCSTR Message)
    {
        Log("%s %S:%d %s", m_Name.GetString(), m_Host, m_Port, Message);
    }
};

#endif
