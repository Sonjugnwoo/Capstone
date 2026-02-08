// Dib2010.h


#ifndef __DIBHANDLE_H__
#define __DIBHANDLE_H__

#define WIDTHBYTES(bits)        (((bits) + 31) /  32 * 4)
#define TYPE_BITMAP             ((WORD) ('M' << 8) | 'B')

#define FORMAT_RGB024	5
#define FORMAT_YUV422	6
#define FORMAT_YUV420	7

#define FILETYPE_NON		0
#define FILETYPE_BMP		1
#define FILETYPE_JPG		2
#define FILETYPE_RAW		3

/****************************************************************
 macros
****************************************************************/

// #define MAX(a,b) (a)>(b)?(a):(b)
// #define MIN(a,b) (a)>(b)?(b):(a)

//******************************************************************************************************
// Dib.cpp -- Global Functions
//******************************************************************************************************
CString GetHomeDirectory();
CString AddFinalBackslash(const CString pString);

   
class CDib : public CObject
{

//******************************************************************************************************
// (2) 주로 호출하는 함수들 
//******************************************************************************************************

public:     

		unsigned char * GetCDibPixels();  // 메인 프로그램에서 CDib로부터 영상데이터 추출할 때 사용 
       
        CDib();
        CDib(const wchar_t *pFileName);
        CDib(int width, int height, int bitcount, PALETTEENTRY *pEntries, unsigned char *pPattern);
        CDib(const VARIANT &pImage);
		CDib(int width, int height, int bitcount, unsigned char *pPattern, int optFormat=FORMAT_YUV422);
		CDib(unsigned char *pPtr, int nWidth, int nHeight, int BytePerPixel);

		CDib(CString strFileName, int nWidth, int nHeight);
		CDib(CString strFileName, int nWidth, int nHeight, int BytePerPixel);

		BOOL ReadRaw(CString strFileName, int nWidth, int nHeight);
		BOOL ReadRaw(unsigned char *pPtr, int nWidth, int nHeight);
		BOOL ReadRawRGB(CString strFileName, int nWidth, int nHeight, int BytePerPixel);
		BOOL ReadRawRGB(unsigned char *pPtr, int nWidth, int nHeight, int BytePerPixel);

		void YUV422ToRGB24(BYTE *pbRGB, BYTE *pbYUV, int w, int h);
		void YUV420ToRGB24(BYTE *pbRGB, BYTE *pbYUV, int w, int h);
        DECLARE_DYNCREATE(CDib)      
		~CDib();

		CDib *GetGrayCDib();     
        CDib *CopyCDib(); 		
        CDib *GetSizedCDib(int width, int height=0);  
		
		BOOL ReadImage(const wchar_t *pFileName);
        BOOL SaveImage(const wchar_t *pFileName);

		inline LONG Width(){  return m_pInfoHeader->biWidth;   };
        inline LONG Height(){ return m_pInfoHeader->biHeight;  };
		inline unsigned char *GetPattern(){ return m_pPattern; };
        unsigned char *GetPointer(int x, int y); 
        unsigned char  Intensity( int x, int y);
		void ResetContents(BYTE value = 0);
		void Allocate(int width, int height, int bitcount);  
		void SetGrayPalette();
		
		inline WORD BitCount(){ return m_pInfoHeader->biBitCount; }; 

		void Inverse();
        void UpsideDown();
		void UpsideDownMirror24();

//******************************************************************************************************
// (3) 주로 내부적으로 사용하는 함수들 
//******************************************************************************************************

protected:
		void LoadImage(CDC *pDC, CRect *pRectTarget=NULL, CRect *pRectSource=NULL, DWORD dwRop=SRCCOPY);        
        void LoadPaletteImage(CDC *pDC, CRect *pRectTarget=NULL, CRect *pRectSource=NULL, DWORD dwRop=SRCCOPY); 

private:
        virtual void Serialize(CArchive& ar);  
		static int ParseFileType(const wchar_t *pFileName);
        static CString m_ExtensionDib;
        static CString m_ExtensionJpg;
		static CString m_ExtensionJpge;
		static CString m_ExtensionRaw;
        static unsigned char m_BitMask[8];  


private:
        void ResetVariables();
        void DestroyContents();
        void AssignReferenceVariables();
        BITMAPINFO *GetCopyOfBitmapInfo(); 
        unsigned char *m_pPackedDib;
        BITMAPINFO *m_pInfo;
        BITMAPINFOHEADER *m_pInfoHeader;
        RGBQUAD *m_pRgbQuad;
        unsigned char *m_pPattern;
        unsigned char m_PaletteUsage[256]; 

protected:      
        void   Initialize(int width, int height, int bitcount, PALETTEENTRY *pEntries, unsigned char *pPattern);        
        void   LoadPackedDib(void *pPackedDib); 
		
        inline BOOL HavePalette(){ return (BitCount()<=8); };
        inline BITMAPINFO *GetBitmapInfo(){ return m_pInfo; };
        inline BITMAPINFOHEADER *GetBitmapInfoHeader(){ return m_pInfoHeader; };
        inline WORD ByteWidth(){ return (WORD) ByteWidth(m_pInfoHeader->biWidth * m_pInfoHeader->biBitCount); }; 
        static inline WORD ByteWidth(LONG nbits){ return (WORD)(WIDTHBYTES(nbits)); };
        inline BOOL IsInitialized(){ return (m_pPackedDib!=NULL); };
        inline unsigned char *GetPackedDibPointer(){ return m_pPackedDib; };
        
        unsigned char GetIndex(int x, int y); 
        RGBQUAD GetColor(int x, int y);  
        DWORD PaletteSize(); 
        DWORD NumberOfColors(); 
        static DWORD CalcPackedDibSize(BITMAPINFOHEADER *pBi);
		static unsigned char GetBrightness(RGBQUAD rgb);
        void SetPaletteEntries(UINT nStartIndex, UINT nNumEntries, PALETTEENTRY *pEntries);
        void GetPaletteEntries(UINT nStartIndex, UINT nNumEntries, PALETTEENTRY *pEntries); 
 		 
        static RGBQUAD Decode16(WORD data);
        static WORD Encode16(unsigned char red, unsigned char green, unsigned char blue);
        static void PaletteEntryToRgbQuad(PALETTEENTRY *pEntries, RGBQUAD *pRgbQuad, int count, unsigned char *pPaletteUsage=NULL);
        static void RgbQuadToPaletteEntry(RGBQUAD *pRgbQuad, PALETTEENTRY *pEntries, int count, unsigned char *pPaletteUsage=NULL);
        CPalette *GetPalette();
        CPalette *GetPaletteNoCollapse();
        CPalette *GetPaletteNoFlag();
        CBitmap  *GetCBitmap(CDC *pDC=NULL);   
        		
        BOOL ReadDib(const wchar_t *pFileName);
        BOOL SaveDib(const wchar_t *pFileName);
		BOOL ReadJPG(const wchar_t *pFileName);
		BOOL SaveAsJPG(const wchar_t *pFileName, BOOL bColor=TRUE, int quality=75,BOOL bPA=FALSE);      
};

class CDibApp : public CDib  
{
public:
	CDibApp();
	virtual ~CDibApp();
	void DrawDibOnPictureControl(CStatic *picture, BOOL bFitSize = TRUE, CPoint ptStart = CPoint(0,0));

	CDib*  Get8BitGrayCDib( );
	CDib*  GetResizedGrayCDib(int nWidth, int nHeight);
};



template<typename T>
inline T limit(const T& value)
{
	return ( (value > 255) ? 255 : ((value < 0) ? 0 : value) );
}

/////////////////////////////////////////////////////////////////
#endif // __DIBHANDLE_H__
