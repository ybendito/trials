// StockList.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "StockList.h"

// CStockList

IMPLEMENT_DYNAMIC(CStockList, CBasicList)

CStockList::CStockList(tStockItemSet& _stock, CStockListCallback& _cb) : stock(_stock), callbackIf(_cb)
{
	bShowEmpty = false;
}

CStockList::~CStockList()
{
}

void CStockList::Prepare()
{
	AddColumn(_T("Class"), cidClass);
	AddColumn(_T("Name"), cidName);
	AddColumn(_T("Source"), cidSource);
	AddColumn(_T("Id"), cidSourceId);
	AddColumn(_T("Cost"), cidPrice);
	AddColumn(_T("Quantity"), cidQuantity);
	AddColumn(_T("Total"), cidTotal);
	AddColumn(_T("From"), cidDate);
	CBasicList::Prepare();
}

void CStockList::FeedItems(int numberOfItems)
{
	CBasicList::FeedItems(stock.items.GetCount());
}

bool CStockList::PassesFilter(const tStockItem *p)
{
	bool b = bShowEmpty || p->quantity;
	if (b && !filter.IsEmpty() && filterBy != cidNothing)
	{
		if (filterBy == cidName)
		{
			CString s = p->name;
			s.MakeLower();
			b = s.Find(filter) >= 0;
		}
		if (filterBy == cidSource)
		{
			CString s = p->source + p->sourceId;
			s.MakeLower();
			b = s.Find(filter) >= 0;
		}
	}
	return b;
}

PVOID CStockList::PullItemData(int itemNo)
{
	tStockItem *p = stock.items[itemNo];
	return PassesFilter(p) ? p : NULL;
}

CString CStockList::GetItemString(const tStockItem *pi, int id)
{
	CString s;
	switch (id)
	{
		case cidClass: s = pi->Class; break;
		case cidName: s = pi->name; break;
		case cidSource: s = pi->source; break;
		case cidSourceId: s = pi->sourceId; break;
		case cidPrice: s.Format(_T("%d"), pi->costOfOne); break;
		case cidQuantity: s.Format(_T("%d"), pi->quantity); break;
		case cidTotal: s.Format(_T("%d"), pi->quantity * pi->costOfOne); break;
		case cidDate: s = pi->entryDate.GetString(); break;
	}
	return s;
}

CString CStockList::PullItemString(PVOID pData, int id)
{
	const tStockItem *pi = (const tStockItem *)pData;
	return GetItemString(pi, (ColumnId)id);
}

void CStockList::RedrawTotal(tStockItem *p, ColumnId colId, int fallback, int val)
{
	int total, val1 = fallback;
	CString s;
	if (ReadItemTextByColumnId(p, colId, s))
	{
		StringToNumber(s, val1);
	}
	total = val1 * val;
	s.Format(_T("%d"), total);
	WriteItemTextByColumnId(p, cidTotal, s);
}

void CStockList::FieldChanged(tStockItem *p, int colId, CString& s, bool bApply)
{
	int val = 0;
	switch (colId)
	{
		case cidClass: 
			break;
		case cidName:
			s.Trim();
			p->listdata |= 1 << colId;
			if (bApply) p->name = s;
			break;
		case cidSource: 
			s.Trim();
			p->listdata |= 1 << colId;
			if (bApply) p->source = s;
			break;
		case cidSourceId:
			s.Trim();
			p->listdata |= 1 << colId;
			if (bApply) p->sourceId = s;
			break;
		case cidPrice: 
			StringToNumber(s, val);
			s.Format(_T("%d"), val);
			RedrawTotal(p, cidQuantity, p->quantity, val);
			p->listdata |= 1 << colId;
			break;
		case cidQuantity: 
			StringToNumber(s, val);
			s.Format(_T("%d"), val);
			RedrawTotal(p, cidPrice, p->costOfOne, val);
			p->listdata |= 1 << colId;
			break;
		case cidTotal: 
			break;
	}
}

bool CStockList::WantItemColor(PVOID pItem)
{
	const tStockItem *p = (const tStockItem *)pItem;
	return p->listdata != 0;
}

int CStockList::CompareItems(PVOID _p1, PVOID _p2, int id, bool bAscendingOrder)
{
	const tStockItem *p1 = (const tStockItem *)_p1;
	const tStockItem *p2 = (const tStockItem *)_p2;
	int val1 = 0, val2 = 0, res = 0;
	switch (id)
	{
		case cidClass: 
			res = p1->Class.CompareNoCase(p2->Class);
			break;
		case cidName:
			res = p1->name.CompareNoCase(p2->name);
			break;
		case cidSource:
			res = p1->source.CompareNoCase(p2->source);
			break;
		case cidSourceId:
			res = (p1->source + p1->sourceId).CompareNoCase((p2->source + p2->sourceId));
			break;
		case cidPrice:
			res = p1->costOfOne - p2->costOfOne;
			break;
		case cidQuantity: 
			res = p1->quantity - p2->quantity;
			break;
		case cidTotal:
			val1 = p1->quantity * p1->costOfOne;
			val2 = p2->quantity * p2->costOfOne;
			break;
		case cidDate:
			val1 = p1->entryDate.ToNumber();
			val2 = p2->entryDate.ToNumber();
			break;
	}
	if (!res) res = val1 - val2;
	if (!bAscendingOrder) res = -res;
	return res;
}

bool CStockList::OnItemEditDone(PVOID pItem, int ColumnId, CString& s)
{
	FieldChanged((tStockItem *)pItem, ColumnId, s, false);
	return true;
}

void CStockList::OnItemEdit(PVOID pItem, int columnId)
{
	if (columnId != cidTotal && columnId != cidClass)
	{
		DoEdit(pItem, columnId);	
	}
}

void CStockList::ApplyItemChange(tStockItem *p)
{
	int id;
	for (id = cidFirst; id < cidLast; ++id)
	{
		CString s;
		if (ReadItemTextByColumnId(p, id, s))
		{
		  FieldChanged(p, id, s, true);
		}
	}
	p->listdata = 0;
}

void CStockList::DiscardItemChange(tStockItem *p)
{
	int id;
	for (id = cidFirst; id < cidLast; ++id)
	{
		p->listdata = 0;
		CString s = GetItemString(p, id);
		WriteItemTextByColumnId(p, id, s);
	}
}


BEGIN_MESSAGE_MAP(CStockList, CBasicList)
END_MESSAGE_MAP()



// CStockList message handlers


