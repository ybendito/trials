#pragma once
// CBasicList

#include "TempControl.h"

class CBasicList : public CListCtrl, public CTempControl::Callback
{
	DECLARE_DYNAMIC(CBasicList)

	friend class CStockList;
private:
	class tListColumn
	{
	public:
		tListColumn(LPCTSTR _name = _T(""), int _id = 0) { name = _name; id = _id; flags.val = 0; }
		tListColumn(tListColumn& col) { name = col.name; id = col.id; }
		CString name;
		int id;
		int index;
		union
		{
			UINT val;
			struct
			{
				UINT fMasked		: 1;
				UINT width			: 16;
			} bits;
		} flags;
	};
	int FindIdByIndex(int index);
	int FindIndexById(int index);
	int FindItemByData(PVOID data);
	tListColumn FindColumnByIndex(int index);

	// for CTempEdit
	void OnTempControlExit(CTempControl *pControl);
	CMenu PopupMenu;
	int	 sortingBy;
	bool bEditable;
	bool bAscending;
	bool bEnableNew;
	bool bEnableDelete;
	bool bShowMenu;
	UINT64 visibleColumns;
public:
	CBasicList();
	virtual ~CBasicList();
	void SetVisibleColumns(UINT64 bitmask) { visibleColumns = bitmask; }
	void EnableNewDelete(bool bAllowNew, bool bAllowDelete) { bEnableNew = bAllowNew; bEnableDelete = bAllowDelete; }
	void EnableMenu(bool b) { bShowMenu = b; }
	virtual PVOID PullItemData(int itemNo) { return NULL; }
	virtual CString PullItemString(PVOID p, int id) { return _T("None"); }
	virtual int CompareItems(PVOID p1, PVOID p2, int id, bool bAscendingOrder) { return 0; }
	void ResizeColumns();
	void SortNow();
	bool WriteItemTextByColumnId(PVOID refData, int colId, CString& s);
	bool ReadItemTextByColumnId(PVOID refData, int colId, CString& s);
	PVOID GetSelectedItemData(int& nSel) 
	{ 
		nSel = GetNextItem(-1, LVNI_SELECTED); 
		return (nSel >= 0) ? (PVOID)GetItemData(nSel) : NULL;
	}
	PVOID GetSelectedItemData() 
	{
		int n;
		return GetSelectedItemData(n);
	}
	void MakeEditable(bool b) { bEditable = b; }
protected:
	virtual void FeedItems(int numberOfItems);
	int AddNewItemToList(PVOID p, int _index);
	bool IsMoreThanOneChanged();
	void RefreshItem(PVOID p, int index);
	void AddColumn(LPCTSTR name, int id)
	{
		tListColumn col(name, id);
		columns.Add(col);
	}
	CArray<tListColumn> columns;
	void DoControl(CTempControl& ctl, PVOID pItem, int columnId);
	void DoEdit(PVOID pItem, int ColumnId);
	void DoDateCtl(PVOID pItem, int ColumnId, const CTime& date);
	void DoCombo(PVOID pItem, int ColumnId, const CStringArray& lines, bool bStrict);
	static int CALLBACK _Comparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	virtual void Prepare();
	virtual void OnItemActivate(PVOID p) {}
	virtual void OnItemEdit(PVOID pItem, int ColumnId) { }
	virtual bool OnItemEditDone(PVOID pItem, int ColumnId, CString& s) { return false; }
	virtual bool WantItemColor(PVOID pItem) { return false; }
	virtual PVOID OnNewItem() { return NULL; }
	virtual bool OnDeleteItem(PVOID p) { return false; }
	virtual void OnAcceptChanges(PVOID p) { }
	virtual void OnRejectChanges(PVOID p) { }
	// not supported yet
	// virtual bool WantSubitemColor(PVOID pItem, int ColumnId) { return false; }


protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHdnItemclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemActivate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnPopupAddnew();
	afx_msg void OnPopupAcceptchanges();
	afx_msg void OnPopupDeleteselecteditem();
	afx_msg void OnPopupAcceptallchanges();
	afx_msg void OnPopupRejectallchanges();
	afx_msg void OnPopupRejectchanges();
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
};
