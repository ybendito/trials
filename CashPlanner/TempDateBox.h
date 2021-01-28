#pragma once

#include "TempControl.h"

// CTempDateBox

class CTempDateBox : public CDateTimeCtrl, public CTempControl
{
	DECLARE_DYNAMIC(CTempDateBox)

public:
	CTempDateBox(CTempControl::Callback& _cbIf);
	virtual ~CTempDateBox();
	virtual void StartEditing();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};

