#pragma once

#include "BasicList.h"
#include "CommonTypes.h"

class CStockEngine
{
public:
	CStockEngine() {}
	virtual void StockRecalculate(tStockItemSet& _items, const tDate& d) = 0;
	virtual void QueryStockClasses(CStringArray& a) = 0;
	virtual bool QueryClassPerModel(const CString& model, CString& Class) = 0;
	virtual void QueryModelsPerClass(const CString& Class, CStringArray& models) = 0;
};

class CStockListCallback
{
public:
	CStockListCallback() {}
	virtual void OnItemActivate(tStockItem *p) = 0;
};

#define STCOLUMN(name) (1 << CStockList::##name)


// CStockList

class CStockList : public CBasicList
{
	DECLARE_DYNAMIC(CStockList)

public:
	CStockList(tStockItemSet& _stock, CStockListCallback& _cb);
	virtual ~CStockList();
	void Prepare();
	enum ColumnId { cidFirst, cidClass = cidFirst, cidName, cidSource, cidSourceId, cidPrice, cidQuantity, cidTotal, cidDate, cidLast, cidNothing = cidLast }; 
	void FeedItems(int numberOfItems = -1);
	bool PassesFilter(const tStockItem *p);
	void SetFilter(const CString& _filter, ColumnId _filterBy, bool _bShowEmpty)
	{
		filter = _filter; filterBy = _filterBy; bShowEmpty = _bShowEmpty;
	}
	CString GetItemString(const tStockItem *pi, int id);
protected:
	bool bShowEmpty;
	void FieldChanged(tStockItem *p, int colId, CString& s, bool bApply);
	void RedrawTotal(tStockItem *p, ColumnId colId, int fallback, int val);
	CString filter;
	ColumnId filterBy;
	tStockItemSet& stock;
	CStockListCallback& callbackIf;
	void ApplyItemChange(tStockItem *p);
	void DiscardItemChange(tStockItem *p);
//  CBasicList
	PVOID PullItemData(int itemNo);
	CString PullItemString(PVOID p, int id);
	int CompareItems(PVOID p1, PVOID p2, int id, bool bAscendingOrder);
	void OnItemEdit(PVOID pItem, int columnId);
	bool OnItemEditDone(PVOID pItem, int ColumnId, CString& s);
	bool WantItemColor(PVOID pItem);

	DECLARE_MESSAGE_MAP()
};


