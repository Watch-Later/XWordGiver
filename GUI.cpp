//////////////////////////////////////////////////////////////////////////////
// GUI.cpp --- XWord Giver (Japanese Crossword Generator)
// Copyright (C) 2012-2020 Katayama Hirofumi MZ. All Rights Reserved.
// (Japanese, Shift_JIS)

#define NOMINMAX
#include "XWordGiver.hpp"
#include "layout.h"

// �N���X���[�h�̃T�C�Y�̐����B
#define XG_MIN_SIZE         3
#define XG_MAX_SIZE         22

#ifndef WM_MOUSEHWHEEL
    #define WM_MOUSEHWHEEL 0x020E
#endif

// �����̍ő吔�B
#define MAX_DICTS 16

#undef HANDLE_WM_MOUSEWHEEL     // might be wrong
#define HANDLE_WM_MOUSEWHEEL(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), \
        (int)(short)HIWORD(wParam), (UINT)(short)LOWORD(wParam)), 0)

#undef FORWARD_WM_MOUSEWHEEL    // might be wrong
#define FORWARD_WM_MOUSEWHEEL(hwnd, xPos, yPos, zDelta, fwKeys, fn) \
    (void)(fn)((hwnd), WM_MOUSEWHEEL, MAKEWPARAM((fwKeys),(zDelta)), MAKELPARAM((xPos),(yPos)))

void __fastcall MainWnd_OnChar(HWND hwnd, TCHAR ch, int cRepeat);
void __fastcall MainWnd_OnKey(HWND hwnd, UINT vk, bool fDown, int /*cRepeat*/, UINT /*flags*/);
void __fastcall MainWnd_OnImeChar(HWND hwnd, WCHAR ch, LPARAM /*lKeyData*/);

// �u���}�X�p�^�[���v�_�C�A���O�̈ʒu�ƃT�C�Y�B
INT xg_nPatWndX = CW_USEDEFAULT;
INT xg_nPatWndY = CW_USEDEFAULT;
INT xg_nPatWndCX = CW_USEDEFAULT;
INT xg_nPatWndCY = CW_USEDEFAULT;

//////////////////////////////////////////////////////////////////////////////
// global variables

// ���̓��[�h�B
XG_InputMode    xg_imode = xg_im_KANA;

// �C���X�^���X�̃n���h���B
HINSTANCE       xg_hInstance = nullptr;

// ���C���E�B���h�E�̃n���h���B
HWND            xg_hMainWnd = nullptr;

// �q���g�E�B���h�E�̃n���h���B
HWND            xg_hHintsWnd = nullptr;

// ���E�B���h�E�̃n���h���B
HWND            xg_hCandsWnd = nullptr;

// ���̓p���b�g�B
HWND            xg_hwndInputPalette = nullptr;

// �X�N���[���o�[�B
HWND xg_hVScrollBar          = nullptr;
HWND xg_hHScrollBar          = nullptr;
HWND xg_hSizeGrip            = nullptr;

// �c�[���o�[�̃n���h���B
HWND            xg_hToolBar  = nullptr;

// �X�e�[�^�X�o�[�̃n���h��
HWND            xg_hStatusBar  = nullptr;

// �c�[���o�[�̃C���[�W���X�g�B
HIMAGELIST      xg_hImageList     = nullptr;
HIMAGELIST      xg_hGrayedImageList = nullptr;

// �����t�@�C���̏ꏊ�i�p�X�j�B
std::wstring xg_dict_name;
std::deque<std::wstring>  xg_dict_files;

// �q���g�ɒǉ������������H
bool            xg_bHintsAdded = false;

// JSON�t�@�C���Ƃ��ĕۑ����邩�H
bool            xg_bSaveAsJsonFile = true;

// ���g�����邩�H
bool            xg_bAddThickFrame = true;

// �}�X�̃t�H���g�B
WCHAR xg_szCellFont[LF_FACESIZE] = L"";

// �����ȕ����̃t�H���g�B
WCHAR xg_szSmallFont[LF_FACESIZE] = L"";

// UI�t�H���g�B
WCHAR xg_szUIFont[LF_FACESIZE] = L"";

// �u���ɖ߂��v���߂̃o�b�t�@�B
XG_UndoBuffer                        xg_ubUndoBuffer;

// ���O�ɉ������L�[���o���Ă����B
WCHAR xg_prev_vk = 0;

// �u���̓p���b�g�v�c�u���H
bool xg_bTateOki = true;

// �\���p�ɕ`�悷�邩�H�iXgGetXWordExtent��XgDrawXWord��XgCreateXWordImage�Ŏg���j�B
INT xg_nForDisplay = 0;

// �Y�[���䗦(%)�B
INT xg_nZoomRate = 100;

// �X�P���g�����[�h���H
BOOL xg_bSkeltonMode = FALSE;

// �ԍ���\�����邩�H
BOOL xg_bShowNumbering = TRUE;
// �L�����b�g��\�����邩�H
BOOL xg_bShowCaret = TRUE;

//////////////////////////////////////////////////////////////////////////////
// static variables

// �ۑ���̃p�X�̃��X�g�B
static std::deque<std::wstring>  s_dirs_save_to;

// ���C���E�B���h�E�̈ʒu�ƃT�C�Y�B
static int s_nMainWndX = CW_USEDEFAULT, s_nMainWndY = CW_USEDEFAULT;
static int s_nMainWndCX = CW_USEDEFAULT, s_nMainWndCY = CW_USEDEFAULT;

// �q���g�E�B���h�E�̈ʒu�ƃT�C�Y�B
static int s_nHintsWndX = CW_USEDEFAULT, s_nHintsWndY = CW_USEDEFAULT;
static int s_nHintsWndCX = CW_USEDEFAULT, s_nHintsWndCY = CW_USEDEFAULT;

// ���E�B���h�E�̈ʒu�ƃT�C�Y�B
static int s_nCandsWndX = CW_USEDEFAULT, s_nCandsWndY = CW_USEDEFAULT;
static int s_nCandsWndCX = CW_USEDEFAULT, s_nCandsWndCY = CW_USEDEFAULT;

// ���̓p���b�g�̈ʒu�B
INT xg_nInputPaletteWndX = CW_USEDEFAULT;
INT xg_nInputPaletteWndY = CW_USEDEFAULT;

// �Ђ炪�ȕ\�����H
BOOL xg_bHiragana = FALSE;
// Lowercase�\�����H
BOOL xg_bLowercase = FALSE;

// ��Ж��B
static const LPCWSTR
    s_pszSoftwareCompanyName = L"Software\\Katayama Hirofumi MZ";

// �A�v�����B
#ifdef _WIN64
    static const LPCWSTR s_pszAppName = L"XWord64";
#else
    static const LPCWSTR s_pszAppName = L"XWord32";
#endif

// ��Ж��ƃA�v�����B
#ifdef _WIN64
    static const LPCWSTR
        s_pszSoftwareCompanyAndApp = L"Software\\Katayama Hirofumi MZ\\XWord64";
#else
    static const LPCWSTR
        s_pszSoftwareCompanyAndApp = L"Software\\Katayama Hirofumi MZ\\XWord32";
#endif

// �Čv�Z���邩�H
static bool s_bAutoRetry = true;

// �Â��p�\�R���ł��邱�Ƃ�ʒm�������H
static bool s_bOldNotice = false;

// �����t�@�C���̕ۑ����[�h�i0: �m�F����A1:�����I�ɕۑ�����A2:�ۑ����Ȃ��j�B
static int s_nDictSaveMode = 2;

// �V���[�g�J�b�g�̊g���q�B
static const LPCWSTR s_szShellLinkDotExt = L".LNK";

// ���C���E�B���h�E�N���X���B
static const LPCWSTR s_pszMainWndClass = L"XWord Giver Main Window";

// �q���g�E�B���h�E�N���X���B
static const LPCWSTR s_pszHintsWndClass = L"XWord Giver Hints Window";

// ���E�B���h�E�N���X���B
static const LPCWSTR s_pszCandsWndClass = L"XWord Giver Candidates Window";

// �A�N�Z�����[�^�̃n���h���B
static HACCEL       s_hAccel = nullptr;

// �v���Z�b�T�̐��B
static DWORD        s_dwNumberOfProcessors = 1;

// �v�Z���ԑ���p�B
static DWORD        s_dwTick0;    // �J�n���ԁB
static DWORD        s_dwTick1;    // �Čv�Z���ԁB
static DWORD        s_dwTick2;    // �I�����ԁB
static DWORD        s_dwWait;     // �҂����ԁB

// �f�B�X�N�e�ʂ�����Ȃ����H
static bool         s_bOutOfDiskSpace = false;

// �A�������̏ꍇ�A���𐶐����鐔�B
static int          s_nNumberToGenerate = 16;

// �A�������̏ꍇ�A���𐶐��������B
static int          s_nNumberGenerated = 0;

// �Čv�Z�̉񐔁B
static LONG         s_nRetryCount;

// �c�[���o�[��\�����邩�H
static bool         s_bShowToolBar = true;

// �X�e�[�^�X�o�[��\�����邩�H
static bool         s_bShowStatusBar = true;

// �T�C�Y���w�肵�ĉ摜���R�s�[����Ƃ��̃T�C�Y�B
static int          s_nImageCopyWidth = 250;
static int          s_nImageCopyHeight = 250;
static bool         s_bImageCopyByHeight = false;
static int          s_nMarksHeight = 40;

// ���}�X�p�^�[���œ�����\������B
BOOL xg_bShowAnswerOnPattern = TRUE;

// ���[���Q�B
INT xg_nRules = DEFAULT_RULES_JAPANESE;

//////////////////////////////////////////////////////////////////////////////
// �X�N���[���֘A�B

// �����X�N���[���̈ʒu���擾����B
int __fastcall XgGetHScrollPos(void)
{
    return ::GetScrollPos(xg_hHScrollBar, SB_CTL);
}

// �����X�N���[���̈ʒu���擾����B
int __fastcall XgGetVScrollPos(void)
{
    return ::GetScrollPos(xg_hVScrollBar, SB_CTL);
}

// �����X�N���[���̈ʒu��ݒ肷��B
int __fastcall XgSetHScrollPos(int nPos, BOOL bRedraw)
{
    return ::SetScrollPos(xg_hHScrollBar, SB_CTL, nPos, bRedraw);
}

// �����X�N���[���̈ʒu��ݒ肷��B
int __fastcall XgSetVScrollPos(int nPos, BOOL bRedraw)
{
    return ::SetScrollPos(xg_hVScrollBar, SB_CTL, nPos, bRedraw);
}

// �����X�N���[���̏����擾����B
BOOL __fastcall XgGetHScrollInfo(LPSCROLLINFO psi)
{
    return ::GetScrollInfo(xg_hHScrollBar, SB_CTL, psi);
}

// �����X�N���[���̏����擾����B
BOOL __fastcall XgGetVScrollInfo(LPSCROLLINFO psi)
{
    return ::GetScrollInfo(xg_hVScrollBar, SB_CTL, psi);
}

// �����X�N���[���̏���ݒ肷��B
BOOL __fastcall XgSetHScrollInfo(LPSCROLLINFO psi, BOOL bRedraw)
{
    return ::SetScrollInfo(xg_hHScrollBar, SB_CTL, psi, bRedraw);
}

// �����X�N���[���̏���ݒ肷��B
BOOL __fastcall XgSetVScrollInfo(LPSCROLLINFO psi, BOOL bRedraw)
{
    return ::SetScrollInfo(xg_hVScrollBar, SB_CTL, psi, bRedraw);
}

// �{���̃N���C�A���g�̈���v�Z����B
void __fastcall XgGetRealClientRect(HWND hwnd, LPRECT prcClient)
{
    MRect rc, rcClient;
    ::GetClientRect(hwnd, &rcClient);

    if (::IsWindowVisible(xg_hToolBar)) {
        ::GetWindowRect(xg_hToolBar, &rc);
        rcClient.top += rc.Height();
    }

    if (::IsWindowVisible(xg_hStatusBar)) {
        ::GetWindowRect(xg_hStatusBar, &rc);
        rcClient.bottom -= rc.Height();
    }

    rcClient.right -= ::GetSystemMetrics(SM_CXVSCROLL);
    rcClient.bottom -= ::GetSystemMetrics(SM_CYHSCROLL);

    assert(prcClient);
    *prcClient = rcClient;
}

// �X�N���[������ݒ肷��B
void __fastcall XgUpdateScrollInfo(HWND hwnd, int x, int y)
{
    SIZE siz;
    MRect rcClient;
    SCROLLINFO si;

    // �N���X���[�h�̑傫�����擾����B
    ForDisplay for_display;
    XgGetXWordExtent(&siz);

    // �N���C�A���g�È���擾����B
    XgGetRealClientRect(hwnd, &rcClient);

    // ���X�N���[������ݒ肷��B
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
    XgGetHScrollInfo(&si);
    si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
    si.nMin = 0;
    si.nMax = siz.cx;
    si.nPage = rcClient.Width();
    si.nPos = x;
    XgSetHScrollInfo(&si, TRUE);

    // �c�X�N���[������ݒ肷��B
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
    XgGetVScrollInfo(&si);
    si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
    si.nMin = 0;
    si.nMax = siz.cy;
    si.nPage = rcClient.Height();
    si.nPos = y;
    XgSetVScrollInfo(&si, TRUE);
}

// �L�����b�g��������悤�ɁA�K�v�Ȃ�΃X�N���[������B
void __fastcall XgEnsureCaretVisible(HWND hwnd)
{
    MRect rc, rcClient;
    SCROLLINFO si;
    bool bNeedRedraw = false;

    // �N���C�A���g�̈���擾����B
    ::GetClientRect(hwnd, &rcClient);

    // �c�[���o�[��������Ȃ�A�N���C�A���g�̈��␳����B
    if (::IsWindowVisible(xg_hToolBar)) {
        ::GetWindowRect(xg_hToolBar, &rc);
        rcClient.top += rc.Height();
    }

    if (::IsWindowVisible(xg_hStatusBar)) {
        ::GetWindowRect(xg_hStatusBar, &rc);
        rcClient.bottom -= rc.Height();
    }

    INT nCellSize = xg_nCellSize * xg_nZoomRate / 100;

    // �L�����b�g�̋�`��ݒ肷��B
    ::SetRect(&rc,
        static_cast<int>(xg_nMargin + xg_caret_pos.m_j * nCellSize), 
        static_cast<int>(xg_nMargin + xg_caret_pos.m_i * nCellSize), 
        static_cast<int>(xg_nMargin + (xg_caret_pos.m_j + 1) * nCellSize), 
        static_cast<int>(xg_nMargin + (xg_caret_pos.m_i + 1) * nCellSize));

    // ���X�N���[�������C������B
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    XgGetHScrollInfo(&si);
    if (rc.left < si.nPos) {
        XgSetHScrollPos(rc.left, TRUE);
        bNeedRedraw = true;
    } else if (rc.right > static_cast<int>(si.nPos + si.nPage)) {
        XgSetHScrollPos(rc.right - si.nPage, TRUE);
        bNeedRedraw = true;
    }

    // �c�X�N���[�������C������B
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    XgGetVScrollInfo(&si);
    if (rc.top < si.nPos) {
        XgSetVScrollPos(rc.top, TRUE);
        bNeedRedraw = true;
    } else if (rc.bottom > static_cast<int>(si.nPos + si.nPage)) {
        XgSetVScrollPos(rc.bottom - si.nPage, TRUE);
        bNeedRedraw = true;
    }

    // �ϊ��E�B���h�E�̈ʒu��ݒ肷��B
    COMPOSITIONFORM CompForm;
    CompForm.dwStyle = CFS_POINT;
    CompForm.ptCurrentPos.x = rcClient.left + rc.left - XgGetHScrollPos();
    CompForm.ptCurrentPos.y = rcClient.top + rc.top - XgGetVScrollPos();
    HIMC hIMC = ::ImmGetContext(hwnd);
    ::ImmSetCompositionWindow(hIMC, &CompForm);
    ::ImmReleaseContext(hwnd, hIMC);

    // �K�v�Ȃ�΍ĕ`�悷��B
    if (bNeedRedraw) {
        int x = XgGetHScrollPos();
        int y = XgGetVScrollPos();
        XgUpdateImage(hwnd, x, y);
    }
}

//////////////////////////////////////////////////////////////////////////////

// �c�[���o�[��UI���X�V����B
void __fastcall XgUpdateToolBarUI(HWND hwnd)
{
    ::SendMessageW(xg_hToolBar, TB_ENABLEBUTTON, ID_SOLVE, !xg_bSolved);
    ::SendMessageW(xg_hToolBar, TB_ENABLEBUTTON, ID_SOLVENOADDBLACK, !xg_bSolved);
    ::SendMessageW(xg_hToolBar, TB_ENABLEBUTTON, ID_SOLVEREPEATEDLY, !xg_bSolved);
    ::SendMessageW(xg_hToolBar, TB_ENABLEBUTTON, ID_SOLVEREPEATEDLYNOADDBLACK, !xg_bSolved);
    ::SendMessageW(xg_hToolBar, TB_ENABLEBUTTON, ID_PRINTANSWER, xg_bSolved);
}

//////////////////////////////////////////////////////////////////////////////

BOOL XgLoadDictsFromDir(LPWSTR pszDir)
{
    WCHAR szPath[MAX_PATH];
    WIN32_FIND_DATAW find;
    HANDLE hFind;

    // �t�@�C�� *.dic ��񋓂���B
    StringCbCopy(szPath, sizeof(szPath), pszDir);
    PathAppend(szPath, L"*.dic");
    hFind = FindFirstFileW(szPath, &find);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            StringCbCopy(szPath, sizeof(szPath), pszDir);
            PathAppend(szPath, find.cFileName);
            xg_dict_files.emplace_back(szPath);
        } while (FindNextFile(hFind, &find));
        FindClose(hFind);
    }

    // �t�@�C�� *.tsv ��񋓂���B
    StringCbCopy(szPath, sizeof(szPath), pszDir);
    PathAppend(szPath, L"*.tsv");
    hFind = FindFirstFileW(szPath, &find);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            StringCbCopy(szPath, sizeof(szPath), pszDir);
            PathAppend(szPath, find.cFileName);
            xg_dict_files.emplace_back(szPath);
        } while (FindNextFile(hFind, &find));
        FindClose(hFind);
    }

    return !xg_dict_files.empty();
}

// �����t�@�C�������ׂēǂݍ��ށB
BOOL XgLoadDictsAll(void)
{
    xg_dict_files.clear();

    // ���s�t�@�C���̃p�X���擾�B
    WCHAR sz[MAX_PATH];
    ::GetModuleFileNameW(nullptr, sz, sizeof(sz));

    // ���s�t�@�C���̋߂��ɂ���.dic/.tsv�t�@�C����񋓂���B
    PathRemoveFileSpec(sz);
    PathAppend(sz, L"DICT");
    if (!XgLoadDictsFromDir(sz))
    {
        PathRemoveFileSpec(sz);
        PathRemoveFileSpec(sz);
        PathAppend(sz, L"DICT");
        if (!XgLoadDictsFromDir(sz))
        {
            PathRemoveFileSpec(sz);
            PathAppend(sz, L"DICT");
            XgLoadDictsFromDir(sz);
        }
    }

    // �ǂݍ��񂾒����猩���邩�H
    bool bFound = false;
    for (auto& file : xg_dict_files)
    {
        if (lstrcmpiW(file.c_str(), xg_dict_name.c_str()) == 0)
        {
            bFound = true;
            break;
        }
    }

    if (xg_dict_name.empty() || !bFound)
    {
        LPCWSTR pszNormal = XgLoadStringDx1(IDS_NORMAL_DICT);
        LPCWSTR pszBasicDict = XgLoadStringDx2(IDS_BASICDICTDATA);
        for (auto& file : xg_dict_files)
        {
            if (file.find(pszBasicDict) != std::wstring::npos &&
                file.find(pszNormal) != std::wstring::npos &&
                PathFileExistsW(file.c_str()))
            {
                xg_dict_name = file;
                break;
            }
        }
        if (xg_dict_name.empty() && xg_dict_files.size())
        {
            xg_dict_name = xg_dict_files[0];
        }
    }

    // �t�@�C�������ۂɑ��݂��邩�`�F�b�N���A���݂��Ȃ����ڂ͏����B
    for (size_t i = 0; i < xg_dict_files.size(); ++i) {
        auto& file = xg_dict_files[i];
        if (!PathFileExistsW(file.c_str())) {
            xg_dict_files.erase(xg_dict_files.begin() + i);
            --i;
        }
    }

    return !xg_dict_files.empty();
}

// �ݒ��ǂݍ��ށB
bool __fastcall XgLoadSettings(void)
{
    int i, nDirCount = 0;
    WCHAR sz[MAX_PATH];
    WCHAR szFormat[32];
    DWORD dwValue;

    // ����������B
    s_nMainWndX = CW_USEDEFAULT;
    s_nMainWndY = CW_USEDEFAULT;
    s_nMainWndCX = 475;
    s_nMainWndCY = 515;

    s_nHintsWndX = CW_USEDEFAULT;
    s_nHintsWndY = CW_USEDEFAULT;
    s_nHintsWndCX = 420;
    s_nHintsWndCY = 250;

    s_nCandsWndX = CW_USEDEFAULT;
    s_nCandsWndY = CW_USEDEFAULT;
    s_nCandsWndCX = 420;
    s_nCandsWndCY = 250;

    xg_nInputPaletteWndX = CW_USEDEFAULT;
    xg_nInputPaletteWndY = CW_USEDEFAULT;

    xg_bTateInput = false;
    xg_dict_name.clear();
    s_dirs_save_to.clear();
    s_bAutoRetry = true;
    xg_nRows = xg_nCols = 7;
    xg_szCellFont[0] = 0;
    xg_szSmallFont[0] = 0;
    xg_szUIFont[0] = 0;
    s_nDictSaveMode = 2;
    s_bShowToolBar = true;
    s_bShowStatusBar = true;
    xg_bShowInputPalette = false;
    xg_bSaveAsJsonFile = true;
    s_nImageCopyWidth = 250;
    s_nImageCopyHeight = 250;
    s_bImageCopyByHeight = false;
    s_nMarksHeight = 40;
    xg_bAddThickFrame = true;
    xg_bTateOki = true;
    xg_bCharFeed = true;
    xg_rgbWhiteCellColor = RGB(255, 255, 255);
    xg_rgbBlackCellColor = RGB(0x33, 0x33, 0x33);
    xg_rgbMarkedCellColor = RGB(255, 255, 255);
    xg_bDrawFrameForMarkedCell = TRUE;
    xg_bSmartResolution = TRUE;
    xg_imode = xg_im_KANA;
    xg_nZoomRate = 100;
    xg_bSkeltonMode = FALSE;
    xg_bShowNumbering = TRUE;
    xg_bShowCaret = TRUE;

    xg_bHiragana = FALSE;
    xg_bLowercase = FALSE;

    xg_nCellCharPercents = DEF_CELL_CHAR_SIZE;
    xg_nSmallCharPercents = DEF_SMALL_CHAR_SIZE;

    xg_strBlackCellImage.clear();

    xg_nPatWndX = CW_USEDEFAULT;
    xg_nPatWndY = CW_USEDEFAULT;
    xg_nPatWndCX = CW_USEDEFAULT;
    xg_nPatWndCY = CW_USEDEFAULT;
    xg_bShowAnswerOnPattern = TRUE;

    if (XgIsUserJapanese())
        xg_nRules = DEFAULT_RULES_JAPANESE;
    else
        xg_nRules = DEFAULT_RULES_ENGLISH;

    s_nNumberToGenerate = 16;

    // ��Ж��L�[���J���B
    MRegKey company_key(HKEY_CURRENT_USER, s_pszSoftwareCompanyName, FALSE);
    if (company_key) {
        // �A�v�����L�[���J���B
        MRegKey app_key(company_key, s_pszAppName, FALSE);
        if (app_key) {
            if (!app_key.QueryDword(L"WindowX", dwValue)) {
                s_nMainWndX = dwValue;
            }
            if (!app_key.QueryDword(L"WindowY", dwValue)) {
                s_nMainWndY = dwValue;
            }
            if (!app_key.QueryDword(L"WindowCX", dwValue)) {
                s_nMainWndCX = dwValue;
            }
            if (!app_key.QueryDword(L"WindowCY", dwValue)) {
                s_nMainWndCY = dwValue;
            }

            if (!app_key.QueryDword(L"TateInput", dwValue)) {
                xg_bTateInput = !!dwValue;
            }

            if (!app_key.QueryDword(L"HintsX", dwValue)) {
                s_nHintsWndX = dwValue;
            }
            if (!app_key.QueryDword(L"HintsY", dwValue)) {
                s_nHintsWndY = dwValue;
            }
            if (!app_key.QueryDword(L"HintsCX", dwValue)) {
                s_nHintsWndCX = dwValue;
            }
            if (!app_key.QueryDword(L"HintsCY", dwValue)) {
                s_nHintsWndCY = dwValue;
            }

            if (!app_key.QueryDword(L"CandsX", dwValue)) {
                s_nCandsWndX = dwValue;
            }
            if (!app_key.QueryDword(L"CandsY", dwValue)) {
                s_nCandsWndY = dwValue;
            }
            if (!app_key.QueryDword(L"CandsCX", dwValue)) {
                s_nCandsWndCX = dwValue;
            }
            if (!app_key.QueryDword(L"CandsCY", dwValue)) {
                s_nCandsWndCY = dwValue;
            }

            if (!app_key.QueryDword(L"IPaletteX", dwValue)) {
                xg_nInputPaletteWndX = dwValue;
            }
            if (!app_key.QueryDword(L"IPaletteY", dwValue)) {
                xg_nInputPaletteWndY = dwValue;
            }

            if (!app_key.QueryDword(L"OldNotice", dwValue)) {
                s_bOldNotice = !!dwValue;
            }
            if (!app_key.QueryDword(L"AutoRetry", dwValue)) {
                s_bAutoRetry = !!dwValue;
            }
            if (!app_key.QueryDword(L"Rows", dwValue)) {
                xg_nRows = dwValue;
            }
            if (!app_key.QueryDword(L"Cols", dwValue)) {
                xg_nCols = dwValue;
            }

            if (!app_key.QuerySz(L"CellFont", sz, ARRAYSIZE(sz))) {
                StringCbCopy(xg_szCellFont, sizeof(xg_szCellFont), sz);
            }
            if (!app_key.QuerySz(L"SmallFont", sz, ARRAYSIZE(sz))) {
                StringCbCopy(xg_szSmallFont, sizeof(xg_szSmallFont), sz);
            }
            if (!app_key.QuerySz(L"UIFont", sz, ARRAYSIZE(sz))) {
                StringCbCopy(xg_szUIFont, sizeof(xg_szUIFont), sz);
            }

            if (!app_key.QueryDword(L"ShowToolBar", dwValue)) {
                s_bShowToolBar = !!dwValue;
            }
            if (!app_key.QueryDword(L"ShowStatusBar", dwValue)) {
                s_bShowStatusBar = !!dwValue;
            }
            if (!app_key.QueryDword(L"ShowInputPalette", dwValue)) {
                xg_bShowInputPalette = !!dwValue;
            }

            xg_bSaveAsJsonFile = true;

            if (!app_key.QueryDword(L"NumberToGenerate", dwValue)) {
                s_nNumberToGenerate = dwValue;
            }
            if (!app_key.QueryDword(L"ImageCopyWidth", dwValue)) {
                s_nImageCopyWidth = dwValue;
            }
            if (!app_key.QueryDword(L"ImageCopyHeight", dwValue)) {
                s_nImageCopyHeight = dwValue;
            }
            if (!app_key.QueryDword(L"ImageCopyByHeight", dwValue)) {
                s_bImageCopyByHeight = dwValue;
            }
            if (!app_key.QueryDword(L"MarksHeight", dwValue)) {
                s_nMarksHeight = dwValue;
            }
            if (!app_key.QueryDword(L"AddThickFrame", dwValue)) {
                xg_bAddThickFrame = !!dwValue;
            }

            if (!app_key.QueryDword(L"CharFeed", dwValue)) {
                xg_bCharFeed = !!dwValue;
            }

            if (!app_key.QueryDword(L"TateOki", dwValue)) {
                xg_bTateOki = !!dwValue;
            }
            if (!app_key.QueryDword(L"WhiteCellColor", dwValue)) {
                xg_rgbWhiteCellColor = dwValue;
            }
            if (!app_key.QueryDword(L"BlackCellColor", dwValue)) {
                xg_rgbBlackCellColor = dwValue;
            }
            if (!app_key.QueryDword(L"MarkedCellColor", dwValue)) {
                xg_rgbMarkedCellColor = dwValue;
            }
            if (!app_key.QueryDword(L"DrawFrameForMarkedCell", dwValue)) {
                xg_bDrawFrameForMarkedCell = dwValue;
            }
            if (!app_key.QueryDword(L"SmartResolution", dwValue)) {
                xg_bSmartResolution = dwValue;
            }
            if (!app_key.QueryDword(L"InputMode", dwValue)) {
                xg_imode = (XG_InputMode)dwValue;
            }
            if (!app_key.QueryDword(L"ZoomRate", dwValue)) {
                xg_nZoomRate = dwValue;
            }
            if (!app_key.QueryDword(L"SkeltonMode", dwValue)) {
                xg_bSkeltonMode = dwValue;
            }
            if (!app_key.QueryDword(L"ShowNumbering", dwValue)) {
                xg_bShowNumbering = dwValue;
            }
            if (!app_key.QueryDword(L"ShowCaret", dwValue)) {
                xg_bShowCaret = dwValue;
            }

            if (!app_key.QueryDword(L"Hiragana", dwValue)) {
                xg_bHiragana = !!dwValue;
            }
            if (!app_key.QueryDword(L"Lowercase", dwValue)) {
                xg_bLowercase = !!dwValue;
            }

            if (!app_key.QueryDword(L"CellCharPercents", dwValue)) {
                xg_nCellCharPercents = dwValue;
            }
            if (!app_key.QueryDword(L"SmallCharPercents", dwValue)) {
                xg_nSmallCharPercents = dwValue;
            }

            if (!app_key.QueryDword(L"PatWndX", dwValue)) {
                xg_nPatWndX = dwValue;
            }
            if (!app_key.QueryDword(L"PatWndY", dwValue)) {
                xg_nPatWndY = dwValue;
            }
            if (!app_key.QueryDword(L"PatWndCX", dwValue)) {
                xg_nPatWndCX = dwValue;
            }
            if (!app_key.QueryDword(L"PatWndCY", dwValue)) {
                xg_nPatWndCY = dwValue;
            }
            if (!app_key.QueryDword(L"ShowAnsOnPat", dwValue)) {
                xg_bShowAnswerOnPattern = dwValue;
            }
            if (!app_key.QueryDword(L"Rules", dwValue)) {
                xg_nRules = dwValue;
            }

            if (!app_key.QuerySz(L"Recent", sz, ARRAYSIZE(sz))) {
                xg_dict_name = sz;
                if (!PathFileExists(xg_dict_name.c_str()))
                {
                    xg_dict_name.clear();
                }
            }

            if (!app_key.QuerySz(L"BlackCellImage", sz, ARRAYSIZE(sz))) {
                xg_strBlackCellImage = sz;
                if (!PathFileExists(xg_strBlackCellImage.c_str()))
                {
                    xg_strBlackCellImage.clear();
                }
            }

            // �ۑ���̃��X�g���擾����B
            if (!app_key.QueryDword(L"SaveToCount", dwValue)) {
                nDirCount = dwValue;
                for (i = 0; i < nDirCount; i++) {
                    StringCbPrintf(szFormat, sizeof(szFormat), L"SaveTo %d", i + 1);
                    if (!app_key.QuerySz(szFormat, sz, ARRAYSIZE(sz))) {
                        s_dirs_save_to.emplace_back(sz);
                    } else {
                        nDirCount = i;
                        break;
                    }
                }
            }
        }
    }

    // �ۑ��惊�X�g���󂾂�����A����������B
    if (nDirCount == 0 || s_dirs_save_to.empty()) {
        LPITEMIDLIST pidl;
        WCHAR szPath[MAX_PATH];
        ::SHGetSpecialFolderLocation(nullptr, CSIDL_PERSONAL, &pidl);
        ::SHGetPathFromIDListW(pidl, szPath);
        ::CoTaskMemFree(pidl);
        s_dirs_save_to.emplace_back(szPath);
    }

    ::DeleteObject(xg_hbmBlackCell);
    xg_hbmBlackCell = NULL;

    DeleteEnhMetaFile(xg_hBlackCellEMF);
    xg_hBlackCellEMF = NULL;

    if (!xg_strBlackCellImage.empty())
    {
        xg_hbmBlackCell = LoadBitmapFromFile(xg_strBlackCellImage.c_str());
        if (!xg_hbmBlackCell)
        {
            xg_hBlackCellEMF = GetEnhMetaFile(xg_strBlackCellImage.c_str());
        }

        if (!xg_hbmBlackCell && !xg_hBlackCellEMF)
            xg_strBlackCellImage.clear();
    }

    return true;
}

// �ݒ��ۑ�����B
bool __fastcall XgSaveSettings(void)
{
    int i, nCount;
    WCHAR szFormat[32];

    // ��Ж��L�[���J���B�L�[���Ȃ���΍쐬����B
    MRegKey company_key(HKEY_CURRENT_USER, s_pszSoftwareCompanyName, TRUE);
    if (company_key) {
        // �A�v�����L�[���J���B�L�[���Ȃ���΍쐬����B
        MRegKey app_key(company_key, s_pszAppName, TRUE);
        if (app_key) {
            app_key.SetDword(L"OldNotice", s_bOldNotice);
            app_key.SetDword(L"AutoRetry", s_bAutoRetry);
            app_key.SetDword(L"Rows", xg_nRows);
            app_key.SetDword(L"Cols", xg_nCols);

            app_key.SetSz(L"CellFont", xg_szCellFont, ARRAYSIZE(xg_szCellFont));
            app_key.SetSz(L"SmallFont", xg_szSmallFont, ARRAYSIZE(xg_szSmallFont));
            app_key.SetSz(L"UIFont", xg_szUIFont, ARRAYSIZE(xg_szUIFont));

            app_key.SetDword(L"ShowToolBar", s_bShowToolBar);
            app_key.SetDword(L"ShowStatusBar", s_bShowStatusBar);
            app_key.SetDword(L"ShowInputPalette", xg_bShowInputPalette);

            app_key.SetDword(L"SaveAsJsonFile", xg_bSaveAsJsonFile);
            app_key.SetDword(L"NumberToGenerate", s_nNumberToGenerate);
            app_key.SetDword(L"ImageCopyWidth", s_nImageCopyWidth);
            app_key.SetDword(L"ImageCopyHeight", s_nImageCopyHeight);
            app_key.SetDword(L"ImageCopyByHeight", s_bImageCopyByHeight);
            app_key.SetDword(L"MarksHeight", s_nMarksHeight);
            app_key.SetDword(L"AddThickFrame", xg_bAddThickFrame);
            app_key.SetDword(L"CharFeed", xg_bCharFeed);
            app_key.SetDword(L"TateOki", xg_bTateOki);

            app_key.SetDword(L"WhiteCellColor", xg_rgbWhiteCellColor);
            app_key.SetDword(L"BlackCellColor", xg_rgbBlackCellColor);
            app_key.SetDword(L"MarkedCellColor", xg_rgbMarkedCellColor);

            app_key.SetDword(L"DrawFrameForMarkedCell", xg_bDrawFrameForMarkedCell);
            app_key.SetDword(L"SmartResolution", xg_bSmartResolution);
            app_key.SetDword(L"InputMode", (DWORD)xg_imode);
            app_key.SetDword(L"ZoomRate", xg_nZoomRate);
            app_key.SetDword(L"SkeltonMode", xg_bSkeltonMode);
            app_key.SetDword(L"ShowNumbering", xg_bShowNumbering);
            app_key.SetDword(L"ShowCaret", xg_bShowCaret);

            app_key.SetDword(L"Hiragana", xg_bHiragana);
            app_key.SetDword(L"Lowercase", xg_bLowercase);

            app_key.SetDword(L"CellCharPercents", xg_nCellCharPercents);
            app_key.SetDword(L"SmallCharPercents", xg_nSmallCharPercents);

            app_key.SetDword(L"PatWndX", xg_nPatWndX);
            app_key.SetDword(L"PatWndY", xg_nPatWndY);
            app_key.SetDword(L"PatWndCX", xg_nPatWndCX);
            app_key.SetDword(L"PatWndCY", xg_nPatWndCY);
            app_key.SetDword(L"ShowAnsOnPat", xg_bShowAnswerOnPattern);
            app_key.SetDword(L"Rules", xg_nRules);

            app_key.SetSz(L"Recent", xg_dict_name.c_str());
            app_key.SetSz(L"BlackCellImage", xg_strBlackCellImage.c_str());

            // �ۑ���̃��X�g��ݒ肷��B
            nCount = static_cast<int>(s_dirs_save_to.size());
            app_key.SetDword(L"SaveToCount", nCount);
            for (i = 0; i < nCount; i++)
            {
                StringCbPrintf(szFormat, sizeof(szFormat), L"SaveTo %d", i + 1);
                app_key.SetSz(szFormat, s_dirs_save_to[i].c_str());
            }

            app_key.SetDword(L"HintsX", s_nHintsWndX);
            app_key.SetDword(L"HintsY", s_nHintsWndY);
            app_key.SetDword(L"HintsCX", s_nHintsWndCX);
            app_key.SetDword(L"HintsCY", s_nHintsWndCY);

            app_key.SetDword(L"CandsX", s_nCandsWndX);
            app_key.SetDword(L"CandsY", s_nCandsWndY);
            app_key.SetDword(L"CandsCX", s_nCandsWndCX);
            app_key.SetDword(L"CandsCY", s_nCandsWndCY);

            app_key.SetDword(L"IPaletteX", xg_nInputPaletteWndX);
            app_key.SetDword(L"IPaletteY", xg_nInputPaletteWndY);

            app_key.SetDword(L"WindowX", s_nMainWndX);
            app_key.SetDword(L"WindowY", s_nMainWndY);
            app_key.SetDword(L"WindowCX", s_nMainWndCX);
            app_key.SetDword(L"WindowCY", s_nMainWndCY);

            app_key.SetDword(L"TateInput", xg_bTateInput);
        }
    }

    return true;
}

// �ݒ����������B
bool __fastcall XgEraseSettings(void)
{
    // ��Ж��L�[���J���B
    RegDeleteTreeDx(HKEY_CURRENT_USER, s_pszSoftwareCompanyAndApp);

    return true;
}

//////////////////////////////////////////////////////////////////////////////

// �u�q���g�̓��́v�_�C�A���O�B
extern "C"
INT_PTR CALLBACK
XgInputHintDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WCHAR sz[512];
    static std::wstring s_word;

    switch (uMsg) {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��Ɋ񂹂�B
        XgCenterDialog(hwnd);

        // �_�C�A���O������������B
        s_word = *reinterpret_cast<std::wstring *>(lParam);
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_REGISTERWORD), s_word.data(), s_word.data());
        ::SetDlgItemTextW(hwnd, stc1, sz);

        // �q���g���ǉ����ꂽ�B
        xg_bHintsAdded = true;
        return true;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            // �e�L�X�g���擾����B
            ::GetDlgItemTextW(hwnd, edt1, sz, ARRAYSIZE(sz));

            // �����f�[�^�ɒǉ�����B
            xg_dict_1.emplace_back(s_word, sz);

            // �񕪒T���̂��߂ɁA���ёւ��Ă����B
            XgSortAndUniqueDictData(xg_dict_1);

            // �_�C�A���O���I���B
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            // �_�C�A���O���I���B
            ::EndDialog(hwnd, IDCANCEL);
            break;
        }
        break;
    }

    return false;
}

// �N���X���[�h���`�F�b�N����B
bool __fastcall XgCheckCrossWord(HWND hwnd, bool check_words = true)
{
    // �l�����ցB
    if ((xg_nRules & RULE_DONTCORNERBLACK) && xg_xword.CornerBlack()) {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CORNERBLOCK), nullptr, MB_ICONERROR);
        return false;
    }

    // �A���ցB
    if ((xg_nRules & RULE_DONTDOUBLEBLACK) && xg_xword.DoubleBlack()) {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ADJACENTBLOCK), nullptr, MB_ICONERROR);
        return false;
    }

    // �O�����ցB
    if ((xg_nRules & RULE_DONTTRIDIRECTIONS) && xg_xword.TriBlackAround()) {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_TRIBLOCK), nullptr, MB_ICONERROR);
        return false;
    }

    // ���f�ցB
    if ((xg_nRules & RULE_DONTDIVIDE) && xg_xword.DividedByBlack()) {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_DIVIDED), nullptr, MB_ICONERROR);
        return false;
    }

    // ���ΎO�A�ցB
    if (xg_nRules & RULE_DONTTHREEDIAGONALS) {
        if (xg_xword.ThreeDiagonals()) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_THREEDIAGONALS), nullptr, MB_ICONERROR);
            return false;
        }
    } else if (xg_nRules & RULE_DONTFOURDIAGONALS) {
        // ���Ύl�A�ցB
        if (xg_xword.FourDiagonals()) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_FOURDIAGONALS), nullptr, MB_ICONERROR);
            return false;
        }
    }

    // ���}�X�_�Ώ́B
    if ((xg_nRules & RULE_POINTSYMMETRY) && !xg_xword.IsPointSymmetry()) {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_NOTPOINTSYMMETRY), nullptr, MB_ICONERROR);
        return false;
    }

    // �N���X���[�h�Ɋ܂܂��P��̃`�F�b�N�B
    XG_Pos pos;
    std::vector<std::wstring> vNotFoundWords;
    XG_EpvCode code = xg_xword.EveryPatternValid1(vNotFoundWords, pos, xg_bNoAddBlack);
    if (code == xg_epv_PATNOTMATCH) {
        if (check_words) {
            // �p�^�[���Ƀ}�b�`���Ȃ��}�X���������B
            WCHAR sz[128];
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_NOCANDIDATE), pos.m_i + 1, pos.m_j + 1);
            XgCenterMessageBoxW(hwnd, sz, nullptr, MB_ICONERROR);
            return false;
        }
    } else if (code == xg_epv_DOUBLEWORD) {
        // ���łɎg�p�����P�ꂪ�������B
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_DOUBLEDWORD), nullptr, MB_ICONERROR);
        return false;
    } else if (code == xg_epv_LENGTHMISMATCH) {
        if (check_words) {
            // �o�^����Ă���P��ƒ����̈�v���Ȃ��X�y�[�X���������B
            WCHAR sz[128];
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_TOOLONGSPACE), pos.m_i + 1, pos.m_j + 1);
            XgCenterMessageBoxW(hwnd, sz, nullptr, MB_ICONERROR);
            return false;
        }
    }

    // ������Ȃ������P�ꂪ���邩�H
    if (!vNotFoundWords.empty()) {
        if (check_words) {
            // �P�ꂪ�o�^����Ă��Ȃ��B
            for (auto& word : vNotFoundWords) {
                // �q���g�̓��͂𑣂��B
                if (::DialogBoxParamW(xg_hInstance, MAKEINTRESOURCE(IDD_INPUTHINT),
                                      hwnd, XgInputHintDlgProc,
                                      reinterpret_cast<LPARAM>(&word)) != IDOK)
                {
                    // �L�����Z�����ꂽ�B
                    return false;
                }
            }
        }
    }

    // �����B
    return true;
}

// ���������Z�b�g����B
void XgSetDict(const std::wstring& strFile)
{
    // ���������i�[�B
    xg_dict_name = strFile;

    // �����Ƃ��Ēǉ��A�\�[�g�A��ӂɂ���B
    if (xg_dict_files.size() < MAX_DICTS)
    {
        xg_dict_files.emplace_back(strFile);
        std::sort(xg_dict_files.begin(), xg_dict_files.end());
        auto last = std::unique(xg_dict_files.begin(), xg_dict_files.end());
        xg_dict_files.erase(last, xg_dict_files.end());
    }
}

//////////////////////////////////////////////////////////////////////////////

// �Ղ����̕����Ŗ��ߐs�����B
void __fastcall XgNewCells(HWND hwnd, WCHAR ch, INT nRows, INT nCols)
{
    auto sa1 = std::make_shared<XG_UndoData_SetAll>();
    auto sa2 = std::make_shared<XG_UndoData_SetAll>();
    sa1->Get();
    // ���E�B���h�E��j������B
    XgDestroyCandsWnd();
    // �q���g�E�B���h�E��j������B
    XgDestroyHintsWnd();
    // ����������B
    xg_bSolved = false;
    xg_nRows = nRows;
    xg_nCols = nCols;
    xg_xword.ResetAndSetSize(xg_nRows, xg_nCols);
    xg_bHintsAdded = false;
    xg_bShowAnswer = false;
    xg_caret_pos.clear();
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_strHeader.clear();
    xg_strNotes.clear();
    xg_strFileName.clear();
    xg_vMarks.clear();
    xg_vMarkedCands.clear();
    for (INT iRow = 0; iRow < xg_nRows; ++iRow) {
        for (INT jCol = 0; jCol < xg_nCols; ++jCol) {
            xg_xword.SetAt(iRow, jCol, ch);
        }
    }
    // �C���[�W���X�V����B
    XgMarkUpdate();
    XgUpdateImage(hwnd, 0, 0);
    // �u���ɖ߂��v����ݒ肷��B
    sa2->Get();
    xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
    // �c�[���o�[��UI���X�V����B
    XgUpdateToolBarUI(hwnd);
}

// �Ղ̃T�C�Y��ύX����B
void __fastcall XgResizeCells(HWND hwnd, INT nNewRows, INT nNewCols)
{
    auto sa1 = std::make_shared<XG_UndoData_SetAll>();
    auto sa2 = std::make_shared<XG_UndoData_SetAll>();
    sa1->Get();
    // ���E�B���h�E��j������B
    XgDestroyCandsWnd();
    // �q���g�E�B���h�E��j������B
    XgDestroyHintsWnd();
    // �}�[�N�̍X�V��ʒm����B
    XgMarkUpdate();
    // �T�C�Y��ύX����B
    INT nOldRows = xg_nRows, nOldCols = xg_nCols;
    INT iMin = std::min(xg_nRows, nNewRows);
    INT jMin = std::min(xg_nCols, nNewCols);
    XG_Board copy;
    copy.ResetAndSetSize(nNewRows, nNewCols);
    for (INT i = 0; i < iMin; ++i) {
        for (INT j = 0; j < jMin; ++j) {
            xg_nRows = nOldRows;
            xg_nCols = nOldCols;
            WCHAR ch = xg_xword.GetAt(i, j);
            xg_nRows = nNewRows;
            xg_nCols = nNewCols;
            copy.SetAt(i, j, ch);
        }
    }
    xg_nRows = nNewRows;
    xg_nCols = nNewCols;
    xg_xword = copy;
    xg_bSolved = false;
    xg_bHintsAdded = false;
    xg_bShowAnswer = false;
    xg_caret_pos.clear();
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_strHeader.clear();
    xg_strNotes.clear();
    xg_strFileName.clear();
    xg_vMarks.clear();
    xg_vMarkedCands.clear();
    // �C���[�W���X�V����B
    XgMarkUpdate();
    XgUpdateImage(hwnd, 0, 0);
    // �u���ɖ߂��v����ݒ肷��B
    sa2->Get();
    xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
    // �c�[���o�[��UI���X�V����B
    XgUpdateToolBarUI(hwnd);
}

// [�V�K�쐬]�_�C�A���O�̃_�C�A���O �v���V�[�W���B
extern "C" INT_PTR CALLBACK
XgNewDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    INT n1, n2;
    switch (uMsg) {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �T�C�Y�̗���ݒ肷��B
        ::SetDlgItemInt(hwnd, edt1, xg_nRows, FALSE);
        ::SetDlgItemInt(hwnd, edt2, xg_nCols, FALSE);
        // IME��OFF�ɂ���B
        {
            HWND hwndCtrl;

            hwndCtrl = ::GetDlgItem(hwnd, edt1);
            ::ImmAssociateContext(hwndCtrl, NULL);

            hwndCtrl = ::GetDlgItem(hwnd, edt2);
            ::ImmAssociateContext(hwndCtrl, NULL);
        }
        SendDlgItemMessageW(hwnd, scr1, UDM_SETRANGE, 0, MAKELPARAM(XG_MAX_SIZE, XG_MIN_SIZE));
        SendDlgItemMessageW(hwnd, scr2, UDM_SETRANGE, 0, MAKELPARAM(XG_MAX_SIZE, XG_MIN_SIZE));
        CheckRadioButton(hwnd, rad1, rad3, rad1);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            // �T�C�Y�̗����`�F�b�N����B
            n1 = static_cast<int>(::GetDlgItemInt(hwnd, edt1, nullptr, FALSE));
            if (n1 < XG_MIN_SIZE || n1 > XG_MAX_SIZE) {
                ::SendDlgItemMessageW(hwnd, edt1, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt1));
                return 0;
            }
            n2 = static_cast<int>(::GetDlgItemInt(hwnd, edt2, nullptr, FALSE));
            if (n2 < XG_MIN_SIZE || n2 > XG_MAX_SIZE) {
                ::SendDlgItemMessageW(hwnd, edt2, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt2));
                return 0;
            }
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDOK);
            // �������s���B
            if (IsDlgButtonChecked(hwnd, rad1) == BST_CHECKED) {
                // �Ղ����̕����Ŗ��ߐs�����B
                XgNewCells(xg_hMainWnd, ZEN_SPACE, n1, n2);
            } else if (IsDlgButtonChecked(hwnd, rad2) == BST_CHECKED) {
                // �Ղ����̕����Ŗ��ߐs�����B
                XgNewCells(xg_hMainWnd, ZEN_BLACK, n1, n2);
            } else if (IsDlgButtonChecked(hwnd, rad3) == BST_CHECKED) {
                // �Ղ̃T�C�Y��ύX����B
                XgResizeCells(xg_hMainWnd, n1, n2);
            }
            break;

        case IDCANCEL:
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

// [���̍쐬]�_�C�A���O�̃_�C�A���O �v���V�[�W���B
extern "C" INT_PTR CALLBACK
XgGenerateDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    INT n1, n2, n3;

    switch (uMsg) {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �T�C�Y�̗���ݒ肷��B
        ::SetDlgItemInt(hwnd, edt1, xg_nRows, FALSE);
        ::SetDlgItemInt(hwnd, edt2, xg_nCols, FALSE);

        // �X�P���g�����[�h�Ɠ��̓��[�h�ɉ����ĒP��̍ő咷��ݒ肷��B
        if (xg_imode == xg_im_KANJI) {
            n3 = 4;
        } else if (xg_bSkeltonMode) {
            n3 = 6;
        } else if (xg_imode == xg_im_RUSSIA || xg_imode == xg_im_ABC) {
            n3 = 5;
        } else if (xg_imode == xg_im_DIGITS) {
            n3 = 7;
        } else {
            n3 = 4;
        }
        ::SetDlgItemInt(hwnd, edt3, n3, FALSE);

        // �����ōČv�Z�����邩�H
        if (s_bAutoRetry)
            ::CheckDlgButton(hwnd, chx1, BST_CHECKED);
        // �X�}�[�g�������H
        if (xg_bSmartResolution)
            ::CheckDlgButton(hwnd, chx2, BST_CHECKED);
        // IME��OFF�ɂ���B
        {
            HWND hwndCtrl;

            hwndCtrl = ::GetDlgItem(hwnd, edt1);
            ::ImmAssociateContext(hwndCtrl, NULL);

            hwndCtrl = ::GetDlgItem(hwnd, edt2);
            ::ImmAssociateContext(hwndCtrl, NULL);
        }
        SendDlgItemMessageW(hwnd, scr1, UDM_SETRANGE, 0, MAKELPARAM(XG_MAX_SIZE, XG_MIN_SIZE));
        SendDlgItemMessageW(hwnd, scr2, UDM_SETRANGE, 0, MAKELPARAM(XG_MAX_SIZE, XG_MIN_SIZE));
        SendDlgItemMessageW(hwnd, scr3, UDM_SETRANGE, 0, MAKELPARAM(10, 4));
        if (::IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED) {
            EnableWindow(GetDlgItem(hwnd, stc1), TRUE);
            EnableWindow(GetDlgItem(hwnd, edt3), TRUE);
            EnableWindow(GetDlgItem(hwnd, scr3), TRUE);
        } else {
            EnableWindow(GetDlgItem(hwnd, stc1), FALSE);
            EnableWindow(GetDlgItem(hwnd, edt3), FALSE);
            EnableWindow(GetDlgItem(hwnd, scr3), FALSE);
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            // �T�C�Y�̗����`�F�b�N����B
            n1 = static_cast<int>(::GetDlgItemInt(hwnd, edt1, nullptr, FALSE));
            if (n1 < XG_MIN_SIZE || n1 > XG_MAX_SIZE) {
                ::SendDlgItemMessageW(hwnd, edt1, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt1));
                return 0;
            }
            n2 = static_cast<int>(::GetDlgItemInt(hwnd, edt2, nullptr, FALSE));
            if (n2 < XG_MIN_SIZE || n2 > XG_MAX_SIZE) {
                ::SendDlgItemMessageW(hwnd, edt2, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt2));
                return 0;
            }
            n3 = static_cast<int>(::GetDlgItemInt(hwnd, edt3, nullptr, FALSE));
            if (n3 < 4 || n3 > 10) {
                ::SendDlgItemMessageW(hwnd, edt3, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt3));
                return 0;
            }
            xg_nMaxWordLen = n3;
            // �����ōČv�Z�����邩�H
            s_bAutoRetry = (::IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED);
            // �X�}�[�g�������H
            xg_bSmartResolution = (::IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED);
            // ����������B
            {
                xg_bSolved = false;
                xg_bShowAnswer = false;
                xg_xword.ResetAndSetSize(n1, n2);
                xg_nRows = n1;
                xg_nCols = n2;
                xg_vTateInfo.clear();
                xg_vYokoInfo.clear();
                xg_vMarks.clear();
                xg_vMarkedCands.clear();
            }
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDCANCEL);
            break;

        case chx2:
            if (::IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED) {
                EnableWindow(GetDlgItem(hwnd, stc1), TRUE);
                EnableWindow(GetDlgItem(hwnd, edt3), TRUE);
                EnableWindow(GetDlgItem(hwnd, scr3), TRUE);
            } else {
                EnableWindow(GetDlgItem(hwnd, stc1), FALSE);
                EnableWindow(GetDlgItem(hwnd, edt3), FALSE);
                EnableWindow(GetDlgItem(hwnd, scr3), FALSE);
            }
            break;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

// �ۑ���B
WCHAR xg_szDir[MAX_PATH] = L"";

// �u�ۑ���v�Q�ƁB
extern "C"
int CALLBACK XgBrowseCallbackProc(
    HWND hwnd,
    UINT uMsg,
    LPARAM /*lParam*/,
    LPARAM /*lpData*/)
{
    if (uMsg == BFFM_INITIALIZED) {
        // �������̍ۂɁA�t�H���_�[�̏ꏊ���w�肷��B
        ::SendMessageW(hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(xg_szDir));
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

// [���̘A���쐬]�_�C�A���O�̃_�C�A���O �v���V�[�W���B
extern "C" INT_PTR CALLBACK
XgGenerateRepeatedlyDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    INT n1, n2, n3;
    WCHAR szFile[MAX_PATH];
    std::wstring strDir;
    COMBOBOXEXITEMW item;
    BROWSEINFOW bi;
    DWORD attrs;
    LPITEMIDLIST pidl;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �T�C�Y�̗���ݒ肷��B
        ::SetDlgItemInt(hwnd, edt1, xg_nRows, FALSE);
        ::SetDlgItemInt(hwnd, edt2, xg_nCols, FALSE);
        // �X�P���g�����[�h�Ɠ��̓��[�h�ɉ����ĒP��̍ő咷��ݒ肷��B
        if (xg_imode == xg_im_KANJI) {
            n3 = 4;
        } else if (xg_bSkeltonMode) {
            n3 = 6;
        } else if (xg_imode == xg_im_RUSSIA || xg_imode == xg_im_ABC) {
            n3 = 5;
        } else if (xg_imode == xg_im_DIGITS) {
            n3 = 7;
        } else {
            n3 = 4;
        }
        ::SetDlgItemInt(hwnd, edt3, n3, FALSE);
        // �X�}�[�g�������H
        if (xg_bSmartResolution) {
            ::CheckDlgButton(hwnd, chx2, BST_CHECKED);
        } else {
            ::CheckDlgButton(hwnd, chx2, BST_UNCHECKED);
        }
        // �ۑ����ݒ肷��B
        for (const auto& dir : s_dirs_save_to) {
            item.mask = CBEIF_TEXT;
            item.iItem = -1;
            StringCbCopy(szFile, sizeof(szFile), dir.data());
            item.pszText = szFile;
            item.cchTextMax = -1;
            ::SendDlgItemMessageW(hwnd, cmb2, CBEM_INSERTITEMW, 0,
                                  reinterpret_cast<LPARAM>(&item));
        }
        // �R���{�{�b�N�X�̍ŏ��̍��ڂ�I������B
        ::SendDlgItemMessageW(hwnd, cmb2, CB_SETCURSEL, 0, 0);
        // �����ōČv�Z�����邩�H
        if (s_bAutoRetry)
            ::CheckDlgButton(hwnd, chx1, BST_CHECKED);
        // �������鐔��ݒ肷��B
        ::SetDlgItemInt(hwnd, edt4, s_nNumberToGenerate, FALSE);
        // IME��OFF�ɂ���B
        {
            HWND hwndCtrl = ::GetDlgItem(hwnd, edt1);
            ::ImmAssociateContext(hwndCtrl, NULL);
            hwndCtrl = ::GetDlgItem(hwnd, edt2);
            ::ImmAssociateContext(hwndCtrl, NULL);
            hwndCtrl = ::GetDlgItem(hwnd, edt3);
            ::ImmAssociateContext(hwndCtrl, NULL);
            hwndCtrl = ::GetDlgItem(hwnd, edt4);
            ::ImmAssociateContext(hwndCtrl, NULL);
        }
        SendDlgItemMessageW(hwnd, scr1, UDM_SETRANGE, 0, MAKELPARAM(XG_MAX_SIZE, XG_MIN_SIZE));
        SendDlgItemMessageW(hwnd, scr2, UDM_SETRANGE, 0, MAKELPARAM(XG_MAX_SIZE, XG_MIN_SIZE));
        SendDlgItemMessageW(hwnd, scr3, UDM_SETRANGE, 0, MAKELPARAM(10, 4));
        SendDlgItemMessageW(hwnd, scr4, UDM_SETRANGE, 0, MAKELPARAM(100, 1));
        if (::IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED) {
            EnableWindow(GetDlgItem(hwnd, stc1), TRUE);
            EnableWindow(GetDlgItem(hwnd, edt3), TRUE);
            EnableWindow(GetDlgItem(hwnd, scr3), TRUE);
        } else {
            EnableWindow(GetDlgItem(hwnd, stc1), FALSE);
            EnableWindow(GetDlgItem(hwnd, edt3), FALSE);
            EnableWindow(GetDlgItem(hwnd, scr3), FALSE);
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            // �T�C�Y�̗����`�F�b�N����B
            n1 = static_cast<int>(::GetDlgItemInt(hwnd, edt1, nullptr, FALSE));
            if (n1 < XG_MIN_SIZE || n1 > XG_MAX_SIZE) {
                ::SendDlgItemMessageW(hwnd, edt1, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt1));
                return 0;
            }
            n2 = static_cast<int>(::GetDlgItemInt(hwnd, edt2, nullptr, FALSE));
            if (n2 < XG_MIN_SIZE || n2 > XG_MAX_SIZE) {
                ::SendDlgItemMessageW(hwnd, edt2, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt2));
                return 0;
            }
            n3 = static_cast<int>(::GetDlgItemInt(hwnd, edt3, nullptr, FALSE));
            if (n3 < 4 || n3 > 10) {
                ::SendDlgItemMessageW(hwnd, edt3, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt3));
                return 0;
            }
            xg_nMaxWordLen = n3;
            // �����ōČv�Z�����邩�H
            s_bAutoRetry = (::IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED);
            // �X�}�[�g�������H
            xg_bSmartResolution = (::IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED);
            // �ۑ���̃p�X�����擾����B
            ::GetDlgItemTextW(hwnd, cmb2, szFile, ARRAYSIZE(szFile));
            attrs = ::GetFileAttributesW(szFile);
            if (attrs == 0xFFFFFFFF || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                // �p�X���Ȃ���΍쐬����B
                if (!XgMakePathW(szFile)) {
                    // �쐬�Ɏ��s�B
                    ::SendDlgItemMessageW(hwnd, cmb2, CB_SETEDITSEL, 0, -1);
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_STORAGEINVALID), nullptr, MB_ICONERROR);
                    ::SetFocus(::GetDlgItem(hwnd, cmb2));
                    return 0;
                }
            }
            // �ۑ�����Z�b�g����B
            strDir = szFile;
            xg_str_trim(strDir);
            {
                auto end = s_dirs_save_to.end();
                for (auto it = s_dirs_save_to.begin(); it != end; it++) {
                    if (_wcsicmp((*it).data(), strDir.data()) == 0) {
                        s_dirs_save_to.erase(it);
                        break;
                    }
                }
                s_dirs_save_to.emplace_front(strDir);
            }
            // ���̐����擾����B
            {
                // �������ł͂Ȃ��B
                BOOL bTranslated;
                s_nNumberToGenerate = ::GetDlgItemInt(hwnd, edt4, &bTranslated, FALSE);
                if (!bTranslated || s_nNumberToGenerate == 0) {
                    ::SendDlgItemMessageW(hwnd, edt4, EM_SETSEL, 0, -1);
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERPOSITIVE), nullptr, MB_ICONERROR);
                    ::SetFocus(::GetDlgItem(hwnd, edt4));
                    return 0;
                }
            }
            // JSON�`���Ƃ��ĕۑ����邩�H
            xg_bSaveAsJsonFile = true;
            // ����������B
            {
                xg_bSolved = false;
                xg_bShowAnswer = false;
                xg_xword.ResetAndSetSize(n1, n2);
                xg_nRows = n1;
                xg_nCols = n2;
                xg_vTateInfo.clear();
                xg_vYokoInfo.clear();
                xg_vMarks.clear();
                xg_vMarkedCands.clear();
            }
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDCANCEL);
            break;

        case chx2:
            if (::IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED) {
                EnableWindow(GetDlgItem(hwnd, stc1), TRUE);
                EnableWindow(GetDlgItem(hwnd, edt3), TRUE);
                EnableWindow(GetDlgItem(hwnd, scr3), TRUE);
            } else {
                EnableWindow(GetDlgItem(hwnd, stc1), FALSE);
                EnableWindow(GetDlgItem(hwnd, edt3), FALSE);
                EnableWindow(GetDlgItem(hwnd, scr3), FALSE);
            }
            break;

        case psh2:
            // ���[�U�[�ɕۑ���̏ꏊ��₢���킹��B
            ZeroMemory(&bi, sizeof(bi));
            bi.hwndOwner = hwnd;
            bi.lpszTitle = XgLoadStringDx1(IDS_CROSSSTORAGE);
            bi.ulFlags = BIF_RETURNONLYFSDIRS;
            bi.lpfn = XgBrowseCallbackProc;
            ::GetDlgItemTextW(hwnd, cmb2, xg_szDir, ARRAYSIZE(xg_szDir));
            pidl = ::SHBrowseForFolderW(&bi);
            if (pidl) {
                // �p�X���R���{�{�b�N�X�ɐݒ�B
                ::SHGetPathFromIDListW(pidl, szFile);
                ::SetDlgItemTextW(hwnd, cmb2, szFile);
                ::CoTaskMemFree(pidl);
            }
            break;
        }
    }
    return 0;
}

// [���}�X�p�^�[���̘A���쐬]�_�C�A���O�̃_�C�A���O �v���V�[�W���B
extern "C" INT_PTR CALLBACK
XgGenerateBlacksRepeatedlyDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    INT n3;
    WCHAR szFile[MAX_PATH];
    std::wstring strDir;
    COMBOBOXEXITEMW item;
    BROWSEINFOW bi;
    DWORD attrs;
    LPITEMIDLIST pidl;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �X�P���g�����[�h�Ɠ��̓��[�h�ɉ����ĒP��̍ő咷��ݒ肷��B
        if (xg_imode == xg_im_KANJI) {
            n3 = 4;
        } else if (xg_bSkeltonMode) {
            n3 = 6;
        } else if (xg_imode == xg_im_RUSSIA || xg_imode == xg_im_ABC) {
            n3 = 5;
        } else if (xg_imode == xg_im_DIGITS) {
            n3 = 7;
        } else {
            n3 = 4;
        }
        ::SetDlgItemInt(hwnd, edt3, n3, FALSE);
        // �ۑ����ݒ肷��B
        for (const auto& dir : s_dirs_save_to) {
            item.mask = CBEIF_TEXT;
            item.iItem = -1;
            StringCbCopy(szFile, sizeof(szFile), dir.data());
            item.pszText = szFile;
            item.cchTextMax = -1;
            ::SendDlgItemMessageW(hwnd, cmb2, CBEM_INSERTITEMW, 0,
                                  reinterpret_cast<LPARAM>(&item));
        }
        // �R���{�{�b�N�X�̍ŏ��̍��ڂ�I������B
        ::SendDlgItemMessageW(hwnd, cmb2, CB_SETCURSEL, 0, 0);
        // �������鐔��ݒ肷��B
        ::SetDlgItemInt(hwnd, edt4, s_nNumberToGenerate, FALSE);
        // IME��OFF�ɂ���B
        {
            HWND hwndCtrl = ::GetDlgItem(hwnd, edt3);
            ::ImmAssociateContext(hwndCtrl, NULL);
            hwndCtrl = ::GetDlgItem(hwnd, edt4);
            ::ImmAssociateContext(hwndCtrl, NULL);
        }
        SendDlgItemMessageW(hwnd, scr3, UDM_SETRANGE, 0, MAKELPARAM(10, 4));
        SendDlgItemMessageW(hwnd, scr4, UDM_SETRANGE, 0, MAKELPARAM(100, 1));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            n3 = static_cast<int>(::GetDlgItemInt(hwnd, edt3, nullptr, FALSE));
            if (n3 < 4 || n3 > 10) {
                ::SendDlgItemMessageW(hwnd, edt3, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt3));
                return 0;
            }
            xg_nMaxWordLen = n3;
            // �ۑ���̃p�X�����擾����B
            ::GetDlgItemTextW(hwnd, cmb2, szFile, ARRAYSIZE(szFile));
            attrs = ::GetFileAttributesW(szFile);
            if (attrs == 0xFFFFFFFF || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                // �p�X���Ȃ���΍쐬����B
                if (!XgMakePathW(szFile)) {
                    // �쐬�Ɏ��s�B
                    ::SendDlgItemMessageW(hwnd, cmb2, CB_SETEDITSEL, 0, -1);
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_STORAGEINVALID), nullptr, MB_ICONERROR);
                    ::SetFocus(::GetDlgItem(hwnd, cmb2));
                    return 0;
                }
            }
            // �ۑ�����Z�b�g����B
            strDir = szFile;
            xg_str_trim(strDir);
            {
                auto end = s_dirs_save_to.end();
                for (auto it = s_dirs_save_to.begin(); it != end; it++) {
                    if (_wcsicmp((*it).data(), strDir.data()) == 0) {
                        s_dirs_save_to.erase(it);
                        break;
                    }
                }
                s_dirs_save_to.emplace_front(strDir);
            }
            // ���̐����擾����B
            {
                // �������ł͂Ȃ��B
                BOOL bTranslated;
                s_nNumberToGenerate = ::GetDlgItemInt(hwnd, edt4, &bTranslated, FALSE);
                if (!bTranslated || s_nNumberToGenerate == 0) {
                    ::SendDlgItemMessageW(hwnd, edt4, EM_SETSEL, 0, -1);
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERPOSITIVE), nullptr, MB_ICONERROR);
                    ::SetFocus(::GetDlgItem(hwnd, edt4));
                    return 0;
                }
            }
            // JSON�`���Ƃ��ĕۑ����邩�H
            xg_bSaveAsJsonFile = true;
            // ����������B
            {
                xg_bSolved = false;
                xg_bShowAnswer = false;
                xg_xword.clear();
                xg_vTateInfo.clear();
                xg_vYokoInfo.clear();
                xg_vMarks.clear();
                xg_vMarkedCands.clear();
            }
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDCANCEL);
            break;

        case psh2:
            // ���[�U�[�ɕۑ���̏ꏊ��₢���킹��B
            ZeroMemory(&bi, sizeof(bi));
            bi.hwndOwner = hwnd;
            bi.lpszTitle = XgLoadStringDx1(IDS_CROSSSTORAGE);
            bi.ulFlags = BIF_RETURNONLYFSDIRS;
            bi.lpfn = XgBrowseCallbackProc;
            ::GetDlgItemTextW(hwnd, cmb2, xg_szDir, ARRAYSIZE(xg_szDir));
            pidl = ::SHBrowseForFolderW(&bi);
            if (pidl) {
                // �p�X���R���{�{�b�N�X�ɐݒ�B
                ::SHGetPathFromIDListW(pidl, szFile);
                ::SetDlgItemTextW(hwnd, cmb2, szFile);
                ::CoTaskMemFree(pidl);
            }
            break;
        }
    }
    return 0;
}

// [���}�X�p�^�[���̍쐬]�_�C�A���O�̃_�C�A���O �v���V�[�W���B
extern "C" INT_PTR CALLBACK
XgGenerateBlacksDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    INT n3;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �X�P���g�����[�h�Ɠ��̓��[�h�ɉ����ĒP��̍ő咷��ݒ肷��B
        if (xg_imode == xg_im_KANJI) {
            n3 = 4;
        } else if (xg_bSkeltonMode) {
            n3 = 6;
        } else if (xg_imode == xg_im_RUSSIA || xg_imode == xg_im_ABC) {
            n3 = 5;
        } else if (xg_imode == xg_im_DIGITS) {
            n3 = 7;
        } else {
            n3 = 4;
        }
        ::SetDlgItemInt(hwnd, edt3, n3, FALSE);
        SendDlgItemMessageW(hwnd, scr3, UDM_SETRANGE, 0, MAKELPARAM(10, 4));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            n3 = static_cast<int>(::GetDlgItemInt(hwnd, edt3, nullptr, FALSE));
            if (n3 < 4 || n3 > 10) {
                ::SendDlgItemMessageW(hwnd, edt3, EM_SETSEL, 0, -1);
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERINT), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, edt3));
                return 0;
            }
            xg_nMaxWordLen = n3;
            // ����������B
            {
                xg_bSolved = false;
                xg_bShowAnswer = false;
                xg_xword.clear();
                xg_vTateInfo.clear();
                xg_vYokoInfo.clear();
                xg_vMarks.clear();
                xg_vMarkedCands.clear();
            }
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
    return 0;
}

// [���̘A���쐬]�_�C�A���O�̃_�C�A���O �v���V�[�W���[�B
extern "C" INT_PTR CALLBACK
XgSolveRepeatedlyDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    int i;
    WCHAR szFile[MAX_PATH], szTarget[MAX_PATH];
    std::wstring strFile, strDir;
    COMBOBOXEXITEMW item;
    HDROP hDrop;
    BROWSEINFOW bi;
    DWORD attrs;
    LPITEMIDLIST pidl;

    switch (uMsg) {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �ۑ����ݒ肷��B
        for (const auto& dir : s_dirs_save_to) {
            item.mask = CBEIF_TEXT;
            item.iItem = -1;
            StringCbCopy(szFile, sizeof(szFile), dir.data());
            item.pszText = szFile;
            item.cchTextMax = -1;
            ::SendDlgItemMessageW(hwnd, cmb1, CBEM_INSERTITEMW, 0,
                                  reinterpret_cast<LPARAM>(&item));
        }
        // �R���{�{�b�N�X�̍ŏ��̍��ڂ�I������B
        ::SendDlgItemMessageW(hwnd, cmb1, CB_SETCURSEL, 0, 0);
        // �����ōČv�Z�����邩�H
        if (s_bAutoRetry) {
            ::CheckDlgButton(hwnd, chx1, BST_CHECKED);
        }
        // ���������H
        // �������鐔��ݒ肷��B
        ::SetDlgItemInt(hwnd, edt1, s_nNumberToGenerate, FALSE);
        // �t�@�C���h���b�v��L���ɂ���B
        ::DragAcceptFiles(hwnd, TRUE);
        SendDlgItemMessageW(hwnd, scr1, UDM_SETRANGE, 0, MAKELPARAM(100, 2));
        return TRUE;

    case WM_DROPFILES:
        // �h���b�v���ꂽ�t�@�C���̃p�X�����擾����B
        hDrop = reinterpret_cast<HDROP>(wParam);
        ::DragQueryFileW(hDrop, 0, szFile, ARRAYSIZE(szFile));
        ::DragFinish(hDrop);

        // �V���[�g�J�b�g�������ꍇ�́A�^�[�Q�b�g�̃p�X���擾����B
        if (::lstrcmpiW(PathFindExtensionW(szFile), s_szShellLinkDotExt) == 0) {
            if (!XgGetPathOfShortcutW(szFile, szTarget)) {
                ::MessageBeep(0xFFFFFFFF);
                break;
            }
            StringCbCopy(szFile, sizeof(szFile), szTarget);
        }

        // �t�@�C���̑������m�F����B
        attrs = ::GetFileAttributesW(szFile);
        if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
            // �f�B���N�g���[�������B
            // �������ڂ����łɂ���΁A�폜����B
            i = static_cast<int>(::SendDlgItemMessageW(
                hwnd, cmb1, CB_FINDSTRINGEXACT, 0,
                reinterpret_cast<LPARAM>(szFile)));
            if (i != CB_ERR) {
                ::SendDlgItemMessageW(hwnd, cmb1, CB_DELETESTRING, i, 0);
            }
            // �R���{�{�b�N�X�̍ŏ��ɑ}������B
            item.mask = CBEIF_TEXT;
            item.iItem = 0;
            item.pszText = szFile;
            item.cchTextMax = -1;
            ::SendDlgItemMessageW(hwnd, cmb1, CBEM_INSERTITEMW, 0,
                                reinterpret_cast<LPARAM>(&item));
            // �R���{�{�b�N�X�̍ŏ��̍��ڂ�I������B
            ::SendDlgItemMessageW(hwnd, cmb1, CB_SETCURSEL, 0, 0);
        } else {
            // �f�B���N�g���[�ł͂Ȃ������B
            ::MessageBeep(0xFFFFFFFF);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            // �ۑ���̃p�X�����擾����B
            ::GetDlgItemTextW(hwnd, cmb1, szFile, ARRAYSIZE(szFile));
            attrs = ::GetFileAttributesW(szFile);
            if (attrs == 0xFFFFFFFF || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                // �p�X���Ȃ���΍쐬����B
                if (!XgMakePathW(szFile)) {
                    // �쐬�Ɏ��s�B
                    ::SendDlgItemMessageW(hwnd, cmb1, CB_SETEDITSEL, 0, -1);
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_STORAGEINVALID), nullptr, MB_ICONERROR);
                    ::SetFocus(::GetDlgItem(hwnd, cmb1));
                    return 0;
                }
            }
            // �ۑ�����Z�b�g����B
            strDir = szFile;
            xg_str_trim(strDir);
            {
                auto end = s_dirs_save_to.end();
                for (auto it = s_dirs_save_to.begin(); it != end; it++) {
                    if (_wcsicmp((*it).data(), strDir.data()) == 0) {
                        s_dirs_save_to.erase(it);
                        break;
                    }
                }
                s_dirs_save_to.emplace_front(strDir);
            }
            // �����ōČv�Z�����邩�H
            s_bAutoRetry = (::IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED);
            // ���̐����擾����B
            {
                BOOL bTranslated;
                s_nNumberToGenerate = ::GetDlgItemInt(hwnd, edt1, &bTranslated, FALSE);
                if (!bTranslated || s_nNumberToGenerate == 0) {
                    ::SendDlgItemMessageW(hwnd, edt1, EM_SETSEL, 0, -1);
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERPOSITIVE), nullptr, MB_ICONERROR);
                    ::SetFocus(::GetDlgItem(hwnd, edt1));
                    return 0;
                }
            }
            // JSON�`���ŕۑ����邩�H
            xg_bSaveAsJsonFile = true;
            // ����������B
            xg_bSolved = false;
            xg_bShowAnswer = false;
            xg_vTateInfo.clear();
            xg_vYokoInfo.clear();
            xg_vMarks.clear();
            xg_vMarkedCands.clear();
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDCANCEL);
            break;

        case psh1:
            // ���[�U�[�ɕۑ����₢���킹��B
            ZeroMemory(&bi, sizeof(bi));
            bi.hwndOwner = hwnd;
            bi.lpszTitle = XgLoadStringDx1(IDS_CROSSSTORAGE);
            bi.ulFlags = BIF_RETURNONLYFSDIRS;
            bi.lpfn = XgBrowseCallbackProc;
            ::GetDlgItemTextW(hwnd, cmb1, xg_szDir, ARRAYSIZE(xg_szDir));
            pidl = ::SHBrowseForFolderW(&bi);
            if (pidl) {
                // �R���{�{�b�N�X�Ƀp�X��ݒ肷��B
                ::SHGetPathFromIDListW(pidl, szFile);
                ::SetDlgItemTextW(hwnd, cmb1, szFile);
                ::CoTaskMemFree(pidl);
            }
            break;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

// �q���g�̓��e���������ŊJ���B
bool __fastcall XgOpenHintsByNotepad(HWND /*hwnd*/, bool bShowAnswer)
{
    WCHAR szPath[MAX_PATH];
    WCHAR szCmdLine[MAX_PATH * 2];
    std::wstring str;
    DWORD cb;
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    // �����Ȃ��Ƃ��́A�q���g�͂Ȃ��B
    if (!xg_bSolved) {
        ::MessageBeep(0xFFFFFFFF);
        return false;
    }

    // �ꎞ�t�@�C�����쐬����B
    ::GetTempPathW(MAX_PATH, szPath);
    StringCbCat(szPath, sizeof(szPath), XgLoadStringDx1(IDS_HINTSTXT));
    HANDLE hFile = ::CreateFileW(szPath, GENERIC_WRITE, FILE_SHARE_READ,
        nullptr, CREATE_ALWAYS, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    // BOM�ƃq���g�̕�������t�@�C���ɏ������ށB
    str = reinterpret_cast<LPCWSTR>("\xFF\xFE\x00");
    str += xg_pszNewLine;
    xg_solution.GetHintsStr(xg_strHints, 2, true);
    str += xg_strHints;
    cb = static_cast<DWORD>(str.size() * sizeof(WCHAR));
    if (::WriteFile(hFile, str.data(), cb, &cb, nullptr)) {
        // �t�@�C�������B
        ::CloseHandle(hFile);

        // �������Ńt�@�C�����J���B
        StringCbPrintf(szCmdLine, sizeof(szCmdLine),
                       XgLoadStringDx1(IDS_NOTEPAD), szPath);
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOWNORMAL;
        if (::CreateProcessW(nullptr, szCmdLine, nullptr, nullptr, FALSE,
                             0, nullptr, nullptr, &si, &pi))
        {
            // ���������҂���ԂɂȂ�܂ő҂B
            if (::WaitForInputIdle(pi.hProcess, 5 * 1000) != WAIT_TIMEOUT) {
                // 0.2�b�҂B
                ::Sleep(200);
                // �t�@�C�����폜����B
                ::DeleteFileW(szPath);
            }
            // �n���h�������B
            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);
            return true;
        }
    }
    // �t�@�C�������B
    ::CloseHandle(hFile);
    return false;
}

// �q���g�̓��e���q���g�E�B���h�E�ŊJ���B
bool __fastcall XgOpenHintsByWindow(HWND hwnd)
{
    // �����q���g�E�B���h�E�����݂���Δj������B
    if (xg_hHintsWnd) {
        HWND hwnd = xg_hHintsWnd;
        xg_hHintsWnd = NULL;
        ::DestroyWindow(hwnd);
    }

    // �q���g�E�B���h�E���쐬����B
    if (XgCreateHintsWnd(xg_hMainWnd)) {
        ::ShowWindow(xg_hHintsWnd, SW_SHOWNOACTIVATE);
        ::SetForegroundWindow(xg_hMainWnd);
        return true;
    }
    return false;
}

// �q���g���X�V����B
void __fastcall XgUpdateHints(HWND hwnd)
{
    xg_vecTateHints.clear();
    xg_vecYokoHints.clear();
    xg_strHints.clear();
    if (xg_bSolved) {
        xg_solution.GetHintsStr(xg_strHints, 2, true);
        if (!XgParseHintsStr(hwnd, xg_strHints)) {
            xg_strHints.clear();
        }
    }
}

// �q���g��\������B
void __fastcall XgShowHints(HWND hwnd)
{
    #if 1
        XgOpenHintsByWindow(hwnd);
    #else
        XgOpenHintsByNotepad(hwnd, xg_bShowAnswer);
    #endif
}

// �X���b�h�����B
void __fastcall XgCloseThreads(void)
{
    for (DWORD i = 0; i < xg_dwThreadCount; i++) {
        ::CloseHandle(xg_ahThreads[i]);
        xg_ahThreads[i] = nullptr;
    }
}

// �X���b�h��҂B
inline void __fastcall XgWaitForThreads(void)
{
    ::WaitForMultipleObjects(xg_dwThreadCount, xg_ahThreads.data(), true, 1000);
}

// �X���b�h���I���������H
bool __fastcall XgIsAnyThreadTerminated(void)
{
    DWORD dwExitCode;
    for (DWORD i = 0; i < xg_dwThreadCount; i++) {
        ::GetExitCodeThread(xg_ahThreads[i], &dwExitCode);
        if (dwExitCode != STILL_ACTIVE)
            return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////

// �v���O���X�o�[�̍X�V�p�x�B
#define xg_dwTimerInterval 500

// �Čv�Z�܂ł̎��Ԃ��T�Z����B
inline DWORD XgGetRetryInterval(void)
{
    return 8 * (xg_nRows + xg_nCols) * (xg_nRows + xg_nCols) + 1000;
}

// �L�����Z���_�C�A���O�B
extern "C" INT_PTR CALLBACK
XgCancelSolveDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        #ifdef NO_RANDOM
        {
            extern int xg_random_seed;
            xg_random_seed = 100;
        }
        #endif
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �Ď��s�J�E���g���N���A����B
        s_nRetryCount = 0;
        // �v���O���X�o�[�͈̔͂��Z�b�g����B
        ::SendDlgItemMessageW(hwnd, ctl1, PBM_SETRANGE, 0,
                            MAKELPARAM(0, xg_nRows * xg_nCols));
        ::SendDlgItemMessageW(hwnd, ctl2, PBM_SETRANGE, 0,
                            MAKELPARAM(0, xg_nRows * xg_nCols));
        // �v�Z���Ԃ����߂邽�߂ɁA�J�n���Ԃ��擾����B
        s_dwTick0 = s_dwTick1 = ::GetTickCount();
        // �Čv�Z�܂ł̎��Ԃ��T�Z����B
        s_dwWait = XgGetRetryInterval();
        // �������߂�̂��J�n�B
        XgStartSolve_AddBlack();
        // �^�C�}�[���Z�b�g����B
        ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
        // �t�H�[�J�X���Z�b�g����B
        ::SetFocus(::GetDlgItem(hwnd, psh1));
        // �����������̌���\������B
        if (s_nNumberGenerated > 0) {
            WCHAR sz[MAX_PATH];
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_PROBLEMSMAKING),
                           s_nNumberGenerated);
            ::SetDlgItemTextW(hwnd, stc2, sz);
        }
        return false;

    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case psh1:
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �L�����Z�����ăX���b�h��҂B
            xg_bCancelled = true;
            XgWaitForThreads();
            // �X���b�h�����B
            XgCloseThreads();
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �������߂悤�Ƃ�����̌㏈���B
            XgEndSolve();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDCANCEL);
            break;

        case psh2:
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �Čv�Z���Ȃ����B
            xg_bRetrying = true;
            XgWaitForThreads();
            XgCloseThreads();
            ::InterlockedIncrement(&s_nRetryCount);
            s_dwTick1 = ::GetTickCount();
            XgStartSolve_AddBlack();
            // �^�C�}�[���Z�b�g����B
            ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
            break;
        }
        break;

    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE) {
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �L�����Z�����ăX���b�h��҂B
            xg_bCancelled = true;
            XgWaitForThreads();
            // �X���b�h�����B
            XgCloseThreads();
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �������߂悤�Ƃ�����̌㏈���B
            XgEndSolve();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDCANCEL);
        }
        break;

    case WM_TIMER:
        // �v���O���X�o�[���X�V����B
        //for (DWORD i = 0; i < xg_dwThreadCount; i++)
        for (DWORD i = 0; i < 2; i++) {
            ::SendDlgItemMessageW(hwnd, ctl1 + i, PBM_SETPOS,
                static_cast<WPARAM>(xg_aThreadInfo[i].m_count), 0);
        }

        {
            // �o�ߎ��Ԃ�\������B
            WCHAR sz[MAX_PATH];
            DWORD dwTick = ::GetTickCount();
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_NOWSOLVING),
                    (dwTick - s_dwTick0) / 1000,
                    (dwTick - s_dwTick0) / 100 % 10, s_nRetryCount);
            ::SetDlgItemTextW(hwnd, stc1, sz);
        }

        // �I�������X���b�h�����邩�H
        if (XgIsAnyThreadTerminated()) {
            // �X���b�h���I�������B�^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �������߂悤�Ƃ�����̌㏈���B
            XgEndSolve();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDOK);
            // �X���b�h�����B
            XgCloseThreads();
        } else {
            // �Čv�Z���K�v���H
            if (s_bAutoRetry && ::GetTickCount() - s_dwTick1 > s_dwWait) {
                // �^�C�}�[����������B
                ::KillTimer(hwnd, 999);
                // �Čv�Z���Ȃ����B
                xg_bRetrying = true;
                XgWaitForThreads();
                XgCloseThreads();
                ::InterlockedIncrement(&s_nRetryCount);
                s_dwTick1 = ::GetTickCount();
                XgStartSolve_AddBlack();
                // �^�C�}�[���Z�b�g����B
                ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
            }
        }
        break;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////

// �L�����Z���_�C�A���O�i���}�X�ǉ��Ȃ��j�B
extern "C" INT_PTR CALLBACK
XgCancelSolveDlgProcNoAddBlack(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        #ifdef NO_RANDOM
        {
            extern int xg_random_seed;
            xg_random_seed = 100;
        }
        #endif
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // ����������B
        s_nRetryCount = 0;
        // �v���O���X�o�[�͈̔͂��Z�b�g����B
        ::SendDlgItemMessageW(hwnd, ctl1, PBM_SETRANGE, 0,
                            MAKELPARAM(0, xg_nRows * xg_nCols));
        ::SendDlgItemMessageW(hwnd, ctl2, PBM_SETRANGE, 0,
                            MAKELPARAM(0, xg_nRows * xg_nCols));
        // �v�Z���Ԃ����߂邽�߂ɁA�J�n���Ԃ��擾����B
        s_dwTick0 = s_dwTick1 = ::GetTickCount();
        // �Čv�Z�܂ł̎��Ԃ��T�Z����B
        s_dwWait = XgGetRetryInterval();
        // �������߂�̂��J�n�B
        XgStartSolve_NoAddBlack();
        // �^�C�}�[���Z�b�g����B
        ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
        // �t�H�[�J�X���Z�b�g����B
        ::SetFocus(::GetDlgItem(hwnd, psh1));
        // �����������̌���\������B
        if (s_nNumberGenerated > 0) {
            WCHAR sz[MAX_PATH];
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_PROBLEMSMAKING),
                           s_nNumberGenerated);
            ::SetDlgItemTextW(hwnd, stc2, sz);
        }
        return false;

    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case psh1:
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �L�����Z�����ăX���b�h��҂B
            xg_bCancelled = true;
            XgWaitForThreads();
            // �X���b�h�����B
            XgCloseThreads();
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �������߂悤�Ƃ�����̌㏈���B
            XgEndSolve();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDCANCEL);
            break;

        case psh2:
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �Čv�Z���Ȃ����B
            xg_bRetrying = true;
            XgWaitForThreads();
            XgCloseThreads();
            ::InterlockedIncrement(&s_nRetryCount);
            s_dwTick1 = ::GetTickCount();
            XgStartSolve_NoAddBlack();
            // �^�C�}�[���Z�b�g����B
            ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
            break;
        }
        break;

    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE) {
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �L�����Z�����ăX���b�h��҂B
            xg_bCancelled = true;
            XgWaitForThreads();
            // �X���b�h�����B
            XgCloseThreads();
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �������߂悤�Ƃ�����̌㏈���B
            XgEndSolve();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDCANCEL);
        }
        break;

    case WM_TIMER:
        // �v���O���X�o�[���X�V����B
        for (DWORD i = 0; i < xg_dwThreadCount; i++) {
            ::SendDlgItemMessageW(hwnd, ctl1 + i, PBM_SETPOS,
                static_cast<WPARAM>(xg_aThreadInfo[i].m_count), 0);
        }
        // �o�ߎ��Ԃ�\������B
        {
            WCHAR sz[MAX_PATH];
            DWORD dwTick = ::GetTickCount();
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_NOWSOLVING),
                    (dwTick - s_dwTick0) / 1000,
                    (dwTick - s_dwTick0) / 100 % 10, s_nRetryCount);
            ::SetDlgItemTextW(hwnd, stc1, sz);
        }
        // ��ȏ�̃X���b�h���I���������H
        if (XgIsAnyThreadTerminated()) {
            // �X���b�h���I�������B�^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �������߂悤�Ƃ�����̌㏈���B
            XgEndSolve();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDOK);
            // �X���b�h�����B
            XgCloseThreads();
        } else {
            // �Čv�Z���K�v���H
            if (s_bAutoRetry && ::GetTickCount() - s_dwTick1 > s_dwWait) {
                // �^�C�}�[����������B
                ::KillTimer(hwnd, 999);
                // �Čv�Z���Ȃ����B
                xg_bRetrying = true;
                XgWaitForThreads();
                XgCloseThreads();
                ::InterlockedIncrement(&s_nRetryCount);
                s_dwTick1 = ::GetTickCount();
                // �X�}�[�g�����Ȃ�A���}�X�𐶐�����B
                XgStartSolve_NoAddBlack();
                // �^�C�}�[���Z�b�g����B
                ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
            }
        }
        break;
    }
    return false;
}

// �L�����Z���_�C�A���O�i�X�}�[�g�����j�B
extern "C" INT_PTR CALLBACK
XgCancelSolveDlgProcSmart(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        #ifdef NO_RANDOM
        {
            extern int xg_random_seed;
            xg_random_seed = 100;
        }
        #endif
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // ����������B
        s_nRetryCount = 0;
        // �v���O���X�o�[�͈̔͂��Z�b�g����B
        ::SendDlgItemMessageW(hwnd, ctl1, PBM_SETRANGE, 0,
                            MAKELPARAM(0, xg_nRows * xg_nCols));
        ::SendDlgItemMessageW(hwnd, ctl2, PBM_SETRANGE, 0,
                            MAKELPARAM(0, xg_nRows * xg_nCols));
        // �v�Z���Ԃ����߂邽�߂ɁA�J�n���Ԃ��擾����B
        s_dwTick0 = s_dwTick1 = ::GetTickCount();
        // �Čv�Z�܂ł̎��Ԃ��T�Z����B
        s_dwWait = XgGetRetryInterval();
        // �������߂�̂��J�n�B
        XgStartSolve_Smart();
        // �^�C�}�[���Z�b�g����B
        ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
        // �t�H�[�J�X���Z�b�g����B
        ::SetFocus(::GetDlgItem(hwnd, psh1));
        // �����������̌���\������B
        if (s_nNumberGenerated > 0) {
            WCHAR sz[MAX_PATH];
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_PROBLEMSMAKING),
                           s_nNumberGenerated);
            ::SetDlgItemTextW(hwnd, stc2, sz);
        }
        return false;

    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case psh1:
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �L�����Z�����ăX���b�h��҂B
            xg_bCancelled = true;
            XgWaitForThreads();
            // �X���b�h�����B
            XgCloseThreads();
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �������߂悤�Ƃ�����̌㏈���B
            XgEndSolve();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDCANCEL);
            break;

        case psh2:
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �Čv�Z���Ȃ����B
            xg_bRetrying = true;
            XgWaitForThreads();
            XgCloseThreads();
            ::InterlockedIncrement(&s_nRetryCount);
            s_dwTick1 = ::GetTickCount();
            XgStartSolve_Smart();
            // �^�C�}�[���Z�b�g����B
            ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
            break;
        }
        break;

    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE) {
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �L�����Z�����ăX���b�h��҂B
            xg_bCancelled = true;
            XgWaitForThreads();
            // �X���b�h�����B
            XgCloseThreads();
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �������߂悤�Ƃ�����̌㏈���B
            XgEndSolve();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDCANCEL);
        }
        break;

    case WM_TIMER:
        // �v���O���X�o�[���X�V����B
        for (DWORD i = 0; i < xg_dwThreadCount; i++) {
            ::SendDlgItemMessageW(hwnd, ctl1 + i, PBM_SETPOS,
                static_cast<WPARAM>(xg_aThreadInfo[i].m_count), 0);
        }
        // �o�ߎ��Ԃ�\������B
        {
            WCHAR sz[MAX_PATH];
            DWORD dwTick = ::GetTickCount();
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_NOWSOLVING),
                    (dwTick - s_dwTick0) / 1000,
                    (dwTick - s_dwTick0) / 100 % 10, s_nRetryCount);
            ::SetDlgItemTextW(hwnd, stc1, sz);
        }
        // ��ȏ�̃X���b�h���I���������H
        if (XgIsAnyThreadTerminated()) {
            // �X���b�h���I�������B�^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �������߂悤�Ƃ�����̌㏈���B
            XgEndSolve();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDOK);
            // �X���b�h�����B
            XgCloseThreads();
        } else {
            // �Čv�Z���K�v���H
            if (s_bAutoRetry && ::GetTickCount() - s_dwTick1 > s_dwWait) {
                // �^�C�}�[����������B
                ::KillTimer(hwnd, 999);
                // �Čv�Z���Ȃ����B
                xg_bRetrying = true;
                XgWaitForThreads();
                XgCloseThreads();
                ::InterlockedIncrement(&s_nRetryCount);
                s_dwTick1 = ::GetTickCount();
                XgStartSolve_Smart();
                // �^�C�}�[���Z�b�g����B
                ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
            }
        }
        break;
    }
    return false;
}

// �L�����Z���_�C�A���O�B
extern "C" INT_PTR CALLBACK
XgCancelGenBlacksDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �������߂�̂��J�n�B
        XgStartGenerateBlacks(!!lParam);
        // �t�H�[�J�X���Z�b�g����B
        ::SetFocus(::GetDlgItem(hwnd, psh1));
        // �J�n���ԁB
        s_dwTick0 = ::GetTickCount();
        s_nRetryCount = 0;
        // ���������p�^�[���̌���\������B
        if (s_nNumberGenerated > 0) {
            WCHAR sz[MAX_PATH];
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_PATMAKING), s_nNumberGenerated);
            ::SetDlgItemTextW(hwnd, stc2, sz);
        }
        // �^�C�}�[���Z�b�g����B
        ::SetTimer(hwnd, 999, xg_dwTimerInterval, nullptr);
        return false;

    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case psh1:
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �L�����Z�����ăX���b�h��҂B
            xg_bCancelled = true;
            XgWaitForThreads();
            // �X���b�h�����B
            XgCloseThreads();
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDCANCEL);
            break;
        }
        break;

    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE) {
            // �^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �L�����Z�����ăX���b�h��҂B
            xg_bCancelled = true;
            XgWaitForThreads();
            // �X���b�h�����B
            XgCloseThreads();
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDCANCEL);
        }
        break;

    case WM_TIMER:
        {
            // �o�ߎ��Ԃ�\������B
            WCHAR sz[MAX_PATH];
            DWORD dwTick = ::GetTickCount();
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_CALCULATING),
                    (dwTick - s_dwTick0) / 1000,
                    (dwTick - s_dwTick0) / 100 % 10);
            ::SetDlgItemTextW(hwnd, stc1, sz);
        }

        // �I�������X���b�h�����邩�H
        if (XgIsAnyThreadTerminated()) {
            // �X���b�h���I�������B�^�C�}�[����������B
            ::KillTimer(hwnd, 999);
            // �v�Z���Ԃ����߂邽�߂ɁA�I�����Ԃ��擾����B
            s_dwTick2 = ::GetTickCount();
            // �_�C�A���O���I������B
            ::EndDialog(hwnd, IDOK);
            // �X���b�h�����B
            XgCloseThreads();
        }
        break;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// ����B

// �������B
void __fastcall XgPrintIt(HDC hdc, PRINTDLGW* ppd, bool bPrintAnswer)
{
    LOGFONTW lf;
    HFONT hFont;
    HGDIOBJ hFontOld;
    RECT rc;

    const int nTate = ::GetDeviceCaps(hdc, VERTSIZE);
    const int nYoko = ::GetDeviceCaps(hdc, HORZSIZE);
    if (nTate < nYoko) {
        // ����p���������B���b�Z�[�W��\�����ďI���B
        ::XgCenterMessageBoxW(xg_hMainWnd, XgLoadStringDx1(IDS_SETPORTRAIT), nullptr,
                            MB_ICONERROR);
        ::DeleteDC(hdc);
        ::GlobalFree(ppd->hDevMode);
        ::GlobalFree(ppd->hDevNames);
        return;
    }

    DOCINFOW di;
    ZeroMemory(&di, sizeof(di));
    di.cbSize = sizeof(di);
    di.lpszDocName = XgLoadStringDx1(IDS_CROSSWORD);

    // �w�肳�ꂽ�������������B
    for (int i = 0; i < ppd->nCopies; i++) {
        // �������J�n����B
        if (::StartDocW(hdc, &di) <= 0)
            continue;

        size_t ich, ichOld;
        std::wstring str, strMarkWord, strHints;
        int yLast = 0;

        // �_���s�N�Z���̃A�X�y�N�g����擾����B
        const int nLogPixelX = ::GetDeviceCaps(hdc, LOGPIXELSX);
        const int nLogPixelY = ::GetDeviceCaps(hdc, LOGPIXELSY);

        // �p���̃s�N�Z���T�C�Y���擾����B
        int cxPaper = ::GetDeviceCaps(hdc, HORZRES);
        int cyPaper = ::GetDeviceCaps(hdc, VERTRES);

        // �y�[�W�J�n�B
        if (::StartPage(hdc) > 0) {
            // ��d�}�X�P���`�悷��B
            if (bPrintAnswer && XgGetMarkWord(&xg_solution, strMarkWord)) {
                // ����̃t�H���g�����擾����B
                hFont = reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
                ::GetObjectW(hFont, sizeof(LOGFONTW), &lf);

                // �t�H���g�����擾����B
                StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), XgLoadStringDx1(IDS_MONOFONT));
                if (xg_szCellFont[0])
                    StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szCellFont);

                // �t�H���g����ݒ肷��B
                lf.lfHeight = cyPaper / 2 / 15;
                lf.lfWidth = 0;
                lf.lfWeight = FW_NORMAL;
                lf.lfQuality = ANTIALIASED_QUALITY;

                // ��d�}�X�P���`�悷��B
                hFont = ::CreateFontIndirectW(&lf);
                hFontOld = ::SelectObject(hdc, hFont);
                ::SetRect(&rc, cxPaper / 8, cyPaper / 16,
                    cxPaper * 7 / 8, cyPaper / 8);
                str = XgLoadStringDx1(IDS_ANSWER);
                str += strMarkWord;
                ::DrawTextW(hdc, str.data(), static_cast<int>(str.size()), &rc,
                    DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX);
                ::SelectObject(hdc, hFontOld);
                ::DeleteFont(hFont);
            }

            // EMF�I�u�W�F�N�g���쐬����B
            HDC hdcEMF = ::CreateEnhMetaFileW(hdc, nullptr, nullptr, XgLoadStringDx1(IDS_APPNAME));
            if (hdcEMF != nullptr) {
                // EMF�I�u�W�F�N�g�ɃN���X���[�h��`�悷��B
                SIZE siz;
                XgGetXWordExtent(&siz);
                if (bPrintAnswer)
                    XgDrawXWord(xg_solution, hdcEMF, &siz, false);
                else
                    XgDrawXWord(xg_xword, hdcEMF, &siz, false);

                // EMF�I�u�W�F�N�g�����B
                HENHMETAFILE hEMF = ::CloseEnhMetaFile(hdcEMF);
                if (hEMF != nullptr) {
                    // EMF��`�悷��̈���v�Z����B
                    int x, y, cx, cy;
                    if (xg_nCols >= 10) {
                        x = cxPaper / 6;
                        y = cyPaper / 8;
                        cx = cxPaper * 2 / 3;
                        cy = cxPaper * nLogPixelY * siz.cy / nLogPixelX / siz.cx * 2 / 3;
                    } else {
                        x = cxPaper / 4;
                        y = cyPaper / 8;
                        cx = cxPaper / 2;
                        cy = cxPaper * nLogPixelY * siz.cy / nLogPixelX / siz.cx / 2;
                    }
                    ::SetRect(&rc, x, y, x + cx, y + cy);

                    // EMF��`�悷��B
                    ::PlayEnhMetaFile(hdc, hEMF, &rc);
                    ::DeleteEnhMetaFile(hEMF);
                    yLast = y + cy;
                }
            }

            // �q���g���擾����B
            if (xg_bSolved) {
                xg_solution.GetHintsStr(xg_strHints, 2, bPrintAnswer);
                strHints = xg_strHints;
            } else {
                strHints.clear();
            }

            // �����F���Z�b�g����B
            ::SetTextColor(hdc, RGB(0, 0, 0));

            // ����̃t�H���g�����擾����B
            hFont = reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
            ::GetObjectW(hFont, sizeof(LOGFONTW), &lf);

            // �t�H���g�����擾����B
            StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), XgLoadStringDx1(IDS_MONOFONT));
            if (xg_szCellFont[0])
                StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szCellFont);

            // �t�H���g����ݒ肷��B
            lf.lfHeight = cyPaper / 2 / 45;
            lf.lfWidth = 0;
            lf.lfWeight = FW_NORMAL;
            lf.lfQuality = ANTIALIASED_QUALITY;
            hFont = ::CreateFontIndirectW(&lf);
            hFontOld = ::SelectObject(hdc, hFont);

            // �q���g����s���`�悷��B
            ::SetRect(&rc, cxPaper / 8, cyPaper / 2,
                cxPaper * 7 / 8, cyPaper * 8 / 9);
            for (ichOld = ich = 0; rc.bottom <= cyPaper * 8 / 9; ichOld = ich + 1) {
                ich = strHints.find(L"\n", ichOld);
                if (ich == std::wstring::npos)
                    break;
                str = strHints.substr(0, ich);  // ��s���o���B
                ::SetRect(&rc, cxPaper / 8, yLast,
                    cxPaper * 7 / 8, cyPaper * 8 / 9);
                ::DrawTextW(hdc, str.data(), static_cast<int>(str.size()), &rc,
                    DT_LEFT | DT_TOP | DT_CALCRECT | DT_NOPREFIX | DT_WORDBREAK);
            }
            // �Ō�̍s��`�悷��B
            ::SetRect(&rc, cxPaper / 8, yLast,
                cxPaper * 7 / 8, cyPaper * 8 / 9);
            if (ich == std::wstring::npos) {
                str = strHints;
                ::DrawTextW(hdc, str.data(), static_cast<int>(str.size()), &rc,
                    DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);
                strHints.clear();
            } else {
                str = strHints.substr(0, ichOld);
                ::DrawTextW(hdc, str.data(), static_cast<int>(str.size()), &rc,
                    DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);
                strHints = strHints.substr(ichOld);
            }

            // �����̃t�H���g�̑I�����������č폜����B
            ::SelectObject(hdc, hFontOld);
            ::DeleteFont(hFont);

            // �y�[�W�I���B
            ::EndPage(hdc);
        }

        // �q���g�̎c���`�悷��B
        if (!strHints.empty()) {
            // �y�[�W�J�n�B
            if (::StartPage(hdc) > 0) {
                // �����̃t�H���g���쐬����B
                hFont = reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
                ::GetObjectW(hFont, sizeof(LOGFONTW), &lf);
                StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), XgLoadStringDx1(IDS_MONOFONT));
                lf.lfHeight = cyPaper / 2 / 45;
                lf.lfWidth = 0;
                lf.lfWeight = FW_NORMAL;
                lf.lfQuality = ANTIALIASED_QUALITY;

                // �q���g�̎c���`�悷��B
                hFont = ::CreateFontIndirectW(&lf);
                hFontOld = ::SelectObject(hdc, hFont);
                str = strHints;
                ::SetRect(&rc, cxPaper / 8, cyPaper / 9,
                    cxPaper * 7 / 8, cyPaper * 8 / 9);
                ::DrawTextW(hdc, str.data(), static_cast<int>(str.size()), &rc,
                    DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);
                ::SelectObject(hdc, hFontOld);
                ::DeleteFont(hFont);

                // �y�[�W�I���B
                ::EndPage(hdc);
            }
        }
        // �����I���B
        ::EndDoc(hdc);
    }

    ::DeleteDC(hdc);
    ::GlobalFree(ppd->hDevMode);
    ::GlobalFree(ppd->hDevNames);
}

// ���݂̂��������B
void __fastcall XgPrintProblem(void)
{
    PRINTDLGW pd;

    ZeroMemory(&pd, sizeof(pd));
    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = xg_hMainWnd;
    pd.Flags = PD_ALLPAGES | PD_HIDEPRINTTOFILE |
        PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC;
    pd.nCopies = 1;
    if (::PrintDlgW(&pd)) {
        XgPrintIt(pd.hDC, &pd, false);
    }
}

// ���Ɖ𓚂��������B
void __fastcall XgPrintAnswer(void)
{
    PRINTDLGW pd;

    ZeroMemory(&pd, sizeof(pd));
    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = xg_hMainWnd;
    pd.Flags = PD_ALLPAGES | PD_HIDEPRINTTOFILE |
        PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC;
    pd.nCopies = 1;
    if (::PrintDlgW(&pd)) {
        XgPrintIt(pd.hDC, &pd, true);
    }
}

//////////////////////////////////////////////////////////////////////////////

// �o�[�W��������\������B
void __fastcall XgOnAbout(HWND hwnd)
{
    MSGBOXPARAMS params;
    memset(&params, 0, sizeof(params));
    params.cbSize = sizeof(params);
    params.hwndOwner = hwnd;
    params.hInstance = xg_hInstance;
    params.lpszText = XgLoadStringDx1(IDS_VERSION);
    params.lpszCaption = XgLoadStringDx2(IDS_APPNAME);
    params.dwStyle = MB_USERICON;
    params.lpszIcon = MAKEINTRESOURCE(1);
    XgCenterMessageBoxIndirectW(&params);
}

// �V�K�쐬�_�C�A���O�B
bool __fastcall XgOnNew(HWND hwnd)
{
    INT nID;
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    nID = INT(::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_NEW), hwnd, XgNewDlgProc));
    ::EnableWindow(xg_hwndInputPalette, TRUE);
    if (nID != IDOK)
        return false;

    return true;
}

// ����̏ꏊ�Ƀt�@�C����ۑ�����B
bool __fastcall XgDoSaveToLocation(HWND hwnd)
{
    WCHAR szPath[MAX_PATH], szDir[MAX_PATH];
    WCHAR szName[32];

    // �p�X�𐶐�����B
    StringCbCopy(szDir, sizeof(szDir), s_dirs_save_to[0].data());
    PathAddBackslashW(szDir);

    // �t�@�C�����𐶐�����B
    UINT u;
    for (u = 1; u <= 0xFFFF; u++) {
        StringCbPrintf(szName, sizeof(szName), L"%dx%d-%04u.xwj",
                  xg_nRows, xg_nCols, u);
        StringCbCopy(szPath, sizeof(szPath), szDir);
        StringCbCat(szPath, sizeof(szPath), szName);
        if (::GetFileAttributesW(szPath) == 0xFFFFFFFF)
            break;
    }

    bool bOK = false;
    if (u != 0x10000) {
        // �t�@�C�������쐬�ł����B�r�����䂵�Ȃ���ۑ�����B
        ::EnterCriticalSection(&xg_cs);
        {
            // ������H
            if (xg_bSolved) {
                // �J�M�ԍ����X�V����B
                xg_solution.DoNumberingNoCheck();

                // �q���g���X�V����B
                XgUpdateHints(hwnd);
            }

            // �t�@�C���ɕۑ�����B
            bOK = XgDoSave(hwnd, szPath);
        }
        ::LeaveCriticalSection(&xg_cs);
    }

    // �f�B�X�N�e�ʂ��m�F����B
    ULARGE_INTEGER ull1, ull2, ull3;
    if (::GetDiskFreeSpaceExW(szPath, &ull1, &ull2, &ull3)) {
        if (ull1.QuadPart < 0x1000000)  // 1MB
        {
            s_bOutOfDiskSpace = true;
        }
    }

    return bOK;
}

// ���̍쐬�B
bool __fastcall XgOnGenerate(HWND hwnd, bool show_answer, bool multiple = false)
{
    INT nID;
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    if (multiple) {
        // [���̘A���쐬]�_�C�A���O�B
        nID = INT(::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_SEQCREATE), hwnd,
                               XgGenerateRepeatedlyDlgProc));
    } else {
        // [���̍쐬]�_�C�A���O�B
        nID = INT(::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CREATE), hwnd,
                               XgGenerateDlgProc));
    }
    ::EnableWindow(xg_hwndInputPalette, TRUE);
    if (nID != IDOK) {
        return false;
    }

    xg_bSolvingEmpty = true;
    xg_bNoAddBlack = false;
    s_nNumberGenerated = 0;
    s_bOutOfDiskSpace = false;
    xg_strHeader.clear();
    xg_strNotes.clear();
    xg_strFileName.clear();

    // ������ǂݍ��ށB
    XgLoadDictFile(xg_dict_name.c_str());
    XgSetInputModeFromDict(hwnd);

    // �L�����Z���_�C�A���O��\�����A���s���J�n����B
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    do {
        if (xg_imode == xg_im_KANJI) {
            // �����̏ꍇ�̓X�}�[�g�������g�p���Ȃ��B
            nID = ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CALCULATING), hwnd, XgCancelSolveDlgProc);
        } else if (xg_bSmartResolution && xg_nRows >= 7 && xg_nCols >= 7) {
            nID = ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CALCULATING), hwnd, XgCancelSolveDlgProcSmart);
        } else if (xg_nRules & RULE_POINTSYMMETRY) {
            nID = ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CALCULATING), hwnd, XgCancelSolveDlgProcSmart);
        } else {
            nID = ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CALCULATING), hwnd, XgCancelSolveDlgProc);
        }
        // ���������̂Ƃ���s_nNumberGenerated�𑝂₷�B
        if (nID == IDOK && xg_bSolved) {
            ++s_nNumberGenerated;
            if (multiple) {
                if (!XgDoSaveToLocation(hwnd)) {
                    s_bOutOfDiskSpace = true;
                    break;
                }
                // �������B
                xg_bSolved = false;
                xg_xword.ResetAndSetSize(xg_nRows, xg_nCols);
                xg_vTateInfo.clear();
                xg_vYokoInfo.clear();
                xg_vMarks.clear();
                xg_vMarkedCands.clear();
                xg_bSolvingEmpty = true;
                xg_strHeader.clear();
                xg_strNotes.clear();
                xg_strFileName.clear();
                // ������ǂݍ��ށB
                XgLoadDictFile(xg_dict_name.c_str());
                XgSetInputModeFromDict(hwnd);
            }
        }
    } while (nID == IDOK && multiple && s_nNumberGenerated < s_nNumberToGenerate);
    ::EnableWindow(xg_hwndInputPalette, TRUE);

    // ����������B
    if (multiple) {
        xg_xword.clear();
        xg_vMarkedCands.clear();
        xg_vMarks.clear();
        xg_vTateInfo.clear();
        xg_vYokoInfo.clear();
        xg_vecTateHints.clear();
        xg_vecYokoHints.clear();
        xg_bSolved = false;
        xg_bShowAnswer = false;
    } else {
        xg_bShowAnswer = show_answer;
        if (xg_bSmartResolution && xg_bCancelled) {
            xg_xword.clear();
        }
    }

    return true;
}

// ���}�X�p�^�[����A����������B
bool __fastcall XgOnGenerateBlacksRepeatedly(HWND hwnd)
{
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    INT nID = DialogBoxW(xg_hInstance, MAKEINTRESOURCEW(IDD_SEQPATGEN), hwnd,
                         XgGenerateBlacksRepeatedlyDlgProc);
    ::EnableWindow(xg_hwndInputPalette, TRUE);
    if (nID != IDOK) {
        return false;
    }

    // ����������B
    xg_xword.clear();
    xg_caret_pos.clear();
    xg_vMarks.clear();
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_vecTateHints.clear();
    xg_vecYokoHints.clear();
    xg_bSolved = false;
    xg_bHintsAdded = false;
    xg_bShowAnswer = false;
    xg_strHeader.clear();
    xg_strNotes.clear();
    xg_strFileName.clear();
    xg_bBlacksGenerated = false;
    s_nNumberGenerated = 0;

    // ���E�B���h�E��j������B
    XgDestroyCandsWnd();
    // �q���g�E�B���h�E��j������B
    XgDestroyHintsWnd();

    // �L�����Z���_�C�A���O��\�����A�������J�n����B
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    do
    {
        nID = ::DialogBoxParamW(xg_hInstance, MAKEINTRESOURCEW(IDD_CALCULATING),
                                hwnd, XgCancelGenBlacksDlgProc,
                                !!(xg_nRules & RULE_POINTSYMMETRY));
        // ���������̂Ƃ���s_nNumberGenerated�𑝂₷�B
        if (nID == IDOK && xg_bBlacksGenerated) {
            ++s_nNumberGenerated;
            if (!XgDoSaveToLocation(hwnd)) {
                s_bOutOfDiskSpace = true;
                break;
            }
            // �������B
            xg_bSolved = false;
            xg_xword.clear();
            xg_vTateInfo.clear();
            xg_vYokoInfo.clear();
            xg_vMarks.clear();
            xg_vMarkedCands.clear();
            xg_strHeader.clear();
            xg_strNotes.clear();
            xg_strFileName.clear();
            xg_bBlacksGenerated = false;
        }
    } while (nID == IDOK && s_nNumberGenerated < s_nNumberToGenerate);
    ::EnableWindow(xg_hwndInputPalette, TRUE);
    xg_caret_pos.clear();
    XgUpdateImage(hwnd, 0, 0);

    WCHAR sz[MAX_PATH];
    if (xg_bCancelled) {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_CANCELLED),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);
    } else {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_BLOCKSGENERATED),
            s_nNumberGenerated,
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);
    }

    // �ۑ���t�H���_���J���B
    if (s_nNumberGenerated && !s_dirs_save_to.empty()) {
        ::ShellExecuteW(hwnd, nullptr, s_dirs_save_to[0].data(),
                      nullptr, nullptr, SW_SHOWNORMAL);
    }
    return true;
}

// ���}�X�p�^�[���𐶐�����B
bool __fastcall XgOnGenerateBlacks(HWND hwnd, bool sym)
{
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    INT nID = DialogBoxW(xg_hInstance, MAKEINTRESOURCEW(IDD_PATGEN), hwnd,
                         XgGenerateBlacksDlgProc);
    ::EnableWindow(xg_hwndInputPalette, TRUE);
    if (nID != IDOK) {
        return false;
    }

    // ����������B
    xg_caret_pos.clear();
    xg_vMarks.clear();
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_vecTateHints.clear();
    xg_vecYokoHints.clear();
    xg_bSolved = false;
    xg_bHintsAdded = false;
    xg_bShowAnswer = false;
    xg_strHeader.clear();
    xg_strNotes.clear();
    xg_strFileName.clear();
    xg_bBlacksGenerated = false;
    s_nNumberGenerated = 0;

    // ���E�B���h�E��j������B
    XgDestroyCandsWnd();
    // �q���g�E�B���h�E��j������B
    XgDestroyHintsWnd();

    // �L�����Z���_�C�A���O��\�����A�������J�n����B
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    ::DialogBoxParamW(xg_hInstance, MAKEINTRESOURCEW(IDD_CALCULATING),
                      hwnd, XgCancelGenBlacksDlgProc,
                      !!(xg_nRules & RULE_POINTSYMMETRY));
    ::EnableWindow(xg_hwndInputPalette, TRUE);
    xg_caret_pos.clear();
    XgUpdateImage(hwnd, 0, 0);

    WCHAR sz[MAX_PATH];
    if (xg_bCancelled) {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_CANCELLED),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);
    } else {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_BLOCKSGENERATED), 1,
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);
    }
    return true;
}

// �������߂�B
bool __fastcall XgOnSolve_AddBlack(HWND hwnd)
{
    // ���łɉ�����Ă���ꍇ�́A���s�����ۂ���B
    if (xg_bSolved) {
        ::MessageBeep(0xFFFFFFFF);
        return false;
    }

    // ������ǂݍ��ށB
    XgLoadDictFile(xg_dict_name.c_str());
    XgSetInputModeFromDict(hwnd);

    // ���}�X���[���Ȃǂ��`�F�b�N����B
    xg_bNoAddBlack = false;
    if (!XgCheckCrossWord(hwnd)) {
        return false;
    }

    // ����������B
    xg_bSolvingEmpty = xg_xword.IsEmpty();
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_vMarks.clear();
    xg_vMarkedCands.clear();
    xg_strFileName.clear();
    // �����������Ɛ������鐔�B
    s_nNumberGenerated = 0;

    // ���E�B���h�E��j������B
    XgDestroyCandsWnd();
    // �q���g�E�B���h�E��j������B
    XgDestroyHintsWnd();

    // �L�����Z���_�C�A���O��\�����A���s���J�n����B
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CALCULATING), hwnd, XgCancelSolveDlgProc);
    ::EnableWindow(xg_hwndInputPalette, TRUE);

    WCHAR sz[MAX_PATH];
    if (xg_bCancelled) {
        // �L�����Z�����ꂽ�B
        // ���Ȃ��B�\�����X�V����B
        xg_bShowAnswer = false;
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);

        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_CANCELLED),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);
    } else if (xg_bSolved) {
        // ��}�X���Ȃ����H
        if (xg_xword.IsFulfilled()) {
            // ��}�X���Ȃ��B�N���A�B
            xg_xword.clear();
            // ���ɍ��킹�āA���ɍ��}�X��u���B
            for (int i = 0; i < xg_nRows; i++) {
                for (int j = 0; j < xg_nCols; j++) {
                    if (xg_solution.GetAt(i, j) == ZEN_BLACK)
                        xg_xword.SetAt(i, j, ZEN_BLACK);
                }
            }
        }

        // ������B�\�����X�V����B
        xg_bShowAnswer = true;
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);

        // �������b�Z�[�W��\������B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_SOLVED),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);

        // �q���g���X�V���ĊJ���B
        XgUpdateHints(hwnd);
        XgShowHints(hwnd);
    } else {
        // ���Ȃ��B�\�����X�V����B
        xg_bShowAnswer = false;
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);
        // ���s���b�Z�[�W��\������B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_CANTSOLVE),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        ::InvalidateRect(hwnd, nullptr, FALSE);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONERROR);
    }
    return true;
}

// �������߂�i���}�X�ǉ��Ȃ��j�B
bool __fastcall XgOnSolve_NoAddBlack(HWND hwnd, bool bShowAnswer = true)
{
    // ���łɉ�����Ă���ꍇ�́A���s�����ۂ���B
    if (xg_bSolved) {
        ::MessageBeep(0xFFFFFFFF);
        return false;
    }

    // ������ǂݍ��ށB
    XgLoadDictFile(xg_dict_name.c_str());
    XgSetInputModeFromDict(hwnd);

    // ���}�X���[���Ȃǂ��`�F�b�N����B
    xg_bNoAddBlack = true;
    if (!XgCheckCrossWord(hwnd)) {
        return false;
    }

    // ����������B
    xg_bSolvingEmpty = xg_xword.IsEmpty();
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_vMarks.clear();
    xg_vMarkedCands.clear();
    xg_strFileName.clear();
    // �����������Ɛ������鐔�B
    s_nNumberGenerated = 0;

    // ���E�B���h�E��j������B
    XgDestroyCandsWnd();
    // �q���g�E�B���h�E��j������B
    XgDestroyHintsWnd();
    // �L�����Z���_�C�A���O��\�����A���s���J�n����B
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CALCULATING), hwnd,
                 XgCancelSolveDlgProcNoAddBlack);
    ::EnableWindow(xg_hwndInputPalette, TRUE);

    WCHAR sz[MAX_PATH];
    if (xg_bCancelled) {
        // �L�����Z�����ꂽ�B
        // ���Ȃ��B�\�����X�V����B
        xg_bShowAnswer = false;
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);

        // �I�����b�Z�[�W��\������B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_CANCELLED),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);
    } else if (xg_bSolved) {
        // ��}�X���Ȃ����H
        if (xg_xword.IsFulfilled()) {
            // ��}�X���Ȃ��B�N���A�B
            xg_xword.clear();
            // ���ɍ��킹�āA���ɍ��}�X��u���B
            for (int i = 0; i < xg_nRows; i++) {
                for (int j = 0; j < xg_nCols; j++) {
                    if (xg_solution.GetAt(i, j) == ZEN_BLACK)
                        xg_xword.SetAt(i, j, ZEN_BLACK);
                }
            }
        }

        // ������B�\�����X�V����B
        xg_bShowAnswer = bShowAnswer;
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);

        // �������b�Z�[�W��\������B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_SOLVED),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);

        // �q���g���X�V���ĊJ���B
        XgUpdateHints(hwnd);
        XgShowHints(hwnd);
    } else {
        // ���Ȃ��B�\�����X�V����B
        xg_bShowAnswer = false;
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);
        // ���s���b�Z�[�W��\������B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_CANTSOLVE),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        ::InvalidateRect(hwnd, nullptr, FALSE);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONERROR);
    }

    return true;
}

// �A���ŉ������߂�B
bool __fastcall XgOnSolveRepeatedly(HWND hwnd)
{
    // ���łɉ�����Ă���ꍇ�́A���s�����ۂ���B
    if (xg_bSolved) {
        ::MessageBeep(0xFFFFFFFF);
        return false;
    }

    // ������ǂݍ��ށB
    XgLoadDictFile(xg_dict_name.c_str());
    XgSetInputModeFromDict(hwnd);

    // ���}�X���[���Ȃǂ��`�F�b�N����B
    xg_bNoAddBlack = false;
    if (!XgCheckCrossWord(hwnd)) {
        return false;
    }

    // ���s�O�̃}�X�̏�Ԃ�ۑ�����B
    XG_Board xword_save(xg_xword);

    // [���̘A���쐬]�_�C�A���O�B
    INT nID;
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    nID = INT(::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_SEQSOLVE), hwnd,
                           XgSolveRepeatedlyDlgProc));
    ::EnableWindow(xg_hwndInputPalette, TRUE);
    if (nID != IDOK) {
        // �C���[�W���X�V����B
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);
        return false;
    }

    // ����������B
    xg_strFileName.clear();
    xg_strHeader.clear();
    xg_strNotes.clear();
    s_nNumberGenerated = 0;
    s_bOutOfDiskSpace = false;

    xg_bSolvingEmpty = xg_xword.IsEmpty();
    xg_bNoAddBlack = false;
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_vecTateHints.clear();
    xg_vecYokoHints.clear();

    // �L�����Z���_�C�A���O��\�����A���s���J�n����B
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    do
    {
        nID = ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CALCULATING), hwnd,
                           XgCancelSolveDlgProc);
        // ���������̂Ƃ���s_nNumberGenerated�𑝂₷�B
        if (nID == IDOK && xg_bSolved) {
            ++s_nNumberGenerated;
            if (!XgDoSaveToLocation(hwnd)) {
                s_bOutOfDiskSpace = true;
                break;
            }
            // �������B
            xg_bSolved = false;
            xg_xword = xword_save;
            xg_vTateInfo.clear();
            xg_vYokoInfo.clear();
            xg_vMarks.clear();
            xg_vMarkedCands.clear();
            xg_strHeader.clear();
            xg_strNotes.clear();
            xg_strFileName.clear();
            // ������ǂݍ��ށB
            XgLoadDictFile(xg_dict_name.c_str());
            XgSetInputModeFromDict(hwnd);
        }
    } while (nID == IDOK && s_nNumberGenerated < s_nNumberToGenerate);
    ::EnableWindow(xg_hwndInputPalette, TRUE);

    // ����������B
    xg_xword = xword_save;
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_vecTateHints.clear();
    xg_vecYokoHints.clear();
    xg_strFileName.clear();

    // �C���[�W���X�V����B
    xg_bSolved = false;
    xg_bShowAnswer = false;
    xg_caret_pos.clear();
    XgMarkUpdate();
    XgUpdateImage(hwnd, 0, 0);

    // �I�����b�Z�[�W��\������B
    WCHAR sz[MAX_PATH];
    if (s_bOutOfDiskSpace) {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_OUTOFSTORAGE), s_nNumberGenerated,
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
    } else {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_PROBLEMSMADE), s_nNumberGenerated,
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
    }
    XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);

    // �ۑ���t�H���_���J���B
    if (s_nNumberGenerated && !s_dirs_save_to.empty()) {
        ::ShellExecuteW(hwnd, nullptr, s_dirs_save_to[0].data(),
                      nullptr, nullptr, SW_SHOWNORMAL);
    }

    return true;
}

// �A���ŉ������߂�i���}�X�ǉ��Ȃ��j�B
bool __fastcall XgOnSolveRepeatedlyNoAddBlack(HWND hwnd)
{
    // ���łɉ�����Ă���ꍇ�́A���s�����ۂ���B
    if (xg_bSolved) {
        ::MessageBeep(0xFFFFFFFF);
        return false;
    }

    // ������ǂݍ��ށB
    XgLoadDictFile(xg_dict_name.c_str());
    XgSetInputModeFromDict(hwnd);

    // ���}�X���[���Ȃǂ��`�F�b�N����B
    xg_bNoAddBlack = true;
    if (!XgCheckCrossWord(hwnd)) {
        return false;
    }

    // ���s�O�̃}�X�̏�Ԃ�ۑ�����B
    XG_Board xword_save(xg_xword);

    // [���̘A���쐬]�_�C�A���O�B
    INT nID;
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    nID = INT(::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_SEQSOLVE), hwnd,
                           XgSolveRepeatedlyDlgProc));
    ::EnableWindow(xg_hwndInputPalette, TRUE);
    if (nID != IDOK) {
        // �C���[�W���X�V����B
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);
        return false;
    }

    // ����������B
    xg_strFileName.clear();
    xg_strHeader.clear();
    xg_strNotes.clear();
    s_nNumberGenerated = 0;
    s_bOutOfDiskSpace = false;

    xg_bSolvingEmpty = xg_xword.IsEmpty();
    xg_bNoAddBlack = true;
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_vecTateHints.clear();
    xg_vecYokoHints.clear();

    // �L�����Z���_�C�A���O��\�����A���s���J�n����B
    ::EnableWindow(xg_hwndInputPalette, FALSE);
    do
    {
        nID = ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CALCULATING), hwnd,
                           XgCancelSolveDlgProcNoAddBlack);
        // ���������̂Ƃ���s_nNumberGenerated�𑝂₷�B
        if (nID == IDOK && xg_bSolved) {
            ++s_nNumberGenerated;
            if (!XgDoSaveToLocation(hwnd)) {
                s_bOutOfDiskSpace = true;
                break;
            }
            // �������B
            xg_bSolved = false;
            xg_xword = xword_save;
            xg_vTateInfo.clear();
            xg_vYokoInfo.clear();
            xg_vMarks.clear();
            xg_vMarkedCands.clear();
            xg_strHeader.clear();
            xg_strNotes.clear();
            xg_strFileName.clear();
            // ������ǂݍ��ށB
            XgLoadDictFile(xg_dict_name.c_str());
            XgSetInputModeFromDict(hwnd);
        }
    } while (nID == IDOK && s_nNumberGenerated < s_nNumberToGenerate);
    ::EnableWindow(xg_hwndInputPalette, TRUE);

    // ����������B
    xg_xword = xword_save;
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_vecTateHints.clear();
    xg_vecYokoHints.clear();
    xg_strFileName.clear();

    // �C���[�W���X�V����B
    xg_bSolved = false;
    xg_bShowAnswer = false;
    xg_caret_pos.clear();
    XgMarkUpdate();
    XgUpdateImage(hwnd, 0, 0);

    // �I�����b�Z�[�W��\������B
    WCHAR sz[MAX_PATH];
    if (s_bOutOfDiskSpace) {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_OUTOFSTORAGE), s_nNumberGenerated,
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
    } else {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_PROBLEMSMADE), s_nNumberGenerated,
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
    }
    XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);

    // �ۑ���t�H���_���J���B
    if (s_nNumberGenerated && !s_dirs_save_to.empty()) {
        ::ShellExecuteW(hwnd, nullptr, s_dirs_save_to[0].data(),
                      nullptr, nullptr, SW_SHOWNORMAL);
    }
    return true;
}

// ���}�X���Ώ̃`�F�b�N�B
void XgOnLineSymmetryCheck(HWND hwnd)
{
    if (xg_xword.IsLineSymmetry()) {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_LINESYMMETRY), XgLoadStringDx2(IDS_APPNAME),
                          MB_ICONINFORMATION);
    } else {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_NOTLINESYMMETRY), XgLoadStringDx2(IDS_APPNAME),
                          MB_ICONINFORMATION);
    }
}

// ���}�X�_�Ώ̃`�F�b�N�B
void XgOnPointSymmetryCheck(HWND hwnd)
{
    if (xg_xword.IsPointSymmetry()) {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_POINTSYMMETRY), XgLoadStringDx2(IDS_APPNAME),
                          MB_ICONINFORMATION);
    } else {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_NOTPOINTSYMMETRY), XgLoadStringDx2(IDS_APPNAME),
                          MB_ICONINFORMATION);
    }
}

//////////////////////////////////////////////////////////////////////////////

// 24BPP�r�b�g�}�b�v���쐬�B
HBITMAP XgCreate24BppBitmap(HDC hDC, LONG width, LONG height)
{
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    return CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);
}

// BITMAPINFOEX�\���́B
typedef struct tagBITMAPINFOEX
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[256];
} BITMAPINFOEX, FAR * LPBITMAPINFOEX;

BOOL
PackedDIB_CreateFromHandle(std::vector<BYTE>& vecData, HBITMAP hbm)
{
    vecData.clear();

    BITMAP bm;
    if (!GetObject(hbm, sizeof(bm), &bm))
        return FALSE;

    BITMAPINFOEX bi;
    BITMAPINFOHEADER *pbmih;
    DWORD cColors, cbColors;

    pbmih = &bi.bmiHeader;
    ZeroMemory(pbmih, sizeof(BITMAPINFOHEADER));
    pbmih->biSize             = sizeof(BITMAPINFOHEADER);
    pbmih->biWidth            = bm.bmWidth;
    pbmih->biHeight           = bm.bmHeight;
    pbmih->biPlanes           = 1;
    pbmih->biBitCount         = bm.bmBitsPixel;
    pbmih->biCompression      = BI_RGB;
    pbmih->biSizeImage        = bm.bmWidthBytes * bm.bmHeight;

    if (bm.bmBitsPixel < 16)
        cColors = 1 << bm.bmBitsPixel;
    else
        cColors = 0;
    cbColors = cColors * sizeof(RGBQUAD);

    std::vector<BYTE> Bits(pbmih->biSizeImage);
    HDC hDC = CreateCompatibleDC(NULL);
    if (hDC == NULL)
        return FALSE;

    LPBITMAPINFO pbi = LPBITMAPINFO(&bi);
    if (!GetDIBits(hDC, hbm, 0, bm.bmHeight, &Bits[0], pbi, DIB_RGB_COLORS))
    {
        DeleteDC(hDC);
        return FALSE;
    }

    DeleteDC(hDC);

    std::string stream;
    stream.append((const char *)pbmih, sizeof(*pbmih));
    stream.append((const char *)bi.bmiColors, cbColors);
    stream.append((const char *)&Bits[0], Bits.size());
    vecData.assign(stream.begin(), stream.end());
    return TRUE;
}

// �N���b�v�{�[�h�ɃN���X���[�h���R�s�[�B
void __fastcall XgCopyBoard(HWND hwnd)
{
    std::wstring str;

    // �R�s�[����Ղ�I�ԁB
    XG_Board *pxw = (xg_bSolved && xg_bShowAnswer) ? &xg_solution : &xg_xword;

    // �N���X���[�h�̕�������擾����B
    pxw->GetString(str);

    // �q�[�v���烁�������m�ۂ���B
    DWORD cb = static_cast<DWORD>((str.size() + 1) * sizeof(WCHAR));
    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cb);
    if (hGlobal) {
        // �����������b�N����B
        LPWSTR psz = reinterpret_cast<LPWSTR>(::GlobalLock(hGlobal));
        if (psz) {
            // �������ɕ�������R�s�[����B
            ::CopyMemory(psz, str.data(), cb);
            // �������̃��b�N����������B
            ::GlobalUnlock(hGlobal);

            // �N���b�v�{�[�h���J���B
            if (::OpenClipboard(hwnd)) {
                // �N���b�v�{�[�h����ɂ���B
                if (::EmptyClipboard()) {
                    // Unicode�e�L�X�g��ݒ�B
                    ::SetClipboardData(CF_UNICODETEXT, hGlobal);

                    // �`��T�C�Y���擾����B
                    SIZE siz;
                    XgGetXWordExtent(&siz);

                    // EMF���쐬����B
                    HDC hdcRef = ::GetDC(hwnd);
                    HDC hdc = ::CreateEnhMetaFileW(hdcRef, nullptr, nullptr, XgLoadStringDx1(IDS_APPNAME));
                    if (hdc) {
                        // EMF�ɕ`�悷��B
                        XgDrawXWord(*pxw, hdc, &siz, false);

                        // EMF��ݒ�B
                        HENHMETAFILE hEMF = ::CloseEnhMetaFile(hdc);
                        ::SetClipboardData(CF_ENHMETAFILE, hEMF);

                        // DIB��ݒ�B
                        if (HDC hDC = CreateCompatibleDC(NULL))
                        {
                            HBITMAP hbm = XgCreate24BppBitmap(hDC, siz.cx, siz.cy);
                            HGDIOBJ hbmOld = SelectObject(hDC, hbm);
                            XgDrawXWord(*pxw, hDC, &siz, false);
                            SelectObject(hDC, hbmOld);
                            ::DeleteDC(hDC);

                            std::vector<BYTE> data;
                            if (PackedDIB_CreateFromHandle(data, hbm))
                            {
                                HGLOBAL hGlobal2 = GlobalAlloc(GHND | GMEM_SHARE, data.size());
                                LPVOID pv = GlobalLock(hGlobal2);
                                memcpy(pv, &data[0], data.size());
                                GlobalUnlock(hGlobal2);
                                ::SetClipboardData(CF_DIB, hGlobal2);
                            }
                            ::DeleteObject(hbm);
                        }
                    }
                    ::ReleaseDC(hwnd, hdcRef);
                }
                // �N���b�v�{�[�h�����B
                ::CloseClipboard();
                return;
            }
        }
        // �m�ۂ������������������B
        ::GlobalFree(hGlobal);
    }
}

// �N���b�v�{�[�h�ɃN���X���[�h���摜�Ƃ��ăR�s�[�B
void __fastcall XgCopyBoardAsImage(HWND hwnd)
{
    XG_Board *pxw = (xg_bSolved && xg_bShowAnswer) ? &xg_solution : &xg_xword;

    // �`��T�C�Y���擾����B
    SIZE siz;
    XgGetXWordExtent(&siz);

    // EMF���쐬����B
    HENHMETAFILE hEMF = NULL;
    HDC hdcRef = ::GetDC(hwnd);
    HDC hdc = ::CreateEnhMetaFileW(hdcRef, nullptr, nullptr, XgLoadStringDx1(IDS_APPNAME));
    if (hdc) {
        // EMF�ɕ`�悷��B
        XgDrawXWord(*pxw, hdc, &siz, false);
        hEMF = ::CloseEnhMetaFile(hdc);
    }

    // BMP���쐬����B
    HBITMAP hbm = NULL;
    if (HDC hDC = CreateCompatibleDC(NULL))
    {
        BITMAPINFO bi;
        ZeroMemory(&bi, sizeof(bi));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = siz.cx;
        bi.bmiHeader.biHeight = siz.cy;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 24;
        bi.bmiHeader.biCompression = BI_RGB;
        LPVOID pvBits;
        hbm = CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0);
        if (HGDIOBJ hbmOld = SelectObject(hDC, hbm))
        {
            RECT rc;
            SetRect(&rc, 0, 0, siz.cx, siz.cy);
            PlayEnhMetaFile(hDC, hEMF, &rc);
            SelectObject(hDC, hbmOld);
        }
        DeleteDC(hDC);
    }
    HGLOBAL hGlobal = NULL;
    if (hbm)
    {
        std::vector<BYTE> data;
        PackedDIB_CreateFromHandle(data, hbm);

        hGlobal = GlobalAlloc(GHND | GMEM_SHARE, DWORD(data.size()));
        if (hGlobal)
        {
            if (LPVOID pv = GlobalLock(hGlobal))
            {
                memcpy(pv, &data[0], data.size());
                GlobalUnlock(hGlobal);
            }
        }
        DeleteObject(hbm);
        hbm = NULL;
    }

    // �N���b�v�{�[�h���J���B
    if (::OpenClipboard(hwnd)) {
        // �N���b�v�{�[�h����ɂ���B
        if (::EmptyClipboard()) {
            // BMP��ݒ�B
            ::SetClipboardData(CF_DIB, hGlobal);
            // EMF��ݒ�B
            ::SetClipboardData(CF_ENHMETAFILE, hEMF);

            ::ReleaseDC(hwnd, hdcRef);
        }
        // �N���b�v�{�[�h�����B
        ::CloseClipboard();
    }
}

// �u�T�C�Y���w��v�_�C�A���O�v���V�[�W���[�B
INT_PTR CALLBACK
ImageSize_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);

        ::SetDlgItemInt(hwnd, edt1, s_nImageCopyWidth, FALSE);
        ::SetDlgItemInt(hwnd, edt2, s_nImageCopyHeight, FALSE);
        if (s_bImageCopyByHeight) {
            ::CheckRadioButton(hwnd, rad1, rad2, rad2);
            ::EnableWindow(::GetDlgItem(hwnd, edt1), FALSE);
            ::EnableWindow(::GetDlgItem(hwnd, edt2), TRUE);
        } else {
            ::CheckRadioButton(hwnd, rad1, rad2, rad1);
            ::EnableWindow(::GetDlgItem(hwnd, edt1), TRUE);
            ::EnableWindow(::GetDlgItem(hwnd, edt2), FALSE);
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case rad1:
            if (HIWORD(wParam) == BN_CLICKED) {
                ::EnableWindow(::GetDlgItem(hwnd, edt1), TRUE);
                ::EnableWindow(::GetDlgItem(hwnd, edt2), FALSE);
            }
            break;

        case rad2:
            if (HIWORD(wParam) == BN_CLICKED) {
                ::EnableWindow(::GetDlgItem(hwnd, edt1), FALSE);
                ::EnableWindow(::GetDlgItem(hwnd, edt2), TRUE);
            }
            break;

        case IDOK:
            s_nImageCopyWidth = ::GetDlgItemInt(hwnd, edt1, NULL, FALSE);
            if (s_nImageCopyWidth <= 0) {
                ::SendDlgItemMessageW(hwnd, edt1, EM_SETSEL, 0, -1);
                SetFocus(::GetDlgItem(hwnd, edt1));
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERPOSITIVE), NULL,
                                    MB_ICONERROR);
                break;
            }
            s_nImageCopyHeight = ::GetDlgItemInt(hwnd, edt2, NULL, FALSE);
            if (s_nImageCopyHeight <= 0) {
                ::SendDlgItemMessageW(hwnd, edt2, EM_SETSEL, 0, -1);
                SetFocus(::GetDlgItem(hwnd, edt2));
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERPOSITIVE), NULL,
                                    MB_ICONERROR);
                break;
            }
            if (::IsDlgButtonChecked(hwnd, rad2) == BST_CHECKED) {
                s_bImageCopyByHeight = true;
            } else {
                s_bImageCopyByHeight = false;
            }
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            ::EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
    return 0;
}

// �T�C�Y���w�肵�ăN���b�v�{�[�h�ɃN���X���[�h���摜�Ƃ��ăR�s�[�B
void __fastcall XgCopyBoardAsImageSized(HWND hwnd)
{
    if (IDOK != ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_SIZES), hwnd,
                             ImageSize_DlgProc))
    {
        return;
    }

    HENHMETAFILE hEMF = NULL, hEMF2 = NULL;
    XG_Board *pxw = (xg_bSolved && xg_bShowAnswer) ? &xg_solution : &xg_xword;

    // �`��T�C�Y���擾����B
    SIZE siz;
    XgGetXWordExtent(&siz);

    // EMF���쐬����B
    HDC hdcRef = ::GetDC(hwnd);
    HDC hdc = ::CreateEnhMetaFileW(hdcRef, nullptr, nullptr, XgLoadStringDx1(IDS_APPNAME));
    // EMF�ɕ`�悷��B
    XgDrawXWord(*pxw, hdc, &siz, false);
    hEMF = ::CloseEnhMetaFile(hdc);

    MRect rc;
    if (s_bImageCopyByHeight) {
        int cx = siz.cx * s_nImageCopyHeight / siz.cy;
        ::SetRect(&rc, 0, 0, cx, s_nImageCopyHeight);
    } else {
        int cy = siz.cy * s_nImageCopyWidth / siz.cx;
        ::SetRect(&rc, 0, 0, s_nImageCopyWidth, cy);
    }
    siz.cx = rc.right - rc.left;
    siz.cy = rc.bottom - rc.top;

    HDC hdc2 = ::CreateEnhMetaFileW(hdcRef, nullptr, nullptr, XgLoadStringDx1(IDS_APPNAME));
    ::PlayEnhMetaFile(hdc2, hEMF, &rc);
    hEMF2 = ::CloseEnhMetaFile(hdc2);
    ::DeleteEnhMetaFile(hEMF);

    // BMP���쐬����B
    HBITMAP hbm = NULL;
    if (HDC hDC = CreateCompatibleDC(NULL))
    {
        BITMAPINFO bi;
        ZeroMemory(&bi, sizeof(bi));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = siz.cx;
        bi.bmiHeader.biHeight = siz.cy;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 24;
        bi.bmiHeader.biCompression = BI_RGB;
        LPVOID pvBits;
        hbm = CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0);
        if (HGDIOBJ hbmOld = SelectObject(hDC, hbm))
        {
            PlayEnhMetaFile(hDC, hEMF2, &rc);
            SelectObject(hDC, hbmOld);
        }
        DeleteDC(hDC);
    }
    HGLOBAL hGlobal = NULL;
    if (hbm)
    {
        std::vector<BYTE> data;
        PackedDIB_CreateFromHandle(data, hbm);

        hGlobal = GlobalAlloc(GHND | GMEM_SHARE, DWORD(data.size()));
        if (hGlobal)
        {
            if (LPVOID pv = GlobalLock(hGlobal))
            {
                memcpy(pv, &data[0], data.size());
                GlobalUnlock(hGlobal);
            }
        }
        DeleteObject(hbm);
        hbm = NULL;
    }

    // �N���b�v�{�[�h���J���B
    if (::OpenClipboard(hwnd)) {
        // �N���b�v�{�[�h����ɂ���B
        if (::EmptyClipboard()) {
            // BMP��ݒ�B
            ::SetClipboardData(CF_DIB, hGlobal);
            // EMF��ݒ�B
            ::SetClipboardData(CF_ENHMETAFILE, hEMF2);
        }
        // �N���b�v�{�[�h�����B
        ::CloseClipboard();
    }

    ::ReleaseDC(hwnd, hdcRef);
}

// ��d�}�X�P����摜�Ƃ��ăR�s�[�B
void __fastcall XgCopyMarkWordAsImage(HWND hwnd)
{
    // �}�[�N���Ȃ���΁A���s�����ۂ���B
    if (xg_vMarks.empty()) {
        ::MessageBeep(0xFFFFFFFF);
        return;
    }

    // �`��T�C�Y���擾����B
    SIZE siz;
    int nCount = static_cast<int>(xg_vMarks.size());
    XgGetMarkWordExtent(nCount, &siz);

    // EMF���쐬����B
    HENHMETAFILE hEMF = NULL;
    HDC hdcRef = ::GetDC(hwnd);
    HDC hdc = ::CreateEnhMetaFileW(hdcRef, nullptr, nullptr, XgLoadStringDx1(IDS_APPNAME));
    if (hdc) {
        // EMF�ɕ`�悷��B
        XgDrawMarkWord(hdc, &siz);
        ::ReleaseDC(hwnd, hdcRef);
        hEMF = ::CloseEnhMetaFile(hdc);
    }

    // �N���b�v�{�[�h���J���B
    if (::OpenClipboard(hwnd)) {
        // �N���b�v�{�[�h����ɂ���B
        if (::EmptyClipboard()) {
            // EMF��ݒ�B
            ::SetClipboardData(CF_ENHMETAFILE, hEMF);
        }
        // �N���b�v�{�[�h�����B
        ::CloseClipboard();
    }
}

// �u�������w��v�_�C�A���O�v���V�[�W���[�B
INT_PTR CALLBACK
MarksHeight_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);

        ::SetDlgItemInt(hwnd, edt1, s_nMarksHeight, FALSE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            s_nMarksHeight = ::GetDlgItemInt(hwnd, edt1, NULL, FALSE);
            if (s_nMarksHeight <= 0) {
                ::SendDlgItemMessageW(hwnd, edt1, EM_SETSEL, 0, -1);
                SetFocus(::GetDlgItem(hwnd, edt1));
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ENTERPOSITIVE), NULL,
                                    MB_ICONERROR);
                break;
            }
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            ::EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
    return 0;
}

// �������w�肵�ē�d�}�X�P����摜�Ƃ��ăR�s�[�B
void __fastcall XgCopyMarkWordAsImageSized(HWND hwnd)
{
    // �}�[�N���Ȃ���΁A���s�����ۂ���B
    if (xg_vMarks.empty()) {
        ::MessageBeep(0xFFFFFFFF);
        return;
    }

    if (IDOK != ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_HEIGHT), hwnd,
                             MarksHeight_DlgProc))
    {
        return;
    }

    HENHMETAFILE hEMF = NULL, hEMF2 = NULL;

    // �`��T�C�Y���擾����B
    SIZE siz;
    int nCount = static_cast<int>(xg_vMarks.size());
    XgGetMarkWordExtent(nCount, &siz);

    // EMF���쐬����B
    HDC hdcRef = ::GetDC(hwnd);
    HDC hdc = ::CreateEnhMetaFileW(hdcRef, nullptr, nullptr, XgLoadStringDx1(IDS_APPNAME));
    // EMF�ɕ`�悷��B
    XgDrawMarkWord(hdc, &siz);
    hEMF = ::CloseEnhMetaFile(hdc);

    MRect rc;
    int cx = siz.cx * s_nMarksHeight / siz.cy;
    ::SetRect(&rc, 0, 0, cx, s_nMarksHeight);

    // EMF���쐬����B
    HDC hdc2 = ::CreateEnhMetaFileW(hdcRef, nullptr, nullptr, XgLoadStringDx1(IDS_APPNAME));
    ::PlayEnhMetaFile(hdc2, hEMF, &rc);
    hEMF2 = ::CloseEnhMetaFile(hdc2);

    ::DeleteEnhMetaFile(hEMF);
    ::ReleaseDC(hwnd, hdcRef);

    // BMP���쐬����B
    HBITMAP hbm = NULL;
    if (HDC hDC = CreateCompatibleDC(NULL))
    {
        BITMAPINFO bi;
        ZeroMemory(&bi, sizeof(bi));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = rc.right - rc.left;
        bi.bmiHeader.biHeight = rc.bottom - rc.top;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 24;
        bi.bmiHeader.biCompression = BI_RGB;
        LPVOID pvBits;
        hbm = CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0);
        if (HGDIOBJ hbmOld = SelectObject(hDC, hbm))
        {
            PlayEnhMetaFile(hDC, hEMF2, &rc);
            SelectObject(hDC, hbmOld);
        }
        DeleteDC(hDC);
    }
    HGLOBAL hGlobal = NULL;
    if (hbm)
    {
        std::vector<BYTE> data;
        PackedDIB_CreateFromHandle(data, hbm);

        hGlobal = GlobalAlloc(GHND | GMEM_SHARE, DWORD(data.size()));
        if (hGlobal)
        {
            if (LPVOID pv = GlobalLock(hGlobal))
            {
                memcpy(pv, &data[0], data.size());
                GlobalUnlock(hGlobal);
            }
        }
        DeleteObject(hbm);
        hbm = NULL;
    }

    // �N���b�v�{�[�h���J���B
    if (::OpenClipboard(hwnd)) {
        // �N���b�v�{�[�h����ɂ���B
        if (::EmptyClipboard()) {
            // EMF��ݒ�B
            ::SetClipboardData(CF_ENHMETAFILE, hEMF2);
            // BMP��ݒ�B
            ::SetClipboardData(CF_DIB, hGlobal);
        }
        // �N���b�v�{�[�h�����B
        ::CloseClipboard();
    }
}

std::wstring XgGetClipboardUnicodeText(HWND hwnd)
{
    std::wstring str;

    // �N���b�v�{�[�h���J���B
    HGLOBAL hGlobal;
    if (::OpenClipboard(hwnd)) {
        // Unicode��������擾�B
        hGlobal = ::GetClipboardData(CF_UNICODETEXT);
        LPWSTR psz = reinterpret_cast<LPWSTR>(::GlobalLock(hGlobal));
        if (psz) {
            str = psz;
            ::GlobalUnlock(hGlobal);
        }
        // �N���b�v�{�[�h�����B
        ::CloseClipboard();
    }

    return str;
}

// �N���b�v�{�[�h����\��t���B
void __fastcall XgPasteBoard(HWND hwnd, const std::wstring& str)
{
    // �����񂪋󂶂�Ȃ����H
    if (!str.empty()) {
        // �����񂪃N���X���[�h��\���Ă���Ɖ��肷��B
        // �N���X���[�h�ɕ������ݒ�B
        if (xg_xword.SetString(str)) {
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �q���g�E�B���h�E��j������B
            XgDestroyHintsWnd();

            xg_bSolved = false;
            xg_bShowAnswer = false;
            xg_caret_pos.clear();
            xg_vMarks.clear();
            XgMarkUpdate();
            XgUpdateImage(hwnd, 0, 0);
        } else {
            ::MessageBeep(0xFFFFFFFF);
        }
    }
}

// �N���b�v�{�[�h�Ƀe�L�X�g���R�s�[����B
BOOL XgSetClipboardUnicodeText(HWND hwnd, const std::wstring& str)
{
    // �q�[�v���烁�������m�ۂ���B
    DWORD cb = static_cast<DWORD>((str.size() + 1) * sizeof(WCHAR));
    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cb);
    if (hGlobal) {
        // �����������b�N����B
        LPWSTR psz = reinterpret_cast<LPWSTR>(::GlobalLock(hGlobal));
        if (psz) {
            // �������ɕ�������R�s�[����B
            ::CopyMemory(psz, str.data(), cb);
            // �������̃��b�N����������B
            ::GlobalUnlock(hGlobal);

            // �N���b�v�{�[�h���J���B
            if (::OpenClipboard(hwnd)) {
                // �N���b�v�{�[�h����ɂ���B
                if (::EmptyClipboard()) {
                    // Unicode�e�L�X�g��ݒ�B
                    ::SetClipboardData(CF_UNICODETEXT, hGlobal);
                }
                // �N���b�v�{�[�h�����B
                ::CloseClipboard();
                return TRUE;
            }
        }
        // �m�ۂ������������������B
        ::GlobalFree(hGlobal);
    }

    return FALSE;
}

// �N���b�v�{�[�h�Ƀq���g���R�s�[�i�X�^�C���[���j�B
void __fastcall XgCopyHintsStyle0(HWND hwnd, int hint_type)
{
    // ������Ă��Ȃ���Ώ��������ۂ���B
    if (!xg_bSolved) {
        ::MessageBeep(0xFFFFFFFF);
        return;
    }

    // �N���X���[�h�̕�������擾����B
    std::wstring str;
    xg_solution.GetHintsStr(str, hint_type, false);
    xg_str_trim(str);

    // �N���b�v�{�[�h�Ƀe�L�X�g���R�s�[����B
    XgSetClipboardUnicodeText(hwnd, str);
}

// �N���b�v�{�[�h�Ƀq���g���R�s�[�i�X�^�C�������j�B
void __fastcall XgCopyHintsStyle1(HWND hwnd, int hint_type)
{
    // ������Ă��Ȃ��Ƃ��́A���������ۂ���B
    if (!xg_bSolved) {
        ::MessageBeep(0xFFFFFFFF);
        return;
    }

    // �N���X���[�h�̕�������擾����B
    std::wstring str;
    xg_solution.GetHintsStr(str, hint_type, false);
    xg_str_trim(str);

    // �X�^�C�������ł͗v��Ȃ��������폜����B
    xg_str_replace_all(str, XgLoadStringDx1(IDS_DOWNLEFT), L"");
    xg_str_replace_all(str, XgLoadStringDx1(IDS_ACROSSLEFT), L"");
    xg_str_replace_all(str, XgLoadStringDx1(IDS_KEYRIGHT), XgLoadStringDx2(IDS_DOT));

    // HTML�f�[�^ (UTF-8)��p�ӂ���B
    std::wstring html;
    xg_solution.GetHintsStr(html, hint_type + 3, false);
    xg_str_trim(html);
    std::string htmldata = XgMakeClipHtmlData(html,
        L"p, ol, li { margin-top: 0px; margin-bottom: 0px; }\r\n");

    // �N���b�v�{�[�h��HTML�`����o�^����B
    UINT CF_HTML = ::RegisterClipboardFormatW(L"HTML Format");

    // �q�[�v���烁�������m�ۂ���B
    DWORD cb = static_cast<DWORD>((str.size() + 1) * sizeof(WCHAR));
    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cb);
    if (hGlobal) {
        // �����������b�N����B
        LPWSTR psz = reinterpret_cast<LPWSTR>(::GlobalLock(hGlobal));
        if (psz) {
            // �������ɕ�������R�s�[����B
            ::CopyMemory(psz, str.data(), cb);
            // �������̃��b�N����������B
            ::GlobalUnlock(hGlobal);

            // �q�[�v���烁�������m�ۂ���B
            cb = static_cast<DWORD>((htmldata.size() + 1) * sizeof(char));
            HGLOBAL hGlobal2 = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cb);
            if (hGlobal2) {
                // �����������b�N����B
                LPVOID p2 = ::GlobalLock(hGlobal2);
                if (p2) {
                    // �������ɕ�������R�s�[����B
                    LPBYTE pb2 = reinterpret_cast<LPBYTE>(p2);
                    CopyMemory(pb2, htmldata.data(), htmldata.size());
                    pb2[htmldata.size()] = 0;
                    // �������̃��b�N����������B
                    ::GlobalUnlock(hGlobal);

                    // �N���b�v�{�[�h���J���B
                    if (::OpenClipboard(hwnd)) {
                        // �N���b�v�{�[�h����ɂ���B
                        if (::EmptyClipboard()) {
                            // Unicode�e�L�X�g��ݒ�B
                            ::SetClipboardData(CF_UNICODETEXT, hGlobal);
                            // HTML�f�[�^��ݒ�B
                            ::SetClipboardData(CF_HTML, hGlobal2);
                        }
                        // �N���b�v�{�[�h�����B
                        ::CloseClipboard();
                        return;
                    }
                }
                // �m�ۂ������������������B
                ::GlobalFree(hGlobal2);
            }
        }
        // �m�ۂ������������������B
        ::GlobalFree(hGlobal);
    }
}

//////////////////////////////////////////////////////////////////////////////

// �E�B���h�E���j�����ꂽ�B
void __fastcall MainWnd_OnDestroy(HWND /*hwnd*/)
{
    // �C���[�W���X�g��j������B
    ::ImageList_Destroy(xg_hImageList);
    xg_hImageList = NULL;
    ::ImageList_Destroy(xg_hGrayedImageList);
    xg_hGrayedImageList = NULL;

    // �E�B���h�E��j������B
    ::DestroyWindow(xg_hToolBar);
    xg_hToolBar = NULL;
    ::DestroyWindow(xg_hHScrollBar);
    xg_hHScrollBar = NULL;
    ::DestroyWindow(xg_hVScrollBar);
    xg_hVScrollBar = NULL;
    ::DestroyWindow(xg_hSizeGrip);
    xg_hSizeGrip = NULL;
    ::DestroyWindow(xg_hCandsWnd);
    xg_hCandsWnd = NULL;
    ::DestroyWindow(xg_hHintsWnd);
    xg_hHintsWnd = NULL;
    ::DestroyWindow(xg_hwndInputPalette);
    xg_hwndInputPalette = NULL;

    // �A�v�����I������B
    ::PostQuitMessage(0);

    xg_hMainWnd = NULL;
}

// �E�B���h�E��`�悷��B
void __fastcall MainWnd_OnPaint(HWND hwnd)
{
    // �c�[���o�[���Ȃ���΁A�������̑O�Ȃ̂ŁA��������B
    if (xg_hToolBar == NULL)
        return;

    // �N���X���[�h�̕`��T�C�Y���擾����B
    SIZE siz;
    ForDisplay for_display;
    XgGetXWordExtent(&siz);

    // �`����J�n����B
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(hwnd, &ps);
    assert(hdc);
    if (hdc) {
        // �C���[�W���Ȃ��ꍇ�́A�C���[�W���擾����B
        if (xg_hbmImage == nullptr) {
            if (xg_bSolved && xg_bShowAnswer)
                xg_hbmImage = XgCreateXWordImage(xg_solution, &siz, true);
            else
                xg_hbmImage = XgCreateXWordImage(xg_xword, &siz, true);
        }

        // �N���C�A���g�̈�𓾂�B
        MRect rcClient;
        XgGetRealClientRect(hwnd, &rcClient);

        // �X�N���[���ʒu���擾����B
        int x = XgGetHScrollPos();
        int y = XgGetVScrollPos();

        // �r�b�g�}�b�v �C���[�W���E�B���h�E�ɓ]������B
        HDC hdcMem = ::CreateCompatibleDC(hdc);
        ::IntersectClipRect(hdc, rcClient.left, rcClient.top,
            rcClient.right, rcClient.bottom);
        HGDIOBJ hbmOld = ::SelectObject(hdcMem, xg_hbmImage);
        ::BitBlt(hdc, rcClient.left - x, rcClient.top - y,
            siz.cx, siz.cy, hdcMem, 0, 0, SRCCOPY);
        ::SelectObject(hdcMem, hbmOld);
        ::DeleteDC(hdcMem);

        // �`����I������B
        ::EndPaint(hwnd, &ps);
    }
}

// ���X�N���[������B
void __fastcall MainWnd_OnHScroll(HWND hwnd, HWND /*hwndCtl*/, UINT code, int pos)
{
    SCROLLINFO si;

    // ���X�N���[�������擾����B
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    XgGetHScrollInfo(&si);

    // �R�[�h�ɉ����Ĉʒu����ݒ肷��B
    switch (code) {
    case SB_LEFT:
        si.nPos = si.nMin;
        break;

    case SB_RIGHT:
        si.nPos = si.nMax;
        break;

    case SB_LINELEFT:
        si.nPos -= 10;
        if (si.nPos < si.nMin)
            si.nPos = si.nMin;
        break;

    case SB_LINERIGHT:
        si.nPos += 10;
        if (si.nPos > si.nMax)
            si.nPos = si.nMax;
        break;

    case SB_PAGELEFT:
        si.nPos -= si.nPage;
        if (si.nPos < si.nMin)
            si.nPos = si.nMin;
        break;

    case SB_PAGERIGHT:
        si.nPos += si.nPage;
        if (si.nPos > si.nMax)
            si.nPos = si.nMax;
        break;

    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        si.nPos = pos;
        break;
    }

    // �X�N���[������ݒ肵�A�C���[�W���X�V����B
    XgSetHScrollInfo(&si, TRUE);
    XgUpdateImage(hwnd, si.nPos, XgGetVScrollPos());
}

// �c�X�N���[������B
void __fastcall MainWnd_OnVScroll(HWND hwnd, HWND /*hwndCtl*/, UINT code, int pos)
{
    SCROLLINFO si;

    // �c�X�N���[�������擾����B
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
    XgGetVScrollInfo(&si);

    // �R�[�h�ɉ����Ĉʒu����ݒ肷��B
    switch (code) {
    case SB_TOP:
        si.nPos = si.nMin;
        break;

    case SB_BOTTOM:
        si.nPos = si.nMax;
        break;

    case SB_LINEUP:
        si.nPos -= 10;
        if (si.nPos < si.nMin)
            si.nPos = si.nMin;
        break;

    case SB_LINEDOWN:
        si.nPos += 10;
        if (si.nPos > si.nMax)
            si.nPos = si.nMax;
        break;

    case SB_PAGEUP:
        si.nPos -= si.nPage;
        if (si.nPos < si.nMin)
            si.nPos = si.nMin;
        break;

    case SB_PAGEDOWN:
        si.nPos += si.nPage;
        if (si.nPos > si.nMax)
            si.nPos = si.nMax;
        break;

    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        si.nPos = pos;
        break;
    }

    // �X�N���[������ݒ肵�A�C���[�W���X�V����B
    XgSetVScrollPos(si.nPos, TRUE);
    XgUpdateImage(hwnd, XgGetHScrollPos(), si.nPos);
}

// �u�����v���j���[���擾����B
HMENU DoFindDictMenu(HMENU hMenu)
{
    WCHAR szText[128];
    LPCWSTR pszDict = XgLoadStringDx1(IDS_DICTIONARY);
    for (INT i = 0; i < 16; ++i)
    {
        if (GetMenuStringW(hMenu, i, szText, ARRAYSIZE(szText), MF_BYPOSITION))
        {
            if (wcsstr(szText, pszDict) != NULL)
            {
                return GetSubMenu(hMenu, i);
            }
        }
    }
    return NULL;
}

// �u�����v���j���[���X�V����B
void DoUpdateDictMenu(HMENU hDictMenu)
{
    while (RemoveMenu(hDictMenu, 4, MF_BYPOSITION))
    {
        ;
    }

    if (xg_dict_files.empty())
    {
        AppendMenuW(hDictMenu, MF_STRING | MF_GRAYED, -1, XgLoadStringDx1(IDS_NONE));
        return;
    }

    INT index = 4, count = 0, id = ID_DICTIONARY00;
    WCHAR szText[MAX_PATH];
    for (const auto& file : xg_dict_files)
    {
        LPCWSTR pszFileTitle = PathFindFileNameW(file.c_str());
        StringCbPrintfW(szText, sizeof(szText), L"&%c ", L"0123456789ABCDEF"[count]);
        StringCbCatW(szText, sizeof(szText), pszFileTitle);
        AppendMenuW(hDictMenu, MF_STRING | MF_ENABLED, id, szText);
        ++index;
        ++count;
        ++id;
        if (count >= MAX_DICTS)
            break;
    }

    index = 4;
    for (const auto& file : xg_dict_files)
    {
        if (lstrcmpiW(file.c_str(), xg_dict_name.c_str()) == 0)
        {
            INT nCount = GetMenuItemCount(hDictMenu);
            CheckMenuRadioItem(hDictMenu, 2, nCount - 1, index, MF_BYPOSITION);
            break;
        }
        ++index;
    }
}

// ���j���[������������B
void __fastcall MainWnd_OnInitMenu(HWND /*hwnd*/, HMENU hMenu)
{
    if (HMENU hDictMenu = DoFindDictMenu(hMenu))
    {
        // �������j���[���X�V�B
        DoUpdateDictMenu(hDictMenu);
    }

    // �X�P���g�����[�h�B
    if (xg_bSkeltonMode) {
        CheckMenuItem(hMenu, ID_SKELTONMODE, MF_BYCOMMAND | MF_CHECKED);
    } else {
        CheckMenuItem(hMenu, ID_SKELTONMODE, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // ������\�����邩�H
    if (xg_bShowNumbering) {
        CheckMenuItem(hMenu, ID_SHOWHIDENUMBERING, MF_BYCOMMAND | MF_CHECKED);
    } else {
        CheckMenuItem(hMenu, ID_SHOWHIDENUMBERING, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // �L�����b�g��\�����邩�H
    if (xg_bShowCaret) {
        CheckMenuItem(hMenu, ID_SHOWHIDECARET, MF_BYCOMMAND | MF_CHECKED);
    } else {
        CheckMenuItem(hMenu, ID_SHOWHIDECARET, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // ��莞�Ԃ��߂����烊�g���C�B
    if (s_bAutoRetry) {
        CheckMenuItem(hMenu, ID_RETRYIFTIMEOUT, MF_BYCOMMAND | MF_CHECKED);
    } else {
        CheckMenuItem(hMenu, ID_RETRYIFTIMEOUT, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // �e�[�}�B
    if (!xg_bThemeModified) {
        CheckMenuItem(hMenu, ID_THEME, MF_BYCOMMAND | MF_UNCHECKED);
        EnableMenuItem(hMenu, ID_RESETTHEME, MF_BYCOMMAND | MF_GRAYED);
    } else {
        CheckMenuItem(hMenu, ID_THEME, MF_BYCOMMAND | MF_CHECKED);
        if (xg_tag_histgram.empty()) {
            EnableMenuItem(hMenu, ID_RESETTHEME, MF_BYCOMMAND | MF_GRAYED);
        } else {
            EnableMenuItem(hMenu, ID_RESETTHEME, MF_BYCOMMAND | MF_ENABLED);
        }
    }

    // �A���ցB
    if (xg_nRules & RULE_DONTDOUBLEBLACK)
        ::CheckMenuItem(hMenu, ID_RULE_DONTDOUBLEBLACK, MF_CHECKED);
    else
        ::CheckMenuItem(hMenu, ID_RULE_DONTDOUBLEBLACK, MF_UNCHECKED);
    // �l�����ցB
    if (xg_nRules & RULE_DONTCORNERBLACK)
        ::CheckMenuItem(hMenu, ID_RULE_DONTCORNERBLACK, MF_CHECKED);
    else
        ::CheckMenuItem(hMenu, ID_RULE_DONTCORNERBLACK, MF_UNCHECKED);
    // �O�����ցB
    if (xg_nRules & RULE_DONTTRIDIRECTIONS)
        ::CheckMenuItem(hMenu, ID_RULE_DONTTRIDIRECTIONS, MF_CHECKED);
    else
        ::CheckMenuItem(hMenu, ID_RULE_DONTTRIDIRECTIONS, MF_UNCHECKED);
    // ���f�ցB
    if (xg_nRules & RULE_DONTDIVIDE)
        ::CheckMenuItem(hMenu, ID_RULE_DONTDIVIDE, MF_CHECKED);
    else
        ::CheckMenuItem(hMenu, ID_RULE_DONTDIVIDE, MF_UNCHECKED);
    // ���ΎO�A�ցB
    if (xg_nRules & RULE_DONTTHREEDIAGONALS) {
        ::CheckMenuItem(hMenu, ID_RULE_DONTTHREEDIAGONALS, MF_CHECKED);
        ::CheckMenuItem(hMenu, ID_RULE_DONTFOURDIAGONALS, MF_UNCHECKED);
    } else {
        ::CheckMenuItem(hMenu, ID_RULE_DONTTHREEDIAGONALS, MF_UNCHECKED);
        // ���Ύl�A�ցB
        if (xg_nRules & RULE_DONTFOURDIAGONALS)
            ::CheckMenuItem(hMenu, ID_RULE_DONTFOURDIAGONALS, MF_CHECKED);
        else
            ::CheckMenuItem(hMenu, ID_RULE_DONTFOURDIAGONALS, MF_UNCHECKED);
    }
    // ���}�X�_�Ώ́B
    if (xg_nRules & RULE_POINTSYMMETRY)
        ::CheckMenuItem(hMenu, ID_RULE_POINTSYMMETRY, MF_CHECKED);
    else
        ::CheckMenuItem(hMenu, ID_RULE_POINTSYMMETRY, MF_UNCHECKED);

    // ���̓��[�h�B
    switch (xg_imode) {
    case xg_im_KANA:
        ::CheckMenuRadioItem(hMenu, ID_KANAINPUT, ID_DIGITINPUT, ID_KANAINPUT, MF_BYCOMMAND);
        break;

    case xg_im_ABC:
        ::CheckMenuRadioItem(hMenu, ID_KANAINPUT, ID_DIGITINPUT, ID_ABCINPUT, MF_BYCOMMAND);
        break;

    case xg_im_KANJI:
        ::CheckMenuRadioItem(hMenu, ID_KANAINPUT, ID_DIGITINPUT, ID_KANJIINPUT, MF_BYCOMMAND);
        break;

    case xg_im_RUSSIA:
        ::CheckMenuRadioItem(hMenu, ID_KANAINPUT, ID_DIGITINPUT, ID_RUSSIAINPUT, MF_BYCOMMAND);
        break;

    case xg_im_DIGITS:
        ::CheckMenuRadioItem(hMenu, ID_KANAINPUT, ID_DIGITINPUT, ID_DIGITINPUT, MF_BYCOMMAND);
        break;
    }

    // �u���ɖ߂��v�u��蒼���v���j���[�X�V�B
    if (xg_ubUndoBuffer.CanUndo()) {
        ::EnableMenuItem(hMenu, ID_UNDO, MF_BYCOMMAND | MF_ENABLED);
    } else {
        ::EnableMenuItem(hMenu, ID_UNDO, MF_BYCOMMAND | MF_GRAYED);
    }
    if (xg_ubUndoBuffer.CanRedo()) {
        ::EnableMenuItem(hMenu, ID_REDO, MF_BYCOMMAND | MF_ENABLED);
    } else {
        ::EnableMenuItem(hMenu, ID_REDO, MF_BYCOMMAND | MF_GRAYED);
    }

    if (xg_bSolved) {
        ::EnableMenuItem(hMenu, ID_SOLVE, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_SOLVENOADDBLACK, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_SOLVEREPEATEDLY, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_SOLVEREPEATEDLYNOADDBLACK, MF_BYCOMMAND | MF_GRAYED);
        if (xg_bShowAnswer) {
            ::EnableMenuItem(hMenu, ID_SHOWSOLUTION, MF_BYCOMMAND | MF_GRAYED);
            ::EnableMenuItem(hMenu, ID_NOSOLUTION, MF_BYCOMMAND | MF_ENABLED);
            ::CheckMenuRadioItem(hMenu, ID_SHOWSOLUTION, ID_NOSOLUTION, ID_SHOWSOLUTION, MF_BYCOMMAND);
            ::CheckMenuItem(hMenu, ID_SHOWHIDESOLUTION, MF_BYCOMMAND | MF_CHECKED);
        } else {
            ::EnableMenuItem(hMenu, ID_SHOWSOLUTION, MF_BYCOMMAND | MF_ENABLED);
            ::EnableMenuItem(hMenu, ID_NOSOLUTION, MF_BYCOMMAND | MF_GRAYED);
            ::CheckMenuRadioItem(hMenu, ID_SHOWSOLUTION, ID_NOSOLUTION, ID_NOSOLUTION, MF_BYCOMMAND);
            ::CheckMenuItem(hMenu, ID_SHOWHIDESOLUTION, MF_BYCOMMAND | MF_UNCHECKED);
        }
        ::EnableMenuItem(hMenu, ID_SHOWHIDEHINTS, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_MARKSNEXT, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_MARKSPREV, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_SAVEANSASIMAGE, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_PRINTANSWER, MF_BYCOMMAND | MF_ENABLED);

        ::EnableMenuItem(hMenu, ID_COPYHINTSSTYLE0, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_COPYHINTSSTYLE1, MF_BYCOMMAND | MF_ENABLED);

        ::EnableMenuItem(hMenu, ID_COPYHHINTSSTYLE0, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_COPYVHINTSSTYLE0, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_COPYHHINTSSTYLE1, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_COPYVHINTSSTYLE1, MF_BYCOMMAND | MF_ENABLED);

        ::EnableMenuItem(hMenu, ID_OPENCANDSWNDHORZ, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_OPENCANDSWNDVERT, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_MOSTLEFT, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_MOSTRIGHT, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_MOSTUPPER, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_MOSTLOWER, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_LEFT, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_RIGHT, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_UP, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_DOWN, MF_BYCOMMAND | MF_GRAYED);
    } else {
        ::EnableMenuItem(hMenu, ID_SOLVE, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_SOLVENOADDBLACK, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_SOLVEREPEATEDLY, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_SOLVEREPEATEDLYNOADDBLACK, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_SHOWSOLUTION, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_NOSOLUTION, MF_BYCOMMAND | MF_GRAYED);
        ::CheckMenuRadioItem(hMenu, ID_SHOWSOLUTION, ID_NOSOLUTION, ID_NOSOLUTION, MF_BYCOMMAND);
        ::EnableMenuItem(hMenu, ID_SHOWHIDEHINTS, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_MARKSNEXT, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_MARKSPREV, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_SAVEANSASIMAGE, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_PRINTANSWER, MF_BYCOMMAND | MF_GRAYED);

        ::EnableMenuItem(hMenu, ID_COPYHINTSSTYLE0, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_COPYHINTSSTYLE1, MF_BYCOMMAND | MF_GRAYED);

        ::EnableMenuItem(hMenu, ID_COPYHHINTSSTYLE0, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_COPYVHINTSSTYLE0, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_COPYHHINTSSTYLE1, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_COPYVHINTSSTYLE1, MF_BYCOMMAND | MF_GRAYED);

        ::EnableMenuItem(hMenu, ID_OPENCANDSWNDHORZ, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_OPENCANDSWNDVERT, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_MOSTLEFT, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_MOSTRIGHT, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_MOSTUPPER, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_MOSTLOWER, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_LEFT, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_RIGHT, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_UP, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_DOWN, MF_BYCOMMAND | MF_ENABLED);
    }

    // ��d�}�X���j���[�X�V�B
    if (xg_vMarks.empty()) {
        ::EnableMenuItem(hMenu, ID_KILLMARKS, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_COPYMARKWORDASIMAGE, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, ID_COPYMARKWORDASIMAGESIZED, MF_BYCOMMAND | MF_GRAYED);
    } else {
        ::EnableMenuItem(hMenu, ID_KILLMARKS, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_COPYMARKWORDASIMAGE, MF_BYCOMMAND | MF_ENABLED);
        ::EnableMenuItem(hMenu, ID_COPYMARKWORDASIMAGESIZED, MF_BYCOMMAND | MF_ENABLED);
    }

    // �u�����폜���ĔՂ̃��b�N�������v
    if (xg_bSolved) {
        ::EnableMenuItem(hMenu, ID_ERASESOLUTIONANDUNLOCKEDIT, MF_BYCOMMAND | MF_ENABLED);
    } else {
        ::EnableMenuItem(hMenu, ID_ERASESOLUTIONANDUNLOCKEDIT, MF_BYCOMMAND | MF_GRAYED);
    }

    // �X�e�[�^�X�o�[�̃��j���[�X�V�B
    if (s_bShowStatusBar) {
        ::CheckMenuItem(hMenu, ID_STATUS, MF_BYCOMMAND | MF_CHECKED);
    } else {
        ::CheckMenuItem(hMenu, ID_STATUS, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // ���̓p���b�g�̃��j���[�X�V�B
    if (xg_hwndInputPalette) {
        ::CheckMenuItem(hMenu, ID_PALETTE, MF_BYCOMMAND | MF_CHECKED);
        ::CheckMenuItem(hMenu, ID_PALETTE2, MF_BYCOMMAND | MF_CHECKED);
    } else {
        ::CheckMenuItem(hMenu, ID_PALETTE, MF_BYCOMMAND | MF_UNCHECKED);
        ::CheckMenuItem(hMenu, ID_PALETTE2, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // �c�[���o�[�̃��j���[�X�V�B
    if (s_bShowToolBar) {
        ::CheckMenuItem(hMenu, ID_TOOLBAR, MF_BYCOMMAND | MF_CHECKED);
    } else {
        ::CheckMenuItem(hMenu, ID_TOOLBAR, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // �q���g�E�B���h�E�̃��j���[�X�V�B
    if (xg_hHintsWnd) {
        ::CheckMenuItem(hMenu, ID_SHOWHIDEHINTS, MF_BYCOMMAND | MF_CHECKED);
    } else {
        ::CheckMenuItem(hMenu, ID_SHOWHIDEHINTS, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // �Ђ炪�ȃE�B���h�E�̃��j���[�X�V�B
    if (xg_bHiragana) {
        ::CheckMenuRadioItem(hMenu, ID_HIRAGANA, ID_KATAKANA, ID_HIRAGANA, MF_BYCOMMAND);
    } else {
        ::CheckMenuRadioItem(hMenu, ID_HIRAGANA, ID_KATAKANA, ID_KATAKANA, MF_BYCOMMAND);
    }

    // Lowercase�E�B���h�E�̃��j���[�X�V�B
    if (xg_bLowercase) {
        ::CheckMenuRadioItem(hMenu, ID_UPPERCASE, ID_LOWERCASE, ID_LOWERCASE, MF_BYCOMMAND);
    } else {
        ::CheckMenuRadioItem(hMenu, ID_UPPERCASE, ID_LOWERCASE, ID_UPPERCASE, MF_BYCOMMAND);
    }

    // �^�e���R���͂̃��j���[�X�V�B
    if (xg_bTateInput) {
        ::CheckMenuRadioItem(hMenu, ID_INPUTH, ID_INPUTV, ID_INPUTV, MF_BYCOMMAND);
    } else {
        ::CheckMenuRadioItem(hMenu, ID_INPUTH, ID_INPUTV, ID_INPUTH, MF_BYCOMMAND);
    }

    // ��������̃��j���[�X�V�B
    if (xg_bCharFeed) {
        ::CheckMenuItem(hMenu, ID_CHARFEED, MF_BYCOMMAND | MF_CHECKED);
    } else {
        ::CheckMenuItem(hMenu, ID_CHARFEED, MF_BYCOMMAND | MF_UNCHECKED);
    }
}

// �}�E�X�̍��{�^���������ꂽ�B
void __fastcall MainWnd_OnLButtonUp(HWND hwnd, int x, int y, UINT /*keyFlags*/)
{
    int i, j;
    RECT rc;
    POINT pt;
    INT nCellSize = xg_nCellSize * xg_nZoomRate / 100;

    // ���{�^���������ꂽ�ʒu�����߂�B
    pt.x = x + XgGetHScrollPos();
    pt.y = y + XgGetVScrollPos();

    // �c�[���o�[���\������Ă�����A�ʒu��␳����B
    if (::IsWindowVisible(xg_hToolBar)) {
        ::GetWindowRect(xg_hToolBar, &rc);
        pt.y -= (rc.bottom - rc.top);
    }

    // ���ꂼ��̃}�X�ɂ��Ē��ׂ�B
    for (i = 0; i < xg_nRows; i++) {
        for (j = 0; j < xg_nCols; j++) {
            // �}�X�̋�`�����߂�B
            ::SetRect(&rc,
                static_cast<int>(xg_nMargin + j * nCellSize),
                static_cast<int>(xg_nMargin + i * nCellSize),
                static_cast<int>(xg_nMargin + (j + 1) * nCellSize),
                static_cast<int>(xg_nMargin + (i + 1) * nCellSize));

            // �}�X�̒����H
            if (::PtInRect(&rc, pt)) {
                // �L�����b�g���ړ����āA�C���[�W���X�V����B
                xg_caret_pos.m_i = i;
                xg_caret_pos.m_j = j;
                XgEnsureCaretVisible(hwnd);
                XgUpdateStatusBar(hwnd);

                // �C���[�W���X�V����B
                x = XgGetHScrollPos();
                y = XgGetVScrollPos();
                XgUpdateImage(hwnd, x, y);
                return;
            }
        }
    }
}

// �_�u���N���b�N���ꂽ�B
void __fastcall MainWnd_OnLButtonDown(HWND hwnd, bool fDoubleClick, int x, int y, UINT /*keyFlags*/)
{
    int i, j;
    RECT rc;
    POINT pt;
    INT nCellSize = xg_nCellSize * xg_nZoomRate / 100;

    // �_�u���N���b�N�͖����B
    if (!fDoubleClick)
        return;

    // ���{�^���������ꂽ�ʒu�����߂�B
    pt.x = x + XgGetHScrollPos();
    pt.y = y + XgGetVScrollPos();

    // �c�[���o�[���\������Ă�����A�ʒu��␳����B
    if (::IsWindowVisible(xg_hToolBar)) {
        ::GetWindowRect(xg_hToolBar, &rc);
        pt.y -= (rc.bottom - rc.top);
    }

    // ���ꂼ��̃}�X�ɂ��Ē��ׂ�B
    for (i = 0; i < xg_nRows; i++) {
        for (j = 0; j < xg_nCols; j++) {
            // �}�X�̋�`�����߂�B
            ::SetRect(&rc,
                static_cast<int>(xg_nMargin + j * nCellSize),
                static_cast<int>(xg_nMargin + i * nCellSize),
                static_cast<int>(xg_nMargin + (j + 1) * nCellSize),
                static_cast<int>(xg_nMargin + (i + 1) * nCellSize));

            // �}�X�̒����H
            if (::PtInRect(&rc, pt)) {
                // �L�����b�g���ړ����āA�C���[�W���X�V����B
                xg_caret_pos.m_i = i;
                xg_caret_pos.m_j = j;
                XgEnsureCaretVisible(hwnd);
                XgUpdateStatusBar(hwnd);

                // �}�[�N����Ă��Ȃ����H
                if (XgGetMarked(i, j) == -1) {
                    // �}�[�N����Ă��Ȃ��}�X�B�}�[�N���Z�b�g����B
                    XG_Board *pxw;

                    if (xg_bSolved && xg_bShowAnswer)
                        pxw = &xg_solution;
                    else
                        pxw = &xg_xword;

                    if (pxw->GetAt(i, j) != ZEN_BLACK)
                        XgSetMark(i, j);
                    else
                        ::MessageBeep(0xFFFFFFFF);
                } else {
                    // �}�[�N���Z�b�g����Ă���}�X�B�}�[�N����������B
                    XgDeleteMark(i, j);
                }

                // �C���[�W���X�V����B
                x = XgGetHScrollPos();
                y = XgGetVScrollPos();
                XgUpdateImage(hwnd, x, y);
                return;
            }
        }
    }
}

// �X�e�[�^�X�o�[���X�V����B
void __fastcall XgUpdateStatusBar(HWND hwnd)
{
    // �N���C�A���g�̈���擾����B
    RECT rc;
    GetClientRect(hwnd, &rc);

    // �p�[�c�̃T�C�Y�����肷��B
    INT anWidth[] = { rc.right - 200, rc.right - 100, rc.right };

    // �X�e�[�^�X�o�[���p�[�c�ɕ�����B
    SendMessageW(xg_hStatusBar, SB_SETPARTS, 3, (LPARAM)anWidth);

    // ��ԕ������ݒ�B
    std::wstring str;
    if (xg_bTateInput) {
        str = XgLoadStringDx1(IDS_VINPUT3);
    } else {
        str = XgLoadStringDx1(IDS_HINPUT3);
    }
    str += L" - ";

    switch (xg_imode) {
    case xg_im_ABC: str += XgLoadStringDx1(IDS_ABC); break;
    case xg_im_KANA: str += XgLoadStringDx1(IDS_KANA); break;
    case xg_im_KANJI: str += XgLoadStringDx1(IDS_KANJI); break;
    case xg_im_RUSSIA: str += XgLoadStringDx1(IDS_RUSSIA); break;
    case xg_im_DIGITS: str += XgLoadStringDx1(IDS_DIGITS); break;
    default:
        break;
    }

    if (xg_bCharFeed) {
        str += L" - ";
        str += XgLoadStringDx1(IDS_CHARFEED);
    }

    // ��Ԃ�\���B
    SendMessageW(xg_hStatusBar, SB_SETTEXT, 0, (LPARAM)str.c_str());

    // �L�����b�g�ʒu�B
    WCHAR szText[64];
    StringCbPrintf(szText, sizeof(szText), L"(%u, %u)", xg_caret_pos.m_j + 1, xg_caret_pos.m_i + 1);
    SendMessageW(xg_hStatusBar, SB_SETTEXT, 1, (LPARAM)szText);

    // �Ղ̃T�C�Y�B
    StringCbPrintf(szText, sizeof(szText), L"%u x %u", xg_nCols, xg_nRows);
    SendMessageW(xg_hStatusBar, SB_SETTEXT, 2, (LPARAM)szText);
}

// �T�C�Y���ύX���ꂽ�B
void __fastcall MainWnd_OnSize(HWND hwnd, UINT /*state*/, int /*cx*/, int /*cy*/)
{
    int x, y;

    // �c�[���o�[���쐬����Ă��Ȃ���΁A�������O�Ȃ̂ŁA�����B
    if (xg_hToolBar == NULL)
        return;

    // �X�e�[�^�X�o�[�̍������擾�B
    INT cyStatus = 0;
    if (s_bShowStatusBar) {
        // �X�e�[�^�X�o�[�̈ʒu�������ŏC���B
        ::SendMessageW(xg_hStatusBar, WM_SIZE, 0, 0);

        MRect rcStatus;
        ::GetWindowRect(xg_hStatusBar, &rcStatus);
        cyStatus = rcStatus.Height();

        XgUpdateStatusBar(hwnd);

        ::ShowWindow(xg_hSizeGrip, SW_HIDE);
    } else {
        ::ShowWindow(xg_hSizeGrip, SW_SHOWNOACTIVATE);
    }

    MRect rc, rcClient;

    // �N���C�A���g�̈���v�Z����B
    ::GetClientRect(hwnd, &rcClient);
    x = rcClient.left;
    y = rcClient.top;
    INT cx = rcClient.Width(), cy = rcClient.Height();

    // �c�[���o�[�̍������擾�B
    INT cyToolBar = 0;
    if (s_bShowToolBar) {
        // �c�[���o�[�̈ʒu�������ŏC���B
        ::SendMessageW(xg_hToolBar, WM_SIZE, 0, 0);

        ::GetWindowRect(xg_hToolBar, &rc);
        cyToolBar = rc.Height();
    }

    // �c�[���o�[���\������Ă�����A�ʒu��␳����B
    y += cyToolBar;
    cy -= cyToolBar;
    cy -= cyStatus;

    // �X�N���[���o�[�̃T�C�Y���擾����B
    int cyHScrollBar = ::GetSystemMetrics(SM_CYHSCROLL);
    int cxVScrollBar = ::GetSystemMetrics(SM_CXVSCROLL);

    // �����̃E�B���h�E�̈ʒu�ƃT�C�Y�������؂�ɕύX����B
    HDWP hDwp = ::BeginDeferWindowPos(3);
    if (hDwp) {
        ::DeferWindowPos(hDwp, xg_hHScrollBar, NULL,
            x, y + cy - cyHScrollBar,
            cx - cxVScrollBar, cyHScrollBar,
            SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        ::DeferWindowPos(hDwp, xg_hVScrollBar, NULL,
            cx - cxVScrollBar, y,
            cxVScrollBar, cy - cyHScrollBar,
            SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        if (::IsWindowVisible(xg_hSizeGrip)) {
            ::DeferWindowPos(hDwp, xg_hSizeGrip, NULL,
                x + cx - cxVScrollBar, y + cy - cyHScrollBar,
                cxVScrollBar, cyHScrollBar,
                SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        }
        ::EndDeferWindowPos(hDwp);
    }

    // �ĕ`�悷��B
    ::InvalidateRect(xg_hToolBar, NULL, TRUE);
    ::InvalidateRect(xg_hStatusBar, NULL, TRUE);
    ::InvalidateRect(xg_hHScrollBar, NULL, TRUE);
    ::InvalidateRect(xg_hVScrollBar, NULL, TRUE);
    ::InvalidateRect(xg_hSizeGrip, NULL, TRUE);

    // �X�N���[���ʒu���擾���A�X�N���[�������X�V����B
    x = XgGetHScrollPos();
    y = XgGetVScrollPos();
    XgUpdateImage(hwnd, x, y);

    if (!IsZoomed(hwnd) && !IsIconic(hwnd))
    {
        // �ݒ�̕ۑ��̂��߂ɁA�E�B���h�E�̒ʏ��Ԃ̂Ƃ��̕��ƍ������i�[���Ă����B
        WINDOWPLACEMENT wndpl;
        wndpl.length = sizeof(wndpl);
        ::GetWindowPlacement(hwnd, &wndpl);
        s_nMainWndCX = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
        s_nMainWndCY = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
    }
}

// �ʒu���ύX���ꂽ�B
void __fastcall MainWnd_OnMove(HWND hwnd, int /*x*/, int /*y*/)
{
    if (!IsZoomed(hwnd) && !IsIconic(hwnd)) {
        MRect rc;
        ::GetWindowRect(hwnd, &rc);
        s_nMainWndX = rc.left;
        s_nMainWndY = rc.top;
    }
}

// �ꎞ�I�ɕۑ�����F�̃f�[�^�B
COLORREF s_rgbColors[3];

// BLOCK�̃v���r���[�B
void UpdateBlockPreview(HWND hwnd)
{
    HWND hIco1 = GetDlgItem(hwnd, ico1);
    HWND hIco2 = GetDlgItem(hwnd, ico2);
    SetWindowPos(hIco1, NULL, 0, 0, 32, 32, SWP_NOMOVE | SWP_NOZORDER | SWP_HIDEWINDOW);
    SetWindowPos(hIco2, NULL, 0, 0, 32, 32, SWP_NOMOVE | SWP_NOZORDER | SWP_HIDEWINDOW);
    HBITMAP hbmOld = (HBITMAP)SendMessageW(hIco1, STM_GETIMAGE, IMAGE_BITMAP, 0);
    HENHMETAFILE hOldEMF = (HENHMETAFILE)SendMessageW(hIco2, STM_GETIMAGE, IMAGE_ENHMETAFILE, 0);

    WCHAR szPath[MAX_PATH], szName[128];
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_GetText(hCmb1, szName, ARRAYSIZE(szName));

    GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
    PathRemoveFileSpec(szPath);
    PathAppend(szPath, L"BLOCK");
    PathAppend(szPath, szName);

    if (PathFileExistsW(szPath))
    {
        if (lstrcmpiW(PathFindExtensionW(szPath), L".bmp") == 0)
        {
            HBITMAP hbm1 = LoadBitmapFromFile(szPath);
            if (hbm1)
            {
                HBITMAP hbm2 = (HBITMAP)CopyImage(hbm1, IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
                DeleteObject(hbm1);
                SendMessageW(hIco1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm2);
                SendMessageW(hIco2, STM_SETIMAGE, IMAGE_ENHMETAFILE, (LPARAM)NULL);
                ShowWindow(hIco1, SW_SHOWNOACTIVATE);
                DeleteObject(hbmOld);
                DeleteEnhMetaFile(hOldEMF);
                return;
            }
        }
        else if (lstrcmpiW(PathFindExtensionW(szPath), L".emf") == 0)
        {
            HENHMETAFILE hEMF = GetEnhMetaFile(szPath);
            if (hEMF)
            {
                SendMessageW(hIco1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
                SendMessageW(hIco2, STM_SETIMAGE, IMAGE_ENHMETAFILE, (LPARAM)hEMF);
                ShowWindow(hIco2, SW_SHOWNOACTIVATE);
                DeleteObject(hbmOld);
                DeleteEnhMetaFile(hOldEMF);
                return;
            }
        }
    }

    SendDlgItemMessageW(hwnd, ico1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)0);
    SendDlgItemMessageW(hwnd, ico2, STM_SETIMAGE, IMAGE_ENHMETAFILE, (LPARAM)0);
    DeleteObject(hbmOld);
    DeleteEnhMetaFile(hOldEMF);
}

// [�ݒ�]�_�C�A���O�̏������B
BOOL SettingsDlg_OnInitDialog(HWND hwnd)
{
    // �F���ꎞ�I�ȃf�[�^�Ƃ��ăZ�b�g����B
    s_rgbColors[0] = xg_rgbWhiteCellColor;
    s_rgbColors[1] = xg_rgbBlackCellColor;
    s_rgbColors[2] = xg_rgbMarkedCellColor;

    // ��ʂ̒����Ɋ񂹂�B
    XgCenterDialog(hwnd);

    // �t�H���g�����i�[����B
    ::SetDlgItemTextW(hwnd, edt1, xg_szCellFont);
    ::SetDlgItemTextW(hwnd, edt2, xg_szSmallFont);
    ::SetDlgItemTextW(hwnd, edt3, xg_szUIFont);

    // �c�[���o�[��\�����邩�H
    ::CheckDlgButton(hwnd, chx1,
        (s_bShowToolBar ? BST_CHECKED : BST_UNCHECKED));
    // ���g�����邩�H
    ::CheckDlgButton(hwnd, chx2,
        (xg_bAddThickFrame ? BST_CHECKED : BST_UNCHECKED));
    // ��d�}�X�ɘg�����邩�H
    ::CheckDlgButton(hwnd, chx3,
        (xg_bDrawFrameForMarkedCell ? BST_CHECKED : BST_UNCHECKED));

    // �����̑傫���B
    ::SetDlgItemInt(hwnd, edt4, xg_nCellCharPercents, FALSE);
    ::SetDlgItemInt(hwnd, edt5, xg_nSmallCharPercents, FALSE);
    // �傫���͈̔͂��w��B
    ::SendDlgItemMessage(hwnd, scr1, UDM_SETRANGE, 0, MAKELPARAM(100, 3));
    ::SendDlgItemMessage(hwnd, scr2, UDM_SETRANGE, 0, MAKELPARAM(100, 3));

    WCHAR szPath[MAX_PATH];
    std::vector<std::wstring> items;
    WIN32_FIND_DATA find;
    HANDLE hFind;

    // BLOCK\*.bmp
    GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
    PathRemoveFileSpec(szPath);
    PathAppend(szPath, L"BLOCK\\*.bmp");
    hFind = FindFirstFile(szPath, &find);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            items.push_back(find.cFileName);
        } while (FindNextFile(hFind, &find));

        FindClose(hFind);
    }

    // BLOCK\*.emf
    GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
    PathRemoveFileSpec(szPath);
    PathAppend(szPath, L"BLOCK\\*.emf");
    hFind = FindFirstFile(szPath, &find);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            items.push_back(find.cFileName);
        } while (FindNextFile(hFind, &find));

        FindClose(hFind);
    }

    // �\�[�g����B
    std::sort(items.begin(), items.end());

    // �R���{�{�b�N�X�ɍ��ڂ�ǉ�����B
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_AddString(hCmb1, XgLoadStringDx1(IDS_NONE));
    for (auto& item : items)
    {
        ComboBox_AddString(hCmb1, item.c_str());
    }

    if (xg_strBlackCellImage.empty())
    {
        // ���}�X�摜�Ȃ��B
        ComboBox_SetText(hCmb1, XgLoadStringDx1(IDS_NONE));
        ComboBox_SetCurSel(hCmb1, ComboBox_FindStringExact(hCmb1, -1, XgLoadStringDx1(IDS_NONE)));
    }
    else
    {
        // ���}�X�摜����B
        LPCWSTR psz = PathFindFileName(xg_strBlackCellImage.c_str());
        ComboBox_SetText(hCmb1, psz);
        ComboBox_SetCurSel(hCmb1, ComboBox_FindStringExact(hCmb1, -1, psz));
    }

    UpdateBlockPreview(hwnd);

    return TRUE;
}

// [�ݒ�]�_�C�A���O��[OK]�{�^���������ꂽ�B
void SettingsDlg_OnOK(HWND hwnd)
{
    // �����t�@�C���̕ۑ����[�h���擾����B
    //s_nDictSaveMode = 
    //    static_cast<int>(::SendDlgItemMessageW(hwnd, cmb1, CB_GETCURSEL, 0, 0));
    s_nDictSaveMode = 2;

    INT nValue1, nValue2;
    BOOL bTranslated;

    // �Z���̕����̑傫���B
    bTranslated = FALSE;
    nValue1 = GetDlgItemInt(hwnd, edt4, &bTranslated, FALSE);
    if (bTranslated && 0 <= nValue1 && nValue1 <= 100)
    {
        ;
    }
    else
    {
        // �G���[�B
        HWND hEdt4 = GetDlgItem(hwnd, edt4);
        Edit_SetSel(hEdt4, 0, -1);
        SetFocus(hEdt4);
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_INVALIDVALUE), NULL, MB_ICONERROR);
        return;
    }

    // �����������̑傫���B
    bTranslated = FALSE;
    nValue2 = GetDlgItemInt(hwnd, edt5, &bTranslated, FALSE);
    if (bTranslated && 0 <= nValue2 && nValue2 <= 100)
    {
        ;
    }
    else
    {
        // �G���[�B
        HWND hEdt5 = GetDlgItem(hwnd, edt5);
        Edit_SetSel(hEdt5, 0, -1);
        SetFocus(hEdt5);
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_INVALIDVALUE), NULL, MB_ICONERROR);
        return;
    }

    // �����̑傫���̐ݒ�B
    xg_nCellCharPercents = nValue1;
    xg_nSmallCharPercents = nValue2;

    // �t�H���g�����擾����B
    WCHAR szName[LF_FACESIZE];

    // �Z���t�H���g�B
    ::GetDlgItemTextW(hwnd, edt1, szName, ARRAYSIZE(szName));
    StringCbCopy(xg_szCellFont, sizeof(xg_szCellFont), szName);

    // �����������̃t�H���g�B
    ::GetDlgItemTextW(hwnd, edt2, szName, ARRAYSIZE(szName));
    StringCbCopy(xg_szSmallFont, sizeof(xg_szSmallFont), szName);

    // UI�t�H���g�B
    ::GetDlgItemTextW(hwnd, edt3, szName, ARRAYSIZE(szName));
    StringCbCopy(xg_szUIFont, sizeof(xg_szUIFont), szName);

    // ���}�X�摜�̖��O���擾�B
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_GetText(hCmb1, szName, ARRAYSIZE(szName));

    // ���}�X�摜�̏������B
    xg_strBlackCellImage.clear();
    ::DeleteObject(xg_hbmBlackCell);
    xg_hbmBlackCell = NULL;
    DeleteEnhMetaFile(xg_hBlackCellEMF);
    xg_hBlackCellEMF = NULL;

    // �������}�X�摜���w�肳��Ă����
    if (szName[0])
    {
        // �p�X�����Z�b�g�B
        WCHAR szPath[MAX_PATH];
        GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
        PathRemoveFileSpec(szPath);
        PathAppend(szPath, L"BLOCK");
        PathAppend(szPath, szName);

        if (PathFileExists(szPath))
        {
            // �t�@�C�������݂���΁A�摜��ǂݍ��ށB
            xg_strBlackCellImage = szPath;
            xg_hbmBlackCell = LoadBitmapFromFile(xg_strBlackCellImage.c_str());
            if (!xg_hbmBlackCell)
            {
                xg_hBlackCellEMF = GetEnhMetaFile(xg_strBlackCellImage.c_str());
            }
        }

        if (!xg_hbmBlackCell && !xg_hBlackCellEMF)
        {
            // �摜�������Ȃ�A�p�X���������B
            xg_strBlackCellImage.clear();
        }
    }

    // �c�[���o�[��\�����邩�H
    s_bShowToolBar = (::IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED);
    if (s_bShowToolBar)
        ::ShowWindow(xg_hToolBar, SW_SHOWNOACTIVATE);
    else
        ::ShowWindow(xg_hToolBar, SW_HIDE);

    // ���g�����邩�H
    xg_bAddThickFrame = (::IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED);

    // �F��ݒ肷��B
    xg_rgbWhiteCellColor = s_rgbColors[0];
    xg_rgbBlackCellColor = s_rgbColors[1];
    xg_rgbMarkedCellColor = s_rgbColors[2];

    // ��d�}�X�ɘg�����邩�H
    xg_bDrawFrameForMarkedCell = (::IsDlgButtonChecked(hwnd, chx3) == BST_CHECKED);

    // ���C�A�E�g�𒲐�����B
    ::PostMessageW(xg_hMainWnd, WM_SIZE, 0, 0);
    if (::IsWindow(xg_hHintsWnd)) {
        XgUpdateHintsData();
        XgOpenHintsByWindow(xg_hHintsWnd);
        ::PostMessageW(xg_hHintsWnd, WM_SIZE, 0, 0);
    }

    // �C���[�W���X�V����B
    int x = XgGetHScrollPos();
    int y = XgGetVScrollPos();
    XgUpdateImage(xg_hMainWnd, x, y);
}

// UI�t�H���g�̘_���I�u�W�F�N�g���擾����B
LOGFONTW *XgGetUIFont(void)
{
    static LOGFONTW s_lf;
    ::GetObjectW(::GetStockObject(DEFAULT_GUI_FONT), sizeof(s_lf), &s_lf);
    if (xg_szUIFont[0]) {
        WCHAR szData[LF_FACESIZE];
        StringCbCopy(szData, sizeof(szData), xg_szUIFont);
        LPWSTR pch = wcsrchr(szData, L',');
        if (pch) {
            *pch++ = 0;
            std::wstring name(szData);
            std::wstring size(pch);
            xg_str_trim(name);
            xg_str_trim(size);

            StringCbCopy(s_lf.lfFaceName, sizeof(s_lf.lfFaceName), name.data());

            HDC hdc = ::CreateCompatibleDC(NULL);
            int point_size = _wtoi(size.data());
            s_lf.lfHeight = -MulDiv(point_size, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);
            ::DeleteDC(hdc);
        } else {
            std::wstring name(szData);
            xg_str_trim(name);

            StringCbCopy(s_lf.lfFaceName, sizeof(s_lf.lfFaceName), name.data());
        }
    }
    return &s_lf;
}

// UI�t�H���g�̘_���I�u�W�F�N�g��ݒ肷��B
void SettingsDlg_SetUIFont(HWND hwnd, const LOGFONTW *plf)
{
    if (plf == NULL) {
        SetDlgItemTextW(hwnd, edt3, NULL);
        return;
    }

    HDC hdc = ::CreateCompatibleDC(NULL);
    int point_size = -MulDiv(plf->lfHeight, 72, ::GetDeviceCaps(hdc, LOGPIXELSY));
    ::DeleteDC(hdc);

    WCHAR szData[128];
    StringCbPrintf(szData, sizeof(szData), L"%s, %upt", plf->lfFaceName, point_size);
    ::SetDlgItemTextW(hwnd, edt3, szData);
}

// [�ݒ�]�_�C�A���O��[�ύX...]�{�^���������ꂽ�B
void SettingsDlg_OnChange(HWND hwnd, int i)
{
    LOGFONTW lf;
    ZeroMemory(&lf, sizeof(lf));
    lf.lfQuality = ANTIALIASED_QUALITY;

    // ���[�U�[�Ƀt�H���g����₢���킹��B
    CHOOSEFONTW cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = hwnd;
    cf.lpLogFont = &lf;
    cf.nFontType = SCREEN_FONTTYPE | SIMULATED_FONTTYPE | REGULAR_FONTTYPE;
    lf.lfCharSet = DEFAULT_CHARSET;

    switch (i) {
    case 0:
        cf.Flags = CF_NOSCRIPTSEL | CF_NOVERTFONTS | CF_SCALABLEONLY |
                   CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szCellFont);
        if (::ChooseFontW(&cf)) {
            // �擾�����t�H���g���_�C�A���O�֊i�[����B
            ::SetDlgItemTextW(hwnd, edt1, lf.lfFaceName);
        }
        break;

    case 1:
        cf.Flags = CF_NOSCRIPTSEL | CF_NOVERTFONTS | CF_SCALABLEONLY |
                   CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szSmallFont);
        if (::ChooseFontW(&cf)) {
            // �擾�����t�H���g���_�C�A���O�֊i�[����B
            ::SetDlgItemTextW(hwnd, edt2, lf.lfFaceName);
        }
        break;

    case 2:
        cf.lpLogFont = XgGetUIFont();
        cf.Flags = CF_NOSCRIPTSEL | CF_NOVERTFONTS | CF_SCREENFONTS |
                   CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;
        cf.nSizeMin = 8;
        cf.nSizeMax = 20;
        if (::ChooseFontW(&cf)) {
            // �擾�����t�H���g���_�C�A���O�֊i�[����B
            SettingsDlg_SetUIFont(hwnd, cf.lpLogFont);
        }
        break;
    }
}

// [�ݒ�]�_�C�A���O��[���Z�b�g]�{�^���������ꂽ�B
void SettingsDlg_OnReset(HWND hwnd, int i)
{
    switch (i) {
    case 0:
        ::SetDlgItemTextW(hwnd, edt1, L"");
        break;

    case 1:
        ::SetDlgItemTextW(hwnd, edt2, L"");
        break;

    case 2:
        ::SetDlgItemTextW(hwnd, edt3, L"");
        break;
    }
}

// [�ݒ�]�_�C�A���O�̃I�[�i�[�h���[�B
void SettingDlg_OnDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT pdis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
    HDC hdc = pdis->hDC;

    if (pdis->CtlType != ODT_BUTTON) {
        return;
    }

    BOOL bSelected = !!(pdis->itemState & ODS_SELECTED);
    BOOL bFocus = !!(pdis->itemState & ODS_FOCUS);
    RECT& rcItem = pdis->rcItem;

    ::DrawFrameControl(hdc, &rcItem, DFC_BUTTON,
        DFCS_BUTTONPUSH |
        (bSelected ? DFCS_PUSHED : 0)
    );

    HBRUSH hbr = NULL;
    switch (pdis->CtlID) {
    case psh7:
        hbr = ::CreateSolidBrush(s_rgbColors[0]);
        break;

    case psh8:
        hbr = ::CreateSolidBrush(s_rgbColors[1]);
        break;

    case psh9:
        hbr = ::CreateSolidBrush(s_rgbColors[2]);
        break;

    default:
        return;
    }

    ::InflateRect(&rcItem, -4, -4);
    ::FillRect(hdc, &rcItem, hbr);
    ::DeleteObject(hbr);

    if (bFocus) {
        ::InflateRect(&rcItem, 2, 2);
        ::DrawFocusRect(hdc, &rcItem);
    }
}

COLORREF s_rgbColorTable[] = {
    RGB(0, 0, 0),
    RGB(0x33, 0x33, 0x33),
    RGB(0x66, 0x66, 0x66),
    RGB(0x99, 0x99, 0x99),
    RGB(0xCC, 0xCC, 0xCC),
    RGB(0xFF, 0xFF, 0xFF),
    RGB(0xFF, 0xFF, 0xCC),
    RGB(0xFF, 0xCC, 0xFF),
    RGB(0xFF, 0xCC, 0xCC),
    RGB(0xCC, 0xFF, 0xFF),
    RGB(0xCC, 0xFF, 0xCC),
    RGB(0xCC, 0xCC, 0xFF),
    RGB(0xCC, 0xCC, 0xCC),
    RGB(0, 0, 0xCC),
    RGB(0, 0xCC, 0),
    RGB(0xCC, 0, 0),
};

// �F���w�肷��B
void SettingsDlg_OnSetColor(HWND hwnd, int nIndex)
{
    COLORREF clr;
    switch (nIndex) {
    case 0:
        clr = s_rgbColors[0];
        break;

    case 1:
        clr = s_rgbColors[1];
        break;

    case 2:
        clr = s_rgbColors[2];
        break;

    default:
        return;
    }

    CHOOSECOLORW cc;
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.rgbResult = clr;
    cc.lpCustColors = s_rgbColorTable;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColorW(&cc)) {
        switch (nIndex) {
        case 0:
            s_rgbColors[0] = cc.rgbResult;
            ::InvalidateRect(::GetDlgItem(hwnd, psh7), NULL, TRUE);
            break;

        case 1:
            s_rgbColors[1] = cc.rgbResult;
            ::InvalidateRect(::GetDlgItem(hwnd, psh8), NULL, TRUE);
            break;

        case 2:
            s_rgbColors[2] = cc.rgbResult;
            ::InvalidateRect(::GetDlgItem(hwnd, psh9), NULL, TRUE);
            break;

        default:
            return;
        }

    }
}

// [�ݒ�]�_�C�A���O�̃_�C�A���O �v���V�[�W���[�B
INT_PTR CALLBACK
XgSettingsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        return SettingsDlg_OnInitDialog(hwnd);

    case WM_DRAWITEM:
        SettingDlg_OnDrawItem(hwnd, wParam, lParam);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            SettingsDlg_OnOK(hwnd);
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            ::EndDialog(hwnd, IDCANCEL);
            break;

        case psh1:
            SettingsDlg_OnChange(hwnd, 0);
            break;

        case psh2:
            SettingsDlg_OnChange(hwnd, 1);
            break;

        case psh3:
            SettingsDlg_OnReset(hwnd, 0);
            break;

        case psh4:
            SettingsDlg_OnReset(hwnd, 1);
            break;

        case psh5:
            SettingsDlg_OnChange(hwnd, 2);
            break;

        case psh6:
            SettingsDlg_OnReset(hwnd, 2);
            break;

        case psh7:
            SettingsDlg_OnSetColor(hwnd, 0);
            break;

        case psh8:
            SettingsDlg_OnSetColor(hwnd, 1);
            break;

        case psh9:
            SettingsDlg_OnSetColor(hwnd, 2);
            break;

        case psh10:
            SetDlgItemInt(hwnd, edt4, DEF_CELL_CHAR_SIZE, FALSE);
            break;

        case psh11:
            SetDlgItemInt(hwnd, edt5, DEF_SMALL_CHAR_SIZE, FALSE);
            break;

        case cmb1:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                UpdateBlockPreview(hwnd);
            }
        }
        break;
    }
    return 0;
}

// �ݒ�B
void MainWnd_OnSettings(HWND hwnd)
{
    XgDestroyCandsWnd();
    ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_CONFIG), hwnd, XgSettingsDlgProc);
}

// �e�[�}���ύX���ꂽ�B
void XgUpdateTheme(HWND hwnd)
{
    std::unordered_set<std::wstring> priority, forbidden;
    XgParseTheme(priority, forbidden, xg_strDefaultTheme);
    xg_bThemeModified = (priority != xg_priority_tags || forbidden != xg_forbidden_tags);

    HMENU hMenu = ::GetMenu(hwnd);
    INT nCount = ::GetMenuItemCount(hMenu);
    assert(nCount > 0);
    WCHAR szText[32];
    MENUITEMINFOW info = { sizeof(info) };
    info.fMask = MIIM_TYPE;
    info.fType = MFT_STRING;
    for (INT i = 0; i < nCount; ++i) {
        szText[0] = 0;
        ::GetMenuStringW(hMenu, i, szText, ARRAYSIZE(szText), MF_BYPOSITION);
        if (wcsstr(szText, XgLoadStringDx1(IDS_DICT)) != NULL) {
            if (xg_bThemeModified) {
                StringCbCopyW(szText, sizeof(szText), XgLoadStringDx1(IDS_MODIFIEDDICT));
            } else {
                StringCbCopyW(szText, sizeof(szText), XgLoadStringDx1(IDS_DEFAULTDICT));
            }
            info.dwTypeData = szText;
            SetMenuItemInfoW(hMenu, i, TRUE, &info);
            break;
        }
    }
    // ���j���[�o�[���ĕ`��B
    ::DrawMenuBar(hwnd);
}

// ���[�����ύX���ꂽ�B
void XgUpdateRules(HWND hwnd)
{
    HMENU hMenu = ::GetMenu(hwnd);
    INT nCount = ::GetMenuItemCount(hMenu);
    WCHAR szText[32];
    MENUITEMINFOW info = { sizeof(info) };
    info.fMask = MIIM_TYPE;
    info.fType = MFT_STRING;
    for (INT i = nCount - 1; i >= 0; --i) {
        szText[0] = 0;
        ::GetMenuStringW(hMenu, i, szText, ARRAYSIZE(szText), MF_BYPOSITION);
        if (wcsstr(szText, XgLoadStringDx1(IDS_RULES)) != NULL) {
            if (XgIsUserJapanese()) {
                if (xg_nRules == DEFAULT_RULES_JAPANESE) {
                    StringCbCopyW(szText, sizeof(szText), XgLoadStringDx1(IDS_STANDARDRULES));
                } else {
                    StringCbCopyW(szText, sizeof(szText), XgLoadStringDx1(IDS_MODIFIEDRULES));
                }
            } else {
                if (xg_nRules == DEFAULT_RULES_ENGLISH) {
                    StringCbCopyW(szText, sizeof(szText), XgLoadStringDx1(IDS_STANDARDRULES));
                } else {
                    StringCbCopyW(szText, sizeof(szText), XgLoadStringDx1(IDS_MODIFIEDRULES));
                }
            }
            info.dwTypeData = szText;
            SetMenuItemInfoW(hMenu, i, TRUE, &info);
            break;
        }
    }
    // ���j���[�o�[���ĕ`��B
    ::DrawMenuBar(hwnd);
}

// �ݒ����������B
void MainWnd_OnEraseSettings(HWND hwnd)
{
    // ���E�B���h�E��j������B
    XgDestroyCandsWnd();
    // �q���g�E�B���h�E��j������B
    XgDestroyHintsWnd();

    // ��������̂��m�F�B
    if (XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_QUERYERASESETTINGS), XgLoadStringDx2(IDS_APPNAME),
                            MB_ICONWARNING | MB_YESNO) != IDYES)
    {
        return;
    }

    // �ݒ����������B
    bool bSuccess = XgEraseSettings();

    // ����������B
    XgLoadSettings();
    XgUpdateRules(hwnd);
    // �����t�@�C���̖��O��ǂݍ��ށB
    XgLoadDictsAll();

    xg_bSolved = false;
    xg_bHintsAdded = false;
    xg_bShowAnswer = false;
    xg_xword.clear();
    xg_solution.clear();
    xg_caret_pos.clear();
    xg_strHeader.clear();
    xg_strNotes.clear();
    xg_strFileName.clear();
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();
    xg_vMarks.clear();
    xg_vMarkedCands.clear();
    XgMarkUpdate();

    // �c�[���o�[�̕\����؂�ւ���B
    if (s_bShowToolBar)
        ::ShowWindow(xg_hToolBar, SW_SHOWNOACTIVATE);
    else
        ::ShowWindow(xg_hToolBar, SW_HIDE);

    // �c�[���o�[�̕\����؂�ւ���B
    if (s_bShowStatusBar)
        ::ShowWindow(xg_hStatusBar, SW_SHOWNOACTIVATE);
    else
        ::ShowWindow(xg_hStatusBar, SW_HIDE);

    // �C���[�W���X�V����B
    XgUpdateImage(hwnd, 0, 0);
    // �e�[�}������������B
    XgResetTheme(hwnd);
    XgUpdateTheme(hwnd);

    if (bSuccess) {
        // ���b�Z�[�W��\������B
        XgCenterMessageBoxW(hwnd,
            XgLoadStringDx1(IDS_ERASEDSETTINGS), XgLoadStringDx2(IDS_APPNAME),
            MB_ICONINFORMATION);
    } else {
        // ���b�Z�[�W��\������B
        XgCenterMessageBoxW(hwnd,
            XgLoadStringDx1(IDS_FAILERASESETTINGS), XgLoadStringDx2(IDS_APPNAME),
            MB_ICONINFORMATION);
    }
}

// ������ǂݍ��ށB
extern "C"
INT_PTR CALLBACK
XgLoadDictDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int i;
    WCHAR szFile[MAX_PATH], szTarget[MAX_PATH];
    std::wstring strFile;
    HWND hCmb1;
    COMBOBOXEXITEMW item;
    OPENFILENAMEW ofn;
    HDROP hDrop;

    switch (uMsg) {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �����t�@�C�������ׂēǂݍ��ށB
        XgLoadDictsAll();
        // �����t�@�C���̃p�X���̃x�N�^�[���R���{�{�b�N�X�ɐݒ肷��B
        hCmb1 = GetDlgItem(hwnd, cmb1);
        for (const auto& dict_file : xg_dict_files) {
            item.mask = CBEIF_TEXT;
            item.iItem = -1;
            StringCbCopy(szFile, sizeof(szFile), dict_file.data());
            item.pszText = szFile;
            item.cchTextMax = -1;
            ::SendMessageW(hCmb1, CBEM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item));
        }
        ComboBox_SetText(hCmb1, xg_dict_name.c_str());

        // �h���b�O���h���b�v���󂯕t����B
        ::DragAcceptFiles(hwnd, TRUE);
        return TRUE;

    case WM_DROPFILES:
        // �h���b�v���ꂽ�t�@�C���̃p�X�����擾����B
        hDrop = reinterpret_cast<HDROP>(wParam);
        ::DragQueryFileW(hDrop, 0, szFile, MAX_PATH);
        ::DragFinish(hDrop);

        // �V���[�g�J�b�g�������ꍇ�́A�^�[�Q�b�g�̃p�X���擾����B
        if (::lstrcmpiW(PathFindExtensionW(szFile), s_szShellLinkDotExt) == 0) {
            if (!XgGetPathOfShortcutW(szFile, szTarget)) {
                ::MessageBeep(0xFFFFFFFF);
                break;
            }
            StringCbCopy(szFile, sizeof(szFile), szTarget);
        }

        // �������ڂ����łɂ���΁A�폜����B
        i = static_cast<int>(::SendDlgItemMessageW(hwnd, cmb1, CB_FINDSTRINGEXACT, 0,
                                                 reinterpret_cast<LPARAM>(szFile)));
        if (i != CB_ERR) {
            ::SendDlgItemMessageW(hwnd, cmb1, CB_DELETESTRING, i, 0);
        }
        // �R���{�{�b�N�X�̍ŏ��ɑ}������B
        item.mask = CBEIF_TEXT;
        item.iItem = 0;
        item.pszText = szFile;
        item.cchTextMax = -1;
        ::SendDlgItemMessageW(hwnd, cmb1, CBEM_INSERTITEMW, 0,
                            reinterpret_cast<LPARAM>(&item));
        // �R���{�{�b�N�X�̍ŏ��̍��ڂ�I������B
        ::SendDlgItemMessageW(hwnd, cmb1, CB_SETCURSEL, 0, 0);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            // �����t�@�C���̃p�X�����擾����B
            ::GetDlgItemTextW(hwnd, cmb1, szFile, ARRAYSIZE(szFile));
            strFile = szFile;
            xg_str_trim(strFile);
            // �������ǂݍ��߂邩�H
            if (XgLoadDictFile(strFile.data())) {
                // �ǂݍ��߂��B
                XgSetDict(strFile);

                // �_�C�A���O�����B
                ::EndDialog(hwnd, IDOK);
            } else {
                // �ǂݍ��߂Ȃ������̂ŃG���[��\������B
                ::SendDlgItemMessageW(hwnd, cmb1, CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTLOAD), nullptr, MB_ICONERROR);
                ::SetFocus(::GetDlgItem(hwnd, cmb1));
            }
            break;

        case IDCANCEL:
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDCANCEL);
            break;

        case psh1:  // [�Q��]�{�^���B
            // ���[�U�[�Ɏ����t�@�C���̏ꏊ��₢���킹��B
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = XgMakeFilterString(XgLoadStringDx2(IDS_DICTFILTER));
            szFile[0] = 0;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = XgLoadStringDx1(IDS_OPENDICTDATA);
            ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST |
                OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
            ofn.lpstrDefExt = L"dic";
            if (::GetOpenFileNameW(&ofn))
            {
                // �R���{�{�b�N�X�Ƀe�L�X�g��ݒ�B
                ::SetDlgItemTextW(hwnd, cmb1, szFile);
            }
            break;
        }
    }
    return 0;
}

// [�w�b�_�[�Ɣ��l��]�_�C�A���O�B
extern "C"
INT_PTR CALLBACK
XgNotesDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WCHAR sz[512];
    std::wstring str;
    LPWSTR psz;

    switch (uMsg) {
    case WM_INITDIALOG:
        // �_�C�A���O�𒆉��ֈړ�����B
        XgCenterDialog(hwnd);
        // �w�b�_�[��ݒ肷��B
        ::SetDlgItemTextW(hwnd, edt1, xg_strHeader.data());
        // ���l����ݒ肷��B
        str = xg_strNotes;
        xg_str_trim(str);
        psz = XgLoadStringDx1(IDS_BELOWISNOTES);
        if (str.find(psz) == 0) {
            str = str.substr(::lstrlenW(psz));
        }
        ::SetDlgItemTextW(hwnd, edt2, str.data());
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            // �w�b�_�[���擾����B
            ::GetDlgItemTextW(hwnd, edt1, sz, static_cast<int>(ARRAYSIZE(sz)));
            str = sz;
            xg_str_trim(str);
            xg_strHeader = str;

            // ���l�����擾����B
            ::GetDlgItemTextW(hwnd, edt2, sz, static_cast<int>(ARRAYSIZE(sz)));
            str = sz;
            xg_str_trim(str);
            xg_strNotes = str;

            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            // �_�C�A���O�����B
            ::EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
    return 0;
}

// �c�Ɖ������ւ���B
void __fastcall MainWnd_OnFlipVH(HWND hwnd)
{
    xg_xword.SwapXandY();
    if (xg_bSolved) {
        xg_solution.SwapXandY();
    }
    std::swap(xg_nRows, xg_nCols);
    if (xg_bSolved) {
        auto old_dict_1 = xg_dict_1;
        auto old_dict_2 = xg_dict_2;
        xg_dict_1 = XgCreateMiniDict();
        xg_dict_2.clear();
        xg_solution.DoNumbering();
        xg_solution.GetHintsStr(xg_strHints, 2, true);
        if (!XgParseHintsStr(hwnd, xg_strHints)) {
            xg_strHints.clear();
        }
        std::swap(xg_dict_1, old_dict_1);
        std::swap(xg_dict_2, old_dict_2);
    }
    std::swap(xg_caret_pos.m_i, xg_caret_pos.m_j);
    for (auto& mark : xg_vMarks) {
        std::swap(mark.m_i, mark.m_j);
    }
    // �C���[�W���X�V����B
    XgUpdateImage(hwnd, 0, 0);
}

std::wstring URL_encode(const std::wstring& url)
{
    std::string str;

    size_t len = url.size() * 4;
    str.resize(len);
    if (len > 0)
        WideCharToMultiByte(CP_UTF8, 0, url.c_str(), -1, &str[0], (INT)len, NULL, NULL);

    len = strlen(str.c_str());
    str.resize(len);

    std::wstring ret;
    WCHAR buf[4];
    static const WCHAR s_hex[] = L"0123456789ABCDEF";
    for (size_t i = 0; i < str.size(); ++i)
    {
        using namespace std;
        if (str[i] == ' ')
        {
            ret += L'+';
        }
        else if (isalnum(str[i]))
        {
            ret += (char)str[i];
        }
        else
        {
            switch (str[i])
            {
            case L'.':
            case L'-':
            case L'_':
            case L'*':
                ret += (char)str[i];
                break;
            default:
                buf[0] = L'%';
                buf[1] = s_hex[(str[i] >> 4) & 0xF];
                buf[2] = s_hex[str[i] & 0xF];
                buf[3] = 0;
                ret += buf;
                break;
            }
        }
    }

    return ret;
}

// �E�F�u�����B
void DoWebSearch(HWND hwnd, LPCWSTR str)
{
    std::wstring query = XgLoadStringDx1(IDS_GOOGLESEARCH);
    std::wstring raw = str;
    switch (xg_imode)
    {
    case xg_im_ABC:
        raw += L" ";
        raw += XgLoadStringDx2(IDS_ABC);
        break;
    case xg_im_KANA:
    case xg_im_KANJI:
        raw += L" ";
        raw += XgLoadStringDx2(IDS_DICTIONARY);
        break;
    case xg_im_RUSSIA:
    case xg_im_DIGITS:
        break;
    default:
        break;
    }
    std::wstring encoded = URL_encode(raw.c_str());
    query += encoded;

    ::ShellExecuteW(hwnd, NULL, query.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void __fastcall MainWnd_OnCopyPattern(HWND hwnd, BOOL bTate)
{
    XG_Board *pxword;
    if (xg_bSolved && xg_bShowAnswer) {
        pxword = &xg_solution;
    } else {
        pxword = &xg_xword;
    }

    // �p�^�[�����擾����B
    std::wstring pattern;
    if (bTate) {
        pattern = pxword->GetPatternV(xg_caret_pos);
    } else {
        pattern = pxword->GetPatternH(xg_caret_pos);
    }

    XgSetClipboardUnicodeText(hwnd, pattern);
}

void __fastcall MainWnd_OnCopyPatternHorz(HWND hwnd)
{
    MainWnd_OnCopyPattern(hwnd, FALSE);
}

void __fastcall MainWnd_OnCopyPatternVert(HWND hwnd)
{
    MainWnd_OnCopyPattern(hwnd, TRUE);
}

// �I�����C�������������B
void __fastcall XgOnlineDict(HWND hwnd, BOOL bTate)
{
    XG_Board *pxword;
    if (xg_bSolved && xg_bShowAnswer) {
        pxword = &xg_solution;
    } else {
        pxword = &xg_xword;
    }

    // �p�^�[�����擾����B
    std::wstring pattern;
    if (bTate) {
        pattern = pxword->GetPatternV(xg_caret_pos);
    } else {
        pattern = pxword->GetPatternH(xg_caret_pos);
    }

    // �󔒂��܂�ł�����A�����B
    if (pattern.find(ZEN_SPACE) != std::wstring::npos) {
        return;
    }

    DoWebSearch(hwnd, pattern.c_str());
}

bool __fastcall MainWnd_OnCommand2(HWND hwnd, INT id)
{
    bool bOK = false;

    bool bOldFeed = xg_bCharFeed;
    xg_bCharFeed = false;
    switch (id)
    {
    // kana
    case 10000: MainWnd_OnImeChar(hwnd, ZEN_A, 0); bOK = true; break;
    case 10001: MainWnd_OnImeChar(hwnd, ZEN_I, 0); bOK = true; break;
    case 10002: MainWnd_OnImeChar(hwnd, ZEN_U, 0); bOK = true; break;
    case 10003: MainWnd_OnImeChar(hwnd, ZEN_E, 0); bOK = true; break;
    case 10004: MainWnd_OnImeChar(hwnd, ZEN_O, 0); bOK = true; break;
    case 10010: MainWnd_OnImeChar(hwnd, ZEN_KA, 0); bOK = true; break;
    case 10011: MainWnd_OnImeChar(hwnd, ZEN_KI, 0); bOK = true; break;
    case 10012: MainWnd_OnImeChar(hwnd, ZEN_KU, 0); bOK = true; break;
    case 10013: MainWnd_OnImeChar(hwnd, ZEN_KE, 0); bOK = true; break;
    case 10014: MainWnd_OnImeChar(hwnd, ZEN_KO, 0); bOK = true; break;
    case 10020: MainWnd_OnImeChar(hwnd, ZEN_SA, 0); bOK = true; break;
    case 10021: MainWnd_OnImeChar(hwnd, ZEN_SI, 0); bOK = true; break;
    case 10022: MainWnd_OnImeChar(hwnd, ZEN_SU, 0); bOK = true; break;
    case 10023: MainWnd_OnImeChar(hwnd, ZEN_SE, 0); bOK = true; break;
    case 10024: MainWnd_OnImeChar(hwnd, ZEN_SO, 0); bOK = true; break;
    case 10030: MainWnd_OnImeChar(hwnd, ZEN_TA, 0); bOK = true; break;
    case 10031: MainWnd_OnImeChar(hwnd, ZEN_CHI, 0); bOK = true; break;
    case 10032: MainWnd_OnImeChar(hwnd, ZEN_TSU, 0); bOK = true; break;
    case 10033: MainWnd_OnImeChar(hwnd, ZEN_TE, 0); bOK = true; break;
    case 10034: MainWnd_OnImeChar(hwnd, ZEN_TO, 0); bOK = true; break;
    case 10040: MainWnd_OnImeChar(hwnd, ZEN_NA, 0); bOK = true; break;
    case 10041: MainWnd_OnImeChar(hwnd, ZEN_NI, 0); bOK = true; break;
    case 10042: MainWnd_OnImeChar(hwnd, ZEN_NU, 0); bOK = true; break;
    case 10043: MainWnd_OnImeChar(hwnd, ZEN_NE, 0); bOK = true; break;
    case 10044: MainWnd_OnImeChar(hwnd, ZEN_NO, 0); bOK = true; break;
    case 10050: MainWnd_OnImeChar(hwnd, ZEN_HA, 0); bOK = true; break;
    case 10051: MainWnd_OnImeChar(hwnd, ZEN_HI, 0); bOK = true; break;
    case 10052: MainWnd_OnImeChar(hwnd, ZEN_FU, 0); bOK = true; break;
    case 10053: MainWnd_OnImeChar(hwnd, ZEN_HE, 0); bOK = true; break;
    case 10054: MainWnd_OnImeChar(hwnd, ZEN_HO, 0); bOK = true; break;
    case 10060: MainWnd_OnImeChar(hwnd, ZEN_MA, 0); bOK = true; break;
    case 10061: MainWnd_OnImeChar(hwnd, ZEN_MI, 0); bOK = true; break;
    case 10062: MainWnd_OnImeChar(hwnd, ZEN_MU, 0); bOK = true; break;
    case 10063: MainWnd_OnImeChar(hwnd, ZEN_ME, 0); bOK = true; break;
    case 10064: MainWnd_OnImeChar(hwnd, ZEN_MO, 0); bOK = true; break;
    case 10070: MainWnd_OnImeChar(hwnd, ZEN_YA, 0); bOK = true; break;
    case 10071: MainWnd_OnImeChar(hwnd, ZEN_YU, 0); bOK = true; break;
    case 10072: MainWnd_OnImeChar(hwnd, ZEN_YO, 0); bOK = true; break;
    case 10080: MainWnd_OnImeChar(hwnd, ZEN_RA, 0); bOK = true; break;
    case 10081: MainWnd_OnImeChar(hwnd, ZEN_RI, 0); bOK = true; break;
    case 10082: MainWnd_OnImeChar(hwnd, ZEN_RU, 0); bOK = true; break;
    case 10083: MainWnd_OnImeChar(hwnd, ZEN_RE, 0); bOK = true; break;
    case 10084: MainWnd_OnImeChar(hwnd, ZEN_RO, 0); bOK = true; break;
    case 10090: MainWnd_OnImeChar(hwnd, ZEN_WA, 0); bOK = true; break;
    case 10091: MainWnd_OnImeChar(hwnd, ZEN_NN, 0); bOK = true; break;
    case 10092: MainWnd_OnImeChar(hwnd, ZEN_PROLONG, 0); bOK = true; break;
    case 10100: MainWnd_OnImeChar(hwnd, ZEN_GA, 0); bOK = true; break;
    case 10101: MainWnd_OnImeChar(hwnd, ZEN_GI, 0); bOK = true; break;
    case 10102: MainWnd_OnImeChar(hwnd, ZEN_GU, 0); bOK = true; break;
    case 10103: MainWnd_OnImeChar(hwnd, ZEN_GE, 0); bOK = true; break;
    case 10104: MainWnd_OnImeChar(hwnd, ZEN_GO, 0); bOK = true; break;
    case 10110: MainWnd_OnImeChar(hwnd, ZEN_ZA, 0); bOK = true; break;
    case 10111: MainWnd_OnImeChar(hwnd, ZEN_JI, 0); bOK = true; break;
    case 10112: MainWnd_OnImeChar(hwnd, ZEN_ZU, 0); bOK = true; break;
    case 10113: MainWnd_OnImeChar(hwnd, ZEN_ZE, 0); bOK = true; break;
    case 10114: MainWnd_OnImeChar(hwnd, ZEN_ZO, 0); bOK = true; break;
    case 10120: MainWnd_OnImeChar(hwnd, ZEN_DA, 0); bOK = true; break;
    case 10121: MainWnd_OnImeChar(hwnd, ZEN_DI, 0); bOK = true; break;
    case 10122: MainWnd_OnImeChar(hwnd, ZEN_DU, 0); bOK = true; break;
    case 10123: MainWnd_OnImeChar(hwnd, ZEN_DE, 0); bOK = true; break;
    case 10124: MainWnd_OnImeChar(hwnd, ZEN_DO, 0); bOK = true; break;
    case 10130: MainWnd_OnImeChar(hwnd, ZEN_BA, 0); bOK = true; break;
    case 10131: MainWnd_OnImeChar(hwnd, ZEN_BI, 0); bOK = true; break;
    case 10132: MainWnd_OnImeChar(hwnd, ZEN_BU, 0); bOK = true; break;
    case 10133: MainWnd_OnImeChar(hwnd, ZEN_BE, 0); bOK = true; break;
    case 10134: MainWnd_OnImeChar(hwnd, ZEN_BO, 0); bOK = true; break;
    case 10140: MainWnd_OnImeChar(hwnd, ZEN_PA, 0); bOK = true; break;
    case 10141: MainWnd_OnImeChar(hwnd, ZEN_PI, 0); bOK = true; break;
    case 10142: MainWnd_OnImeChar(hwnd, ZEN_PU, 0); bOK = true; break;
    case 10143: MainWnd_OnImeChar(hwnd, ZEN_PE, 0); bOK = true; break;
    case 10144: MainWnd_OnImeChar(hwnd, ZEN_PO, 0); bOK = true; break;
    case 10150: MainWnd_OnImeChar(hwnd, ZEN_PROLONG, 0); bOK = true; break;
    // ABC
    case 20000: MainWnd_OnChar(hwnd, L'A', 1); bOK = true; break;
    case 20001: MainWnd_OnChar(hwnd, L'B', 1); bOK = true; break;
    case 20002: MainWnd_OnChar(hwnd, L'C', 1); bOK = true; break;
    case 20003: MainWnd_OnChar(hwnd, L'D', 1); bOK = true; break;
    case 20004: MainWnd_OnChar(hwnd, L'E', 1); bOK = true; break;
    case 20005: MainWnd_OnChar(hwnd, L'F', 1); bOK = true; break;
    case 20006: MainWnd_OnChar(hwnd, L'G', 1); bOK = true; break;
    case 20007: MainWnd_OnChar(hwnd, L'H', 1); bOK = true; break;
    case 20008: MainWnd_OnChar(hwnd, L'I', 1); bOK = true; break;
    case 20009: MainWnd_OnChar(hwnd, L'J', 1); bOK = true; break;
    case 20010: MainWnd_OnChar(hwnd, L'K', 1); bOK = true; break;
    case 20011: MainWnd_OnChar(hwnd, L'L', 1); bOK = true; break;
    case 20012: MainWnd_OnChar(hwnd, L'M', 1); bOK = true; break;
    case 20013: MainWnd_OnChar(hwnd, L'N', 1); bOK = true; break;
    case 20014: MainWnd_OnChar(hwnd, L'O', 1); bOK = true; break;
    case 20015: MainWnd_OnChar(hwnd, L'P', 1); bOK = true; break;
    case 20016: MainWnd_OnChar(hwnd, L'Q', 1); bOK = true; break;
    case 20017: MainWnd_OnChar(hwnd, L'R', 1); bOK = true; break;
    case 20018: MainWnd_OnChar(hwnd, L'S', 1); bOK = true; break;
    case 20019: MainWnd_OnChar(hwnd, L'T', 1); bOK = true; break;
    case 20020: MainWnd_OnChar(hwnd, L'U', 1); bOK = true; break;
    case 20021: MainWnd_OnChar(hwnd, L'V', 1); bOK = true; break;
    case 20022: MainWnd_OnChar(hwnd, L'W', 1); bOK = true; break;
    case 20023: MainWnd_OnChar(hwnd, L'X', 1); bOK = true; break;
    case 20024: MainWnd_OnChar(hwnd, L'Y', 1); bOK = true; break;
    case 20025: MainWnd_OnChar(hwnd, L'Z', 1); bOK = true; break;
    // Russian
    case 30000: MainWnd_OnImeChar(hwnd, 0x0410, 0); bOK = true; break;
    case 30001: MainWnd_OnImeChar(hwnd, 0x0411, 0); bOK = true; break;
    case 30002: MainWnd_OnImeChar(hwnd, 0x0412, 0); bOK = true; break;
    case 30003: MainWnd_OnImeChar(hwnd, 0x0413, 0); bOK = true; break;
    case 30004: MainWnd_OnImeChar(hwnd, 0x0414, 0); bOK = true; break;
    case 30005: MainWnd_OnImeChar(hwnd, 0x0415, 0); bOK = true; break;
    case 30006: MainWnd_OnImeChar(hwnd, 0x0401, 0); bOK = true; break;
    case 30007: MainWnd_OnImeChar(hwnd, 0x0416, 0); bOK = true; break;
    case 30008: MainWnd_OnImeChar(hwnd, 0x0417, 0); bOK = true; break;
    case 30009: MainWnd_OnImeChar(hwnd, 0x0418, 0); bOK = true; break;
    case 30010: MainWnd_OnImeChar(hwnd, 0x0419, 0); bOK = true; break;
    case 30011: MainWnd_OnImeChar(hwnd, 0x041A, 0); bOK = true; break;
    case 30012: MainWnd_OnImeChar(hwnd, 0x041B, 0); bOK = true; break;
    case 30013: MainWnd_OnImeChar(hwnd, 0x041C, 0); bOK = true; break;
    case 30014: MainWnd_OnImeChar(hwnd, 0x041D, 0); bOK = true; break;
    case 30015: MainWnd_OnImeChar(hwnd, 0x041E, 0); bOK = true; break;
    case 30016: MainWnd_OnImeChar(hwnd, 0x041F, 0); bOK = true; break;
    case 30017: MainWnd_OnImeChar(hwnd, 0x0420, 0); bOK = true; break;
    case 30018: MainWnd_OnImeChar(hwnd, 0x0421, 0); bOK = true; break;
    case 30019: MainWnd_OnImeChar(hwnd, 0x0422, 0); bOK = true; break;
    case 30020: MainWnd_OnImeChar(hwnd, 0x0423, 0); bOK = true; break;
    case 30021: MainWnd_OnImeChar(hwnd, 0x0424, 0); bOK = true; break;
    case 30022: MainWnd_OnImeChar(hwnd, 0x0425, 0); bOK = true; break;
    case 30023: MainWnd_OnImeChar(hwnd, 0x0426, 0); bOK = true; break;
    case 30024: MainWnd_OnImeChar(hwnd, 0x0427, 0); bOK = true; break;
    case 30025: MainWnd_OnImeChar(hwnd, 0x0428, 0); bOK = true; break;
    case 30026: MainWnd_OnImeChar(hwnd, 0x0429, 0); bOK = true; break;
    case 30027: MainWnd_OnImeChar(hwnd, 0x042A, 0); bOK = true; break;
    case 30028: MainWnd_OnImeChar(hwnd, 0x042B, 0); bOK = true; break;
    case 30029: MainWnd_OnImeChar(hwnd, 0x042C, 0); bOK = true; break;
    case 30030: MainWnd_OnImeChar(hwnd, 0x042D, 0); bOK = true; break;
    case 30031: MainWnd_OnImeChar(hwnd, 0x042E, 0); bOK = true; break;
    case 30032: MainWnd_OnImeChar(hwnd, 0x042F, 0); bOK = true; break;
    // Digits
    case 40000: MainWnd_OnImeChar(hwnd, L'0', 0); bOK = true; break;
    case 40001: MainWnd_OnImeChar(hwnd, L'1', 0); bOK = true; break;
    case 40002: MainWnd_OnImeChar(hwnd, L'2', 0); bOK = true; break;
    case 40003: MainWnd_OnImeChar(hwnd, L'3', 0); bOK = true; break;
    case 40004: MainWnd_OnImeChar(hwnd, L'4', 0); bOK = true; break;
    case 40005: MainWnd_OnImeChar(hwnd, L'5', 0); bOK = true; break;
    case 40006: MainWnd_OnImeChar(hwnd, L'6', 0); bOK = true; break;
    case 40007: MainWnd_OnImeChar(hwnd, L'7', 0); bOK = true; break;
    case 40008: MainWnd_OnImeChar(hwnd, L'8', 0); bOK = true; break;
    case 40009: MainWnd_OnImeChar(hwnd, L'9', 0); bOK = true; break;
    default:
        break;
    }
    xg_bCharFeed = bOldFeed;

    return bOK;
}

// ���}�X�p�^�[���̍\���́B
struct PATDATA
{
    int num_columns;
    int num_rows;
    std::wstring data;
};

// ���}�X�p�^�[���̃f�[�^�B
static std::vector<PATDATA> s_patterns;
static LAYOUT_DATA *s_pLayout = NULL;

// �p�^�[���̃e�L�X�g�f�[�^�������₷���悤�A���H����B
static void
XgConvertPatternData(std::vector<WCHAR>& data, std::wstring text, INT cx, INT cy)
{
    xg_str_replace_all(text, L"\r\n", L"\n");
    xg_str_replace_all(text, L"\u2501", L"");
    xg_str_replace_all(text, L"\u250F\u2513\n", L"");
    xg_str_replace_all(text, L"\u2517\u251B\n", L"");
    data.resize(cx * cy);
    int x = 0, y = 0;
    for (auto& ch : text)
    {
        switch (ch)
        {
        case ZEN_BLACK:
            data[x + cx * y] = ZEN_BLACK;
            ++x;
            break;
        case ZEN_SPACE:
            data[x + cx * y] = ZEN_SPACE;
            ++x;
            break;
        case L'\n':
            x = 0;
            ++y;
            break;
        }
        if (y == cy)
            break;
    }
}

static BOOL
XgPattern_RefreshContents(HWND hwnd, INT type)
{
    // �p�^�[���f�[�^���N���A����B
    s_patterns.clear();
    ListBox_ResetContent(GetDlgItem(hwnd, lst1));

    // �p�^�[���f�[�^��ǂݍ��ށB
    WCHAR szPath[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, MAX_PATH);
    PathRemoveFileSpecW(szPath);
    WCHAR szFile[MAX_PATH];
    StringCbCopyW(szFile, sizeof(szFile), szPath);
    PathAppendW(szFile, L"pat\\data.json");
    if (!PathFileExistsW(szFile))
    {
        StringCbCopyW(szFile, sizeof(szFile), szPath);
        PathAppendW(szFile, L"..\\pat\\data.json");
        if (!PathFileExistsW(szFile))
        {
            StringCbCopyW(szFile, sizeof(szFile), szPath);
            PathAppendW(szFile, L"..\\..\\pat\\data.json");
        }
    }
    std::string utf8;
    CHAR buf[256];
    if (FILE *fp = _wfopen(szFile, L"rb"))
    {
        while (fgets(buf, 256, fp))
        {
            utf8 += buf;
        }
        fclose(fp);
    }
    else
    {
        return FALSE;
    }

    json j = json::parse(utf8);
    for (auto& item : j)
    {
        PATDATA pat;
        pat.num_columns = item["num_columns"];
        pat.num_rows = item["num_rows"];

        // �^�C�v�ɂ��t�B���^�[���s���B
        switch (type)
        {
        case rad1:
            if (pat.num_columns != pat.num_rows)
                continue;
            if (!(pat.num_columns >= 13 && pat.num_rows >= 13))
                continue;
            break;
        case rad2:
            if (pat.num_columns != pat.num_rows)
                continue;
            if (!(8 <= pat.num_columns && pat.num_columns <= 12 &&
                  8 <= pat.num_rows && pat.num_rows <= 12))
            {
                continue;
            }
            break;
        case rad3:
            if (pat.num_columns != pat.num_rows)
                continue;
            if (!(pat.num_columns <= 8 && pat.num_rows <= 8))
                continue;
            break;
        case rad4:
            if (pat.num_columns <= pat.num_rows)
                continue;
            break;
        case rad5:
            if (pat.num_columns >= pat.num_rows)
                continue;
            break;
        case rad6:
            if (pat.num_columns != pat.num_rows)
                continue;
            break;
        }

        // data���e�L�X�g�f�[�^�ɂ���B
        std::string str;
        for (auto& subitem : item["data"])
        {
            str += subitem;
            str += "\r\n";
        }
        pat.data = XgUtf8ToUnicode(str);

        // �p�^�[���̃e�L�X�g�f�[�^�������₷���悤�A���H����B
        std::vector<WCHAR> data;
        XgConvertPatternData(data, pat.data, pat.num_columns, pat.num_rows);

        // ���}�X���[����K������B
#define GET_DATA(x, y) data[(y) * pat.num_columns + (x)]
        if (xg_nRules & RULE_DONTDOUBLEBLACK) {
            BOOL bFound = FALSE;
            for (INT y = 0; y < pat.num_rows; ++y) {
                for (INT x = 0; x < pat.num_columns - 1; ++x) {
                    if (GET_DATA(x, y) == ZEN_BLACK && GET_DATA(x + 1, y) == ZEN_BLACK) {
                        x = pat.num_columns;
                        y = pat.num_rows;
                        bFound = TRUE;
                    }
                }
            }
            if (bFound)
                continue;
            bFound = FALSE;
            for (INT x = 0; x < pat.num_columns; ++x) {
                for (INT y = 0; y < pat.num_rows - 1; ++y) {
                    if (GET_DATA(x, y) == ZEN_BLACK && GET_DATA(x, y + 1) == ZEN_BLACK) {
                        x = pat.num_columns;
                        y = pat.num_rows;
                        bFound = TRUE;
                    }
                }
            }
            if (bFound)
                continue;
        }
        if (xg_nRules & RULE_DONTCORNERBLACK) {
            if (GET_DATA(0, 0) == ZEN_BLACK)
                continue;
            if (GET_DATA(pat.num_columns - 1, 0) == ZEN_BLACK)
                continue;
            if (GET_DATA(pat.num_columns - 1, pat.num_rows - 1) == ZEN_BLACK)
                continue;
            if (GET_DATA(0, pat.num_rows - 1) == ZEN_BLACK)
                continue;
        }
        //if (xg_nRules & RULE_DONTDIVIDE)
        if (xg_nRules & RULE_DONTTHREEDIAGONALS) {
            BOOL bFound = FALSE;
            for (INT y = 0; y < pat.num_rows - 2; ++y) {
                for (INT x = 0; x < pat.num_columns - 2; ++x) {
                    if (GET_DATA(x, y) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x + 1, y + 1) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x + 2, y + 2) != ZEN_BLACK)
                        continue;
                    x = pat.num_columns;
                    y = pat.num_rows;
                    bFound = TRUE;
                }
            }
            if (bFound)
                continue;
            bFound = FALSE;
            for (INT y = 0; y < pat.num_rows - 2; ++y) {
                for (INT x = 2; x < pat.num_columns; ++x) {
                    if (GET_DATA(x, y) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x - 1, y + 1) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x - 2, y + 2) != ZEN_BLACK)
                        continue;
                    x = pat.num_columns;
                    y = pat.num_rows;
                    bFound = TRUE;
                }
            }
            if (bFound)
                continue;
        } else if (xg_nRules & RULE_DONTFOURDIAGONALS) {
            BOOL bFound = FALSE;
            for (INT y = 0; y < pat.num_rows - 3; ++y) {
                for (INT x = 0; x < pat.num_columns - 3; ++x) {
                    if (GET_DATA(x, y) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x + 1, y + 1) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x + 2, y + 2) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x + 3, y + 3) != ZEN_BLACK)
                        continue;
                    x = pat.num_columns;
                    y = pat.num_rows;
                    bFound = TRUE;
                }
            }
            if (bFound)
                continue;
            bFound = FALSE;
            for (INT y = 0; y < pat.num_rows - 3; ++y) {
                for (INT x = 3; x < pat.num_columns; ++x) {
                    if (GET_DATA(x, y) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x - 1, y + 1) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x - 2, y + 2) != ZEN_BLACK)
                        continue;
                    if (GET_DATA(x - 3, y + 3) != ZEN_BLACK)
                        continue;
                    x = pat.num_columns;
                    y = pat.num_rows;
                    bFound = TRUE;
                }
            }
            if (bFound)
                continue;
        }
        if (xg_nRules & RULE_DONTTRIDIRECTIONS) {
            BOOL bFound = FALSE;
            for (INT y = 0; y < pat.num_rows; ++y) {
                for (INT x = 0; x < pat.num_columns; ++x) {
                    INT nCount = 0;
                    if (x > 0 && GET_DATA(x - 1, y) == ZEN_BLACK)
                        ++nCount;
                    if (y > 0 && GET_DATA(x, y - 1) == ZEN_BLACK)
                        ++nCount;
                    if (x + 1 < pat.num_columns && GET_DATA(x + 1, y) == ZEN_BLACK)
                        ++nCount;
                    if (y + 1 < pat.num_rows && GET_DATA(x, y + 1) == ZEN_BLACK)
                        ++nCount;
                    if (nCount >= 3) {
                        x = pat.num_columns;
                        y = pat.num_rows;
                        bFound = TRUE;
                    }
                }
            }
            if (bFound)
                continue;
        }
        if (xg_nRules & RULE_POINTSYMMETRY) {
            BOOL bFound = FALSE;
            for (INT y = 0; y < pat.num_rows; ++y) {
                for (INT x = 0; x < pat.num_columns; ++x) {
                    if ((GET_DATA(x, y) == ZEN_BLACK) !=
                        (GET_DATA(pat.num_columns - (x + 1), pat.num_rows - (y + 1)) == ZEN_BLACK))
                    {
                        x = pat.num_columns;
                        y = pat.num_rows;
                        bFound = TRUE;
                    }
                }
            }
            if (bFound)
                continue;
        }
#undef GET_DATA

        s_patterns.push_back(pat);
    }

    // ����������B
    std::random_shuffle(s_patterns.begin(), s_patterns.end());

    // �C���f�b�N�X�Ƃ��Ēǉ�����B
    for (size_t i = 0; i < s_patterns.size(); ++i)
    {
        SendDlgItemMessageW(hwnd, lst1, LB_ADDSTRING, 0, i);
    }

    return TRUE;
}

// WM_INITDIALOG
static BOOL
XgPattern_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    if (s_pLayout)
    {
        LayoutDestroy(s_pLayout);
        s_pLayout = NULL;
    }

    if (xg_bShowAnswerOnPattern)
        CheckDlgButton(hwnd, chx1, BST_CHECKED);
    else
        CheckDlgButton(hwnd, chx1, BST_UNCHECKED);

    CheckRadioButton(hwnd, rad1, rad6, rad6);
    XgPattern_RefreshContents(hwnd, rad6);

    static const LAYOUT_INFO layouts[] =
    {
        { stc1, BF_LEFT | BF_TOP },
        { rad1, BF_LEFT | BF_TOP },
        { rad2, BF_LEFT | BF_TOP },
        { rad3, BF_LEFT | BF_TOP },
        { rad4, BF_LEFT | BF_TOP },
        { rad5, BF_LEFT | BF_TOP },
        { rad6, BF_LEFT | BF_TOP },
        { stc2, BF_LEFT | BF_TOP },
        { lst1, BF_LEFT | BF_TOP | BF_RIGHT | BF_BOTTOM },
        { psh1, BF_LEFT | BF_BOTTOM },
        { chx1, BF_LEFT | BF_BOTTOM },
        { IDOK, BF_RIGHT | BF_BOTTOM },
        { IDCANCEL, BF_RIGHT | BF_BOTTOM },
    };
    s_pLayout = LayoutInit(hwnd, layouts, ARRAYSIZE(layouts));
    LayoutEnableResize(s_pLayout, TRUE);

    if (xg_nPatWndX != CW_USEDEFAULT && xg_nPatWndCX != CW_USEDEFAULT)
    {
        MoveWindow(hwnd, xg_nPatWndX, xg_nPatWndY, xg_nPatWndCX, xg_nPatWndCY, TRUE);
    }
    else
    {
        RECT rc;
        XgCenterDialog(hwnd);
        GetWindowRect(hwnd, &rc);
        xg_nPatWndX = rc.left;
        xg_nPatWndY = rc.top;
        xg_nPatWndCX = rc.right - rc.left;
        xg_nPatWndCY = rc.bottom - rc.top;
    }

    SetFocus(GetDlgItem(hwnd, lst1));
    return FALSE;
}

// ���}�X�p�^�[���̃R�s�[�B
static void XgPattern_OnCopy(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT i = ListBox_GetCurSel(hLst1);
    if (i == LB_ERR || i >= INT(s_patterns.size()))
        return;

    auto& pat = s_patterns[i];
    {
        auto sa1 = std::make_shared<XG_UndoData_SetAll>();
        auto sa2 = std::make_shared<XG_UndoData_SetAll>();
        sa1->Get();
        {
            XgPasteBoard(xg_hMainWnd, pat.data);
            XgCopyBoard(xg_hMainWnd);
        }
        sa2->Get();
        // ���ɖ߂�����ݒ肷��B
        xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
    }
    XgUpdateImage(xg_hMainWnd, 0, 0);

    // �c�[���o�[��UI���X�V����B
    XgUpdateToolBarUI(xg_hMainWnd);

    EndDialog(hwnd, IDCANCEL);
}

// ���}�X�p�^�[���ŁuOK�v�{�^�����������B
static void XgPattern_OnOK(HWND hwnd)
{
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT i = ListBox_GetCurSel(hLst1);
    if (i == LB_ERR || i >= INT(s_patterns.size()))
        return;

    auto& pat = s_patterns[i];
    {
        auto sa1 = std::make_shared<XG_UndoData_SetAll>();
        auto sa2 = std::make_shared<XG_UndoData_SetAll>();
        sa1->Get();
        {
            // �R�s�[���\��t���B
            XgPasteBoard(xg_hMainWnd, pat.data);
            XgCopyBoard(xg_hMainWnd);
        }
        sa2->Get();
        // ���ɖ߂�����ݒ肷��B
        xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
    }

    // �_�C�A���O�����B
    EndDialog(hwnd, IDOK);

    // ������\�����邩�H
    xg_bShowAnswerOnPattern = (IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED);

    {
        auto sa1 = std::make_shared<XG_UndoData_SetAll>();
        auto sa2 = std::make_shared<XG_UndoData_SetAll>();
        sa1->Get();
        {
            // �������߂�i���}�X�ǉ��Ȃ��j�B
            XgOnSolve_NoAddBlack(xg_hMainWnd, xg_bShowAnswerOnPattern);
        }
        sa2->Get();
        // ���ɖ߂�����ݒ肷��B
        xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
    }

    // �\�����X�V����B
    XgUpdateImage(xg_hMainWnd, 0, 0);

    // �c�[���o�[��UI���X�V����B
    XgUpdateToolBarUI(xg_hMainWnd);
}

// WM_COMMAND
static void
XgPattern_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
        XgPattern_OnOK(hwnd);
        break;
    case IDCANCEL:
        EndDialog(hwnd, IDCANCEL);
        break;
    case psh1:
        XgPattern_OnCopy(hwnd);
        break;
    case lst1:
        if (codeNotify == LBN_DBLCLK)
        {
            XgPattern_OnOK(hwnd);
        }
        break;
    case rad1:
    case rad2:
    case rad3:
    case rad4:
    case rad5:
    case rad6:
        XgPattern_RefreshContents(hwnd, id);
        break;
    }
}

static INT cxCell = 6, cyCell = 6; // �����ȃZ���̃T�C�Y�B

// WM_MEASUREITEM
static void
XgPattern_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
    // ���X�g�{�b�N�X�� lst1 ���H
    if (lpMeasureItem->CtlType != ODT_LISTBOX || lpMeasureItem->CtlID != lst1)
        return;

    HDC hDC = CreateCompatibleDC(NULL);
    SelectObject(hDC, GetStockFont(DEFAULT_GUI_FONT));
    TEXTMETRIC tm;
    GetTextMetrics(hDC, &tm);
    DeleteDC(hDC);
    lpMeasureItem->itemWidth = cxCell * 19 + 3;
    lpMeasureItem->itemHeight = cyCell * 18 + 3 + tm.tmHeight;
}

// WM_DRAWITEM
static void
XgPattern_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
    // ���X�g�{�b�N�X�� lst1 ���H
    if (lpDrawItem->CtlType != ODT_LISTBOX || lpDrawItem->CtlID != lst1)
        return;

    // �f�[�^���p�^�[���̃C���f�b�N�X���H
    LPARAM lParam = lpDrawItem->itemData;
    if ((int)lParam >= (int)s_patterns.size())
        return;

    // �C���f�b�N�X�ɑΉ�����p�^�[�����擾�B
    const auto& pat = s_patterns[(int)lParam];
    HDC hDC = lpDrawItem->hDC;
    RECT rcItem = lpDrawItem->rcItem;

    // �K�v�Ȃ�t�H�[�J�X�g��`���B
    if (lpDrawItem->itemAction & ODA_FOCUS)
    {
        DrawFocusRect(hDC, &rcItem);
    }

    // ���̑��͖����B
    if (!(lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)))
        return;

    // �T�C�Y��\���e�L�X�g�B
    WCHAR szText[64];
    StringCbPrintfW(szText, sizeof(szText), L"%u x %u", int(pat.num_columns), int(pat.num_rows));

    // �w�i�ƃe�L�X�g��`�悷��B
    SelectObject(hDC, GetStockFont(DEFAULT_GUI_FONT));
    SetBkMode(hDC, TRANSPARENT);
    TEXTMETRIC tm;
    GetTextMetrics(hDC, &tm);
    if (lpDrawItem->itemState & ODS_SELECTED)
    {
        FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
        SetTextColor(hDC, COLOR_HIGHLIGHTTEXT);
    }
    else
    {
        FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_WINDOW));
        SetTextColor(hDC, COLOR_WINDOWTEXT);
    }
    DrawTextW(hDC, szText, -1, &rcItem, DT_CENTER | DT_TOP | DT_SINGLELINE);

    // �K�v�Ȃ�t�H�[�J�X�g��`���B
    if (lpDrawItem->itemState & ODS_FOCUS)
    {
        DrawFocusRect(hDC, &rcItem);
    }

    // �p�^�[���̃e�L�X�g�f�[�^�������₷���悤�A���H����B
    std::vector<WCHAR> data;
    XgConvertPatternData(data, pat.data, pat.num_columns, pat.num_rows);

    // �`�捀�ڂ̃T�C�Y�B
    INT cxItem = rcItem.right - rcItem.left;

    // �������[�f�o�C�X�R���e�L�X�g���쐬�B
    if (HDC hdcMem = CreateCompatibleDC(hDC))
    {
        INT cx = cxCell * pat.num_columns, cy = cyCell * pat.num_rows; // �S�̂̃T�C�Y�B
        // �r�b�g�}�b�v���쐬����B
        if (HBITMAP hbm = XgCreate24BppBitmap(hdcMem, cx + 3, cy + 3))
        {
            // �r�b�g�}�b�v��I���B
            HGDIOBJ hbmOld = SelectObject(hdcMem, hbm);
            // �l�p�`��`���B
            SelectObject(hdcMem, GetStockPen(BLACK_PEN)); // �����y��
            SelectObject(hdcMem, GetStockBrush(WHITE_BRUSH)); // �����u���V
            Rectangle(hdcMem, 0, 0, cx + 2, cy + 2);
            // ���}�X��`�悷��B
            for (INT y = 0; y < pat.num_rows; ++y)
            {
                RECT rc;
                for (INT x = 0; x < pat.num_columns; ++x)
                {
                    rc.left = 1 + x * cxCell;
                    rc.top = 1 + y * cyCell;
                    rc.right = rc.left + cxCell;
                    rc.bottom = rc.top + cyCell;
                    if (data[x + pat.num_columns * y] == ZEN_BLACK)
                        FillRect(hdcMem, &rc, GetStockBrush(BLACK_BRUSH));
                }
            }
            // ���E����`�悷��B
            for (INT y = 0; y < pat.num_rows + 1; ++y)
            {
                MoveToEx(hdcMem, 1, 1 + y * cyCell, NULL);
                LineTo(hdcMem, 1 + pat.num_columns * cxCell, 1 + y * cyCell);
            }
            for (INT x = 0; x < pat.num_columns + 1; ++x)
            {
                MoveToEx(hdcMem, 1 + x * cxCell, 1, NULL);
                LineTo(hdcMem, 1 + x * cxCell, 1 + pat.num_rows * cyCell);
            }
            // �r�b�g�}�b�v�C���[�W��hDC�ɓ]������B
            BitBlt(hDC,
                   rcItem.left + (cxItem - (cx + 3)) / 2,
                   rcItem.top + tm.tmHeight,
                   cx + 3, cy + 3, hdcMem, 0, 0, SRCCOPY);
            // �r�b�g�}�b�v�̑I������������B
            SelectObject(hdcMem, hbmOld);
            // �r�b�g�}�b�v��j������B
            DeleteObject(hbm);
        }
        // �������[�f�o�C�X�R���e�L�X�g��j������B
        DeleteDC(hdcMem);
    }
}

static void XgPattern_OnDestroy(HWND hwnd)
{
    if (s_pLayout)
    {
        LayoutDestroy(s_pLayout);
        s_pLayout = NULL;
    }
}

static void XgPattern_OnMove(HWND hwnd, int x, int y)
{
    RECT rc;
    if (!IsMinimized(hwnd) && !IsMaximized(hwnd))
    {
        GetWindowRect(hwnd, &rc);
        xg_nPatWndX = rc.left;
        xg_nPatWndY = rc.top;
    }
}

static void XgPattern_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    RECT rc;

    LayoutUpdate(hwnd, s_pLayout, NULL, 0);

    if (!IsMinimized(hwnd) && !IsMaximized(hwnd))
    {
        GetWindowRect(hwnd, &rc);
        xg_nPatWndCX = rc.right - rc.left;
        xg_nPatWndCY = rc.bottom - rc.top;
    }
}

static void XgPattern_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
    lpMinMaxInfo->ptMinTrackSize.x = 600;
    lpMinMaxInfo->ptMinTrackSize.y = 300;
}

// �u���}�X�p�^�[���v�_�C�A���O�v���V�[�W���B
INT_PTR CALLBACK
XgPatternDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, XgPattern_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, XgPattern_OnCommand);
        HANDLE_MSG(hwnd, WM_MEASUREITEM, XgPattern_OnMeasureItem);
        HANDLE_MSG(hwnd, WM_DRAWITEM, XgPattern_OnDrawItem);
        HANDLE_MSG(hwnd, WM_MOVE, XgPattern_OnMove);
        HANDLE_MSG(hwnd, WM_SIZE, XgPattern_OnSize);
        HANDLE_MSG(hwnd, WM_GETMINMAXINFO, XgPattern_OnGetMinMaxInfo);
        HANDLE_MSG(hwnd, WM_DESTROY, XgPattern_OnDestroy);
    }
    return 0;
}

// �u���}�X�p�^�[���v���J���B
void __fastcall XgOpenPatterns(HWND hwnd)
{
    DialogBoxW(xg_hInstance, MAKEINTRESOURCEW(IDD_BLOCKPATTERN), hwnd, XgPatternDlgProc);
}

// ������؂�ւ���B
void MainWnd_DoDictionary(HWND hwnd, size_t iDict)
{
    // �͈͊O�͖����B
    if (iDict >= xg_dict_files.size())
        return;

    // ������ǂݍ��݁A�Z�b�g����B
    const auto& file = xg_dict_files[iDict];
    if (XgLoadDictFile(file.c_str()))
    {
        XgSetDict(file.c_str());
        XgSetInputModeFromDict(hwnd);
    }

    // ��d�}�X�P��̌����N���A����B
    xg_vMarkedCands.clear();
}

// �u���}�X���[���̐���.txt�v���J���B
static void OnOpenRulesTxt(HWND hwnd)
{
    WCHAR szPath[MAX_PATH], szDir[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, MAX_PATH);
    PathRemoveFileSpecW(szPath);
    StringCbCopyW(szDir, sizeof(szDir), szPath);
    StringCbCopyW(szPath, sizeof(szPath), szDir);
    PathAppendW(szPath, XgLoadStringDx1(IDS_RULESTXT));
    if (!PathFileExistsW(szPath)) {
        StringCbCopyW(szPath, sizeof(szPath), szDir);
        PathAppendW(szPath, L"..");
        PathAppendW(szPath, XgLoadStringDx1(IDS_RULESTXT));
        if (!PathFileExistsW(szPath)) {
            StringCbCopyW(szPath, sizeof(szPath), szDir);
            PathAppendW(szPath, L"..\\..");
            PathAppendW(szPath, XgLoadStringDx1(IDS_RULESTXT));
            if (!PathFileExistsW(szPath)) {
                StringCbCopyW(szPath, sizeof(szPath), szDir);
                PathAppendW(szPath, L"..\\..\\..");
                PathAppendW(szPath, XgLoadStringDx1(IDS_RULESTXT));
            }
        }
    }
    ShellExecuteW(hwnd, NULL, szPath, NULL, NULL, SW_SHOWNORMAL);
}

// ���}�X���[�����`�F�b�N����B
void __fastcall XgRuleCheck(HWND hwnd)
{
    XG_Board& board = (xg_bShowAnswer ? xg_solution : xg_xword);
    // �A���ցB
    if (xg_nRules & RULE_DONTDOUBLEBLACK) {
        if (board.DoubleBlack()) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_ADJACENTBLOCK), nullptr, MB_ICONERROR);
            return;
        }
    }
    // �l�����ցB
    if (xg_nRules & RULE_DONTCORNERBLACK) {
        if (board.CornerBlack()) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CORNERBLOCK), nullptr, MB_ICONERROR);
            return;
        }
    }
    // �O�����ցB
    if (xg_nRules & RULE_DONTTRIDIRECTIONS) {
        if (board.TriBlackAround()) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_TRIBLOCK), nullptr, MB_ICONERROR);
            return;
        }
    }
    // ���f�ցB
    if (xg_nRules & RULE_DONTDIVIDE) {
        if (board.DividedByBlack()) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_DIVIDED), nullptr, MB_ICONERROR);
            return;
        }
    }
    // ���ΎO�A�ցB
    if (xg_nRules & RULE_DONTTHREEDIAGONALS) {
        if (board.ThreeDiagonals()) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_THREEDIAGONALS), nullptr, MB_ICONERROR);
            return;
        }
    } else {
        // ���Ύl�A�ցB
        if (xg_nRules & RULE_DONTFOURDIAGONALS) {
            if (board.FourDiagonals()) {
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_FOURDIAGONALS), nullptr, MB_ICONERROR);
                return;
            }
        }
    }
    // ���}�X�_�Ώ́B
    if (xg_nRules & RULE_POINTSYMMETRY) {
        if (!board.IsPointSymmetry()) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_NOTPOINTSYMMETRY), nullptr, MB_ICONERROR);
            return;
        }
    }

    // ���i�B
    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_RULESPASSED),
                        XgLoadStringDx2(IDS_PASSED), MB_ICONINFORMATION);
}

// �^�O���X�g�{�b�N�X���������B
static void XgInitTagListView(HWND hwndLV)
{
    DWORD exstyle = LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_GRIDLINES;
    ListView_SetExtendedListViewStyleEx(hwndLV, exstyle, exstyle);

    LV_COLUMN column = { LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT };

    column.pszText = XgLoadStringDx1(IDS_TAGS);
    column.fmt = LVCFMT_LEFT;
    column.cx = 90;
    column.iSubItem = 0;
    ListView_InsertColumn(hwndLV, 0, &column);

    column.pszText = XgLoadStringDx1(IDS_TAGCOUNT);
    column.fmt = LVCFMT_RIGHT;
    column.cx = 84;
    column.iSubItem = 1;
    ListView_InsertColumn(hwndLV, 1, &column);
}

template <typename T_STR_CONTAINER>
inline void
mstr_split(T_STR_CONTAINER& container,
           const typename T_STR_CONTAINER::value_type& str,
           const typename T_STR_CONTAINER::value_type& chars)
{
    container.clear();
    size_t i = 0, k = str.find_first_of(chars);
    while (k != T_STR_CONTAINER::value_type::npos)
    {
        container.push_back(str.substr(i, k - i));
        i = k + 1;
        k = str.find_first_of(chars, i);
    }
    container.push_back(str.substr(i));
}

static void XgTheme_SetPreset(HWND hwnd, LPCWSTR pszText)
{
    HWND hLst2 = GetDlgItem(hwnd, lst2);
    HWND hLst3 = GetDlgItem(hwnd, lst3);
    ListView_DeleteAllItems(hLst2);
    ListView_DeleteAllItems(hLst3);

    std::vector<std::wstring> strs;
    std::wstring strText = pszText;

    xg_str_replace_all(strText, L" ", L"");
    mstr_split(strs, strText, L",");

    WCHAR szText[64];

    for (auto& str : strs) {
        if (str.empty())
            continue;

        bool minus = false;
        if (str[0] == L'-') {
            minus = true;
            str = str.substr(1);
        }
        if (str[0] == L'+') {
            str = str.substr(1);
        }

        LV_ITEM item = { LVIF_TEXT };
        INT iItem = ListView_GetItemCount(hLst3);
        StringCbCopyW(szText, sizeof(szText), str.c_str());
        item.iItem = iItem;
        item.pszText = szText;
        item.iSubItem = 0;
        if (minus)
            ListView_InsertItem(hLst3, &item);
        else
            ListView_InsertItem(hLst2, &item);

        StringCbCopyW(szText, sizeof(szText), std::to_wstring(xg_tag_histgram[str]).c_str());
        item.iItem = iItem;
        item.pszText = szText;
        item.iSubItem = 1;
        if (minus)
            ListView_SetItem(hLst3, &item);
        else
            ListView_SetItem(hLst2, &item);
    }

    // �ŏ��̍��ڂ�I������B
    LV_ITEM item;
    item.mask = LVIF_STATE;
    item.iItem = 0;
    item.iSubItem = 0;
    item.state = item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
    ListView_SetItem(hLst2, &item);

    item.mask = LVIF_STATE;
    item.iItem = 0;
    item.iSubItem = 0;
    item.state = item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
    ListView_SetItem(hLst3, &item);
}

// �^�O�Q�̍ő咷�B
#define MAX_TAGSLEN 256

static void XgTheme_SetPreset(HWND hwnd)
{
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    INT iItem = ComboBox_GetCurSel(hCmb1);

    WCHAR szText[MAX_TAGSLEN];
    if (iItem == CB_ERR) {
        GetDlgItemTextW(hwnd, cmb1, szText, ARRAYSIZE(szText));
    } else {
        ComboBox_GetLBText(hCmb1, iItem, szText);
    }

    XgTheme_SetPreset(hwnd, szText);
}

static BOOL xg_bUpdatingPreset = FALSE;

static void XgTheme_UpdatePreset(HWND hwnd)
{
    HWND hLst2 = GetDlgItem(hwnd, lst2);
    HWND hLst3 = GetDlgItem(hwnd, lst3);

    std::wstring str;
    WCHAR szText[64];
    INT nCount2 = ListView_GetItemCount(hLst2);
    INT nCount3 = ListView_GetItemCount(hLst3);
    for (INT i = 0; i < nCount2; ++i) {
        if (str.size()) {
            str += L",";
        }
        ListView_GetItemText(hLst2, i, 0, szText, ARRAYSIZE(szText));
        str += L"+";
        str += szText;
    }
    for (INT i = 0; i < nCount3; ++i) {
        if (str.size()) {
            str += L",";
        }
        ListView_GetItemText(hLst3, i, 0, szText, ARRAYSIZE(szText));
        str += L"-";
        str += szText;
    }

    // ���������B
    if (str.size() > MAX_TAGSLEN - 1)
        str.resize(MAX_TAGSLEN - 1);

    xg_bUpdatingPreset = TRUE;
    SetDlgItemTextW(hwnd, cmb1, str.c_str());
    xg_bUpdatingPreset = FALSE;
}

// �u�e�[�}�v�_�C�A���O�̏������B
static BOOL XgTheme_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    // �_�C�A���O�𒆉��񂹂���B
    XgCenterDialog(hwnd);

    // �����𐧌�����B
    SendDlgItemMessageW(hwnd, cmb1, CB_LIMITTEXT, MAX_TAGSLEN - 1, 0);

    // ���X�g�r���[���������B
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    HWND hLst2 = GetDlgItem(hwnd, lst2);
    HWND hLst3 = GetDlgItem(hwnd, lst3);
    XgInitTagListView(hLst1);
    XgInitTagListView(hLst2);
    XgInitTagListView(hLst3);

    // �q�X�g�O�������擾�B
    std::vector<std::pair<size_t, std::wstring> > histgram;
    for (auto& pair : xg_tag_histgram) {
        histgram.emplace_back(std::make_pair(pair.second, pair.first));
    }
    // �o���񐔂̋t���Ń\�[�g�B
    std::sort(histgram.begin(), histgram.end(),
        [](const std::pair<size_t, std::wstring>& a, const std::pair<size_t, std::wstring>& b) {
            return a.first > b.first;
        }
    );

    // ���X�g�r���[���t���̃q�X�g�O�����Ŗ��߂�B
    INT iItem = 0;
    LV_ITEM item = { LVIF_TEXT };
    WCHAR szText[64];
    for (auto& pair : histgram) {
        StringCbCopyW(szText, sizeof(szText), pair.second.c_str());
        item.iItem = iItem;
        item.pszText = szText;
        item.iSubItem = 0;
        ListView_InsertItem(hLst1, &item);

        StringCbCopyW(szText, sizeof(szText), std::to_wstring(pair.first).c_str());
        item.iItem = iItem;
        item.pszText = szText;
        item.iSubItem = 1;
        ListView_SetItem(hLst1, &item);

        ++iItem;
    }

    // �R���{�{�b�N�X�Ƀe�L�X�g��ݒ肷��B
    SetDlgItemTextW(hwnd, cmb1, xg_strTheme.c_str());
    // �v���Z�b�g��ݒ肷��B
    XgTheme_SetPreset(hwnd, xg_strTheme.c_str());

    // �ŏ��̍��ڂ�I���B
    item.mask = LVIF_STATE;
    item.iItem = 0;
    item.iSubItem = 0;
    item.state = item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
    ListView_SetItem(hLst1, &item);

    return TRUE;
}

// ���X�g�r���[�Ƀ^�O���ڂ�ǉ�����B
static void XgTheme_AddTag(HWND hwnd, BOOL bPriority)
{
    // �I�𒆂̃e�L�X�g���擾����B
    HWND hLst1 = GetDlgItem(hwnd, lst1);
    INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_ALL | LVNI_SELECTED);
    if (iItem < 0)
        return; // �I���Ȃ��B
    WCHAR szText1[64], szText2[64];
    ListView_GetItemText(hLst1, iItem, 0, szText1, ARRAYSIZE(szText1));
    ListView_GetItemText(hLst1, iItem, 1, szText2, ARRAYSIZE(szText2));

    LV_FINDINFO find = { LVFI_STRING, szText1 };
    if (bPriority) {
        HWND hLst2 = GetDlgItem(hwnd, lst2);
        iItem = ListView_FindItem(hLst2, -1, &find);
        if (iItem >= 0)
            return; // ���łɂ������B

        // �^�O���ڂ�ǉ��B
        INT cItems = ListView_GetItemCount(hLst2);
        LV_ITEM item = { LVIF_TEXT };
        item.iItem = cItems;
        item.iSubItem = 0;
        item.pszText = szText1;
        iItem = ListView_InsertItem(hLst2, &item);
        item.iItem = iItem;
        item.iSubItem = 1;
        item.pszText = szText2;
        ListView_SetItem(hLst2, &item);

        // �J�E���^�[���X�V�B
        size_t count = 0;
        cItems = ListView_GetItemCount(hLst2);
        for (iItem = 0; iItem < cItems; ++iItem) {
            ListView_GetItemText(hLst2, iItem, 1, szText2, ARRAYSIZE(szText2));
            count += _wtoi(szText2);
        }
        SetDlgItemInt(hwnd, stc1, INT(count), FALSE);
    } else {
        HWND hLst3 = GetDlgItem(hwnd, lst3);
        iItem = ListView_FindItem(hLst3, -1, &find);
        if (iItem >= 0)
            return; // ���łɂ������B

        // �^�O���ڂ�ǉ��B
        INT cItems = ListView_GetItemCount(hLst3);
        LV_ITEM item = { LVIF_TEXT };
        item.iItem = cItems;
        item.iSubItem = 0;
        item.pszText = szText1;
        iItem = ListView_InsertItem(hLst3, &item);
        item.iItem = iItem;
        item.iSubItem = 1;
        item.pszText = szText2;
        ListView_SetItem(hLst3, &item);

        // �J�E���^�[���X�V�B
        size_t count = 0;
        cItems = ListView_GetItemCount(hLst3);
        for (iItem = 0; iItem < cItems; ++iItem) {
            ListView_GetItemText(hLst3, iItem, 1, szText2, ARRAYSIZE(szText2));
            count += _wtoi(szText2);
        }
        SetDlgItemInt(hwnd, stc2, INT(count), FALSE);
    }

    // �v���Z�b�g���X�V�B
    XgTheme_UpdatePreset(hwnd);
}

// ���X�g�r���[����^�O���ڂ��폜����B
static void XgTheme_RemoveTag(HWND hwnd, BOOL bPriority)
{
    WCHAR szText[64];
    if (bPriority) {
        HWND hLst2 = GetDlgItem(hwnd, lst2);
        INT iItem = ListView_GetNextItem(hLst2, -1, LVNI_ALL | LVNI_SELECTED);
        ListView_DeleteItem(hLst2, iItem);

        // �J�E���^�[���X�V�B
        INT cItems = ListView_GetItemCount(hLst2);
        size_t count = 0;
        for (iItem = 0; iItem < cItems; ++iItem) {
            ListView_GetItemText(hLst2, iItem, 1, szText, ARRAYSIZE(szText));
            count += _wtoi(szText);
        }
        SetDlgItemInt(hwnd, stc1, INT(count), FALSE);
    } else {
        HWND hLst3 = GetDlgItem(hwnd, lst3);
        INT iItem = ListView_GetNextItem(hLst3, -1, LVNI_ALL | LVNI_SELECTED);
        ListView_DeleteItem(hLst3, iItem);

        // �J�E���^�[���X�V�B
        INT cItems = ListView_GetItemCount(hLst3);
        size_t count = 0;
        for (iItem = 0; iItem < cItems; ++iItem) {
            ListView_GetItemText(hLst3, iItem, 1, szText, ARRAYSIZE(szText));
            count += _wtoi(szText);
        }
        SetDlgItemInt(hwnd, stc2, INT(count), FALSE);
    }

    // �v���Z�b�g���X�V�B
    XgTheme_UpdatePreset(hwnd);
}

// �u�e�[�}�v�_�C�A���O�ŁuOK�v�{�^���������ꂽ�B
static BOOL XgTheme_OnOK(HWND hwnd)
{
    HWND hLst2 = GetDlgItem(hwnd, lst2);
    HWND hLst3 = GetDlgItem(hwnd, lst3);

    xg_priority_tags.clear();
    xg_forbidden_tags.clear();

    std::wstring strTheme;
    WCHAR szText[MAX_TAGSLEN];
    INT cItems;

    cItems = ListView_GetItemCount(hLst2);
    for (INT iItem = 0; iItem < cItems; ++iItem) {
        ListView_GetItemText(hLst2, iItem, 0, szText, ARRAYSIZE(szText));
        xg_priority_tags.emplace(szText);
        if (strTheme.size())
            strTheme += L',';
        strTheme += L'+';
        strTheme += szText;
    }

    cItems = ListView_GetItemCount(hLst3);
    for (INT iItem = 0; iItem < cItems; ++iItem) {
        ListView_GetItemText(hLst3, iItem, 0, szText, ARRAYSIZE(szText));
        xg_forbidden_tags.emplace(szText);
        if (strTheme.size())
            strTheme += L',';
        strTheme += L'-';
        strTheme += szText;
    }

    XgSetThemeString(strTheme);

    return TRUE;
}

// �^�O�̌����B
static void XgTheme_OnEdt1(HWND hwnd)
{
    WCHAR szText[64];
    GetDlgItemTextW(hwnd, edt1, szText, ARRAYSIZE(szText));

    HWND hLst1 = GetDlgItem(hwnd, lst1);

    LV_FINDINFO find = { LVFI_STRING | LVFI_PARTIAL };
    find.psz = szText;
    INT iItem = ListView_FindItem(hLst1, -1, &find);
    UINT state = LVIS_FOCUSED | LVIS_SELECTED;
    ListView_SetItemState(hLst1, iItem, state, state);
    ListView_EnsureVisible(hLst1, iItem, FALSE);
}

// �u�e�[�}�v�_�C�A���O�̃R�}���h�����B
static void XgTheme_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
        if (XgTheme_OnOK(hwnd)) {
            EndDialog(hwnd, IDOK);
        }
        break;
    case IDCANCEL:
        EndDialog(hwnd, id);
        break;
    case psh1: // ��
        XgTheme_AddTag(hwnd, TRUE);
        break;
    case psh2: // ��
        XgTheme_RemoveTag(hwnd, TRUE);
        break;
    case psh3: // ��
        XgTheme_AddTag(hwnd, FALSE);
        break;
    case psh4: // ��
        XgTheme_RemoveTag(hwnd, FALSE);
        break;
    case psh5: // ���Z�b�g
        XgSetThemeString(xg_strDefaultTheme);
        EndDialog(hwnd, IDOK);
        break;
    case edt1:
        if (codeNotify == EN_CHANGE) {
            XgTheme_OnEdt1(hwnd);
        }
        break;
    case cmb1:
        if (codeNotify == CBN_EDITCHANGE && !xg_bUpdatingPreset) {
            XgTheme_SetPreset(hwnd);
        }
        break;
    }
}

LRESULT XgTheme_OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    LV_KEYDOWN *pKeyDown;
    switch (idFrom) {
    case lst1:
        if (pnmhdr->code == NM_DBLCLK) {
            XgTheme_AddTag(hwnd, TRUE);
        }
        break;
    case lst2:
        if (pnmhdr->code == NM_DBLCLK) {
            XgTheme_RemoveTag(hwnd, TRUE);
        } else if (pnmhdr->code == LVN_KEYDOWN) {
            pKeyDown = reinterpret_cast<LV_KEYDOWN *>(pnmhdr);
            if (pKeyDown->wVKey == VK_DELETE)
                XgTheme_RemoveTag(hwnd, TRUE);
        }
        break;
    case lst3:
        if (pnmhdr->code == NM_DBLCLK) {
            XgTheme_RemoveTag(hwnd, FALSE);
        } else if (pnmhdr->code == LVN_KEYDOWN) {
            pKeyDown = reinterpret_cast<LV_KEYDOWN *>(pnmhdr);
            if (pKeyDown->wVKey == VK_DELETE)
                XgTheme_RemoveTag(hwnd, FALSE);
        }
        break;
    }
    return 0;
}

// �u�e�[�}�v�_�C�A���O�v���V�[�W���B
static INT_PTR CALLBACK
XgThemeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, XgTheme_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, XgTheme_OnCommand);
        HANDLE_MSG(hwnd, WM_NOTIFY, XgTheme_OnNotify);
    }
    return 0;
}

// �u�e�[�}�v�_�C�A���O��\������B
void __fastcall XgTheme(HWND hwnd)
{
    if (xg_tag_histgram.empty()) {
        // �^�O������܂���B
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_NOTAGS), NULL, MB_ICONERROR);
        return;
    }

    INT id = DialogBoxW(xg_hInstance, MAKEINTRESOURCEW(IDD_THEME), hwnd, XgThemeDlgProc);
    if (id == IDOK) {
        XgUpdateTheme(hwnd);
    }
}

// �e�[�}�����Z�b�g����B
void __fastcall XgResetTheme(HWND hwnd, BOOL bQuery)
{
    if (bQuery) {
        INT id = XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_RESETTHEME),
                                     XgLoadStringDx2(IDS_APPNAME),
                                     MB_ICONINFORMATION | MB_YESNOCANCEL);
        if (id != IDYES)
            return;
    }
    XgResetTheme(hwnd);
    XgUpdateTheme(hwnd);
}

void __fastcall XgShowResults(HWND hwnd)
{
    WCHAR sz[MAX_PATH];
    if (xg_bCancelled) {
        // �L�����Z�����ꂽ�B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_CANCELLED),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);
    } else if (xg_bSolved) {
        // �������b�Z�[�W��\������B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_MADEPROBLEM),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);

        // �q���g���X�V���ĊJ���B
        XgUpdateHints(hwnd);
        XgShowHints(hwnd);
    } else {
        // ���s���b�Z�[�W��\������B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_CANTMAKEPROBLEM),
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
        XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONERROR);
    }
}

// ���b�Z�[�W�{�b�N�X��\������B
void __fastcall XgShowResultsRepeatedly(HWND hwnd)
{
    WCHAR sz[MAX_PATH];

    // �f�B�X�N�ɋ󂫂����邩�H
    if (s_bOutOfDiskSpace) {
        // �Ȃ������B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_OUTOFSTORAGE), s_nNumberGenerated,
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
    } else {
        // �������B
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_PROBLEMSMADE), s_nNumberGenerated,
            (s_dwTick2 - s_dwTick0) / 1000,
            (s_dwTick2 - s_dwTick0) / 100 % 10);
    }

    // �I�����b�Z�[�W��\������B
    XgCenterMessageBoxW(hwnd, sz, XgLoadStringDx2(IDS_RESULTS), MB_ICONINFORMATION);

    // �ۑ���t�H���_���J���B
    if (s_nNumberGenerated && !s_dirs_save_to.empty())
        ::ShellExecuteW(hwnd, nullptr, s_dirs_save_to[0].data(),
                        nullptr, nullptr, SW_SHOWNORMAL);
}

static void XgSetZoomRate(HWND hwnd, INT nZoomRate)
{
    xg_nZoomRate = nZoomRate;
    INT x = XgGetHScrollPos();
    INT y = XgGetVScrollPos();
    XgUpdateScrollInfo(hwnd, x, y);
    XgUpdateImage(hwnd, x, y);
}

// �R�}���h�����s����B
void __fastcall MainWnd_OnCommand(HWND hwnd, int id, HWND /*hwndCtl*/, UINT /*codeNotify*/)
{
    WCHAR sz[MAX_PATH];
    OPENFILENAMEW ofn;
    int x, y;

    switch (id) {
    case ID_LEFT:
        // �L�����b�g�����ֈړ��B
        if (xg_caret_pos.m_j > 0) {
            xg_caret_pos.m_j--;
            XgEnsureCaretVisible(hwnd);
            x = XgGetHScrollPos();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        } else {
            // ��ԍ��̃L�����b�g�Ȃ�A���[�ֈړ��B
            x = 0;
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        }
        xg_prev_vk = 0;
        break;
    case ID_RIGHT:
        // �L�����b�g���E�ֈړ��B
        if (xg_caret_pos.m_j + 1 < xg_nCols) {
            xg_caret_pos.m_j++;
            XgEnsureCaretVisible(hwnd);
            x = XgGetHScrollPos();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        } else {
            // ��ԉE�̃L�����b�g�Ȃ�A�E�[�ֈړ��B
            SIZE siz;
            ForDisplay for_display;
            XgGetXWordExtent(&siz);
            MRect rcClient;
            XgGetRealClientRect(hwnd, &rcClient);
            x = siz.cx - rcClient.Width();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        }
        xg_prev_vk = 0;
        break;
    case ID_UP:
        // �L�����b�g����ֈړ��B
        if (xg_caret_pos.m_i > 0) {
            xg_caret_pos.m_i--;
            XgEnsureCaretVisible(hwnd);
            x = XgGetHScrollPos();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        } else {
            // ��ԏ�̃L�����b�g�Ȃ�A��[�ֈړ��B
            x = XgGetHScrollPos();
            y = 0;
            XgUpdateImage(hwnd, x, y);
        }
        xg_prev_vk = 0;
        break;
    case ID_DOWN:
        // �L�����b�g�����ֈړ��B
        if (xg_caret_pos.m_i + 1 < xg_nRows) {
            xg_caret_pos.m_i++;
            XgEnsureCaretVisible(hwnd);
            x = XgGetHScrollPos();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        } else {
            // ��ԉ��̃L�����b�g�Ȃ�A���[�ֈړ��B
            SIZE siz;
            ForDisplay for_display;
            XgGetXWordExtent(&siz);
            MRect rcClient;
            XgGetRealClientRect(hwnd, &rcClient);
            x = XgGetHScrollPos();
            y = siz.cy - rcClient.Height();
            XgUpdateImage(hwnd, x, y);
        }
        xg_prev_vk = 0;
        break;
    case ID_MOSTLEFT:
        // Ctrl+���B
        if (xg_caret_pos.m_j > 0) {
            xg_caret_pos.m_j = 0;
            XgUpdateStatusBar(hwnd);
            XgEnsureCaretVisible(hwnd);
            x = XgGetHScrollPos();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        } else {
            // ��ԍ��̃L�����b�g�Ȃ�A���[�ֈړ��B
            x = 0;
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        }
        xg_prev_vk = 0;
        break;
    case ID_MOSTRIGHT:
        // Ctrl+���B
        if (xg_caret_pos.m_j + 1 < xg_nCols) {
            xg_caret_pos.m_j = xg_nCols - 1;
            XgUpdateStatusBar(hwnd);
            XgEnsureCaretVisible(hwnd);
            x = XgGetHScrollPos();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        } else {
            // ��ԉE�̃L�����b�g�Ȃ�A�E�[�ֈړ��B
            SIZE siz;
            ForDisplay for_display;
            XgGetXWordExtent(&siz);
            MRect rcClient;
            XgGetRealClientRect(hwnd, &rcClient);
            x = siz.cx - rcClient.Width();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        }
        xg_prev_vk = 0;
        break;
    case ID_MOSTUPPER:
        // Ctrl+���B
        if (xg_caret_pos.m_i > 0) {
            xg_caret_pos.m_i = 0;
            XgUpdateStatusBar(hwnd);
            XgEnsureCaretVisible(hwnd);
            x = XgGetHScrollPos();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        } else {
            // ��ԏ�̃L�����b�g�Ȃ�A��[�ֈړ��B
            x = XgGetHScrollPos();
            y = 0;
            XgUpdateImage(hwnd, x, y);
        }
        xg_prev_vk = 0;
        break;
    case ID_MOSTLOWER:
        // Ctrl+���B
        if (xg_caret_pos.m_i + 1 < xg_nRows) {
            xg_caret_pos.m_i = xg_nRows - 1;
            XgUpdateStatusBar(hwnd);
            XgEnsureCaretVisible(hwnd);
            x = XgGetHScrollPos();
            y = XgGetVScrollPos();
            XgUpdateImage(hwnd, x, y);
        } else {
            // ��ԉ��̃L�����b�g�Ȃ�A���[�ֈړ��B
            SIZE siz;
            ForDisplay for_display;
            XgGetXWordExtent(&siz);
            MRect rcClient;
            XgGetRealClientRect(hwnd, &rcClient);
            x = XgGetHScrollPos();
            y = siz.cy - rcClient.Height();
            XgUpdateImage(hwnd, x, y);
        }
        xg_prev_vk = 0;
        break;
    case ID_OPENCANDSWNDHORZ:
        XgOpenCandsWnd(hwnd, false);
        xg_prev_vk = 0;
        break;
    case ID_OPENCANDSWNDVERT:
        XgOpenCandsWnd(hwnd, true);
        xg_prev_vk = 0;
        break;

    case ID_HEADERANDNOTES:
        ::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_NOTES), hwnd, XgNotesDlgProc);
        break;

    case ID_COPYHINTSSTYLE0:
        XgCopyHintsStyle0(hwnd, 2);
        break;

    case ID_COPYHINTSSTYLE1:
        XgCopyHintsStyle1(hwnd, 2);
        break;

    case ID_COPYVHINTSSTYLE0:
        XgCopyHintsStyle0(hwnd, 0);
        break;

    case ID_COPYHHINTSSTYLE0:
        XgCopyHintsStyle0(hwnd, 1);
        break;

    case ID_COPYVHINTSSTYLE1:
        XgCopyHintsStyle1(hwnd, 0);
        break;

    case ID_COPYHHINTSSTYLE1:
        XgCopyHintsStyle1(hwnd, 1);
        break;

    case ID_LOADDICTFILE:
        if (::DialogBoxW(xg_hInstance, MAKEINTRESOURCE(IDD_READDICT),
                         hwnd, XgLoadDictDlgProc) == IDOK)
        {
            // ��d�}�X�P����N���A����B
            SendMessageW(hwnd, WM_COMMAND, ID_KILLMARKS, 0);
        }
        break;

    case ID_SETTINGS:   // �ݒ�B
        MainWnd_OnSettings(hwnd);
        break;

    case ID_ERASESETTINGS:  // �ݒ�̍폜�B
        MainWnd_OnEraseSettings(hwnd);
        break;

    case ID_FLIPVH: // �c�Ɖ������ւ���B
        {
            bool flag = !!::IsWindow(xg_hHintsWnd);
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �q���g�E�B���h�E��j������B
            XgDestroyHintsWnd();
            // �c�Ɖ������ւ���B
            MainWnd_OnFlipVH(hwnd);
            if (flag) {
                XgShowHints(hwnd);
            }
            sa2->Get();
            xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
            // �c�[���o�[��UI���X�V����B
            XgUpdateToolBarUI(hwnd);
        }
        break;

    case ID_NEW:    // �V�K�쐬�B
        {
            bool flag = false;
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �q���g�E�B���h�E��j������B
            XgDestroyHintsWnd();
            // �V�K�쐬�_�C�A���O�B
            if (XgOnNew(hwnd)) {
                flag = true;
                sa2->Get();
            }
            if (flag) {
                xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
            } else {
                sa1->Apply();
            }
        }
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        break;

    case ID_GENERATE:   // ����������������B
        {
            bool flag = false;
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �q���g�E�B���h�E��j������B
            XgDestroyHintsWnd();
            // ���̍쐬�B
            if (XgOnGenerate(hwnd, false, false)) {
                flag = true;
                sa2->Get();
                xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
                // �C���[�W���X�V����B
                xg_caret_pos.clear();
                XgMarkUpdate();
                XgUpdateImage(hwnd, 0, 0);
                // ���b�Z�[�W�{�b�N�X��\������B
                XgShowResults(hwnd);
            }
            if (!flag) {
                sa1->Apply();
            }
            // �C���[�W���X�V����B
            xg_caret_pos.clear();
            XgMarkUpdate();
            XgUpdateImage(hwnd, 0, 0);
        }
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        break;

    case ID_GENERATEANSWER:   // ����������������i�����t���j�B
        {
            bool flag = false;
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �q���g�E�B���h�E��j������B
            XgDestroyHintsWnd();
            // ���̍쐬�B
            if (XgOnGenerate(hwnd, true, false)) {
                flag = true;
                sa2->Get();
                xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
                // �C���[�W���X�V����B
                xg_caret_pos.clear();
                XgMarkUpdate();
                XgUpdateImage(hwnd, 0, 0);
                // ���b�Z�[�W�{�b�N�X��\������B
                XgShowResults(hwnd);
            }
            if (!flag) {
                sa1->Apply();
            }
            // �C���[�W���X�V����B
            xg_caret_pos.clear();
            XgMarkUpdate();
            XgUpdateImage(hwnd, 0, 0);
        }
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        break;

    case ID_GENERATEREPEATEDLY:     // ����A��������������
        {
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �q���g�E�B���h�E��j������B
            XgDestroyHintsWnd();
            // �A�������_�C�A���O�B
            if (XgOnGenerate(hwnd, false, true)) {
                // �N���A����B
                xg_bShowAnswer = false;
                xg_bSolved = false;
                xg_xword.clear();
                xg_solution.clear();
                // ���ɖ߂������c���B
                sa2->Get();
                xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
                // �C���[�W���X�V����B
                xg_caret_pos.clear();
                XgMarkUpdate();
                XgUpdateImage(hwnd, 0, 0);
                // ���b�Z�[�W�{�b�N�X��\������B
                XgShowResultsRepeatedly(hwnd);
            }
            // �C���[�W���X�V����B
            xg_caret_pos.clear();
            XgMarkUpdate();
            XgUpdateImage(hwnd, 0, 0);
        }
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        break;

    case ID_OPEN:   // �t�@�C�����J���B
        // ���[�U�[�Ƀt�@�C���̏ꏊ��₢���킹��B
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = XgMakeFilterString(XgLoadStringDx2(IDS_CROSSFILTER));
        sz[0] = 0;
        ofn.lpstrFile = sz;
        ofn.nMaxFile = static_cast<DWORD>(ARRAYSIZE(sz));
        ofn.lpstrTitle = XgLoadStringDx1(IDS_OPENCROSSDATA);
        ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST |
            OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = L"xwd";
        if (::GetOpenFileNameW(&ofn)) {
            // JSON�`�����H
            bool is_json = false;
            bool is_builder = false;
            if (::lstrcmpiW(PathFindExtensionW(sz), L".xwj") == 0 ||
                ::lstrcmpiW(PathFindExtensionW(sz), L".json") == 0)
            {
                is_json = true;
            }
            if (::lstrcmpiW(PathFindExtensionW(sz), L".crp") == 0 ||
                ::lstrcmpiW(PathFindExtensionW(sz), L".crx") == 0)
            {
                is_builder = true;
            }
            // �J���B
            if (is_builder) {
                if (!XgDoLoadCrpFile(hwnd, sz)) {
                    // ���s�B
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTLOAD), nullptr, MB_ICONERROR);
                } else {
                    // �����B
                    xg_ubUndoBuffer.Empty();
                    xg_caret_pos.clear();
                    // �C���[�W���X�V����B
                    XgUpdateImage(hwnd, 0, 0);
                }
            } else {
                if (!XgDoLoadFile(hwnd, sz, is_json)) {
                    // ���s�B
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTLOAD), nullptr, MB_ICONERROR);
                } else {
                    // �����B
                    xg_ubUndoBuffer.Empty();
                    xg_caret_pos.clear();
                    // �C���[�W���X�V����B
                    XgUpdateImage(hwnd, 0, 0);
                    // �e�[�}���X�V����B
                    XgSetThemeString(xg_strTheme);
                    XgUpdateTheme(hwnd);
                }
            }
        }
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        // ���[�����X�V����B
        XgUpdateRules(hwnd);
        break;

    case ID_SAVEAS: // �t�@�C����ۑ�����B
        if (xg_dict_files.empty()) {
            // �����t�@�C���̖��O��ǂݍ��ށB
            XgLoadDictsAll();
        }
        // ���[�U�[�Ƀt�@�C���̏ꏊ��₢���킹�鏀���B
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = XgMakeFilterString(XgLoadStringDx2(IDS_SAVEFILTER));
        StringCbCopy(sz, sizeof(sz), xg_strFileName.data());
        ofn.lpstrFile = sz;
        ofn.nMaxFile = static_cast<DWORD>(ARRAYSIZE(sz));
        ofn.lpstrTitle = XgLoadStringDx1(IDS_SAVECROSSDATA);
        ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT |
            OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        // JSON only
        ofn.nFilterIndex = 1;
        ofn.lpstrDefExt = L"xwj";
        if (lstrcmpiW(PathFindExtensionW(sz), L".xwd") == 0)
        {
            PathRemoveExtensionW(sz);
        }
        // ���[�U�[�Ƀt�@�C���̏ꏊ��₢���킹��B
        if (::GetSaveFileNameW(&ofn)) {
            // �ۑ�����B
            if (!XgDoSave(hwnd, sz)) {
                // �ۑ��Ɏ��s�B
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTSAVE2), nullptr, MB_ICONERROR);
            }
        }
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        break;

    case ID_SAVEPROBASIMAGE:    // �����摜�t�@�C���Ƃ��ĕۑ�����B
        XgSaveProbAsImage(hwnd);
        break;

    case ID_SAVEANSASIMAGE:     // �𓚂��摜�t�@�C���Ƃ��ĕۑ�����B
        XgSaveAnsAsImage(hwnd);
        break;

    case ID_LINESYMMETRYCHECK:  // ���Ώ̃`�F�b�N�B
        XgOnLineSymmetryCheck(hwnd);
        break;

    case ID_POINTSYMMETRYCHECK: // �_�Ώ̃`�F�b�N�B
        XgOnPointSymmetryCheck(hwnd);
        break;

    case ID_EXIT:   // �I������B
        DestroyWindow(hwnd);
        break;

    case ID_UNDO:   // ���ɖ߂��B
        if (::GetForegroundWindow() != xg_hMainWnd) {
            HWND hwnd = ::GetFocus();
            ::SendMessageW(hwnd, WM_UNDO, 0, 0);
        } else {
            if (xg_ubUndoBuffer.CanUndo()) {
                xg_ubUndoBuffer.Undo();
                if (::IsWindow(xg_hHintsWnd)) {
                    XgSetHintsData();
                }
                // �C���[�W���X�V����B
                XgUpdateImage(hwnd, 0, 0);
            } else {
                ::MessageBeep(0xFFFFFFFF);
            }
        }
        break;

    case ID_REDO:   // ��蒼���B
        if (::GetForegroundWindow() != xg_hMainWnd) {
            ;
        } else {
            if (xg_ubUndoBuffer.CanRedo()) {
                xg_ubUndoBuffer.Redo();
                if (::IsWindow(xg_hHintsWnd)) {
                    XgSetHintsData();
                }
                // �C���[�W���X�V����B
                XgUpdateImage(hwnd, 0, 0);
            } else {
                ::MessageBeep(0xFFFFFFFF);
            }
        }
        break;

    case ID_GENERATEBLACKS: // ���}�X�p�^�[���𐶐��B
        {
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            if (XgOnGenerateBlacks(hwnd, false)) {
                sa2->Get();
                // ���ɖ߂�����ݒ肷��B
                xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
            }
            // �c�[���o�[��UI���X�V����B
            XgUpdateToolBarUI(hwnd);
        }
        break;

    case ID_GENERATEBLACKSSYMMETRIC2: // ���}�X�p�^�[���𐶐��i�_�Ώ́j�B
        {
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            if (XgOnGenerateBlacks(hwnd, true)) {
                sa2->Get();
                // ���ɖ߂�����ݒ肷��B
                xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
            }
            // �c�[���o�[��UI���X�V����B
            XgUpdateToolBarUI(hwnd);
        }
        break;

    case ID_SOLVE:  // �������߂�B
        // ���[���u���}�X���Ώ́v�ł͍��}�X�ǉ�����̉������߂邱�Ƃ͂ł��܂���B
        if (xg_nRules & RULE_POINTSYMMETRY) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTSOLVESYMMETRY), NULL, MB_ICONERROR);
            return;
        }
        {
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            {
                // �������߂�B
                XgOnSolve_AddBlack(hwnd);
            }
            sa2->Get();
            // ���ɖ߂�����ݒ肷��B
            xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
            // �c�[���o�[��UI���X�V����B
            XgUpdateToolBarUI(hwnd);
        }
        break;

    case ID_SOLVENOADDBLACK:    // �������߂�i���}�X�ǉ��Ȃ��j�B
        {
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            {
                // �������߂�i���}�X�ǉ��Ȃ��j�B
                XgOnSolve_NoAddBlack(hwnd);
            }
            sa2->Get();
            // ���ɖ߂�����ݒ肷��B
            xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
            // �c�[���o�[��UI���X�V����B
            XgUpdateToolBarUI(hwnd);
        }
        break;

    case ID_SOLVEREPEATEDLY:    // �A���ŉ������߂�
        // ���[���u���}�X���Ώ́v�ł͍��}�X�ǉ�����̉������߂邱�Ƃ͂ł��܂���B
        if (xg_nRules & RULE_POINTSYMMETRY) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTSOLVESYMMETRY), NULL, MB_ICONERROR);
            return;
        }
        {
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �q���g�E�B���h�E��j������B
            XgDestroyHintsWnd();
            // �A���ŉ������߂�B
            if (XgOnSolveRepeatedly(hwnd)) {
                sa1->Apply();
            }
        }
        // �C���[�W���X�V����B
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        break;

    case ID_SOLVEREPEATEDLYNOADDBLACK:  // �A���ŉ������߂�(���}�X�ǉ��Ȃ�)
        {
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            // ���E�B���h�E��j������B
            XgDestroyCandsWnd();
            // �q���g�E�B���h�E��j������B
            XgDestroyHintsWnd();
            // �A���ŉ������߂�B
            if (XgOnSolveRepeatedlyNoAddBlack(hwnd)) {
                sa1->Apply();
            }
        }
        // �C���[�W���X�V����B
        xg_caret_pos.clear();
        XgMarkUpdate();
        XgUpdateImage(hwnd, 0, 0);
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        break;

    case ID_SHOWSOLUTION:   // ����\������B
        xg_bShowAnswer = true;
        XgMarkUpdate();
        // �C���[�W���X�V����B
        XgUpdateImage(hwnd, 0, 0);
        break;

    case ID_NOSOLUTION: // ����\�����Ȃ��B
        xg_bShowAnswer = false;
        XgMarkUpdate();
        // �C���[�W���X�V����B
        XgUpdateImage(hwnd, 0, 0);
        break;

    case ID_SHOWHIDESOLUTION:   // ���̕\����؂�ւ���B
        xg_bShowAnswer = !xg_bShowAnswer;
        XgMarkUpdate();
        // �C���[�W���X�V����B
        XgUpdateImage(hwnd, 0, 0);
        break;

    case ID_OPENREADME:     // ReadMe���J���B
        XgOpenReadMe(hwnd);
        break;

    case ID_OPENLICENSE:    // License���J���B
        XgOpenLicense(hwnd);
        break;

    case ID_OPENPATTERNS:    // �p�^�[�����J���B
        XgOpenPatterns(hwnd);
        break;

    case ID_ABOUT:      // �o�[�W�������B
        XgOnAbout(hwnd);
        break;

    case ID_PANENEXT:   // ���̃y�[���B
        {
            HWND ahwnd[] = {
                xg_hMainWnd,
                xg_hHintsWnd,
                xg_hCandsWnd,
                xg_hwndInputPalette,
            };

            size_t i = 0, k, m, count = ARRAYSIZE(ahwnd);
            for (i = 0; i < count; ++i) {
                if (ahwnd[i] == ::GetForegroundWindow()) {
                    for (k = 1; k < count; ++k) {
                        m = (i + k) % count;
                        if (::IsWindow(ahwnd[m])) {
                            ::SetForegroundWindow(ahwnd[m]);
                            break;
                        }
                    }
                    break;
                }
            }
        }
        break;

    case ID_PANEPREV:   // �O�̃y�[���B
        {
            HWND ahwnd[] = {
                xg_hwndInputPalette,
                xg_hCandsWnd,
                xg_hHintsWnd,
                xg_hMainWnd,
            };

            size_t i = 0, k, m, count = ARRAYSIZE(ahwnd);
            for (i = 0; i < count; ++i) {
                if (ahwnd[i] == ::GetForegroundWindow()) {
                    for (k = 1; k < count; ++k) {
                        m = (i + k) % count;
                        if (::IsWindow(ahwnd[m])) {
                            ::SetForegroundWindow(ahwnd[m]);
                            break;
                        }
                    }
                    break;
                }
            }
        }
        break;

    case ID_CUT:    // �؂���B
        if (::GetForegroundWindow() == xg_hMainWnd) {
            ;
        } else {
            HWND hwnd = ::GetFocus();
            ::SendMessageW(hwnd, WM_CUT, 0, 0);
        }
        break;

    case ID_COPY:   // �R�s�[�B
        if (::GetForegroundWindow() == xg_hMainWnd) {
            XgCopyBoard(hwnd);
        } else {
            HWND hwnd = ::GetFocus();
            ::SendMessageW(hwnd, WM_COPY, 0, 0);
        }
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        break;

    case ID_COPYASIMAGE:    // �摜���R�s�[�B
        if (::GetForegroundWindow() == xg_hMainWnd) {
            XgCopyBoardAsImage(hwnd);
        }
        break;

    case ID_COPYASIMAGESIZED:   // �T�C�Y���w�肵�ĉ摜���R�s�[�B
        if (::GetForegroundWindow() == xg_hMainWnd) {
            XgCopyBoardAsImageSized(hwnd);
        }
        break;

    case ID_COPYMARKWORDASIMAGE: // ��d�}�X�P��̉摜���R�s�[�B
        XgCopyMarkWordAsImage(hwnd);
        break;

    case ID_COPYMARKWORDASIMAGESIZED:   // �������w�肵�ē�d�}�X�P��̉摜���R�s�[�B
        XgCopyMarkWordAsImageSized(hwnd);
        break;

    case ID_PASTE:  // �\��t���B
        if (::GetForegroundWindow() == xg_hMainWnd) {
            std::wstring str = XgGetClipboardUnicodeText(hwnd);
            if (str.find(ZEN_ULEFT) != std::wstring::npos &&
                str.find(ZEN_LRIGHT) != std::wstring::npos)
            {
                auto sa1 = std::make_shared<XG_UndoData_SetAll>();
                auto sa2 = std::make_shared<XG_UndoData_SetAll>();
                sa1->Get();
                {
                    // �Ղ̓\��t���B
                    XgPasteBoard(hwnd, str);
                }
                sa2->Get();
                // ���ɖ߂�����ݒ肷��B
                xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
            } else {
                // �P��̓\��t���B
                for (auto& ch : str) {
                    MainWnd_OnImeChar(hwnd, ch, 0);
                }
            }
        } else {
            HWND hwnd = ::GetFocus();
            ::SendMessageW(hwnd, WM_PASTE, 0, 0);
        }
        // �c�[���o�[��UI���X�V����B
        XgUpdateToolBarUI(hwnd);
        break;

    case ID_KANAINPUT:  // �J�i���̓��[�h�B
        XgSetInputMode(hwnd, xg_im_KANA);
        break;

    case ID_ABCINPUT:   // �p�����̓��[�h�B
        XgSetInputMode(hwnd, xg_im_ABC);
        break;

    case ID_KANJIINPUT: // �������̓��[�h�B
        XgSetInputMode(hwnd, xg_im_KANJI);
        break;

    case ID_RUSSIAINPUT: // ���V�A���̓��[�h�B
        XgSetInputMode(hwnd, xg_im_RUSSIA);
        break;

    case ID_DIGITINPUT: // �������̓��[�h�B
        XgSetInputMode(hwnd, xg_im_DIGITS);
        break;

    case ID_SHOWHIDEHINTS:
        if (IsWindow(xg_hHintsWnd)) {
            ::DestroyWindow(xg_hHintsWnd);
            xg_hHintsWnd = NULL;
        } else {
            XgShowHints(hwnd);
            ::SetForegroundWindow(xg_hHintsWnd);
        }
        break;

    case ID_MARKSNEXT:  // ���̓�d�}�X�P��
        {
            auto mu1 = std::make_shared<XG_UndoData_MarksUpdated>();
            auto mu2 = std::make_shared<XG_UndoData_MarksUpdated>();
            mu1->Get();
            {
                XgGetNextMarkedWord();
            }
            mu2->Get();
            // ���ɖ߂�����ݒ肷��B
            xg_ubUndoBuffer.Commit(UC_MARKS_UPDATED, mu1, mu2);
        }
        break;

    case ID_MARKSPREV:  // �O�̓�d�}�X�P��
        {
            auto mu1 = std::make_shared<XG_UndoData_MarksUpdated>();
            auto mu2 = std::make_shared<XG_UndoData_MarksUpdated>();
            mu1->Get();
            {
                XgGetPrevMarkedWord();
            }
            mu2->Get();
            // ���ɖ߂�����ݒ肷��B
            xg_ubUndoBuffer.Commit(UC_MARKS_UPDATED, mu1, mu2);
        }
        break;

    case ID_KILLMARKS:  // ��d�}�X�����ׂĉ�������
        {
            auto mu1 = std::make_shared<XG_UndoData_MarksUpdated>();
            auto mu2 = std::make_shared<XG_UndoData_MarksUpdated>();
            mu1->Get();
            {
                xg_vMarks.clear();
                xg_vMarkedCands.clear();
                XgMarkUpdate();
                // �C���[�W���X�V����B
                XgUpdateImage(hwnd, 0, 0);
            }
            mu2->Get();
            // ���ɖ߂�����ݒ肷��B
            xg_ubUndoBuffer.Commit(UC_MARKS_UPDATED, mu1, mu2);
        }
        break;

    case ID_PRINTPROBLEM:   // ���݂̂��������B
        XgPrintProblem();
        break;

    case ID_PRINTANSWER:    // ���Ɖ𓚂��������B
        if (xg_bSolved)
            XgPrintAnswer();
        else
            ::MessageBeep(0xFFFFFFFF);
        break;

    case ID_OPENHOMEPAGE:   // �z�[���y�[�W���J���B
        {
            static LPCWSTR s_pszHomepage = L"http://katahiromz.web.fc2.com/";
            ::ShellExecuteW(hwnd, NULL, s_pszHomepage, NULL, NULL, SW_SHOWNORMAL);
        }
        break;

    case ID_OPENBBS:        // �f�����J���B
        {
            static LPCWSTR s_pszBBS = L"http://katahiromz.bbs.fc2.com/";
            ::ShellExecuteW(hwnd, NULL, s_pszBBS, NULL, NULL, SW_SHOWNORMAL);
        }
        break;

    case ID_CHARFEED:
        XgSetCharFeed(hwnd, -1);
        break;

    case ID_INPUTH:
        XgInputDirection(hwnd, 0);
        break;

    case ID_INPUTV:
        XgInputDirection(hwnd, 1);
        break;

    case ID_STATUS:
        s_bShowStatusBar = !s_bShowStatusBar;
        if (s_bShowStatusBar)
            ShowWindow(xg_hStatusBar, SW_SHOWNOACTIVATE);
        else
            ShowWindow(xg_hStatusBar, SW_HIDE);
        {
            int x = XgGetHScrollPos();
            int y = XgGetVScrollPos();
            XgUpdateScrollInfo(hwnd, x, y);
        }
        PostMessageW(hwnd, WM_SIZE, 0, 0);
        break;
    case ID_PALETTE:
    case ID_PALETTE2:
        if (xg_hwndInputPalette) {
            XgDestroyInputPalette();
        } else {
            XgCreateInputPalette(hwnd);
        }
        break;
    case ID_VIEW50PER:
        break;
    case ID_VIEW75PER:
        break;
    case ID_VIEW100PER:
        break;
    case ID_VIEW150PER:
        break;
    case ID_VIEW200PER:
        break;
    case ID_BACK:
        XgCharBack(hwnd);
        break;
    case ID_INPUTHV:
        XgInputDirection(hwnd, -1);
        break;
    case ID_RETURN:
        XgReturn(hwnd);
        break;
    case ID_BLOCK:
        PostMessageW(hwnd, WM_CHAR, L'#', 0);
        break;
    case ID_SPACE:
        PostMessageW(hwnd, WM_CHAR, L'_', 0);
        break;
    case ID_CLEARNONBLOCKS:
        XgClearNonBlocks(hwnd);
        break;
    case ID_ONLINEDICT:
        XgOnlineDict(hwnd, xg_bTateInput);
        break;
    case ID_ONLINEDICTV:
        XgOnlineDict(hwnd, TRUE);
        break;
    case ID_ONLINEDICTH:
        XgOnlineDict(hwnd, FALSE);
        break;
    case ID_TOGGLEMARK:
        XgToggleMark(hwnd);
        break;
    case ID_TOOLBAR:
        s_bShowToolBar = !s_bShowToolBar;
        if (s_bShowToolBar) {
            ::ShowWindow(xg_hToolBar, SW_SHOWNOACTIVATE);
        } else {
            ::ShowWindow(xg_hToolBar, SW_HIDE);
        }
        PostMessageW(hwnd, WM_SIZE, 0, 0);
        break;
    case ID_HELPDICTSITE:
        ShellExecuteW(hwnd, NULL, XgLoadStringDx1(IDS_MATRIXSEARCH), NULL, NULL, SW_SHOWNORMAL);
        break;
    case ID_BLOCKNOFEED:
        {
            bool bOldFeed = xg_bCharFeed;
            xg_bCharFeed = false;
            SendMessageW(hwnd, WM_CHAR, L'#', 0);
            xg_bCharFeed = bOldFeed;
        }
        break;
    case ID_SPACENOFEED:
        {
            bool bOldFeed = xg_bCharFeed;
            xg_bCharFeed = false;
            SendMessageW(hwnd, WM_CHAR, L'_', 0);
            xg_bCharFeed = bOldFeed;
        }
        break;
    case ID_SHOWHIDENUMBERING:
        xg_bShowNumbering = !xg_bShowNumbering;
        x = XgGetHScrollPos();
        y = XgGetVScrollPos();
        XgUpdateImage(hwnd, x, y);
        break;
    case ID_SHOWHIDECARET:
        xg_bShowCaret = !xg_bShowCaret;
        x = XgGetHScrollPos();
        y = XgGetVScrollPos();
        XgUpdateImage(hwnd, x, y);
        break;
    case ID_ZOOMIN:
        if (xg_nZoomRate < 50) {
            xg_nZoomRate += 5;
        } else if (xg_nZoomRate < 100) {
            xg_nZoomRate += 10;
        } else if (xg_nZoomRate < 200) {
            xg_nZoomRate += 20;
        }
        x = XgGetHScrollPos();
        y = XgGetVScrollPos();
        XgUpdateScrollInfo(hwnd, x, y);
        XgUpdateImage(hwnd, x, y);
        break;
    case ID_ZOOMOUT:
        if (xg_nZoomRate > 200) {
            xg_nZoomRate -= 20;
        } else if (xg_nZoomRate > 100) {
            xg_nZoomRate -= 10;
        } else if (xg_nZoomRate > 50) {
            xg_nZoomRate -= 5;
        }
        x = XgGetHScrollPos();
        y = XgGetVScrollPos();
        XgUpdateScrollInfo(hwnd, x, y);
        XgUpdateImage(hwnd, x, y);
        break;
    case ID_ZOOM100:
        XgSetZoomRate(hwnd, 100);
        break;
    case ID_ZOOM30:
        XgSetZoomRate(hwnd, 30);
        break;
    case ID_ZOOM50:
        XgSetZoomRate(hwnd, 50);
        break;
    case ID_ZOOM65:
        XgSetZoomRate(hwnd, 65);
        break;
    case ID_ZOOM80:
        XgSetZoomRate(hwnd, 80);
        break;
    case ID_ZOOM90:
        XgSetZoomRate(hwnd, 90);
        break;
    case ID_COPYWORDHORZ:
        MainWnd_OnCopyPatternHorz(hwnd);
        break;
    case ID_COPYWORDVERT:
        MainWnd_OnCopyPatternVert(hwnd);
        break;
    case ID_UPPERCASE:
        xg_bLowercase = FALSE;
        x = XgGetHScrollPos();
        y = XgGetVScrollPos();
        XgUpdateImage(hwnd, x, y);
        if (xg_hwndInputPalette) {
            XgCreateInputPalette(hwnd);
        }
        break;
    case ID_LOWERCASE:
        xg_bLowercase = TRUE;
        x = XgGetHScrollPos();
        y = XgGetVScrollPos();
        XgUpdateImage(hwnd, x, y);
        if (xg_hwndInputPalette) {
            XgCreateInputPalette(hwnd);
        }
        break;
    case ID_HIRAGANA:
        xg_bHiragana = TRUE;
        x = XgGetHScrollPos();
        y = XgGetVScrollPos();
        XgUpdateImage(hwnd, x, y);
        if (xg_hwndInputPalette) {
            XgCreateInputPalette(hwnd);
        }
        break;
    case ID_KATAKANA:
        xg_bHiragana = FALSE;
        x = XgGetHScrollPos();
        y = XgGetVScrollPos();
        XgUpdateImage(hwnd, x, y);
        if (xg_hwndInputPalette) {
            XgCreateInputPalette(hwnd);
        }
        break;
    case ID_DICTIONARY00:
    case ID_DICTIONARY01:
    case ID_DICTIONARY02:
    case ID_DICTIONARY03:
    case ID_DICTIONARY04:
    case ID_DICTIONARY05:
    case ID_DICTIONARY06:
    case ID_DICTIONARY07:
    case ID_DICTIONARY08:
    case ID_DICTIONARY09:
    case ID_DICTIONARY10:
    case ID_DICTIONARY11:
    case ID_DICTIONARY12:
    case ID_DICTIONARY13:
    case ID_DICTIONARY14:
    case ID_DICTIONARY15:
        MainWnd_DoDictionary(hwnd, id - ID_DICTIONARY00);
        XgResetTheme(hwnd, FALSE);
        break;
    case ID_RESETRULES:
        if (XgIsUserJapanese())
            xg_nRules = DEFAULT_RULES_JAPANESE;
        else
            xg_nRules = DEFAULT_RULES_ENGLISH;
        xg_bSkeltonMode = FALSE;
        XgUpdateRules(hwnd);
        break;
    case ID_OPENRULESTXT:
        OnOpenRulesTxt(hwnd);
        break;
    case ID_RULE_DONTDOUBLEBLACK:
        if (xg_nRules & RULE_DONTDOUBLEBLACK) {
            xg_nRules &= ~RULE_DONTDOUBLEBLACK;
        } else {
            xg_nRules |= RULE_DONTDOUBLEBLACK;
            if (xg_bSkeltonMode) {
                xg_bSkeltonMode = FALSE;
            }
        }
        XgUpdateRules(hwnd);
        break;
    case ID_RULE_DONTCORNERBLACK:
        if (xg_nRules & RULE_DONTCORNERBLACK) {
            xg_nRules &= ~RULE_DONTCORNERBLACK;
        } else {
            xg_nRules |= RULE_DONTCORNERBLACK;
            if (xg_bSkeltonMode) {
                xg_bSkeltonMode = FALSE;
            }
        }
        XgUpdateRules(hwnd);
        break;
    case ID_RULE_DONTTRIDIRECTIONS:
        if (xg_nRules & RULE_DONTTRIDIRECTIONS) {
            xg_nRules &= ~RULE_DONTTRIDIRECTIONS;
        } else {
            xg_nRules |= RULE_DONTTRIDIRECTIONS;
            if (xg_bSkeltonMode) {
                xg_bSkeltonMode = FALSE;
            }
        }
        XgUpdateRules(hwnd);
        break;
    case ID_RULE_DONTDIVIDE:
        if (xg_nRules & RULE_DONTDIVIDE) {
            xg_nRules &= ~RULE_DONTDIVIDE;
        } else {
            xg_nRules |= RULE_DONTDIVIDE;
        }
        XgUpdateRules(hwnd);
        break;
    case ID_RULE_DONTTHREEDIAGONALS:
        if ((xg_nRules & RULE_DONTTHREEDIAGONALS) && (xg_nRules & RULE_DONTFOURDIAGONALS)) {
            xg_nRules &= ~(RULE_DONTTHREEDIAGONALS | RULE_DONTFOURDIAGONALS);
        } else if (!(xg_nRules & RULE_DONTTHREEDIAGONALS) && (xg_nRules & RULE_DONTFOURDIAGONALS)) {
            xg_nRules |= RULE_DONTTHREEDIAGONALS | RULE_DONTFOURDIAGONALS;
            xg_bSkeltonMode = FALSE;
        } else if ((xg_nRules & RULE_DONTTHREEDIAGONALS) && !(xg_nRules & RULE_DONTFOURDIAGONALS)) {
            xg_nRules &= ~RULE_DONTTHREEDIAGONALS;
        } else {
            xg_nRules |= (RULE_DONTTHREEDIAGONALS | RULE_DONTFOURDIAGONALS);
            xg_bSkeltonMode = FALSE;
        }
        XgUpdateRules(hwnd);
        break;
    case ID_RULE_DONTFOURDIAGONALS:
        if ((xg_nRules & RULE_DONTTHREEDIAGONALS) && (xg_nRules & RULE_DONTFOURDIAGONALS)) {
            xg_nRules &= ~RULE_DONTTHREEDIAGONALS;
            xg_nRules |= RULE_DONTFOURDIAGONALS;
            xg_bSkeltonMode = FALSE;
        } else if (!(xg_nRules & RULE_DONTTHREEDIAGONALS) && (xg_nRules & RULE_DONTFOURDIAGONALS)) {
            xg_nRules &= ~(RULE_DONTTHREEDIAGONALS | RULE_DONTFOURDIAGONALS);
        } else if ((xg_nRules & RULE_DONTTHREEDIAGONALS) && !(xg_nRules & RULE_DONTFOURDIAGONALS)) {
            xg_nRules &= ~RULE_DONTFOURDIAGONALS;
        } else {
            xg_nRules |= RULE_DONTFOURDIAGONALS;
            xg_bSkeltonMode = FALSE;
        }
        XgUpdateRules(hwnd);
        break;
    case ID_RULE_POINTSYMMETRY:
        if (xg_nRules & RULE_POINTSYMMETRY) {
            xg_nRules &= ~RULE_POINTSYMMETRY;
        } else {
            xg_nRules |= RULE_POINTSYMMETRY;
        }
        XgUpdateRules(hwnd);
        break;
    case ID_RULECHECK:
        XgRuleCheck(hwnd);
        break;
    case ID_THEME:
        XgTheme(hwnd);
        break;
    case ID_RESETTHEME:
        XgResetTheme(hwnd, TRUE);
        break;
    case ID_RETRYIFTIMEOUT:
        s_bAutoRetry = !s_bAutoRetry;
        break;
    case ID_SKELTONMODE:
        if (xg_bSkeltonMode) {
            xg_bSkeltonMode = FALSE;
            xg_nRules |= (RULE_DONTDOUBLEBLACK | RULE_DONTCORNERBLACK | RULE_DONTTRIDIRECTIONS | RULE_DONTFOURDIAGONALS);
        } else {
            xg_bSkeltonMode = TRUE;
            xg_nRules &= ~(RULE_DONTDOUBLEBLACK | RULE_DONTCORNERBLACK | RULE_DONTTRIDIRECTIONS | RULE_DONTFOURDIAGONALS | RULE_DONTTHREEDIAGONALS);
        }
        XgUpdateRules(hwnd);
        break;
    case ID_SEQPATGEN:
        {
            auto sa1 = std::make_shared<XG_UndoData_SetAll>();
            auto sa2 = std::make_shared<XG_UndoData_SetAll>();
            sa1->Get();
            if (XgOnGenerateBlacksRepeatedly(hwnd)) {
                // �N���A����B
                xg_bShowAnswer = false;
                xg_bSolved = false;
                xg_xword.clear();
                xg_solution.clear();
                // ���ɖ߂������c���B
                sa2->Get();
                xg_ubUndoBuffer.Commit(UC_SETALL, sa1, sa2);
            }
            // �c�[���o�[��UI���X�V����B
            XgUpdateToolBarUI(hwnd);
        }
        break;
    case ID_FILLBYBLOCKS:
        XgNewCells(hwnd, ZEN_BLACK, xg_nRows, xg_nCols);
        break;
    case ID_FILLBYWHITES:
        XgNewCells(hwnd, ZEN_SPACE, xg_nRows, xg_nCols);
        break;
    case ID_ERASESOLUTIONANDUNLOCKEDIT:
        {
            std::wstring str;
            XG_Board *pxw = (xg_bSolved && xg_bShowAnswer) ? &xg_solution : &xg_xword;
            pxw->GetString(str);
            XgPasteBoard(hwnd, str);
        }
        break;
    case ID_RELOADDICTS:
        XgLoadDictsAll();
        break;
    default:
        if (!MainWnd_OnCommand2(hwnd, id)) {
            ::MessageBeep(0xFFFFFFFF);
        }
        break;
    }

    XgUpdateStatusBar(hwnd);
}

// �t�@�C�����h���b�v���ꂽ�B
void __fastcall MainWnd_OnDropFiles(HWND hwnd, HDROP hDrop)
{
    // �ŏ��̃t�@�C���̃p�X�����擾����B
    WCHAR szFile[MAX_PATH], szTarget[MAX_PATH];
    ::DragQueryFileW(hDrop, 0, szFile, ARRAYSIZE(szFile));
    ::DragFinish(hDrop);

    // �V���[�g�J�b�g�������ꍇ�́A�^�[�Q�b�g�̃p�X���擾����B
    if (XgGetPathOfShortcutW(szFile, szTarget))
        StringCbCopy(szFile, sizeof(szFile), szTarget);

    // �g���q���擾����B
    LPWSTR pch = PathFindExtensionW(szFile);

    if (::lstrcmpiW(pch, L".xwd") == 0) {
        // �g���q��.xwd�������B�t�@�C�����J���B
        if (!XgDoLoadFile(hwnd, szFile, false)) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTLOAD), nullptr, MB_ICONERROR);
        } else {
            xg_caret_pos.clear();
            // �e�[�}���X�V����B
            XgSetThemeString(xg_strTheme);
            XgUpdateTheme(hwnd);
            // �C���[�W���X�V����B
            XgMarkUpdate();
            XgUpdateImage(hwnd, 0, 0);
        }
    } else if (::lstrcmpiW(pch, L".xwj") == 0 || lstrcmpiW(pch, L".json") == 0) {
        // �g���q��.xwj��.json�������B�t�@�C�����J���B
        if (!XgDoLoadFile(hwnd, szFile, true)) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTLOAD), nullptr, MB_ICONERROR);
        } else {
            xg_caret_pos.clear();
            // �e�[�}���X�V����B
            XgSetThemeString(xg_strTheme);
            XgUpdateTheme(hwnd);
            // �C���[�W���X�V����B
            XgMarkUpdate();
            XgUpdateImage(hwnd, 0, 0);
        }
    } else if (::lstrcmpiW(pch, L".crp") == 0 || ::lstrcmpiW(pch, L".crx") == 0) {
        if (!XgDoLoadCrpFile(hwnd, szFile)) {
            XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTLOAD), nullptr, MB_ICONERROR);
        } else {
            xg_caret_pos.clear();
            // �e�[�}���X�V����B
            XgSetThemeString(xg_strTheme);
            XgUpdateTheme(hwnd);
            // �C���[�W���X�V����B
            XgMarkUpdate();
            XgUpdateImage(hwnd, 0, 0);
        }
    } else {
        ::MessageBeep(0xFFFFFFFF);
    }

    // �c�[���o�[��UI���X�V����B
    XgUpdateToolBarUI(hwnd);
    // ���[�����X�V����B
    XgUpdateRules(hwnd);
}

// ������Ԃ̃r�b�g�}�b�v���쐬����B
HBITMAP XgCreateGrayedBitmap(HBITMAP hbm, COLORREF crMask = CLR_INVALID)
{
    HDC hdc = ::GetDC(NULL);
    if (::GetDeviceCaps(hdc, BITSPIXEL) < 24) {
        HPALETTE hPal = reinterpret_cast<HPALETTE>(::GetCurrentObject(hdc, OBJ_PAL));
        UINT index = ::GetNearestPaletteIndex(hPal, crMask);
        if (index != CLR_INVALID)
            crMask = PALETTEINDEX(index);
    }
    ::ReleaseDC(NULL, hdc);

    BITMAP bm;
    ::GetObject(hbm, sizeof(bm), &bm);

    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = bm.bmWidth;
    bi.bmiHeader.biHeight = bm.bmHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    RECT rc;
    rc.left = 0;
    rc.top = 0;
    rc.right = bm.bmWidth;
    rc.bottom = bm.bmHeight;

    HDC hdc1 = ::CreateCompatibleDC(NULL);
    HDC hdc2 = ::CreateCompatibleDC(NULL);

    LPVOID pvBits;
    HBITMAP hbmNew = ::CreateDIBSection(
        NULL, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    assert(hbmNew);
    if (hbmNew) {
        HGDIOBJ hbm1Old = ::SelectObject(hdc1, hbm);
        HGDIOBJ hbm2Old = ::SelectObject(hdc2, hbmNew);
        if (crMask == CLR_INVALID) {
            HBRUSH hbr = ::CreateSolidBrush(crMask);
            ::FillRect(hdc2, &rc, hbr);
            ::DeleteObject(hbr);
        }

        BYTE by;
        COLORREF cr;
        if (crMask == CLR_INVALID) {
            for (int y = 0; y < bm.bmHeight; ++y) {
                for (int x = 0; x < bm.bmWidth; ++x) {
                    cr = ::GetPixel(hdc1, x, y);
                    by = BYTE(
                        95 + (
                            GetRValue(cr) * 3 +
                            GetGValue(cr) * 6 +
                            GetBValue(cr)
                        ) / 20
                    );
                    ::SetPixelV(hdc2, x, y, RGB(by, by, by));
                }
            }
        } else {
            for (int y = 0; y < bm.bmHeight; ++y) {
                for (int x = 0; x < bm.bmWidth; ++x) {
                    cr = ::GetPixel(hdc1, x, y);
                    if (cr != crMask)
                    {
                        by = BYTE(
                            95 + (
                                GetRValue(cr) * 3 +
                                GetGValue(cr) * 6 +
                                GetBValue(cr)
                            ) / 20
                        );
                        ::SetPixelV(hdc2, x, y, RGB(by, by, by));
                    }
                }
            }
        }
        ::SelectObject(hdc1, hbm1Old);
        ::SelectObject(hdc2, hbm2Old);
    }

    ::DeleteDC(hdc1);
    ::DeleteDC(hdc2);

    return hbmNew;
}

// �E�B���h�E�̍쐬�̍ۂɌĂ΂��B
bool __fastcall MainWnd_OnCreate(HWND hwnd, LPCREATESTRUCT /*lpCreateStruct*/)
{
    xg_hMainWnd = hwnd;

    // �����X�N���[���o�[�����B
    xg_hHScrollBar = ::CreateWindowW(
        L"SCROLLBAR", NULL,
        WS_CHILD | WS_VISIBLE | SBS_HORZ,
        0, 0, 0, 0,
        hwnd, NULL, xg_hInstance, NULL);
    if (xg_hHScrollBar == NULL)
        return false;

    // �����X�N���[���o�[�����B
    xg_hVScrollBar = ::CreateWindowW(
        L"SCROLLBAR", NULL,
        WS_CHILD | WS_VISIBLE | SBS_VERT,
        0, 0, 0, 0,
        hwnd, NULL, xg_hInstance, NULL);
    if (xg_hVScrollBar == NULL)
        return false;

    // �T�C�Y�O���b�v�����B
    xg_hSizeGrip = ::CreateWindowW(
        L"SCROLLBAR", NULL,
        WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd, NULL, xg_hInstance, NULL);
    if (xg_hSizeGrip == NULL)
        return false;

    // �C���[�W���X�g�̏���������B
    xg_hImageList = ::ImageList_Create(32, 32, ILC_COLOR24, 0, 0);
    if (xg_hImageList == NULL)
        return FALSE;
    xg_hGrayedImageList = ::ImageList_Create(32, 32, ILC_COLOR24, 0, 0);
    if (xg_hGrayedImageList == NULL)
        return FALSE;

    ::ImageList_SetBkColor(xg_hImageList, CLR_NONE);
    ::ImageList_SetBkColor(xg_hGrayedImageList, CLR_NONE);

    HBITMAP hbm = reinterpret_cast<HBITMAP>(::LoadImageW(
        xg_hInstance, MAKEINTRESOURCE(1),
        IMAGE_BITMAP,
        0, 0,
        LR_COLOR));
    ::ImageList_Add(xg_hImageList, hbm, NULL);
    HBITMAP hbmGrayed = XgCreateGrayedBitmap(hbm);
    ::ImageList_Add(xg_hGrayedImageList, hbmGrayed, NULL);
    ::DeleteObject(hbm);
    ::DeleteObject(hbmGrayed);

    // �c�[���o�[�̃{�^�����B
    static const TBBUTTON atbb[] = {
        {0, ID_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {1, ID_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {2, ID_SAVEAS, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
        {3, ID_GENERATE, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {4, ID_GENERATEANSWER, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {5, ID_GENERATEREPEATEDLY, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
        {6, ID_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {7, ID_PASTE, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
        {8, ID_SOLVE, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {9, ID_SOLVENOADDBLACK, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
        {10, ID_SOLVEREPEATEDLY, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {11, ID_SOLVEREPEATEDLYNOADDBLACK, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
        {12, ID_PRINTPROBLEM, TBSTATE_ENABLED, TBSTYLE_BUTTON},
        {13, ID_PRINTANSWER, TBSTATE_ENABLED, TBSTYLE_BUTTON},
    };

    xg_hStatusBar = ::CreateStatusWindow(WS_CHILD | WS_VISIBLE, L"", hwnd, 256);
    if (xg_hStatusBar == NULL)
        return FALSE;

    XgUpdateStatusBar(hwnd);

    // �c�[���o�[���쐬����B
    const int c_IDW_TOOLBAR = 1;
    xg_hToolBar = ::CreateWindowW(
        TOOLBARCLASSNAMEW, NULL, 
        WS_CHILD | CCS_TOP | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0,
        hwnd,
        reinterpret_cast<HMENU>(UINT_PTR(c_IDW_TOOLBAR)),
        xg_hInstance,
        NULL);
    if (xg_hToolBar == NULL)
        return FALSE;

    // �c�[���o�[������������B
    ::SendMessageW(xg_hToolBar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    ::SendMessageW(xg_hToolBar, TB_SETBITMAPSIZE, 0, MAKELPARAM(32, 32));
    ::SendMessageW(xg_hToolBar, TB_SETIMAGELIST, 0, (LPARAM)xg_hImageList);
    ::SendMessageW(xg_hToolBar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)xg_hGrayedImageList);
    ::SendMessageW(xg_hToolBar, TB_ADDBUTTONS, ARRAYSIZE(atbb), (LPARAM)atbb);
    ::SendMessageW(xg_hToolBar, WM_SIZE, 0, 0);

    if (s_bShowToolBar)
        ::ShowWindow(xg_hToolBar, SW_SHOWNOACTIVATE);
    else
        ::ShowWindow(xg_hToolBar, SW_HIDE);
    
    if (s_bShowStatusBar)
        ::ShowWindow(xg_hStatusBar, SW_SHOWNOACTIVATE);
    else
        ::ShowWindow(xg_hStatusBar, SW_HIDE);

    if (xg_bShowInputPalette)
        XgCreateInputPalette(hwnd);

    // �p�\�R�����Â��ꍇ�A�x����\������B
    if (s_dwNumberOfProcessors <= 1 && !s_bOldNotice) {
        XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_TOOOLDPC), XgLoadStringDx2(IDS_APPNAME),
                          MB_ICONWARNING | MB_OK);
        s_bOldNotice = true;
    }

    // �N���X���[�h������������B
    xg_xword.clear();

    // �����t�@�C����ǂݍ��ށB
    if (xg_dict_name.size())
        XgLoadDictFile(xg_dict_name.c_str());

    // �t�@�C���h���b�v���󂯕t����B
    ::DragAcceptFiles(hwnd, TRUE);

    // �C���[�W���X�V����B
    XgUpdateImage(hwnd, 0, 0);

    int argc;
    LPWSTR *wargv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    if (argc >= 2) {
        WCHAR szFile[MAX_PATH], szTarget[MAX_PATH];
        StringCbCopy(szFile, sizeof(szFile), wargv[1]);

        // �R�}���h���C������������΁A������J���B
        bool bSuccess = true;
        if (::lstrcmpiW(PathFindExtensionW(szFile), s_szShellLinkDotExt) == 0)
        {
            // �V���[�g�J�b�g�������ꍇ�́A�^�[�Q�b�g�̃p�X���擾����B
            if (XgGetPathOfShortcutW(szFile, szTarget)) {
                StringCbCopy(szFile, sizeof(szFile), szTarget);
            } else {
                bSuccess = false;
                MessageBeep(0xFFFFFFFF);
            }
        }
        bool is_json = false;
        if (::lstrcmpiW(PathFindExtensionW(szFile), L".xwj") == 0 ||
            ::lstrcmpiW(PathFindExtensionW(szFile), L".json") == 0)
        {
            is_json = true;
        }
        bool is_builder = false;
        if (::lstrcmpiW(PathFindExtensionW(szFile), L".crp") == 0 ||
            ::lstrcmpiW(PathFindExtensionW(szFile), L".crx") == 0)
        {
            is_builder = true;
        }
        if (bSuccess) {
            if (is_builder) {
                if (!XgDoLoadCrpFile(hwnd, szFile)) {
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTLOAD), nullptr, MB_ICONERROR);
                } else {
                    xg_caret_pos.clear();
                    // �C���[�W���X�V����B
                    XgUpdateImage(hwnd, 0, 0);
                }
            } else {
                if (!XgDoLoadFile(hwnd, szFile, is_json)) {
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTLOAD), nullptr, MB_ICONERROR);
                } else {
                    xg_caret_pos.clear();
                    // �e�[�}���X�V����B
                    XgSetThemeString(xg_strTheme);
                    XgUpdateTheme(hwnd);
                    // �C���[�W���X�V����B
                    XgUpdateImage(hwnd, 0, 0);
                }
            }
            // ���[�����X�V����B
            XgUpdateRules(hwnd);
        }
    }
    GlobalFree(wargv);

    ::PostMessageW(hwnd, WM_SIZE, 0, 0);
    // ���[�����X�V����B
    XgUpdateRules(hwnd);
    // �c�[���o�[��UI���X�V����B
    XgUpdateToolBarUI(hwnd);

    return true;
}

// �}�E�X�z�C�[������]�����B
void __fastcall
MainWnd_OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
    POINT pt = {xPos, yPos};

    RECT rc;
    if (::GetWindowRect(xg_hHintsWnd, &rc) && ::PtInRect(&rc, pt)) {
        FORWARD_WM_MOUSEWHEEL(xg_hHintsWnd, rc.left, rc.top,
            zDelta, fwKeys, ::SendMessageW);
    } else {
        if (::GetAsyncKeyState(VK_CONTROL) < 0) {
            if (zDelta < 0)
                ::PostMessageW(hwnd, WM_COMMAND, ID_ZOOMOUT, 0);
            else if (zDelta > 0)
                ::PostMessageW(hwnd, WM_COMMAND, ID_ZOOMIN, 0);
        } else if (::GetAsyncKeyState(VK_SHIFT) < 0) {
            if (zDelta < 0)
                ::PostMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
            else if (zDelta > 0)
                ::PostMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
        } else {
            if (zDelta < 0)
                ::PostMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
            else if (zDelta > 0)
                ::PostMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
        }
    }
}

// �|�b�v�A�b�v���j���[��ǂݍ��ށB
HMENU XgLoadPopupMenu(HWND hwnd, INT nPos)
{
    HMENU hMenu = LoadMenuW(xg_hInstance, MAKEINTRESOURCE(2));
    HMENU hSubMenu = GetSubMenu(hMenu, nPos);

    // POPUP "�J�i"���C���f�b�N�X5�B
    // POPUP "�p��"���C���f�b�N�X6�B
    // POPUP "���V�A"���C���f�b�N�X7�B
    // POPUP "����"���C���f�b�N�X8�B
    // �傫���C���f�b�N�X�̃��j���[���ڂ���폜�B
    switch (xg_imode)
    {
    case xg_im_ABC:
        DeleteMenu(hSubMenu, 8, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 7, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 5, MF_BYPOSITION);
        break;
    case xg_im_KANA:
        DeleteMenu(hSubMenu, 8, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 7, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 6, MF_BYPOSITION);
        break;
    case xg_im_KANJI:
        DeleteMenu(hSubMenu, 8, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 7, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 6, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 5, MF_BYPOSITION);
        break;
    case xg_im_RUSSIA:
        DeleteMenu(hSubMenu, 8, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 6, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 5, MF_BYPOSITION);
        break;
    case xg_im_DIGITS:
        DeleteMenu(hSubMenu, 7, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 6, MF_BYPOSITION);
        DeleteMenu(hSubMenu, 5, MF_BYPOSITION);
        break;
    default:
        break;
    }

    return hMenu;
}

// �E�N���b�N���ꂽ�B
void
MainWnd_OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    MainWnd_OnLButtonUp(hwnd, x, y, keyFlags);

    HMENU hMenu = XgLoadPopupMenu(hwnd, 0);
    HMENU hSubMenu = GetSubMenu(hMenu, 0);

    // �X�N���[�����W�֕ϊ�����B
    POINT pt;
    pt.x = x;
    pt.y = y;
    ::ClientToScreen(hwnd, &pt);

    // �E�N���b�N���j���[��\������B
    ::SetForegroundWindow(hwnd);
    ::TrackPopupMenu(
        hSubMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN,
        pt.x, pt.y, 0, hwnd, NULL);
    ::PostMessageW(hwnd, WM_NULL, 0, 0);

    ::DestroyMenu(hMenu);
}

// �ʒm�B
void MainWnd_OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    if (pnmh->code == TTN_NEEDTEXT) {
        // �c�[���`�b�v�̏����Z�b�g����B
        LPTOOLTIPTEXT pttt;
        pttt = reinterpret_cast<LPTOOLTIPTEXT>(pnmh);
        pttt->hinst = xg_hInstance;
        pttt->lpszText = MAKEINTRESOURCE(pttt->hdr.idFrom + ID_TT_BASE);
        assert(IDS_TT_NEW == ID_TT_BASE + ID_NEW);
        assert(IDS_TT_GENERATE == ID_TT_BASE + ID_GENERATE);
        assert(IDS_TT_GENERATEANSWER == ID_TT_BASE + ID_GENERATEANSWER);
        assert(IDS_TT_OPEN == ID_TT_BASE + ID_OPEN);
        assert(IDS_TT_SAVEAS == ID_TT_BASE + ID_SAVEAS);
        assert(IDS_TT_SOLVE == ID_TT_BASE + ID_SOLVE);
        assert(IDS_TT_COPY == ID_TT_BASE + ID_COPY);
        assert(IDS_TT_PASTE == ID_TT_BASE + ID_PASTE);
        assert(IDS_TT_PRINTPROBLEM == ID_TT_BASE + ID_PRINTPROBLEM);
        assert(IDS_TT_PRINTANSWER == ID_TT_BASE + ID_PRINTANSWER);
        assert(IDS_TT_SOLVENOADDBLACK == ID_TT_BASE + ID_SOLVENOADDBLACK);
        assert(IDS_TT_GENERATEREPEATEDLY == ID_TT_BASE + ID_GENERATEREPEATEDLY);
        assert(IDS_TT_SOLVEREPEATEDLY == ID_TT_BASE + ID_SOLVEREPEATEDLY);
        assert(IDS_TT_SOLVEREPEATEDLYNOADDBLACK == ID_TT_BASE + ID_SOLVEREPEATEDLYNOADDBLACK);
    }
}

// �E�B���h�E�̃T�C�Y�𐧌�����B
void MainWnd_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
    lpMinMaxInfo->ptMinTrackSize.x = 300;
    lpMinMaxInfo->ptMinTrackSize.y = 150;
}

//////////////////////////////////////////////////////////////////////////////

// �E�B���h�E�v���V�[�W���B
extern "C"
LRESULT CALLBACK
XgWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    HANDLE_MSG(hWnd, WM_CREATE, MainWnd_OnCreate);
    HANDLE_MSG(hWnd, WM_DESTROY, MainWnd_OnDestroy);
    HANDLE_MSG(hWnd, WM_PAINT, MainWnd_OnPaint);
    HANDLE_MSG(hWnd, WM_HSCROLL, MainWnd_OnHScroll);
    HANDLE_MSG(hWnd, WM_VSCROLL, MainWnd_OnVScroll);
    HANDLE_MSG(hWnd, WM_MOVE, MainWnd_OnMove);
    HANDLE_MSG(hWnd, WM_SIZE, MainWnd_OnSize);
    HANDLE_MSG(hWnd, WM_KEYDOWN, MainWnd_OnKey);
    HANDLE_MSG(hWnd, WM_KEYUP, MainWnd_OnKey);
    HANDLE_MSG(hWnd, WM_CHAR, MainWnd_OnChar);
    HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK, MainWnd_OnLButtonDown);
    HANDLE_MSG(hWnd, WM_LBUTTONUP, MainWnd_OnLButtonUp);
    HANDLE_MSG(hWnd, WM_RBUTTONDOWN, MainWnd_OnRButtonDown);
    HANDLE_MSG(hWnd, WM_COMMAND, MainWnd_OnCommand);
    HANDLE_MSG(hWnd, WM_INITMENU, MainWnd_OnInitMenu);
    HANDLE_MSG(hWnd, WM_DROPFILES, MainWnd_OnDropFiles);
    HANDLE_MSG(hWnd, WM_MOUSEWHEEL, MainWnd_OnMouseWheel);
    HANDLE_MSG(hWnd, WM_GETMINMAXINFO, MainWnd_OnGetMinMaxInfo);
    case WM_NOTIFY:
        MainWnd_OnNotify(hWnd, static_cast<int>(wParam), reinterpret_cast<LPNMHDR>(lParam));
        break;

    case WM_IME_CHAR:
        MainWnd_OnImeChar(hWnd, static_cast<WCHAR>(wParam), lParam);
        break;

    default:
        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// �q���g�E�B���h�E�B

// �q���g�E�B���h�E�̃X�N���[���r���[�B
MScrollView         xg_svHintsScrollView;

// �q���g�E�B���h�E��UI�t�H���g�B
HFONT               xg_hHintsUIFont = NULL;

// �c�̃J�M�̃R���g���[���Q�B
HWND                xg_hwndTateCaptionStatic = NULL;
std::vector<HWND>   xg_ahwndTateStatics;
std::vector<HWND>   xg_ahwndTateEdits;

// ���̃J�M�̃R���g���[���Q�B
HWND                xg_hwndYokoCaptionStatic = NULL;
std::vector<HWND>   xg_ahwndYokoStatics;
std::vector<HWND>   xg_ahwndYokoEdits;

// �q���g�E�B���h�E�̃T�C�Y���ς�����B
void HintsWnd_OnSize(HWND hwnd, UINT /*state*/, int /*cx*/, int /*cy*/)
{
    if (xg_hwndTateCaptionStatic == NULL)
        return;

    xg_svHintsScrollView.clear();

    MRect rcClient;
    ::GetClientRect(hwnd, &rcClient);

    MSize size1, size2;
    {
        HDC hdc = ::CreateCompatibleDC(NULL);
        WCHAR label[64];
        StringCbPrintf(label, sizeof(label), XgLoadStringDx1(IDS_DOWNNUMBER), 100);
        std::wstring strLabel = label;
        ::SelectObject(hdc, ::GetStockObject(SYSTEM_FIXED_FONT));
        ::GetTextExtentPoint32W(hdc, strLabel.data(), int(strLabel.size()), &size1);
        ::SelectObject(hdc, xg_hHintsUIFont);
        ::GetTextExtentPoint32W(hdc, strLabel.data(), int(strLabel.size()), &size2);
        ::DeleteDC(hdc);
    }

    WCHAR szText[512];
    HDC hdc = ::CreateCompatibleDC(NULL);
    HGDIOBJ hFontOld = ::SelectObject(hdc, xg_hHintsUIFont);
    int y = 0;

    // �^�e�̃J�M�B
    {
        MRect rcCtrl(MPoint(0, y + 4), 
                     MSize(rcClient.Width(), size1.cy + 4));
        xg_svHintsScrollView.AddCtrlInfo(xg_hwndTateCaptionStatic, rcCtrl);
        y += size1.cy + 8;
    }
    if (rcClient.Width() - size2.cx - 8 > 0) {
        for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
            ::GetWindowTextW(xg_ahwndTateEdits[i], szText,
                             ARRAYSIZE(szText));
            MRect rcCtrl(MPoint(size2.cx, y),
                         MSize(rcClient.Width() - size2.cx - 8, 0));
            if (szText[0] == 0) {
                szText[0] = L' ';
                szText[1] = 0;
            }
            ::DrawTextW(hdc, szText, -1, &rcCtrl,
                DT_LEFT | DT_EDITCONTROL | DT_CALCRECT | DT_WORDBREAK);
            rcCtrl.right = rcClient.right;
            rcCtrl.bottom += 8;
            xg_svHintsScrollView.AddCtrlInfo(xg_ahwndTateEdits[i], rcCtrl);
            rcCtrl.left = 0;
            rcCtrl.right = size2.cx;
            xg_svHintsScrollView.AddCtrlInfo(xg_ahwndTateStatics[i], rcCtrl);
            y += rcCtrl.Height();
        }
    }
    // ���R�̃J�M�B
    {
        MRect rcCtrl(MPoint(0, y + 4),
                     MSize(rcClient.Width(), size1.cy + 4));
        xg_svHintsScrollView.AddCtrlInfo(xg_hwndYokoCaptionStatic, rcCtrl);
        y += size1.cy + 8;
    }
    if (rcClient.Width() - size2.cx - 8 > 0) {
        for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
            ::GetWindowTextW(xg_ahwndYokoEdits[i], szText, ARRAYSIZE(szText));
            MRect rcCtrl(MPoint(size2.cx, y),
                         MSize(rcClient.Width() - size2.cx - 8, 0));
            if (szText[0] == 0) {
                szText[0] = L' ';
                szText[1] = 0;
            }
            ::DrawTextW(hdc, szText, -1, &rcCtrl,
                DT_LEFT | DT_EDITCONTROL | DT_CALCRECT | DT_WORDBREAK);
            rcCtrl.right = rcClient.right;
            rcCtrl.bottom += 8;
            xg_svHintsScrollView.AddCtrlInfo(xg_ahwndYokoEdits[i], rcCtrl);
            rcCtrl.left = 0;
            rcCtrl.right = size2.cx;
            xg_svHintsScrollView.AddCtrlInfo(xg_ahwndYokoStatics[i], rcCtrl);
            y += rcCtrl.Height();
        }
    }

    ::SelectObject(hdc, hFontOld);
    ::DeleteDC(hdc);

    xg_svHintsScrollView.SetExtentForAllCtrls();
    xg_svHintsScrollView.UpdateAll();
}

struct XG_HintEditData
{
    WNDPROC m_fnOldWndProc;
    bool    m_fTate;
};

extern "C"
LRESULT CALLBACK
XgHintEdit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC fn;
    XG_HintEditData *data =
        reinterpret_cast<XG_HintEditData *>(
            ::GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg) {
    case WM_CHAR:
        if (wParam == L'\r' || wParam == L'\n') {
            // ���s�������ꂽ�B�K�v�Ȃ�΃f�[�^���X�V����B
            if (XgAreHintsModified()) {
                auto hu1 = std::make_shared<XG_UndoData_HintsUpdated>();
                auto hu2 = std::make_shared<XG_UndoData_HintsUpdated>();
                hu1->Get();
                {
                    // �q���g���X�V����B
                    XgUpdateHintsData();
                }
                hu2->Get();
                xg_ubUndoBuffer.Commit(UC_HINTS_UPDATED, hu1, hu2);
            }
        }
        return ::CallWindowProc(data->m_fnOldWndProc,
            hwnd, uMsg, wParam, lParam);

    case WM_SETFOCUS: // �t�H�[�J�X�𓾂��B
        if (wParam) {
            // �t�H�[�J�X�������R���g���[���̑I������������B
            HWND hwndLoseFocus = reinterpret_cast<HWND>(wParam);
            ::SendMessageW(hwndLoseFocus, EM_SETSEL, 0, 0);
        }
        // �t�H�[�J�X�̂���R���g���[����������悤�Ɉړ�����B
        xg_svHintsScrollView.EnsureCtrlVisible(hwnd);
        return ::CallWindowProc(data->m_fnOldWndProc,
            hwnd, uMsg, wParam, lParam);

    case WM_KILLFOCUS:  // �t�H�[�J�X���������B
        // �q���g�ɍX�V������΁A�f�[�^���X�V����B
        if (XgAreHintsModified()) {
            auto hu1 = std::make_shared<XG_UndoData_HintsUpdated>();
            auto hu2 = std::make_shared<XG_UndoData_HintsUpdated>();
            hu1->Get();
            {
                // �q���g���X�V����B
                XgUpdateHintsData();
            }
            hu2->Get();
            xg_ubUndoBuffer.Commit(UC_HINTS_UPDATED, hu1, hu2);
        }
        // ���C�A�E�g�𒲐�����B
        ::PostMessageW(xg_hHintsWnd, WM_SIZE, 0, 0);
        return ::CallWindowProc(data->m_fnOldWndProc,
            hwnd, uMsg, wParam, lParam);

    case WM_KEYDOWN:
        if (wParam == VK_RETURN) {
            ::PostMessageW(xg_hHintsWnd, WM_SIZE, 0, 0);
            ::SetFocus(NULL);
            break;
        }

        if (wParam == VK_TAB) {
            HWND hwndNext;
            if (::GetAsyncKeyState(VK_SHIFT) < 0)
                hwndNext = ::GetNextDlgTabItem(xg_hHintsWnd, hwnd, TRUE);
            else
                hwndNext = ::GetNextDlgTabItem(xg_hHintsWnd, hwnd, FALSE);
            ::SendMessageW(hwnd, EM_SETSEL, 0, 0);
            ::SendMessageW(hwndNext, EM_SETSEL, 0, -1);
            ::SetFocus(hwndNext);
            break;
        }

        if (wParam == VK_PRIOR || wParam == VK_NEXT) {
            HWND hwndParent = ::GetParent(hwnd);
            ::SendMessageW(hwndParent, uMsg, wParam, lParam);
            break;
        }

        if (wParam == VK_F6) {
            if (::GetAsyncKeyState(VK_SHIFT) < 0)
                ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANEPREV, 0);
            else
                ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANENEXT, 0);
            break;
        }

        return ::CallWindowProc(data->m_fnOldWndProc,
            hwnd, uMsg, wParam, lParam);

    case WM_NCDESTROY:
        fn = data->m_fnOldWndProc;
        ::LocalFree(data);
        return ::CallWindowProc(fn, hwnd, uMsg, wParam, lParam);

    case WM_MOUSEWHEEL:
        {
            HWND hwndParent = ::GetParent(hwnd);
            ::SendMessageW(hwndParent, uMsg, wParam, lParam);
        }
        break;

    default:
        return ::CallWindowProc(data->m_fnOldWndProc,
            hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// �q���g�E�B���h�E���쐬���ꂽ�B
BOOL HintsWnd_OnCreate(HWND hwnd, LPCREATESTRUCT /*lpCreateStruct*/)
{
    xg_hHintsWnd = hwnd;

    // �������B
    xg_ahwndTateStatics.clear();
    xg_ahwndTateEdits.clear();
    xg_ahwndYokoStatics.clear();
    xg_ahwndYokoEdits.clear();

    xg_svHintsScrollView.SetParent(hwnd);
    xg_svHintsScrollView.ShowScrollBars(FALSE, TRUE);

    if (xg_hHintsUIFont) {
        ::DeleteObject(xg_hHintsUIFont);
    }
    xg_hHintsUIFont = ::CreateFontIndirectW(XgGetUIFont());
    if (xg_hHintsUIFont == NULL) {
        xg_hHintsUIFont =
            reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
    }

    HWND hwndCtrl;

    hwndCtrl = ::CreateWindowW(
        TEXT("STATIC"), XgLoadStringDx1(IDS_DOWN),
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX | SS_NOTIFY |
        SS_CENTER | SS_CENTERIMAGE,
        0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
    if (hwndCtrl == NULL)
        return FALSE;
    xg_hwndTateCaptionStatic = hwndCtrl;
    ::SendMessageW(hwndCtrl, WM_SETFONT,
        reinterpret_cast<WPARAM>(::GetStockObject(SYSTEM_FIXED_FONT)),
        TRUE);

    hwndCtrl = ::CreateWindowW(
        TEXT("STATIC"), XgLoadStringDx1(IDS_ACROSS),
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX | SS_NOTIFY |
        SS_CENTER | SS_CENTERIMAGE,
        0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
    if (hwndCtrl == NULL)
        return FALSE;
    xg_hwndYokoCaptionStatic = hwndCtrl;
    ::SendMessageW(hwndCtrl, WM_SETFONT,
        reinterpret_cast<WPARAM>(::GetStockObject(SYSTEM_FIXED_FONT)),
        TRUE);

    XG_HintEditData *data;
    WCHAR sz[256];
    for (const auto& hint : xg_vecTateHints) {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_DOWNNUMBER), hint.m_number);
        hwndCtrl = ::CreateWindowW(TEXT("STATIC"), sz,
            WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_NOPREFIX | SS_NOTIFY | SS_CENTERIMAGE,
            0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
        assert(hwndCtrl);
        ::SendMessageW(hwndCtrl, WM_SETFONT,
            reinterpret_cast<WPARAM>(xg_hHintsUIFont),
            TRUE);
        if (hwndCtrl == NULL)
            return FALSE;

        xg_ahwndTateStatics.emplace_back(hwndCtrl);

        hwndCtrl = ::CreateWindowExW(
            WS_EX_CLIENTEDGE,
            TEXT("EDIT"), hint.m_strHint.data(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE,
            0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
        assert(hwndCtrl);
        ::SendMessageW(hwndCtrl, WM_SETFONT,
            reinterpret_cast<WPARAM>(xg_hHintsUIFont),
            TRUE);
        if (hwndCtrl == NULL)
            return FALSE;

        data = reinterpret_cast<XG_HintEditData *>(
            ::LocalAlloc(LMEM_FIXED, sizeof(XG_HintEditData)));
        data->m_fTate = true;
        data->m_fnOldWndProc = reinterpret_cast<WNDPROC>(
            ::SetWindowLongPtr(hwndCtrl, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(XgHintEdit_WndProc)));
        ::SetWindowLongPtr(hwndCtrl, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(data));
        xg_ahwndTateEdits.emplace_back(hwndCtrl);
    }
    for (const auto& hint : xg_vecYokoHints) {
        StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_ACROSSNUMBER), hint.m_number);
        hwndCtrl = ::CreateWindowW(TEXT("STATIC"), sz,
            WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_NOPREFIX | SS_NOTIFY | SS_CENTERIMAGE,
            0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
        assert(hwndCtrl);
        ::SendMessageW(hwndCtrl, WM_SETFONT,
            reinterpret_cast<WPARAM>(xg_hHintsUIFont),
            TRUE);
        if (hwndCtrl == NULL)
            return FALSE;

        xg_ahwndYokoStatics.emplace_back(hwndCtrl);

        hwndCtrl = ::CreateWindowExW(
            WS_EX_CLIENTEDGE,
            TEXT("EDIT"), hint.m_strHint.data(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE,
            0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
        assert(hwndCtrl);
        ::SendMessageW(hwndCtrl, WM_SETFONT,
            reinterpret_cast<WPARAM>(xg_hHintsUIFont),
            TRUE);
        if (hwndCtrl == NULL)
            return FALSE;

        data = reinterpret_cast<XG_HintEditData *>(
            ::LocalAlloc(LMEM_FIXED, sizeof(XG_HintEditData)));
        data->m_fTate = false;
        data->m_fnOldWndProc = reinterpret_cast<WNDPROC>(
            ::SetWindowLongPtr(hwndCtrl, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(XgHintEdit_WndProc)));
        ::SetWindowLongPtr(hwndCtrl, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(data));
        xg_ahwndYokoEdits.emplace_back(hwndCtrl);
    }

    if (xg_ahwndTateEdits.size())
        ::SetFocus(xg_ahwndTateEdits[0]);

    ::PostMessageW(hwnd, WM_SIZE, 0, 0);

    return TRUE;
}

// �q���g�E�B���h�E���c�ɃX�N���[�����ꂽ�B
void HintsWnd_OnVScroll(HWND /*hwnd*/, HWND /*hwndCtl*/, UINT code, int pos)
{
    xg_svHintsScrollView.Scroll(SB_VERT, code, pos);
}

// �q���g���ύX���ꂽ���H
bool __fastcall XgAreHintsModified(void)
{
    if (xg_bHintsAdded) {
        return true;
    }

    if (::IsWindow(xg_hHintsWnd)) {
        for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
            if (::SendMessageW(xg_ahwndTateEdits[i], EM_GETMODIFY, 0, 0)) {
                return true;
            }
        }
        for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
            if (::SendMessageW(xg_ahwndYokoEdits[i], EM_GETMODIFY, 0, 0)) {
                return true;
            }
        }
    }
    return false;
}

// �q���g�f�[�^��ݒ肷��B
void __fastcall XgSetHintsData(void)
{
    for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
        ::SetWindowTextW(xg_ahwndTateEdits[i], xg_vecTateHints[i].m_strHint.data());
    }
    for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
        ::SetWindowTextW(xg_ahwndYokoEdits[i], xg_vecYokoHints[i].m_strHint.data());
    }
}

// �q���g�f�[�^���X�V����B
bool __fastcall XgUpdateHintsData(void)
{
    bool updated = false;
    if (::IsWindow(xg_hHintsWnd)) {
        WCHAR sz[512];
        for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
            if (::SendMessageW(xg_ahwndTateEdits[i], EM_GETMODIFY, 0, 0)) {
                updated = true;
                ::GetWindowTextW(xg_ahwndTateEdits[i], sz, 
                                 static_cast<int>(ARRAYSIZE(sz)));
                xg_vecTateHints[i].m_strHint = sz;
                ::SendMessageW(xg_ahwndTateEdits[i], EM_SETMODIFY, FALSE, 0);
            }
        }
        for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
            if (::SendMessageW(xg_ahwndYokoEdits[i], EM_GETMODIFY, 0, 0)) {
                updated = true;
                ::GetWindowTextW(xg_ahwndYokoEdits[i], sz, 
                                 static_cast<int>(ARRAYSIZE(sz)));
                xg_vecYokoHints[i].m_strHint = sz;
                ::SendMessageW(xg_ahwndTateEdits[i], EM_SETMODIFY, FALSE, 0);
            }
        }
    }
    return updated;
}

// �q���g�E�B���h�E���j�����ꂽ�B
void HintsWnd_OnDestroy(HWND hwnd)
{
    if (xg_hHintsWnd) {
        // �q���g�f�[�^���X�V����B
        XgUpdateHintsData();
    }

    // ���݂̈ʒu�ƃT�C�Y��ۑ�����B
    MRect rc;
    ::GetWindowRect(hwnd, &rc);
    s_nHintsWndX = rc.left;
    s_nHintsWndY = rc.top;
    s_nHintsWndCX = rc.Width();
    s_nHintsWndCY = rc.Height();

    xg_hHintsWnd = NULL;
    xg_hwndTateCaptionStatic = NULL;
    xg_hwndYokoCaptionStatic = NULL;
    xg_ahwndTateStatics.clear();
    xg_ahwndTateEdits.clear();
    xg_ahwndYokoStatics.clear();
    xg_ahwndYokoEdits.clear();
    xg_svHintsScrollView.clear();

    ::DeleteObject(xg_hHintsUIFont);
    xg_hHintsUIFont = NULL;
}

void HintsWnd_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (codeNotify == STN_CLICKED) {
        for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
            if (xg_ahwndTateStatics[i] == hwndCtl) {
                ::SendMessageW(xg_ahwndTateEdits[i], EM_SETSEL, 0, -1);
                ::SetFocus(xg_ahwndTateEdits[i]);
                return;
            }
        }
        for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
            if (xg_ahwndYokoStatics[i] == hwndCtl) {
                ::SendMessageW(xg_ahwndYokoEdits[i], EM_SETSEL, 0, -1);
                ::SetFocus(xg_ahwndYokoEdits[i]);
                return;
            }
        }
    }
}

// �L�[�������ꂽ�B
void HintsWnd_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    if (!fDown)
        return;

    switch (vk) {
    case VK_PRIOR:
        ::PostMessageW(hwnd, WM_VSCROLL, MAKELPARAM(SB_PAGEUP, 0), 0);
        break;

    case VK_NEXT:
        ::PostMessageW(hwnd, WM_VSCROLL, MAKELPARAM(SB_PAGEDOWN, 0), 0);
        break;

    case VK_TAB:
        if (xg_ahwndTateEdits.size())
            ::SetFocus(xg_ahwndTateEdits[0]);
        break;

    case VK_F6:
        if (::GetAsyncKeyState(VK_SHIFT) < 0)
            ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANEPREV, 0);
        else
            ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANENEXT, 0);
        break;
    }
}


// �}�E�X�z�C�[������]�����B
void __fastcall
HintsWnd_OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
    if (::GetAsyncKeyState(VK_SHIFT) < 0) {
        if (zDelta < 0)
            ::PostMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
        else if (zDelta > 0)
            ::PostMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
    } else {
        if (zDelta < 0)
            ::PostMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
        else if (zDelta > 0)
            ::PostMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
    }
}

// �q���g �E�B���h�E�̃T�C�Y�𐧌�����B
void HintsWnd_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
    lpMinMaxInfo->ptMinTrackSize.x = 256;
    lpMinMaxInfo->ptMinTrackSize.y = 128;
}

// �u�q���g�v�E�B���h�E�̃R���e�L�X�g���j���[�B
void HintsWnd_OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
{
    WCHAR szClass[8];
    szClass[0] = 0;
    GetClassNameW(hwndContext, szClass, ARRAYSIZE(szClass));
    if (lstrcmpiW(szClass, L"EDIT") == 0)
        return;

    HMENU hMenu = LoadMenuW(xg_hInstance, MAKEINTRESOURCEW(2));
    HMENU hSubMenu = GetSubMenu(hMenu, 1);

    // �E�N���b�N���j���[��\������B
    ::SetForegroundWindow(hwnd);
    INT nCmd = ::TrackPopupMenu(
        hSubMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD,
        xPos, yPos, 0, hwnd, NULL);
    ::PostMessageW(hwnd, WM_NULL, 0, 0);
    if (nCmd)
        ::PostMessageW(xg_hMainWnd, WM_COMMAND, nCmd, 0);

    ::DestroyMenu(hMenu);
}

// �q���g �E�B���h�E�̃E�B���h�E �v���V�[�W���[�B
extern "C"
LRESULT CALLBACK
XgHintsWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    HANDLE_MSG(hWnd, WM_CREATE, HintsWnd_OnCreate);
    HANDLE_MSG(hWnd, WM_SIZE, HintsWnd_OnSize);
    HANDLE_MSG(hWnd, WM_VSCROLL, HintsWnd_OnVScroll);
    HANDLE_MSG(hWnd, WM_KEYDOWN, HintsWnd_OnKey);
    HANDLE_MSG(hWnd, WM_KEYUP, HintsWnd_OnKey);
    HANDLE_MSG(hWnd, WM_DESTROY, HintsWnd_OnDestroy);
    HANDLE_MSG(hWnd, WM_COMMAND, HintsWnd_OnCommand);
    HANDLE_MSG(hWnd, WM_MOUSEWHEEL, HintsWnd_OnMouseWheel);
    HANDLE_MSG(hWnd, WM_GETMINMAXINFO, HintsWnd_OnGetMinMaxInfo);
    HANDLE_MSG(hWnd, WM_CONTEXTMENU, HintsWnd_OnContextMenu);

    default:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

// �q���g�E�B���h�E���쐬����B
BOOL XgCreateHintsWnd(HWND hwnd)
{
    const DWORD style = WS_OVERLAPPED | WS_CAPTION |
        WS_SYSMENU | WS_THICKFRAME | WS_HSCROLL | WS_VSCROLL;
    CreateWindowExW(WS_EX_TOOLWINDOW,
        s_pszHintsWndClass, XgLoadStringDx1(IDS_HINTS), style,
        s_nHintsWndX, s_nHintsWndY, s_nHintsWndCX, s_nHintsWndCY,
        hwnd, nullptr, xg_hInstance, nullptr);
    if (xg_hHintsWnd) {
        ShowWindow(xg_hHintsWnd, SW_SHOWNORMAL);
        UpdateWindow(xg_hHintsWnd);
        return TRUE;
    }
    return FALSE;
}

// �q���g�E�B���h�E��j������B
void XgDestroyHintsWnd(void)
{
    // �q���g�E�B���h�E�����݂��邩�H
    if (xg_hHintsWnd && ::IsWindow(xg_hHintsWnd)) {
        // �X�V�𖳎��E�j������B
        HWND hwnd = xg_hHintsWnd;
        xg_hHintsWnd = NULL;
        ::DestroyWindow(hwnd);
    }
}

//////////////////////////////////////////////////////////////////////////////

// hook for Ctrl+A
HHOOK xg_hCtrlAHook = NULL;

// hook proc for Ctrl+A
LRESULT CALLBACK XgCtrlAMessageProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
        return ::CallNextHookEx(xg_hCtrlAHook, nCode, wParam, lParam);

    MSG *pMsg = reinterpret_cast<MSG *>(lParam);
    WCHAR szClassName[64];

    HWND hWnd;
    if (pMsg->message == WM_KEYDOWN) {
        if (static_cast<int>(pMsg->wParam) == 'A' &&
            ::GetAsyncKeyState(VK_CONTROL) < 0 &&
            ::GetAsyncKeyState(VK_SHIFT) >= 0 &&
            ::GetAsyncKeyState(VK_MENU) >= 0)
        {
            // Ctrl+A is pressed
            hWnd = ::GetFocus();
            if (hWnd) {
                ::GetClassNameW(hWnd, szClassName, 64);
                if (::lstrcmpiW(szClassName, L"EDIT") == 0) {
                    ::SendMessageW(hWnd, EM_SETSEL, 0, -1);
                    return 1;
                }
            }
        }
    }

    return ::CallNextHookEx(xg_hCtrlAHook, nCode, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////
// ���E�B���h�E�B

// ���E�B���h�E�̃X�N���[���r���[�B
MScrollView                 xg_svCandsScrollView;

// ���E�B���h�E��UI�t�H���g�B
HFONT                       xg_hCandsUIFont = NULL;

// ���B
std::vector<std::wstring>   xg_vecCandidates;

// ���{�^���B
std::vector<HWND>           xg_ahwndCandButtons;

// �������߂�ʒu�B
int  xg_jCandPos;
int  xg_iCandPos;

// ���̓^�e�����R���B
bool xg_bCandVertical;

struct XG_CandsButtonData
{
    WNDPROC m_fnOldWndProc;
};

extern "C"
LRESULT CALLBACK
XgCandsButton_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC fn;
    XG_CandsButtonData *data =
        reinterpret_cast<XG_CandsButtonData *>(
            ::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg) {
    case WM_CHAR:
        #if 0
            if (wParam == L'\t' || wParam == L'\r' || wParam == L'\n')
                break;
        #endif
        return ::CallWindowProc(data->m_fnOldWndProc,
            hwnd, uMsg, wParam, lParam);

    case WM_SETFOCUS:
        xg_svCandsScrollView.EnsureCtrlVisible(hwnd);
        return ::CallWindowProc(data->m_fnOldWndProc,
            hwnd, uMsg, wParam, lParam);

    case WM_KEYDOWN:
        if (wParam == VK_RETURN) {
            ::SetFocus(NULL);
            break;
        }

        if (wParam == VK_PRIOR || wParam == VK_NEXT) {
            HWND hwndParent = ::GetParent(hwnd);
            ::SendMessageW(hwndParent, uMsg, wParam, lParam);
            break;
        }

        if (wParam == VK_F6) {
            if (::GetAsyncKeyState(VK_SHIFT) < 0)
                ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANEPREV, 0);
            else
                ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANENEXT, 0);
            break;
        }

        if (wParam == VK_ESCAPE) {
            DestroyWindow(GetParent(hwnd));
            break;
        }

        return ::CallWindowProc(data->m_fnOldWndProc, hwnd, uMsg, wParam, lParam);

    case WM_NCDESTROY:
        fn = data->m_fnOldWndProc;
        ::LocalFree(data);
        return ::CallWindowProc(fn, hwnd, uMsg, wParam, lParam);

    case WM_MOUSEWHEEL:
        {
            HWND hwndParent = ::GetParent(hwnd);
            ::SendMessageW(hwndParent, uMsg, wParam, lParam);
        }
        break;

    default:
        return ::CallWindowProc(data->m_fnOldWndProc,
            hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// ���E�B���h�E�̃T�C�Y���ς�����B
void CandsWnd_OnSize(HWND hwnd, UINT /*state*/, int /*cx*/, int /*cy*/)
{
    if (xg_ahwndCandButtons.empty())
        return;

    xg_svCandsScrollView.clear();

    MRect rcClient;
    ::GetClientRect(hwnd, &rcClient);

    HDC hdc = ::CreateCompatibleDC(NULL);
    HGDIOBJ hFontOld = ::SelectObject(hdc, xg_hCandsUIFont);
    {
        MPoint pt;
        for (size_t i = 0; i < xg_vecCandidates.size(); ++i) {
            const std::wstring& strLabel = xg_vecCandidates[i];

            MSize siz;
            ::GetTextExtentPoint32W(hdc, strLabel.data(),
                                    static_cast<int>(strLabel.size()), &siz);

            if (pt.x != 0 && pt.x + siz.cx + 16 > rcClient.Width()) {
                pt.x = 0;
                pt.y += siz.cy + 16;
            }

            MRect rcCtrl(MPoint(pt.x + 4, pt.y + 4), MSize(siz.cx + 8, siz.cy + 8));
            xg_svCandsScrollView.AddCtrlInfo(xg_ahwndCandButtons[i], rcCtrl);

            pt.x += siz.cx + 16;
        }
    }
    ::SelectObject(hdc, hFontOld);
    ::DeleteDC(hdc);

    xg_svCandsScrollView.SetExtentForAllCtrls();
    xg_svCandsScrollView.Extent().cy += 4;
    xg_svCandsScrollView.EnsureCtrlVisible(::GetFocus(), false);
    xg_svCandsScrollView.UpdateAll();
}

// ���E�B���h�E���쐬���ꂽ�B
BOOL CandsWnd_OnCreate(HWND hwnd, LPCREATESTRUCT /*lpCreateStruct*/)
{
    xg_hCandsWnd = hwnd;

    if (xg_hCandsUIFont) {
        ::DeleteObject(xg_hCandsUIFont);
    }
    xg_hCandsUIFont = ::CreateFontIndirectW(XgGetUIFont());
    if (xg_hCandsUIFont == NULL) {
        xg_hCandsUIFont = reinterpret_cast<HFONT>(
            ::GetStockObject(DEFAULT_GUI_FONT));
    }

    // �������B
    xg_ahwndCandButtons.clear();
    xg_svCandsScrollView.clear();

    xg_svCandsScrollView.SetParent(hwnd);
    xg_svCandsScrollView.ShowScrollBars(FALSE, TRUE);

    HWND hwndCtrl;
    XG_CandsButtonData *data;
    for (size_t i = 0; i < xg_vecCandidates.size(); ++i) {
        WCHAR szText[64];
        if (xg_bHiragana) {
            LCMapStringW(JPN_LOCALE, LCMAP_FULLWIDTH | LCMAP_HIRAGANA,
                         xg_vecCandidates[i].data(), -1, szText, ARRAYSIZE(szText));
            xg_vecCandidates[i] = szText;
        }
        if (xg_bLowercase) {
            LCMapStringW(JPN_LOCALE, LCMAP_FULLWIDTH | LCMAP_LOWERCASE,
                         xg_vecCandidates[i].data(), -1, szText, ARRAYSIZE(szText));
            xg_vecCandidates[i] = szText;
        }

        hwndCtrl = ::CreateWindowW(
            TEXT("BUTTON"), xg_vecCandidates[i].data(),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            0, 0, 0, 0, hwnd, NULL, xg_hInstance, NULL);
        assert(hwndCtrl);
        if (hwndCtrl == NULL)
            return FALSE;

        ::SendMessageW(hwndCtrl, WM_SETFONT,
            reinterpret_cast<WPARAM>(xg_hCandsUIFont),
            FALSE);
        xg_ahwndCandButtons.emplace_back(hwndCtrl);

        data = reinterpret_cast<XG_CandsButtonData *>(
            ::LocalAlloc(LMEM_FIXED, sizeof(XG_CandsButtonData)));
        data->m_fnOldWndProc = reinterpret_cast<WNDPROC>(
            ::SetWindowLongPtr(hwndCtrl, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(XgCandsButton_WndProc)));
        ::SetWindowLongPtr(hwndCtrl, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(data));
    }
    CandsWnd_OnSize(hwnd, 0, 0, 0);

    if (xg_ahwndCandButtons.size())
        ::SetFocus(xg_ahwndCandButtons[0]);
    else
        ::SetFocus(hwnd);

    return TRUE;
}

// ���E�B���h�E���c�ɃX�N���[�����ꂽ�B
void CandsWnd_OnVScroll(HWND /*hwnd*/, HWND /*hwndCtl*/, UINT code, int pos)
{
    xg_svCandsScrollView.Scroll(SB_VERT, code, pos);
}

// �L�[�������ꂽ�B
void CandsWnd_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    if (!fDown)
        return;

    switch (vk) {
    case VK_PRIOR:
        ::PostMessageW(hwnd, WM_VSCROLL, MAKELPARAM(SB_PAGEUP, 0), 0);
        break;

    case VK_NEXT:
        ::PostMessageW(hwnd, WM_VSCROLL, MAKELPARAM(SB_PAGEDOWN, 0), 0);
        break;

    case VK_TAB:
        if (xg_ahwndCandButtons.size())
            ::SetFocus(xg_ahwndCandButtons[0]);
        break;

    case VK_F6:
        if (::GetAsyncKeyState(VK_SHIFT) < 0)
            ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANEPREV, 0);
        else
            ::SendMessageW(xg_hMainWnd, WM_COMMAND, ID_PANENEXT, 0);
        break;

    case VK_ESCAPE:
        DestroyWindow(hwnd);
        break;
    }
}

// ���E�B���h�E���j�����ꂽ�B
void CandsWnd_OnDestroy(HWND hwnd)
{
    // ���݂̈ʒu�ƃT�C�Y��ۑ�����B
    MRect rc;
    ::GetWindowRect(hwnd, &rc);
    s_nCandsWndX = rc.left;
    s_nCandsWndY = rc.top;
    s_nCandsWndCX = rc.Width();
    s_nCandsWndCY = rc.Height();

    xg_hCandsWnd = NULL;
    xg_ahwndCandButtons.clear();
    xg_svCandsScrollView.clear();

    ::DeleteObject(xg_hCandsUIFont);
    xg_hCandsUIFont = NULL;

    SetForegroundWindow(xg_hMainWnd);
}

// ���E�B���h�E��j������B
void XgDestroyCandsWnd(void)
{
    // ���E�B���h�E�����݂��邩�H
    if (xg_hCandsWnd && ::IsWindow(xg_hCandsWnd)) {
        // �X�V�𖳎��E�j������B
        HWND hwnd = xg_hCandsWnd;
        xg_hCandsWnd = NULL;
        ::DestroyWindow(hwnd);
    }
}

// ����K�p����B
void XgApplyCandidate(XG_Board& xword, const std::wstring& strCand)
{
    std::wstring cand = XgNormalizeString(strCand);

    int lo, hi;
    if (xg_bCandVertical) {
        for (lo = xg_iCandPos; lo > 0; --lo) {
            if (xword.GetAt(lo - 1, xg_jCandPos) == ZEN_BLACK)
                break;
        }
        for (hi = xg_iCandPos; hi + 1 < xg_nRows; ++hi) {
            if (xword.GetAt(hi + 1, xg_jCandPos) == ZEN_BLACK)
                break;
        }

        int m = 0;
        for (int k = lo; k <= hi; ++k, ++m) {
            xword.SetAt(k, xg_jCandPos, cand[m]);
        }
    }
    else
    {
        for (lo = xg_jCandPos; lo > 0; --lo) {
            if (xword.GetAt(xg_iCandPos, lo - 1) == ZEN_BLACK)
                break;
        }
        for (hi = xg_jCandPos; hi + 1 < xg_nCols; ++hi) {
            if (xword.GetAt(xg_iCandPos, hi + 1) == ZEN_BLACK)
                break;
        }

        int m = 0;
        for (int k = lo; k <= hi; ++k, ++m) {
            xword.SetAt(xg_iCandPos, k, cand[m]);
        }
    }
}

void CandsWnd_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (codeNotify == BN_CLICKED) {
        for (size_t i = 0; i < xg_ahwndCandButtons.size(); ++i) {
            if (xg_ahwndCandButtons[i] == hwndCtl)
            {
                // ����K�p����B
                XgApplyCandidate(xg_xword, xg_vecCandidates[i]);

                // ���E�B���h�E��j������B
                XgDestroyCandsWnd();

                // �C���[�W���X�V����B
                int x = XgGetHScrollPos();
                int y = XgGetVScrollPos();
                XgUpdateImage(xg_hMainWnd, x, y);
                return;
            }
        }
    }
}

// �}�E�X�z�C�[������]�����B
void __fastcall
CandsWnd_OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
    if (::GetAsyncKeyState(VK_SHIFT) < 0) {
        if (zDelta < 0)
            ::PostMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
        else if (zDelta > 0)
            ::PostMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
    } else {
        if (zDelta < 0)
            ::PostMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
        else if (zDelta > 0)
            ::PostMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
    }
}

// ���E�B���h�E�̃T�C�Y�𐧌�����B
void CandsWnd_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
    lpMinMaxInfo->ptMinTrackSize.x = 256;
    lpMinMaxInfo->ptMinTrackSize.y = 128;
}

// ���E�B���h�E��`�悷��B
void CandsWnd_OnPaint(HWND hwnd)
{
    if (xg_vecCandidates.empty()) {
        PAINTSTRUCT ps;
        HDC hdc = ::BeginPaint(hwnd, &ps);
        if (hdc) {
            MRect rcClient;
            ::GetClientRect(hwnd, &rcClient);
            ::SetTextColor(hdc, RGB(0, 0, 0));
            ::SetBkMode(hdc, TRANSPARENT);
            ::DrawTextW(hdc, XgLoadStringDx1(IDS_NOCANDIDATES), -1,
                &rcClient,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
            ::EndPaint(hwnd, &ps);
        }
    } else {
        FORWARD_WM_PAINT(hwnd, DefWindowProcW);
    }
}

extern "C"
LRESULT CALLBACK
XgCandsWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    HANDLE_MSG(hwnd, WM_CREATE, CandsWnd_OnCreate);
    HANDLE_MSG(hwnd, WM_SIZE, CandsWnd_OnSize);
    HANDLE_MSG(hwnd, WM_VSCROLL, CandsWnd_OnVScroll);
    HANDLE_MSG(hwnd, WM_KEYDOWN, CandsWnd_OnKey);
    HANDLE_MSG(hwnd, WM_KEYUP, CandsWnd_OnKey);
    HANDLE_MSG(hwnd, WM_DESTROY, CandsWnd_OnDestroy);
    HANDLE_MSG(hwnd, WM_COMMAND, CandsWnd_OnCommand);
    HANDLE_MSG(hwnd, WM_MOUSEWHEEL, CandsWnd_OnMouseWheel);
    HANDLE_MSG(hwnd, WM_GETMINMAXINFO, CandsWnd_OnGetMinMaxInfo);
    HANDLE_MSG(hwnd, WM_PAINT, CandsWnd_OnPaint);

    default:
        return ::DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// ���E�B���h�E���쐬����B
BOOL XgCreateCandsWnd(HWND hwnd)
{
    const DWORD style = WS_OVERLAPPED | WS_CAPTION |
        WS_SYSMENU | WS_THICKFRAME | WS_HSCROLL | WS_VSCROLL;
    ::CreateWindowExW(WS_EX_TOOLWINDOW,
        s_pszCandsWndClass, XgLoadStringDx1(IDS_CANDIDATES), style,
        s_nCandsWndX, s_nCandsWndY, s_nCandsWndCX, s_nCandsWndCY,
        hwnd, nullptr, xg_hInstance, nullptr);
    if (xg_hCandsWnd) {
        return TRUE;
    }

    return FALSE;
}

// ���̓��e�����E�B���h�E�ŊJ���B
bool __fastcall XgOpenCandsWnd(HWND hwnd, bool vertical)
{
    // �������E�B���h�E�����݂���Δj������B
    if (xg_hCandsWnd) {
        HWND hwnd = xg_hCandsWnd;
        xg_hCandsWnd = NULL;
        DestroyWindow(hwnd);
    }

    // �����쐬����B
    xg_iCandPos = xg_caret_pos.m_i;
    xg_jCandPos = xg_caret_pos.m_j;
    xg_bCandVertical = vertical;
    if (xg_xword.GetAt(xg_iCandPos, xg_jCandPos) == ZEN_BLACK) {
        ::MessageBeep(0xFFFFFFFF);
        return false;
    }

    // �p�^�[�����擾����B
    int lo, hi;
    std::wstring pattern;
    bool left_black, right_black;
    if (xg_bCandVertical) {
        lo = hi = xg_iCandPos;
        while (lo > 0) {
            if (xg_xword.GetAt(lo - 1, xg_jCandPos) != ZEN_BLACK)
                --lo;
            else
                break;
        }
        while (hi + 1 < xg_nRows) {
            if (xg_xword.GetAt(hi + 1, xg_jCandPos) != ZEN_BLACK)
                ++hi;
            else
                break;
        }

        for (int i = lo; i <= hi; ++i) {
            pattern += xg_xword.GetAt(i, xg_jCandPos);
        }

        right_black = (hi + 1 != xg_nRows);
    } else {
        lo = hi = xg_jCandPos;
        while (lo > 0) {
            if (xg_xword.GetAt(xg_iCandPos, lo - 1) != ZEN_BLACK)
                --lo;
            else
                break;
        }
        while (hi + 1 < xg_nCols) {
            if (xg_xword.GetAt(xg_iCandPos, hi + 1) != ZEN_BLACK)
                ++hi;
            else
                break;
        }

        for (int j = lo; j <= hi; ++j) {
            pattern += xg_xword.GetAt(xg_iCandPos, j);
        }

        right_black = (hi + 1 != xg_nCols);
    }
    left_black = (lo != 0);

    // �����擾����B
    int nSkip = 0;
    std::vector<std::wstring> cands, cands2;
    XgGetCandidatesAddBlack<false>(cands, pattern, nSkip, left_black, right_black);
    XgGetCandidatesAddBlack<true>(cands2, pattern, nSkip, left_black, right_black);
    cands.insert(cands.end(), cands2.begin(), cands2.end());

    // ���ɓK�p���āA�������ǂ����m���߁A�����Ȃ��̂�����
    // �ŏI�I�Ȍ��Ƃ���B
    xg_vecCandidates.clear();
    for (const auto& cand : cands) {
        XG_Board xword(xg_xword);
        XgApplyCandidate(xword, cand);
        if (xword.CornerBlack() || xword.DoubleBlack() ||
            xword.TriBlackAround() || xword.DividedByBlack())
        {
            ;
        } else {
            xg_vecCandidates.emplace_back(cand);
        }
    }

    // �������B
    if (xg_vecCandidates.size() > xg_nMaxCandidates)
        xg_vecCandidates.resize(xg_nMaxCandidates);

    if (xg_vecCandidates.empty()) {
        if (XgCheckCrossWord(hwnd, false)) {
            ::MessageBeep(0xFFFFFFFF);
        } else {
            return false;
        }
    }

    // �q���g�E�B���h�E���쐬����B
    if (XgCreateCandsWnd(xg_hMainWnd)) {
        ::ShowWindow(xg_hCandsWnd, SW_SHOWNORMAL);
        ::UpdateWindow(xg_hCandsWnd);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////

// Windows�A�v���̃��C���֐��B
extern "C"
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/,
    LPSTR /*pszCmdLine*/,
    int nCmdShow)
{
    // �A�v���̃C���X�^���X��ۑ�����B
    xg_hInstance = hInstance;

    // �ݒ��ǂݍ��ށB
    XgLoadSettings();

    // �����t�@�C���̖��O��ǂݍ��ށB
    XgLoadDictsAll();

    // �������W���[��������������B
    srand(::GetTickCount() ^ ::GetCurrentThreadId());

    // �v���Z�b�T�̐����擾����B
    SYSTEM_INFO si;
    ::GetSystemInfo(&si);
    s_dwNumberOfProcessors = si.dwNumberOfProcessors;

    // �v���Z�b�T�̐��ɍ��킹�ăX���b�h�̐������߂�B
    if (s_dwNumberOfProcessors <= 3)
        xg_dwThreadCount = 2;
    else
        xg_dwThreadCount = s_dwNumberOfProcessors - 1;

    xg_aThreadInfo.resize(xg_dwThreadCount);
    xg_ahThreads.resize(xg_dwThreadCount);

    // �R���� �R���g���[��������������B
    INITCOMMONCONTROLSEX iccx;
    iccx.dwSize = sizeof(iccx);
    iccx.dwICC = ICC_USEREX_CLASSES | ICC_PROGRESS_CLASS;
    ::InitCommonControlsEx(&iccx);

    // �A�N�Z�����[�^��ǂݍ��ށB
    s_hAccel = ::LoadAcceleratorsW(hInstance, MAKEINTRESOURCE(1));
    if (s_hAccel == nullptr) {
        // �A�N�Z�����[�^�쐬���s���b�Z�[�W�B
        XgCenterMessageBoxW(nullptr, XgLoadStringDx1(IDS_CANTACCEL), nullptr, MB_ICONERROR);
        return 3;
    }

    // �E�B���h�E�N���X��o�^����B
    WNDCLASSEXW wcx;
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.lpfnWndProc = XgWindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(1));
    wcx.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcx.hbrBackground = reinterpret_cast<HBRUSH>(INT_PTR(COLOR_3DFACE + 1));
    wcx.lpszMenuName = MAKEINTRESOURCE(1);
    wcx.lpszClassName = s_pszMainWndClass;
    wcx.hIconSm = nullptr;
    if (!::RegisterClassExW(&wcx)) {
        // �E�B���h�E�o�^���s���b�Z�[�W�B
        XgCenterMessageBoxW(nullptr, XgLoadStringDx1(IDS_CANTREGWND), nullptr, MB_ICONERROR);
        return 1;
    }
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.lpfnWndProc = XgHintsWndProc;
    wcx.hIcon = NULL;
    wcx.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<INT_PTR>(COLOR_3DFACE + 1));
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = s_pszHintsWndClass;
    if (!::RegisterClassExW(&wcx)) {
        // �E�B���h�E�o�^���s���b�Z�[�W�B
        XgCenterMessageBoxW(nullptr, XgLoadStringDx1(IDS_CANTREGWND), nullptr, MB_ICONERROR);
        return 1;
    }
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.lpfnWndProc = XgCandsWndProc;
    wcx.hIcon = NULL;
    wcx.hbrBackground = ::CreateSolidBrush(RGB(255, 255, 192));
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = s_pszCandsWndClass;
    if (!::RegisterClassExW(&wcx)) {
        // �E�B���h�E�o�^���s���b�Z�[�W�B
        XgCenterMessageBoxW(nullptr, XgLoadStringDx1(IDS_CANTREGWND), nullptr, MB_ICONERROR);
        return 1;
    }

    // �N���e�B�J���Z�N�V����������������B
    ::InitializeCriticalSection(&xg_cs);

    // ���C���E�B���h�E���쐬����B
    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS;
    ::CreateWindowW(s_pszMainWndClass, XgLoadStringDx1(IDS_APPINFO), style,
        s_nMainWndX, s_nMainWndY, s_nMainWndCX, s_nMainWndCY,
        nullptr, nullptr, hInstance, nullptr);
    if (xg_hMainWnd == nullptr) {
        // �E�B���h�E�쐬���s���b�Z�[�W�B
        XgCenterMessageBoxW(nullptr, XgLoadStringDx1(IDS_CANTMAKEWND), nullptr, MB_ICONERROR);
        return 2;
    }

    // �E�B���h�E��\������B
    ::ShowWindow(xg_hMainWnd, nCmdShow);
    ::UpdateWindow(xg_hMainWnd);

    // Ctrl+A�̋@�\��L���ɂ���B
    xg_hCtrlAHook = ::SetWindowsHookEx(WH_MSGFILTER,
        XgCtrlAMessageProc, NULL, ::GetCurrentThreadId());

    // ���b�Z�[�W���[�v�B
    MSG msg;
    while (::GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_F6) {
            if (GetAsyncKeyState(VK_SHIFT) < 0)
                PostMessageW(xg_hMainWnd, WM_COMMAND, ID_PANEPREV, 0);
            else
                PostMessageW(xg_hMainWnd, WM_COMMAND, ID_PANENEXT, 0);
            continue;
        }

        if (xg_hHintsWnd && GetParent(msg.hwnd) == xg_hHintsWnd && 
            msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) 
        {
            msg.wParam = VK_TAB;
        }

        if (msg.message == WM_KEYDOWN &&
            msg.wParam == L'L' && GetAsyncKeyState(VK_CONTROL) < 0)
        {
            msg.hwnd = xg_hMainWnd;
        }

        if (msg.message == WM_KEYDOWN &&
            msg.wParam == L'U' && GetAsyncKeyState(VK_CONTROL) < 0)
        {
            msg.hwnd = xg_hMainWnd;
        }

        if (xg_hHintsWnd && ::IsDialogMessageW(xg_hHintsWnd, &msg))
            continue;

        if (xg_hCandsWnd) {
            if (msg.message != WM_KEYDOWN || msg.wParam != VK_ESCAPE) {
                if (::IsDialogMessageW(xg_hCandsWnd, &msg))
                    continue;
            }
        }

        if (xg_hwndInputPalette) {
            if (::IsDialogMessageW(xg_hwndInputPalette, &msg))
                continue;
        }

        if (!::TranslateAcceleratorW(xg_hMainWnd, s_hAccel, &msg)) {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
    }

    // Ctrl+A�̋@�\����������B
    ::UnhookWindowsHookEx(xg_hCtrlAHook);
    xg_hCtrlAHook = NULL;

    // �N���e�B�J���Z�N�V������j������B
    ::DeleteCriticalSection(&xg_cs);

    // �ݒ��ۑ��B
    XgSaveSettings();

    return static_cast<int>(msg.wParam);
}

//////////////////////////////////////////////////////////////////////////////
