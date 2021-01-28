// SalesTimingReport.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "SalesTimingReport.h"
#include "afxdialogex.h"


// CSalesTimingReport dialog

IMPLEMENT_DYNAMIC(CSalesTimingReport, CDialogEx)

CSalesTimingReport::CSalesTimingReport(tMonthlyDataCollection& _data, CWnd* pParent /*=NULL*/)
	: CDialogEx(CSalesTimingReport::IDD, pParent), m_List(_data)
{

}

CSalesTimingReport::~CSalesTimingReport()
{
}

void CSalesTimingReport::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
}


BEGIN_MESSAGE_MAP(CSalesTimingReport, CDialogEx)
END_MESSAGE_MAP()


// CSalesTimingReport message handlers

//

PVOID CSalesTimingReportList::PullItemData(int itemNo) 
{ 
	if (itemNo < table.GetCount())
	{
		return &table.ElementAt(itemNo);
	}
	return NULL;
}

CString CSalesTimingReportList::PullItemString(PVOID p, int id) 
{ 
	tDataUnit *pu = (tDataUnit *)p;
	CString s;
	switch (id)
	{
		case 0:
			pu->date.day = 0;
			s = pu->date.GetString();
			break;
		case 1:
			s.Format(_T("%d"), pu->Total);
			break;
		case 2:
			s.Format(_T("%d"), (pu->Now * 100) / pu->Total);
			break;
		case 3:
			s.Format(_T("%d"), (pu->Next * 100) / pu->Total);
			break;
		case 4:
			s.Format(_T("%d"), (pu->_30  * 100) / pu->Total);
			break;
		case 5:
			s.Format(_T("%d"), (pu->_60 * 100) / pu->Total);
			break;
		case 6:
			s.Format(_T("%d"), (pu->_90 * 100) / pu->Total);
			break;
		default:
			break;
	}
	return s;
}


void CSalesTimingReportList::Prepare()
{
	CBasicList::Prepare();
	AddColumn(_T("Date"), 0);
	AddColumn(_T("Total"), 1);
	AddColumn(_T("Now%"), 2);
	AddColumn(_T("+0%"), 3);
	AddColumn(_T("+30"), 4);
	AddColumn(_T("+60"), 5);
	AddColumn(_T("+90"), 6);

	tDate from = data.workspaceRef.From;
	tDate to = data.workspaceRef.To;

	while (from.CompareMonth(to) <= 0)
	{
		tMonthlyData *pd = data.FindByDate(from);
		if (pd)
		{
			tDataUnit u = {0};
			u.date = from;
			OpsArray& Sales = pd->Sales;
			for (int i = 0; i < Sales.GetCount(); ++i)
			{
				tOperation *p = Sales[i];
				tDate t1 = p->dateCreated;
				tDate t2 = p->dateEffective;
				int diff = p->dateEffective.month - p->dateCreated.month;
				if (diff < 0) diff += 12;
				u.Total += p->sum;
				switch (diff)
				{
					case 0: u.Now += p->sum; break;
					case 1: u.Next += p->sum; break;
					case 2: u._30 += p->sum; break;
					case 3: u._60 += p->sum; break;
					default: 
						u._90 += p->sum; break;
						break;
				}
			}
			if (u.Total)
			{
				table.Add(u);
			}
		}
		from = from.NextMonth();
	}

	FeedItems(table.GetCount());
}

BOOL CSalesTimingReport::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_List.Prepare();

	SetWindowText(_T("Sales: payment distribution"));
	SetDlgItemText(IDC_LEGEND, _T("Distribution shown in percents"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
