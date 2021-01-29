#pragma once

class CApplicationProfile
{
public:
    void Load()
    {
        m_Binary = AfxGetApp()->GetProfileString(ProfileName, _T("Bin"), _T("None"));
        m_DebugEnabled = AfxGetApp()->GetProfileInt(ProfileName, _T("Log"), 0);
    }
    void Save()
    {
        AfxGetApp()->WriteProfileString(ProfileName, _T("Bin"), m_Binary);
        AfxGetApp()->WriteProfileInt(ProfileName, _T("Log"), m_DebugEnabled);
    }
    CString m_Binary;
    bool    m_DebugEnabled;
private:
    LPCTSTR ProfileName = _T("Default");
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
