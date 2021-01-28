#pragma once
#include "stdafx.h"
#include "LoadClearOptionsBar.h"
#include "CommonTypes.h"

// CMergeDataDlg dialog

class CMergeDataDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMergeDataDlg)

public:
	CMergeDataDlg(LoadClearCfg& _clear, LoadClearCfg& _load, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMergeDataDlg();

// Dialog Data
	enum { IDD = IDD_LOAD_SNAPSHOT };

	LoadClearCfg& cfgClear;
	LoadClearCfg& cfgLoad;

protected:
	void PrepareControl(CLoadClearOptionsBar& bar, CStatic& ctl);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
protected:
	CLoadClearOptionsBar m_LoadBar;
	CLoadClearOptionsBar m_ClearBar;
	CStatic m_Placeholder1;
	CStatic m_Placeholder2;
};
