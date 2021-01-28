// TempEdit.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "TempEdit.h"


// CTempEdit

IMPLEMENT_DYNAMIC(CTempEdit, CEdit)

CTempEdit::CTempEdit(CTempControl::Callback& _if) : CTempControl(_if)
{

}

CTempEdit::~CTempEdit()
{
}


BEGIN_MESSAGE_MAP(CTempEdit, CEdit)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_KEYUP()
END_MESSAGE_MAP()



// CTempEdit message handlers




void CTempEdit::OnSetFocus(CWnd* pOldWnd)
{
	CEdit::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
}


void CTempEdit::OnKillFocus(CWnd* pNewWnd)
{
	GetWindowText(data.Text);
	CEdit::OnKillFocus(pNewWnd);
    ShowWindow(SW_HIDE);
	cbIf.OnTempControlExit(this);
}


void CTempEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
    {
        case VK_TAB : 
        case VK_UP :
        case VK_DOWN :
        case VK_RETURN : GetParent()->SetFocus(); 
    }
	
	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);

	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CTempEdit::StartEditing() 
{ 
    if (!GetSafeHwnd())
	{
        Create(
			ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_BORDER, 
			data.rect, data.pParent, 
			IDC_EDIT_FRAME);
	}
    else
		MoveWindow(&data.rect, TRUE);
	SetFont(data.pFont, FALSE);
	SetWindowText(data.Text);
    ShowWindow(SW_SHOW); 
    SetSel(0, -1);
	SetFocus();
}
