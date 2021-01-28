#pragma once

#include "Docker.h"

// CDockDialog dialog

class CDockDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CDockDialog)

public:
	CDockDialog(UINT idd, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDockDialog();
	bool bAllowOK;
protected:
	struct tDockTable { UINT id; UINT DOCKF_flags; };
	virtual const tDockTable *GetDockedControlsTable() = 0; 
	virtual void ResizeDone(POINT change) {};
	void OnModelessCreate();
	tDocker docker;
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	virtual void OnOK();
};
