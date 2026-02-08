#include "pch.h"
#include "CapstoneDlg.h"
#include "afxdialogex.h"
// ============================================================
// OpenCV Mat → CDib 형식 변환기
// ============================================================
CDib* CCapstoneDlg::Mat2CDib(cv::Mat& Image){
	// =====================================================
	// 이미지 정보 추출 (너비, 높이, 채널수)
	// =====================================================
	int w = Image.cols;             // 이미지 너비 (픽셀)
	int h = Image.rows;             // 이미지 높이 (픽셀)
	int channels = Image.channels(); // 채널수 (3=RGB, 1=그레이스케일)

	// RGB 24비트 컬러 이미지만 지원 (BGR 3채널)
	if (channels != 3) return NULL; // 그레이/4채널 등은 지원안함 → NULL 반환

	// =====================================================
	// 픽셀 데이터 메모리 할당
	// =====================================================
	unsigned char* pxl = new unsigned char[w * h * channels];  // CDib용 연속 메모리
	unsigned char* mat = (unsigned char*)Image.data;           // OpenCV Mat 데이터 포인터

	// =====================================================
	// 픽셀 복사 (OpenCV BGR → CDib BGR 순서 유지)
	// =====================================================
	// OpenCV Mat: BGRBGRBGR... (행 우선, 채널 마지막)
	// CDib:      BGRBGRBGR... (동일한 메모리 레이아웃)
	for (int j = 0; j < h; j++)                    // 높이 루프 (아래→위)
		for (int i = 0; i < w; i++){               // 너비 루프 (좌→우)		
			int idx = 3 * ((w) * (j)+(i));         // 1차원 인덱스 계산 (y*w + x)*3
			pxl[idx + 0] = mat[idx + 0];           // Blue 채널 복사
			pxl[idx + 1] = mat[idx + 1];           // Green 채널 복사  
			pxl[idx + 2] = mat[idx + 2];           // Red 채널 복사
		}

	// =====================================================
	// CDib 객체 생성 및 반환
	// =====================================================
	CDib* pDib = new CDib(pxl, w, h, channels);    // CDib 생성자 호출 (소유권 이전)
	delete[] pxl;                                  // 임시 버퍼 해제 (CDib가 복사함)
	return pDib;                                   // MFC Static 컨트롤 표시용 반환
}
// ============================================================
// CDib → OpenCV Mat 형식 변환기 (역방향)
// ============================================================
cv::Mat CCapstoneDlg::CDib2Mat(CDib* pDib){
	// =====================================================
	// CDib 이미지 정보 추출
	// =====================================================
	int w = pDib->Width();                      // DIB 너비 (픽셀)
	int h = pDib->Height();                     // DIB 높이 (픽셀)
	int channels = pDib->BitCount() / 8;        // 비트수→채널수 변환 (24bit=3채널)

	// RGB 24비트만 지원 (CDib 표준)
	if (channels != 3) {
		AfxMessageBox(L"RGB 24bits color image only!!");  // 비표준 형식 경고
		return cv::Mat();                           // 빈 Mat 반환 (실패)
	}

	// =====================================================
	// OpenCV Mat 생성 + CDib 픽셀 데이터 접근
	// =====================================================
	unsigned char* pxl = pDib->GetCDibPixels();    // CDib 내부 RGB 픽셀 배열 직접 접근
	cv::Mat cvImage(h, w, CV_8UC3);                // 높이×너비×3채널 unsigned char Mat 생성
	unsigned char* ptr = (unsigned char*)cvImage.data;  // OpenCV Mat 데이터 포인터

	// =====================================================
	// 픽셀 복사 (CDib BGR → OpenCV BGR, 순서 동일)
	// =====================================================
	// CDib:  BGRBGRBGR... (행 우선 저장)
	// OpenCV: BGRBGRBGR... (동일한 메모리 레이아웃)
	for (int j = 0; j < h; j++)                    // 세로 루프 (위→아래)
		for (int i = 0; i < w; i++) {              // 가로 루프 (좌→우)
			int idx = 3 * ((w) * (j)+(i));		   // 1차원 인덱스: (y*w + x)*3
			ptr[idx + 0] = pxl[idx + 0];           // Blue 채널 복사
			ptr[idx + 1] = pxl[idx + 1];           // Green 채널 복사
			ptr[idx + 2] = pxl[idx + 2];           // Red 채널 복사
		}

	return cvImage;                             // OpenCV 처리용 Mat 반환
}
// ============================================================
// OpenCV Mat → Raw 바이트 배열 추출기
// ============================================================
unsigned char* CCapstoneDlg::Mat2Byte(cv::Mat& Image)
{
	// =====================================================
	// 이미지 크기 정보 추출
	// =====================================================
	int w = Image.cols;                         // 이미지 너비 (픽셀)
	int h = Image.rows;                         // 이미지 높이 (픽셀)
	int channels = Image.channels();            // 채널수 (3=RGB, 1=그레이스케일)

	// =====================================================
	// 연속 메모리 버퍼 동적 할당
	// =====================================================
	unsigned char* pxl = new unsigned char[w * h * channels];  // raw 픽셀 데이터 버퍼
	unsigned char* mat = (unsigned char*)Image.data;           // OpenCV Mat 내부 데이터

	// =====================================================
	// RGB 24비트 (3채널) 이미지 처리
	// =====================================================
	if (channels == 3) {
		// BGRBGRBGR... 형식으로 완전 복사
		for (int j = 0; j < h; j++)                // 세로 루프
			for (int i = 0; i < w; i++){           // 가로 루프
				int idx = 3 * ((w) * (j)+(i));     // 1D 인덱스: (y*w + x)*3
				pxl[idx + 0] = mat[idx + 0];       // Blue 채널
				pxl[idx + 1] = mat[idx + 1];       // Green 채널
				pxl[idx + 2] = mat[idx + 2];       // Red 채널
			}
	}
	// =====================================================
	// 그레이스케일 (1채널) 이미지 처리
	// =====================================================
	else if (channels == 1) {
		// GGGG... 형식으로 1:1 복사
		for (int j = 0; j < h; j++)
			for (int i = 0; i < w; i++)			
				pxl[(w) * (j)+(i)] = mat[(w) * (j)+(i)];  // 단일 채널 복사			
	}

	return pxl;                               
}


// 이메일 용 
// ============================================================
// CString → PWSTR 변환 유틸리티
// ============================================================
PWSTR CCapstoneDlg::fCStringToPWSTRcasting(CString str)
{
	PWSTR pwstr;
	// LPWSTR, PWSTR 방식은 동일
	BSTR bstr = str.AllocSysString();
	PWSTR wstr = (PWSTR)bstr;
	SysFreeString(bstr);
	return wstr;
}