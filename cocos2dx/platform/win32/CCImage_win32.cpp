/****************************************************************************
Copyright (c) 2010 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

NS_CC_BEGIN;


////////////////////////////////////////////////////////////////////////////////
// copy frome cximage
#if defined (_WIN32_WCE)

#ifndef DEFAULT_GUI_FONT
#define DEFAULT_GUI_FONT 17
#endif

#ifndef PROOF_QUALITY
#define PROOF_QUALITY 2
#endif

struct DIBINFO : public BITMAPINFO
{
	RGBQUAD    arColors[255];    // Color table info - adds an extra 255 entries to palette
	operator LPBITMAPINFO()          { return (LPBITMAPINFO) this; }
	operator LPBITMAPINFOHEADER()    { return &bmiHeader;          }
	RGBQUAD* ColorTable()            { return bmiColors;           }
};

int BytesPerLine(int nWidth, int nBitsPerPixel)
{
	return ( (nWidth * nBitsPerPixel + 31) & (~31) ) / 8;
}

int NumColorEntries(int nBitsPerPixel, int nCompression, DWORD biClrUsed)
{
	int nColors = 0;
	switch (nBitsPerPixel)
	{
	case 1:
		nColors = 2;  break;
	case 2:
		nColors = 4;  break;   // winCE only
	case 4:
		nColors = 16; break;
	case 8:
		nColors =256; break;
	case 24:
		nColors = 0;  break;
	case 16:
	case 32:
		nColors = 3;  break; // I've found that PocketPCs need this regardless of BI_RGB or BI_BITFIELDS
	default:
		ASSERT(FALSE);
	}
	// If biClrUsed is provided, and it is a legal value, use it
	if (biClrUsed > 0 && biClrUsed <= (DWORD)nColors)
		return biClrUsed;

	return nColors;
}

int GetDIBits(
			  HDC hdc,           // handle to DC
			  HBITMAP hbmp,      // handle to bitmap
			  UINT uStartScan,   // first scan line to set
			  UINT cScanLines,   // number of scan lines to copy
			  LPVOID lpvBits,    // array for bitmap bits
			  LPBITMAPINFO lpbi, // bitmap data buffer
			  UINT uUsage        // RGB or palette index
			  )
{
	UINT	iColorTableSize = 0;

	if (!hbmp)
		return 0;

	// Get dimensions of bitmap
	BITMAP bm;
	if (!::GetObject(hbmp, sizeof(bm),(LPVOID)&bm))
		return 0;

	//3. Creating new bitmap and receive pointer to it's bits.
	HBITMAP hTargetBitmap;
	void *pBuffer;

	//3.1 Initilize DIBINFO structure
	DIBINFO  dibInfo;
	dibInfo.bmiHeader.biBitCount = 24;
	dibInfo.bmiHeader.biClrImportant = 0;
	dibInfo.bmiHeader.biClrUsed = 0;
	dibInfo.bmiHeader.biCompression = 0;
	dibInfo.bmiHeader.biHeight = bm.bmHeight;
	dibInfo.bmiHeader.biPlanes = 1;
	dibInfo.bmiHeader.biSize = 40;
	dibInfo.bmiHeader.biSizeImage = bm.bmHeight*BytesPerLine(bm.bmWidth,24);
	dibInfo.bmiHeader.biWidth = bm.bmWidth;
	dibInfo.bmiHeader.biXPelsPerMeter = 3780;
	dibInfo.bmiHeader.biYPelsPerMeter = 3780;
	dibInfo.bmiColors[0].rgbBlue = 0;
	dibInfo.bmiColors[0].rgbGreen = 0;
	dibInfo.bmiColors[0].rgbRed = 0;
	dibInfo.bmiColors[0].rgbReserved = 0;

	//3.2 Create bitmap and receive pointer to points into pBuffer
	HDC hDC = ::GetDC(NULL);
	ASSERT(hDC);
	hTargetBitmap = CreateDIBSection(
		hDC,
		(const BITMAPINFO*)dibInfo,
		DIB_RGB_COLORS,
		(void**)&pBuffer,
		NULL,
		0);

	::ReleaseDC(NULL, hDC);

	//4. Copy source bitmap into the target bitmap.

	//4.1 Create 2 device contexts
	HDC memDc = CreateCompatibleDC(NULL);
	if (!memDc) {
		ASSERT(FALSE);
	}

	HDC targetDc = CreateCompatibleDC(NULL);
	if (!targetDc) {
		ASSERT(FALSE);
	}

	//4.2 Select source bitmap into one DC, target into another
	HBITMAP hOldBitmap1 = (HBITMAP)::SelectObject(memDc, hbmp);
	HBITMAP hOldBitmap2 = (HBITMAP)::SelectObject(targetDc, hTargetBitmap);

	//4.3 Copy source bitmap into the target one
	BitBlt(targetDc, 0, 0, bm.bmWidth, bm.bmHeight, memDc, 0, 0, SRCCOPY);

	//4.4 Restore device contexts
	::SelectObject(memDc, hOldBitmap1);
	::SelectObject(targetDc, hOldBitmap2);
	DeleteDC(memDc);
	DeleteDC(targetDc);

	//Here we can bitmap bits: pBuffer. Note:
	// 1. pBuffer contains 3 bytes per point
	// 2. Lines ane from the bottom to the top!
	// 3. Points in the line are from the left to the right
	// 4. Bytes in one point are BGR (blue, green, red) not RGB
	// 5. Don't delete pBuffer, it will be automatically deleted
	//    when delete hTargetBitmap
	lpvBits = pBuffer;

	DeleteObject(hbmp);
	//DeleteObject(hTargetBitmap);

	return 1;
}
#endif 

/**
@brief	A memory DC which uses to draw text on bitmap.
*/
class BitmapDC
{
public:
    BitmapDC(HWND hWnd = NULL)
    : m_hDC(NULL)
    , m_hBmp(NULL)
    , m_hFont((HFONT)GetStockObject(DEFAULT_GUI_FONT))
	, m_hWnd(NULL)
    {
		m_hWnd = hWnd;
        HDC hdc = GetDC(hWnd);
        m_hDC   = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);
    }

    ~BitmapDC()
    {
        prepareBitmap(0, 0);
        if (m_hDC)
        {
            DeleteDC(m_hDC);
        }
        HFONT hDefFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (hDefFont != m_hFont)
        {
            DeleteObject(m_hFont);
            m_hFont = hDefFont;
        }
		// release temp font resource	
		if (m_curFontPath.size() > 0)
		{
			wchar_t * pwszBuffer = utf8ToUtf16(m_curFontPath);
			if (pwszBuffer)
			{
				RemoveFontResource(pwszBuffer);
				SendMessage( m_hWnd, WM_FONTCHANGE, 0, 0);
				delete [] pwszBuffer;
				pwszBuffer = NULL;
			}
		}
    }

	wchar_t * utf8ToUtf16(std::string nString)
	{
		wchar_t * pwszBuffer = NULL;
		do 
		{
			if (nString.size() < 0)
			{
				break;
			}
			// utf-8 to utf-16
			int nLen = nString.size();
			int nBufLen  = nLen + 1;			
			pwszBuffer = new wchar_t[nBufLen];
			CC_BREAK_IF(! pwszBuffer);
			memset(pwszBuffer,0,nBufLen);
			nLen = MultiByteToWideChar(CP_UTF8, 0, nString.c_str(), nLen, pwszBuffer, nBufLen);		
			pwszBuffer[nLen] = '\0';
		} while (0);	
		return pwszBuffer;

	}

    bool setFont(const char * pFontName = NULL, int nSize = 0)
    {
        bool bRet = false;
        do 
        {
			std::string fontName = pFontName;
			std::string fontPath;
            HFONT       hDefFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            LOGFONTW    tNewFont = {0};
            LOGFONTW    tOldFont = {0};
            GetObjectW(hDefFont, sizeof(tNewFont), &tNewFont);
            if (fontName.c_str())
            {	
				// create font from ttf file
				int nFindttf = fontName.find(".ttf");
				int nFindTTF = fontName.find(".TTF");
				if (nFindttf >= 0 || nFindTTF >= 0)
				{
					fontPath = CCFileUtils::fullPathFromRelativePath(fontName.c_str());
					int nFindPos = fontName.rfind("/");
					fontName = &fontName[nFindPos+1];
					nFindPos = fontName.rfind(".");
					fontName = fontName.substr(0,nFindPos);				
				}
				tNewFont.lfCharSet = DEFAULT_CHARSET;
				WCHAR temp[LF_FACESIZE] = L"\0";
				MultiByteToWideChar(CP_ACP, 0, fontName.c_str() , fontName.size(), temp , LF_FACESIZE);
                wcscpy(tNewFont.lfFaceName, temp);
            }
            if (nSize)
            {
                tNewFont.lfHeight = -nSize;
            }
            GetObjectW(m_hFont,  sizeof(tOldFont), &tOldFont);

            if (tOldFont.lfHeight == tNewFont.lfHeight
                && ! wcscpy(tOldFont.lfFaceName, tNewFont.lfFaceName))
            {
                // already has the font 
                bRet = true;
                break;
            }

            // delete old font
            if (m_hFont != hDefFont)
            {
                DeleteObject(m_hFont);
				// release old font register
				if (m_curFontPath.size() > 0)
				{
					wchar_t * pwszBuffer = utf8ToUtf16(m_curFontPath);
					if (pwszBuffer)
					{
						if(RemoveFontResource(pwszBuffer))
						{
							SendMessage( m_hWnd, WM_FONTCHANGE, 0, 0);
						}						
						delete [] pwszBuffer;
						pwszBuffer = NULL;
					}
				}
				fontPath.size()>0?(m_curFontPath = fontPath):(m_curFontPath.clear());
				// register temp font
				if (m_curFontPath.size() > 0)
				{
					wchar_t * pwszBuffer = utf8ToUtf16(m_curFontPath);
					if (pwszBuffer)
					{
						if(AddFontResource(pwszBuffer))
						{
							SendMessage( m_hWnd, WM_FONTCHANGE, 0, 0);
						}						
						delete [] pwszBuffer;
						pwszBuffer = NULL;
					}
				}
            }
            m_hFont = NULL;

            // create new font
            m_hFont = CreateFontIndirectW(&tNewFont);
            if (! m_hFont)
            {
                // create failed, use default font
                m_hFont = hDefFont;
                break;
            }
            
            bRet = true;
        } while (0);
        return bRet;
    }

    SIZE sizeWithText(const char * pszText, int nLen, DWORD dwFmt, LONG nWidthLimit)
    {
        SIZE tRet = {0};
        do 
        {
            CC_BREAK_IF(! pszText || nLen <= 0);

            RECT rc = {0, 0, 0, 0};
            DWORD dwCalcFmt = DT_CALCRECT;

            if (nWidthLimit > 0)
            {
                rc.right = nWidthLimit;
                dwCalcFmt |= DT_WORDBREAK
                    | (dwFmt & DT_CENTER)
                    | (dwFmt & DT_RIGHT);
            }
            // use current font to measure text extent
            HGDIOBJ hOld = SelectObject(m_hDC, m_hFont);

            // measure text size
            WCHAR* pwszText = new WCHAR[nLen + 1];
			MultiByteToWideChar(CP_ACP, 0, pszText , nLen, pwszText , nLen+1);
            DrawTextW(m_hDC, pwszText, nLen, &rc, dwCalcFmt);
			delete [] pwszText;
            SelectObject(m_hDC, hOld);

            tRet.cx = rc.right;
            tRet.cy = rc.bottom;
        } while (0);

        return tRet;
    }

    bool prepareBitmap(int nWidth, int nHeight)
    {
        // release bitmap
        if (m_hBmp)
        {
            DeleteObject(m_hBmp);
            m_hBmp = NULL;
        }
        if (nWidth > 0 && nHeight > 0)
        {
            m_hBmp = CreateBitmap(nWidth, nHeight, 1, 32, NULL);
            if (! m_hBmp)
            {
                return false;
            }
        }
        return true;
    }

    int drawText(const char * pszText, SIZE& tSize, CCImage::ETextAlign eAlign)
    {
        int nRet = 0;
        wchar_t * pwszBuffer = 0;
        do 
        {
            CC_BREAK_IF(! pszText);

            DWORD dwFmt = DT_WORDBREAK;
            DWORD dwHoriFlag = eAlign & 0x0f;
            DWORD dwVertFlag = (eAlign & 0xf0) >> 4;

            switch (dwHoriFlag)
            {
            case 1: // left
                dwFmt |= DT_LEFT;
                break;
            case 2: // right
                dwFmt |= DT_RIGHT;
                break;
            case 3: // center
                dwFmt |= DT_CENTER;
                break;
            }

            int nLen = strlen(pszText);
            SIZE newSize = sizeWithText(pszText, nLen, dwFmt, tSize.cx);

            RECT rcText = {0};
            // if content width is 0, use text size as content size
            if (tSize.cx <= 0)
            {
                tSize = newSize;
                rcText.right  = newSize.cx;
                rcText.bottom = newSize.cy;
            }
            else
            {

                LONG offsetX = 0;
                LONG offsetY = 0;
                rcText.right = newSize.cx; // store the text width to rectangle

                // calculate text horizontal offset
                if (1 != dwHoriFlag          // and text isn't align to left
                    && newSize.cx < tSize.cx)   // and text's width less then content width,
                {                               // then need adjust offset of X.
                    offsetX = (2 == dwHoriFlag) ? tSize.cx - newSize.cx     // align to right
                        : (tSize.cx - newSize.cx) / 2;                      // align to center
                }

                // if content height is 0, use text height as content height
                // else if content height less than text height, use content height to draw text
                if (tSize.cy <= 0)
                {
                    tSize.cy = newSize.cy;
                    dwFmt   |= DT_NOCLIP;
                    rcText.bottom = newSize.cy; // store the text height to rectangle
                }
                else if (tSize.cy < newSize.cy)
                {
                    // content height larger than text height need, clip text to rect
                    rcText.bottom = tSize.cy;
                }
                else
                {
                    rcText.bottom = newSize.cy; // store the text height to rectangle

                    // content larger than text, need adjust vertical position
                    dwFmt |= DT_NOCLIP;

                    // calculate text vertical offset
                    offsetY = (2 == dwVertFlag) ? tSize.cy - newSize.cy     // align to bottom
                        : (3 == dwVertFlag) ? (tSize.cy - newSize.cy) / 2   // align to middle
                        : 0;                                                // align to top
                }

                if (offsetX || offsetY)
                {
                    OffsetRect(&rcText, offsetX, offsetY);
                }
            }

            CC_BREAK_IF(! prepareBitmap(tSize.cx, tSize.cy));

            // draw text
            HGDIOBJ hOldFont = SelectObject(m_hDC, m_hFont);
            HGDIOBJ hOldBmp  = SelectObject(m_hDC, m_hBmp);
            
            SetBkMode(m_hDC, TRANSPARENT);
            SetTextColor(m_hDC, RGB(255, 255, 255)); // white color

            // utf-8 to utf-16
            int nBufLen  = nLen + 1;
            pwszBuffer = new wchar_t[nBufLen];
            CC_BREAK_IF(! pwszBuffer);
            nLen = MultiByteToWideChar(CP_ACP, 0, pszText, nLen, pwszBuffer, nBufLen);

            // draw text
            nRet = DrawTextW(m_hDC, pwszBuffer, nLen, &rcText, dwFmt);
            //DrawTextA(m_hDC, pszText, nLen, &rcText, dwFmt);

            SelectObject(m_hDC, hOldBmp);
            SelectObject(m_hDC, hOldFont);
        } while (0);
        CC_SAFE_DELETE_ARRAY(pwszBuffer);
        return nRet;
    }

    CC_SYNTHESIZE_READONLY(HDC, m_hDC, DC);
    CC_SYNTHESIZE_READONLY(HBITMAP, m_hBmp, Bitmap);
private:
    friend class CCImage;
    HFONT   m_hFont;
	HWND	m_hWnd;
	std::string m_curFontPath;
};

static BitmapDC& sharedBitmapDC()
{
    static BitmapDC s_BmpDC;
    return s_BmpDC;
}

bool CCImage::initWithString(
                               const char *    pText, 
                               int             nWidth/* = 0*/, 
                               int             nHeight/* = 0*/,
                               ETextAlign      eAlignMask/* = kAlignCenter*/,
                               const char *    pFontName/* = nil*/,
                               int             nSize/* = 0*/)
{
    bool bRet = false;
    unsigned char * pImageData = 0;
    do 
    {
        CC_BREAK_IF(! pText);       

        BitmapDC& dc = sharedBitmapDC();

        if (! dc.setFont(pFontName, nSize))
        {
            CCLog("Can't found font(%s), use system default", pFontName);
        }

        // draw text
        SIZE size = {nWidth, nHeight};
        CC_BREAK_IF(! dc.drawText(pText, size, eAlignMask));

        pImageData = new unsigned char[size.cx * size.cy * 4];
        CC_BREAK_IF(! pImageData);

        struct
        {
            BITMAPINFOHEADER bmiHeader;
            int mask[4];
        } bi = {0};
        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        CC_BREAK_IF(! GetDIBits(dc.getDC(), dc.getBitmap(), 0, 0, 
            NULL, (LPBITMAPINFO)&bi, DIB_RGB_COLORS));

        m_nWidth    = (short)size.cx;
        m_nHeight   = (short)size.cy;
        m_bHasAlpha = true;
        m_bPreMulti = false;
        m_pData     = pImageData;
        pImageData  = 0;
        m_nBitsPerComponent = 8;
        // copy pixed data
        bi.bmiHeader.biHeight = (bi.bmiHeader.biHeight > 0)
           ? - bi.bmiHeader.biHeight : bi.bmiHeader.biHeight;
        GetDIBits(dc.getDC(), dc.getBitmap(), 0, m_nHeight, m_pData, 
            (LPBITMAPINFO)&bi, DIB_RGB_COLORS);

        // change pixel's alpha value to 255, when it's RGB != 0
        COLORREF * pPixel = NULL;
        for (int y = 0; y < m_nHeight; ++y)
        {
            pPixel = (COLORREF *)m_pData + y * m_nWidth;
            for (int x = 0; x < m_nWidth; ++x)
            {
                COLORREF& clr = *pPixel;
                if (GetRValue(clr) || GetGValue(clr) || GetBValue(clr))
                {
                    clr |= 0xff000000;
                }
                ++pPixel;
            }
        }

        bRet = true;
    } while (0);

    return bRet;
}

NS_CC_END;
