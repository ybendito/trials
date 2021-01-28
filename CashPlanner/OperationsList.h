#pragma once

#include "BasicList.h"
#include "CommonTypes.h"

class COperationsListCallback
{
public:
	COperationsListCallback() {}
	virtual void OnItemActivate(tOperation *p) = 0;
	virtual bool RequestStrings(tOperationType t, CStringArray& a) { return false; }
};

// COperationsList

#define OPCOLUMN(name) (1 << COperationsList::##name)

class COperationsList : public CBasicList
{
	DECLARE_DYNAMIC(COperationsList)

public:
	COperationsList(COperationsListCallback& _cb);
	virtual ~COperationsList();
	void Prepare();
	enum ColumnId { cidFirst, cidName = cidFirst, cidFullName, cidClient, cidSum, cidCost, cidProfit, cidEffDate, cidDealDate, cidReminderDate, cidTaxable, cidLast, cidNothing = cidLast}; 
	enum FilterOp { foFindString, foGreater, foLess }; 
	void FeedItems(int numberOfItems = -1);
	void SetOps(CArray<tOperation *> *_ops) 
	{ 
		ops = _ops; 
	}
	void SetFilter(const CString& _filter, ColumnId _filterBy = cidFullName);
	bool PassesFilter(const tOperation *p);
protected:
	COperationsListCallback& callbackIf;
	CArray<tOperation *> *ops;
	CString filter;
	ColumnId filterBy;
	int filterValue;
	FilterOp filterOp;

	virtual void FieldChanged(tOperation *p, int columnId, CString& s, bool bApply);
//  CBasicList
	PVOID PullItemData(int itemNo);
	CString PullItemString(PVOID p, int id);
	int CompareItems(PVOID p1, PVOID p2, int id, bool bAscendingOrder);
	void OnItemActivate(PVOID p) { callbackIf.OnItemActivate((tOperation *)p); }
	void OnItemEdit(PVOID pItem, int columnId);
	bool OnItemEditDone(PVOID pItem, int ColumnId, CString& s);
	bool WantItemColor(PVOID pItem);
	void OnAcceptChanges(PVOID p);
	void OnRejectChanges(PVOID p);
	bool OnDeleteItem(PVOID p);

	DECLARE_MESSAGE_MAP()
};


