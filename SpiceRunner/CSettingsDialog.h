#pragma once


// CSettingsDialog dialog

class CSettingsDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingsDialog)

public:
	CSettingsDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSettingsDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedSelectBin();
};
