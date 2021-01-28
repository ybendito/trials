// ShowSetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "ShowSetDlg.h"
#include "afxdialogex.h"


// CShowSetDlg dialog

IMPLEMENT_DYNAMIC(CShowSetDlg, CDockDialog)

CShowSetDlg::CShowSetDlg(CWnd* pParent, tStockItemSet& _items)
	: CDockDialog(CShowSetDlg::IDD, pParent), items(_items), m_Items(_items, *this)
{
	pEngine = NULL;
	pick1 = pick2 = NULL;
	filterBy = CStockList::cidNothing;
	mode = modeNone;
}

const CDockDialog::tDockTable *CShowSetDlg::GetDockedControlsTable()
{
	static const tDockTable table[] = {
	{IDC_LIST_ITEMS, DOCKF_YSIZE | DOCKF_XSIZE},
	{IDC_ITEMS_LABEL, DOCKF_YPOS},
	{IDC_NOF_ITEMS, DOCKF_YPOS},
	{IDC_TOTAL_LABEL, DOCKF_YPOS},
	{IDC_TOTAL_COST, DOCKF_YPOS},
	{IDC_FOR_DATE, DOCKF_YPOS},
	{IDC_CHECK_SHOW_EMPTY, DOCKF_YPOS | DOCKF_XPOS},
	{IDC_FILTER, DOCKF_YPOS | DOCKF_XPOS},
	{IDC_COMBO_FILTER, DOCKF_YPOS | DOCKF_XPOS},
	{IDC_DATETIMEPICKER1, DOCKF_YPOS},
	{IDC_APPLY, DOCKF_YPOS | DOCKF_XPOS},
	{0, 0},
	};
	return table;
}


CShowSetDlg::~CShowSetDlg()
{
}

void CShowSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDockDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ITEMS, m_Items);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_Date);
	DDX_Control(pDX, IDC_COMBO_FILTER, m_ComboFilter);
}


BEGIN_MESSAGE_MAP(CShowSetDlg, CDockDialog)
	ON_BN_CLICKED(IDC_FOR_DATE, &CShowSetDlg::OnBnClickedForDate)
	ON_BN_CLICKED(IDC_APPLY, &CShowSetDlg::OnBnClickedApply)
	ON_BN_CLICKED(IDC_PICK1, &CShowSetDlg::OnBnClickedPick1)
	ON_BN_CLICKED(IDC_PICK2, &CShowSetDlg::OnBnClickedPick2)
	ON_BN_CLICKED(IDC_TEST, &CShowSetDlg::OnBnClickedTest)
	ON_EN_CHANGE(IDC_FILTER, &CShowSetDlg::OnEnChangeFilter)
	ON_CBN_SELCHANGE(IDC_COMBO_FILTER, &CShowSetDlg::OnCbnSelchangeComboFilter)
	ON_BN_CLICKED(IDC_CHECK_SHOW_EMPTY, &CShowSetDlg::OnBnClickedCheckShowEmpty)
END_MESSAGE_MAP()


// CShowSetDlg message handlers

void CShowSetDlg::Update()
{
	//m_Items.SetVisibleColumns(STCOLUMN() | STCOLUMN())

	m_Items.SetFilter(filter, filterBy, !!IsDlgButtonChecked(IDC_CHECK_SHOW_EMPTY));
	
	m_Items.FeedItems();

	SetDlgItemInt(IDC_NOF_ITEMS, items.QueryQuantity(_T("")));
	
	SetDlgItemInt(IDC_TOTAL_COST, items.TotalCost());
}



BOOL CShowSetDlg::OnInitDialog()
{
	CDockDialog::OnInitDialog();

	GetDlgItem(IDC_APPLY)->EnableWindow(false);
	switch (mode)
	{
		case modeViewByDate:
			GetDlgItem(IDC_FOR_DATE)->EnableWindow(pEngine != NULL);
			break;
		case modeViewFile:
			GetDlgItem(IDC_APPLY)->EnableWindow(true);
			GetDlgItem(IDC_FOR_DATE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DATETIMEPICKER1)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CHECK_SHOW_EMPTY)->ShowWindow(SW_HIDE);
			break;
		default:
			GetDlgItem(IDC_FOR_DATE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DATETIMEPICKER1)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CHECK_SHOW_EMPTY)->ShowWindow(SW_HIDE);
			break;
	}

	m_ComboFilter.SetCurSel(0);
	m_Items.Prepare();

	Update();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CShowSetDlg::OnBnClickedForDate()
{
	bool b = !!IsDlgButtonChecked(IDC_FOR_DATE);
	m_Date.EnableWindow(b);
	if (mode == modeViewByDate)
		GetDlgItem(IDC_APPLY)->EnableWindow(b);
}


void CShowSetDlg::OnBnClickedApply()
{
	switch (mode)
	{
		case modeViewByDate:
			if (pEngine)
			{
				CTime t;
				m_Date.GetTime(t);
				tDate d(t);
				pEngine->StockRecalculate(items, d);
				Update();
			}
			break;
		case modeViewFile:
			EndDialog(IDOK);
			break;
		default:
			// no common action for all
			break;
	}
}

void CShowSetDlg::OnBnClickedPick1()
{
	if (!pick1)
		pick1 = (tStockItem *)m_Items.GetSelectedItemData();
	else
		pick1 = NULL;
	CheckDlgButton(IDC_PICK1, pick1 != NULL);
	SetDlgItemText(IDC_PICK1, pick1 ? pick1->name : _T("Pick item"));
}


void CShowSetDlg::OnBnClickedPick2()
{
	if (!pick2)
		pick2 = (tStockItem *)m_Items.GetSelectedItemData();
	else
		pick2 = NULL;
	CheckDlgButton(IDC_PICK2, pick2 != NULL);
	SetDlgItemText(IDC_PICK2, pick2 ? pick2->name : _T("Pick item"));
}

void CShowSetDlg::OnBnClickedTest()
{
	if (pick1 && pick2 && pick1 != pick2)
	{
		tStockItem *p = new tStockItem(*pick2);
		if (!pick1->Absorb(p))
		{
			delete p;
		}
		else
		{
			tStockItem i;
			*pick2 = i;
		}
		Update();
	}
}


void CShowSetDlg::OnEnChangeFilter()
{
	GetDlgItemText(IDC_FILTER, filter);
	filter.MakeLower();
	if (filterBy != CStockList::cidNothing)
		Update();
}


void CShowSetDlg::OnCbnSelchangeComboFilter()
{
	static const CStockList::ColumnId filters[] = 
	{
		CStockList::cidNothing,
		CStockList::cidName,
		CStockList::cidSource,
		CStockList::cidClass,
	};
	int n = m_ComboFilter.GetCurSel();
	filterBy = n < 0 ? CStockList::cidNothing : filters[n];
	Update();
}


void CShowSetDlg::OnBnClickedCheckShowEmpty()
{
	Update();
}
