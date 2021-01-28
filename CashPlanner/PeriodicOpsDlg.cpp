// PeriodicOpsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "PeriodicOpsDlg.h"
#include "afxdialogex.h"


// CPeriodicOpsDlg dialog

IMPLEMENT_DYNAMIC(CPeriodicOpsDlg, CDialogEx)

CPeriodicOpsDlg::CPeriodicOpsDlg(tListOfPeriodicOperations& _list, CStringArray& _names, CWnd* pParent /*=NULL*/)
	: CDialogEx(CPeriodicOpsDlg::IDD, pParent), m_OpList(*this), list(_list), names(_names)
{

}

CPeriodicOpsDlg::~CPeriodicOpsDlg()
{
}

void CPeriodicOpsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_OpList);
}


BEGIN_MESSAGE_MAP(CPeriodicOpsDlg, CDialogEx)
END_MESSAGE_MAP()


// CPeriodicOpsDlg message handlers

BOOL CPeriodicOpsDlg::OnInitDialog()
{
	__super::OnInitDialog();

	while (!list.IsEmpty())
	{
		tPeriodicOperation *p = list.RemoveHead();
		ops.Add(p);
	}

	m_OpList.Prepare();
	m_OpList.SetOps(&ops);
	m_OpList.FeedItems();
	m_OpList.MakeEditable(true);
	m_OpList.EnableNewDelete(true, true);

	SetWindowText(_T("Periodic operations"));
	SetDlgItemText(IDC_LEGEND, _T("Use right click and double click for editing"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


INT_PTR CPeriodicOpsDlg::DoModal()
{
	// TODO: Add your specialized code here and/or call the base class
	INT_PTR val = __super::DoModal();
	while (!ops.IsEmpty())
	{
		tPeriodicOperation *p = (tPeriodicOperation *)ops[0];
		ops.RemoveAt(0);
		list.AddTail(p);
	}
	return val;
}


void CPeriodicOpsDlg::OnOK()
{

}

void CPeriodicOpList::OnItemEdit(PVOID pItem, int columnId)
{
	switch (columnId)
	{
		case cidFromDate:
		case cidToDate:
			{
				tPeriodicOperation *p = (tPeriodicOperation *)pItem;
				tDate d = columnId == cidFromDate ? p->FromDate : p->ToDate;
				CTime t = d.ToCTime();
				DoDateCtl(p, columnId, t);
			}
			break;
		default:
			__super::OnItemEdit(pItem, columnId);
			break;
	}
}

void CPeriodicOpList::OnAcceptChanges(PVOID pItem)
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
	__super::OnAcceptChanges(p);
}

void CPeriodicOpList::FieldChanged(tOperation *_p, int columnId, CString& s, bool bApply)
{
	tPeriodicOperation *p = (tPeriodicOperation *)_p;
	switch(columnId)
	{
		case cidFromDate:
			if (bApply) 
				p->FromDate.FromString(s);
			p->listdata |= 1 << columnId;
			break;
		case cidToDate:
			if (bApply) 
				p->ToDate.FromString(s);
			p->listdata |= 1 << columnId;
			break;
		case cidPeriod:
			if (!bApply)
				CheckNumber(s, p->period, 1, 12);
			else
				StringToNumber(s, p->period);
			p->listdata |= 1 << columnId;
			break;
		case cidIncrement:
			if (!bApply)
				CheckNumber(s, p->increment);
			else
				StringToNumber(s, p->increment);
			p->listdata |= 1 << columnId;
			break;
		default:
			__super::FieldChanged(p, columnId, s, bApply);
			break;
	}
}

CString CPeriodicOpList::PullItemString(PVOID _p, int id)
{
	tPeriodicOperation *p = (tPeriodicOperation *)_p;
	CString s;
	switch (id)
	{
		case cidFromDate:
			s = p->FromDate.GetString();
			break;
		case cidToDate:
			s = p->ToDate.GetString();
			break;
		case cidPeriod:
			s.Format(_T("%d"), p->period); 
			break;
		case cidIncrement:
			s.Format(_T("%d"), p->increment); 
			break;
		default:
			s = __super::PullItemString(p, id);
			break;
	}
	return s;
}

