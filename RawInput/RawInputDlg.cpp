
// RawInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RawInput.h"
#include "RawInputDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRawInputDlg dialog



CRawInputDlg::CRawInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_RAWINPUT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRawInputDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_Devices);
    DDX_Control(pDX, IDC_REGISTERED, m_Registered);
}

BEGIN_MESSAGE_MAP(CRawInputDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_INPUT()
    ON_WM_INPUT_DEVICE_CHANGE()
    ON_BN_CLICKED(IDOK, &CRawInputDlg::OnBnClickedRefresh)
    ON_BN_CLICKED(IDC_APPLY, &CRawInputDlg::OnBnClickedApply)
END_MESSAGE_MAP()


// CRawInputDlg message handlers

BOOL CRawInputDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    Refresh();

    SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    
    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRawInputDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRawInputDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static CString DeviceName(HANDLE h, INT limit = 0)
{
    WCHAR name[256] = L"";
    UINT len = ELEMENTS_IN(name);
    INT nRes = GetRawInputDeviceInfo(h, RIDI_DEVICENAME, name, &len);
    CString s = name;
    if (s.IsEmpty()) s.Format(L"%p", h);
    if (limit > 0 && limit < s.GetLength()) s = s.Left(limit);
    return s;
}

static void PresentData(RAWINPUT& input, CString& s)
{
    if (input.header.dwType == RIM_TYPEKEYBOARD)
    {
        USHORT fl = input.data.keyboard.Flags;
        s.Format(L"%s%s%s %04X",
            (fl & RI_KEY_BREAK) ? L"Break" : L"Make",
            (fl & RI_KEY_E0) ? L"-E0" : L"",
            (fl & RI_KEY_E1) ? L"-E1" : L"",
            input.data.keyboard.MakeCode);
    }
    if (input.header.dwType == RIM_TYPEMOUSE)
    {
        USHORT fl = input.data.mouse.usFlags;
        USHORT bfl = input.data.mouse.usButtonFlags;
        s.Format(L"[%s%s%s] %s%s%s%s%s",
            (fl & MOUSE_MOVE_ABSOLUTE) ? L"A" : L"R",
            (fl & MOUSE_VIRTUAL_DESKTOP) ? L"V" : L"",
            (fl & MOUSE_ATTRIBUTES_CHANGED) ? L"C" : L"",
            (bfl & RI_MOUSE_WHEEL) ? L"W" : L"",
            (bfl & RI_MOUSE_LEFT_BUTTON_DOWN) ? L"-LD-" : L"",
            (bfl & RI_MOUSE_LEFT_BUTTON_UP) ? L"-LU-" : L"",
            (bfl & RI_MOUSE_RIGHT_BUTTON_DOWN) ? L"-RD-" : L"",
            (bfl & RI_MOUSE_RIGHT_BUTTON_UP) ? L"-RU-" : L"",
            input.data.mouse.ulRawButtons);
    }
    if (input.header.dwType == RIM_TYPEHID)
    {
        HANDLE h = input.header.hDevice;
        DWORD reportSize = input.data.hid.dwSizeHid;
        DWORD dataSize = input.data.hid.dwCount;
        s.Format(L"h %p, reports of %d, data len %d", h, reportSize, dataSize);
    }
}

void CRawInputDlg::OnRawInput(UINT nInputcode, HRAWINPUT hRawInput)
{
    //nInputcode = 1 means we are on background
    RAWINPUT input;
    RAWINPUTHEADER& header = input.header;
    CString s, sData;
    RID_DEVICE_INFO info;
    info.cbSize = sizeof(info);
    UINT len = sizeof(header), datalen;
    header.dwSize = sizeof(header);
    INT nRes = GetRawInputData(hRawInput, RID_HEADER, &header, &len, sizeof(header));
    if (nRes > 0)
    {
        CString sDevName = DeviceName(header.hDevice, 16);  
        datalen = header.dwSize - len;
        switch (header.dwType)
        {
            case RIM_TYPEKEYBOARD:
                len = sizeof(input);
                GetRawInputData(hRawInput, RID_INPUT, &input, &len, sizeof(header));
                PresentData(input, sData);
                s.Format(L"%s Kbd %s (%s)", nInputcode ? L"Bg" : L"Fg", (LPCTSTR)sDevName, (LPCTSTR)sData);
                break;
            case RIM_TYPEMOUSE:
                len = sizeof(input);
                GetRawInputData(hRawInput, RID_INPUT, &input, &len, sizeof(header));
                PresentData(input, sData);
                s.Format(L"%s Mouse %s (%s)", nInputcode ? L"Bg" : L"Fg", (LPCTSTR)sDevName, (LPCTSTR)sData);
                break;
            case RIM_TYPEHID:
                len = sizeof(info);
                nRes = GetRawInputDeviceInfo(header.hDevice, RIDI_DEVICEINFO, &info, &len);
                PresentData(input, sData);
                if (nRes > 0)
                {
                    s.Format(L"%s Hid %04X:%04X (len %d)", nInputcode ? L"Bg" : L"Fg", info.hid.usUsagePage, info.hid.usUsage, datalen);
                }
                else
                {
                    s.Format(L"%s Hid (len %d)", nInputcode ? L"Bg" : L"Fg", datalen);
                }
                break;
        }
        s += "\n";
        OutputDebugString(s);
    }

    CDialogEx::OnRawInput(nInputcode, hRawInput);
}


void CRawInputDlg::OnInputDeviceChange(unsigned short nState, HANDLE hDevice)
{
    // This feature requires Windows Vista or greater.
    // The symbol _WIN32_WINNT must be >= 0x0600.
    // TODO: Add your message handler code here and/or call default

    CDialogEx::OnInputDeviceChange(nState, hDevice);
}

static void AddToRegistered(RAWINPUTDEVICE *p, UINT& nRegistered, UINT numDevs, RID_DEVICE_INFO& info, HWND m_hWnd)
{
    USHORT usPage = 0;
    USHORT usUsage = 0;
    switch (info.dwType)
    {
        case RIM_TYPEMOUSE: usPage = 1; usUsage = 2; break;
        case RIM_TYPEKEYBOARD: usPage = 1; usUsage = 6; break;
        case RIM_TYPEHID: usPage = info.hid.usUsagePage; usUsage = info.hid.usUsage; break;
        default: break;
    }
    for (UINT i = nRegistered; i < numDevs; ++i)
    {
        if (p[i].usUsage == usUsage && p[i].usUsagePage == usPage)
        {
            // already here
            return;
        }
    }
    if (nRegistered >= numDevs)
    {
        return;
    }
    p[nRegistered].hwndTarget = m_hWnd;
    p[nRegistered].usUsage = usUsage;
    p[nRegistered].usUsagePage = usPage;
    p[nRegistered].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
    nRegistered++;
}


void CRawInputDlg::Refresh()
{
    RAWINPUTDEVICELIST *p = NULL;
    RAWINPUTDEVICE *pRegister = NULL;
    UINT numDevs = 0;
    CString s;
    GetRawInputDeviceList(NULL, &numDevs, sizeof(*p));
    UINT nRegistered = 0;
    if (numDevs)
    {
        s.Format(L"Requesting data for %d devices\n", numDevs);
        OutputDebugString(s);
        p = new RAWINPUTDEVICELIST[numDevs];
        pRegister = new RAWINPUTDEVICE[numDevs];
        INT n = GetRawInputDeviceList(p, &numDevs, sizeof(*p));
        if (n > 0)
        {
            for (INT i = 0; i < n; ++i)
            {
                WCHAR name[256] = L"?????";
                CString s = L"Unclear";
                RID_DEVICE_INFO info;
                info.cbSize = sizeof(info);
                UINT len = ELEMENTS_IN(name);
                GetRawInputDeviceInfo(p[i].hDevice, RIDI_DEVICENAME, name, &len);
                len = sizeof(info);
                INT nRes = GetRawInputDeviceInfo(p[i].hDevice, RIDI_DEVICEINFO, &info, &len);
                if (nRes > 0)
                {
                    switch (info.dwType)
                    {
                        case RIM_TYPEHID:
                            s.Format(L"Hid [%p] %04X %04X %s",
                                p[i].hDevice,
                                info.hid.usUsagePage, info.hid.usUsage, name);
                            break;
                        case RIM_TYPEKEYBOARD:
                            s.Format(L"Kbd %s", name);
                            break;
                        case RIM_TYPEMOUSE:
                            s.Format(L"Mouse %s", name);
                            break;
                        default:
                            break;
                    }
                    INT nIndex = m_Devices.AddString(s);
                    if (nIndex >= 0)
                    {
                        m_Devices.SetItemDataPtr(nIndex, p[i].hDevice);
                    }
                    AddToRegistered(pRegister, nRegistered, numDevs, info, m_hWnd);
                }
                else
                {
                    s.Format(L"Error %d getting device info\n", GetLastError());
                    OutputDebugString(s);
                }
            }
        }
    }
    if (p) delete[] p;
    if (pRegister)
    {
        if (nRegistered && !RegisterRawInputDevices(pRegister, nRegistered, sizeof(*pRegister)))
        {
            MessageBox(L"Failed to register");
        }
        else if (nRegistered)
        {
            m_Registered.SetWindowText(L"All");
        }
        delete[] pRegister;
    }
}

void CRawInputDlg::OnBnClickedRefresh()
{
    m_Devices.ResetContent();
    Refresh();
}


static void UnregisterAll()
{
    UINT nDevs = 0;
    RAWINPUTDEVICE *p = NULL;
    GetRegisteredRawInputDevices(NULL, &nDevs, sizeof(*p));
    if (nDevs)
    {
        p = new RAWINPUTDEVICE[nDevs];
        INT nRes = GetRegisteredRawInputDevices(p, &nDevs, sizeof(*p));
        if (nRes > 0)
        {
            for (UINT i = 0; i < nDevs; ++i)
            {
                p[i].dwFlags = RIDEV_REMOVE;
                p[i].hwndTarget = NULL;
            }
            RegisterRawInputDevices(p, nDevs, sizeof(*p));
        }
        delete[] p;
    }
}

void CRawInputDlg::OnBnClickedApply()
{
    UnregisterAll();
    m_Registered.SetWindowText(L"Nothing registered");
    INT n = m_Devices.GetCurSel();
    if (n >= 0)
    {
        PVOID data = m_Devices.GetItemDataPtr(n);
        RAWINPUTDEVICE dev;
        RID_DEVICE_INFO info;
        info.cbSize = sizeof(info);
        UINT len = sizeof(info);
        INT nRes = GetRawInputDeviceInfo(data, RIDI_DEVICEINFO, &info, &len);
        if (nRes)
        {
            USHORT usPage = 0;
            USHORT usUsage = 0;
            switch (info.dwType)
            {
            case RIM_TYPEMOUSE: usPage = 1; usUsage = 2; break;
            case RIM_TYPEKEYBOARD: usPage = 1; usUsage = 6; break;
            case RIM_TYPEHID: usPage = info.hid.usUsagePage; usUsage = info.hid.usUsage; break;
            default: break;
            }
            dev.hwndTarget = m_hWnd;
            dev.dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
            if (info.dwType != RIM_TYPEHID)
            {
                dev.dwFlags |= RIDEV_NOLEGACY;
            }
            if (info.dwType == RIM_TYPEMOUSE)
            {
                dev.dwFlags |= RIDEV_CAPTUREMOUSE;
            }
            dev.usUsage = usUsage;
            dev.usUsagePage = usPage;
            if (RegisterRawInputDevices(&dev, 1, sizeof(dev)))
            {
                CString s;
                s.Format(L"%04X:%04X", dev.usUsagePage, dev.usUsage);
                m_Registered.SetWindowText(s);
            }
        }
    }
}
