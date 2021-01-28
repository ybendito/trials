#pragma once

class tExcelConverter
{
public:
	tExcelConverter();
	~tExcelConverter();
	bool Start(LPCTSTR filename);
	CString ResultFile();
protected:
	CTempFileName *pTempFile;
};