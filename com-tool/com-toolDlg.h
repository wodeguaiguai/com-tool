
// com-toolDlg.h : header file
//

#pragma once
#include "enumser.h"


// CcomtoolDlg dialog
class CcomtoolDlg : public CDialogEx
{
// Construction
public:
	CcomtoolDlg(CWnd* pParent = nullptr);	// standard constructor

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
    afx_msg void OnBnClickedBtnTest();

private:
	CEnumerateSerial::CNamesArray m_names;
};
