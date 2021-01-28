#include "stdafx.h"
#include "Docker.h"

void tDocker::Start()
{
	INT i;
	wi.cbSize = sizeof(wi);
	d.GetWindowInfo(&wi);
	minSize.x = wi.rcWindow.right - wi.rcWindow.left;
	minSize.y = wi.rcWindow.bottom - wi.rcWindow.top;
	OutputDebugStringX(_T("%s: r %d, l %d, t %d, b %d"), _T(__FUNCTION__), wi.rcWindow.right, wi.rcWindow.left, wi.rcWindow.top, wi.rcWindow.bottom);
	for  (i = 0; i < controls.GetCount(); ++i)
	{
		WINDOWINFO cwi;
		tDockingInfo& di = controls[i];
		cwi.cbSize = sizeof(cwi);
		CWnd *p = d.GetDlgItem(di.id);
		if (p) 
			p->GetWindowInfo(&cwi);
		di.x = cwi.rcWindow.left;
		di.y = cwi.rcWindow.top;
		di.cx = cwi.rcWindow.right - cwi.rcWindow.left;
		di.cy = cwi.rcWindow.bottom - cwi.rcWindow.top;
		di.x -= wi.rcClient.left;
		di.y -= wi.rcClient.top;
	}
}

POINT tDocker::Track(int cx, int cy)
{
	CPoint change;
	OutputDebugStringX(_T("%s: %d,%d"), _T(__FUNCTION__), cx, cy);
	if (wi.cbSize && minSize.x && minSize.y)
	{
		WINDOWINFO temp = wi;
		INT i;
		change.x = temp.rcClient.left + cx - temp.rcClient.right;
		change.y = temp.rcClient.top + cy - temp.rcClient.bottom;

		for  (i = 0; i < controls.GetCount(); ++i)
		{
			tDockingInfo& di = controls[i];
			CWnd *p = d.GetDlgItem(di.id);
			if (p)
			{
				if (di.flags & DOCKF_XPOS) di.x += change.x;
				if (di.flags & DOCKF_YPOS) di.y += change.y;
				if (di.flags & DOCKF_XSIZE) di.cx += change.x;
				if (di.flags & DOCKF_YSIZE) di.cy += change.y;
				if (di.flags)
					p->SetWindowPos(NULL, di.x, di.y, di.cx, di.cy, SWP_NOZORDER | SWP_NOCOPYBITS); 
			}
		}
		
		d.GetWindowInfo(&wi);
	}
	return change;
}

void tDocker::MinMaxInfo(MINMAXINFO *pMMI)
{
	if (minSize.x && minSize.y)
		pMMI->ptMinTrackSize = minSize;
}
