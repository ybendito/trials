#pragma once

#include "CommonTypes.h"

// CEditItems

class CEditItems : public CEdit
{
	DECLARE_DYNAMIC(CEditItems)

public:
	CEditItems(CDialog *parent);
	virtual ~CEditItems();
	tStockItemSet items;
	void Reset(tOperation *p);
protected:
	CDialog *dlg;
	bool bAddToStock;
	bool bIgnoreChange;
	int LastCost;
	void InternalUpdate();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEnUpdate();
	afx_msg void OnEnChange();
};


