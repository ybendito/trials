// SelectLinesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "SelectLinesDlg.h"
#include "afxdialogex.h"
#include "TextParser.h"
#include "FileWorks.h"

// CSelectLinesDlg dialog

IMPLEMENT_DYNAMIC(CSelectLinesDlg, CDialogEx)

CSelectLinesDlg::CSelectLinesDlg(bool _bSalesFile, CWnd* pParent /*=NULL*/)
	: CDialogEx(CSelectLinesDlg::IDD, pParent),
	bSalesFile(_bSalesFile),
	m_FirstLine(0),
	m_LastLine(0),
	m_Debug(false),
	m_UnnamedBeforeDate(true),
	m_NoReminders(false)
{
	bProcessed = false;
	if (bSalesFile)
		m_Date = CTime(2015,9,1,1,0,0);
	else
		m_Date = CTime(2014,10,1,1,0,0);
}

CSelectLinesDlg::~CSelectLinesDlg()
{
	ops.ClearItems();
}

void CSelectLinesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FIRST_LINE, m_FirstLine);
	DDX_Text(pDX, IDC_LAST_LINE, m_LastLine);
	DDX_Check(pDX, IDC_PARSER_DEBUG, m_Debug);
	DDX_Check(pDX, IDC_NO_REMINDERS, m_NoReminders);
	DDX_Check(pDX, IDC_CHECK_UNNAMED_BEFORE, m_UnnamedBeforeDate);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_Date);
	DDX_Control(pDX, IDC_POINTS, m_PointsBox);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_DateBox);
}


BEGIN_MESSAGE_MAP(CSelectLinesDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BROWSE, &CSelectLinesDlg::OnBnClickedBrowse)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_APPLY, &CSelectLinesDlg::OnBnClickedApply)
	ON_CBN_SELCHANGE(IDC_POINTS, &CSelectLinesDlg::OnCbnSelchangePoints)
	ON_BN_CLICKED(IDC_START_POS, &CSelectLinesDlg::OnBnClickedStartPos)
	ON_BN_CLICKED(IDC_END_POS, &CSelectLinesDlg::OnBnClickedEndPos)
END_MESSAGE_MAP()


// CSelectLinesDlg message handlers


void CSelectLinesDlg::OnBnClickedBrowse()
{
	ResetState(true);
	CFileDialog fd(true, NULL, NULL, OFN_FILEMUSTEXIST, _T("Excel files|*.xls;*.xlsx|All files|*.*||"), this);
	if (fd.DoModal() == IDOK)
	{
		CString filename = fd.GetPathName();
		SetDlgItemText(IDC_NAME, filename);
		if (Converter.Start(filename))
			GetDlgItem(IDC_APPLY)->EnableWindow(true);
	}
}


void CSelectLinesDlg::OnDropFiles(HDROP hDropInfo)
{
	TCHAR name[MAX_PATH];
	// TODO: Add your message handler code here and/or call default
	ResetState(true);
	if (DragQueryFile(hDropInfo, 0, name, ELEMENTS_IN(name)))
	{
		CString filename = name;
		SetDlgItemText(IDC_NAME, filename);
		if (Converter.Start(filename))
			GetDlgItem(IDC_APPLY)->EnableWindow(true);
	}

	CDialogEx::OnDropFiles(hDropInfo);
}

void CSelectLinesDlg::PreprocessFile()
{
	tArrayOfOperations dummy;
	CArray<tNamedInt> points;
	tTextParser *Parser;
	if (bSalesFile)
	{
		Parser = new tSalesFileTextParser(Converter.ResultFile(), 0, INT_MAX, !!m_Debug);
	}
	else
	{
		Parser = new tStockFileTextParser(Converter.ResultFile(), 0, INT_MAX, !!m_Debug);
	}
	Parser->Parse(dummy, &points);
	if (points.GetCount())
	{
		m_PointsBox.ResetContent();
		while (points.GetCount())
		{
			tNamedInt t = points[0];
			points.RemoveAt(0);
			int n = m_PointsBox.AddString(t.name);
			m_PointsBox.SetItemData(n, t.num);
		}
		m_PointsBox.SetCurSel(0);
		OnCbnSelchangePoints();
	}
	delete Parser;
}

void CSelectLinesDlg::ExamineLog(CString sLog)
{
	CFileDialog fd(false);
	fd.m_ofn.lpstrTitle = _T("Save log file");
	if (fd.DoModal() == IDOK)
	{
		CWriteProfile f(fd.GetPathName());
		f.WriteString(sLog);
	}
}


void CSelectLinesDlg::OnBnClickedApply()
{
	UpdateData(true);
	if (!m_FirstLine || !m_LastLine || m_FirstLine == m_LastLine)
	{
		PreprocessFile();	
	}
	else if (!bProcessed)
	{
		bool b;
		CString sLog;
		ops.ClearItems();
		tTextParser *Parser;
		if (bSalesFile)
		{
			Parser = new tSalesFileTextParser(Converter.ResultFile(), m_FirstLine, m_LastLine, !!m_Debug);
		}
		else
		{
			Parser = new tStockFileTextParser(Converter.ResultFile(), m_FirstLine, m_LastLine, !!m_Debug);
		}
		b = Parser->Parse(ops);
		sLog.Format(_T("Parsing finished %s. %d entries were read from file"), 
			b ? _T("OK") : _T("by error"),
			ops.GetCount());
		AfxMessageBox(sLog);
		if (Parser->GetLog(sLog) && m_Debug)
		{
			ExamineLog(sLog);
		}
		GetDlgItem(IDC_APPLY)->SetWindowText(_T("Done"));
		delete Parser;
		bProcessed = true;
	}
	else
	{
		EndDialog(IDOK);
	}
}


void CSelectLinesDlg::OnCbnSelchangePoints()
{
	int n = m_PointsBox.GetCurSel();
	int  nEnable = 0;
	if (n >= 0)
	{
		n = (int)m_PointsBox.GetItemData(n);
		nEnable = n != 0;
	}
	GetDlgItem(IDC_START_POS)->EnableWindow(nEnable && n > 0);
	GetDlgItem(IDC_END_POS)->EnableWindow(nEnable && n != 0);
}

void CSelectLinesDlg::SetPosition(int ctrlId,int lineNo)
{
	if (lineNo < 0) lineNo = -lineNo;
	SetDlgItemInt(ctrlId, lineNo);
	ResetState(false);
}

void CSelectLinesDlg::OnBnClickedStartPos()
{
	SetPosition(IDC_FIRST_LINE, m_PointsBox.GetItemData(m_PointsBox.GetCurSel()));
}


void CSelectLinesDlg::OnBnClickedEndPos()
{
	SetPosition(IDC_LAST_LINE, m_PointsBox.GetItemData(m_PointsBox.GetCurSel()));
}


BOOL CSelectLinesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetTimeFormat(m_DateBox);

	GetDlgItem(IDC_NO_REMINDERS)->EnableWindow(bSalesFile);
	GetDlgItem(IDC_CHECK_UNNAMED_BEFORE)->EnableWindow(bSalesFile);
	if (!bSalesFile)
	{
		GetDlgItem(IDC_CHECK_UNNAMED_BEFORE)->SetWindowText(_T("Purchase date"));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectLinesDlg::ResetState(bool bAlsoCombo)
{
	if (bAlsoCombo)
	{
		m_PointsBox.ResetContent();
		m_FirstLine = 0;
		m_LastLine = 0;
		UpdateData(false);			
		GetDlgItem(IDC_APPLY)->EnableWindow(false);
	}
	bProcessed = false;
	GetDlgItem(IDC_APPLY)->SetWindowText(_T("Start"));
}