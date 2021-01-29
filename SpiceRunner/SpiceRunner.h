
// SpiceRunner.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSpiceRunnerApp:
// See SpiceRunner.cpp for the implementation of this class
//

class CSpiceRunnerApp : public CWinApp
{
public:
	CSpiceRunnerApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CSpiceRunnerApp theApp;
