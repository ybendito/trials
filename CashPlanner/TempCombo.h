#pragma once

#include "TempControl.h"

// CTempCombo

class CTempCombo : public CComboBox, public CTempControl
{
	DECLARE_DYNAMIC(CTempCombo)

public:
	CTempCombo(CTempControl::Callback& _cbIf);
	virtual ~CTempCombo();

protected:
	virtual void StartEditing();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnKillfocus();
};


