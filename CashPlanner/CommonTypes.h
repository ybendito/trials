#pragma once

#include "stdafx.h"

inline CTime CurrentTime()
{
	SYSTEMTIME t;
	GetLocalTime(&t);
	return CTime(t);	
}

inline int sign(int sum)
{
	if (sum > 0) return 1;
	if (sum < 0) return -1;
	return 0;
}

inline int Multiply(float a, int b)
{
	float f = (float)b;
	f = f * a;
	f += 0.5f * sign(b);
	return (int)f;
}

inline int VAT(int sum)
{
	return Multiply(0.18f, sum);
}

inline int TAX(int sum)
{
	return Multiply(0.03f, sum);
}

class CWriteProfile : public CFile
{
public:
	CWriteProfile(LPCTSTR name) : CFile(name, CFile::modeWrite | CFile::modeCreate)
	{
		BYTE buf[] = { 0xFF, 0xFE };
		Write(buf, sizeof(buf));
	}
	void WriteString(const CString& s)
	{
		Write((LPCTSTR)s, s.GetLength() * sizeof(s[0]));
	}
	void WriteProfileString(LPCTSTR _sSection, LPCTSTR sKey, LPCTSTR sValue)
	{
		CString s;
		CString sSection = _sSection;
		sSection.MakeUpper();
		sections.Lookup(sSection, s);
		s.AppendFormat(_T("%s=%s\r\n"), sKey, sValue);
		sections.SetAt(sSection, s);
	}
	void WriteProfileInt(LPCTSTR _sSection, LPCTSTR sKey, int value)
	{
		CString s;
		CString sSection = _sSection;
		sSection.MakeUpper();
		sections.Lookup(sSection, s);
		s.AppendFormat(_T("%s=%d\r\n"), sKey, value);
		sections.SetAt(sSection, s);
	}
	void Flush()
	{
		POSITION pos = sections.GetStartPosition();
		while (pos)
		{
			CString sKey, sVal;
			sections.GetNextAssoc(pos, sKey, sVal);
			CString s;
			s.Format(_T("[%s]\r\n"), sKey);
			WriteString(s);
			WriteString(sVal);
			WriteString(CString(_T("\r\n")));
		}
		sections.RemoveAll();
		CFile::Flush();
	}
	void Close()
	{
		Flush();
	}
	~CWriteProfile() 
	{ 
		Flush(); 
	}
protected:
	CMap<CString,LPCTSTR,CString,LPCTSTR> sections;
	
};

class CTempFileName
{
public:
	CTempFileName(bool _bPermanent = false) 
	{ 
		TCHAR tempPath[MAX_PATH];
		TCHAR fname[MAX_PATH];
		GetTempPath(ELEMENTS_IN(tempPath), tempPath);
		bPermanent = _bPermanent;
		GetTempFileName(tempPath, _T("x"), 0, fname);
		name = fname;
	}
	CString Name() { return name; }
	~CTempFileName()
	{
		if (!bPermanent)
		{
			DeleteFile(name);
		}
	}
protected:
	bool bPermanent;
	CString name;
};

class tProfileSetup
{
public:
	tProfileSetup(CString& filename)
	{
		CWinApp *a = AfxGetApp();
		keep = a->m_pszProfileName;
		sTemp = filename;
		// should be persistent during object's lifetime
		a->m_pszProfileName = sTemp;
	}
	~tProfileSetup()
	{
		CWinApp *a = AfxGetApp();
		a->m_pszProfileName = keep;
	}
protected:
	LPCTSTR keep;
	CString sTemp;
};

struct tDate
{
	LONG month;
	ULONG year;
	ULONG day;
	static LPCTSTR TimeFormat;
	//static LPCTSTR TimeForm = 
	bool IsValid() const { return month && year; } 
	tDate() {month = year = day = 0; }
	void Normalize()
	{
		while (month > 12)
		{
			month -= 12;
			year++;
		}
		while (month < 1 && year)
		{
			month += 12;
			year--;
		}
	}
	tDate(CTime t)
	{
		month = t.GetMonth();
		year = t.GetYear();
		day = t.GetDay();
	}
	tDate(LONG _month, ULONG _year) 
	{
		month = _month;
		year = _year;
		day = 0;
		Normalize();
	}
	tDate(ULONG _day, LONG _month, ULONG _year) 
	{
		month = _month;
		year = _year;
		day = _day;
		Normalize();
	}
	CString GetString(int fmt = 0) const
	{
		CString s;
		if (IsValid())
		{
			if (!fmt)
			{
				if (day) s.Format(TEXT("%02d/%02d/%d"), day, month, year);
				else s.Format(TEXT("%02d/%d"), month, year);
			}
			else if (fmt == 1)
			{
				LPCTSTR table[12] = { TEXT("Jan"), TEXT("Feb"), TEXT("Mar"), TEXT("Apr"), TEXT("May"), TEXT("Jun"), TEXT("Jul"), TEXT("Aug"), TEXT("Sep"), TEXT("Oct"), TEXT("Nov"), TEXT("Dec") };
				if (day) s.Format(TEXT("%d-%s"), day, table[(month - 1) % 12]);
				else s.Format(TEXT("%02d/%d"), month, year);
			}
		}
		return s;
	}
	bool FromString(CString& text);
	ULONG ToNumber () const
	{
		return day + month * 100 + year * 10000;
	}
	void FromNumber (ULONG val)
	{
		day = val % 100;
		val = (val - day) / 100; 
		month = val % 100;
		val = (val - month) / 100;
		year = val;
	}
	// positive if 'this' is later than 'date'
	int Compare(const tDate& date) const
	{
		int diff = (int)(ToNumber() - date.ToNumber());
		return diff;
	}
	int CompareMonth(const tDate& date) const
	{
		tDate date1, date2;
		date1 = *this;
		date2 = date;
		date1.day = date2.day = 0;
		return date1.Compare(date2);
	}
	CTime ToCTime(ULONG _day = 0)
	{
		CTime t(year, month, _day ? _day : day, 1, 0, 0);
		return t;
	}
	void IfNotValid(tDate& date, ULONG _day = 1)
	{
		if (!IsValid()) *this = date;
		if (!day) day = _day;
		Normalize();
	}
	bool IsCurrent()
	{
		CTime t = CurrentTime();
		return year == t.GetYear() && month == t.GetMonth();
	}
	tDate NextMonth()
	{
		tDate date = *this;
		date.month += 1;
		date.Normalize();
		return date;
	}
};


class tStockItem
{
friend class tStockItemSet;
public:
	tStockItem() { costOfOne = quantity = listdata = 0; }
	tStockItem(const tStockItem& i) { Clone(i); }
	tStockItem& operator = (const tStockItem& i) { Clone(i); return *this; }
	int TotalCost() const { return quantity * costOfOne; }
	tStockItem *Remove(int n, bool bForce = false)
	{
		if (n <= quantity || bForce)
		{
			tStockItem *p = new tStockItem(*this);
			p->quantity = n;
			quantity -= n;
			return p;
		}
		return NULL;
	}
	bool SameClass(const tStockItem *pi) const
	{
		return !Class.CompareNoCase(pi->Class);
	}
	bool IsSame(const tStockItem *pi) const
	{
		bool b = SameClass(pi);
		b = b && !name.CompareNoCase(pi->name);
		b = b && costOfOne == pi->costOfOne;
		b = b && !source.CompareNoCase(pi->source);
		b = b && !sourceId.CompareNoCase(pi->sourceId);
		return b;
	}
	bool Absorb(tStockItem *pi)
	{
		bool b = IsSame(pi);
		if (b)
		{
			quantity += pi->quantity;
			delete pi;
		}
		return b;
	}
//protected:
	CString Class;
	int costOfOne;
	int quantity;
	tDate entryDate;
	CString name;
	CString source;
	CString sourceId;
	// used by list objects
	ULONG listdata;
	CString FullSourceName() const { return source + sourceId; }
	CString Serialize(CWriteProfile *f) const;
	static tStockItem *Deserialize(const CString& s);
	void Clone(const tStockItem& i) 
	{ 
		listdata = 0;
		Class = i.Class; 
		costOfOne = i.costOfOne; 
		name = i.name; 
		source = i.source; 
		sourceId = i.sourceId; 
		quantity = i.quantity; 
		entryDate = i.entryDate;
	}
};

class tUnnamedStockItem : public tStockItem
{
public:
	tUnnamedStockItem(int _quantity) : tStockItem()
	{ 
		costOfOne = 1;
		quantity = _quantity; 
	}

};

class tStockItemSet
{
public:
	int multiplier;
	CArray<tStockItem *> items;
	tStockItemSet()
	{
		multiplier = 1;
	}
	~tStockItemSet()
	{
		FreeAll();
	}
	void FreeAll()
	{
		for (INT i = 0; i < items.GetCount(); ++i)
			delete items[i];
		items.RemoveAll();
	}
	int TotalCost() const
	{
		INT i;
		int sum = 0;
		for (i = 0; i < items.GetCount(); ++i)
			sum += items[i]->TotalCost();
		return sum * multiplier;
	}
	void Include(tStockItem *pi)
	{
		bool bDone = false;
		int i;
		for (i = 0; !bDone && i < items.GetCount(); ++i)
		{
			bDone = items[i]->Absorb(pi);
			if (!items[i]->quantity)
			{
				tStockItem *p = items[i];
				delete p;
				items.RemoveAt(i);
				break;
			}
		}
		if (!bDone) items.Add(pi);
	}
	int QueryQuantityByClass(CString Class)
	{
		INT i, count = 0;
		for (i = 0; i < items.GetCount(); ++i)
		{
			if (!items[i]->Class.CompareNoCase(Class))
				count += items[i]->quantity;
		}
		return count;
	}
	int QueryCostByClass(CString Class)
	{
		INT i, sum = 0;
		for (i = 0; i < items.GetCount(); ++i)
		{
			if (!items[i]->Class.CompareNoCase(Class))
				sum += items[i]->TotalCost();
		}
		return sum;
	}
	int QueryQuantity(const CString& name, bool bAlsoUnnamed = false)
	{
		INT i, count = 0;
		for (i = 0; i < items.GetCount(); ++i)
		{
			bool bInclude = false;
			if (name.IsEmpty())
			{
				bInclude = bAlsoUnnamed || !items[i]->name.IsEmpty();
			}
			else
			{
				bInclude = !items[i]->name.CompareNoCase(name);
			}
			if (bInclude) count += items[i]->quantity;
		}
		return count;
	}
	tStockItem* Enumerate(int& i)
	{
		if (i < items.GetCount()) return items[i++];
		i = -1;
		return NULL;
	}
	void Copy(const tStockItemSet& is, bool bUseMultiplier = false)
	{
		INT i;
		for (i = 0; i < is.items.GetCount(); ++i)
		{
			tStockItem *p = new tStockItem(*is.items[i]);
			if (bUseMultiplier && is.multiplier < 0) p->quantity = -p->quantity;
			Include(p);
		}
	}
	
};

typedef enum tTypeOfPayment
{
	tpUnknown,
	tpDate,
	tpNow,
	tpEndOfMonth,
	tpPlus30,
	tpPlus60,
	tpPlus90
};

typedef enum _tOperationType
{
	opSale,
	opPurchase,
	opExpense,
	opMoney,
	opTaxes,
	opStock,
	opNone,
	opMax
}tOperationType;

CString OpTypeName(tOperationType t);

struct tOperation
{
	CArray<tOperation *> *pContainer;
	CList<tOperation *> ChildList;
	tDate dateCreated;
	tDate dateEffective;
	CString name;
	CString ClientName;
	tOperationType type;
	int sum;
	tStockItemSet items;
	int  ReminderTime;
	bool bTaxable;
	bool bDerived;
	bool bOriginIsConstant;
	ULONG listdata;
	CString Serialize(CWriteProfile *f);
	static tOperation *Deserialize(const CString& s);
	void Clone(const tOperation& _op)
	{
		pContainer = NULL;
		dateCreated = _op.dateCreated;
		dateEffective = _op.dateEffective;
		name = _op.name;
		ClientName = _op.ClientName;
		sum = _op.sum;
		type = _op.type;
		bTaxable = _op.bTaxable;
		bOriginIsConstant = false;
		bDerived = false;
		ReminderTime = 0;
		listdata = 0;
	}
	CString FullName() const
	{
		CString s = ClientName;
		if (!s.IsEmpty()) s += TEXT(".");
		s += name;
		return s;
	}
	tOperation& operator = (const tOperation& _op)
	{
		Clone(_op);
		return *this;
	}
	tOperation(const tOperation& _op)
	{
		Clone(_op);
	}
	tOperation()
	{
		sum = 0; type = opNone; pContainer = NULL;
		ReminderTime = 0;
		listdata = 0;
		bTaxable = bDerived = bOriginIsConstant = false;
	}
	virtual ~tOperation()
	{
		while (ChildList.GetCount())
		{
			tOperation *p = ChildList.RemoveHead();
			delete p;
		}
		if (pContainer)
		{
			INT i;
			for (i =0; i < pContainer->GetCount(); ++i)
			{
				if (pContainer->GetAt(i) == this)
				{
					pContainer->RemoveAt(i);
					break;
				}
			}
		}
	}
};

struct tOperationReminder : public tOperation
{
	tOperationReminder(tOperation *p) 
	{ 
		Clone(*p);
		pOwner = p;
		type = opNone;
		dateEffective.FromNumber(p->ReminderTime);
		bDerived = true; 
	}
	tOperation *pOwner;
};

//typedef CArray<tOperation *> tArrayOfOperations;

class tArrayOfOperations : public CArray<tOperation *>
{
public:
	void ClearItems()
	{
		while (GetCount())
		{
			tOperation *op = GetAt(0);
			delete op;
			RemoveAt(0);
		}	
	}
	~tArrayOfOperations()
	{
		ClearItems();
	}
};

class OpsArray : public tArrayOfOperations
{
public:
	OpsArray(tOperationType _type) { type = _type; }
	tOperationType type;
	INT Add(tOperation *op)
	{
		if (op->type != type)
		{
			AfxMessageBox(TEXT("Something is not good"));
			op->type = type;
		}
		op->pContainer = this;
		return tArrayOfOperations::Add(op);
	}
	~OpsArray()
	{
		while (GetCount())
		{
			tOperation *op = GetAt(0);
			RemoveAt(0);
			op->pContainer = NULL;
			if (!op->bDerived) delete op;
		}
	}
	CString Serialize(CWriteProfile *f);
protected:
};

typedef int tMonthlyIntArray[32];

class tControlEventsSubscriber
{
public:
	virtual void OnRightClick(UINT ctrlId) = 0;
};

struct tPeriodicOperation : public tOperation
{
	tPeriodicOperation() : tOperation(), FromDate(1,1,2014), ToDate(31,12,2080) 
	{
		name = _T("Unnamed");
		type = opExpense;
		increment = 0;
		period = 1;
	}
	tDate FromDate;
	tDate ToDate;
	int   increment;
	int period; // months
	void Serialize(CWriteProfile *f);
	bool CheckPeriod(const tDate& month, int& _sum)
	{
		bool b = true;
		_sum = sum;
		if (period > 1)
		{
			b = false;
			tDate d = FromDate;
			while (!b && d.CompareMonth(ToDate) <= 0)
			{
				b = !month.CompareMonth(d) && _sum;
				d.month += period;
				d.Normalize();
				if (!b) _sum += increment;
			} 
		}
		return b;
	}
};

struct tWorkspaceConfig		
{
	// data members
	CString name;				// not saved 
	CString description;		// saved
	tDate From;					// saved
	tDate To;					// saved
	ULONG fOperational : 1;			// not saved
	ULONG fArchived    : 1;			// not saved
	ULONG fInvalid     : 1;			// not saved
	bool bExisting;				// not saved
	bool bTypeChangeable;		// not saved
	bool bAutosave;				// saved

	tWorkspaceConfig() 
	{ 
		fOperational = false;
		fArchived = false;
		fInvalid = false;
		bExisting = false; 
		bTypeChangeable = true;
		bAutosave = false;
	}

	tWorkspaceConfig& operator =(const tWorkspaceConfig& _w)
	{
		Copy(_w);
		return *this;
	}

	void Copy(const tWorkspaceConfig& _w)
	{
		fOperational = _w.fOperational;
		fArchived = false;
		fInvalid = false;
		bExisting = _w.bExisting; 
		bTypeChangeable = _w.bTypeChangeable;
		bAutosave = _w.bAutosave;
		name = _w.name;
		description = _w.description;
		From = _w.From;
		To = _w.To;
	}
	tWorkspaceConfig(const tWorkspaceConfig& _w)
	{
		Copy(_w);
	}
	static bool MakeBaseDir() 
	{ 
		bool b;
		CString s; 
		b = BaseDir(s);
		if (b)
		{
			b = !!CreateDirectory(s, NULL);
			if (!b) b = GetLastError() == ERROR_ALREADY_EXISTS;
		}
		return b;
	}
	static bool BaseDir(CString& s)
	{
		s.Empty();
		TCHAR path[MAX_PATH] = {0};
		bool b = !!SHGetSpecialFolderPath(NULL, path, CSIDL_COMMON_APPDATA, true);
		if (b)
		{
			s = path;
			s += (TCHAR)'\\';
			s += _T("Other");
		}
		return b;
	}
	CString Filename()
	{
		CString res, s;
		if (BaseDir(res) && !name.IsEmpty())
		{
			s = res;
			s.AppendFormat(_T("\\%s.%s%cpff"), (LPCTSTR)name, fArchived ? _T("_") : _T(""), fOperational ? 'o' : 'a'); 
		}
		return s;
	}
	bool CheckFileExists()
	{
		CFile f;
		CString s = Filename();
		if (!s.IsEmpty() && f.Open(s, CFile::modeReadWrite | CFile::modeNoTruncate))
		{
			return true;
		}
		return false;
	}

	void Serialize(CWriteProfile *f);
	void Deserialize();
};


class tListOfPeriodicOperations : public CList<tPeriodicOperation *>
{
public:
	class Handler
	{
	public:
		virtual void onNextPeriodicOperation(tPeriodicOperation *p, bool bParam) = 0;
	};
	void Serialize(CWriteProfile *f);
	void Deserialize();
	void Enumerate(Handler& handler, bool bParam);
	void FreeAll()
	{
		while (GetCount())
		{
			tPeriodicOperation *p = RemoveHead();
			delete p;
		}
	}
	~tListOfPeriodicOperations() { FreeAll(); }
};

struct LoadClearCfg 
{ 
	struct boxCfg 
	{ 
		bool check; bool show; 
		boxCfg() { check = show = true; }
	};
	LoadClearCfg() : 
		all(allconfig[0]), sales(allconfig[1]), purchases(allconfig[2]), expenses(allconfig[3]),
		cash(allconfig[4]), workspace(allconfig[5]), periodic(allconfig[6]) 
	{}
	void SetAll(bool bCheck, bool bShow)
	{
		for (int i = 0; i < ELEMENTS_IN(allconfig); ++i)
		{
			allconfig[i].check = bCheck;
			allconfig[i].show = bShow;
		}
	}
	boxCfg& all; 
	boxCfg& sales; 
	boxCfg& purchases; 
	boxCfg& expenses; 
	boxCfg& cash; 
	boxCfg& workspace; 
	boxCfg& periodic;
protected:
	boxCfg allconfig[7];
};


struct tMonthlyData
{
	tMonthlyData() : 
	Sales(opSale), 
	Purchases(opPurchase), 
	Expenses(opExpense), 
	Money(opMoney), 
	Stock(opStock)
	{ 
		StartCash = EndCash = LowCash = 0;
	}
	tDate month;
	OpsArray Sales;
	OpsArray Purchases;
	OpsArray Expenses;
	OpsArray Money;
	OpsArray Stock;
	int StartCash;
	int EndCash;
	int LowCash;
	int HighStock;
	int LowStock;
	tMonthlyIntArray CashPerDay; 
	ULONG CashChangeMask;
	CString Serialize();
};

class tMonthlyDataCollection : public CArray<tMonthlyData *>
{
public:	
	tMonthlyDataCollection(tWorkspaceConfig& _ws) : workspaceRef(_ws) {}
	tListOfPeriodicOperations PeriodicOps;
	tWorkspaceConfig& workspaceRef;
	void FreeAll()
	{
		while (GetCount())
		{
			tMonthlyData *p = ElementAt(0);
			RemoveAt(0);
			delete p;
		}
		PeriodicOps.FreeAll();
	}
	~tMonthlyDataCollection()
	{
		FreeAll();
	}
	void Serialize(LPCTSTR profile);
	static void Deserialize(tArrayOfOperations& arr, LoadClearCfg& loadCfg);
	tMonthlyData *FindByDate(const tDate& d)
	{
		for (int i = 0; i < GetCount(); ++i)
		{
			if (!ElementAt(i)->month.CompareMonth(d))
				return ElementAt(i);
		}
		return NULL;
	}
	bool CheckCanClear(LoadClearCfg& clearCfg)
	{
		bool bSales = false;
		bool bPurchases = false;
		bool bCash = false;
		bool bExpenses = false;
		bool bPeriodic = false;
		
		for (int i = 0; i < GetCount(); ++i)
		{
			int j;
			tMonthlyData *pmd = ElementAt(i);
			for (j = 0; !bSales && j < pmd->Sales.GetCount(); ++j)
				bSales = !pmd->Sales[j]->bDerived;
			for (j = 0; !bPurchases && j < pmd->Purchases.GetCount(); ++j)
				bPurchases = !pmd->Purchases[j]->bDerived;
			for (j = 0; !bCash && j < pmd->Money.GetCount(); ++j)
				bCash = !pmd->Money[j]->bDerived;
			for (j = 0; !bExpenses && j < pmd->Expenses.GetCount(); ++j)
				bExpenses = !pmd->Expenses[j]->bDerived;
		}
		
		bPeriodic = PeriodicOps.GetCount() > 0;

		if (!bSales) clearCfg.sales.check = clearCfg.sales.show = false;
		if (!bPurchases) clearCfg.purchases.check = clearCfg.purchases.show = false;
		if (!bCash) clearCfg.cash.check = clearCfg.cash.show = false;
		if (!bExpenses) clearCfg.expenses.check = clearCfg.expenses.show = false;
		if (!bPeriodic) clearCfg.periodic.check = clearCfg.periodic.show = false;


		return bSales || bPurchases || bCash || bExpenses || bPeriodic;
	}
	static void CheckCanLoad(LoadClearCfg& loadCfg, CString& fname);
};


void __inline SetTimeFormat(CDateTimeCtrl& ctl)
{
	ctl.SetFormat(tDate::TimeFormat);
}
