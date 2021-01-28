#pragma once

#include "CashPlannerDlg.h"

struct tNamedInt
{
	int num;
	CString name;
	tNamedInt() { num = 0; }
	tNamedInt(const tNamedInt& i) { num = i.num; name = i.name; }
};

class tOperationsParser
{
public:
	tOperationsParser(tOperationType _type, bool _bDebug) { type = _type; bDebug = _bDebug; }
	virtual ~tOperationsParser() {}
	virtual bool Parse(tArrayOfOperations& ops, CArray<tNamedInt> *PreparsingPoints = NULL) = 0;
	virtual UINT Raise(UINT flags, LPCTSTR format, ...) = 0;
	bool GetNumber(CStringArray& a, INT index, int& val, int min = 1, int max = -1, bool RaiseIfBad = false);
	bool IsDebug() const { return bDebug; }
	bool GetLog(CString& s) const { s = log; return !log.IsEmpty(); }
protected:
	tOperationType type;
	bool bDebug;
	CString log;
};

class tOperationsParserX : public tOperationsParser
{
public:
	tOperationsParserX() : tOperationsParser(opNone, false) {}
    bool Parse(tArrayOfOperations& ops, CArray<tNamedInt> *PreparsingPoints = NULL) { return false; }
	UINT Raise(UINT flags, LPCTSTR format, ...) { return 0; }
};

class tTextParser : public tOperationsParser
{
public:
	tTextParser(tOperationType _type, const CString& file, bool _bDebug = false) :  
	  tOperationsParser(_type, _bDebug), fileName(file) 
	  {
		 memset(&Settings, 0, sizeof(Settings));
		 currentLine = 0;
		 Settings.firstLine = 1;
		 Settings.lastLine = INT_MAX;
	  }
	virtual bool Parse(tArrayOfOperations& ops, CArray<tNamedInt> *PreparsingPoints = NULL);
protected:
	virtual bool GetDate(CStringArray& a, INT index, tDate& date, bool bRaiseIfBad);
	virtual bool FeedOperation(CStringArray& a) = 0;
	virtual bool PreparseOperation(CStringArray& a, CArray<tNamedInt> *PreparsingPoints) = 0;
	virtual bool FinishParsing(bool bOK, tArrayOfOperations& ops) = 0;
	virtual UINT Raise(UINT flags, LPCTSTR format, ...)
	{
		bool bNoLine = format[0] == '-';
		va_list va;
		va_start(va, format);
		CString s;
		if (bNoLine) format++;
		s.FormatV(format, va);
		if (!bNoLine) s.AppendFormat(TEXT(" (line %d)"), currentLine);
		log += s;
		log += _T("\r\n");
		return AfxMessageBox(s, flags);
	}
	CString fileName;
	TCHAR buffer[4096];
	tDate currentMonth;
	int currentLine;
	struct tParsingSettings
	{
		int firstLine;
		int lastLine;
		int nMinumumFieldsToParse;
		bool bErrorOnLessThanMinimum;
		int nStandardTransportFee;
	} Settings;
};

class tSalesFileTextParser : public tTextParser
{
public:
	tSalesFileTextParser(const CString& file, int firstLine, int lastLine, bool _bDebug = false);
protected:
	typedef CMap<int,int,PVOID,PVOID> tMap;
	void GetTransportFee(CStringArray& a, INT index, int& sum);
	bool GetTypeOfPayment(CStringArray& a, INT index, tTypeOfPayment& type, bool RaiseIfBad);
	bool UpdateCurrentMonth(CStringArray& a);
	bool FinishParsing(bool bOK, tArrayOfOperations& ops);
	bool IsCancelled(CStringArray& a, int index);
	void ClearQuotesEtc(CString& s);
	bool FeedOperation(CStringArray& a);
	bool PreparseOperation(CStringArray& a, CArray<tNamedInt> *PreparsingPoints);
	bool GetMonthYear(CStringArray& _a, tDate& date, bool bRaise);
	tMap map;
	CString sMonth;
	CString sCredit;
	CString sCash;
	CString sWithout;
	CString sFree;
	CString sLtd;
	CString sCancelled;
	int currentInvoice;
};

class tStockFileTextParser : public tTextParser
{
public:
	tStockFileTextParser(const CString& file, int firstLine, int lastLine, bool _bDebug = false);
protected:
	bool FinishParsing(bool bOK, tArrayOfOperations& ops);
	bool FeedOperation(CStringArray& a);
	bool PreparseOperation(CStringArray& a, CArray<tNamedInt> *PreparsingPoints);
	int nEmpty;
	tArrayOfOperations _ops;
};
