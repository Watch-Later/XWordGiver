//////////////////////////////////////////////////////////////////////////////
// XWordGiver.hpp --- XWord Giver (Japanese Crossword Generator)
// Copyright (C) 2012-2020 Katayama Hirofumi MZ. All Rights Reserved.
// (Japanese, Shift_JIS)

#ifndef __XWORDGIVER_XG_H__
#define __XWORDGIVER_XG_H__

#ifndef __cplusplus
    #error You lose.    // not C++ compiler
#endif

#if __cplusplus < 199711L
    #error Modern C++ required. You lose.
#endif

#if __cplusplus < 201103L
    #define xg_constexpr   /*empty*/
#else
    #define xg_constexpr   constexpr
#endif

#if !defined(NDEBUG) && !defined(_DEBUG)
    #define _DEBUG
#endif

#define _CRT_SECURE_NO_WARNINGS // kill security warnings
#define _CRT_NON_CONFORMING_WCSTOK  // use legacy wcstok 

////////////////////////////////////////////////////////////////////////////

#include "TargetVer.h"  // for WINVER, _WIN32_WINNT, _WIN32_IE

#include <windows.h>    // for Windows API
#include <windowsx.h>   // for HANDLE_MSG
#include <commctrl.h>   // common controls
#include <shlobj.h>     // for CoCreateInstance, IShellLink, IPersistFile
#include <imm.h>        // for ImmSetOpenStatus
#include <shlwapi.h>    // Shell Light-weight API

#include <cstdlib>      // for std::memcpy, std::memset
#include <cassert>      // for assert
#include <vector>       // for std::vector
#include <set>          // for std::set, std::multiset
#include <map>          // for std::map

#include <algorithm>        // for std::replace
#include <unordered_set>    // for std::unordered_set, std::unordered_multiset
#include <unordered_map>    // for std::unordered_map
#include <utility>          // for std::move

#include <queue>        // for std::queue
#include <deque>        // for std::deque
#include <algorithm>    // for std::sort, std::random_shuffle
#include <string>       // for std::wstring

#include <process.h>    // for _beginthreadex

#include <strsafe.h>    // for String... functions

#include "resource.h"   // resource-related macros

////////////////////////////////////////////////////////////////////////////

#include "MPointSizeRect.hpp"
#include "MRegKey.hpp"
#include "MScrollView.hpp"

#include "Dictionary.hpp"
#include "xword.hpp"
#include "Marks.hpp"
#include "SaveBitmapToFile.h"

////////////////////////////////////////////////////////////////////////////

#undef min
#undef max

#include "nlohmann/json.hpp"
using json = nlohmann::json;

//////////////////////////////////////////////////////////////////////////////
// std::wstring���r����t�@���N�^�B

class xg_wstrinxg_less
{
public:
    bool __fastcall operator()(
        const std::wstring& s1, const std::wstring& s2) const
    {
        return s1 < s2;
    }
};

// ������std::wstring���r����t�@���N�^�B
class xg_wstring_size_greater
{
public:
    bool __fastcall operator()(
        const std::wstring& s1, const std::wstring& s2) const
    {
        return s1.size() > s2.size();
    }
};

//////////////////////////////////////////////////////////////////////////////
// ���̓��[�h�B

enum XG_InputMode
{
    xg_im_KANA,     // �J�i���́B
    xg_im_ABC,      // �p�����́B
    xg_im_KANJI,    // �������́B
    xg_im_RUSSIA,   // ���V�A�����B
};
extern XG_InputMode xg_imode;

//////////////////////////////////////////////////////////////////////////////
// �����̔���B

// ���p�啶���p�����H
inline bool XgIsCharHankakuUpperW(WCHAR ch)
{
    return (L'A' <= ch && ch <= L'Z');
}

// ���p�������p�����H
inline bool XgIsCharHankakuLowerW(WCHAR ch)
{
    return (L'a' <= ch && ch <= L'z');
}

// ���p�p�����H
inline bool XgIsCharHankakuAlphaW(WCHAR ch)
{
    return XgIsCharHankakuUpperW(ch) || XgIsCharHankakuLowerW(ch);
}

// �S�p�啶���p�����H
inline bool XgIsCharZenkakuUpperW(WCHAR ch)
{
    return (L'\xFF21' <= ch && ch <= L'\xFF3A');
}

// �S�p�������p�����H
inline bool XgIsCharZenkakuLowerW(WCHAR ch)
{
    return (L'\xFF41' <= ch && ch <= L'\xFF5A');
}

// �S�p�p�����H
inline bool XgIsCharZenkakuAlphaW(WCHAR ch)
{
    return XgIsCharZenkakuUpperW(ch) || XgIsCharZenkakuLowerW(ch);
}

// �Ђ炪�Ȃ��H
inline bool XgIsCharHiraganaW(WCHAR ch)
{
    return ((L'\x3041' <= ch && ch <= L'\x3093') || ch == L'\x30FC' || ch == L'\x3094');
}

// �J�^�J�i���H
inline bool XgIsCharKatakanaW(WCHAR ch)
{
    return ((L'\x30A1' <= ch && ch <= L'\x30F3') || ch == L'\x30FC' || ch == L'\x30F4');
}

// ���Ȃ��H
inline bool XgIsCharKanaW(WCHAR ch)
{
    return XgIsCharHiraganaW(ch) || XgIsCharKatakanaW(ch);
}

// �������H
inline bool XgIsCharKanjiW(WCHAR ch)
{
    return ((0x3400 <= ch && ch <= 0x9FFF) ||
            (0xF900 <= ch && ch <= 0xFAFF) || ch == L'\x3007');
}

// �n���O�����H
inline bool XgIsCharHangulW(WCHAR ch)
{
    return ((0x1100 <= ch && ch <= 0x11FF) ||
            (0xAC00 <= ch && ch <= 0xD7A3) ||
            (0x3130 <= ch && ch <= 0x318F));
}

// �L�����������H
inline bool XgIsCharZenkakuCyrillicW(WCHAR ch)
{
    return 0x0400 <= ch && ch <= 0x04FF;
}

//////////////////////////////////////////////////////////////////////////////

// �X���b�h���B
struct XG_ThreadInfo
{
    // �X���b�hID�B
    unsigned    m_threadid;
    // ��ł͂Ȃ��}�X�̐��B
    int         m_count;
};

// �X���b�h���B
extern std::vector<XG_ThreadInfo>       xg_aThreadInfo;

//////////////////////////////////////////////////////////////////////////////
// �i�[���B

// �P��̊i�[���B
struct XG_PlaceInfo
{
    int             m_iRow;         // �s�̃C���f�b�N�X�B
    int             m_jCol;         // ��̃C���f�b�N�X�B
    std::wstring    m_word;         // �P��B
    int             m_number;       // �ԍ��B

    // �R���X�g���N�^�B
    XG_PlaceInfo()
    {
    }

    // �R���X�g���N�^�B
    XG_PlaceInfo(int iRow_, int jCol_, const std::wstring& word_) :
        m_iRow(iRow_), m_jCol(jCol_), m_word(word_)
    {
    }

    // �R���X�g���N�^�B
    XG_PlaceInfo(int iRow_, int jCol_, std::wstring&& word_) :
        m_iRow(iRow_), m_jCol(jCol_), m_word(std::move(word_))
    {
    }

    // �R���X�g���N�^�B
    XG_PlaceInfo(int iRow_, int jCol_, const std::wstring& word_, int number_) :
        m_iRow(iRow_), m_jCol(jCol_), m_word(word_), m_number(number_)
    {
    }

    // �R���X�g���N�^�B
    XG_PlaceInfo(int iRow_, int jCol_, std::wstring&& word_, int number_) :
        m_iRow(iRow_), m_jCol(jCol_), m_word(std::move(word_)), m_number(number_)
    {
    }

    // �R�s�[�R���X�g���N�^�B
    XG_PlaceInfo(const XG_PlaceInfo& pi)
    {
        m_iRow = pi.m_iRow;
        m_jCol = pi.m_jCol;
        m_word = pi.m_word;
        m_number = pi.m_number;
    }

    // �R�s�[�R���X�g���N�^�B
    XG_PlaceInfo(XG_PlaceInfo&& pi)
    {
        m_iRow = pi.m_iRow;
        m_jCol = pi.m_jCol;
        m_word = std::move(pi.m_word);
        m_number = pi.m_number;
    }

    // ����B
    void __fastcall operator=(const XG_PlaceInfo& pi)
    {
        m_iRow = pi.m_iRow;
        m_jCol = pi.m_jCol;
        m_word = pi.m_word;
        m_number = pi.m_number;
    }

    // ����B
    void __fastcall operator=(XG_PlaceInfo&& pi)
    {
        m_iRow = pi.m_iRow;
        m_jCol = pi.m_jCol;
        m_word = std::move(pi.m_word);
        m_number = pi.m_number;
    }
};

namespace std
{
    inline void swap(XG_PlaceInfo& info1, XG_PlaceInfo& info2)
    {
        std::swap(info1.m_iRow, info2.m_iRow);
        std::swap(info1.m_jCol, info2.m_jCol);
        std::swap(info1.m_word, info2.m_word);
        std::swap(info1.m_number, info2.m_number);
    }
}

// �^�e�ƃ��R�̂����B
extern std::vector<XG_PlaceInfo> xg_vTateInfo, xg_vYokoInfo;

//////////////////////////////////////////////////////////////////////////////

// XG_PlaceInfo�\���̂�ԍ��Ŕ�r����t�@���N�^�B
class xg_placeinfo_compare_number
{
public:
    xg_constexpr bool __fastcall operator()
        (const XG_PlaceInfo& pi1, const XG_PlaceInfo& pi2) const
    {
        return pi1.m_number < pi2.m_number;
    }
};

// XG_PlaceInfo�\���̂��ʒu�Ŕ�r����t�@���N�^�B
class xg_placeinfo_compare_position
{
public:
    bool __fastcall operator()
        (const XG_PlaceInfo *ppi1, const XG_PlaceInfo *ppi2) const
    {
        if (ppi1->m_iRow < ppi2->m_iRow)
            return true;
        if (ppi1->m_iRow > ppi2->m_iRow)
            return false;
        if (ppi1->m_jCol < ppi2->m_jCol)
            return true;
        return false;
    }
};

//////////////////////////////////////////////////////////////////////////////
// Utils.cpp

// ���\�[�X�������ǂݍ��ށB
LPWSTR __fastcall XgLoadStringDx1(int id);
// ���\�[�X�������ǂݍ��ށB
LPWSTR __fastcall XgLoadStringDx2(int id);

// �_�C�A���O�𒆉��ɂ悹��֐��B
void __fastcall XgCenterDialog(HWND hwnd);
// ���b�Z�[�W�{�b�N�X�t�b�N�p�̊֐��B
extern "C" LRESULT CALLBACK
XgMsgBoxCbtProc(int nCode, WPARAM wParam, LPARAM /*lParam*/);
// �����񂹃��b�Z�[�W�{�b�N�X��\������B
int __fastcall
XgCenterMessageBoxW(HWND hwnd, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType);
// �����񂹃��b�Z�[�W�{�b�N�X��\������B
int __fastcall
XgCenterMessageBoxIndirectW(LPMSGBOXPARAMS lpMsgBoxParams);

// �������u������B
void __fastcall xg_str_replace_all(
    std::wstring &s, const std::wstring& from, const std::wstring& to);
// ������̑O��̋󔒂���菜���B
void __fastcall xg_str_trim(std::wstring& str);
// �V���[�g�J�b�g�̃^�[�Q�b�g�̃p�X���擾����B
bool __fastcall XgGetPathOfShortcutW(LPCWSTR pszLnkFile, LPWSTR pszPath);
// �t�B���^�[����������B
LPWSTR __fastcall XgMakeFilterString(LPWSTR psz);

// �����񂩂�}���`�Z�b�g�֕ϊ�����B
void __fastcall xg_str_to_multiset(
    std::unordered_multiset<WCHAR>& mset, const std::wstring& str);
// �x�N�^�[����}���`�Z�b�g�֕ϊ�����B
void __fastcall xg_vec_to_multiset(
    std::unordered_multiset<WCHAR>& mset, const std::vector<WCHAR>& str);
// �����}���`�Z�b�g���ǂ����H
bool __fastcall xg_submultiseteq(const std::unordered_multiset<WCHAR>& ms1,
                                 const std::unordered_multiset<WCHAR>& ms2);
// ReadMe���J���B
void __fastcall XgOpenReadMe(HWND hwnd);
// License���J���B
void __fastcall XgOpenLicense(HWND hwnd);
// �p�^�[�����J���B
void __fastcall XgOpenPatterns(HWND hwnd);

// �t�@�C�����������݉\���H
bool __fastcall XgCanWriteFile(const WCHAR *pszFile);

void __fastcall XgSetInputModeFromDict(HWND hwnd);

// Unicode -> UTF8
std::string XgUnicodeToUtf8(const std::wstring& wide);

// ANSI -> Unicode
std::wstring XgAnsiToUnicode(const std::string& ansi);

// Unicode -> ANSI
std::string XgUnicodeToAnsi(const std::wstring& wide);

// UTF-8 -> Unicode.
std::wstring __fastcall XgUtf8ToUnicode(const std::string& ansi);

// JSON����������B
std::wstring XgJsonEncodeString(const std::wstring& str);

// �p�X�����B
BOOL XgMakePathW(LPCWSTR pszPath);

// �G���f�B�A���ϊ��B
void XgSwab(LPBYTE pbFile, DWORD cbFile);

// HTML�`���̃N���b�v�{�[�h�f�[�^���쐬����B
std::string XgMakeClipHtmlData(
    const std::string& html_utf8, const std::string& style_utf8 = "");

// HTML�`���̃N���b�v�{�[�h�f�[�^���쐬����B
std::string XgMakeClipHtmlData(
    const std::wstring& html_wide, const std::wstring& style_wide = L"");

//////////////////////////////////////////////////////////////////////////////

// ���s�B
extern const LPCWSTR xg_pszNewLine;

// �C���X�^���X�̃n���h���B
extern HINSTANCE   xg_hInstance;

// ���C���E�B���h�E�̃n���h���B
extern HWND        xg_hMainWnd;

// �c�[���o�[�̃n���h���B
extern HWND        xg_hToolBar;

// �X�e�[�^�X�o�[�̃n���h���B
extern HWND        xg_hStatusBar;

// �q���g�E�B���h�E�̃n���h���B
extern HWND        xg_hHintsWnd;

// ������\�����邩�H
extern bool xg_bShowAnswer;

// ���̓p���b�g��\�����邩�H
extern bool xg_bShowInputPalette;

// ���̓p���b�g�̈ʒu�B
extern INT xg_nInputPaletteWndX;
extern INT xg_nInputPaletteWndY;

// ���}�X�ǉ��Ȃ����H
extern bool xg_bNoAddBlack;

// �r������̂��߂̃N���e�B�J���Z�N�V�����B
extern CRITICAL_SECTION xg_cs;

// �X���b�h�̐��B
extern DWORD xg_dwThreadCount;

// �v�Z���L�����Z�����ꂽ���H
extern bool xg_bCancelled;

// ��̃N���X���[�h�̉��������ꍇ���H
extern bool xg_bSolvingEmpty;

// ���}�X�p�^�[�����������ꂽ���H
extern bool xg_bBlacksGenerated;

// �X�}�[�g�������H
extern bool xg_bSmartResolution;

// ���g�����邩�H
extern bool xg_bAddThickFrame;

// �u���̓p���b�g�v�c�u���H
extern bool xg_bTateOki;

// ���O�ɊJ�����N���X���[�h�f�[�^�t�@�C���̃p�X�t�@�C�����B
extern std::wstring xg_strFileName;

// �q���g�ǉ��t���O�B
extern bool xg_bHintsAdded;

// JSON�t�@�C���Ƃ��ĕۑ����邩�H
extern bool xg_bSaveAsJsonFile;

// �X���ϊ��p�f�[�^�B
extern const LPCWSTR xg_small[11];
extern const LPCWSTR xg_large[11];

// �r�b�g�}�b�v�̃n���h���B
extern HBITMAP          xg_hbmImage;

// �q���g������B
extern std::wstring     xg_strHints;

// �Čv�Z���Ȃ����Ă��邩�H
extern bool             xg_bRetrying;

// �X���b�h�̃n���h���B
extern std::vector<HANDLE> xg_ahThreads;

// �}�X�̃t�H���g�B
extern WCHAR xg_szCellFont[];

// �ԍ��̃t�H���g�B
extern WCHAR xg_szSmallFont[];

// ���O�ɉ������L�[���o���Ă����B
extern WCHAR xg_prev_vk;

// ���̓p���b�g�B
extern HWND xg_hwndInputPalette;

// �Ђ炪�ȕ\�����H
extern BOOL xg_bHiragana;
// Lowercase�\�����H
extern BOOL xg_bLowercase;

// �����̑傫���i���j�B
extern INT xg_nCellCharPercents;

// �����������̑傫���i���j�B
extern INT xg_nSmallCharPercents;

// �u���}�X�p�^�[���v�_�C�A���O�̈ʒu�ƃT�C�Y�B
extern INT xg_nPatWndX;
extern INT xg_nPatWndY;
extern INT xg_nPatWndCX;
extern INT xg_nPatWndCY;

// ���݂̎������B
extern std::wstring xg_dict_name;
// ���ׂĂ̎����t�@�C���B
extern std::deque<std::wstring>  xg_dict_files;

// ���������Z�b�g����B
void XgSetDict(const std::wstring& strFile);

//////////////////////////////////////////////////////////////////////////////

// �`��C���[�W���X�V����B
void __fastcall XgUpdateImage(HWND hwnd, int x, int y);

// �t�@�C�����J���B
bool __fastcall XgDoLoadFile(HWND hwnd, LPCWSTR pszFile, bool json);
// �t�@�C�����J���B
bool __fastcall XgDoLoadCrpFile(HWND hwnd, LPCWSTR pszFile);

// �t�@�C����ۑ�����B
bool __fastcall XgDoSave(HWND /*hwnd*/, LPCWSTR pszFile);

// �t�@�C���iJSON�`���j��ۑ�����B
bool __fastcall XgDoSaveJson(HWND /*hwnd*/, LPCWSTR pszFile);
// �t�@�C���iCRP�`���j��ۑ�����B
bool __fastcall XgDoSaveCrpFile(HWND /*hwnd*/, LPCWSTR pszFile);

// �q���g��\������B
void __fastcall XgShowHints(HWND hwnd);

// �q���g�̓��e���q���g�E�B���h�E�ŊJ���B
bool __fastcall XgOpenHintsByWindow(HWND /*hwnd*/);

// �q���g�̓��e���������ŊJ���B
bool __fastcall XgOpenHintsByNotepad(HWND /*hwnd*/, bool bShowAnswer);

// �q���g���ύX���ꂽ���H
bool __fastcall XgAreHintsModified(void);

// �q���g�f�[�^���X�V����B
bool __fastcall XgUpdateHintsData(void);

// �q���g�f�[�^��ݒ肷��B
void __fastcall XgSetHintsData(void);

// �q���g���������͂���B
bool __fastcall XgParseHintsStr(HWND hwnd, const std::wstring& strHints);

// �����摜�t�@�C���Ƃ��ĕۑ�����B
void __fastcall XgSaveProbAsImage(HWND hwnd);

// �𓚂��摜�t�@�C���Ƃ��ĕۑ�����B
void __fastcall XgSaveAnsAsImage(HWND hwnd);

// �q���g�E�B���h�E���쐬����B
BOOL XgCreateHintsWnd(HWND hwnd);
// �q���g�E�B���h�E��j������B
void XgDestroyHintsWnd(void);

template <bool t_alternative>
bool __fastcall XgGetCandidatesAddBlack(
    std::vector<std::wstring>& cands, const std::wstring& pattern, int& nSkip,
    bool left_black_check, bool right_black_check);

// UI�t�H���g�̘_���I�u�W�F�N�g���擾����B
LOGFONTW *XgGetUIFont(void);

// ���̓��e�����E�B���h�E�ŊJ���B
bool __fastcall XgOpenCandsWnd(HWND hwnd, bool vertical);

// ���E�B���h�E��j������B
void XgDestroyCandsWnd(void);

// ���̓p���b�g���쐬����B
BOOL XgCreateInputPalette(HWND hwndOwner);

// ���̓p���b�g��j������B
BOOL XgDestroyInputPalette(void);

// ���̓��[�h��؂�ւ���B
void __fastcall XgSetInputMode(HWND hwnd, XG_InputMode mode);

// ���������͂��ꂽ�B
void __fastcall MainWnd_OnChar(HWND hwnd, TCHAR ch, int cRepeat);

// BackSpace�����s����B
void __fastcall XgCharBack(HWND hwnd);

// ���͕�����؂�ւ���B
void __fastcall XgInputDirection(HWND hwnd, INT nDirection);
// ���������؂�ւ���B
void __fastcall XgSetCharFeed(HWND hwnd, INT nMode);
// ���s����B
void __fastcall XgReturn(HWND hwnd);
// �������N���A�B
void __fastcall XgClearNonBlocks(HWND hwnd);

// �}���`�X���b�h�p�̊֐��B
unsigned __stdcall XgGenerateBlacks(void *param);
// �}���`�X���b�h�p�̊֐��B
unsigned __stdcall XgGenerateBlacksSmart(void *param);
// ��d�}�X�؂�ւ��B
void __fastcall XgToggleMark(HWND hwnd);

// �N���X���[�h�Ŏg�������ɕϊ�����B
std::wstring __fastcall XgNormalizeString(const std::wstring& text);

// �|�b�v�A�b�v���j���[��ǂݍ��ށB
HMENU XgLoadPopupMenu(HWND hwnd, INT nPos);

//////////////////////////////////////////////////////////////////////////////
// �X�N���[���B

// �����X�N���[���̈ʒu���擾����B
int __fastcall XgGetHScrollPos(void);
// �����X�N���[���̈ʒu���擾����B
int __fastcall XgGetVScrollPos(void);
// �����X�N���[���̏����擾����B
BOOL __fastcall XgGetHScrollInfo(LPSCROLLINFO psi);
// �����X�N���[���̏����擾����B
BOOL __fastcall XgGetVScrollInfo(LPSCROLLINFO psi);
// �����X�N���[���̈ʒu��ݒ肷��B
int __fastcall XgSetHScrollPos(int nPos, BOOL bRedraw);
// �����X�N���[���̈ʒu��ݒ肷��B
int __fastcall XgSetVScrollPos(int nPos, BOOL bRedraw);
// �X�N���[������ݒ肷��B
void __fastcall XgUpdateScrollInfo(HWND hwnd, int x, int y);
// �L�����b�g��������悤�ɁA�K�v�Ȃ�΃X�N���[������B
void __fastcall XgEnsureCaretVisible(HWND hwnd);

//////////////////////////////////////////////////////////////////////////////
// ���B

extern int  xg_iCandPos;
extern int  xg_jCandPos;
extern bool xg_bCandVertical;

//////////////////////////////////////////////////////////////////////////////

#include "UndoBuffer.hpp"

#endif  // ndef __XWORDGIVER_XG_H__
