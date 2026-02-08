#include "pch.h"
#include "CapstoneDlg.h"

// ****************************************************************
// 현재 시스템 시간을 문자열로 반환
// ****************************************************************
CString CCapstoneDlg::fSystemTimeGet() {
	CTime cTime = CTime::GetCurrentTime();		// 시간 객체 생성
	CString strTime;							// 시간 문자열 저장 변수

    // 파일명용 시간 포맷 (년도월일_시분초)
    strTime.Format(L"%04d%02d%02d_%02d%02d%02d"
        , cTime.GetYear()
        , cTime.GetMonth()
        , cTime.GetDay()
        , cTime.GetHour()
        , cTime.GetMinute()
        , cTime.GetSecond());

    return strTime;  // "20151107_214559"
}
// ****************************************************************
// 전동킥보드 인도 감지 → 자동 메일 알림 (16프레임 연속 감지시)
// ****************************************************************
void CCapstoneDlg::SendKickboardAlert(const cv::Mat& image) {
	// =====================================================
	// 증거 이미지 저장
	// =====================================================
	CString timestamp = fSystemTimeGet();              // "20151107_214559"
	CString savePath = L"C:\\";                        // 루트 디렉토리
	CString filename = L"Kickboard_" + timestamp + L".jpg";  // 파일명 생성
	CString fullpath = savePath + filename;            // 완전 경로

	std::string filepath_std = CT2A(fullpath);         // CString → std::string 변환
	cv::imwrite(filepath_std, image);				   // 이미지 저장 완료!
 
	// =====================================================
	// MAPI32.DLL 로드 (Outlook 등 기본 메일 클라이언트)
	// =====================================================
	HINSTANCE h_send_mail = ::LoadLibrary(L"MAPI32.DLL");	 // Windows 메일 API DLL
	if (NULL != h_send_mail) {		
		LPMAPISENDMAILW fp_send_mail = (LPMAPISENDMAILW)GetProcAddress(h_send_mail, "MAPISendMailW");
		if (fp_send_mail != NULL) {
			// =====================================================
			// MAPI 구조체 초기화 (메모리 오류 방지)
			// =====================================================
			MapiMessageW tips_msg = { 0 };             // 메일 본문 구조체
			MapiRecipDescW from_user = { 0 }, to_user = { 0 };  // 발신/수신자
			MapiFileDescW attach_file[1] = { 0 };      // 첨부파일 (1개)

			// =====================================================
			// 발신자/수신자 설정
			// =====================================================
			from_user.ulRecipClass = MAPI_ORIG;        // 발신자 타입
			from_user.lpszAddress = from_user.lpszName = L"";  // 발신자

			to_user.ulRecipClass = MAPI_TO;            // 수신자 타입
			to_user.lpszName = L"";                    // 수신자 표시명 
			to_user.lpszAddress = L"";                 // 수신자 주소 (SMTP:이메일)

			// =====================================================
			// 메일 구성 (첨부파일 포함)
			// =====================================================
			tips_msg.lpOriginator = &from_user;        // 발신자 연결
			tips_msg.nRecipCount = 1;                  // 수신자 1명
			tips_msg.lpRecips = &to_user;              // 수신자 연결
			tips_msg.nFileCount = 1;                   // 첨부파일 1개

			// 동적 첨부파일 설정
			attach_file[0].lpszPathName = fCStringToPWSTRcasting(fullpath);   // 실제 파일 경로
			attach_file[0].lpszFileName = fCStringToPWSTRcasting(filename);   // 메일에 표시될 이름
			tips_msg.lpFiles = attach_file;            // 첨부파일 연결


			// =====================================================
			// 메일 제목/내용 설정
			// =====================================================
			tips_msg.lpszSubject = L"전동킥보드 인도 감지 알림";     // 제목
			tips_msg.lpszNoteText = L"인도 주행 감지됨!\n증거 이미지가 첨부되었습니다.";  // 본문

			// =====================================================
			// 메일 발송 실행
			// =====================================================
			(*fp_send_mail)(0, (ULONG)m_hWnd, &tips_msg, MAPI_NEW_SESSION | MAPI_LOGON_UI, 0);
			Sleep(5000);  // 메일 클라이언트 창 닫힘 대기 
		}

		::FreeLibrary(h_send_mail);  // DLL 메모리 해제
	}

	detectedtime = 0;  // 16프레임 타이머 리셋 (다음 감지 대기)
}
