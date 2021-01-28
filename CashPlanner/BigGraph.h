#pragma once

#include "GraphButton.h"
#include "afxwin.h"
#include "DockDialog.h"

// CBigGraph dialog

class CBigGraph : 
	public CDockDialog, 
	public tControlEventsSubscriber
{
	DECLARE_DYNAMIC(CBigGraph)

public:
	CBigGraph(CWnd* pParent);   // standard constructor
	void Toggle();
	void PushData(tMonthlyIntArray *p, int day);
	virtual ~CBigGraph();

// Dialog Data
	enum { IDD = IDD_DIALOG_CASH };

protected:
	bool bVisible;
	CWnd *pParent;
	virtual const tDockTable *GetDockedControlsTable(); 
	virtual void OnRightClick(UINT ctrlId) {}
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CGraphButton m_Graph;
	virtual void OnOK();
	virtual void OnCancel();
};
