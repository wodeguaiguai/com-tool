
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

CcomtoolDlg::~CcomtoolDlg()
{
	if (m_hSerialComm)
	{
		CloseHandle(m_hSerialComm);
		m_hSerialComm = NULL;
	}
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

	((CComboBox*)GetDlgItem(IDC_COMBO_BANDRATE))->SetCurSel(5);
	((CButton*)GetDlgItem(IDC_CHECK_HEX_DISPLAY))->SetCheck(1);
    ((CButton*)GetDlgItem(IDC_CHECK_HEX_SEND))->SetCheck(1);
    ((CButton*)GetDlgItem(IDC_CHECK_ALWAYSSEND))->SetCheck(1);

	SetForegroundWindow();

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
	m_hSerialComm = CreateFile(strCom, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (m_hSerialComm == INVALID_HANDLE_VALUE) {
		m_hSerialComm = NULL;
		if (m_test_model) {
			CString strError;
			strError.Format(L"打开%s失败 %d...\r\n", strCom.Right(strCom.GetLength() - 4), GetLastError());
			AddContent(strError);
		}
		return FALSE;
	}

	if (m_test_model) {
		CString strInfo;
		strInfo.Format(L"打开%s成功...\r\n", strCom.Right(strCom.GetLength() - 4));
		AddContent(strInfo);
	}

	// config com
	DCB dcbConfig;
	SecureZeroMemory(&dcbConfig, sizeof(DCB));
	dcbConfig.DCBlength = sizeof(DCB);
    if (GetCommState(m_hSerialComm, &dcbConfig))
    {
        CString strBaudRate;
        GetDlgItemText(IDC_COMBO_BANDRATE, strBaudRate);
		dcbConfig.fBinary = TRUE;
		dcbConfig.fNull = FALSE;
		dcbConfig.fAbortOnError = FALSE;
		dcbConfig.BaudRate = _ttoi(strBaudRate);
		dcbConfig.ByteSize = 8;
		dcbConfig.fOutxCtsFlow = FALSE;
		dcbConfig.fOutxDsrFlow = FALSE;
		dcbConfig.fDtrControl = DTR_CONTROL_DISABLE;
		dcbConfig.fDsrSensitivity = FALSE;
		dcbConfig.fOutX = FALSE;
		dcbConfig.fInX = FALSE;
		dcbConfig.fRtsControl = RTS_CONTROL_DISABLE;
		dcbConfig.fParity = FALSE; // No parity
		dcbConfig.Parity = NOPARITY;
		dcbConfig.StopBits = ONESTOPBIT; // One stop bit
    }
	SetCommState(m_hSerialComm, &dcbConfig);

	::COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 1;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(m_hSerialComm, &timeouts);

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
    SetCommMask(m_hSerialComm, EV_BREAK);
	CloseHandle(m_hSerialComm);
	m_hSerialComm = NULL;

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
	CString strMsg;
	strMsg.Format(L"第%d次测试开始...\r\n", m_test_times);
	AddContent(strMsg);

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
    strMsg.Format(L"第%d次测试完成...\r\n", m_test_times);
    AddContent(strMsg);
    AddContent(L"\r\n============================================\r\n\r\n");
	m_test_model = false;

	if (((CButton*)GetDlgItem(IDC_CHECK_ALWAYS_TEST))->GetCheck()) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
		m_test_times++;
		OnBnClickedBtnTest();
	}
    else {
        m_test_times = 1;
	}
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
	OVERLAPPED ov;
	ZeroMemory(&ov, sizeof(ov));
	ov.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	BOOL bRet;
	if (((CButton*)GetDlgItem(IDC_CHECK_HEX_SEND))->GetCheck()) {
		std::vector<BYTE> data;
		Hex2Data(strSendMsg, data);
		bRet = WriteFile(m_hSerialComm, &data[0], data.size(), &dwWrited, &ov);
	}
	else 
	{
		bRet = WriteFile(m_hSerialComm, strSendMsg.GetBuffer(strSendMsg.GetLength()), strSendMsg.GetLength(), &dwWrited, &ov);
		strSendMsg.ReleaseBuffer();
	}

    if (!bRet) {
        if (GetLastError() == ERROR_IO_PENDING) {
            if (!GetOverlappedResult(m_hSerialComm, &ov, &dwWrited, TRUE)) {
				return;
            }
        }
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
    DWORD dwEventMask;

	SetCommMask(m_hSerialComm, EV_RXCHAR);

    OVERLAPPED ov; 
	ZeroMemory(&ov, sizeof(ov));
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    while (m_hSerialComm) 
	{
        if (!WaitCommEvent(m_hSerialComm, &dwEventMask, &ov)) {
            if (GetLastError() == ERROR_IO_PENDING) {
                if (!GetOverlappedResult(m_hSerialComm, &ov, &dwReaded, TRUE)) {
					break;
                }
            }
            else {
                break;
            }
        }

        if (dwEventMask & EV_RXCHAR)
        {
            do
            {
				dwReaded = 0;
                if(!ReadFile(m_hSerialComm, &readData[0], 4095, &dwReaded, &ov))
                {
                    if (GetLastError() == ERROR_IO_PENDING) {
                        if (!GetOverlappedResult(m_hSerialComm, &ov, &dwReaded, TRUE)) {
							break;
                        }
                    }
                    else {
                        break;
                    }
				}
                if (dwReaded)
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
                    if (m_test_model)
                    {
                        AddContent(L"收到数据，通信成功...\r\n");
                    }
                    else
                    {
                        AddContent(strData, dwReaded);
                    }
                }
                strData.Empty();
            } while (dwReaded > 0);
        }
    }

	CloseHandle(ov.hEvent);
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
