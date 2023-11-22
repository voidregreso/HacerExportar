

#pragma once

#ifndef __AFXWIN_H__
#error "Include stdafx.h before including this file to generate the PCH file"
#endif

#include "resource.h"

class CHacerExportarApp : public CWinAppEx
{
public:
	CHacerExportarApp();

public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CHacerExportarApp theApp;