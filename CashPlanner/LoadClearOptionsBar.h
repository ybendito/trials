#pragma once

#include "stdafx.h"
#include "CommonTypes.h"

// CLoadClearOptionsBar dialog

class CLoadClearOptionsBar : public CDialogBar
{
	DECLARE_DYNAMIC(CLoadClearOptionsBar)

public:

	CLoadClearOptionsBar(LoadClearCfg& _cfg);   // standard constructor
	virtual ~CLoadClearOptionsBar();
	void Update(bool bSave);
	bool SetupControl(LoadClearCfg::boxCfg& _cfg, int id);
	void GetControlState(LoadClearCfg::boxCfg& _cfg, int id);

// Dialog Data
	enum { IDD = IDD_OPTIONS_BAR };

protected:
	LoadClearCfg& cfg;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAllData();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};
