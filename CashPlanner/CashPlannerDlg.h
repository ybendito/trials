
// CashPlannerDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "afxdtctl.h"
#include "CommonTypes.h"
#include "DockDialog.h"
#include "EditItems.h"
#include "StockList.h"
#include "OperationsList.h"
#include "ShowSetDlg.h"
#include "GraphButton.h"
#include "BigGraph.h"

struct tRecalculationTask
{
	tRecalculationTask()
	{
		bUpdateMontlyData = true;
		bUpdateNames = true;
		pCopyStockTo = NULL;
	}
	bool bUpdateMontlyData;
	bool bUpdateNames;
	tStockItemSet *pCopyStockTo;
	tDate forceLastDay;
};

// CCashPlannerDlg dialog
class CCashPlannerDlg : 
	public CDockDialog, 
	public COperationsListCallback, 
	public CStockEngine,
	public tControlEventsSubscriber,
	public tListOfPeriodicOperations::Handler
{
// Construction
public:
	CCashPlannerDlg(CWnd* pParent = NULL);	// standard constructor
	~CCashPlannerDlg();
// Dialog Data
	enum { IDD = IDD_CASHPLANNER_DIALOG };
	bool bModified;
	bool bTestMode;
	bool bEditExistingItem;
	bool bAllowAnyDate;
	COperationsList::ColumnId filterBy;
	CString currentFilter;
	
	tWorkspaceConfig workSpace;

	INT currentIndex;
	ULONG currentDayForCash;

	tDate DateLimits[2];
	tMonthlyData *currentData;
	OpsArray *currentDestination;
	OpsArray Reminders;
	CStringArray Clients[opMax];
	
	CStringArray stockClasses;
	CMap<CString,LPCTSTR,int,int> stockModels;

	tMonthlyDataCollection AllData;
	void ShowHideControl(UINT id, bool bShow);
	void FillListView(tOperationType opType, bool bClearEdit);
	tOperation *FindSelectedOperation();
	bool VerifySupportedForOperational();
	bool ConfirmContinueIfModified();
	void ClearData(LoadClearCfg& clearCfg);
	void OnWorkspaceDatesChanged();
	bool ExecuteLoadFromFile(CString fname, LoadClearCfg& clearCfg, LoadClearCfg& loadCfg);
	void ChangeCashDay(bool b);
	void UpdatePage();
	void UpdateListItem(INT item, const tOperation& op);
	void UpdateOperationEditControls(tOperation *p);
	void UpdateDailyCash();
	void SetEditMode(bool bEdit = TRUE, bool bEnable = TRUE);
	int GetSumFromBox();
	void CheckReminders();
	bool ValidateInfoFromControl();
	void ProcessCost(tOperation *p);
	void ProcessMoney(tOperation *p);
	void ProcessModels(const OpsArray& ops);
	//void CommitTemporaryItem(tOperation *p, int nItem);
	tMonthlyData *EnsureExists(const tDate& date, INT& index, bool bForce);
	tMonthlyData *EnsureExistsToday(INT& index);
	void AddEntryByDate(tOperation *p, const tDate& date);
	void AddNewOperation(tOperation *p, const tDate& date);
	void RecalculateAll(tRecalculationTask *pTask = NULL);
	void EditItems(tStockItemSet& items, bool bAddToStock);
	void UpdateUI();
	void EnableMenuItem(UINT id, bool bEnable);
	void DoSaveToFile(CString sProfile);
	void ProcessChange();
	void ReapplyPeriodicOps();
	void OnWorkSpaceLoad(bool bOperational, bool bOverride = false);
	void OnNewWorkspace(bool bOperational);
	void ConfigureForWorkspace();
	
	int GetProfileInt(LPCTSTR section, LPCTSTR name, int _default)
	{
		UINT val = AfxGetApp()->GetProfileInt(section, name, _default);
		return (int)val;
	}
	void WriteProfileInt(LPCTSTR section, LPCTSTR name, int value)
	{
		AfxGetApp()->WriteProfileInt(section, name, value);
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	COLORREF initialColor;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	COperationsList m_List;
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnFileNextMonth();
	afx_msg void OnFilePreviousmonth();
	afx_msg void OnUpdateFilePreviousmonth(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedButtonNextMonth();
	afx_msg void OnBnClickedButtonPrevMonth();
	afx_msg void OnTestClickedButton();
	afx_msg void OnBnClickedRadioCash();
	afx_msg void OnBnClickedRadioSale();
	afx_msg void OnBnClickedRadioPurchase();
	afx_msg void OnBnClickedRadioExpense();
	afx_msg void OnBnClickedRadioStock();
	CButton m_ApplyButton;
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedApply();
	CEdit m_NameEditBox;
	CEdit m_SumEditBox;
	CDateTimeCtrl m_DateBoxEffective;
	afx_msg void OnSaveAs();
	afx_msg void OnToolsVatcalculator();
	afx_msg void OnStnClickedLabelSum2();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnFileLoadfrom();
	afx_msg void OnStnClickedLabelInv();
	CDateTimeCtrl m_DateBoxCreated;
	afx_msg void OnAbout();
	afx_msg void OnBnClickedFilterActive();
	afx_msg void OnEnChangeEditFilter();
	afx_msg void OnEditPostpone();
	virtual void OnCancel();
	CComboBox m_ClientBox;
	afx_msg void OnImportSalesfile();
protected:
	CEditItems m_CostBox;
	void OnItemActivate(tOperation *p);
	// enumerator of periodic operations
	void onNextPeriodicOperation(tPeriodicOperation *p, bool bParam);

	// CStockEngine
	void StockRecalculate(tStockItemSet& _items, const tDate& d);
	void QueryStockClasses(CStringArray& a);
	bool QueryClassPerModel(const CString& model, CString& Class);
	void QueryModelsPerClass(const CString& Class, CStringArray& models);

	// docking
	const tDockTable *GetDockedControlsTable(); 
	void ResizeDone(POINT change);
	// additional control events
	virtual void OnRightClick(UINT ctrlId);

public:
	afx_msg void OnStockLatest();
	afx_msg void OnCbnSelchangeComboFilter();
	CComboBox m_FilterCombo;
	CGraphButton m_Graph;
	CBigGraph m_BigGraph;
	afx_msg void OnClickedCashGraph();
	CButton m_RemindersButton;
	afx_msg void OnBnClickedReminders();
	afx_msg void OnViewInventoryfile();
	afx_msg void OnToolsPeriodicoperations();
	afx_msg void OnAnalysisNew();
	afx_msg void OnAnalysisLoad();
	afx_msg void OnOperationalNew();
	afx_msg void OnOperationalLoad();
	afx_msg void OnFileCurrentworkspace();
	afx_msg void OnWorspacesManageworkspaces();
	afx_msg void OnSaveworkspacenow();
	afx_msg void OnDataLoadworkspace();
	afx_msg void OnReportsSalesTiming();
	afx_msg void OnReportsPartners();
	afx_msg void OnImportworkspace();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnEditClearMonthly();
};
