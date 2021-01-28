#include "stdafx.h"
#include "CommonTypes.h"

#define PROFILE						TEXT(".")
#define PROFILE_CONSTANT_OPS		TEXT("ConstantOps")
#define OP_TYPE						TEXT("Type")
#define OP_SUM						TEXT("Sum")
#define OP_TAX						TEXT("Tax")
#define OP_PERIODIC_FROM			TEXT("From")
#define OP_PERIODIC_TO				TEXT("To")
#define OP_PERIOD					TEXT("Period")
#define OP_INCREMENT				TEXT("Increment")
#define OP_CREATED					TEXT("Created")
#define OP_EFFECTIVE				TEXT("Effective")
#define OP_NAME						TEXT("Name")
#define OP_CLIENT					TEXT("Client")
#define OP_OPERATIONS				TEXT("Ops")
#define OP_DATA						TEXT("Data")
#define OP_COST						TEXT("Inventory")
#define OP_REMINDER					TEXT("Reminder")
#define OP_ITEMS					TEXT("Items")
#define OP_ITEM_CLASS				TEXT("Class")
#define OP_ITEM_COST				TEXT("Cost")
#define OP_ITEM_QUANTITY			TEXT("Quantity")
#define OP_ITEM_NAME				TEXT("Name")
#define OP_ITEM_SOURCE				TEXT("Source")
#define OP_ITEM_SOURCE_ID			TEXT("SourceId")
#define OP_ITEM_DATE				TEXT("Date")
#define OP_WORKSPACE_DESC			TEXT("WS.Description")
#define OP_WORKSPACE_AUTOSAVE		TEXT("WS.AutoSave")
#define OP_WORKSPACE_FROM			TEXT("WS.From")
#define OP_WORKSPACE_TO				TEXT("WS.To")

static void AppendString(CString& sTo, const CString& add)
{
	if (!add.IsEmpty())
	{
		if (!sTo.IsEmpty()) sTo += TEXT(",");
		sTo += add;
	}
}

CString tOperation::Serialize(CWriteProfile *a)
{
	//CWinApp *a = AfxGetApp();
	tOperation *p = this;
	CString section;
	section.Format(TEXT("%p"), p);
	a->WriteProfileInt(section, OP_TYPE, p->type);
	a->WriteProfileInt(section, OP_SUM, p->sum);
	a->WriteProfileInt(section, OP_TAX, p->bTaxable);
	a->WriteProfileInt(section, OP_CREATED, p->dateCreated.ToNumber());
	a->WriteProfileInt(section, OP_EFFECTIVE, p->dateEffective.ToNumber());
	a->WriteProfileString(section, OP_NAME, p->name);
	a->WriteProfileString(section, OP_CLIENT, p->ClientName);
	if (p->ReminderTime)
	{
		a->WriteProfileInt(section, OP_REMINDER, p->ReminderTime);
	}
	int nItem = 0;
	CString sItemSections;
	while (nItem >= 0)
	{
		const tStockItem *pi = p->items.Enumerate(nItem);
		if (pi)
		{
			CString sItem = pi->Serialize(a);
			AppendString(sItemSections, sItem);
		}
	}
	if (!sItemSections.IsEmpty())
	{
		a->WriteProfileString(section, OP_ITEMS, sItemSections);
	}
	return section;
}

tOperation *tOperation::Deserialize(const CString& section)
{
	CWinApp *a = AfxGetApp();
	tOperation *p = new tOperation();
	if (p)
	{
		int cost;
		CString sItems;
		p->type = (tOperationType)a->GetProfileInt(section, OP_TYPE,opNone);
		p->sum = a->GetProfileInt(section, OP_SUM, 0);
		p->bTaxable = !!a->GetProfileInt(section, OP_TAX, 0);
		p->dateCreated.FromNumber(a->GetProfileInt(section, OP_CREATED, 0));
		p->dateEffective.FromNumber(a->GetProfileInt(section, OP_EFFECTIVE, 0));
		p->name = a->GetProfileString(section, OP_NAME);
		p->ClientName = a->GetProfileString(section, OP_CLIENT);
		p->ReminderTime = a->GetProfileInt(section, OP_REMINDER, 0);
		cost = a->GetProfileInt(section, OP_COST, 0);
		sItems = a->GetProfileString(section, OP_ITEMS);
		if (!sItems.IsEmpty())
		{
			int nextItem = 0;
			while (nextItem >= 0)
			{
				CString item = sItems.Tokenize(_T(","), nextItem);
				if (!item.IsEmpty())
				{
					tStockItem *pi = tStockItem::Deserialize(item);
					p->items.Include(pi);
				}
			}
		}
		if (cost && p->items.items.IsEmpty())
		{
			tStockItem *pi = new tUnnamedStockItem(cost);
			p->items.Include(pi);
		}
		if (p->type == opTaxes) p->type = opExpense;
	}
	return p;
}

CString tStockItem::Serialize(CWriteProfile *a) const
{
	CString sItem;
	//CWinApp *a = AfxGetApp();
	const tStockItem *pi = this; 
	sItem.Format(_T("%p"), pi);
	if (!pi->source.IsEmpty())
		a->WriteProfileString(sItem, OP_ITEM_SOURCE, pi->source);
	if (!pi->sourceId.IsEmpty())
		a->WriteProfileString(sItem, OP_ITEM_SOURCE_ID, pi->sourceId);
	if (!pi->name.IsEmpty())
		a->WriteProfileString(sItem, OP_ITEM_NAME, pi->name);
	a->WriteProfileInt(sItem, OP_ITEM_COST, pi->costOfOne);
	a->WriteProfileInt(sItem, OP_ITEM_QUANTITY, pi->quantity);
	if (pi->entryDate.IsValid())
		a->WriteProfileInt(sItem, OP_ITEM_DATE, pi->entryDate.ToNumber());
	if (!pi->Class.IsEmpty())
		a->WriteProfileString(sItem, OP_ITEM_CLASS, pi->Class);
	return sItem;
}

tStockItem *tStockItem::Deserialize(const CString& item)
{
	CWinApp *a = AfxGetApp();
	tStockItem *pi = new tStockItem;
	pi->Class = a->GetProfileString(item, OP_ITEM_CLASS);
	pi->name = a->GetProfileString(item, OP_ITEM_NAME);
	pi->source = a->GetProfileString(item, OP_ITEM_SOURCE);
	pi->sourceId = a->GetProfileString(item, OP_ITEM_SOURCE_ID);
	pi->costOfOne = a->GetProfileInt(item, OP_ITEM_COST, 0);
	pi->quantity = a->GetProfileInt(item, OP_ITEM_QUANTITY, 0);
	pi->entryDate.FromNumber(a->GetProfileInt(item, OP_ITEM_DATE, 0));
	return pi;
}


CString OpsArray::Serialize(CWriteProfile *a)
{
	INT i;
	//CWinApp *a = AfxGetApp();
	CString sSections;
	CString sOps;
	sOps.Format(TEXT("%p.%d"), this, type);
	for (i = 0; i < GetCount(); ++i)
	{
		tOperation *p = GetAt(i);
		if (p->sum && !p->bDerived)
		{
			CString section = p->Serialize(a);
			AppendString(sSections, section);
		}
	}
	if (!sSections.IsEmpty())
	{
		a->WriteProfileString(sOps, OP_OPERATIONS, sSections);
	}
	else
	{
		sOps.Empty();
	}
	return sOps;
}

void tMonthlyDataCollection::Serialize(LPCTSTR profile)
{
	int i;
	CString sections;
	CTime t1, t2;
	CTimeSpan ts;
	CWriteProfile f(profile);
	t1 = CurrentTime();
	for (i = 0; i < GetCount(); ++i)
	{
		tMonthlyData *p = ElementAt(i);
		AppendString(sections, p->Expenses.Serialize(&f));
		AppendString(sections, p->Money.Serialize(&f));
		AppendString(sections, p->Sales.Serialize(&f));
		AppendString(sections, p->Purchases.Serialize(&f));
	}
	if (!sections.IsEmpty())
	{
		f.WriteProfileString(PROFILE, OP_DATA, sections);
	}
	if (!PeriodicOps.IsEmpty())
	{
		PeriodicOps.Serialize(&f);
	}
	workspaceRef.Serialize(&f);
	t2 = CurrentTime();
	ts = t2 - t1;
	OutputDebugStringX(_T("%s: Done in %d sec."), _T(__FUNCTION__), ts.GetSeconds());
}

void tMonthlyDataCollection::Deserialize(tArrayOfOperations& arr, LoadClearCfg& loadCfg)
{
	CWinApp *a = AfxGetApp();
	CString sData = a->GetProfileString(PROFILE, OP_DATA);
	int nStartData = 0;
	while (nStartData >= 0)
	{
		CString sOpsSection = sData.Tokenize(_T(", "), nStartData);
		if (!sOpsSection.IsEmpty())
		{
			int nStartOp = 0;
			CString sOps = a->GetProfileString(sOpsSection, OP_OPERATIONS);
			while (nStartOp >= 0)
			{
				CString section = sOps.Tokenize(_T(", "), nStartOp);
				if (!section.IsEmpty())
				{
					tOperation *p = tOperation::Deserialize(section);
					if (p)
					{
						bool b = false;
						switch (p->type)
						{
							case opSale: b = loadCfg.sales.check; break;
							case opPurchase: b = loadCfg.purchases.check; break;
							case opMoney: b = loadCfg.cash.check; break;
							case opExpense: b = loadCfg.expenses.check; break;
						}
						if (b) 
							arr.Add(p);
					}
				}
			}
		}
	}
}

void tMonthlyDataCollection::CheckCanLoad(LoadClearCfg& loadCfg, CString& fname)
{
	tProfileSetup ps(fname);
	CWinApp *a = AfxGetApp();
	CString s = a->GetProfileString(PROFILE, PROFILE_CONSTANT_OPS);
	if (s.IsEmpty())
		loadCfg.periodic.check = loadCfg.periodic.show = false;
	s = a->GetProfileString(PROFILE, OP_DATA);
	int invalidValue = 0x12345678;
	int testValue = a->GetProfileInt(PROFILE, OP_WORKSPACE_AUTOSAVE, invalidValue);
	if (testValue == invalidValue)
	{
		loadCfg.workspace.check = loadCfg.workspace.show = false;
	}
	int nStartData = 0;
	bool bSales = false;
	bool bPurchases = false;
	bool bCash = false;
	bool bExpenses = false;
	while (nStartData >= 0)
	{
		CString sOpsSection = s.Tokenize(_T(", "), nStartData);
		if (!sOpsSection.IsEmpty())
		{
			int val = opNone, dot;
			dot = sOpsSection.Find('.');
			if (dot > 0)
			{
				sOpsSection.Delete(0, dot + 1);
				StringToNumber(sOpsSection, val);
			}
			switch (val)
			{
				case opSale:
					bSales = true;
					break;
				case opPurchase:
					bPurchases = true;
					break;
				case opExpense:
					bExpenses = true;
					break;
				case opMoney:
					bCash = true;
					break;
			}
		}
	}
	if (!bSales) loadCfg.sales.check = loadCfg.sales.show = false;
	if (!bPurchases) loadCfg.purchases.check = loadCfg.purchases.show = false;
	if (!bExpenses) loadCfg.expenses.check = loadCfg.expenses.show = false;
	if (!bCash) loadCfg.cash.check = loadCfg.cash.show = false;
}


void tListOfPeriodicOperations::Deserialize()
{
	CWinApp *a = AfxGetApp();
	CString s = a->GetProfileString(PROFILE, PROFILE_CONSTANT_OPS);
	int nStart = 0;
	while (nStart >= 0)
	{
		tPeriodicOperation *pOp = new tPeriodicOperation;
		CString token = s.Tokenize(_T(", ;"), nStart);
		pOp->sum = a->GetProfileInt(token, OP_SUM, pOp->sum);
		pOp->bTaxable = !!a->GetProfileInt(token, OP_TAX, pOp->bTaxable);
		pOp->name = a->GetProfileString(token, OP_NAME, token);
		pOp->ClientName = a->GetProfileString(token, OP_CLIENT, pOp->ClientName);
		pOp->period = a->GetProfileInt(token, OP_PERIOD, pOp->period);
		pOp->increment = a->GetProfileInt(token, OP_INCREMENT, pOp->increment);
		pOp->FromDate.FromNumber(a->GetProfileInt(token, OP_PERIODIC_FROM, pOp->FromDate.ToNumber()));
		pOp->ToDate.FromNumber(a->GetProfileInt(token, OP_PERIODIC_TO, pOp->ToDate.ToNumber()));
		if (pOp->sum && pOp->FromDate.IsValid() && pOp->ToDate.IsValid())
		{
			AddTail(pOp);
		}
		else
			delete pOp;
	}
}

void tListOfPeriodicOperations::Enumerate(Handler& handler, bool bParam)
{
	POSITION pos;
	for (pos = GetHeadPosition(); pos != NULL; GetNext(pos))
	{
		CString sItem;
		tPeriodicOperation *pOp = GetAt(pos);
		handler.onNextPeriodicOperation(pOp, bParam);
	}
}


void tListOfPeriodicOperations::Serialize(CWriteProfile *a)
{
	//CWinApp *a = AfxGetApp();
	CString sCollection;
	POSITION pos;
	for (pos = GetHeadPosition(); pos != NULL; GetNext(pos))
	{
		CString sItem;
		tPeriodicOperation *pOp = GetAt(pos);
		sItem.Format(_T("%p"), pOp);
		AppendString(sCollection, sItem);
		a->WriteProfileInt(sItem, OP_SUM, pOp->sum);
		a->WriteProfileInt(sItem, OP_TAX, pOp->bTaxable);
		a->WriteProfileStringW(sItem, OP_NAME, pOp->name);
		a->WriteProfileStringW(sItem, OP_CLIENT, pOp->ClientName);
		a->WriteProfileInt(sItem, OP_PERIODIC_FROM, pOp->FromDate.ToNumber());
		a->WriteProfileInt(sItem, OP_PERIODIC_TO, pOp->ToDate.ToNumber());
		a->WriteProfileInt(sItem, OP_INCREMENT, pOp->increment);
		a->WriteProfileInt(sItem, OP_PERIOD, pOp->period);
	}
	a->WriteProfileString(PROFILE, PROFILE_CONSTANT_OPS, sCollection);
}

void tWorkspaceConfig::Deserialize()
{
	CWinApp *a = AfxGetApp();
	description = a->GetProfileString(PROFILE, OP_WORKSPACE_DESC);
	int num = a->GetProfileInt(PROFILE, OP_WORKSPACE_FROM, 0);
	if (num) From.FromNumber(num);
	num = a->GetProfileInt(PROFILE, OP_WORKSPACE_TO, 0);
	if (num) To.FromNumber(num);
	if (!name.IsEmpty())
	{
		bAutosave = !!a->GetProfileInt(PROFILE, OP_WORKSPACE_AUTOSAVE, 0);
	}
	else
	{
		bAutosave = false;
	}
}

void tWorkspaceConfig::Serialize(CWriteProfile *f)
{
	if (!description.IsEmpty())
		f->WriteProfileString(PROFILE, OP_WORKSPACE_DESC, description);
	f->WriteProfileInt(PROFILE, OP_WORKSPACE_FROM, From.ToNumber());
	f->WriteProfileInt(PROFILE, OP_WORKSPACE_TO, To.ToNumber());
	f->WriteProfileInt(PROFILE, OP_WORKSPACE_AUTOSAVE, bAutosave);
}
