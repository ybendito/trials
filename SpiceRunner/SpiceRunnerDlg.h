// SpiceRunnerDlg.h : header file
//

#pragma once

#include "CTaskBarIcon.h"
#include "CServerItem.h"

// CSpiceRunnerDlg dialog
class CSpiceRunnerDlg : public CDialogEx, public CTaskBarIconParent
{
// Construction
public:
	CSpiceRunnerDlg(CWnd* pParent = nullptr);	// standard constructor
    CTaskBarIcon m_Icon;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SPICERUNNER_DIALOG };
#endif

	protected:
    void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
    LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

// Implementation
protected:
	HICON m_hIcon;
    CBitmap m_GreenBitmap;
    CBitmap m_BlackBitmap;
    CServerItemsArray m_Servers;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnFileNew();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
protected:
    //CTaskBarIconParent
    HWND GetWindowHandle() override { return m_hWnd; }
    void PreparePopupMenu(CMenu& Menu) override;
    void ProcessPopupResult(UINT code) override;

protected:
    void Connect(CServerItem& Item);
    void Disconnect(CServerItem& Item);
    void AddServer(CServerItem& Temporary, bool Connect, bool SilentFail);
    bool CanAddItem(CServerItem& Item, UINT maxMatches = 0);
    void PutItemToMenu(const CServerItem& Item, CMenu& Menu);
    void FlushItemsToMenu(CItems& Items, CMenu& Menu);
    void LoadServers();
    void SaveServers();
    void MessageBoxError(const CString& Message)
    {
        MessageBox(Message, NULL, MB_OK | MB_ICONHAND);
    }
};
