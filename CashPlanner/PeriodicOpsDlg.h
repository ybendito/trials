#pragma once
#include "afxcmn.h"
#include "afxdtctl.h"
#include "OperationsList.h"

class CPeriodicOpList : public COperationsList
{
public:
	CPeriodicOpList(COperationsListCallback& cb) : COperationsList(cb) 
	{
		EnableMenu(true);
	}
	// COperationsList
	enum ColumnId { cidFirst = COperationsList::cidLast, cidFromDate = cidFirst, cidToDate, cidPeriod, cidIncrement, cidLast }; 
	virtual void Prepare()
	{
		SetVisibleColumns(
			OPCOLUMN(cidName) | 
			OPCOLUMN(cidSum) | 
			OPCOLUMN(cidClient) |
			OPCOLUMN(cidTaxable) |
			(1 << cidFromDate) |
			(1 << cidToDate) |
			(1 << cidPeriod) |
			(1 << cidIncrement));

		__super::Prepare();
		AddColumn(_T("From"), cidFromDate);
		AddColumn(_T("To"), cidToDate);
		AddColumn(_T("Period"), cidPeriod);
		AddColumn(_T("Increment"), cidIncrement);
	}
protected:
	void FieldChanged(tOperation *p, int columnId, CString& s, bool bApply);
	void OnItemActivate(PVOID p) { }
	void OnItemEdit(PVOID pItem, int columnId);
	void OnAcceptChanges(PVOID p);
	CString PullItemString(PVOID p, int id);
	virtual PVOID OnNewItem() 
	{ 
		tPeriodicOperation *p = new tPeriodicOperation;
		p->FromDate = CurrentTime();
		p->FromDate.day = 1;
		ops->Add(p);
		return p; 
	}
};

// CPeriodicOpsDlg dialog

class CPeriodicOpsDlg : public CDialogEx, public COperationsListCallback
{
	DECLARE_DYNAMIC(CPeriodicOpsDlg)

public:
	CPeriodicOpsDlg(tListOfPeriodicOperations& _list, CStringArray& _names, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPeriodicOpsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_COMMON_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	tListOfPeriodicOperations& list;
	CStringArray& names;
	CArray<tOperation *> ops;
	DECLARE_MESSAGE_MAP()
	CPeriodicOpList m_OpList;
	//COperationsListCallback
	virtual void OnItemActivate(tOperation *p) {}
	bool RequestStrings(tOperationType t, CStringArray& a)
	{
		a.Append(names);
		return !a.IsEmpty();
	}
public:
	virtual BOOL OnInitDialog();
	virtual INT_PTR DoModal();
	virtual void OnOK();
};
