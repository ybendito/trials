// ReminderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "ReminderDlg.h"
#include "afxdialogex.h"


// CReminderDlg dialog

IMPLEMENT_DYNAMIC(CReminderDlg, CDialogEx)

CReminderDlg::CReminderDlg(OpsArray& _reminders, CWnd* pParent /*=NULL*/)
	: CDialogEx(CReminderDlg::IDD, pParent), m_List(*this), reminders(_reminders)
{

}

CReminderDlg::~CReminderDlg()
{
}

void CReminderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_DATE, m_DateBox);
}

void CReminderDlg::OnItemActivate(tOperation *p)
{
	m_DateBox.EnableWindow(p != NULL);
	GetDlgItem(IDC_CHANGE_REMINDER)->EnableWindow(p != NULL);
	GetDlgItem(IDC_STOP)->EnableWindow(p != NULL);
}


BEGIN_MESSAGE_MAP(CReminderDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHANGE_REMINDER, &CReminderDlg::OnBnClickedChangeReminder)
	ON_BN_CLICKED(IDC_STOP, &CReminderDlg::OnBnClickedStop)
END_MESSAGE_MAP()


// CReminderDlg message handlers

void CReminderDlg::ListRefill()
{
	UINT64 visible = OPCOLUMN(cidClient) | OPCOLUMN(cidName) | OPCOLUMN(cidSum) | OPCOLUMN(cidEffDate) | OPCOLUMN(cidDealDate);
	m_List.SetVisibleColumns(visible);
	m_List.SetOps(&reminders);
	m_List.FeedItems();
	OnItemActivate(NULL);
}


BOOL CReminderDlg::OnInitDialog()
{
	__super::OnInitDialog();
	
	SetTimeFormat(m_DateBox);
	m_List.Prepare();
	ListRefill();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CReminderDlg::OnBnClickedChangeReminder()
{
	tOperationReminder *pR = (tOperationReminder *)m_List.GetSelectedItemData();
	
	if (pR)
	{
		CTime t;
		m_DateBox.GetTime(t);
		pR->dateEffective = t;
		pR->pOwner->ReminderTime = pR->dateEffective.ToNumber();
		ListRefill();
	}
}


void CReminderDlg::OnBnClickedStop()
{
	tOperationReminder *pR = (tOperationReminder *)m_List.GetSelectedItemData();
	if (pR && pR->pOwner)
	{
		tOperation *p = pR->pOwner;
		POSITION pos = p->ChildList.GetHeadPosition();
		while (pos)
		{
			if (p->ChildList.GetAt(pos) == pR)
			{
				p->ChildList.RemoveAt(pos);
				p->ReminderTime = 0;
				delete pR;
				ListRefill();
				break;
			}
			p->ChildList.GetNext(pos);
		}
	}
}
