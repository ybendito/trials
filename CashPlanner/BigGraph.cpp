// BigGraph.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "BigGraph.h"
#include "afxdialogex.h"


// CBigGraph dialog

IMPLEMENT_DYNAMIC(CBigGraph, CDockDialog)

CBigGraph::CBigGraph(CWnd* _pParent /*=NULL*/)
	: CDockDialog(CBigGraph::IDD, _pParent), m_Graph(*this)
{
	bVisible = true;
	pParent = _pParent;
}

CBigGraph::~CBigGraph()
{
}

void CBigGraph::PushData(tMonthlyIntArray *p, int day)
{
	if (GetSafeHwnd())
	{
		m_Graph.PushData(p, day);
	}
}

void CBigGraph::Toggle()
{
	if (GetSafeHwnd())
	{
		bVisible = !bVisible;
	}
	else
	{
		Create(CBigGraph::IDD, pParent);
		OnModelessCreate();
	}
	ShowWindow(!IsWindowVisible() ? SW_SHOW : SW_HIDE);
}


void CBigGraph::DoDataExchange(CDataExchange* pDX)
{
	CDockDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CASH_GRAPH, m_Graph);
}

const CDockDialog::tDockTable *CBigGraph::GetDockedControlsTable()
{
	static const tDockTable table[] = {
	{IDC_CASH_GRAPH, DOCKF_XSIZE},
	{0, 0},
	};
	return table;
}


BEGIN_MESSAGE_MAP(CBigGraph, CDockDialog)
END_MESSAGE_MAP()


// CBigGraph message handlers


void CBigGraph::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//__super::OnOK();
}


void CBigGraph::OnCancel()
{
	ShowWindow(SW_HIDE);
}


