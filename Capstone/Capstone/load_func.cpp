#include "pch.h"
#include "CapstoneDlg.h"
#include "afxdialogex.h"

// ============================================================
// YOLO 클래스 이름 파일 로더
// ============================================================
BOOL CCapstoneDlg::LoadClassName(const CString strFilePath){
	// =====================================================
    // 기존 클래스 목록 초기화
	// =====================================================
	m_vecClassName.clear();                     // 이전 로드된 클래스명 모두 삭제
	wchar_t buffer[125];                        // 클래스명 읽기용 버퍼 (최대 125 유니코드 문자)

	// obj-kickboard.names 등 YOLO 클래스 파일 로드
	FILE* stream = _wfopen(strFilePath, L"rt+,ccs=UTF-8");  // 유니코드 + UTF-8 디코딩
	if (stream == NULL)	{
		AfxMessageBox(L"Can't open " + strFilePath);  // "obj-kickboard.names" 파일 없음
		return FALSE;                               // 파일 열기 실패
	}
	// =====================================================
	// 클래스명 한 줄씩 파싱
	// =====================================================
	while (fgetws(buffer, 125, stream) != NULL) {  // 한 줄씩 읽기 (NULL=EOF)
		CString strClass(buffer);                   // 버퍼 → CString 변환
		CString strOnlyChar = strClass.Left(strClass.GetLength() - 1);  // 마지막 개행문자(\n) 제거
		m_vecClassName.push_back(strOnlyChar);      // "kickboard", "person" 등 저장
	}// EX) m_vecClassName[0] = "kickboard" , m_vecClassName[1] = "person"

	fclose(stream);                             // 파일 스트림 닫기
	return TRUE;                                // 모든 클래스명 로드 성공
}

// ============================================================
// MFC 파일 선택 대화상자로 이미지 로드
// ============================================================
cv::Mat CCapstoneDlg::LoadImageFile(){
    cv::Mat MatImage;                              // 반환용 OpenCV 이미지 객체 (초기값: 빈 Mat)
    // =====================================================
    // MFC 파일 선택 대화상자 설정
    // =====================================================
    CFileDialog dlg(
        TRUE,                                      // TRUE=열기, FALSE=저장
        _T("jpg(*.*)"),                            // 기본 파일 확장자
        _T("*.jpg"),                               // 기본 파일명
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,    // 읽기전용 + 덮어쓰기 경고
        _T("All(*.*)|*.*|(*.bmp)|*.bmp|(*.jpg)|*.jpg|")  // 파일 필터 (모든파일/BMP/JPG)
    );
    // =====================================================
    // 파일 선택 대화상자 실행
    // =====================================================
    if (dlg.DoModal() == IDOK) {                    // 사용자가 "열기" 버튼 클릭시    
        CString strImagePath = dlg.GetPathName();  // 전체 파일 경로 (예: "C:\\test.jpg")
        CString strFileName = dlg.GetFileName();   // 파일명만 (예: "test.jpg")

        // =====================================================
        // MFC CString → OpenCV std::string 변환
        // =====================================================
        CT2CA pszString(strImagePath);             // 유니코드 CString → ANSI 문자열
        std::string strPath(pszString);            // std::string으로 최종 변환

        // =====================================================
        // OpenCV로 이미지 파일 로드
        // =====================================================
        MatImage = cv::imread(strPath);            // JPG/BMP/PNG 등 자동 형식 인식
                                                   // 실패시 빈 Mat 반환 (empty()==true)
    }
    // 사용자가 "취소" 클릭시 → MatImage는 빈 Mat 그대로 반환

    return MatImage;                               // YOLO 처리용 이미지 반환
}
