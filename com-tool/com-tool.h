
// com-tool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CcomtoolApp:
// See com-tool.cpp for the implementation of this class
//

class CcomtoolApp : public CWinApp
{
public:
	CcomtoolApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CcomtoolApp theApp;
