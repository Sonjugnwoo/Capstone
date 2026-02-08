
// CapstoneDlg.h: 헤더 파일
//

#pragma once


// CCapstoneDlg 대화 상자
class CCapstoneDlg : public CDialogEx
{
private:
	CString m_strHome;                          // 실행파일 경로
	std::vector<CString> m_vecClassName;        // 클래스명 목록
	int detectedtime = 0;                       // 감지 카운터
// 생성입니다.
public:
	CCapstoneDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

	BOOL LoadClassName(const CString strFilePath);	    // 클래스 이름 로드 함수 (txt 파일에서 클래스명 읽기)

	// ****************************************************************
	// OpenCV Mat ↔ CDib 형식 변환 함수들
	// ****************************************************************
	CDib* Mat2CDib(cv::Mat& Image);         // cv::Mat 이미지를 CDib 객체로 변환
	cv::Mat CDib2Mat(CDib* pDib);           // CDib 객체를 cv::Mat 이미지로 변환
	unsigned char* Mat2Byte(cv::Mat& Image); // cv::Mat을 바이트 배열로 변환 (메모리 버퍼)

	// ****************************************************************
	// YOLO 객체 감지 핵심 함수들
	// ****************************************************************
	HMODULE m_hDLL;                                 // DLL 로딩 핸들 (YOLO DLL 동적 로드)
	BOOL InitializeYOLODetector();					 // YOLO 초기화 (DLL + 모델 + 클래스 로드)
	int DETECTandDISPLAYsingleFRAME(cv::Mat image);        // 단일 프레임에서 객체 감지 + 결과 표시
	void DrawDetectionResults(std::vector<bbox_t>& results, cv::Mat& image);  // 감지 결과 바운딩 박스 그리기

	// ****************************************************************
	// 영상 처리 관련 함수들
	// ****************************************************************
	cv::VideoCapture m_cap;                         // OpenCV 비디오/웹캠 캡처 객체
	cv::Mat LoadImageFile();                // 이미지 파일 로드
	void VideoStream();                     // 비디오 스트림 처리 (실시간 영상)
	void ShowImage();							// 이미지 출력
	void ShowVideo();							// 비디오 출력
	// ****************************************************************
	// 스레드(Thread) for Video Display - UI 멈추지 않게 별도 스레드에서 영상 출력
	// ****************************************************************
	CString GetVideoFilePathName();         // 비디오 파일 전체 경로 반환

	CWinThread* m_pThread;                  // 비디오 표시용 MFC 스레드 객체 포인터
	BOOL m_bGoTHread;                       // 스레드 구동/정지 플래그 (TRUE=실행, FALSE=정지)

	static UINT BatchProcessingByThreadMode(LPVOID lParam);  // 스레드 배치 처리

	// ****************************************************************
	// 메일 관련 함수들 
	// ****************************************************************
	PWSTR fCStringToPWSTRcasting(CString str);  // CString을 PWSTR(WCHAR*)로 변환 (Windows API용)
	void SendKickboardAlert(const cv::Mat& image);         // 증거 이미지 저장 + 자동 메일 발송
	CString fSystemTimeGet();                              // 현재 시각 문자열 반환 (파일명 타임스탬프용)

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAPSTONE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	CString m_str_message;                  // 상태 메시지 표시용 문자열 변수
	CStatic m_picture;                      // 이미지/영상 출력용 Static 컨트롤

	// ****************************************************************
	// 버튼 클릭 이벤트 핸들러들
	// ****************************************************************
	afx_msg void OnBnClickedButtonImage();  // "이미지" 버튼 클릭 - 단일 이미지 처리
	afx_msg void OnBnClickedButtonVideo();  // "비디오" 버튼 클릭 - 비디오 스트림 시작
	afx_msg void OnBnClickedButtonClose();  // "종료" 버튼 클릭 - 프로그램 종료


};
