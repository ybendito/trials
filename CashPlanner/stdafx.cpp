
// stdafx.cpp : source file that includes just the standard includes
// CashPlanner.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "CommonTypes.h"

LPCTSTR tDate::TimeFormat = _T("dd/MM/yyyy");

bool tDate::FromString(CString& text)
{
	bool b;
	CString s = text;
	s.Trim();
	CStringArray a;

	int n = 0;
	while (n >= 0)
	{
		CString field = s.Tokenize(TEXT("/"), n);
		if (!field.IsEmpty()) a.Add(field);
	}
	n = a.GetCount();
	b = (n == 3);
	if (b)
	{
		int _day = 0, _month = 0, _year = 0;
		b = StringToNumber(a[0], _day, 1, 31);
		b = b && StringToNumber(a[1], _month, 1, 12);
		b = b && StringToNumber(a[2], _year, 2014, 2080);
		if (b)
		{
			day = _day;
			month = _month;
			year = _year;
		}
	}

	if (!b)
	{
		text = GetString();
	}
	return b;
}


bool StringToNumber(const CString& _s, int& val, int min, int max)
{
	CString s = _s;
	bool b, bNegative = false;
	val = 0;
	s.Trim();
	if (!s.IsEmpty() && s[0] == '-')
	{
		bNegative = true;
		s.Delete(0);
	}
	while (!s.IsEmpty())
	{
		TCHAR c = s[0];
		s.Delete(0);
		if (_istdigit(c)) { val = 10 * val; val += c - '0'; }
		else break;
	}
	if (bNegative) val = -val;
	b = (min > max ) || ((val >= min) && (val <= max));
	return b;
}

#ifdef DEBUG
bool bDebug = true;
#else
bool bDebug = false;
#endif

void OutputDebugStringX(LPCTSTR sFormat, ...)
{
	if (bDebug) 
	{
		CString s = sFormat, sLine;
		va_list list;
		va_start(list, sFormat);
		s += TCHAR('\n');
		sLine.FormatV(s, list);
		OutputDebugString(sLine);
	}
}

