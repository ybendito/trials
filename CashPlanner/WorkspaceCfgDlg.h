#pragma once
#include "afxdtctl.h"
#include "CommonTypes.h"

// CWorkspaceCfgDlg dialog

class CWorkspaceCfgDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWorkspaceCfgDlg)

public:
	CWorkspaceCfgDlg(tWorkspaceConfig& _cfg, CWnd* pParent = NULL);   // standard constructor
	virtual ~CWorkspaceCfgDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_WORKSPACE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CDateTimeCtrl m_FromBox;
	CDateTimeCtrl m_ToBox;
protected:
	tWorkspaceConfig& initialConfig;
	tWorkspaceConfig cfg;
public:
	void InternalUpdate();
	bool CheckValid(UINT& id);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedRadioOperational();
	afx_msg void OnBnClickedRadioAnalysis();
	afx_msg void OnEnChangeName();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
