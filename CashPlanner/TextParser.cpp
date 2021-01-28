#include "stdafx.h"
#include "TextParser.h"
#include "resource.h"

class tOperation2 : public tOperation
{
public:
	tOperation2() : tOperation() { finalPrice = transportFee = fileLine = 0; bHasK = false;} 
	int fileLine;
	int finalPrice;
	int transportFee;
	bool bHasK;
	bool FinalCheck(tOperationsParser *parser);
};

void tSalesFileTextParser::ClearQuotesEtc(CString& s)
{
	if (s.IsEmpty()) return;
	s.Replace(TEXT("\"\""), TEXT("\""));
	int n = s.GetLength();
	if (n >= 2 && s[0] == '\"' && s[n - 1] == s[0])
	{
		s.Delete(n - 1);
		s.Delete(0);
	}
	s.Replace(sLtd, TEXT(""));
}

bool tTextParser::Parse(tArrayOfOperations& ops, CArray<tNamedInt> *PreparsingPoints)
{
	bool bResult = true;
	FILE *f = _wfopen(fileName, TEXT("rb"));
	currentLine = 0;
	while (bResult && _fgetts(buffer, ELEMENTS_IN(buffer), f))
	{
		currentLine++;
		if (currentLine < Settings.firstLine || currentLine > Settings.lastLine) continue;
		CString sRead = buffer;
		sRead.Remove('\n');
		sRead.Remove('\r');
		if (sRead.IsEmpty()) continue;
		CStringArray a;
		int nPos = 0;
		while (nPos >= 0)
		{
			CString s = sRead.Tokenize(TEXT("\t"), nPos);
			a.Add(s);
		}
		bResult = a.GetCount() >= Settings.nMinumumFieldsToParse;
		if (bResult)
		{
			bResult = !PreparsingPoints ? FeedOperation(a) : PreparseOperation(a, PreparsingPoints);
		}
		else
		{
			// ignore error, if configured
			bResult = !Settings.bErrorOnLessThanMinimum;
		}
	}
	if (f) fclose(f);	

	if (!PreparsingPoints) FinishParsing(bResult, ops);	

	return bResult;
}


bool tSalesFileTextParser::GetTypeOfPayment(CStringArray& a, INT index, tTypeOfPayment& type, bool RaiseIfBad)
{
	const CString& s = a.GetAt(index);
	if (0 <= s.Find(sMonth)) type = tpEndOfMonth;
	else if (0 <= s.Find(sCash)) type = tpNow;
	else if (0 <= s.Find(sCredit)) type = tpEndOfMonth;
	else if (0 <= s.Find(sMonth)) type = tpEndOfMonth;
	else if (0 <= s.Find(TEXT("30"))) type = tpPlus30;
	else if (0 <= s.Find(TEXT("60"))) type = tpPlus60;
	else if (0 <= s.Find(TEXT("90"))) type = tpPlus90;
	if (type == tpUnknown && RaiseIfBad)
	{
		Raise(MB_OK, TEXT(__FUNCTION__) TEXT(": column %d"), index);
	}
	return type != tpUnknown;
}

bool tOperationsParser::GetNumber(CStringArray& a, INT index, int& val, int min, int max, bool RaiseIfBad)
{
	CString s = a.GetAt(index);
	bool b = StringToNumber(s, val, min, max);
	if (!b && RaiseIfBad)
	{
		Raise(MB_OK, TEXT(__FUNCTION__) TEXT(": column %d"), index);
	}
	return b;
}

static bool RecognizeMonth(CString s, int& month)
{
	LPCTSTR table[12] = { TEXT("jan"), TEXT("feb"), TEXT("mar"), TEXT("apr"), TEXT("may"), TEXT("jun"), TEXT("jul"), TEXT("aug"), TEXT("sep"), TEXT("oct"), TEXT("nov"), TEXT("dec") };
	s.MakeLower();
	for (int i = 0; i < ELEMENTS_IN(table); ++i)
	{
		if (s.Find(table[i]) >= 0) { month = i + 1; return true; }
	}
	return false;
}

bool tTextParser::GetDate(CStringArray& _a, INT index, tDate& date, bool bRaiseIfBad)
{
	bool b = true;
	CStringArray a;
	CString s = _a[index];
	s.Trim();
	s.Replace('/','-');
	int nPos = 0;
	while (nPos >= 0)
	{
		CString field = s.Tokenize(TEXT("-"), nPos);
		if (!field.IsEmpty()) a.Add(field);
	}
	int n = a.GetCount();
	if (n == 2)
	{
		// 10-Jan
		int day = 0, month = 0;
		b = GetNumber(a, 0, day, 1, 31, false);
		if (b)
		{
			b = RecognizeMonth(a[1], month);
		}
		if (b)
		{
			date.day = day;
			date.month = month;
			date.year = currentMonth.year;
			b = date.IsValid();
		}
	}
	else if (n == 3)
	{
		// 10/5/15
		int day = 0, month = 0, year = 0;
		b = GetNumber(a, 0, day, 1, 31, false);
		if (b)
		{
			b = GetNumber(a, 1, month, 1, 12, false);
			if (!b)
			{
				b = RecognizeMonth(a[1], month);
			}
		}
		if (b)
		{
			b = GetNumber(a, 2, year, 14, 2080, false);
			if (b && year < 100) year += 2000;
			if (!b)
			{
				year = currentMonth.year + (month < currentMonth.month) ? 1 : 0;
			}
		}
		if (b)
		{
			date.day = day;
			date.month = month;
			date.year = year;
			b = date.IsValid();
		}
	}
	else
		b = false;

	if (!b && bRaiseIfBad)
	{
		Raise(MB_OK, TEXT(__FUNCTION__) TEXT(": column %d"), index);
	}

	return b;
}

static tDate MakeDate(const tDate& date, tTypeOfPayment type)
{
	tDate d = date;
	switch(type)
	{
		case tpNow: break;
		case tpPlus90: d = d.NextMonth(); // no break;
		case tpPlus60: d = d.NextMonth(); // no break;
		case tpPlus30: d = d.NextMonth(); // no break;
		case tpEndOfMonth: d = d.NextMonth(); d.day = 1; break;
		default:
			d = tDate(0, 0);
			break;
	}
	return d;
}

bool tSalesFileTextParser::IsCancelled(CStringArray& a, int index)
{
	return a[index].Find(sCancelled) >= 0;
}

void tSalesFileTextParser::GetTransportFee(CStringArray& a, INT index, int& sum)
{
	if (!GetNumber(a, index, sum))
	{
		if (a[index].Find(sFree))
			sum = -Settings.nStandardTransportFee;
		else
			sum = 0;
	}
}

bool tSalesFileTextParser::GetMonthYear(CStringArray& _a, tDate& date, bool bRaise)
{
	bool b = true;
	CStringArray a;
	CString s = _a[0];
	s.Trim();
	s.Replace('/','-');
	int nPos = 0;
	while (nPos >= 0)
	{
		CString field = s.Tokenize(TEXT("-"), nPos);
		if (!field.IsEmpty()) a.Add(field);
	}
	int n = a.GetCount();
	if (n == 2)
	{
		// Jan-15
		int month = 0, year = 0;
		b = GetNumber(a, 1, year, 14, 2080, false);
		if (b)
		{
			b = RecognizeMonth(a[0], month);
		}
		if (!b)
		{
			if (bRaise)
			{
				Raise(MB_OK, TEXT(__FUNCTION__) TEXT(" failed"));
			}
		}
		else
		{
			date.month = month;
			if (year < 100) year += 2000;
			date.year = year;
		}
	}
	return b;
}

bool tSalesFileTextParser::UpdateCurrentMonth(CStringArray& _a)
{
	bool b = GetMonthYear(_a, currentMonth, true);
	return b;

#if 0
	bool b = true;
	CStringArray a;
	CString s = _a[0];
	s.Trim();
	s.Replace('/','-');
	int nPos = 0;
	while (nPos >= 0)
	{
		CString field = s.Tokenize(TEXT("-"), nPos);
		if (!field.IsEmpty()) a.Add(field);
	}
	int n = a.GetCount();
	if (n == 2)
	{
		// Jan-15
		int month = 0, year = 0;
		b = GetNumber(a, 1, year, 14, 2080, false);
		if (b)
		{
			b = RecognizeMonth(a[0], month);
		}
		if (!b)
		{
			Raise(MB_OK, TEXT(__FUNCTION__) TEXT(" failed"));
		}
		else
		{
			currentMonth.month = month;
			if (year < 100) year += 2000;
			currentMonth.year = year;
		}
	}
	return b;
#endif
}

static void AddItem(tOperation *p, int cost, int quantity, const CString& provider, const CString& productName)
{
	if (!quantity) quantity = 1;
	cost = cost / quantity;
	if (cost)
	{
		tStockItem *pi = new tStockItem;
		pi->quantity = quantity;
		pi->costOfOne = cost;
		pi->name = productName;
		pi->source = provider;
		p->items.Include(pi);
	}
}


// 0 - kind of payment (credit, +something, EOM, cash)
// 1 - total sum
// 2 - invoice
// 3 - interest
// 4 - VAT
// 5 - transportation paid
// 6 - price before VAT
// 7 - cost
// 8 - quantity
// 9 - provider
// 10 - item catalog number
// 11 - client
// 12 - date created

bool tSalesFileTextParser::FeedOperation(CStringArray& a)
{
	bool b = true;
	if (a.GetCount() >= 13)
	{
		if (IsCancelled(a, 0)) return true;
		tOperation2 *pOp = new tOperation2;
		int kindOfPayment = 0;
		int nVat = 0;
		int nInvoice = 0;
		bool bHasK = false;
		int quantity = 0;
		int cost = 0;
		CString provider;
		CString productName;
		pOp->type = opSale;
		pOp->fileLine = currentLine;

		provider = a[9];
		productName = a[10];
		provider.Trim();
		productName.Trim();

		b = b && GetNumber(a, 2, nInvoice, 1, INT_MAX, true);
		b = b && GetNumber(a, 8, quantity, 0, INT_MAX, false);
		if (b)
		{
			PVOID p = NULL;
			bHasK = a.GetAt(2).FindOneOf(_T("kK")) >= 0;
			int nInvoiceForLookup = bHasK ? (nInvoice * 1000 + 999) : nInvoice;
			bool bTheSame = nInvoice == currentInvoice;
			bool bFound = !!map.Lookup(nInvoiceForLookup, p);
			b = bTheSame == bFound;
			if (!b)
			{
				int prevFileLine = currentLine;
				tOperation2 *pCurrent = (tOperation2 *)p;
				if (bFound) prevFileLine = pCurrent->fileLine;
				b = IDRETRY == Raise(MB_ABORTRETRYIGNORE, TEXT("Unordered invoices. Possible due to invoice %d on line %d"), nInvoice, prevFileLine);
				if (b)
				{
					a[2] = a[2] + TEXT("00");
					delete pOp;
					return FeedOperation(a);
				}
			}
			else
			{
				// new operation
				if (!bFound)
				{
					//GetNumber(a, 4, nVat, 0, INT_MAX, false);
					//pOp->bTaxable = nVat != 0;
					pOp->bHasK = bHasK;
					pOp->bTaxable = true;
					pOp->name = a.GetAt(2);
					pOp->ClientName = a.GetAt(11);
					ClearQuotesEtc(pOp->ClientName);
					b = b && GetNumber(a, 7, cost, 0, INT_MAX, true);
					b = b && GetNumber(a, 6, pOp->sum, 0, INT_MAX, true);
					b = b && GetNumber(a, 1, pOp->finalPrice, 0, INT_MAX, true);
					if (b && (cost || pOp->sum || pOp->finalPrice))
					{
						if (!cost || !pOp->sum || !pOp->finalPrice)
						{
							b = IDOK == Raise(MB_OKCANCEL, TEXT("Incomplete invoice %d"), nInvoice);
							if (b)
							{
								delete pOp;
								return true;
							}
						}
						GetTransportFee(a, 5, pOp->transportFee);
						b = b && GetDate(a, 12, pOp->dateCreated, true);
						if (b)
						{
							b = GetDate(a, 0, pOp->dateEffective, false);
							if (!b)
							{
								tTypeOfPayment type = tpUnknown;
								b = GetTypeOfPayment(a, 0, type, true);
								if (b)
								{
									pOp->dateEffective = MakeDate(pOp->dateCreated, type);
								}
							}
							else if (pOp->dateEffective.month < pOp->dateCreated.month)
							{
								// next year payment
								pOp->dateEffective.year++;
							}
						}
						if (b)
						{
							AddItem(pOp, cost, quantity, provider, productName);
							map.SetAt(nInvoiceForLookup, pOp);
							currentInvoice = nInvoice;
						}
						else
						{
							// erroneous
							delete pOp;
						}
					}
					else
					{
						// cancelled
						delete pOp;
					}
				}
				else //existing operation continued
				{
					tOperation2 *pCurrent = (tOperation2 *)p;
					b = b && GetNumber(a, 7, cost, 1, INT_MAX, true);
					b = b && GetNumber(a, 6, pOp->sum, 0, INT_MAX, true);
					if (b)
					{
						pCurrent->sum  += pOp->sum;
						AddItem(pCurrent, cost, quantity, provider, productName);
					}
					else
					{
						b = IDOK == Raise(MB_OKCANCEL, TEXT("Malformed invoice %d"), nInvoice);
					}
					delete pOp;
				}
			}
		}
	}
	else if (a.GetCount() <= 3)
	{
		b = UpdateCurrentMonth(a);
	}
	else
	{
		Raise(MB_OK, TEXT("Unexpected line format"));
		b = false;
	}
	return b;
}

static bool Compare(int a, int b, int Negligible)
{
	return abs(a - b) <= Negligible;
}

bool tOperation2::FinalCheck(tOperationsParser *parser)
{
	CString s;
	int total = sum + transportFee;
	if (bTaxable) total += VAT(total);
	bool b = Compare(total, finalPrice, 2);
	if (!b)
	{
		CString sMsg = TEXT("");
		
		bool bNoVAT = (sum + transportFee) == finalPrice;
		
		if (bNoVAT) sMsg = TEXT("(No VAT?)");
		else sMsg.Format(TEXT(" (diff.%+d)"), total - finalPrice);

		if (bNoVAT)
		{
			bTaxable = false;
			if (!parser->IsDebug()) b = true;
		}
		else if (transportFee && Compare(finalPrice, sum + VAT(sum), 2))
		{
			transportFee = 0;
			sMsg = TEXT("(delivery free?)");
			if (!parser->IsDebug()) b = true;
		}

		if (!b)
		{
			s.Format(TEXT("-%s failed for invoice %s, line %d%s"), TEXT(__FUNCTION__), (LPCTSTR)name, fileLine, (LPCTSTR)sMsg );
			b = IDOK == parser->Raise(MB_OKCANCEL | MB_DEFBUTTON2, s);
		}
	}

	if (b)
	{
		CTimeSpan diff = dateEffective.ToCTime() - dateCreated.ToCTime();
		if (diff.GetDays() > 180 || diff.GetDays() < 0)
		{
			s.Format(TEXT("-%s: Suspicious dates (created %s, effective %s) of invoice %s, line %d"), 
				TEXT(__FUNCTION__),
				(LPCTSTR)dateCreated.GetString(),
				(LPCTSTR)dateEffective.GetString(),
				(LPCTSTR)name, 
				fileLine);
			b = IDOK == parser->Raise(MB_OKCANCEL | MB_DEFBUTTON2, s);
		}
	}
	
	if (b)
	{
		sum = sum + transportFee;
	}

	return b;
}

bool tSalesFileTextParser::PreparseOperation(CStringArray& a, CArray<tNamedInt> *PreparsingPoints)
{
	bool b = true; 
	tDate date;
	if (a.GetCount() < 4 && GetMonthYear(a, date, false) && date.IsValid())
	{
		tNamedInt t;
		t.num = currentLine;
		t.name = date.GetString();
		PreparsingPoints->Add(t);
	}
	else
	{
		int i, nEmpties = 0, n = a.GetCount();
		CString s;
		for (i = 0; i < n; ++i)
		{
			nEmpties += a[i].IsEmpty();
			s += a[i];
		}
		if (n == 2 && nEmpties == 1 && s.GetLength() == 1)
		{
			b = false;
			tNamedInt t;
			t.num = - (currentLine - 1);
			t.name = _T("End of file");
			PreparsingPoints->Add(t);
		}
	}
	return b;
}


bool tSalesFileTextParser::FinishParsing(bool bOK, tArrayOfOperations& ops)
{
	bool b = bOK;
	tArrayOfOperations temp;
	while (!map.IsEmpty())
	{
		tMap::CPair *p = map.PGetFirstAssoc();
		tOperation2 *pOp = (tOperation2 *)p->value;
		temp.Add(pOp);
		bOK = bOK && pOp->FinalCheck(this);
		if (bOK && !pOp->bHasK)
			pOp->ReminderTime = pOp->dateEffective.ToNumber();
		map.RemoveKey(p->key);
	}

	while (!temp.IsEmpty())
	{
		tOperation *pOp = temp.GetAt(0);
		temp.RemoveAt(0);
		if (bOK) 
			ops.Add(pOp);
		else
			delete pOp;
	}
	return bOK;
}


tSalesFileTextParser::tSalesFileTextParser(const CString& file, int firstLine, int lastLine, bool _bDebug) :
	  tTextParser(opSale, file, _bDebug)
{
	Settings.nMinumumFieldsToParse = 1;
	Settings.nStandardTransportFee = 35;
	Settings.firstLine = firstLine;
	Settings.lastLine = lastLine;
	currentInvoice = 0;
	sMonth.LoadString(IDS_PA_MONTH);
	sCash.LoadString(IDS_PA_CASH);
	sCredit.LoadString(IDS_PA_CREDIT);
	sWithout.LoadString(IDS_PA_WITHOUT);
	sFree.LoadString(IDS_PA_FREE);
	sLtd.LoadString(IDS_PA_LTD);
	sCancelled.LoadString(IDS_PA_CANCELLED);
}

tStockFileTextParser::tStockFileTextParser(const CString& file, int firstLine, int lastLine, bool _bDebug) :
	  tTextParser(opPurchase, file, _bDebug)
{
	Settings.firstLine = firstLine;
	Settings.lastLine = lastLine;
	nEmpty = 0;
}

bool tStockFileTextParser::PreparseOperation(CStringArray& a, CArray<tNamedInt> *PreparsingPoints)
{
	bool b = true;
	bool bIsEmpty = true;
	for (int i = 0; bIsEmpty && i < a.GetCount(); ++i)
		bIsEmpty = a[i].IsEmpty();

	if (bIsEmpty)
	{
		nEmpty++;
		b = nEmpty < 2;
		if (!b)
		{
			tNamedInt t;
			t.num = 2;
			t.name = _T("Start");
			PreparsingPoints->Add(t);
			
			t.num = currentLine - 1;
			t.name = _T("End");
			PreparsingPoints->Add(t);
		}
	}
	else
	{
		nEmpty = 0;
	}
	return b;
}

bool tStockFileTextParser::FeedOperation(CStringArray& a)
{
	bool b = true;
	if (a.GetCount() >= 7)
	{
		tStockItem *pi = new tStockItem;
		pi->Class = a[0].Trim();
		pi->name = a[1].Trim();
		pi->source = a[2].Trim();
		pi->sourceId = a[3].Trim();
		b = b && GetNumber(a, 4, pi->costOfOne, 1, INT_MAX, true);
		b = b && GetNumber(a, 5, pi->quantity, 0, INT_MAX, true);
		b = b && GetDate(a, 6, pi->entryDate, true);
		if (b && pi->TotalCost())
		{
			tOperation2 *p = new tOperation2;
			p->type = opPurchase;
			p->ClientName = pi->source;
			p->items.Include(pi);
			p->fileLine = currentLine;
			_ops.Add(p);
		}
		else
			delete pi;
	}
	return b;
}

bool tStockFileTextParser::FinishParsing(bool bOK, tArrayOfOperations& ops)
{
	bool b = bOK;
	if (bOK)
	{
		while (_ops.GetCount())
		{
			tOperation2 *p = (tOperation2 *)_ops[0];
			ops.Add(p);
			_ops.RemoveAt(0);
		}
	}
	return b;
}
