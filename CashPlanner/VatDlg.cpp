// VatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "VatDlg.h"
#include "CashPlannerDlg.h"
#include "afxdialogex.h"


// CVatDlg dialog

IMPLEMENT_DYNAMIC(CVatDlg, CDialogEx)

CVatDlg::CVatDlg(CWnd* pParent, ULONG& _val)
	: CDialogEx(CVatDlg::IDD, pParent), val(_val)
{
	bPlus = true;
	lval = rval = 0;
	maskUpdate = 0;
}

void CVatDlg::UpdateLR()
{
	if (bPlus)
		rval  = lval + VAT(lval);
	else
	{
		float f = 1.0f / 1.18f;
		rval = Multiply(f, lval);
	}
	if (~maskUpdate & 1)
		SetDlgItemInt(IDC_LVAL, lval);
	if (~maskUpdate & 2)
		SetDlgItemInt(IDC_RVAL, rval);
}

void CVatDlg::UpdateRL()
{
	if (!bPlus)
		lval  = rval + VAT(rval);
	else
	{
		float f = 1.0f / 1.18f;
		lval = Multiply(f, rval);
	}
	if (~maskUpdate & 1)
		SetDlgItemInt(IDC_LVAL, lval);
	if (~maskUpdate & 2)
		SetDlgItemInt(IDC_RVAL, rval);
}


CVatDlg::~CVatDlg()
{
}

void CVatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVatDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SIGN, &CVatDlg::OnBnClickedSign)
	ON_BN_CLICKED(IDC_SWAP, &CVatDlg::OnBnClickedSwap)
	ON_EN_CHANGE(IDC_LVAL, &CVatDlg::OnEnChangeLval)
	ON_EN_CHANGE(IDC_RVAL, &CVatDlg::OnEnChangeRval)
END_MESSAGE_MAP()


// CVatDlg message handlers


void CVatDlg::OnBnClickedSign()
{
	bPlus = !bPlus;
	SetDlgItemText(IDC_SIGN, bPlus ? TEXT("+VAT") : TEXT("-VAT"));
	UpdateLR();
}


BOOL CVatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	lval = val;

	UpdateLR();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CVatDlg::OnBnClickedSwap()
{
	lval = rval;
	OnBnClickedSign();
}


void CVatDlg::OnEnChangeLval()
{
	maskUpdate |= 1;
	lval = GetDlgItemInt(IDC_LVAL);
	UpdateLR();
	maskUpdate &= ~1;
}


void CVatDlg::OnEnChangeRval()
{
	maskUpdate |= 2;
	rval = GetDlgItemInt(IDC_RVAL);
	UpdateRL();
	maskUpdate &= ~2;
}


void CVatDlg::OnOK()
{
	//CDialogEx::OnOK();
}
