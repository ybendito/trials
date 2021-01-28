#pragma once

#include "CommonTypes.h"

// CGraphButton

#define CGraphButton_BaseClass CButton


class CGraphButton : public CGraphButton_BaseClass
{
	DECLARE_DYNAMIC(CGraphButton)

public:
	CGraphButton(tControlEventsSubscriber& _Subscriber);
	virtual ~CGraphButton();
	void PushData(tMonthlyIntArray *p, int day) 
	{ 
		pData = p;
		selectedDay = day % ELEMENTS_IN(*p);
		RedrawWindow();
	}
protected:
	void DoDraw(HDC dc, const RECT& r);
	tControlEventsSubscriber& Subscriber;
	tMonthlyIntArray *pData;
	int selectedDay;

	int lowSum;
	int xStart;
	int yStart;
	int prevSum;
	float pixPerDay;
	float pixPerNis;
	CPen goodPen;
	CPen badPen;
	CPen whitePen;

	POINT CalculatePoint(int day, int sum);
	void GoToPoint(CDC *pDC, int day, int sum) 
	{ 
		POINT pt = CalculatePoint(day, sum);
		pDC->MoveTo(pt); 
	} 
	void LineToPoint(CDC *pDC, int day, int sum);
	POINT CalculateIntersection(int nextday, int sum);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};


