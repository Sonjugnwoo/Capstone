#include "pch.h"
#include "CapstoneDlg.h"
#include "afxdialogex.h"

// ============================================================
// YOLOv3 킥보드 감지 시스템 초기화
// ============================================================
BOOL CCapstoneDlg::InitializeYOLODetector() {
	// =====================================================
	// YOLO DLL 동적 로딩
	// =====================================================
	m_hDLL = NULL;                              // DLL 핸들 초기화
	m_hDLL = LoadLibrary(_T("yolov3-kickboard.dll"));  // 커스텀 YOLO DLL 로드
	if (m_hDLL == NULL)	{
		AfxMessageBox(_T("Error! Loading Dll file!!"));  // DLL 파일 없거나 손상
		return FALSE;                               // 초기화 실패 → 프로그램 종료
	}
	// =====================================================
    // 실행파일(.exe) 디렉토리 경로 추출
    // =====================================================
	wchar_t path[MAX_PATH] = { 0 };             // 현재 실행파일 전체 경로 버퍼
	GetModuleFileName(NULL, path, MAX_PATH);    // "C:\\Project\\Exe\\Capstone.exe"
	CString msg(path);                          // 경로를 CString으로 변환
	int n = msg.ReverseFind('\\');              // 마지막 백슬래시 위치 찾기
	m_strHome = msg.Left(n + 1);                // "C:\\Project\\Exe\\" ← 모델파일 경로로 사용

	// =====================================================
	// YOLOv3 모델 파일 로드 및 엔진 초기화
	// =====================================================
	CString str_cfg = m_strHome + L"yolov3-kickboard.cfg";   // 설정 파일 경로
	CString str_wgt = m_strHome + L"yolov3-kickboard.weights"; // 학습된 가중치 파일
	int gpu_id = 0;                             // GPU 0번 사용 (CPU= -1)

	// DLL에서 'init' 함수 추출
	typedef int (DLLInit)(const char*, const char*, int);     // 함수 시그니처 정의
	DLLInit* DllEngineInit = (DLLInit*)GetProcAddress(m_hDLL, (LPCSTR)("init"));
	int iOk = (*DllEngineInit)((CStringA)str_cfg, (CStringA)str_wgt, gpu_id);  // 모델 초기화 호출
	if (iOk != 1) {
		AfxMessageBox(_T("Error! Initialize engine!!"));  // .cfg/.weights 파일 없거나 호환성 문제
		return FALSE;                                   // YOLO 엔진 시작 실패
	}
	// =====================================================
	// YOLO 클래스 이름 로드 (obj-kickboard.names)
	// =====================================================
	LoadClassName(m_strHome + L"obj-kickboard.names");  // "kickboard", "person" 등 클래스명 로드

	return TRUE;
}
// ============================================================
// YOLO 단일 프레임 완전 처리
// ============================================================
int CCapstoneDlg::DETECTandDISPLAYsingleFRAME(cv::Mat image){
	// =====================================================
	// OpenCV Mat → Raw 바이트 배열 변환 (DLL 입력용)
	// =====================================================
	unsigned char* pxl = Mat2Byte(image);          // 이미지 → 연속 메모리 버퍼
	int w = image.cols;                            // 너비
	int h = image.rows;                            // 높이
	int c = image.channels();                      // 채널수 (3=RGB)

	// =====================================================
	// YOLO DLL 감지 함수 동적 호출
	// =====================================================
	bbox_t_container container;                    // DLL 출력 컨테이너 (candidates 배열)
	typedef int (DllDetect)(unsigned char*, int, int, int, bbox_t_container&);  // detect_pixel() 시그니처
	DllDetect* DllEngineDetect = (DllDetect*)GetProcAddress(m_hDLL, "detect_pixel");

	// DLL 감지 실행: 이미지 데이터 + 크기 + 컨테이너
	int cnt = (*DllEngineDetect)(pxl, w, h, c, container);  // 감지된 객체수 반환
	delete[] pxl;                                  // 임시 버퍼 해제

	// =====================================================
	// DLL 컨테이너 → std::vector<bbox_t> 변환
	// =====================================================
	std::vector<bbox_t> results;                    // 표준 벡터로 변환 (DrawDetectionResults 입력)
	for (int i = 0; i < cnt; i++){	
		bbox_t obj;                                  // 개별 객체 정보
		obj.x = container.candidates[i].x;           // 바운딩박스 좌상단 X
		obj.y = container.candidates[i].y;           // 바운딩박스 좌상단 Y
		obj.w = container.candidates[i].w;           // 바운딩박스 너비
		obj.h = container.candidates[i].h;			 // 바운딩박스 높이
		obj.obj_id = container.candidates[i].obj_id; // 클래스 ID (m_vecClassName 인덱스)
		results.push_back(obj);
	}
	// =====================================================
	// 감지 결과 시각화 + MFC 화면 출력
	// =====================================================
	DrawDetectionResults(results, image);          // 이미지에 바운딩박스 그리기
	CDib* pDib = Mat2CDib(image);                  // OpenCV Mat → MFC CDib 변환

	((CDibApp*)pDib)->DrawDibOnPictureControl(&m_picture, true);   // Static 컨트롤에 표시 (true=스트레치)
	delete pDib;			   // CDib 메모리 해제

	return cnt;
}

// ============================================================
// YOLO 감지 결과를 이미지에 그리기
// ============================================================
void CCapstoneDlg::DrawDetectionResults(std::vector<bbox_t>& results, cv::Mat& image){
	for (int i = 0; i < results.size(); i++){
		// -------------------------------------------------
		// 바운딩 박스 그리기 (파란색 사각형)
		// -------------------------------------------------
		int linethickness = 3;  // 테두리 두께
		cv::rectangle(
			image,
			cv::Point(results[i].x, results[i].y),								 // 좌상단 좌표
			cv::Point(results[i].x + results[i].w, results[i].y + results[i].h), // 우하단 좌표
			cv::Scalar(255, 0, 0),												 // BGR: 파란색
			linethickness													     // 두께
		);
		// -------------------------------------------------
		// 클래스 이름 문자열 준비 (CString → std::string)
		// -------------------------------------------------		
		CString strName = m_vecClassName[results[i].obj_id];               // 클래스 ID → 이름 매핑
		std::string stringText = std::string(CT2CA(strName.operator LPCWSTR())); // 유니코드 → 멀티바이트 변환
		// -------------------------------------------------
		// 텍스트 스타일 설정 (폰트, 크기, 굵기 등)
		// -------------------------------------------------
		int fontface = 0; // 0:cv::FONT_HERSHEY_SIMPLEX; 1:cv::FONT_HERSHEY_PLAIN
		double fontscale = 1.0;            // 글자 크기 배율
		int fontthickness = 2;             // 글자 두께
		int baseline = 0;                  // 글자 기준선 정보 (아래 여백)
		// 텍스트 시작 위치 (바운딩 박스 좌상단 기준)
		cv::Point point;
		point.x = results[i].x;
		point.y = results[i].y;

		// 텍스트 박스 크기 계산
		cv::Size text = cv::getTextSize(stringText, fontface, fontscale, fontthickness, &baseline);
		// -------------------------------------------------
	    // 텍스트 배경 박스 그리기 (파란색 채움)
	    // -------------------------------------------------
		cv::rectangle(image, cv::Rect(point.x, point.y - text.height, text.width, text.height),
			cv::Scalar(255, 0, 0), CV_FILLED);	// (텍스트 영역 , 파란색 배경 , 채움 모드)
		// -------------------------------------------------
		// 흰색 텍스트 그리기 (클래스 이름)
		// -------------------------------------------------
		cv::putText(image, stringText, point, fontface,
			fontscale, cv::Scalar(255, 255, 255), fontthickness, cv::LINE_AA);	
		// (출력할 문자열 , 시작 위치, 폰트 종류 , 폰트 크기 , 글자색:흰색 , 글자 두께 , 안티에일리어싱(부드러운 텍스트)
	}
	if (!results.empty()) {
		detectedtime++;
		if (detectedtime >= 16) {
			SendKickboardAlert(image);
			detectedtime = 0;
		}
	}
}
