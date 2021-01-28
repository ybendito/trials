#include "stdafx.h"
#include "CommonTypes.h"
#include "ExcelConvert.h"


tExcelConverter::tExcelConverter()
{
	pTempFile = NULL;
}

tExcelConverter::~tExcelConverter()
{
	if (pTempFile) delete pTempFile;
}

static CString GetPassword()
{
	return CString("187warez");
}

static void CreateScript(CString fname)
{
	LPCSTR buffer = "Dim oExcel\nSet oExcel = CreateObject(\"Excel.Application\")\nDim oBook\nSet oBook = oExcel.Workbooks.Open(Wscript.Arguments.Item(0),0,0,5,Wscript.Arguments.Item(1))\noBook.SaveAs WScript.Arguments.Item(2), 42\noBook.Close False\noExcel.Quit\n";
	CStringA s;
	s = fname;
	FILE *f = fopen(s, "w+t");
	fwrite(buffer, strlen(buffer),1,f);
	fclose(f);
}

CString tExcelConverter::ResultFile()
{
	if (pTempFile) return pTempFile->Name();
	return _T("");
}

bool tExcelConverter::Start(LPCTSTR filename)
{
	CString s;
	CTempFileName tmp;
	CString vbsName = tmp.Name() + _T(".vbs");
	CString vbsParameters;
	CStringA sa; 
	if (pTempFile) delete pTempFile;
	pTempFile = new CTempFileName();
	vbsParameters.AppendFormat(_T("\"%s\" \"%s\" "), (LPCTSTR)vbsName, (LPCTSTR)filename);
	vbsParameters += GetPassword(); 
	vbsParameters.AppendFormat(_T(" \"%s\" "), (LPCTSTR)pTempFile->Name());
	s = CString(_T("cscript //b //nologo ")) + vbsParameters;
	CreateScript(vbsName); 
	sa = s;
	DeleteFile(pTempFile->Name());
	int res = system(sa);
	DeleteFile(vbsName);
	return res == 0;
}
