// GraphButton.cpp : implementation file
//

#include "stdafx.h"
#include "CashPlanner.h"
#include "GraphButton.h"


// CGraphButton

IMPLEMENT_DYNAMIC(CGraphButton, CGraphButton_BaseClass)

CGraphButton::CGraphButton(tControlEventsSubscriber& _Subscriber) : 
	goodPen(PS_SOLID, 1, RGB(0,0,255)), 
	badPen(PS_SOLID, 1, RGB(255,0,0)), 
	whitePen(PS_SOLID, 1, RGB(255,255,255)),
	Subscriber(_Subscriber)
{
	pData = NULL;
}

CGraphButton::~CGraphButton()
{
}


BEGIN_MESSAGE_MAP(CGraphButton, CGraphButton_BaseClass)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CGraphButton::OnNMCustomdraw)
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()


POINT CGraphButton::CalculatePoint(int day, int sum)
{
	POINT pt;
	pt.x = xStart + Multiply(pixPerDay, day); 
	pt.y = yStart - Multiply(pixPerNis, sum - lowSum); 
	return pt;
}

void CGraphButton::LineToPoint(CDC *pDC, int day, int sum) 
{ 
	POINT pt = CalculatePoint(day, sum); 
	pDC->LineTo(pt); 
} 

POINT  CGraphButton::CalculateIntersection(int nextday, int sum)
{
	POINT pt = CalculatePoint(nextday, 0);
	float ratio = (float)abs(prevSum) /(float)abs(sum); 
	pt.x -= Multiply(pixPerDay / (1.0f + ratio), 1);
	return pt;
}

void CGraphButton::DoDraw(HDC dc, const RECT& r)
{
	INT i, val = 0;
	CDC *pDC = CDC::FromHandle(dc);
	tMonthlyIntArray& a = *pData;
	pixPerDay = (float)(r.right - r.left) / (float)ELEMENTS_IN(a);
	xStart = (  (r.right - r.left) - Multiply(pixPerDay, ELEMENTS_IN(a)) )  / 2;
	yStart = r.bottom;

	for (i = 0; i < ELEMENTS_IN(a); ++i)
	{
		if (abs(a[i]) > val) val = abs(a[i]); 
	}

	val = Multiply(1.3f, val) + 10;
	lowSum = -val;

	pixPerNis = (float)(r.bottom - r.top) / (float)(val * 2);

	GoToPoint(pDC, 0, 0);
	LineToPoint(pDC, ELEMENTS_IN(a) - 1, 0);

	pDC->SelectObject(whitePen);
	GoToPoint(pDC, selectedDay, lowSum);
	LineToPoint(pDC, selectedDay, -lowSum);

	GoToPoint(pDC, 0, a[0]);
	prevSum = a[0];

	for (i = 0; i < ELEMENTS_IN(a); ++i)
	{
		if (sign(prevSum) * sign(a[i]) < 0)
		{
			POINT pt = CalculateIntersection(i, a[i]);
			pDC->SelectObject(prevSum < 0 ? &badPen : &goodPen);
			pDC->LineTo(pt.x, pt.y);	
		}
		pDC->SelectObject(a[i] < 0 ? &badPen : &goodPen);
		LineToPoint(pDC, i, a[i]);
		prevSum = a[i];
	}
}


// CGraphButton message handlers




void CGraphButton::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LRESULT res = CDRF_DODEFAULT;
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	switch (pNMCD->dwDrawStage)
	{
		case CDDS_PREERASE:
			break;
		case CDDS_PREPAINT:
			res = CDRF_NOTIFYPOSTPAINT;
			break;
		case CDDS_POSTPAINT:
			if (pData) DoDraw(pNMCD->hdc, pNMCD->rc);
			res = CDRF_SKIPDEFAULT;
			break;
		default:
			res = CDRF_SKIPDEFAULT;
			break;
	}
	*pResult = res;
}


void CGraphButton::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	Subscriber.OnRightClick(GetDlgCtrlID());	
}
