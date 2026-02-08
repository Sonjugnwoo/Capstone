
// CapstoneDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Capstone.h"
#include "CapstoneDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCapstoneDlg 대화 상자



CCapstoneDlg::CCapstoneDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CAPSTONE_DIALOG, pParent)
	, m_str_message(_T(""))
{
	m_bGoTHread = false;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCapstoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_MESSAGE, m_str_message);
	DDX_Control(pDX, IDC_PICTURE, m_picture);
}

BEGIN_MESSAGE_MAP(CCapstoneDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BUTTON_IMAGE, &CCapstoneDlg::OnBnClickedButtonImage)
	ON_BN_CLICKED(ID_BUTTON_VIDEO, &CCapstoneDlg::OnBnClickedButtonVideo)
	ON_BN_CLICKED(ID_BUTTON_CLOSE, &CCapstoneDlg::OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CCapstoneDlg 메시지 처리기

BOOL CCapstoneDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	if (!InitializeYOLODetector())	// YOLO 초기화 + 실패시 프로그램 종료
		return FALSE;
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CCapstoneDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ============================================================
// 단일 이미지 로드 → YOLO 감지 → 화면 출력
// ============================================================
void CCapstoneDlg::ShowImage() {
	// =====================================================
	// 이미지 파일 선택 및 OpenCV 로드
	// =====================================================
	cv::Mat image = LoadImageFile();                   // MFC 파일대화상자 → cv::imread()
	if (image.empty()) return;                         // 취소/로드실패시 조기 종료

	// =====================================================
	// YOLO 감지 + 시각화 + 화면 출력 (원스톱)
	// =====================================================
	int cnt = DETECTandDISPLAYsingleFRAME(image);      // 
													   // ① Mat2Byte() → DLL detect → DrawDetectionResults()
													   // ② Mat2CDib() → m_picture 출력 (전체 처리 완료!)

	// =====================================================
	// 실시간 상태 메시지 갱신
	// =====================================================
	m_str_message.Format(L"감지완료: %d개 (w=%d, h=%d)",
		cnt,                           // 감지된 킥보드/사람 개수
		image.cols,                    // 이미지 너비 (픽셀)
		image.rows);                   // 이미지 높이 (픽셀)

	UpdateData(false);                                 // CString → UI Edit 컨트롤 동기화
													   // (false=UI→변수 복사 안함)
}

// ============================================================
// 비디오 스레드 토글 제어 (시작/일시정지)
// ============================================================
void CCapstoneDlg::ShowVideo() {
	if (m_bGoTHread == false) {
		// =====================================================
		// 스레드 시작 모드
		// =====================================================
		m_pThread = AfxBeginThread(BatchProcessingByThreadMode, this);  // 새 스레드 생성
		m_bGoTHread = true;                                            // 실행 플래그 ON
	}
	else {
		// =====================================================
		// 스레드 일시정지 모드
		// =====================================================
		m_pThread->SuspendThread();                                    // 현재 실행중인 스레드 강제 정지
		m_bGoTHread = false;                                           // 플래그 OFF (다음 시작시 새 스레드)
	}
}
// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CCapstoneDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCapstoneDlg::OnBnClickedButtonImage()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ShowImage();					
}


void CCapstoneDlg::OnBnClickedButtonVideo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ShowVideo();
}


void CCapstoneDlg::OnBnClickedButtonClose()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// =====================================================
	// 비디오 스레드 안전 정지 (메모리 누수 방지)
	// =====================================================
	m_bGoTHread = false;                           // while(m_bGoTHread) 루프 종료 신호

	if (m_pThread) {                               // 스레드 실행중인 경우
		WaitForSingleObject(m_pThread->m_hThread, 2000);  // 최대 2초 대기
		delete m_pThread;                          // 스레드 핸들 해제
		m_pThread = NULL;
	}
	// =====================================================
	// YOLO DLL 메모리 해제
	// =====================================================
	if (m_hDLL) {
		FreeLibrary(m_hDLL);                       // DLL 언로드 (GPU 메모리 반납)
		m_hDLL = NULL;
	}
	// =====================================================
	// OpenCV VideoCapture 정리
	// =====================================================
	if (m_cap.isOpened()) {
		m_cap.release();                           // 카메라/비디오 리소스 해제
	}
	// =====================================================
	// MFC 표준 종료 
	// =====================================================
	CDialog::OnOK();                               // CDialogEx::OnOK()도 동일
}
