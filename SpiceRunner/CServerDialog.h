#pragma once

#include "CServerItem.h"

// CServerDialog dialog

class CServerDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CServerDialog)

public:
	CServerDialog(CServerItem& Item);   // standard constructor
	virtual ~CServerDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DETAILS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
    CServerItem& m_Item;
};
