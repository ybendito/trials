#pragma once

#include "TempControl.h"

// CTempEdit

class CTempEdit : public CEdit, public CTempControl
{
	DECLARE_DYNAMIC(CTempEdit)

public:
	CTempEdit(CTempControl::Callback& _cbIf);
	virtual ~CTempEdit();
	void StartEditing(); 
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};


