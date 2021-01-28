#pragma once

#include "BasicList.h"
#include "CommonTypes.h"

class CSalesTimingReportList : public CBasicList
{
public:
	CSalesTimingReportList(tMonthlyDataCollection& _data) : data(_data) { }
	tMonthlyDataCollection& data;
	virtual void Prepare();
protected:
	virtual PVOID PullItemData(int itemNo);
	virtual CString PullItemString(PVOID p, int id);
	struct tDataUnit
	{
		tDate date;
		int Total;
		int Now;
		int Next;
		int _30;
		int _60;
		int _90;
	};
	CArray<tDataUnit> table;
};


// CSalesTimingReport dialog

class CSalesTimingReport : public CDialogEx
{
	DECLARE_DYNAMIC(CSalesTimingReport)

public:
	CSalesTimingReport(tMonthlyDataCollection& _data, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSalesTimingReport();

// Dialog Data
	enum { IDD = IDD_DIALOG_COMMON_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	CSalesTimingReportList m_List;
	virtual BOOL OnInitDialog();
};
