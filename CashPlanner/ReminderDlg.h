#pragma once

#include "OperationsList.h"
#include "afxcmn.h"
#include "afxdtctl.h"

// CReminderDlg dialog

class CReminderDlg : public CDialogEx, COperationsListCallback
{
	DECLARE_DYNAMIC(CReminderDlg)

public:
	CReminderDlg(OpsArray& _reminders, CWnd* pParent = NULL);   // standard constructor
	virtual ~CReminderDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_REMINDERS };

protected:
	OpsArray& reminders;
	virtual void OnItemActivate(tOperation *p);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void ListRefill();
	DECLARE_MESSAGE_MAP()
public:
	COperationsList m_List;
	CDateTimeCtrl m_DateBox;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedChangeReminder();
	afx_msg void OnBnClickedStop();
};
