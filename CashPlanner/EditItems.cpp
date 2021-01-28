// EditItems.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "EditItems.h"
#include "CashPlannerDlg.h"
#include "ShowSetDlg.h"


// CEditItems

IMPLEMENT_DYNAMIC(CEditItems, CEdit)

#define DLG ((CCashPlannerDlg *)dlg)
	
CEditItems::CEditItems(CDialog *parent) : dlg(parent)
{
	bAddToStock = false;
	
}

CEditItems::~CEditItems()
{
}

void CEditItems::InternalUpdate()
{
	CString s;
	LastCost = items.TotalCost();
	if (LastCost) s.Format(_T("%d"), LastCost);
	bIgnoreChange = true;
	SetWindowText(s);
	bIgnoreChange = false;
}


void CEditItems::Reset(tOperation *p)
{
	items.FreeAll();
	items.Copy(p->items);
	EnableWindow(p->type != opMoney && p->type != opExpense);
	bAddToStock = p->type == opPurchase;
	InternalUpdate();
}

BEGIN_MESSAGE_MAP(CEditItems, CEdit)
	ON_WM_LBUTTONDBLCLK()
	ON_CONTROL_REFLECT(EN_UPDATE, &CEditItems::OnEnUpdate)
	ON_CONTROL_REFLECT(EN_CHANGE, &CEditItems::OnEnChange)
END_MESSAGE_MAP()



// CEditItems message handlers




void CEditItems::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//CEdit::OnLButtonDblClk(nFlags, point);
#if 1
	CShowSetDlg dlg(this, items);
	if (IDOK == dlg.DoModal())
	{
	
	}
#else
	DLG->EditItems(items, bAddToStock);			
#endif
	InternalUpdate();
}


void CEditItems::OnEnUpdate()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CEdit::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
}


void CEditItems::OnEnChange()
{
	if (!bIgnoreChange)	
	{
		CString s;
		int val = 0;
		GetWindowText(s);
		StringToNumber(s, val);
		items.FreeAll();
		if (val)
		{
			tStockItem *pi = new tUnnamedStockItem(val);
			items.Include(pi);
		}
	}
}
