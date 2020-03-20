
// com-toolDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "com-tool.h"
#include "com-toolDlg.h"
#include "afxdialogex.h"
#include <sstream>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_ALWAYS_SEND_TIMER WM_USER + 1000


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CcomtoolDlg dialog



CcomtoolDlg::CcomtoolDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMTOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CcomtoolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CcomtoolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_OPEN, &CcomtoolDlg::OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_TEST, &CcomtoolDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CcomtoolDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_SEND, &CcomtoolDlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_CHECK_ALWAYSSEND, &CcomtoolDlg::OnBnClickedCheckAlwaysSend)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CcomtoolDlg::OnBnClickedBtnClear)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CcomtoolDlg message handlers

BOOL CcomtoolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	GetDlgItem(IDC_EDIT_SEND)->SetWindowText(TEXT("7D 04 11 00 00 12 66 D6 7E"));

    if (!CEnumerateSerial::UsingRegistry(m_coms))
	{
		CEnumerateSerial::CPortsArray ports;
		if (!CEnumerateSerial::UsingQueryDosDevice(ports))
		{
			CEnumerateSerial::UsingCreateFile(ports);
		}

		for (auto item : ports)
		{
			std::wstringstream ss;
			ss << L"COM" << item;
			m_coms.push_back(ss.str());
		}
    }

    CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_COMLIST);
	for (auto& item: m_coms)
    {
        combo->InsertString(-1, item.c_str());
	}

    combo->SetCurSel(0);

	((CButton*)GetDlgItem(IDC_CHECK_HEX_DISPLAY))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHECK_HEX_SEND))->SetCheck(1);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CcomtoolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CcomtoolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CcomtoolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CcomtoolDlg::OpenSerialPort(CString strCom)
{
	strCom.Format(L"\\\\.\\%s", strCom);
	m_port = CreateFile(strCom, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (m_port == INVALID_HANDLE_VALUE) {
		m_port = NULL;
		if (m_test_model) {
			CString strError;
			strError.Format(L"打开%s失败...\r\n", strCom.Right(strCom.GetLength() - 4));
			AddContent(strError);
		}
		return FALSE;
	}

	if (m_test_model) {
		CString strInfo;
		strInfo.Format(L"打开%s成功...\r\n", strCom.Right(strCom.GetLength() - 4));
		AddContent(strInfo);
	}

	m_read_thread = std::move(std::thread(std::bind(&CcomtoolDlg::ReadThread, this)));

	if (!m_test_model) {
		GetDlgItem(IDC_BTN_OPEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_TEST)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CLOSE)->EnableWindow();
		GetDlgItem(IDC_CHECK_ALWAYSSEND)->EnableWindow();
		GetDlgItem(IDC_BTN_SEND)->EnableWindow();
	}
	OnBnClickedCheckAlwaysSend();

	m_read_thread.detach();

	return TRUE;
}

void CcomtoolDlg::OnBnClickedBtnOpen()
{
	OnBnClickedBtnClear();

	CString strCom;
	GetDlgItemText(IDC_COMBO_COMLIST, strCom);
	OpenSerialPort(strCom);
}

void CcomtoolDlg::OnBnClickedBtnClose()
{
	CloseHandle(m_port);
	m_port = NULL;
	Sleep(100);
	if (!m_test_model)
	{
		GetDlgItem(IDC_BTN_OPEN)->EnableWindow();
		GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_TEST)->EnableWindow();
		GetDlgItem(IDC_CHECK_ALWAYSSEND)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SEND)->EnableWindow(FALSE);
	}
}

void CcomtoolDlg::OnBnClickedBtnTest()
{
	m_test_thread = std::move(std::thread(std::bind(&CcomtoolDlg::TestThread, this)));
	m_test_thread.detach();
}

void CcomtoolDlg::TestThread()
{
	m_test_model = true;
	OnBnClickedBtnClear();
	AddContent(L"开始串口测试...\r\n");

	GetDlgItem(IDC_BTN_OPEN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_TEST)->EnableWindow(FALSE);

	for (auto& com : m_coms)
	{
		if (OpenSerialPort(com.c_str()))
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			OnBnClickedBtnClose();
		}
	}

	GetDlgItem(IDC_BTN_OPEN)->EnableWindow();
	GetDlgItem(IDC_BTN_TEST)->EnableWindow();
	AddContent(L"串口测试完成...\r\n");
	m_test_model = false;
}


void CcomtoolDlg::AddContent(CString strMsg, DWORD size)
{
	CString strContent;
	GetDlgItemText(IDC_EDIT_RECV, strContent);
	strContent += strMsg;
	SetDlgItemText(IDC_EDIT_RECV, strContent);
	if (!m_test_model) 
	{
		SetDlgItemInt(IDC_RECV_NUM, GetDlgItemInt(IDC_RECV_NUM, NULL, FALSE) + size, FALSE);
	}

	CEdit* recvEdit = (CEdit*)GetDlgItem(IDC_EDIT_RECV);
	recvEdit->LineScroll(recvEdit->GetLineCount());
}


void CcomtoolDlg::OnBnClickedBtnSend()
{
	DWORD dwWrited;
	CString strSendMsg;
	GetDlgItemText(IDC_EDIT_SEND, strSendMsg);
	if (((CButton*)GetDlgItem(IDC_CHECK_HEX_SEND))->GetCheck()) {
		std::vector<BYTE> data;
		Hex2Data(strSendMsg, data);
		WriteFile(m_port, &data[0], data.size(), &dwWrited, NULL);
	}
	else 
	{
		WriteFile(m_port, strSendMsg.GetBuffer(strSendMsg.GetLength()), strSendMsg.GetLength(), &dwWrited, NULL);
		strSendMsg.ReleaseBuffer();
	}
}


void CcomtoolDlg::OnBnClickedCheckAlwaysSend()
{
	int state = ((CButton*)GetDlgItem(IDC_CHECK_ALWAYSSEND))->GetCheck();
	if (state) 
	{
		GetDlgItem(IDC_BTN_SEND)->EnableWindow(FALSE);
		SetTimer(WM_ALWAYS_SEND_TIMER, 1000, NULL);
	}
	else 
	{
		GetDlgItem(IDC_BTN_SEND)->EnableWindow();
		KillTimer(WM_ALWAYS_SEND_TIMER);
	}
}

int GetHexValue(char hexchar) 
{
	int value = 0;
	if (hexchar >= '0' && hexchar <= '9')
	{
		value = hexchar - '0';
	}
	else if (hexchar >= 'A' && hexchar <= 'Z') 
	{
		value = hexchar - 'A' + 10;
	}

	return value;
}

void CcomtoolDlg::Hex2Data(const CString& str, std::vector<BYTE>& data)
{
	CString strTmp = str;
	strTmp.Replace(L" ", L"");
	strTmp.Replace(L"\r", L"");
	strTmp.Replace(L"\n", L"");
	strTmp.Replace(L"\t", L"");
	strTmp.MakeUpper();

	for (int i = 2; i <= strTmp.GetLength(); i += 2)
	{
		data.push_back(GetHexValue(strTmp.GetAt(i - 2)) * 16 + GetHexValue(strTmp.GetAt(i - 1)));
	}

}

void CcomtoolDlg::Data2Hex(CString& str, const std::vector<BYTE>& data)
{
	CString tmpChar;
	for (int i = 0; i < data.size(); ++i)
	{
		tmpChar.Format(L"%02X ", data[i]);
		str.Format(L"%s%s", str, tmpChar);
	}
}

void CcomtoolDlg::ReadThread()
{
	std::vector<BYTE> readData;
	std::vector<BYTE> data;
	readData.resize(4096);
	DWORD dwReaded;
	CString strData;
	while (m_port)
	{
		ReadFile(m_port, &readData[0], 4095, &dwReaded, NULL);
		if (dwReaded > 0) 
		{
			data.resize(dwReaded);
			std::copy(readData.begin(), readData.begin() + dwReaded, data.begin());
			if (((CButton*)GetDlgItem(IDC_CHECK_HEX_DISPLAY))->GetCheck()) 
			{
				Data2Hex(strData, data);
			}
			else 
			{
				strData = CString(&data[0]);
			}
			if(m_test_model)
			{
				AddContent(L"收到数据，通信成功...\r\n");
			}
			else 
			{
				AddContent(strData, dwReaded);
			}
		} 
		else 
		{
			Sleep(200);
		}
		strData.Empty();
	}
}


void CcomtoolDlg::OnBnClickedBtnClear()
{
	SetDlgItemText(IDC_EDIT_RECV, L""); 
	SetDlgItemInt(IDC_RECV_NUM, 0, FALSE);
}


void CcomtoolDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case WM_ALWAYS_SEND_TIMER:
		OnBnClickedBtnSend();
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}
