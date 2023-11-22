
#include "stdafx.h"
#include "HacerExportar.h"
#include "HacerExportarDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Dbghelp.lib")

CHacerExportarDlg::CHacerExportarDlg(CWnd *pParent /*=NULL*/)
	: CDialog(CHacerExportarDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHacerExportarDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DLLPATH, m_WndDllPath);
	DDX_Control(pDX, IDC_OUTLIST, m_wndOut);
	DDX_Control(pDX, IDC_CHECK1, m_wndCplusplus);
	DDX_Control(pDX, IDC_NORMAL, m_wndnormal);
	DDX_Control(pDX, IDC_DEF, m_wnddef);
	DDX_Control(pDX, IDC_SRC, m_wndsource);
	DDX_Control(pDX, IDC_CHECK2, m_wndRemovecCall);
	DDX_Control(pDX, IDC_CHECK3, m_wndByOrdin);
}

BEGIN_MESSAGE_MAP(CHacerExportarDlg, CDialog)
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDC_OPENFILE, &CHacerExportarDlg::OnBnClickedOpenfile)
ON_BN_CLICKED(IDOK, &CHacerExportarDlg::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CHacerExportarDlg::OnBnClickedCancel)
ON_BN_CLICKED(IDC_BUTTON1, &CHacerExportarDlg::OnBnClickedButton1)
ON_BN_CLICKED(IDC_NORMAL, &CHacerExportarDlg::OnBnClickedNormal)
ON_BN_CLICKED(IDC_DEF, &CHacerExportarDlg::OnBnClickedDef)
ON_BN_CLICKED(IDC_SRC, &CHacerExportarDlg::OnBnClickedSrc)
END_MESSAGE_MAP()

BOOL CHacerExportarDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_wndCplusplus.SetCheck(1);
	m_wndnormal.SetCheck(1);
	m_wndRemovecCall.SetCheck(1);

	GetDlgItem(IDC_CHECK3)->EnableWindow(FALSE);
	m_ObjectList.type = OBJECT_TYPE_namespace;

	return TRUE;
}

void CHacerExportarDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CHacerExportarDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CHacerExportarDlg::OnBnClickedOpenfile()
{
	CString strPath;
	CFileDialog dlg(TRUE, _T("Choose dll"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Archivo dll(*.dll)|*.dll;|Todos los archivos(*.*)|*.*|"), this);
	if (dlg.DoModal() != IDOK)
		return;

	strPath = dlg.GetPathName();
	m_strTitle = dlg.GetFileTitle();

	m_WndDllPath.SetWindowText(strPath);

	LoadExport(strPath);
	OnBnClickedOk();
}

void CHacerExportarDlg::OnBnClickedOk()
{
	CString strText;

	m_bRemoveCCall = m_wndRemovecCall.GetCheck();

	if (m_wnddef.GetCheck())
	{
		strText = ShowAsDef();
	}
	else if (m_wndsource.GetCheck())
	{
		strText = ShowAsSource();
	}
	else
	{
		strText = ShowNormal();
	}

	m_wndOut.SetWindowText(strText);
}

void CHacerExportarDlg::OnBnClickedCancel()
{
	OnCancel();
}

BOOL CHacerExportarDlg::LoadExport(LPCTSTR lpFilePath)
{
	HMODULE hModule = LoadLibraryEx(lpFilePath, 0, DONT_RESOLVE_DLL_REFERENCES);
	if (0 == hModule)
		return FALSE;

	EXPORTDATA exportdata, *lpData;

	IMAGE_DOS_HEADER *dosheader;
	IMAGE_OPTIONAL_HEADER *opthdr;
	PIMAGE_EXPORT_DIRECTORY exports;
	LPVOID lpRet = 0;
	const char *pFuncName = NULL;
	PULONG pAddressOfFunctions, pAddressOfNames;
	PUSHORT pAddressOfNameOrdinals;
	DWORD i;

	dosheader = (IMAGE_DOS_HEADER *)hModule;
	opthdr = (IMAGE_OPTIONAL_HEADER *)((BYTE *)hModule + dosheader->e_lfanew + 24);
	exports = (PIMAGE_EXPORT_DIRECTORY)((BYTE *)hModule + opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	pAddressOfFunctions = (ULONG *)((BYTE *)hModule + exports->AddressOfFunctions);		   // Func list
	pAddressOfNames = (ULONG *)((BYTE *)hModule + exports->AddressOfNames);				   // Name list
	pAddressOfNameOrdinals = (USHORT *)((BYTE *)hModule + exports->AddressOfNameOrdinals); // The address entry index in the func list corresponding to the name

	m_ObjectList.items[0].clear();
	m_ObjectList.items[1].clear();
	m_ObjectList.items[2].clear();
	m_ObjectList.items[3].clear();
	m_ObjectList.name = _T("");
	m_ObjectList.type = OBJECT_TYPE_namespace;
	m_FunctionMap.clear();

	for (i = 0; i < exports->NumberOfFunctions; i++)
	{
		exportdata.index = i;
		exportdata.addr = pAddressOfFunctions[i];
		m_FunctionMap.Insert(i, exportdata);
	}

	for (i = 0; i < exports->NumberOfNames; i++)
	{
		pFuncName = (const char *)((BYTE *)hModule + pAddressOfNames[i]);

		lpData = m_FunctionMap.Find(pAddressOfNameOrdinals[i]);
		if (lpData)
			lpData->name = pFuncName;
	}

	FreeLibrary(hModule);
	return TRUE;
}

CString CHacerExportarDlg::ShowNormal()
{
	CString strText;

	BOOL bCPlusplus = m_wndCplusplus.GetCheck();

	for (auto item = m_FunctionMap.begin(); item != m_FunctionMap.end(); ++item)
	{
		CString str;
		if (bCPlusplus)
		{
			CStringA strFunName(item->second.name);
			char outname[MAX_PATH];
			ZeroMemory(outname, MAX_PATH);
			if (strFunName.GetLength())
			{
				UnDecorateSymbolName(strFunName, outname, MAX_PATH, UNDNAME_COMPLETE);
				str = CString(outname);
			}
		}
		else
		{
			str = item->second.name;
		}

		CString strValue;
		strValue.Format(_T("%d\t%s\t\t%08x\r\n"), item->second.index + 1, str, item->second.addr);
		strText += strValue;
	}

	return strText;
}

CString CHacerExportarDlg::ShowAsDef()
{
	CString strValue;
	CString strText;
	CString strName;
	int index;

	BOOL bOrdin = m_wndByOrdin.GetCheck();

	strText.Format(_T("LIBRARY %s\r\n"), (LPCTSTR)m_strTitle);
	strText += _T("EXPORTS\r\n");

	for (XLIB::CXMap<int, EXPORTDATA>::iterator item = m_FunctionMap.begin(); item != m_FunctionMap.end(); item++)
	{
		strName = item->second.name;
		index = item->second.index + 1;
		if (strName.GetLength())
		{
			if (bOrdin)
			{
				strValue.Format(_T("%s\t\t@%d\r\n"), (LPCTSTR)strName, index);
			}
			else
			{
				strValue.Format(_T("%s\r\n"), (LPCTSTR)strName);
			}
		}
		else
		{
			strValue.Format(_T("nonename_%d\t\t@%d NONAME PRIVATE\r\n"), index, index);
		}
		strText += strValue;
	}

	return strText;
}

VOID CHacerExportarDlg::PrintObjects(LPOBJECTITEM item, CString &strText, LPCTSTR prefix, int itemindex)
{
	CString strValue;
	CString strPrefix;

	strPrefix = prefix;

	for (XLIB::CXMap<CString, OBJECTITEM>::iterator itembase = item->items[itemindex].begin(); itembase != item->items[itemindex].end(); itembase++)
	{
		switch (itembase->second.type)
		{
		case OBJECT_TYPE_unknown:
			strValue.Format(_T("//noparser %s_API\t%s; \r\n"), (LPCTSTR)m_strTitle, (LPCTSTR)itembase->second.name);
			strText += strValue;
			continue;
		case OBJECT_TYPE_variant:
		case OBJECT_TYPE_function:
			strValue.Format(_T("%s%s_API\t%s;\r\n"), (LPCTSTR)strPrefix, (LPCTSTR)m_strTitle, (LPCTSTR)itembase->second.name);
			strText += strValue;
			continue;
		case OBJECT_TYPE_classfun:

			strValue.Format(_T("%s\t%s;\r\n"), (LPCTSTR)strPrefix, (LPCTSTR)itembase->second.name);
			strText += strValue;
			continue;

		case OBJECT_TYPE_namespace:
			strValue.Format(_T("%snamespace %s\r\n%s{\r\n"), (LPCTSTR)strPrefix, (LPCTSTR)itembase->second.name, (LPCTSTR)strPrefix);
			break;
		case OBJECT_TYPE_class:
			strValue.Format(_T("%sclass %s_API %s\r\n%s{\r\n"), (LPCTSTR)strPrefix, (LPCTSTR)m_strTitle, (LPCTSTR)itembase->second.name, (LPCTSTR)strPrefix);
			break;
		}

		strText += strValue;

		if (itembase->second.items[0].size())
		{
			strPrefix = _T("\t");
			strPrefix += prefix;
			PrintObjects(&itembase->second, strText, strPrefix, 0);
		}
		if (itembase->second.items[1].size())
		{
			strPrefix = prefix;
			strValue.Format(_T("%spublic:\r\n"), (LPCTSTR)strPrefix);
			strText += strValue;

			PrintObjects(&itembase->second, strText, strPrefix, 1);
		}
		if (itembase->second.items[2].size())
		{
			strPrefix = prefix;
			strValue.Format(_T("%sprotected:\r\n"), (LPCTSTR)strPrefix);
			strText += strValue;

			PrintObjects(&itembase->second, strText, strPrefix, 2);
		}
		if (itembase->second.items[3].size())
		{
			strPrefix = prefix;
			strValue.Format(_T("%sprivate:\r\n"), (LPCTSTR)strPrefix);
			strText += strValue;

			PrintObjects(&itembase->second, strText, strPrefix, 3);
		}

		strPrefix = prefix;
		strValue.Format(_T("%s};\r\n\r\n"), (LPCTSTR)strPrefix);
		strText += strValue;
	}
}

CString CHacerExportarDlg::ShowAsSource()
{
	CString strName;
	char outname[MAX_PATH];
	CStringA strFunName;

	CString strText;

	for (XLIB::CXMap<int, EXPORTDATA>::iterator item = m_FunctionMap.begin(); item != m_FunctionMap.end(); item++)
	{
		strFunName = item->second.name;

		ZeroMemory(outname, MAX_PATH);
		if (strFunName.GetLength())
		{
			UnDecorateSymbolName(strFunName, outname, MAX_PATH, UNDNAME_COMPLETE);
			strName = outname;
		}
		else
			continue;

		if (strName.Find(_T("public:")) == 0 || strName.Find(_T("protected:")) == 0 || strName.Find(_T("private:")) == 0)
		{
			ParseClass(strName);
		}
		else if (strName.Find(_T("(")) == -1) // Variable
		{
			ParserVariant(strName);
		}
		else
		{
			ParserFunction(strName);
		}
	}

	strText.Format(_T("// DO NOT MODIFY THIS AUTOMATICALLY GENERATED HEADER FILE!\r\n")
				   _T("#pragma once\r\n\r\n")
				   _T("#define %s_API __declspec(dllimport)\r\n\r\n")
				   _T("#pragma comment(lib,\"%s.lib\")\r\n\r\n"),
				   (LPCTSTR)m_strTitle, (LPCTSTR)m_strTitle);
	PrintObjects(&m_ObjectList, strText);

	return strText;
}

/*
namespace
{
 class
   function
 variant
 function
}
*/

CString ReplaceOnce(CString str, LPCTSTR lpFind, LPCTSTR lpReplace)
{
	int pos = str.Find(lpFind);
	if (pos == -1)
		return str;

	str.Delete(pos, lstrlen(lpFind));
	str.Insert(pos, lpReplace);

	return str;
}

VOID ReplaceTemplate(CString &str)
{
	str.Replace(_T("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >"), _T("std::string"));
	str.Replace(_T("class std::basic_string<wchar_t,struct std::char_traits<wchar_t>,class std::allocator<wchar_t> >"), _T("std::wstring"));

	str.Replace(_T("class ATL::CStringT<char,class ATL::StrTraitATL<char,class ATL::ChTraitsCRT<char> > >"), _T("CStringA"));
	str.Replace(_T("class ATL::CStringT<wchar_t,class ATL::StrTraitATL<wchar_t,class ATL::ChTraitsCRT<wchar_t> > >"), _T("CStringW"));

	str.Replace(_T("class ATL::CStringT<char,class StrTraitMFC<char,class ATL::ChTraitsCRT<char> > >"), _T("CStringA"));
	str.Replace(_T("class ATL::CStringT<wchar_t,class StrTraitMFC<wchar_t,class ATL::ChTraitsCRT<wchar_t> > >"), _T("CStringW"));
}

VOID CHacerExportarDlg::ParseClass(LPCTSTR lpName)
{
	/*
	public: class XLIB::Cttt & __thiscall XLIB::Cttt::operator=(class XLIB::Cttt const &)
	1	public: __thiscall XLIB::Cttt::Cttt(void)		000110be
	5	public: void __cdecl CTest::a(void)		00011195
	6	protected: void __thiscall CTest::d(void)		0001101e
	7	private: void __thiscall CTest::e(void)		0001100a
	public: static void __cdecl CTest::f(void)		000110af

	12	public: void __thiscall `public: void __thiscall XLIB::XX::XX::ff(void)'::`2'::XX::e(void)		00011136
	13	public: void __thiscall `public: void __thiscall `public: void __thiscall XLIB::XX::XX::ff(void)'::`2'::XX::e(void)'::`4'::XX::e(void)		00011104

	1	public: class `public: void __thiscall CXX::f(void)'::`2'::CXX & __thiscall `public: void __thiscall CXX::f(void)'::`2'::CXX::operator=(class `public: void __thiscall CXX::f(void)'::`2'::CXX const &)		00011046
	2	public: class `public: void __thiscall `public: void __thiscall CXX::f(void)'::`2'::CXX::f2(void)'::`4'::CXX & __thiscall `public: void __thiscall `public: void __thiscall CXX::f(void)'::`2'::CXX::f2(void)'::`4'::CXX::operator=(class `public: void __thiscall		000110f0
	3	public: class CXX & __thiscall CXX::operator=(class CXX const &)		000110a0
	4	public: void __thiscall `public: void __thiscall CXX::f(void)'::`2'::CXX::f2(void)		00011023
	5	public: void __thiscall `public: void __thiscall `public: void __thiscall CXX::f(void)'::`2'::CXX::f2(void)'::`4'::CXX::f3(void)		00011159
	6	public: void __thiscall CXX::f(void)		0001100a
	*/
	CString strName = lpName;
	int protecttype = DetermineProtectionType(strName);
	strName.Trim();

	int pos = FindFunctionPosition(strName);
	if (pos == -1)
	{
		AddUnParser(lpName);
		return;
	}

	CString strArgu = ExtractArguments(strName, pos);
	CString strType, strObject;
	ExtractTypeAndObject(strName, pos, strType, strObject);

	LPOBJECTITEM item = ProcessNamespaces(strType, strObject, strArgu);
	if (item == nullptr)
	{
		AddUnParser(lpName);
		return;
	}

	CleanTypeString(strType);

	FormatFinalName(strName, strType, strObject, strArgu);

	RemoveDefaultFunctions(strName, item);

	ReplaceTemplate(strName);

	item->type = OBJECT_TYPE_class;
	item = ProbeForWriteObject(strName, item, protecttype);
	item->type = OBJECT_TYPE_classfun;
}

int CHacerExportarDlg::DetermineProtectionType(CString &name)
{
	struct Protection
	{
		LPCTSTR token;
		int type;
	};

	Protection protections[] = {
		{_T("public:"), 1},
		{_T("protected:"), 2},
		{_T("private:"), 3}};

	// public: void __thiscall CEcdsa::`default constructor closure'(void)
	// public: __thiscall CTXBSTR::operator wchar_t *(void)const
	//	if( strName == _T("public: void __thiscall CEcdsa::`default constructor closure'(void)"))__asm int 3
	for (const auto &protection : protections)
	{
		if (name.Find(protection.token) == 0)
		{
			name.Replace(protection.token, _T(""));
			return protection.type;
		}
	}

	return 0;
}

int CHacerExportarDlg::FindFunctionPosition(const CString &name)
{
	int pos = name.ReverseFind(_T('('));
	if (pos == -1)
		return -1;

	// Additional checks can be added here if needed
	return pos;
}

CString CHacerExportarDlg::ExtractArguments(CString &name, int pos)
{
	CString arguments = name.Right(name.GetLength() - pos);
	name = name.Left(pos);
	return arguments;
}

VOID CHacerExportarDlg::ExtractTypeAndObject(CString &name, int pos, CString &type, CString &object)
{
	CString temp;
	int spacePos = FindSpaceBeforeOperator(name, pos);
	if (spacePos == -1)
		return;

	object = name.Right(name.GetLength() - spacePos - 1);
	type = name.Left(spacePos);
}

int CHacerExportarDlg::FindSpaceBeforeOperator(const CString &name, int pos)
{
	CString temp = name.Left(pos);
	int spacePos = temp.ReverseFind(_T(' '));
	return spacePos;
}

LPOBJECTITEM CHacerExportarDlg::ProcessNamespaces(CString &type, CString &object, CString &arguments)
{
	int pos = object.Find(_T("::"));
	LPOBJECTITEM item = nullptr;

	while (pos != -1)
	{
		pos += 2;
		CString strNamespace = object.Left(pos);
		object = object.Right(object.GetLength() - pos);
		CleanNamespace(strNamespace, type, arguments);
		item = ProbeForWriteObject(strNamespace.Left(strNamespace.GetLength() - 2), item);
		pos = object.Find(_T("::"));
	}

	return item;
}

VOID CHacerExportarDlg::CleanNamespace(CString &namespaceName, CString &type, CString &arguments)
{
	type = ReplaceOnce(type, namespaceName, _T(""));
	arguments = ReplaceOnce(arguments, namespaceName, _T(""));
	namespaceName = namespaceName.Left(namespaceName.GetLength() - 2);
}

VOID CHacerExportarDlg::CleanTypeString(CString &type)
{
	type.Replace(_T("__thiscall"), _T(""));
	if (m_bRemoveCCall)
		type.Replace(_T("__cdecl"), _T(""));
	type.Trim();
}

VOID CHacerExportarDlg::FormatFinalName(CString &name, const CString &type, const CString &object, const CString &arguments)
{
	if (type.GetLength())
	{
		name.Format(_T("%s\t%s%s"), static_cast<LPCTSTR>(type), static_cast<LPCTSTR>(object), static_cast<LPCTSTR>(arguments));
	}
	else
	{
		name.Format(_T("%s%s"), static_cast<LPCTSTR>(object), static_cast<LPCTSTR>(arguments));
	}
}

VOID CHacerExportarDlg::RemoveDefaultFunctions(CString &name, LPOBJECTITEM item)
{
	CString DefaultFun;
	// class CXX &	operator=(class CXX const &); Remove the default equal sign
	DefaultFun.Format(_T("class %s &\toperator=(class %s const &)"), static_cast<LPCTSTR>(item->name), static_cast<LPCTSTR>(item->name));
	if (DefaultFun == name)
		return; // Remove the default equal sign

	DefaultFun.Format(_T("%s(class %s const &)"), static_cast<LPCTSTR>(item->name), static_cast<LPCTSTR>(item->name));
	if (DefaultFun == name)
	{
		name.Format(_T("%s(void)"), static_cast<LPCTSTR>(item->name));
	}

	if (name.Find(_T('`')) != -1)
	{
		DefaultFun = _T("//") + name;
		name = DefaultFun;
	}
}

VOID CHacerExportarDlg::ParserVariant(LPCTSTR lpName)
{
	CString strName;
	int pos;
	CString strObject;
	CString strNamespace;
	CString strType;
	LPOBJECTITEM item = 0;
	strName = lpName;

	pos = strName.ReverseFind(_T(' '));
	if (-1 == pos)
	{
		AddUnParser(lpName);
		return;
	}

	strObject = strName.Right(strName.GetLength() - pos - 1);
	strType = strName.Left(pos);

	pos = strObject.Find(_T("::"));
	while (-1 != pos)
	{
		pos += 2;
		strNamespace = strObject.Left(pos);
		strObject = strObject.Right(strObject.GetLength() - pos);
		strType.Replace(strNamespace, _T(""));
		strNamespace = strNamespace.Left(strNamespace.GetLength() - 2);
		item = ProbeForWriteObject(strNamespace, item);
		pos = strObject.Find(_T("::"));
	}

	strName.Format(_T("%s\t%s"), (LPCTSTR)strType, (LPCTSTR)strObject);
	if (_T("const\t`vftable'") == strName)
		return; // virtual function table

	if (-1 != strName.Find(_T('`')))
	{
		CString temp = _T("//");
		temp += strName;
		strName = temp;
	}

	ReplaceTemplate(strName);

	item = ProbeForWriteObject(strName, item);
	item->type = OBJECT_TYPE_variant;
	// int XLIB::nttt
	// class XLIB::Cttt XLIB::aa		0001813c
}

VOID CHacerExportarDlg::ParserFunction(LPCTSTR lpName)
{
	CString functionName = lpName;
	int parenthesisPos = functionName.ReverseFind(_T('('));

	if (parenthesisPos == -1)
	{
		AddUnParser(lpName);
		return;
	}

	CString arguments = functionName.Right(functionName.GetLength() - parenthesisPos);
	functionName = functionName.Left(parenthesisPos);

	int spacePos = functionName.ReverseFind(_T(' '));
	if (spacePos == -1)
	{
		AddUnParser(lpName);
		return;
	}

	CString objectName = functionName.Right(functionName.GetLength() - spacePos - 1);
	CString type = functionName.Left(spacePos);

	CString namespacePrefix;
	LPOBJECTITEM item = 0;
	int namespacePos = objectName.Find(_T("::"));
	while (namespacePos != -1)
	{
		namespacePos += 2;
		namespacePrefix = objectName.Left(namespacePos);
		objectName = objectName.Right(objectName.GetLength() - namespacePos);
		type = ReplaceOnce(type, namespacePrefix, _T(""));
		arguments = ReplaceOnce(arguments, namespacePrefix, _T(""));
		namespacePrefix = namespacePrefix.Left(namespacePrefix.GetLength() - 2);
		item = ProbeForWriteObject(namespacePrefix, item);
		namespacePos = objectName.Find(_T("::"));
	}

	if (m_bRemoveCCall)
	{
		type.Replace(_T("__cdecl"), _T(""));
	}

	functionName.Format(_T("%s\t%s%s"), (LPCTSTR)type, (LPCTSTR)objectName, (LPCTSTR)arguments);

	if (functionName.Find(_T('`')) != -1)
	{
		functionName = _T("//") + functionName;
	}

	ReplaceTemplate(functionName);

	item = ProbeForWriteObject(functionName, item);
	item->type = OBJECT_TYPE_function;
}

VOID CHacerExportarDlg::AddUnParser(LPCTSTR lpname)
{
	LPOBJECTITEM item = 0;
	CString strTemp;

	strTemp = _T(" ");
	strTemp += lpname;
	item = ProbeForWriteObject(strTemp, item);
	item->type = OBJECT_TYPE_unknown;
}

LPOBJECTITEM CHacerExportarDlg::ProbeForWriteObject(LPCTSTR name, LPOBJECTITEM parent, int itemindex)
{
	LPOBJECTITEM lpObject;

	if (0 == name || lstrlen(name) == 0)
		return &m_ObjectList;
	if (0 == parent)
		parent = &m_ObjectList;

	lpObject = parent->items[itemindex].Find(name);

	if (0 == lpObject)
	{
		OBJECTITEM item;
		item.name = name;
		item.type = OBJECT_TYPE_namespace;
		parent->items[itemindex].Insert(name, item);
	}

	lpObject = parent->items[itemindex].Find(name);
	return lpObject;
}

void CHacerExportarDlg::OnBnClickedButton1()
{
	try
	{
		CString strFile;
		CStringA strText;
		CFile file;

		// Write def file
		strFile.Format(_T("%s.def"), static_cast<LPCTSTR>(m_strTitle));
		file.Open(strFile, CFile::modeCreate | CFile::modeWrite);
		CStringA strTextA = CStringA(ShowAsDef());
		file.Write(strTextA, strTextA.GetLength());
		file.Close();

		// Write header file
		strFile.Format(_T("%s.h"), static_cast<LPCTSTR>(m_strTitle));
		file.Open(strFile, CFile::modeCreate | CFile::modeWrite);
		strTextA = CStringA(ShowAsSource());
		file.Write(strTextA, strTextA.GetLength());
		file.Close();

		// Link
		strFile.Format(_T("link /LIB /DEF:%s.def /OUT:%s.lib"), static_cast<LPCTSTR>(m_strTitle), static_cast<LPCTSTR>(m_strTitle));
		strText = strFile;
		WinExec(strText, SW_SHOW);
	}
	catch (CFileException *e)
	{
		e->ReportError();
		e->Delete();
	}
}

void CHacerExportarDlg::OnBnClickedNormal()
{
	// TODO: Add your control notification handler code here
	CButton *pCheckBox = (CButton *)GetDlgItem(IDC_CHECK3);
	pCheckBox->EnableWindow(FALSE);
}

void CHacerExportarDlg::OnBnClickedDef()
{
	// TODO: Add your control notification handler code here
	CButton *pCheckBox = (CButton *)GetDlgItem(IDC_CHECK3);
	pCheckBox->EnableWindow(TRUE);
}

void CHacerExportarDlg::OnBnClickedSrc()
{
	// TODO: Add your control notification handler code here
	CButton *pCheckBox = (CButton *)GetDlgItem(IDC_CHECK3);
	pCheckBox->EnableWindow(FALSE);
}
