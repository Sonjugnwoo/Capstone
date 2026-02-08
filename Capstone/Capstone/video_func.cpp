#include "pch.h"
#include "CapstoneDlg.h"
#include "afxdialogex.h"

// ============================================================
// 비디오 파일 경로 선택기
// ============================================================
CString CCapstoneDlg::GetVideoFilePathName(){
    CString strImagFilePath;                       // 반환용 파일 경로 문자열 (초기값: 빈 문자열)

    // =====================================================
    // MFC 파일 열기 대화상자 설정 (비디오 특화)
    // =====================================================
    CFileDialog dlg(
        TRUE,                                      // TRUE=파일 열기 모드
        _T("All(*.*)"),                            // 기본 확장자 (mp4 권장으로 변경 가능)
        _T("*.*"),                                 // 기본 파일명
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,    // 읽기전용 + 덮어쓰기 경고
        _T("All(*.*)|*.*|(*.mp4)|*.mp4|(*.mov)|*.mov|(*.avi)|*.avi|(*.mkv)|*.mkv|(*.wmv)|*.wmv|")  // 비디오 필터 강화
    );

    // =====================================================
    // 파일 선택 대화상자 실행
    // =====================================================
    if (dlg.DoModal() == IDOK){                     // 사용자가 "열기" 버튼 클릭
        strImagFilePath = dlg.GetPathName();       // 전체 경로 반환 (예: "C:\\video.mp4")

    }
    // 사용자가 "취소" 클릭시 → 빈 CString 반환

    return strImagFilePath;                        // VideoCapture 초기화용 경로 반환
}
// ============================================================
// 비디오 실시간 스트리밍 + YOLO 처리
// ============================================================
void CCapstoneDlg::VideoStream()
{
	// =====================================================
	// 비디오 파일 선택 및 OpenCV VideoCapture 초기화
	// =====================================================
	CString strVideoFileName = GetVideoFilePathName();  // MFC 파일 선택기 호출

	CT2CA pszConvertedAnsiString(strVideoFileName);    // CString → ANSI 변환
	std::string s(pszConvertedAnsiString);             // std::string으로 최종 변환
	cv::Mat frame;                                     // 프레임별 이미지 버퍼

	cv::VideoCapture cap(s);                           // OpenCV 비디오 리더 생성
	if (cap.isOpened() == false) {                     // 파일 열기 실패
		AfxMessageBox(L"Cannot open the video file");  // 경로/코덱/권한 문제
		return;
	}
	// =====================================================
	// 비디오 속성 정보 추출 (UI 상태 표시용)
	// =====================================================
	int w = cap.get(cv::CAP_PROP_FRAME_WIDTH);         // 프레임 너비
	int h = cap.get(cv::CAP_PROP_FRAME_HEIGHT);        // 프레임 높이
	double fps = cap.get(cv::CAP_PROP_FPS);            // FPS (초당 프레임수)
	int total_frames = cap.get(cv::CAP_PROP_FRAME_COUNT); // 전체 프레임수
	int count_frames = 0;                              // 현재 처리 프레임 카운터
	clock_t begin, end;                                // 타이밍 측정 (미사용)

	// =====================================================
	// 무한 루프: 프레임별 실시간 YOLO 처리
	// =====================================================
	while (true)
	{
		bool bSuccess = cap.read(frame);               // 다음 프레임 읽기
		if (bSuccess == false) {                       // 비디오 끝 또는 읽기 실패
			break;                                     // 루프 종료
		}

		// YOLO 감지 + 화면 출력
		int cnt = DETECTandDISPLAYsingleFRAME(frame);  // 객체 감지 + 그리기 + MFC 출력
		frame.release();                               // 이전 프레임 메모리 해제

		//CDib* pDib = Mat2CDib(frame); // OpenCV image를 CDib 이미지로 변환한다.
		//((CDibApp*)pDib)->DrawDibOnPictureControl(&m_picture, true); // CDib 이미지를 화면에 출력한다.
		//delete  pDib;

		count_frames++; // 프레임 카운트 증가
		m_str_message.Format(L"frame counter= %d(%d),objects= %d \n detectedtime= %d", count_frames, total_frames, cnt, detectedtime);
		UpdateData(false);     // CString → UI 컨트롤 복사

	}
	cap.release(); // opencv 비디오 객체 해제
}

// ============================================================
// 비디오 스트리밍 전용 스레드 함수
// ============================================================
UINT CCapstoneDlg::BatchProcessingByThreadMode(LPVOID lParam) {
    // =====================================================
	// 스레드 파라미터 검증 (this 포인터)
	// =====================================================
    CCapstoneDlg* pOwner = (CCapstoneDlg*)lParam;      // lParam → 클래스 인스턴스 캐스팅
    if (!pOwner) {                                     // NULL 체크 (안전성)
        AfxMessageBox(_T("쓰레드를 실행시키는데 실패하였습니다"));
        return 0;                                      // 스레드 비정상 종료
    }
    // =====================================================
    // 무한 루프: 실시간 비디오 스트리밍 처리
    // =====================================================
	while (pOwner->m_bGoTHread) {                      // UI 스레드에서 제어하는 플래그 확인
		pOwner->VideoStream();                         // 한 번의 VideoStream() = 비디오 전체 재생
		// =============================================
		// FPS 제어 (30FPS = 33ms 간격)
		// =============================================
		Sleep(33);                                     // CPU 사용량↓ + 30FPS 안정화
	}   

    // =====================================================
    // 스레드 정상 종료
    // =====================================================
    return 1;                                          // UINT 스레드 성공 반환 (MFC 표준)
}