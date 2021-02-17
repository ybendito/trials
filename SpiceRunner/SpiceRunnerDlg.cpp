
// SpiceRunnerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "SpiceRunner.h"
#include "SpiceRunnerDlg.h"
#include "CServerDialog.h"
#include "CSettingsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSpiceRunnerDlg dialog

CSpiceRunnerDlg::CSpiceRunnerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SPICERUNNER_DIALOG, pParent), m_Icon(IDI_ICON1, *this)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_GreenBitmap.LoadBitmap(IDB_BITMAP_GREEN);
    m_BlackBitmap.LoadBitmap(IDB_BITMAP_BLACK);
}

void CSpiceRunnerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSpiceRunnerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_DESTROY()
    ON_WM_SHOWWINDOW()
    ON_BN_CLICKED(ID_FILE_NEW, &CSpiceRunnerDlg::OnFileNew)
END_MESSAGE_MAP()


// CSpiceRunnerDlg message handlers

BOOL CSpiceRunnerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    m_Icon.Attach();

    LoadServers();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSpiceRunnerDlg::OnPaint()
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
HCURSOR CSpiceRunnerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSpiceRunnerDlg::OnDestroy()
{
    for (int i = 0; i < m_Servers.GetCount(); ++i)
    {
        CServerItem& item = *m_Servers[i];
        CString name = item.Name();
        BOOL dummy;
        AfxGetApp()->WriteProfileInt(name, _T("Connect"), !item.IsDisconnected(dummy));
    }
    m_Icon.Detach();
    CDialogEx::OnDestroy();
}

void CSpiceRunnerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialogEx::OnShowWindow(bShow, nStatus);
    if (bShow)
    {
        // uncomment to hide
        ShowWindowAsync(m_hWnd, SW_HIDE);
    }
}

LRESULT CSpiceRunnerDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    m_Icon.ProcessIconMessage(message, wParam, lParam);
    return __super::DefWindowProc(message, wParam, lParam);
}

void CSpiceRunnerDlg::PutItemToMenu(const CServerItem& item, CMenu& Menu)
{
    CString s;
    CMenu m;
    BOOL  bConnected;
    BOOL  bActive = !item.IsDisconnected(bConnected);
    UINT flag = 0;
    CString label;
    m.CreatePopupMenu();
    if (!bActive)
    {
        m.AppendMenu(MF_STRING, item.m_Id[CServerItem::esaConnect], _T("Connect"));
        m.AppendMenu(MF_SEPARATOR);
        m.AppendMenu(MF_STRING, item.m_Id[CServerItem::esaEdit], _T("Edit..."));
        m.AppendMenu(MF_STRING, item.m_Id[CServerItem::esaDelete], _T("Delete"));
    }
    else
    {
        flag = MF_CHECKED;
        m.AppendMenu(MF_STRING,
            item.m_Id[CServerItem::esaDisconnect],
            _T("Disconnect"));
    }

    Menu.AppendMenu(MF_STRING | MF_POPUP | flag, (UINT_PTR)m.GetSafeHmenu(), item.Name(true));
    Menu.SetMenuItemBitmaps(Menu.GetMenuItemCount() - 1, MF_BYPOSITION, NULL,
        (bActive && bConnected) ? &m_GreenBitmap : &m_BlackBitmap);
    m.Detach();
}

void CSpiceRunnerDlg::PreparePopupMenu(CMenu& Menu)
{
    // use temporary array of const pointers
    CArray<const CServerItem *> ptrs;
    for (UINT i = 0; i < m_Servers.GetCount(); ++i)
    {
        ptrs.Add(m_Servers[i]);
    }
    // sort the pointers
    std::sort(ptrs.GetData(), ptrs.GetData() + ptrs.GetSize(),
        [](const CServerItem *i1, const CServerItem *i2)
        {
            return *i1 < *i2;
        });

    // present classified by viewer type and sorted inside the type
    for (int v = CServerItem::evwSpice; v < CServerItem::evwInvalid; ++v)
    {
        UINT done = 0;
        for (UINT i = 0; i < ptrs.GetCount(); ++i)
        {
            const CServerItem *item = ptrs[i];
            if (item->m_Viewer == v)
            {
                PutItemToMenu(*item, Menu);
                done++;
            }
        }
        if (done)
        {
            Menu.AppendMenu(MF_SEPARATOR);
        }
    }
    
    Menu.AppendMenu(MF_STRING, ID_FILE_NEW, _T("New..."));
    Menu.AppendMenu(MF_SEPARATOR);
    Menu.AppendMenu(MF_STRING, ID_APP_ABOUT, _T("Settings..."));
    Menu.AppendMenu(MF_STRING, ID_APP_EXIT, _T("Exit"));
}

void CSpiceRunnerDlg::ProcessPopupResult(UINT code)
{
    switch (code)
    {
    case ID_APP_EXIT:
        PostMessage(WM_CLOSE);
        break;
    case ID_FILE_NEW:
        OnFileNew();
        break;
    case ID_APP_ABOUT:
    {
        CSettingsDialog d;
        if (d.DoModal() == IDOK)
        {
            Profile.Save();
        }
        break;
    }
    default:
        for (UINT i = 0; i < m_Servers.GetCount(); ++i)
        {
            CServerItem::eServerAction action;
            if (m_Servers[i]->CheckId(code, action))
            {
                CServerItem& item = *m_Servers.GetAt(i);
                switch (action)
                {
                case CServerItem::esaConnect:
                    Connect(item);
                    break;
                case CServerItem::esaDisconnect:
                    Disconnect(item);
                    break;
                case CServerItem::esaEdit:
                {
                    CServerItem newItem = item;
                    CServerDialog d(newItem);
                    if (d.DoModal() == IDOK)
                    {
                        bool bChangedName = item.Name().CompareNoCase(newItem.Name());
                        if (!CanAddItem(newItem, bChangedName ? 0 : 1))
                        {
                            CString msg;
                            msg.Format(_T("Can't change the server entry to %s.\nSuch server already exists."), newItem.Name().GetString());
                            MessageBoxError(msg);
                            break;
                        }
                        item = newItem;
                        SaveServers();
                        if (item.m_ConnectNow)
                        {
                            Connect(item);
                        }
                    }
                    break;
                }
                case CServerItem::esaDelete:
                    delete m_Servers.GetAt(i);
                    m_Servers.RemoveAt(i);
                    SaveServers();
                    break;
                }
            }
        }
        break;
    }
}

bool IsIdAllocated(UINT id, CServerItemsArray& Servers)
{
    for (UINT i = 0; i < Servers.GetCount(); ++i)
    {
        CServerItem::eServerAction action;
        if (Servers[i]->CheckId(id, action))
        {
            return true;
        }
    }
    return false;
}

UINT GetId(CServerItemsArray& Servers)
{
    for (UINT id = 2000; id < 3000; ++id)
    {
        if (!IsIdAllocated(id, Servers))
            return id;
    }
    MessageBox(NULL, __FUNCTIONW__, NULL, MB_OK);
    return IDC_STATIC;
}

bool CSpiceRunnerDlg::CanAddItem(CServerItem& Item, UINT maxMatches)
{
    UINT matches = 0;

    for (UINT i = 0; i < m_Servers.GetCount(); ++i)
    {
        CString s = m_Servers[i]->Name();
        matches += !s.CompareNoCase(Item.Name());
    }
    return matches <= maxMatches;
}

void CSpiceRunnerDlg::AddServer(CServerItem& Temporary, bool DoConnect, bool Loading)
{
    bool bCanAdd = CanAddItem(Temporary);

    if (!bCanAdd)
    {
        if (Loading)
        {
            Log("%s: Can't add duplicated entry %S", __FUNCTION__, Temporary.Name().GetString());
        }
        else
        {
            CString msg;
            msg.Format(_T("Can't add new entry for %s.\nSuch server already exists."), Temporary.Name().GetString());
            MessageBoxError(msg);
        }
        return;
    }

    INT_PTR index = m_Servers.Add(new CServerItem(Temporary));
    CServerItem& newItem = *m_Servers.GetAt(index);

    newItem.m_Id[CServerItem::esaConnect] = GetId(m_Servers);
    newItem.m_Id[CServerItem::esaDisconnect] = GetId(m_Servers);
    newItem.m_Id[CServerItem::esaEdit] = GetId(m_Servers);
    newItem.m_Id[CServerItem::esaDelete] = GetId(m_Servers);

    if (!Loading)
    {
        SaveServers();
    }

    if (index >= 0 && DoConnect)
    {
        Connect(newItem);
    }
}

void CSpiceRunnerDlg::OnFileNew()
{
    CServerItem initial;
    CServerDialog d(initial);
    if (d.DoModal() == IDOK)
    {
        AddServer(initial, initial.m_ConnectNow, false);
    }
}

BOOL CSpiceRunnerDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return CDialogEx::OnCommand(wParam, lParam);
}

void CSpiceRunnerDlg::Connect(CServerItem& Item)
{
    Item.Connect();
}

void CSpiceRunnerDlg::Disconnect(CServerItem& Item)
{
    Item.Disconnect();
}

void CSpiceRunnerDlg::LoadServers()
{
    int nStart = 0;
    CString servers = AfxGetApp()->GetProfileString(_T("."), _T("Servers"), _T(""));
    do
    {
        CString next = servers.Tokenize(_T(","), nStart);
        if (!next.IsEmpty())
        {
            CServerItem item;
            item.m_HostName = AfxGetApp()->GetProfileString(next, _T("Host"), next);
            item.m_Port = AfxGetApp()->GetProfileInt(next, _T("Port"), 0xffff);
            item.m_Viewer = AfxGetApp()->GetProfileInt(next, _T("Viewer"), CServerItem::evwSpice);
            bool connect = AfxGetApp()->GetProfileInt(next, _T("Connect"), false);
            item.m_WaitTime = AfxGetApp()->GetProfileInt(next, _T("WaitTime"), CProcessRunner::DefaultWaitTime);
            AddServer(item, connect && Profile.m_RestoreConnections, true);
        }
        if (nStart < 0) break;
    } while (true);
}

void CSpiceRunnerDlg::SaveServers()
{
    CString servers;
    for (int i = 0; i < m_Servers.GetCount(); ++i)
    {
        CServerItem& item = *m_Servers[i];
        CString name = item.Name();
        AfxGetApp()->WriteProfileString(name, _T("Host"), item.m_HostName);
        AfxGetApp()->WriteProfileInt(name, _T("Port"), item.m_Port);
        AfxGetApp()->WriteProfileInt(name, _T("Viewer"), item.m_Viewer);
        AfxGetApp()->WriteProfileInt(name, _T("WaitTime"), item.m_WaitTime);
        servers += name;
        servers += _T(",");
    }
    AfxGetApp()->WriteProfileString(_T("."), _T("Servers"), servers);
}
