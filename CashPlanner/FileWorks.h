#pragma once

#include "stdafx.h"
#include "resource.h"

class CSalesFileDialog : public CFileDialog
{
public:
	CSalesFileDialog() : CFileDialog(true, NULL, NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		NULL, NULL, 0, true)
	{
		HRESULT res;
		
		StartVisualGroup(10, CString("group1"));
		res = AddEditBox(1, CString("Text1"));
		// does not work
		res = SetControlLabel(1, CString("Label1"));
		res = AddCheckButton(2, CString("Check1"), 1);
		EndVisualGroup();

		StartVisualGroup(20, CString("group1"));
		res = AddText(5, CString("Label2"));
		res = AddEditBox(6, CString("Text2"));
		res = AddText(7, CString("Label3"));
		res = AddEditBox(8, CString("Text3"));
		EndVisualGroup();
	}

	BOOL OnFileNameOK()
	{
		BOOL bDebug = true;
		CString s;
		GetEditBoxText(1, s);
		GetCheckButtonState(2, bDebug);
		GetEditBoxText(6, s);
		GetEditBoxText(8, s);
		return false;
	}
};