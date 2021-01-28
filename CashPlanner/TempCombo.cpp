// TempCombo.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "TempCombo.h"


// CTempCombo

IMPLEMENT_DYNAMIC(CTempCombo, CComboBox)

CTempCombo::CTempCombo(CTempControl::Callback& _cbIf) : CTempControl(_cbIf)
{

}

CTempCombo::~CTempCombo()
{
}

void CTempCombo::StartEditing()
{
	if (!GetSafeHwnd())
	{
		ULONG ulStyle = data.bStrictCombo ? CBS_DROPDOWNLIST : CBS_DROPDOWN;
		ulStyle |= CBS_AUTOHSCROLL | WS_CHILD	/*| WS_BORDER*/;
		Create(	ulStyle, 
			data.rect, data.pParent, 
			IDC_EDIT_FRAME);
	}
    else
	{
		MoveWindow(&data.rect, TRUE);
	}

	ResetContent();
	for (int i = 0; i < data.Lines.GetCount(); ++i)
		AddString(data.Lines[i]);
	SelectString(-1, data.Text);

	SetFont(data.pFont, FALSE);
	ShowWindow(SW_SHOW); 
	SetFocus();
}



BEGIN_MESSAGE_MAP(CTempCombo, CComboBox)
	ON_CONTROL_REFLECT(CBN_KILLFOCUS, &CTempCombo::OnCbnKillfocus)
END_MESSAGE_MAP()



// CTempCombo message handlers



void CTempCombo::OnCbnKillfocus()
{
	OutputDebugStringX(_T(__FUNCTION__));
	GetWindowText(data.Text);
	ShowWindow(SW_HIDE);
	cbIf.OnTempControlExit(this);
}


