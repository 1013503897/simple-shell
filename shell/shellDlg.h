
// shellDlg.h : header file
//

#pragma once

#define SEND_BUFF_SIZE 1024

// CshellDlg dialog
class CshellDlg : public CDialogEx
{
// Construction
public:
	CshellDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHELL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CString m_log;
	static unsigned int WINAPI ShellThread(void*);
	void SendCommand();
	HANDLE hReadPipe1, hWritePipe1, hReadPipe2, hWritePipe2;
	TCHAR Buff[SEND_BUFF_SIZE] = { 0 };
	TCHAR sendBuff[SEND_BUFF_SIZE] = _T("dir \n");
	// Generated message map functions
	virtual BOOL OnInitDialog();
	void RecvCommand(CString tip);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
