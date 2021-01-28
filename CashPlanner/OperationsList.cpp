#include "stdafx.h"
#include "CashPlanner.h"
#include "OperationsList.h"

// COperationsList

IMPLEMENT_DYNAMIC(COperationsList, CBasicList)

COperationsList::COperationsList(COperationsListCallback& _cb) : ops(NULL), callbackIf(_cb)
{
	filterOp = foFindString;
}

COperationsList::~COperationsList()
{
}

void COperationsList::Prepare()
{
	AddColumn(_T("Partner"), cidClient);
	AddColumn(_T("Invoice"), cidName);
	AddColumn(_T("Operation"), cidFullName);
	AddColumn(_T("Sum"), cidSum);
	AddColumn(_T("Cost"), cidCost);
	AddColumn(_T("Profit"), cidProfit);
	AddColumn(_T("Action"), cidEffDate);
	AddColumn(_T("Deal"), cidDealDate);
	AddColumn(_T("Reminder"), cidReminderDate);
	AddColumn(_T("Tax"), cidTaxable);
	CBasicList::Prepare();
}

void COperationsList::FeedItems(int numberOfItems)
{
	CBasicList::FeedItems(ops->GetCount());
}

void COperationsList::SetFilter(const CString& _filter, ColumnId _filterBy)
{
	filter = _filter; filterBy = _filterBy;
	if (filterBy == cidSum)
	{
		filterOp = foFindString;
		if (filter[0] == '<')
		{
			filter.Delete(0);
			filterOp = foLess;
		}
		else if (filter[0] == '>')
		{
			filter.Delete(0);
			filterOp = foGreater;
		}
		StringToNumber(filter, filterValue);
	}
}


bool COperationsList::PassesFilter(const tOperation *p)
{
	bool b = true;
	if (filterBy == cidFullName)
	{
		CString sn = p->FullName();
		sn.MakeLower();
		b = sn.Find(filter) >= 0;
	}
	if (filterBy == cidEffDate)
	{
		CString s = p->dateEffective.GetString();
		b = s.Find(filter) >= 0;
	}
	if (filterBy == cidDealDate)
	{
		CString s = p->dateCreated.GetString();
		b = s.Find(filter) >= 0;
	}
	if (filterBy == cidSum)
	{
		switch (filterOp)
		{
			case foGreater:
				b = filterValue < p->sum;
				break;
			case foLess:
				b = filterValue > p->sum;
				break;
			default:
				b = filterValue <= abs(p->sum);
				break;
		}

	}
	if (filterBy == cidReminderDate)
	{
		tDate d = CurrentTime();
		b = d.ToNumber() > (UINT)p->ReminderTime;
	}
	return b;
}

PVOID COperationsList::PullItemData(int itemNo)
{
	if (filter.IsEmpty() || PassesFilter(ops->GetAt(itemNo)))
		return ops->ElementAt(itemNo);
	return NULL;
}

CString COperationsList::PullItemString(PVOID pData, int id)
{
	const tOperation *p = (const tOperation *)pData;
	CString s;
	switch (id)
	{
		case cidClient: s = p->ClientName;  break;
		case cidName: s = p->name; break;
		case cidFullName: s = p->FullName(); break;
		case cidSum: 
			s.Format(_T("%d"), p->sum); 
			if (p->bTaxable)
			{
				int sum = p->sum + VAT(p->sum);
				s.AppendFormat(_T("(%d)"), sum);
			}
			break;
		case cidCost: s.Format(_T("%d"), p->items.TotalCost()); 
			break;
		case cidProfit:
			s.Format(_T("%d"), p->sum - p->items.TotalCost()); 
			break;
		case cidEffDate: 
			s = p->dateEffective.GetString(); 
			break;
		case cidDealDate: 
			s = p->dateCreated.GetString(); 
			break;
		case cidTaxable:
			s = p->bTaxable ? "Yes" : "No";
			break;
		case cidReminderDate:
		{
			tDate d;
			d.FromNumber(p->ReminderTime);
			s = d.GetString(); 
		}
		break;
	}
	return s;
}

int COperationsList::CompareItems(PVOID _p1, PVOID _p2, int id, bool bAscendingOrder)
{
	tOperation *p1 = (tOperation *)_p1;
	tOperation *p2 = (tOperation *)_p2;
	int val1 = 0, val2 = 0, res = 0;
	switch(id)
	{
		case cidFullName:
			res = p1->FullName().CompareNoCase(p2->FullName());
			break;
		case cidName:
			res = p1->name.CompareNoCase(p2->name);
			break;
		case cidClient:
			res = p1->ClientName.CompareNoCase(p2->ClientName);
			break;
		case cidSum:
			res = p1->sum - p2->sum;
			break;
		case cidCost:
			val1 = p1->items.TotalCost();
			val2 = p2->items.TotalCost();
			break;
		case cidProfit:
			val1 = p1->sum - p1->items.TotalCost();
			val2 = p2->sum - p2->items.TotalCost();
			break;
		case cidEffDate:
			val1 = p1->dateEffective.ToNumber();
			val2 = p2->dateEffective.ToNumber();
			break;
		case cidDealDate:
			val1 = p1->dateCreated.ToNumber();
			val2 = p2->dateCreated.ToNumber();
			break;
		case cidReminderDate:
			val1 = p1->ReminderTime;
			val2 = p2->ReminderTime;
			break;
	}
	if (!res) res = val1 - val2;
	if (!bAscendingOrder) res = -res;
	return res;
}

bool COperationsList::WantItemColor(PVOID pItem)
{
	const tOperation *p = (const tOperation *)pItem;
	return p->listdata != 0;
}

void COperationsList::FieldChanged(tOperation *p, int columnId, CString& s, bool bApply)
{
	switch (columnId)
	{
		case cidName:
			s.Trim();
			p->listdata |= 1 << columnId;
			if (bApply) p->name = s;
			break;
		case cidClient:
			s.Trim();
			p->listdata |= 1 << columnId;
			if (bApply) p->ClientName = s;
			break;
		case cidTaxable:
			if (tolower(s[0]) == 'y')
			{
				s = "Yes";
				p->listdata |= 1 << columnId;
				if (bApply) p->bTaxable = true;
			}
			else if (tolower(s[0]) == 'n')
			{
				s = "No";
				p->listdata |= 1 << columnId;
				if (bApply) p->bTaxable = false;
			}
			else
				s = p->bTaxable ? "Yes" : "No";
			break;
		case cidSum:
			if (!bApply)
				CheckNumber(s, p->sum);
			else
				StringToNumber(s, p->sum);
			p->listdata |= 1 << columnId;
			break;
	}
}

bool COperationsList::OnItemEditDone(PVOID pItem, int ColumnId, CString& s)
{
	FieldChanged((tOperation *)pItem, ColumnId, s, false);
	return true;
}

void COperationsList::OnItemEdit(PVOID pItem, int columnId)
{
	CStringArray dummy;
	switch (columnId)
	{
		case cidTaxable:
			{
				CStringArray YesNo;
				YesNo.Add(_T("Yes"));
				YesNo.Add(_T("No"));
				DoCombo(pItem, columnId, YesNo, true);
			}
			break;
		case cidClient:
			{
				tOperation *p = (tOperation *)pItem;
				CStringArray clients;
				if (callbackIf.RequestStrings(p->type, clients))
					DoCombo(pItem, columnId, clients, false);
				else
					DoEdit(pItem, columnId);	
			}
			break;
		case cidFullName:
		case cidProfit:
		case cidCost:
			break;
		default:
			DoEdit(pItem, columnId);	
			break;
	}
}

void COperationsList::OnAcceptChanges(PVOID pItem)
{
	int id;
	tOperation *p = (tOperation *)pItem;
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

void COperationsList::OnRejectChanges(PVOID pItem)
{
	tOperation *p = (tOperation *)pItem;
	p->listdata = 0;
}


bool COperationsList::OnDeleteItem(PVOID _p)
{
	bool b = false;
	for(int i = 0; i < ops->GetCount(); ++i)
	{
		if (ops->ElementAt(i) == (tOperation *)_p)
		{
			delete ops->ElementAt(i);
			ops->RemoveAt(i);
			b = true;
		}
	}
	return b;
}


BEGIN_MESSAGE_MAP(COperationsList, CBasicList)
END_MESSAGE_MAP()



// COperationsList message handlers


