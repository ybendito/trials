#pragma once
#include "afxwin.h"
#include "CommonTypes.h"
#include "ExcelConvert.h"


// CSelectLinesDlg dialog

class CSelectLinesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectLinesDlg)

public:
	CSelectLinesDlg(bool _bSalesFile, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectLinesDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_IMPORT };

protected:
	void PreprocessFile();	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CString textFilename;
	void ExamineLog(CString s);
	void SetPosition(int ctrlId,int lineNo);
	void ResetState(bool bAlsoCombo);

	DECLARE_MESSAGE_MAP()
public:
	bool bProcessed;
	bool bSalesFile;
	int m_NoReminders; 
	int m_FirstLine;
	int m_LastLine;
	int m_Debug;
	int m_UnnamedBeforeDate;
	tExcelConverter Converter;
	CTime m_Date;
	CDateTimeCtrl m_DateBox;
	tArrayOfOperations ops;

	afx_msg void OnBnClickedBrowse();
	afx_msg void OnCbnCloseupCombo1();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBnClickedApply();
	CComboBox m_PointsBox;
	afx_msg void OnCbnSelchangePoints();
	afx_msg void OnBnClickedStartPos();
	afx_msg void OnBnClickedEndPos();
	virtual BOOL OnInitDialog();
};
