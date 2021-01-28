#pragma once

#include "stdafx.h"


class CTempControl
{
public:
	struct tData
	{
		PVOID itemData;
		int item;
		int subitem;
		int columnId;
		CRect rect;
		CFont *pFont;
		CString Text;
		CStringArray Lines;
		bool bStrictCombo;
		CTime Time;
		//CWnd *pControl;
		CWnd *pParent;
	};

	class Callback
	{
	public:
		Callback() {}
		virtual void OnTempControlExit(CTempControl *pControl) = 0;
	};

	tData data;
	CTempControl(Callback& _cbIf) :  cbIf(_cbIf) { }
	virtual void StartEditing() = 0;
	virtual ~CTempControl() {}
protected:
	Callback& cbIf;
};
