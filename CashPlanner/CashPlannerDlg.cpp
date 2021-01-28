
// CashPlannerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "CashPlannerDlg.h"
#include "afxdialogex.h"
#include "AboutDlg.h"
#include "TextParser.h"
#include "SelectLinesDlg.h"
#include "ShowSetDlg.h"
#include "FileWorks.h"
#include "VatDlg.h"
#include "ReminderDlg.h"
#include "PeriodicOpsDlg.h"
#include "WorkspaceCfgDlg.h"
#include "WorkspacesDlg.h"
#include "WaitDlg.h"
#include "MergeDataDlg.h"
#include "SalesTimingReport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CString OpTypeName(tOperationType t)
{
	switch(t)
	{
		case opNone: return _T("None");
		case opSale: return _T("Sale");
		case opPurchase: return _T("Purchase");
		case opExpense: return _T("Expense");
		case opMoney: return _T("Cash");
		case opStock: return _T("Stock");
	}
	CString s;
	s.Format(_T("Unknown %d"), t);
	return s;
}


// CCashPlannerDlg dialog

CCashPlannerDlg::CCashPlannerDlg(CWnd* pParent /*=NULL*/)
	: CDockDialog(CCashPlannerDlg::IDD, pParent), 
	m_CostBox(this), 
	m_List(*this), 
	m_BigGraph(this),
	m_Graph(*this),
	Reminders(opNone),
	AllData(workSpace)
{
	CWinApp *a = AfxGetApp();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	bModified = false;
	bTestMode = false; //true;
	currentData = NULL;
	currentIndex = -1;
	currentDestination = NULL;
	bAllowAnyDate = true;

	SYSTEMTIME t;
	GetLocalTime(&t);
	tDate date, date1, date2;
	date.month = t.wMonth;
	date.year = t.wYear;
	DateLimits[0] = tDate(1, 10, 2014);
	DateLimits[1] = tDate(31, 12, 2016);
	workSpace.From = DateLimits[0];
	workSpace.To = DateLimits[1];

	currentDayForCash = 1;
	filterBy = COperationsList::cidNothing;
}

const CDockDialog::tDockTable *CCashPlannerDlg::GetDockedControlsTable()
{
	static const tDockTable table[] = {
	{IDC_BUTTON_PREV_MONTH, DOCKF_YPOS},
	{IDC_BUTTON_NEXT_MONTH, DOCKF_YPOS | DOCKF_XPOS},
	{IDC_COMBO_FILTER, DOCKF_YPOS},
	{IDC_EDIT_FILTER, DOCKF_YPOS},
	{IDC_INVENTORY, DOCKF_YPOS},
	{IDC_STOCK_LABEL, DOCKF_YPOS},
	{IDC_CASH_LEVEL, DOCKF_YPOS},
	{IDC_CASH, DOCKF_YPOS},
	{IDC_CASH_GRAPH, DOCKF_YPOS},
	{IDC_TOTAL, DOCKF_XPOS},
	{IDC_TOTAL_LABEL, DOCKF_XPOS},
	{IDC_EDIT_FRAME, DOCKF_XPOS},
	{IDC_CLEAR, DOCKF_XPOS},
	{IDC_COMBO_CLIENT, DOCKF_XPOS},
	{IDC_PARTNER_LABEL, DOCKF_XPOS},
	{IDC_TITLE_LABEL, DOCKF_XPOS},
	{IDC_LABEL_SUM, DOCKF_XPOS},
	{IDC_LABEL_INV, DOCKF_XPOS},
	{IDC_EDIT_INV, DOCKF_XPOS},
	{IDC_VAT, DOCKF_XPOS},
	{IDC_EDIT_SUM, DOCKF_XPOS},
	{IDC_EDIT_NAME, DOCKF_XPOS},
	{IDC_LABEL_PLUS, DOCKF_XPOS},
	{IDC_REMINDER, DOCKF_XPOS},
	{IDC_LABEL_REF_DATE, DOCKF_XPOS},
	{IDC_DATETIMEPICKER1, DOCKF_XPOS},
	{IDC_DATETIMEPICKER2, DOCKF_XPOS},
	{IDC_APPLY, DOCKF_XPOS},
	{IDC_LIST1, DOCKF_XSIZE | DOCKF_YSIZE},
	{IDC_REMINDERS, DOCKF_XSIZE | DOCKF_YPOS},
	{0,0},
	};
	return table;
}

const UINT64 VisibleColumns[opMax] = 
{
	//opSale,
	OPCOLUMN(cidClient) | OPCOLUMN(cidName) | OPCOLUMN(cidSum) | OPCOLUMN(cidProfit) | OPCOLUMN(cidEffDate),
	//opPurchase,
	OPCOLUMN(cidClient) | OPCOLUMN(cidName) | OPCOLUMN(cidSum) | OPCOLUMN(cidEffDate),
	//opExpense,
	OPCOLUMN(cidClient) | OPCOLUMN(cidName) | OPCOLUMN(cidSum) | OPCOLUMN(cidEffDate),
	//opMoney,
	OPCOLUMN(cidClient) | OPCOLUMN(cidName) | OPCOLUMN(cidSum) | OPCOLUMN(cidEffDate),
	//opTaxes,
	0,
	//opStock,
	OPCOLUMN(cidFullName) | OPCOLUMN(cidSum) | OPCOLUMN(cidEffDate),
	//opNone,
	0
};

CCashPlannerDlg::~CCashPlannerDlg()
{
}

void CCashPlannerDlg::SetEditMode(bool bEdit, bool bEnable)
{
	bEditExistingItem = bEdit;
	m_ApplyButton.SetWindowText(!bEdit ? TEXT("Add") : TEXT("Change"));
	m_ApplyButton.EnableWindow(bEnable);
}

void CCashPlannerDlg::ShowHideControl(UINT id, bool bShow)
{
	CWnd *p = GetDlgItem(id);
	if (p) p->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
}

void CCashPlannerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDockDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_APPLY, m_ApplyButton);
	DDX_Control(pDX, IDC_EDIT_NAME, m_NameEditBox);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_DateBoxEffective);
	DDX_Control(pDX, IDC_EDIT_SUM, m_SumEditBox);
	DDX_Control(pDX, IDC_DATETIMEPICKER2, m_DateBoxCreated);
	DDX_Control(pDX, IDC_COMBO_CLIENT, m_ClientBox);
	DDX_Control(pDX, IDC_EDIT_INV, m_CostBox);
	DDX_Control(pDX, IDC_COMBO_FILTER, m_FilterCombo);
	DDX_Control(pDX, IDC_CASH_GRAPH, m_Graph);
	DDX_Control(pDX, IDC_REMINDERS, m_RemindersButton);
}

BEGIN_MESSAGE_MAP(CCashPlannerDlg, CDockDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_COMMAND(ID_FILE_NEXTMONTH, &CCashPlannerDlg::OnFileNextMonth)
	ON_COMMAND(ID_FILE_PREVIOUSMONTH, &CCashPlannerDlg::OnFilePreviousmonth)
	ON_UPDATE_COMMAND_UI(ID_FILE_PREVIOUSMONTH, &CCashPlannerDlg::OnUpdateFilePreviousmonth)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_MONTH, &CCashPlannerDlg::OnBnClickedButtonNextMonth)
	ON_BN_CLICKED(IDC_BUTTON_PREV_MONTH, &CCashPlannerDlg::OnBnClickedButtonPrevMonth)
	ON_BN_CLICKED(IDC_TEST, &CCashPlannerDlg::OnTestClickedButton)
	ON_BN_CLICKED(IDC_RADIO1, &CCashPlannerDlg::OnBnClickedRadioCash)
	ON_BN_CLICKED(IDC_RADIO2, &CCashPlannerDlg::OnBnClickedRadioSale)
	ON_BN_CLICKED(IDC_RADIO3, &CCashPlannerDlg::OnBnClickedRadioPurchase)
	ON_BN_CLICKED(IDC_RADIO4, &CCashPlannerDlg::OnBnClickedRadioExpense)
	ON_BN_CLICKED(IDC_RADIO5, &CCashPlannerDlg::OnBnClickedRadioStock)
	ON_BN_CLICKED(IDC_CLEAR, &CCashPlannerDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_APPLY, &CCashPlannerDlg::OnBnClickedApply)
	ON_COMMAND(ID_SAVEAS, &CCashPlannerDlg::OnSaveAs)
	ON_COMMAND(ID_TOOLS_VATCALCULATOR, &CCashPlannerDlg::OnToolsVatcalculator)
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_FILE_LOADFROM, &CCashPlannerDlg::OnFileLoadfrom)
	ON_STN_CLICKED(IDC_LABEL_INV, &CCashPlannerDlg::OnStnClickedLabelInv)
	ON_COMMAND(ID_ABOUT_ABOUT, &CCashPlannerDlg::OnAbout)
	ON_EN_CHANGE(IDC_EDIT_FILTER, &CCashPlannerDlg::OnEnChangeEditFilter)
	ON_COMMAND(ID_EDIT_POSTPONE, &CCashPlannerDlg::OnEditPostpone)
	ON_COMMAND(ID_IMPORT_SALESFILE, &CCashPlannerDlg::OnImportSalesfile)
	ON_COMMAND(ID_STOCK_LATEST, &CCashPlannerDlg::OnStockLatest)
	ON_CBN_SELCHANGE(IDC_COMBO_FILTER, &CCashPlannerDlg::OnCbnSelchangeComboFilter)
	ON_BN_CLICKED(IDC_CASH_GRAPH, &CCashPlannerDlg::OnClickedCashGraph)
	ON_BN_CLICKED(IDC_REMINDERS, &CCashPlannerDlg::OnBnClickedReminders)
	ON_COMMAND(ID_VIEW_INVENTORYFILE, &CCashPlannerDlg::OnViewInventoryfile)
	ON_COMMAND(ID_TOOLS_PERIODICOPERATIONS, &CCashPlannerDlg::OnToolsPeriodicoperations)
	ON_COMMAND(ID_ANALYSIS_NEW, &CCashPlannerDlg::OnAnalysisNew)
	ON_COMMAND(ID_ANALYSIS_LOAD, &CCashPlannerDlg::OnAnalysisLoad)
	ON_COMMAND(ID_OPERATIONAL_NEW, &CCashPlannerDlg::OnOperationalNew)
	ON_COMMAND(ID_OPERATIONAL_LOAD, &CCashPlannerDlg::OnOperationalLoad)
	ON_COMMAND(ID_FILE_CURRENTWORKSPACE, &CCashPlannerDlg::OnFileCurrentworkspace)
	ON_COMMAND(ID_WORSPACES_MANAGEWORKSPACES, &CCashPlannerDlg::OnWorspacesManageworkspaces)
	ON_COMMAND(ID_SAVEWORKSPACENOW, &CCashPlannerDlg::OnSaveworkspacenow)
	ON_COMMAND(ID_DATA_LOADWORKSPACE, &CCashPlannerDlg::OnDataLoadworkspace)
	ON_COMMAND(ID_REPORTS_SALES1, &CCashPlannerDlg::OnReportsSalesTiming)
	ON_COMMAND(ID_REPORTS_PARTNERS, &CCashPlannerDlg::OnReportsPartners)
	ON_COMMAND(ID_IMPORTWORKSPACE, &CCashPlannerDlg::OnImportworkspace)
	ON_WM_NCACTIVATE()
	ON_COMMAND(ID_EDIT_CLEAR_MONTHLY, &CCashPlannerDlg::OnEditClearMonthly)
END_MESSAGE_MAP()

static bool CheckDateLimits(tDate& date, const tDate& low, const tDate& high)
{
	ULONG fdd,ldd,l,h;
	tDate fd(date), ld(date);
	fd.day = 1;
	ld.day = 31;
	fdd = fd.ToNumber();
	ldd = ld.ToNumber();
	l = low.ToNumber();
	h = high.ToNumber();
	bool b = true;
	if (ldd < l)
	{
		date = low;
		b = false;
	}
	else if (fdd > h)
	{
		date = high;
		b = false;
	}
	return b;
}

void CCashPlannerDlg::onNextPeriodicOperation(tPeriodicOperation *p, bool bParam)
{
	INT index;
	if (bParam)
	{
		tDate date = DateLimits[0];
		while (date.CompareMonth(DateLimits[1]) <= 0)
		{
			tMonthlyData *pd = EnsureExists(date, index, true);
			if (pd)
			{
				tDate temp = date;
				int _sum = p->sum;
				if (CheckDateLimits(temp, p->FromDate, p->ToDate) && p->CheckPeriod(temp, _sum))
				{
					tOperation *pNew = new tOperation(*p);
					if (pNew)
					{
						pNew->sum = _sum;
						pNew->bOriginIsConstant = true;
						pNew->bDerived = true;
						pNew->dateEffective = date;
						pNew->dateEffective.day = p->FromDate.day;
						pNew->dateCreated = pNew->dateEffective;
						p->ChildList.AddTail(pNew);
						AddNewOperation(pNew, pNew->dateCreated);
					}
				}
			}
			date = date.NextMonth();
		}
	}
	else
	{
		while (p->ChildList.GetCount())
		{
			tOperation *pp = p->ChildList.RemoveHead();
			delete pp;
		}
	}
}




// CCashPlannerDlg message handlers

BOOL CCashPlannerDlg::OnInitDialog()
{
	CDockDialog::OnInitDialog();
	initialColor = m_List.GetBkColor();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if (!tWorkspaceConfig::MakeBaseDir())
	{
		AfxMessageBox(_T("MakeBaseDir failed, nothing can work properly!"));
	}

	currentData = EnsureExistsToday(currentIndex);
	
	SetEditMode(false, false);
	
	m_List.Prepare();

	SetTimeFormat(m_DateBoxEffective);
	SetTimeFormat(m_DateBoxCreated);
	
	m_FilterCombo.SetCurSel(0);
	
	RecalculateAll();

	// initialize workspace for sell operation
	CheckDlgButton(IDC_RADIO2, true); 
	OnBnClickedRadioSale();

	UpdatePage();

	UpdateUI();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCashPlannerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDockDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCashPlannerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CCashPlannerDlg::ExecuteLoadFromFile(CString fname, LoadClearCfg& clearCfg, LoadClearCfg& loadCfg)
{
	bool bApply = false, bSkipMergeDlg = false;
	CMergeDataDlg d(clearCfg, loadCfg, this);
	
	AllData.CheckCanClear(clearCfg);
	AllData.CheckCanLoad(loadCfg, fname);

	// this is to allow 'Load existing workspace' without merge dialog
	bSkipMergeDlg = !clearCfg.all.show && !loadCfg.all.show; 
	
	bApply = bSkipMergeDlg || d.DoModal() == IDOK;

	if (bApply)
	{
		ClearData(clearCfg);
	}
	
	if (bApply)
	{
		CWaitDlg w(this);
		tProfileSetup ps(fname);
			
		tArrayOfOperations tempArray;
	
		AllData.Deserialize(tempArray, loadCfg);

		while (!tempArray.IsEmpty())
		{
			tOperation *p = tempArray[0];
			tempArray.RemoveAt(0);
			AddNewOperation(p, p->dateCreated);
		}

		if (loadCfg.periodic.check)
		{
			AllData.PeriodicOps.Deserialize();
			AllData.PeriodicOps.Enumerate(*this, true);
		}

		RecalculateAll();
		UpdatePage();

		if (loadCfg.workspace.check)
		{
			workSpace.Deserialize();
		}
		else
		{
			ProcessChange();
		}
	}

	return bApply;
}

static bool IsEmpty(OpsArray& ops)
{
	INT i;
	bool bEmpty = true;
	for (i = 0; bEmpty && i < ops.GetCount(); ++i)
	{
		bEmpty = bEmpty && ops[i]->bDerived;
	}
	return bEmpty;
}

static bool IsEmpty(CArray<tMonthlyData *>& a)
{
	INT i;
	bool bEmpty = true;
	for (i = 0; bEmpty && i < a.GetCount(); ++i)
	{
		bEmpty = bEmpty && IsEmpty(a[i]->Expenses);
		bEmpty = bEmpty && IsEmpty(a[i]->Purchases);
		bEmpty = bEmpty && IsEmpty(a[i]->Money);
		bEmpty = bEmpty && IsEmpty(a[i]->Sales);
	}
	return bEmpty;
}

static void ClearNonDerived(OpsArray& ops)
{
	INT i;
	for (i = 0; i < ops.GetCount(); ++i)
	{
		if (!ops[i]->bDerived)
		{
			delete ops[i];
			i = -1;
		}
	}
}

void CCashPlannerDlg::ClearData(LoadClearCfg& clearCfg)
{
	INT i;
	for (i = 0; i < AllData.GetCount(); ++i)
	{
		if (clearCfg.expenses.check) ClearNonDerived(AllData[i]->Expenses);
		if (clearCfg.cash.check) ClearNonDerived(AllData[i]->Money);
		if (clearCfg.purchases.check) ClearNonDerived(AllData[i]->Purchases);
		if (clearCfg.sales.check) ClearNonDerived(AllData[i]->Sales);
	}

	if (clearCfg.periodic.check)
	{
		AllData.PeriodicOps.FreeAll();
	}
	if (clearCfg.periodic.check || 
		clearCfg.expenses.check || 
		clearCfg.cash.check || 
		clearCfg.purchases.check ||
		clearCfg.sales.check)
	{
		UpdatePage();
	}
}


bool CCashPlannerDlg::VerifySupportedForOperational()
{
	if (workSpace.fOperational)
	{
		AfxMessageBox(_T("Not supported for operational workspace"));
	}
	return !workSpace.fOperational;
}


void CCashPlannerDlg::OnDropFiles(HDROP hDropInfo)
{
	LoadClearCfg clearCfg, loadCfg;
	loadCfg.workspace.check = loadCfg.workspace.show = false;
	TCHAR name[MAX_PATH];
	// TODO: Add your message handler code here and/or call default
	if (DragQueryFile(hDropInfo, 0, name, ELEMENTS_IN(name)))
	{
		if (VerifySupportedForOperational())
		{
			ExecuteLoadFromFile(name, clearCfg, loadCfg);
		}
	}

	CDockDialog::OnDropFiles(hDropInfo);
}

static OpsArray *GetDestinationByType(tMonthlyData *pd, tOperationType type)
{
	switch(type)
	{
		case opMoney:
			return &pd->Money;
		case opSale:
			return &pd->Sales;
		case opExpense:
			return &pd->Expenses;
		case opPurchase:
			return &pd->Purchases;
		case opStock:
			return &pd->Stock;
		default:
			return NULL;
	}
}


void CCashPlannerDlg::FillListView(tOperationType opType, bool bClearEdit)
{
	int i;
	currentDayForCash = 1;
	int totalSum = 0, totalSumPlusVAT = 0;

	if (currentData)
	{
		currentDestination = GetDestinationByType(currentData, opType);
		m_List.SetOps(currentDestination);

		m_List.SetFilter(currentFilter, filterBy);

		m_List.SetVisibleColumns(VisibleColumns[opType]);
		
		m_List.FeedItems();
	}

	
	for (i = 0; currentDestination && i < currentDestination->GetCount(); ++i)
	{
		tOperation *op = currentDestination->GetAt(i);
		if (m_List.PassesFilter(op))
		{
			totalSum += op->sum;
			totalSumPlusVAT += op->sum;
			if (op->bTaxable)
			{
				totalSumPlusVAT += VAT(op->sum);
			}
		}
	}

	SetEditMode(false, currentDestination != NULL);
	if (bClearEdit)
	{
		OnBnClickedClear();	
	}
	if (currentDestination)
	{
		CString s;
		s.Format(_T("Start: %d, Low: %d, EOM: %d"), currentData->StartCash, currentData->LowCash, currentData->EndCash);
		SetDlgItemText(IDC_CASH_LEVEL, s);
		UpdateDailyCash();

		s.Format(_T("%d"), totalSum);
		if (totalSumPlusVAT != totalSum && totalSumPlusVAT)
			s.AppendFormat(_T(" (%d)"), totalSumPlusVAT);
		SetDlgItemText(IDC_TOTAL, s);
		s.Format(_T("%d\n%d"), currentData->LowStock, currentData->HighStock);
		SetDlgItemText(IDC_INVENTORY, s);
	}
	UpdateUI();
}


void CCashPlannerDlg::UpdateDailyCash()
{
	CString s;
	tDate d = currentData->month;
	d.day = currentDayForCash;
	s = _T("On ");
	s += d.GetString(1);
	s.AppendFormat(_T(":  NIS %+d"), currentData->CashPerDay[currentDayForCash]);
	SetDlgItemText(IDC_CASH, s);
	m_Graph.PushData(&currentData->CashPerDay, currentDayForCash);
	m_BigGraph.PushData(&currentData->CashPerDay, currentDayForCash);
}

void CCashPlannerDlg::UpdatePage()
{
	if (currentDestination)
	{
		FillListView(currentDestination->type, true);
	}
	ConfigureForWorkspace();
}

tMonthlyData *CCashPlannerDlg::EnsureExists(const tDate& date, INT& index, bool bForce)
{
	tDate tempDate = date;
	tMonthlyData *p = NULL;
	for (INT i = 0; !p && i < AllData.GetCount(); ++i)
	{
		if (!AllData[i]->month.CompareMonth(date))
		{
			index = i;
			p = AllData[i];
		}
	}
	
	if (!p)
	{
		p = new tMonthlyData;
		p->month = date;
		p->month.day = 0;
		index = AllData.Add(p);
	}

	if (!CheckDateLimits(tempDate, DateLimits[0], DateLimits[1]))
	{
		if (bForce)
		{
			if (!bAllowAnyDate)
				MessageBox(_T("The target date is out of limits!"));
		}
		else
		{
			p = EnsureExists(tempDate, index, false);
		}
	}

	return p;
}

tMonthlyData *CCashPlannerDlg::EnsureExistsToday(INT& index)
{
	SYSTEMTIME t;
	GetLocalTime(&t);
	tDate date;
	date.month = t.wMonth;
	date.year = t.wYear;
	return EnsureExists(date, currentIndex, true);
}


void CCashPlannerDlg::OnFileNextMonth()
{
	if (!currentData)
	{
		currentData = EnsureExistsToday(currentIndex);
	}
	else
	{
		tDate date(currentData->month.month + 1, currentData->month.year);
		EnsureExists(date, currentIndex, false);
		currentData = AllData[currentIndex];
	}
	UpdatePage();
}

void CCashPlannerDlg::OnFilePreviousmonth()
{
	if (!currentData)
	{
		currentData = EnsureExistsToday(currentIndex);
	}
	else
	{
		tDate date(currentData->month.month - 1, currentData->month.year);
		EnsureExists(date, currentIndex, false);
		currentData = AllData[currentIndex];
	}
	UpdatePage();
}


void CCashPlannerDlg::OnUpdateFilePreviousmonth(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}


void CCashPlannerDlg::OnBnClickedButtonNextMonth()
{
	OnFileNextMonth();
}


void CCashPlannerDlg::OnBnClickedButtonPrevMonth()
{
	OnFilePreviousmonth();
}


void CCashPlannerDlg::OnTestClickedButton()
{

}

void CCashPlannerDlg::UpdateOperationEditControls(tOperation *p)
{
	tOperation temp;
	if (!p)
	{
		if (currentDestination) temp.type = currentDestination->type;
		p = &temp;
	}
	CTime time;
	INT i;
		
	SetDlgItemText(IDC_EDIT_FRAME, OpTypeName(p->type));
		
	// Taxable
	bool bTaxable = p->bTaxable;
	bTaxable = bTaxable || 
		(p->sum == 0 
		&& (p->type == opPurchase || p->type == opSale));
	CheckDlgButton(IDC_VAT, bTaxable);
	GetDlgItem(IDC_VAT)->EnableWindow(p->type != opMoney && p->type != opStock);

	// sum
	if (p->sum)
		SetDlgItemInt(IDC_EDIT_SUM, p->sum, true);
	else
		m_SumEditBox.SetWindowText(_T(""));
	m_SumEditBox.EnableWindow(!p->bDerived && p->type != opPurchase);

	
	// Cost
	m_CostBox.Reset(p);

	// ref data - box
	tDate date = p->dateCreated;
	date.IfNotValid(currentData->month, currentData->month.IsCurrent() ? CurrentTime().GetDay() : 15);
	time = date.ToCTime();
	m_DateBoxCreated.SetTime(&time);
	if (p->type == opNone)
	{
		CTime timemin = date.ToCTime(1), timemax = date.ToCTime(31);
		m_DateBoxCreated.SetRange(&timemin, &timemax);
	}
	
	// effective date for new cash operation - disabled and set to deal date
	if (p->type != opMoney || p->sum)
	{
		date = p->dateEffective;
		date.IfNotValid(currentData->month.NextMonth(), 1);
		time = date.ToCTime();
	}
	m_DateBoxEffective.EnableWindow(p->type != opMoney);
	m_DateBoxEffective.SetTime(&time);

	// name
	m_NameEditBox.SetWindowText(p->name);

	//client
	m_ClientBox.ResetContent();
	for (i = 0; i < Clients[p->type].GetCount(); ++i)
		m_ClientBox.AddString(Clients[p->type].GetAt(i));
	m_ClientBox.SetWindowText(p->ClientName);

	// reminder
	CheckDlgButton(IDC_REMINDER, p->ReminderTime);
	GetDlgItem(IDC_REMINDER)->EnableWindow(p->type == opSale);

	CheckReminders();
}

tOperation *CCashPlannerDlg::FindSelectedOperation()
{
	return (tOperation *)m_List.GetSelectedItemData();
}


void CCashPlannerDlg::OnBnClickedRadioCash()
{
	if (!bTestMode) FillListView(opMoney, true); 
}


void CCashPlannerDlg::OnBnClickedRadioSale()
{
	if (!bTestMode) FillListView(opSale, true); 
}


void CCashPlannerDlg::OnBnClickedRadioPurchase()
{
	if (!bTestMode) FillListView(opPurchase, true); 
}


void CCashPlannerDlg::OnBnClickedRadioExpense()
{
	if (!bTestMode) FillListView(opExpense, true); 
}


void CCashPlannerDlg::OnBnClickedRadioStock()
{
	if (!bTestMode) FillListView(opStock, true); 
}


void CCashPlannerDlg::OnBnClickedClear()
{
	UpdateOperationEditControls(NULL);
	SetEditMode(false, currentDestination != NULL);
}

int CCashPlannerDlg::GetSumFromBox()
{
	int val = 0;
	CDataExchange dx(this, true);
	DDX_Text(&dx, IDC_EDIT_SUM, val);
	return val;
}

void CCashPlannerDlg::AddEntryByDate(tOperation *p, const tDate& date)
{
	INT i;
	EnsureExists(date, i, true);
	tMonthlyData *pd = AllData[i];
	OpsArray *pa = GetDestinationByType(pd, p->type);
	pa->Add(p);
}

static void DoTaxation(tOperation *p, bool bVAT)
{
	ULONG month = p->dateCreated.month + 1;
	if (~month & 1) ++month;
	p->dateEffective = tDate(month, p->dateCreated.year);
	p->dateEffective.day = 15;
	switch(p->type)
	{
		case opSale:
			if (bVAT)			
				p->sum = - VAT(p->sum);
			else
				p->sum = - TAX(p->sum);
			break;
		case opPurchase:
		case opExpense:
			if (bVAT)
				p->sum = VAT(p->sum);
			break;
		default:
			p->sum = 0;
			break;
	}
	p->ClientName.Empty();
	if (bVAT)
		p->name.Format(_T("VAT.%s"), (LPCTSTR)p->name);
	else
		p->name.Format(_T("Tax.%s"), (LPCTSTR)p->name);
	p->type = opMoney;
	p->bDerived = true;
	p->bTaxable = false;

}

void CCashPlannerDlg::ProcessCost(tOperation *p)
{
	tOperation *pChild = new tOperation(*p);
	pChild->bTaxable = false;
	pChild->bDerived = true;
	pChild->items.Copy(p->items);
	pChild->items.multiplier = p->type == opSale ? -1 : 1;
	pChild->sum = pChild->items.TotalCost();
	pChild->type = opStock;
	pChild->dateEffective = p->dateCreated;
	p->ChildList.AddTail(pChild);
	AddEntryByDate(pChild, pChild->dateEffective);
}

void CCashPlannerDlg::ProcessMoney(tOperation *p)
{
	tOperation *pChild = new tOperation(*p);
	pChild->bDerived = true;
	pChild->type = opMoney;
	if (p->bTaxable) pChild->sum = pChild->sum + VAT(pChild->sum);
	if (p->type != opSale)	
	{
		pChild->sum = -pChild->sum;
	}
	pChild->bTaxable = false;
	p->ChildList.AddTail(pChild);
	AddEntryByDate(pChild, pChild->dateEffective);
}

void CCashPlannerDlg::AddNewOperation(tOperation *p, const tDate& date)
{
	if (p->sum)
	{
		AddEntryByDate(p, date);
		if (p->type != opMoney)
		{
			ProcessMoney(p);
		}
		// Add VAT
		if (p->bTaxable)
		{
			tOperation *pChild = new tOperation(*p);
			DoTaxation(pChild, true);
			if (pChild->sum)
			{
				p->ChildList.AddTail(pChild);
				AddEntryByDate(pChild, pChild->dateEffective);
			}
			else
			{
				delete pChild;
			}
		}
		if (p->type == opSale)
		{
			tOperation *pChild = new tOperation(*p);
			DoTaxation(pChild, false);
			if (pChild->sum)
			{
				p->ChildList.AddTail(pChild);
				AddEntryByDate(pChild, pChild->dateEffective);
			}
			else
			{
				delete pChild;
			}
		}
		if (p->items.TotalCost())
		{
			ProcessCost(p);
		}
		
		if (p->ReminderTime)
		{
			tOperationReminder *pRem = new tOperationReminder(p);
			p->ChildList.AddTail(pRem);
			Reminders.Add(pRem);
		}
	}
	else if (!p->bDerived)
	{
		delete p;
	}
}

bool CCashPlannerDlg::ValidateInfoFromControl()
{
	bool b = true;
	int sum = GetSumFromBox();
	int cost = m_CostBox.items.TotalCost();
	if (sign(sum) * sign(cost) < 0)
	{
		b = IDYES == AfxMessageBox(TEXT("Sum and cost both should be both positive or negative.\nAre you sure your data are correct?"), MB_YESNO | MB_DEFBUTTON2);
	}
	if (b && currentDestination->type == opStock)
	{
		AfxMessageBox(TEXT("Stock is updated via sales or purchases.\nAdding stock directly is not supported"), MB_OK);
		b = false;
	}
	if (b)
	{
		CTime tCreated, tEffective;
		m_DateBoxCreated.GetTime(tCreated);
		m_DateBoxEffective.GetTime(tEffective);
		if (tEffective < tCreated && currentDestination->type != opMoney)
		{
			b = IDYES == AfxMessageBox(TEXT("Date of payment is earlier than date of deal.\nAre you sure your data are correct?"), MB_YESNO | MB_DEFBUTTON2);
		}
	}
	return b;
}


void CCashPlannerDlg::OnBnClickedApply()
{
	if (currentDestination->type == opPurchase)
	{
		SetDlgItemInt(IDC_EDIT_SUM, m_CostBox.items.TotalCost());
	}

	if (!ValidateInfoFromControl()) return;

	tOperation *p = NULL;
	if (bEditExistingItem)
	{
		p = FindSelectedOperation();
		if (p) delete p;
		p = NULL;
	}
	
	p = new tOperation;
	if (p)
	{
		CString s;
		CTime time;
		p->type = currentDestination->type;

		p->bTaxable = !!IsDlgButtonChecked(IDC_VAT);
		
		m_DateBoxEffective.GetTime(time);
		p->dateEffective = tDate(time);
		
		m_DateBoxCreated.GetTime(time);
		p->dateCreated = time;

		if (p->type == opMoney)
		{
			p->dateEffective = p->dateCreated;
		}

		if (IsDlgButtonChecked(IDC_REMINDER))
		{
			p->ReminderTime = p->dateEffective.ToNumber();
		}

		m_NameEditBox.GetWindowText(p->name);
		m_ClientBox.GetWindowText(p->ClientName);
		p->items.Copy(m_CostBox.items);
		p->sum = GetSumFromBox();
		AddNewOperation(p, p->dateCreated);
	}
	RecalculateAll();
	FillListView(currentDestination->type, true);
	ProcessChange();

}

void CCashPlannerDlg::ProcessChange()
{
	bModified = true;
	if (workSpace.bAutosave && !workSpace.name.IsEmpty())
	{
		DoSaveToFile(workSpace.Filename());
		bModified = false;
	}
	ConfigureForWorkspace();
}

void CCashPlannerDlg::OnItemActivate(tOperation *p)
{
	if (p)
	{
		UpdateOperationEditControls(p);
		SetEditMode(true, !p->bDerived);
	}
	else if (bEditExistingItem)
	{
		UpdateOperationEditControls(p);
		SetEditMode(false, true);
	}
	UpdateUI();
}

static void ProcessOps(int& level, OpsArray& ops, const tDate& date)
{
	INT i;
	for (i = 0; i < ops.GetCount(); ++i)
	{
		tOperation *p = ops[i];
		if (p->dateEffective.day == date.day && 
			p->dateEffective.month == date.month && 
			p->dateEffective.year == date.year)
		{
			int sum = p->sum;
			level += sum;
		}
	}
}

static void ProcessStockOps(tStockItemSet& stock, OpsArray& ops, const tDate& date)
{
	INT i;
	for (i = 0; i < ops.GetCount(); ++i)
	{
		tOperation *p = ops[i];
		if (p->dateEffective.day == date.day && 
			p->dateEffective.month == date.month && 
			p->dateEffective.year == date.year)
		{
			stock.Copy(p->items, true);
		}
	}
}


static void ProcessNames(const OpsArray& ops, CStringArray& a)
{
	INT i, j;
	for (i = 0; i < ops.GetCount(); ++i)
	{
		CString& name = ops[i]->ClientName;
		if (!name.IsEmpty())
		{
			bool bFound = false;
			for (j = 0; !bFound && j < a.GetCount(); ++j)
			{
				bFound = !name.CompareNoCase(a[j]);
			}
			if (!bFound) a.Add(name);
		}
	}	
}

void CCashPlannerDlg::RecalculateAll(tRecalculationTask *pTask)
{
	tRecalculationTask defaultTask;
	if (!pTask) pTask = &defaultTask;
	
	tDate date = DateLimits[0];
	tDate lastDate = DateLimits[1]; 
	
	int currentLevel = 0;
	bool bExternalLimit = false;
	INT i;
	tStockItemSet CurrentStock;
	
	if (pTask->bUpdateNames)
	{
		for (i = 0; i < ELEMENTS_IN(Clients); ++i) Clients[i].RemoveAll();
		stockClasses.RemoveAll();
		stockModels.RemoveAll();
	}
	
	while (date.CompareMonth(lastDate) <= 0)
	{
		INT index;
		tMonthlyData *pd = EnsureExists(date, index, true);
		if (pd)
		{
			ULONG day;
			if (pTask->bUpdateMontlyData)
			{
				pd->LowCash = INT_MAX;
				pd->StartCash = currentLevel;
				pd->HighStock = INT_MIN;
				pd->LowStock = INT_MAX;
				pd->CashChangeMask = 1 << 31;
				memset(pd->CashPerDay, 0, sizeof(pd->CashPerDay));
			}
			
			for (day = 0; day < ELEMENTS_IN(pd->CashPerDay); ++day)
			{
				int prev = currentLevel;
				date.day = day;
				if (pTask->bUpdateMontlyData)
				{
					ProcessOps(currentLevel, pd->Money, date);
				}
				if (pTask->bUpdateMontlyData || pTask->pCopyStockTo)
				{
					ProcessStockOps(CurrentStock, pd->Stock, date);
				}
				if (pTask->bUpdateMontlyData)
				{
					if (currentLevel < pd->LowCash)
						pd->LowCash = currentLevel;
					pd->CashPerDay[day] = currentLevel;
					if (currentLevel != prev)
					{
						pd->CashChangeMask |= 1 << day;
					}
					int costOfStock = CurrentStock.TotalCost();
					if (costOfStock < pd->LowStock)
						pd->LowStock = costOfStock;
					if (costOfStock > pd->HighStock)
						pd->HighStock = costOfStock;
				}
				
				if (pTask->forceLastDay.IsValid() && !date.Compare(pTask->forceLastDay))
				{
					bExternalLimit = true;
					break;
				}
			}

			if (pTask->bUpdateMontlyData)
			{
				pd->EndCash = currentLevel;
			}
			if (pTask->bUpdateNames)
			{
				ProcessNames(pd->Sales, Clients[opSale]);
				ProcessNames(pd->Purchases, Clients[opPurchase]);
				ProcessNames(pd->Expenses, Clients[opExpense]);
				ProcessModels(pd->Purchases);
			}
		}
		date = date.NextMonth();
		if (bExternalLimit) break;
	}
	if (pTask->pCopyStockTo)
	{
		pTask->pCopyStockTo->Copy(CurrentStock);
	}
}

void CCashPlannerDlg::ProcessModels(const OpsArray& ops)
{
	INT i;
	for (i = 0; i < ops.GetCount(); ++i)
	{
		INT j;
		const tOperation *p = ops[i];
		const CArray<tStockItem *>& items = p->items.items;
		for (j = 0; j < items.GetCount(); ++j)
		{
			const tStockItem *pi = items[j];
			if (!pi->name.IsEmpty() && !pi->Class.IsEmpty())
			{
				CString sName = pi->name;
				sName.MakeUpper();
				int index = -1;
				if (!stockModels.Lookup(sName, index))
				{
					index = stockClasses.Add(pi->Class);
					stockModels.SetAt(sName, index);
				}
			}
		}
	}
}

void CCashPlannerDlg::DoSaveToFile(CString sProfile)
{
	CWinApp *a = AfxGetApp();
	LPCTSTR keep = a->m_pszProfileName;
	a->m_pszProfileName = sProfile;

	AllData.Serialize(sProfile);

	a->m_pszProfileName = keep;
}

void CCashPlannerDlg::OnSaveAs()
{
	CFileDialog fd(false, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (fd.DoModal() == IDOK)
	{
		CString sProfile = fd.GetPathName();
		DoSaveToFile(sProfile);
	}
}

void CCashPlannerDlg::OnToolsVatcalculator()
{
	CString s;
	int val = 0;
	CWnd *pWnd = GetFocus();
	if (pWnd) pWnd->GetWindowText(s);
	StringToNumber(s, val);
	if (!val) val = 1000;
	ULONG uval = val;
	CVatDlg dlg(this, uval);
	dlg.DoModal();
}

void CCashPlannerDlg::ChangeCashDay(bool bIncrement)
{
	if (currentData)
	{
		if (bIncrement)
		{
			while (currentDayForCash < ELEMENTS_IN(currentData->CashPerDay))
			{
				currentDayForCash++;
				if ((1 << currentDayForCash) & currentData->CashChangeMask) break;
			}
		}
		else
		{
			while (currentDayForCash > 0)
			{
				currentDayForCash--;
				if ((1 << currentDayForCash) & currentData->CashChangeMask) break;
			}
		}
		if (currentDayForCash >= ELEMENTS_IN(currentData->CashPerDay))
		{
			currentDayForCash = 1;
		}
		if (!currentDayForCash)
		{
			currentDayForCash = ELEMENTS_IN(currentData->CashPerDay) - 1;
		}
		UpdateDailyCash();
	}
}


HBRUSH CCashPlannerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDockDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_CASH_LEVEL && currentData)
	{
		if (currentData->LowCash < 0)
		{
			pDC->SetTextColor(RGB(255,0,0));
		}
		else
		{
			pDC->SetTextColor(RGB(0,0,255));
		}
	}
	if (pWnd->GetDlgCtrlID() == IDC_CASH)
	{
		int val = currentData->CashPerDay[currentDayForCash];
		if (val < 0)
		{
			pDC->SetTextColor(RGB(255,0,0));
		}
		else
		{
			pDC->SetTextColor(RGB(0,0,255));
		}
	}
	return hbr;
}


void CCashPlannerDlg::OnFileLoadfrom()
{
	LoadClearCfg clearCfg, loadCfg;
	loadCfg.workspace.check = loadCfg.workspace.show = false;
	if (VerifySupportedForOperational())
	{
		CFileDialog fd(true, NULL, NULL, OFN_FILEMUSTEXIST, NULL, this);
		if (fd.DoModal() == IDOK)
		{
			ExecuteLoadFromFile(fd.GetPathName(), clearCfg, loadCfg);
		}
	}
}


void CCashPlannerDlg::OnStnClickedLabelInv()
{
	AfxMessageBox(_T(__FUNCTION__));	
}

void CCashPlannerDlg::OnAbout()
{
	CAboutDlg dlg(this);
	dlg.DoModal();
}

void CCashPlannerDlg::OnEnChangeEditFilter()
{
	GetDlgItemText(IDC_EDIT_FILTER, currentFilter);
	if (filterBy != COperationsList::cidNothing)
	{
		UpdatePage();
	}
}

void CCashPlannerDlg::EnableMenuItem(UINT id, bool bEnable)
{
	CMenu *pMenu = GetMenu();
	if (pMenu)
	{
		UINT nEnable = bEnable ? MF_ENABLED : MF_GRAYED;
		nEnable |= MF_BYCOMMAND;
		pMenu->EnableMenuItem(id, nEnable);
	}
}

void CCashPlannerDlg::UpdateUI()
{
	tOperation *pOp = FindSelectedOperation();
	EnableMenuItem(ID_EDIT_POSTPONE, pOp && pOp->bDerived && pOp->bOriginIsConstant);
	UpdateDialogControls(this, false);
}


void CCashPlannerDlg::OnEditPostpone()
{
}


void CCashPlannerDlg::OnCancel()
{
	bool bEmpty = IsEmpty(AllData) && AllData.PeriodicOps.IsEmpty();
	if (bEmpty || workSpace.bAutosave || !bModified ||
		IDYES == AfxMessageBox(_T("Close the application and discard the data?"), MB_YESNO | MB_DEFBUTTON2))
	{
		CDockDialog::OnCancel();
	}
}

void CCashPlannerDlg::OnImportSalesfile()
{
	CSelectLinesDlg d(true, this);
	if (d.DoModal() == IDOK)
	{
		LoadClearCfg clearCfg, loadCfg;
		CMergeDataDlg mergeDlg(clearCfg, loadCfg, this);
		tDate t = d.m_Date; 
		
		clearCfg.SetAll(false, false);
		clearCfg.all.show = clearCfg.sales.show = clearCfg.sales.check = true;
		AllData.CheckCanClear(clearCfg);

		loadCfg.SetAll(false, false);
		loadCfg.all.show = loadCfg.sales.show = loadCfg.sales.check = true;

		if (mergeDlg.DoModal() == IDOK && loadCfg.sales.check)
		{
			ClearData(clearCfg);

			while (d.ops.GetCount())
			{
				tOperation *p = d.ops[0];
				if (d.m_UnnamedBeforeDate && p->dateCreated.Compare(t) < 0)
				{
					tStockItem *pi = new tUnnamedStockItem(p->items.TotalCost());
					p->items.FreeAll();
					p->items.Include(pi);
				}
				if (d.m_NoReminders)
				{
					p->ReminderTime = 0;
				}
				AddNewOperation(p, p->dateCreated);
				d.ops.RemoveAt(0);
			}
			RecalculateAll();
			UpdatePage();
			ProcessChange();
		}
	}
}

void CCashPlannerDlg::EditItems(tStockItemSet& items, bool bAddToStock)
{

}

void CCashPlannerDlg::StockRecalculate(tStockItemSet& _items, const tDate& d)
{
	tRecalculationTask task;
	task.bUpdateMontlyData = false;
	task.bUpdateNames = false;
	task.forceLastDay = d;
	task.pCopyStockTo = &_items;
	_items.FreeAll();
	RecalculateAll(&task);
}

void CCashPlannerDlg::OnStockLatest()
{
	tStockItemSet temporary;
	tDate dateUnlimited;
	StockRecalculate(temporary, dateUnlimited);
	CShowSetDlg d(this, temporary);
	d.SetMode(this, CShowSetDlg::modeViewByDate);
	d.DoModal();
}

void CCashPlannerDlg::ResizeDone(POINT change)
{
	if (change.x) m_List.ResizeColumns();
}


void CCashPlannerDlg::OnCbnSelchangeComboFilter()
{
	static const COperationsList::ColumnId filters[] = 
	{
		COperationsList::cidNothing,
		COperationsList::cidFullName,
		COperationsList::cidEffDate,
		COperationsList::cidDealDate,
		COperationsList::cidSum
	};
	int n = m_FilterCombo.GetCurSel();
	filterBy = n < 0 ? COperationsList::cidNothing : filters[n];
	UpdatePage();
}


void CCashPlannerDlg::OnClickedCashGraph()
{
	ChangeCashDay(true);
}

void CCashPlannerDlg::OnRightClick(UINT ctrlId)
{
	switch (ctrlId)
	{
		/*
	case IDC_CASH_GRAPH:
			m_BigGraph.Toggle();
			m_BigGraph.PushData(&currentData->CashPerDay, currentDayForCash);
			break;
			*/
	case IDC_CASH_GRAPH:
		ChangeCashDay(false);
		break;
	}
}


void CCashPlannerDlg::OnBnClickedReminders()
{
	CReminderDlg d(Reminders, this);
	d.DoModal();
	CheckReminders();
}

void CCashPlannerDlg::CheckReminders()
{
	INT i, n = 0;
	CString s;
	tDate today = CurrentTime();
	m_RemindersButton.ShowWindow(SW_HIDE);
	for (i = 0; i < Reminders.GetCount(); ++i)
	{
		tOperation *p = Reminders[i];
		if (p->dateEffective.ToNumber() < today.ToNumber())
		{
			++n;
		}
	}
	if (n)
	{
		s.Format(_T("You have %d reminder%c"), n, n > 1 ? 's' : ' ');
		m_RemindersButton.SetWindowText(s);
		m_RemindersButton.ShowWindow(SW_SHOW);
	}
}


void CCashPlannerDlg::OnViewInventoryfile()
{
	int i;
	CSelectLinesDlg d(false, this);
	if (d.DoModal() == IDOK)
	{
		tStockItemSet stock;
		CShowSetDlg sd(this, stock); 
		sd.SetMode(NULL, CShowSetDlg::modeViewFile);		
		tDate t = d.m_Date; 

		for (i = 0; i < d.ops.GetCount(); ++i)
		{
			tOperation *p = d.ops[i];
			stock.Copy(p->items);
		}

		if (sd.DoModal() == IDOK && VerifySupportedForOperational())
		{
			while (!stock.items.IsEmpty())
			{
				int nItems = 1;
				tStockItem *pi = stock.items[0];
				stock.items.RemoveAt(0);
				tOperation *p = new tOperation;
				p->type = opPurchase;
				p->bTaxable = true;
				p->ClientName = pi->FullSourceName();
				p->dateCreated = p->dateEffective = t;
				p->name = _T("Sync"); 
				p->items.Include(pi);
				for (i = 0; i < stock.items.GetCount(); ++i)
				{
					tStockItem *pi2 = stock.items[i];
					CString s = pi2->source + pi2->sourceId;
					if (!p->ClientName.CompareNoCase(s))
					{
						nItems++;
						p->items.Include(pi2);
						stock.items.RemoveAt(i--);
						if (nItems >= 100) break;
					}
				}
				p->sum = p->items.TotalCost();
				AddNewOperation(p, p->dateCreated);
			}

			RecalculateAll();
			UpdatePage();
			ProcessChange();
		}
	}
}

void CCashPlannerDlg::QueryStockClasses(CStringArray& a)
{
	a.RemoveAll();
	a.Copy(stockClasses);
}

bool CCashPlannerDlg::QueryClassPerModel(const CString& _model, CString& Class)
{
	CString model = _model;
	model.MakeUpper();
	Class.Empty();
	int iClass;
	if (stockModels.Lookup(model, iClass) && iClass >= 0 && iClass < stockClasses.GetCount())
	{
		Class = stockClasses[iClass];
	}
	return !Class.IsEmpty();
}

void CCashPlannerDlg::QueryModelsPerClass(const CString& Class, CStringArray& models)
{
	POSITION pos = stockModels.GetStartPosition();
	int existing = -1;
	if (!Class.IsEmpty())
	{
		for (int i = 0; existing < 0 && i < stockClasses.GetCount(); ++i)
		{
			if (!stockClasses[i].CompareNoCase(Class))
			existing = i;
		}
		if (existing < 0) return;
	}
	while (pos)
	{
		CString modelName;
		int index;
		stockModels.GetNextAssoc(pos, modelName, index);
		if (existing < 0 && existing == index)
		{
			models.Add(modelName);
		}
	}
}

void CCashPlannerDlg::ReapplyPeriodicOps()
{
	AllData.PeriodicOps.Enumerate(*this, false);
	AllData.PeriodicOps.Enumerate(*this, true);
	// must refill the list as its content can be changed!
	UpdatePage();
}

void CCashPlannerDlg::OnToolsPeriodicoperations()
{
	CPeriodicOpsDlg d(AllData.PeriodicOps, Clients[opExpense], this);
	d.DoModal();
	ReapplyPeriodicOps();
	
	RecalculateAll();
	UpdatePage();
	ProcessChange();
}

void CCashPlannerDlg::ConfigureForWorkspace()
{
	bool bEnable;
	int i;
	UINT idsToDisableInOperational[] = 
	{
		ID_FILE_LOADFROM,
		ID_IMPORT_SALESFILE,
		ID_VIEW_INVENTORYFILE,
		ID_DATA_LOADWORKSPACE,
		ID_IMPORTWORKSPACE,
		ID_EDIT_CLEAR_MONTHLY
	};
	COLORREF aColor = RGB(255,255,220);
	COLORREF color = workSpace.fOperational ? initialColor : aColor;
	CString s;
	workSpace.bExisting = workSpace.CheckFileExists();
	
	bEnable = !workSpace.fOperational; 
	for (i = 0; i < ELEMENTS_IN(idsToDisableInOperational); ++i)
	{
		GetMenu()->EnableMenuItem(idsToDisableInOperational[i], (bEnable ? MF_ENABLED :  MF_GRAYED) | MF_BYCOMMAND);
	}

	bEnable = workSpace.From.IsValid() && workSpace.To.IsValid();
	GetMenu()->EnableMenuItem(ID_FILE_CURRENTWORKSPACE, (bEnable ? MF_ENABLED :  MF_GRAYED) | MF_BYCOMMAND);
	
	bEnable = !workSpace.name.IsEmpty() && !workSpace.bAutosave;
	GetMenu()->EnableMenuItem(ID_SAVEWORKSPACENOW, (bEnable ? MF_ENABLED :  MF_GRAYED) | MF_BYCOMMAND);
	
	// configure title etc according to workspace
	CTime t = currentData->month.ToCTime(1);
	s = t.Format(_T("[%B, %Y]"));
	s.AppendFormat(_T(" %s "), workSpace.name.IsEmpty() ? _T("<unnamed>") : (LPCTSTR)workSpace.name);
	if (bModified)
	{
		s += _T(" * ");
	}
	if (workSpace.name.IsEmpty()) workSpace.bAutosave = false;
	if (workSpace.bAutosave)
	{
		s += _T("(AutoUpdate)");
	}

	SetWindowText(s);
	
	if (color != m_List.GetBkColor())
	{
		m_List.SetBkColor(color);
		m_List.SetTextBkColor(color);
		m_List.RedrawWindow();
	}
}


void CCashPlannerDlg::OnWorkspaceDatesChanged()
{
	if (workSpace.From.Compare(DateLimits[0]) || workSpace.To.Compare(DateLimits[1]))
	{
		tDate date = currentData->month;
		DateLimits[0] = workSpace.From;
		DateLimits[1] = workSpace.To;
		if (!CheckDateLimits(date, DateLimits[0], DateLimits[1]))
		{
			EnsureExists(date, currentIndex, false);
			currentData = AllData[currentIndex];
		}
		ReapplyPeriodicOps();
		RecalculateAll();
		UpdatePage();
	}
}

bool CCashPlannerDlg::ConfirmContinueIfModified()
{
	bool b = !bModified;
	CString s = _T("The workspace contains changes that was not saved.\nDo you want to continue and disregard these changes?");
	if (!b)
	{
		b = IDYES == AfxMessageBox(s, MB_YESNO | MB_DEFBUTTON2);
	}
	return b;
}

void CCashPlannerDlg::OnNewWorkspace(bool bOperational)
{
	tWorkspaceConfig w;
	w.fOperational = bOperational;
	w.bTypeChangeable = false;
	w.From = DateLimits[0];
	w.To = DateLimits[1];
	CWorkspaceCfgDlg d(w, this);
	if (IDOK == d.DoModal() && ConfirmContinueIfModified())
	{
		LoadClearCfg clearCfg;
		workSpace = w;
		workSpace.bTypeChangeable = !workSpace.fOperational;
		ClearData(clearCfg);
		OnWorkspaceDatesChanged();
		RecalculateAll();
		ProcessChange();
	}
}


void CCashPlannerDlg::OnOperationalNew()
{
	OnNewWorkspace(true);
}

void CCashPlannerDlg::OnAnalysisNew()
{
	OnNewWorkspace(false);
}

void CCashPlannerDlg::OnWorkSpaceLoad(bool bOperational, bool bOverride)
{
	CWorkspacesDlg d(this);
	d.bOperational = bOperational;
	d.bSelect = true;
	if (d.DoModal() == IDOK && ConfirmContinueIfModified())
	{
		LoadClearCfg loadCfg, clearCfg;
		workSpace.name = d.SelectedName;
		workSpace.fOperational = bOperational;
		CString filename = workSpace.Filename();
		if (bOverride)
		{
			workSpace.name.Empty();
			workSpace.fOperational = false;
		}
		// this effectively skips merge dialog
		// and sets: clear all, load all
		clearCfg.SetAll(true, false);
		loadCfg.SetAll(true, false);
		
		workSpace.bTypeChangeable = !workSpace.fOperational;
		ExecuteLoadFromFile(filename, clearCfg, loadCfg);	
		OnWorkspaceDatesChanged();
		ProcessChange();
	}
}

void CCashPlannerDlg::OnOperationalLoad()
{
	OnWorkSpaceLoad(true);
}

void CCashPlannerDlg::OnAnalysisLoad()
{
	OnWorkSpaceLoad(false);
}

void CCashPlannerDlg::OnFileCurrentworkspace()
{
	CWorkspaceCfgDlg d(workSpace, this);
	if (d.DoModal() == IDOK)
	{
		workSpace.bTypeChangeable = !workSpace.fOperational;
		OnWorkspaceDatesChanged();
		ProcessChange();
	}
}


void CCashPlannerDlg::OnWorspacesManageworkspaces()
{
	CWorkspacesDlg d(this);
	d.bSelect = false;
	d.DoModal();
	if (workSpace.bAutosave && !workSpace.CheckFileExists())
	{
		ProcessChange();
	}
}


void CCashPlannerDlg::OnSaveworkspacenow()
{
	CString sProfile = workSpace.Filename();
	DoSaveToFile(sProfile);
	bModified = false;
	ConfigureForWorkspace();
}


void CCashPlannerDlg::OnDataLoadworkspace()
{
	OnWorkSpaceLoad(true, true);
}

void CCashPlannerDlg::OnImportworkspace()
{
	CFileDialog fd(true, NULL, NULL, OFN_FILEMUSTEXIST, NULL, this);
	if (fd.DoModal() == IDOK)
	{
		LoadClearCfg loadCfg, ClearCfg;
		workSpace.name.Empty();
		workSpace.fOperational = false;
		workSpace.bTypeChangeable = true;
		ExecuteLoadFromFile(fd.GetPathName(), ClearCfg, loadCfg);
		OnWorkspaceDatesChanged();
		ProcessChange();
	}
}

void CCashPlannerDlg::OnReportsSalesTiming()
{
	CSalesTimingReport d(AllData, this);
	d.DoModal();
}


void CCashPlannerDlg::OnReportsPartners()
{
	
}

BOOL CCashPlannerDlg::OnNcActivate(BOOL bActive)
{
	//bActive = true;
	BOOL b = __super::OnNcActivate(bActive);
	if (!b)
	{
		OutputDebugStringX(_T(__FUNCTION__) _T(": %d = 0!"), bActive);
	}
	return true;
}

void CCashPlannerDlg::OnEditClearMonthly()
{
	LoadClearCfg loadCfg, clearCfg;
	clearCfg.SetAll(true, true);
	clearCfg.periodic.check = clearCfg.periodic.show = false;
	loadCfg.SetAll(false, false);
	CMergeDataDlg d(clearCfg, loadCfg, this);
	if (d.DoModal() == IDOK)
	{
		if (clearCfg.sales.check)
			ClearNonDerived(currentData->Sales);
		if (clearCfg.purchases.check)
			ClearNonDerived(currentData->Purchases);
		if (clearCfg.expenses.check)
			ClearNonDerived(currentData->Expenses);
		if (clearCfg.cash.check)
			ClearNonDerived(currentData->Money);
		RecalculateAll();
		UpdatePage();
	}
}


