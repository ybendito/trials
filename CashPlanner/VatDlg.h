#pragma once

#include "resource.h"

// CVatDlg dialog

class CVatDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVatDlg)

public:
	CVatDlg(CWnd* pParent, ULONG& _val);   // standard constructor
	virtual ~CVatDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_VAT };

protected:
	bool bPlus;
	ULONG& val;
	ULONG lval;
	ULONG rval;
	ULONG maskUpdate;
	void Update();
	void UpdateLR();
	void UpdateRL();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSign();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSwap();
	afx_msg void OnEnChangeLval();
	afx_msg void OnEnChangeRval();
	virtual void OnOK();
};
