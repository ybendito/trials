#pragma once

class CTaskBarIconParent
{
public:
    virtual HWND GetWindowHandle() = 0;
    virtual void PreparePopupMenu(CMenu& Menu) = 0;
    virtual void ProcessPopupResult(UINT code) = 0;
};

class CTaskBarIcon
{
public:
    CTaskBarIcon(UINT IconResource, CTaskBarIconParent& Parent, CString InitialName = AfxGetAppName()) :
        m_Parent(Parent)
    {
        m_Message = 0x7fff;
        RtlZeroMemory(&m_Data, sizeof(m_Data));
        m_Data.cbSize = sizeof(m_Data);
        m_Data.uID = 1;
        m_Data.hIcon = AfxGetApp()->LoadIcon(IconResource);
        m_Data.uFlags = NIF_SHOWTIP | NIF_TIP | NIF_ICON | NIF_MESSAGE | NIF_INFO;
        m_Data.uVersion = NOTIFYICON_VERSION_4;
        wcscpy_s(m_Data.szTip, InitialName);
        m_Data.uCallbackMessage = m_Message;
        m_TaskBarMessage = RegisterWindowMessage(TEXT("TaskbarCreated"));
        Log("TaskbarCreated message = %d", m_TaskBarMessage);
    }
    ~CTaskBarIcon()
    {
    }
    void Attach()
    {
        Log("%s", __FUNCTION__);
        m_Data.hWnd = m_Parent.GetWindowHandle();
        Op(NIM_ADD);
        Op(NIM_SETVERSION);
    }
    void Detach()
    {
        Log("%s", __FUNCTION__);
        Op(NIM_DELETE);
    }
    void ProcessIconMessage(UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_TIMER && !CheckIcon()) {
            Detach();
            Attach();
            return;
        }
        if (m_TaskBarMessage && message == m_TaskBarMessage) {
            Detach();
            Attach();
            return;
        }
        if (!IsIconMessage(message))
            return;
        if (m_IconActive)
            return;

        UINT x, y, msg;
        msg = LOWORD(lParam);

        if (msg != WM_CONTEXTMENU)
            return;

        x = GET_X_LPARAM(wParam);
        y = GET_Y_LPARAM(wParam);

        CMenu m;
        m.CreatePopupMenu();
        m_Parent.PreparePopupMenu(m);

        BooleanHolder holder(m_IconActive);

        ::SetForegroundWindow(m_Data.hWnd);
        UINT val = ::TrackPopupMenu(m.GetSafeHmenu(), TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
            x, y, 0, m_Data.hWnd, NULL);
        m_Parent.ProcessPopupResult(val);
    }
private:
    CTaskBarIconParent& m_Parent;
    NOTIFYICONDATA m_Data;
    UINT m_Message;
    UINT m_TaskBarMessage;
    bool m_IconActive = false;
    bool Op(UINT operation)
    {
        bool b = Shell_NotifyIcon(operation, &m_Data);
        if (!b) {
            Log("%s: error on operation %d", __FUNCTION__, operation);
        }
        return b;
    }
    bool IsIconMessage(UINT msg)
    {
        return msg == m_Data.uCallbackMessage;
    }
    bool CheckIcon()
    {
        bool b = Op(NIM_MODIFY);
        Log("%s: result %d", __FUNCTION__, b);
        return b;
    }
    class BooleanHolder
    {
        public:
            BooleanHolder(bool &b) : m_b(b) { m_b = true; }
            ~BooleanHolder() { m_b = false; }
        private:
            bool& m_b;
    };
};
