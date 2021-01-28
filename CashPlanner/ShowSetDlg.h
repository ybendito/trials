#pragma once

#include "CommonTypes.h"
#include "StockList.h"
#include "afxdtctl.h"
#include "DockDialog.h"
#include "resource.h"
#include "afxwin.h"

// CShowSetDlg dialog

class CShowSetDlg : public CDockDialog, public CStockListCallback
{
	DECLARE_DYNAMIC(CShowSetDlg)

public:
	enum opMode { modeNone, modeViewByDate, modeViewFile, modePurchase, modeSale };
	CShowSetDlg(CWnd* pParent, tStockItemSet& _items);   // standard constructor
	void SetMode(CStockEngine *p, opMode _mode ) { pEngine = p; mode = _mode; }
	virtual ~CShowSetDlg();
	
// Dialog Data
	enum { IDD = IDD_DIALOG_SIMPLE_STOCK_VIEW };

protected:
	tStockItem *pick1;
	tStockItem *pick2;
	
	opMode mode;
	CString filter;
	CStockList::ColumnId filterBy;

	tStockItemSet& items;
	CStockEngine *pEngine;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void Update();
	void OnItemActivate(tStockItem *p) {}
	const tDockTable *GetDockedControlsTable(); 
	void ResizeDone(POINT change) { if (change.x) m_Items.ResizeColumns(); }

	DECLARE_MESSAGE_MAP()
public:
	CStockList m_Items;
	virtual BOOL OnInitDialog();
	CDateTimeCtrl m_Date;
	afx_msg void OnBnClickedForDate();
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedPick1();
	afx_msg void OnBnClickedPick2();
	afx_msg void OnBnClickedTest();
	afx_msg void OnEnChangeFilter();
	CComboBox m_ComboFilter;
	afx_msg void OnCbnSelchangeComboFilter();
	afx_msg void OnBnClickedCheckShowEmpty();
};
