#pragma once

class CApplicationProfile
{
public:
    void Load()
    {
        m_SpiceBinary = AfxGetApp()->GetProfileString(ProfileName, _T("SpiceBin"), _T(""));
        m_VncBinary = AfxGetApp()->GetProfileString(ProfileName, _T("VncBin"), _T(""));
        m_DebugEnabled = AfxGetApp()->GetProfileInt(ProfileName, _T("Log"), 0);
        m_RestoreConnections = AfxGetApp()->GetProfileInt(ProfileName, _T("Connect"), 0);
    }
    void Save()
    {
        AfxGetApp()->WriteProfileString(ProfileName, _T("SpiceBin"), m_SpiceBinary);
        AfxGetApp()->WriteProfileString(ProfileName, _T("VncBin"), m_VncBinary);
        AfxGetApp()->WriteProfileInt(ProfileName, _T("Log"), m_DebugEnabled);
        AfxGetApp()->WriteProfileInt(ProfileName, _T("Connect"), m_RestoreConnections);
    }
    void EnableAutoStart(bool Enable)
    {
        CRunKey k;
        if (!k.m_hKey)
            return;
        if (Enable)
        {
            CString s;
            AfxGetModuleFileName(NULL, s);
            k.SetStringValue(AfxGetAppName(), s);
        }
        else
        {
            k.DeleteValue(AfxGetAppName());
        }
    }
    bool IsAutoStartEnabled()
    {
        CRunKey k;
        if (!k.m_hKey)
            return false;
        TCHAR buffer[MAX_PATH];
        ULONG len = ARRAYSIZE(buffer);
        return !k.QueryStringValue(AfxGetAppName(), buffer, &len);
    }
    CString m_SpiceBinary;
    CString m_VncBinary;
    bool    m_DebugEnabled;
    bool    m_RestoreConnections;
private:
    LPCTSTR ProfileName = _T("Default");
    class CRunKey : public CRegKey
    {
        public:
            CRunKey()
            {
                if (Open(HKEY_CURRENT_USER, m_Name))
                {
                    Create(HKEY_CURRENT_USER, m_Name);
                }
            }
        private:
            LPCTSTR m_Name = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    };
};

extern CApplicationProfile Profile;


#define Log(format, ...) _Log(format ## "\r\n", __VA_ARGS__)

static inline void _Log(LPCSTR format, ...)
{
    if (!Profile.m_DebugEnabled)
        return;

    char buf[4096];
    va_list list;
    va_start(list, format);
    vsnprintf(buf, sizeof(buf), format, list);
    va_end(list);
    OutputDebugStringA(buf);
}
