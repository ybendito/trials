#pragma once
#include "afxwin.h"
#include "CommonTypes.h"


// CWorkspacesDlg dialog

class CWorkspacesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWorkspacesDlg)

public:
	CWorkspacesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWorkspacesDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MANAGE_WORKSPACES };

	bool bSelect;
	bool bOperational;
	bool bShowArchived;
	CString SelectedName;

protected:
	CStringArray Strings;
	void ShowWorkspaceDetails(tWorkspaceConfig& tempWorkspace);
	void Search(CString& mask);
	void FillList();
	void SetupButtons();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_List;
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedDelete();
	afx_msg void OnSelchangeList1();
	afx_msg void OnBnClickedArchive();
	afx_msg void OnBnClickedUnarchive();
	afx_msg void OnBnClickedShowArchived();
};
