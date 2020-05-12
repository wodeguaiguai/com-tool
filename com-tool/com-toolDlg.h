
// com-toolDlg.h : header file
//

#pragma once
#include "enumser.h"
#include <thread>


// CcomtoolDlg dialog
class CcomtoolDlg : public CDialogEx
{
// Construction
public:
	CcomtoolDlg(CWnd* pParent = nullptr);	// standard constructor
	~CcomtoolDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMTOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOpen();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnBnClickedCheckAlwaysSend();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
	CEnumerateSerial::CNamesArray m_coms;
	HANDLE m_hSerialComm;
	std::thread m_read_thread, m_test_thread;
	bool m_test_model = false;
	DWORD m_test_times = 1;

private:
	BOOL OpenSerialPort(CString strCom);
	void AddContent(CString strMsg, DWORD size = 0);
	void Hex2Data(const CString& str, std::vector<BYTE>& data);
	void Data2Hex(CString& str, const std::vector<BYTE>& data);
	void ReadThread();
	void TestThread();
};
