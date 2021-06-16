
// RawInputDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CRawInputDlg dialog
class CRawInputDlg : public CDialogEx
{
// Construction
public:
	CRawInputDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RAWINPUT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnRawInput(UINT nInputcode, HRAWINPUT hRawInput);
    afx_msg void OnInputDeviceChange(unsigned short nState, HANDLE hDevice);
    void Refresh();
//    CCheckListBox m_Devices;
    CListBox m_Devices;
    afx_msg void OnBnClickedRefresh();
    CStatic m_Registered;
    afx_msg void OnBnClickedApply();
};
