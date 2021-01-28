#include "stdafx.h"
#include "BasicList.h"
#include "resource.h"

#include "TempEdit.h"
#include "TempDateBox.h"
#include "TempCombo.h"

// CBasicList

IMPLEMENT_DYNAMIC(CBasicList, CListCtrl)

struct tWindowUpdateLocker
{
	tWindowUpdateLocker(CWnd *_pw) : pw(_pw) { pw->SetRedraw(false); }
	~tWindowUpdateLocker() { pw->SetRedraw(true); }
	bool b;
	CWnd *pw;
};

	
CBasicList::CBasicList()
{
	bEditable = bEnableNew = bEnableDelete = false;
	bShowMenu = false;
	visibleColumns = (UINT64)-1;
	sortingBy = -1;
	bAscending = false;
	PopupMenu.LoadMenu(IDR_MENU2);
}

void CBasicList::Prepare()
{
	ULONG style = GetExtendedStyle();
	style |= LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_INFOTIP | LVS_EX_GRIDLINES;
	SetExtendedStyle(style);
	ModifyStyle(0, LVS_SINGLESEL);
}

CBasicList::~CBasicList()
{

}

void CBasicList::RefreshItem(PVOID p, int index)
{
	for (int j = 0; j < columns.GetCount(); ++j)
	{
		if (columns[j].index >= 0)
		{
			CString s = PullItemString(p, columns[j].id);
			SetItemText(index, columns[j].index, s);
		}
	}
}

int CBasicList::AddNewItemToList(PVOID p, int _index)
{
	int index = InsertItem(_index, _T(""));
	SetItemData(index, (DWORD_PTR)p);
	RefreshItem(p, index);
	return index;
}


void CBasicList::FeedItems(int numberOfItems)
{
	int i;
	tWindowUpdateLocker(this);

	DeleteAllItems();
	while (DeleteColumn(0));

	for (i = 0; i < columns.GetCount(); ++i)
	{
		columns[i].index = -1;
		columns[i].flags.bits.fMasked = !(visibleColumns & (UINT64)(1 << columns[i].id));
		if (!columns[i].flags.bits.fMasked)
			columns[i].index = InsertColumn(i, columns[i].name);
	}

	for (i = 0; i < numberOfItems; ++i)
	{
		PVOID p = PullItemData(i);
		if (p)
		{
			AddNewItemToList(p, i);
		}
	}

	ResizeColumns();

	if (sortingBy != -1)
	{
		SortNow();
	}
}

void CBasicList::ResizeColumns()
{
	WINDOWINFO info;
	info.cbSize = sizeof(info);
	GetWindowInfo(&info);

	int maxWidth = info.rcClient.right - info.rcClient.left;
	int nColumns = 0;
	int currentWidth = 0;

	for (int j = 0; j < columns.GetCount(); ++j)
	{
		if (columns[j].index >= 0)
		{
			SetColumnWidth(columns[j].index, -1);
			nColumns++;
			columns[j].flags.bits.width = GetColumnWidth(columns[j].index);
			currentWidth += columns[j].flags.bits.width;
		}
	}

	if (currentWidth < maxWidth && nColumns)
	{
		int nAdd = (maxWidth - currentWidth) / nColumns;
		for (int j = 0; j < columns.GetCount(); ++j)
		{
			if (columns[j].index >= 0)
			{
				SetColumnWidth(columns[j].index, columns[j].flags.bits.width + nAdd);
			}
		}
	}
}


BEGIN_MESSAGE_MAP(CBasicList, CListCtrl)
	ON_NOTIFY(HDN_ITEMCLICKA, 0, &CBasicList::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CBasicList::OnHdnItemclick)
	ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, &CBasicList::OnLvnItemActivate)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CBasicList::OnNMDblclk)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CBasicList::OnNMCustomdraw)
	ON_NOTIFY_REFLECT(NM_CLICK, &CBasicList::OnNMClick)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_POPUP_ADDNEW, &CBasicList::OnPopupAddnew)
	ON_COMMAND(ID_POPUP_ACCEPTCHANGES, &CBasicList::OnPopupAcceptchanges)
	ON_COMMAND(ID_POPUP_DELETESELECTEDITEM, &CBasicList::OnPopupDeleteselecteditem)
	ON_COMMAND(ID_POPUP_ACCEPTALLCHANGES, &CBasicList::OnPopupAcceptallchanges)
	ON_COMMAND(ID_POPUP_REJECTALLCHANGES, &CBasicList::OnPopupRejectallchanges)
	ON_COMMAND(ID_POPUP_REJECTCHANGES, &CBasicList::OnPopupRejectchanges)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CBasicList::OnLvnItemchanged)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


int CBasicList::FindIndexById(int id)
{
	for (int j = 0; j < columns.GetCount(); ++j)
	{
		if (columns[j].id == id)
		{
			return columns[j].index;
		}
	}
	return -1;	
}

CBasicList::tListColumn CBasicList::FindColumnByIndex(int index)
{
	for (int j = 0; j < columns.GetCount(); ++j)
	{
		if (columns[j].index == index)
		{
			return columns[j];
		}
	}
	tListColumn col(_T("None"), -1);
	return col;
}

int CBasicList::FindIdByIndex(int index)
{
	tListColumn col = FindColumnByIndex(index);
	return col.id;
}

int CALLBACK CBasicList::_Comparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CBasicList *p = (CBasicList *)lParamSort;
	return p->CompareItems((PVOID)lParam1, (PVOID)lParam2, p->sortingBy, p->bAscending);
}

void CBasicList::SortNow()
{
	SortItems(_Comparator, (ULONG_PTR)this);
}

void CBasicList::OnHdnItemclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	tListColumn col = FindColumnByIndex(phdr->iItem);
	OutputDebugStringX(_T("%s:%d (%s)"), _T(__FUNCTION__), phdr->iItem, col.name);

	if (col.id != sortingBy)
	{
		sortingBy = col.id;
		bAscending = false;
	}
	else
	{
		bAscending = !bAscending;
	}
	SortNow();
	*pResult = 0;
}


void CBasicList::OnLvnItemActivate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	PVOID p = (PVOID)GetItemData(pNMIA->iItem);
	OnItemActivate(p);
	*pResult = 0;
}

void CBasicList::OnTempControlExit(CTempControl *pControl)
{
	OutputDebugStringX(_T("%s: %s"), TEXT(__FUNCTION__), pControl->data.Text);
	if (OnItemEditDone(pControl->data.itemData, pControl->data.columnId, pControl->data.Text))
	{
		SetItemText(pControl->data.item, pControl->data.subitem, pControl->data.Text);
	}
	delete pControl;
}

void CBasicList::DoControl(CTempControl& ctl, PVOID pItem, int columnId)
{
	int item = FindItemByData(pItem);
	int subitem = FindIndexById(columnId);
	if (item >= 0 && subitem >= 0)
	{
		ctl.data.itemData = pItem;
		ctl.data.item = item;
		ctl.data.subitem = subitem;
		ctl.data.columnId = columnId;
		GetSubItemRect(item, subitem, LVIR_BOUNDS, ctl.data.rect);
		if (subitem == 0)
		{
			CRect r1;
			if (GetSubItemRect(item, 1, LVIR_BOUNDS, r1))
				ctl.data.rect.right = r1.left;
		}
		ctl.data.Text = GetItemText(item, subitem);
		ctl.data.pParent = this;
		ctl.data.pFont = GetFont();
		OutputDebugStringX(_T("%s: item %d, subitem %d, l %d, t %d, r %d, b %d"), _T(__FUNCTION__), 
			item, subitem, 	ctl.data.rect.left, ctl.data.rect.top, ctl.data.rect.right, ctl.data.rect.bottom);
		ctl.StartEditing();
	}

}

void CBasicList::DoEdit(PVOID pItem, int ColumnId)
{
	CTempEdit *pTemp = new CTempEdit(*this);
	DoControl(*pTemp, pItem, ColumnId);
}

void CBasicList::DoDateCtl(PVOID pItem, int ColumnId, const CTime& date)
{
	CTempDateBox *pTemp = new CTempDateBox(*this);
	pTemp->data.Time = date;
	DoControl(*pTemp, pItem, ColumnId);
}

void CBasicList::DoCombo(PVOID pItem, int ColumnId, const CStringArray& lines, bool bStrict)
{
	CTempCombo *pTemp = new CTempCombo(*this);
	pTemp->data.bStrictCombo = bStrict;
	pTemp->data.Lines.RemoveAll();
	pTemp->data.Lines.Append(lines);
	DoControl(*pTemp, pItem, ColumnId);
}


void CBasicList::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int item, subitem, id;
	item = pNMIA->iItem;
	subitem = pNMIA->iSubItem;
	id = FindIdByIndex(subitem);

	if (item >= 0 && bEditable)
	{
		PVOID pItemData = (PVOID)GetItemData(item);
		OnItemEdit(pItemData, id);
	}
	*pResult = 0;
}

bool CBasicList::WriteItemTextByColumnId(PVOID refData, int colId, CString& s)
{
	int item = FindItemByData(refData);
	int subitem = FindIndexById(colId);
	if (item >= 0 && subitem >= 0)
	{
		SetItemText(item, subitem, s);
		return true;
	}
	return false;
}

int CBasicList::FindItemByData(PVOID data)
{
	int nSel = -1;
	if (GetSelectedItemData(nSel) == data)
		return nSel;
	for (int i = 0; i < GetItemCount(); ++i)
	{
		if ((PVOID)GetItemData(i) == data)
			return i;
	}
	return -1;
}


bool CBasicList::ReadItemTextByColumnId(PVOID refData, int colId, CString& s)
{
	bool b;
	int item = FindItemByData(refData);
	int subitem = FindIndexById(colId);
	b = item >= 0 && subitem >= 0;
	if (b)
	{
		s = GetItemText(item, subitem);
	}
	return b;
}


void CBasicList::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LRESULT res = CDRF_DODEFAULT;
	LPNMLVCUSTOMDRAW pl = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	switch (pl->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			res = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			if (WantItemColor((PVOID)pl->nmcd.lItemlParam))
			{
				pl->clrTextBk = RGB(255,255,128);
				res = CDRF_NEWFONT;
			}
			break;
	}
	*pResult = res;
}


void CBasicList::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int item = pNMIA->iItem;
	if (item < 0)
	{
		OnItemActivate(NULL);
	}
	*pResult = 0;
}

bool CBasicList::IsMoreThanOneChanged()
{
	int nCount = 0;
	for (int i = 0; nCount < 2 && i < GetItemCount(); ++i)
	{
		PVOID p = (PVOID)GetItemData(i);
		if (p && WantItemColor(p))
		{
			++nCount;
		}
	}
	return nCount > 1;
}

void CBasicList::OnContextMenu(CWnd* pWnd, CPoint point)
{
	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	CMenu *pMenu = PopupMenu.GetSubMenu(0);
	if (bShowMenu)
	{
		pMenu->GetMenuInfo(&mi);
		mi.dwStyle |= MNS_CHECKORBMP;
		pMenu->SetMenuInfo(&mi);
	
		bool bSelectedChanged = false;
		bool bMoreThanOneChanged = IsMoreThanOneChanged(); 
		PVOID pSelected = GetSelectedItemData();
		if (pSelected) bSelectedChanged = WantItemColor(pSelected);
		// (TODO) try SetMenuItemInfo with MF_MENUBARBREAK
		// (TODO) try to add some consistent bitmap to all items
		pMenu->EnableMenuItem(ID_POPUP_ACCEPTCHANGES, (bSelectedChanged ? MF_ENABLED :  MF_GRAYED) | MF_BYCOMMAND);
		pMenu->EnableMenuItem(ID_POPUP_REJECTCHANGES, (bSelectedChanged ? MF_ENABLED :  MF_GRAYED) | MF_BYCOMMAND);
		pMenu->EnableMenuItem(ID_POPUP_ADDNEW, (bEnableNew ? MF_ENABLED :  MF_GRAYED) | MF_BYCOMMAND);
		pMenu->EnableMenuItem(ID_POPUP_DELETESELECTEDITEM, ((pSelected && bEnableDelete) ? MF_ENABLED :  MF_GRAYED) | MF_BYCOMMAND);
		pMenu->EnableMenuItem(ID_POPUP_ACCEPTALLCHANGES, ( bMoreThanOneChanged ? MF_ENABLED :  MF_GRAYED) | MF_BYCOMMAND);
		pMenu->EnableMenuItem(ID_POPUP_REJECTALLCHANGES, (bMoreThanOneChanged ? MF_ENABLED :  MF_GRAYED) | MF_BYCOMMAND);
		pMenu->TrackPopupMenu(0, point.x, point.y, this);
	}
}


void CBasicList::OnPopupAddnew()
{
	PVOID p = OnNewItem();
	if (p)
	{
		int index = AddNewItemToList(p, 0);
		if (index >= 0)
		{
			SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);		
		}
	}
}


void CBasicList::OnPopupAcceptchanges()
{
	PVOID p = GetSelectedItemData();
	if (p)
	{
		OnAcceptChanges(p);
		int index = FindItemByData(p);
		RefreshItem(p, index);
	}
}

void CBasicList::OnPopupDeleteselecteditem()
{
	int index = -1;
	PVOID p = GetSelectedItemData(index);
	if (p && index >= 0)
	{
		if (OnDeleteItem(p))
		{
			DeleteItem(index);
			int count = GetItemCount();
			if (index < count && GetItemData(index))
			{
				SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);		
			}
			else if (index && (index - 1) < count && GetItemData(index - 1))
			{
				SetItemState(index - 1, LVIS_SELECTED, LVIS_SELECTED);		
			}
		}
	}
}


void CBasicList::OnPopupAcceptallchanges()
{
	for (int i = 0; i < GetItemCount(); ++i)
	{
		PVOID p = (PVOID)GetItemData(i);
		if (p && WantItemColor(p))
		{
			OnAcceptChanges(p);
			int index = FindItemByData(p);
			RefreshItem(p, index);
		}
	}
}

void CBasicList::OnPopupRejectallchanges()
{
	for (int i = 0; i < GetItemCount(); ++i)
	{
		PVOID p = (PVOID)GetItemData(i);
		if (p && WantItemColor(p))
		{
			int index = FindItemByData(p);
			OnRejectChanges(p);
			RefreshItem(p, index);
		}
	}
}


void CBasicList::OnPopupRejectchanges()
{
	int index = -1;
	PVOID p = GetSelectedItemData(index);
	if (p && index >= 0)
	{
		OnRejectChanges(p);
		RefreshItem(p, index);
	}
}


void CBasicList::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	UINT stateChange = (pNMLV->uNewState ^ pNMLV->uOldState);
	if ((stateChange & LVIS_SELECTED) & (pNMLV->uNewState & LVIS_SELECTED))  
	{
		OnItemActivate(GetSelectedItemData());			
/*
		CString s;
		OutputDebugStringX(TEXT("%s: %d selected"), TEXT(__FUNCTION__), pNMLV->iItem);
*/
	}
	*pResult = 0;
}


void CBasicList::OnDestroy()
{
	OnPopupRejectallchanges();
	__super::OnDestroy();

	// TODO: Add your message handler code here
}
