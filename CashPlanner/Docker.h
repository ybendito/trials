#pragma once



#define DOCKF_XPOS		1
#define DOCKF_XSIZE		2
#define DOCKF_YPOS		4
#define DOCKF_YSIZE		8

struct tDockingInfo
{
	UINT id;
	UINT flags;
	int x;
	int y;
	int cx;
	int cy;
};

class tDocker
{
public:
	tDocker(CDialog& dlg) : d(dlg)
	{
		wi.cbSize = 0;
		minSize.x = minSize.y = 0;
	}
	void AddControl(UINT id, UINT flags) { tDockingInfo i; i.id = id; i.flags = flags; controls.Add(i); }
	void Start();
	POINT Track(int cx, int cy);
	void MinMaxInfo(MINMAXINFO *pMMI);
protected:
	CArray<tDockingInfo> controls; 
	CDialog& d;
	WINDOWINFO wi;
	POINT minSize;
};
