#pragma once


// CWaitDlg dialog

class CWaitDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWaitDlg)

public:
	CWaitDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWaitDlg();

// Dialog Data
	enum { IDD = IDD_MESSAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();
	virtual void OnOK();
};
