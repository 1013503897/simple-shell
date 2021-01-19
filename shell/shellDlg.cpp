
// shellDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "shell.h"
#include "shellDlg.h"
#include "afxdialogex.h"
#include <Windows.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CshellDlg dialog



CshellDlg::CshellDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SHELL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CshellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CshellDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CshellDlg message handlers

BOOL CshellDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		


	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = 0;
	sa.bInheritHandle = true;

	if (!CreatePipe(&hReadPipe1, &hWritePipe1, &sa, 0) || !CreatePipe(&hReadPipe2, &hWritePipe2, &sa, 0))
	{
		return -1;
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = hReadPipe2;
	si.hStdOutput = si.hStdError = hWritePipe1;

	TCHAR cmdLine[256] = { 0 };
	GetSystemDirectory(cmdLine, sizeof(cmdLine));
	_tcscat(cmdLine, _T("\\cmd.exe"));

	PROCESS_INFORMATION ProcessInformation;
	if (!CreateProcess(cmdLine, NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &ProcessInformation))
	{
		return -1;
	}

	_beginthreadex(NULL, 0, ShellThread, this, 0, NULL);

	return TRUE;  
}

void CshellDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CshellDlg::OnPaint()
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
HCURSOR CshellDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CshellDlg::RecvCommand(CString tip)
{
	CEdit* edit_cmd = (CEdit*)GetDlgItem(IDC_COMMAND);
	edit_cmd->GetWindowText(m_log);
	if (m_log != "")
		m_log += "\r\n";
	m_log += tip;
	edit_cmd->SetWindowText(m_log);
	edit_cmd->SetSel(m_log.GetLength(), m_log.GetLength());
}

void CshellDlg::SendCommand()
{
	DWORD lBytesWrite;
	CEdit* edit_cmd = (CEdit*)GetDlgItem(IDC_COMMAND);
	CString new_log;
	edit_cmd->GetWindowText(new_log);
	new_log += '\n';
	int cmd_length = new_log.GetLength() - m_log.GetLength();
	_tcscpy(sendBuff, new_log.Right(cmd_length));
	WriteFile(hWritePipe2, sendBuff, _tcslen(sendBuff), &lBytesWrite, 0);
}


unsigned int WINAPI CshellDlg::ShellThread(void* param)
{
	int ret;
	unsigned long lBytesRead, lBytesWrite;
	CshellDlg* _this = (CshellDlg*)param;
	while (TRUE) {
		lBytesRead = 0;
		Sleep(50);
		ret = PeekNamedPipe(_this->hReadPipe1, _this->Buff, SEND_BUFF_SIZE, &lBytesRead, 0, 0);
		if (lBytesRead)
		{
			ret = ReadFile(_this->hReadPipe1, _this->Buff, SEND_BUFF_SIZE, &lBytesRead, 0);

			char* tmp = _this->Buff;
			while (*tmp != '\n')tmp++;
			if (ret)
			{
				_this->RecvCommand(tmp);
				ZeroMemory(_this->Buff, sizeof(_this->Buff));
			}
		}
	}
}

BOOL CshellDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) 
	{
		CEdit* edit_cmd = (CEdit*)GetDlgItem(IDC_COMMAND);
		if (GetFocus() == edit_cmd)
		{

			DWORD selectedRegion = edit_cmd->GetSel();
			int selectedStart = LOWORD(selectedRegion);
			int selectedEnd = HIWORD(selectedRegion);

			if (selectedStart != selectedEnd && selectedStart < m_log.GetLength())
				return true;

			if (pMsg->wParam == 8 && selectedStart <= m_log.GetLength()) 
				return true;

			if (pMsg->wParam == 13 || pMsg->wParam == 32) 
			{
				SendCommand();
			}

		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
