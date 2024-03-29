// CServerDialog.cpp : implementation file
//

#include "pch.h"
#include "SpiceRunner.h"
#include "CServerDialog.h"


// CServerDialog dialog

IMPLEMENT_DYNAMIC(CServerDialog, CDialogEx)

CServerDialog::CServerDialog(CServerItem& item)
	: CDialogEx(IDD_SERVER_DETAILS, NULL), m_Item(item)
{

}

CServerDialog::~CServerDialog()
{
}

void CServerDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CONNECTNOW, m_Item.m_ConnectNow);
    DDX_Text(pDX, IDC_ADDRESS, m_Item.m_HostName);
    if (pDX->m_bSaveAndValidate && m_Item.m_HostName.IsEmpty())
    {
        pDX->Fail();
    }
    DDX_Text(pDX, IDC_PORT, m_Item.m_Port);
    DDX_Text(pDX, IDC_WAITTIME, m_Item.m_WaitTime);
    DDX_Radio(pDX, IDC_RADIO_SPICE, m_Item.m_Viewer);
    DDV_MinMaxUInt(pDX, m_Item.m_Port, 1, 0xffff);
}


BEGIN_MESSAGE_MAP(CServerDialog, CDialogEx)
END_MESSAGE_MAP()


// CServerDialog message handlers
