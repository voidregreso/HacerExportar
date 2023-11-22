#pragma once
#include "afxwin.h"

typedef enum
{
	OBJECT_TYPE_unknown,
	OBJECT_TYPE_namespace,
	OBJECT_TYPE_class,
	OBJECT_TYPE_classfun, // member funcs in a class
	OBJECT_TYPE_function,
	OBJECT_TYPE_variant
} OBJECT_TYPE;

typedef struct objectitem
{
	OBJECT_TYPE type;
	CString name;
	XLIB::CXMap<CString, objectitem> items[4]; // Subtree, the first one is public, the second one is publicitems, protecteditems, privateitems.
} OBJECTITEM, *LPOBJECTITEM;

typedef struct
{
	int index; // Starting from 0, but the derived function number starts from 1
	CString name;
	DWORD addr; // Offset, the actual address needs to be added to the base address
} EXPORTDATA, *LPEXPORTDATA;

class CHacerExportarDlg : public CDialog
{
public:
	CHacerExportarDlg(CWnd *pParent = NULL);

	enum
	{
		IDD = IDD_HACEREXPORTAR_DIALOG
	};

protected:
	virtual void DoDataExchange(CDataExchange *pDX);

	XLIB::CXMap<int, EXPORTDATA> m_FunctionMap;

	BOOL LoadExport(LPCTSTR lpFilePath);
	CString ShowNormal();
	CString ShowAsDef();
	CString ShowAsSource();

	VOID ParseClass(LPCTSTR lpName);
	int DetermineProtectionType(CString &name);
	int FindFunctionPosition(const CString &name);
	CString ExtractArguments(CString &name, int pos);
	VOID ExtractTypeAndObject(CString &name, int pos, CString &type, CString &object);
	int FindSpaceBeforeOperator(const CString &name, int pos);
	LPOBJECTITEM ProcessNamespaces(CString &type, CString &object, CString &arguments);
	VOID CleanNamespace(CString &namespaceName, CString &type, CString &arguments);
	VOID CleanTypeString(CString &type);
	VOID FormatFinalName(CString &name, const CString &type, const CString &object, const CString &arguments);
	VOID RemoveDefaultFunctions(CString &name, LPOBJECTITEM item);
	VOID ParserVariant(LPCTSTR lpName);
	VOID ParserFunction(LPCTSTR lpName);

	CString m_strTitle;

	OBJECTITEM m_ObjectList;

	BOOL m_bRemoveCCall;
	VOID PrintObjects(LPOBJECTITEM item, CString &strText, LPCTSTR prefix = 0, int itemindex = 0);
	LPOBJECTITEM ProbeForWriteObject(LPCTSTR name, LPOBJECTITEM parent = 0, int itemindex = 0);

	VOID AddUnParser(LPCTSTR lpname);

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_WndDllPath;
	afx_msg void OnBnClickedOpenfile();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CEdit m_wndOut;
	CButton m_wndCplusplus;
	CButton m_wndnormal;
	CButton m_wnddef;
	CButton m_wndsource;
	CButton m_wndRemovecCall;
	afx_msg void OnBnClickedButton1();
	CButton m_wndByOrdin;
	afx_msg void OnBnClickedNormal();
	afx_msg void OnBnClickedDef();
	afx_msg void OnBnClickedSrc();
};
