// TempDateBox.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "TempDateBox.h"
#include "CommonTypes.h"


// CTempDateBox

IMPLEMENT_DYNAMIC(CTempDateBox, CDateTimeCtrl)

CTempDateBox::CTempDateBox(CTempControl::Callback& _cbIf) : CTempControl(_cbIf)
{

}

CTempDateBox::~CTempDateBox()
{
}


BEGIN_MESSAGE_MAP(CTempDateBox, CDateTimeCtrl)
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()



// CTempDateBox message handlers




void CTempDateBox::OnKillFocus(CWnd* pNewWnd)
{
	GetWindowText(data.Text);
	GetTime(data.Time);
	__super::OnKillFocus(pNewWnd);
    ShowWindow(SW_HIDE);
	cbIf.OnTempControlExit(this);
}

void CTempDateBox::StartEditing()
{
    if (!GetSafeHwnd())
	{
        Create(
			DTS_SHORTDATEFORMAT | WS_CHILD	/*| WS_BORDER*/ , 
			data.rect, data.pParent, 
			IDC_EDIT_FRAME);
	}
    else
		MoveWindow(&data.rect, TRUE);
	SetTimeFormat(*this);
	SetTime(&data.Time);	

	SetFont(data.pFont, FALSE);
	ShowWindow(SW_SHOW); 
	SetFocus();
}

