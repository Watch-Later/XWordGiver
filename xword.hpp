//////////////////////////////////////////////////////////////////////////////
// xword.hpp --- XWord Giver (Japanese Crossword Generator)
// Copyright (C) 2012-2020 Katayama Hirofumi MZ. All Rights Reserved.
// (Japanese, Shift_JIS)

#ifndef __XWORD_HPP__
#define __XWORD_HPP__

//////////////////////////////////////////////////////////////////////////////
// �N���X���[�h�B

// ���{�ꃍ�P�[���B
#define JPN_LOCALE \
    MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT), SORT_DEFAULT)

// ���V�A�ꃍ�P�[���B
#define RUS_LOCALE \
    MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT), SORT_DEFAULT)

// Shift_JIS codepage
#define SJIS_CODEPAGE 932

// Russian codepage
#define RUS_CODEPAGE 1251

// ����̕����̑傫��(%)�B
#define DEF_CELL_CHAR_SIZE 66
#define DEF_SMALL_CHAR_SIZE 21

// �\���p�ɕ`�悷�邩�H�iXgGetXWordExtent��XgDrawXWord��XgCreateXWordImage�Ŏg���j�B
extern INT xg_nForDisplay;
struct ForDisplay {
    ForDisplay() {
        ++xg_nForDisplay;
    }
    ~ForDisplay() {
        --xg_nForDisplay;
    }
};

// �Y�[���䗦(%)�B
extern INT xg_nZoomRate;
// �Z���̑傫���B
#define xg_nCellSize        48
// �㉺���E�̃}�[�W���B
#define xg_nMargin          16
// �㉺���E�̃}�[�W���B
#define xg_nNarrowMargin    8
// ���̍ő吔�B
#define xg_nMaxCandidates   500

// XG_Board::EveryPatternValid1, XG_Board::EveryPatternValid2�̖߂�l�B
enum XG_EpvCode
{
    xg_epv_SUCCESS,        // �����B
    xg_epv_PATNOTMATCH,    // �}�b�`���Ȃ������p�^�[��������B
    xg_epv_DOUBLEWORD,     // �P�ꂪ�d�����Ă���B
    xg_epv_LENGTHMISMATCH, // �󔒂̒�������v���Ȃ��B
    xg_epv_NOTFOUNDWORD    // �P��Ƀ}�b�`���Ȃ������}�X������B
};

// �Z���̐F�B
extern COLORREF xg_rgbWhiteCellColor;
extern COLORREF xg_rgbBlackCellColor;
extern COLORREF xg_rgbMarkedCellColor;

// ��d�}�X�ɘg��`�����H
extern bool xg_bDrawFrameForMarkedCell;

// ��������H
extern bool xg_bCharFeed;

// �^�e���́H
extern bool xg_bTateInput;

// �S�p�����B
#define ZEN_SPACE       WCHAR(0x3000)  // L'�@'
#define ZEN_BLACK       WCHAR(0x25A0)  // L'��'
#define ZEN_LARGE_A     WCHAR(0xFF21)  // L'�`'
#define ZEN_SMALL_A     WCHAR(0xFF41)  // L'��'
#define ZEN_A           WCHAR(0x30A2)  // L'�A'
#define ZEN_I           WCHAR(0x30A4)  // L'�C'
#define ZEN_U           WCHAR(0x30A6)  // L'�E'
#define ZEN_E           WCHAR(0x30A8)  // L'�G'
#define ZEN_O           WCHAR(0x30AA)  // L'�I'
#define ZEN_KA          WCHAR(0x30AB)  // L'�J'
#define ZEN_KI          WCHAR(0x30AD)  // L'�L'
#define ZEN_KU          WCHAR(0x30AF)  // L'�N'
#define ZEN_KE          WCHAR(0x30B1)  // L'�P'
#define ZEN_KO          WCHAR(0x30B3)  // L'�R'
#define ZEN_SA          WCHAR(0x30B5)  // L'�T'
#define ZEN_SI          WCHAR(0x30B7)  // L'�V'
#define ZEN_SU          WCHAR(0x30B9)  // L'�X'
#define ZEN_SE          WCHAR(0x30BB)  // L'�Z'
#define ZEN_SO          WCHAR(0x30BD)  // L'�\'
#define ZEN_TA          WCHAR(0x30BF)  // L'�^'
#define ZEN_CHI         WCHAR(0x30C1)  // L'�`'
#define ZEN_TSU         WCHAR(0x30C4)  // L'�c'
#define ZEN_TE          WCHAR(0x30C6)  // L'�e'
#define ZEN_TO          WCHAR(0x30C8)  // L'�g'
#define ZEN_NA          WCHAR(0x30CA)  // L'�i'
#define ZEN_NI          WCHAR(0x30CB)  // L'�j'
#define ZEN_NU          WCHAR(0x30CC)  // L'�k'
#define ZEN_NE          WCHAR(0x30CD)  // L'�l'
#define ZEN_NO          WCHAR(0x30CE)  // L'�m'
#define ZEN_NN          WCHAR(0x30F3)  // L'��'
#define ZEN_HA          WCHAR(0x30CF)  // L'�n'
#define ZEN_HI          WCHAR(0x30D2)  // L'�q'
#define ZEN_FU          WCHAR(0x30D5)  // L'�t'
#define ZEN_HE          WCHAR(0x30D8)  // L'�w'
#define ZEN_HO          WCHAR(0x30DB)  // L'�z'
#define ZEN_MA          WCHAR(0x30DE)  // L'�}'
#define ZEN_MI          WCHAR(0x30DF)  // L'�~'
#define ZEN_MU          WCHAR(0x30E0)  // L'��'
#define ZEN_ME          WCHAR(0x30E1)  // L'��'
#define ZEN_MO          WCHAR(0x30E2)  // L'��'
#define ZEN_YA          WCHAR(0x30E4)  // L'��'
#define ZEN_YU          WCHAR(0x30E6)  // L'��'
#define ZEN_YO          WCHAR(0x30E8)  // L'��'
#define ZEN_RA          WCHAR(0x30E9)  // L'��'
#define ZEN_RI          WCHAR(0x30EA)  // L'��'
#define ZEN_RU          WCHAR(0x30EB)  // L'��'
#define ZEN_RE          WCHAR(0x30EC)  // L'��'
#define ZEN_RO          WCHAR(0x30ED)  // L'��'
#define ZEN_WA          WCHAR(0x30EF)  // L'��'
#define ZEN_WI          WCHAR(0x30F0)  // L'��'
#define ZEN_WE          WCHAR(0x30F1)  // L'��'
#define ZEN_WO          WCHAR(0x30F2)  // L'��'
#define ZEN_GA          WCHAR(0x30AC)  // L'�K'
#define ZEN_GI          WCHAR(0x30AE)  // L'�M'
#define ZEN_GU          WCHAR(0x30B0)  // L'�O'
#define ZEN_GE          WCHAR(0x30B2)  // L'�Q'
#define ZEN_GO          WCHAR(0x30B4)  // L'�S'
#define ZEN_ZA          WCHAR(0x30B6)  // L'�U'
#define ZEN_JI          WCHAR(0x30B8)  // L'�W'
#define ZEN_ZU          WCHAR(0x30BA)  // L'�Y'
#define ZEN_ZE          WCHAR(0x30BC)  // L'�['
#define ZEN_ZO          WCHAR(0x30BE)  // L'�]'
#define ZEN_DA          WCHAR(0x30C0)  // L'�_'
#define ZEN_DI          WCHAR(0x30C2)  // L'�a'
#define ZEN_DU          WCHAR(0x30C5)  // L'�d'
#define ZEN_DE          WCHAR(0x30C7)  // L'�f'
#define ZEN_DO          WCHAR(0x30C9)  // L'�h'
#define ZEN_BA          WCHAR(0x30D0)  // L'�o'
#define ZEN_BI          WCHAR(0x30D3)  // L'�r'
#define ZEN_BU          WCHAR(0x30D6)  // L'�u'
#define ZEN_BE          WCHAR(0x30D9)  // L'�x'
#define ZEN_BO          WCHAR(0x30DC)  // L'�{'
#define ZEN_PA          WCHAR(0x30D1)  // L'�p'
#define ZEN_PI          WCHAR(0x30D4)  // L'�s'
#define ZEN_PU          WCHAR(0x30D7)  // L'�v'
#define ZEN_PE          WCHAR(0x30DA)  // L'�y'
#define ZEN_PO          WCHAR(0x30DD)  // L'�|'
#define ZEN_PROLONG     WCHAR(0x30FC)  // L'�['
#define ZEN_ULEFT       WCHAR(0x250F)  // L'��'
#define ZEN_URIGHT      WCHAR(0x2513)  // L'��'
#define ZEN_LLEFT       WCHAR(0x2517)  // L'��'
#define ZEN_LRIGHT      WCHAR(0x251B)  // L'��'
#define ZEN_VLINE       WCHAR(0x2503)  // L'��'
#define ZEN_HLINE       WCHAR(0x2501)  // L'��'
#define ZEN_UP          WCHAR(0x2191)  // L'��'
#define ZEN_DOWN        WCHAR(0x2193)  // L'��'
#define ZEN_LEFT        WCHAR(0x2190)  // L'��'
#define ZEN_RIGHT       WCHAR(0x2192)  // L'��'
#define ZEN_ASTERISK    WCHAR(0xFF0A)  // L'��'
#define ZEN_BULLET      WCHAR(0x25CF)  // L'��'
#define ZEN_UNDERLINE   WCHAR(0xFF3F)  // L'�Q'

//////////////////////////////////////////////////////////////////////////////
// ���[���Q�B

enum RULES
{
    RULE_DONTDOUBLEBLACK = (1 << 0),    // �A���ցB
    RULE_DONTCORNERBLACK = (1 << 1),    // �l�����ցB
    RULE_DONTTRIDIRECTIONS = (1 << 2),  // �O�����ցB
    RULE_DONTDIVIDE = (1 << 3),         // ���f�ցB
    RULE_DONTFOURDIAGONALS = (1 << 4),  // ���Ύl�A�ցB
    RULE_POINTSYMMETRY = (1 << 5)       // ���}�X�_�Ώ́B
};

// �f�t�H���g�̃��[���B
#define DEFAULT_RULES (RULE_DONTDOUBLEBLACK | RULE_DONTCORNERBLACK | \
                       RULE_DONTTRIDIRECTIONS | RULE_DONTDIVIDE)

// ���[���Q�B
extern INT xg_nRules;

//////////////////////////////////////////////////////////////////////////////
// �}�X�̈ʒu�B

struct XG_Pos
{
    int m_i;    // �s�̃C���f�b�N�X�B
    int m_j;    // ��̃C���f�b�N�X�B

    // �R���X�g���N�^�B
    XG_Pos() { }

    // �R���X�g���N�^�B
    xg_constexpr XG_Pos(int i, int j) : m_i(i), m_j(j) { }

    // �R���X�g���N�^�B
    xg_constexpr XG_Pos(const XG_Pos& pos) : m_i(pos.m_i), m_j(pos.m_j) { }

    // ��r�B
    xg_constexpr bool __fastcall operator==(const XG_Pos& pos) const {
        return m_i == pos.m_i && m_j == pos.m_j;
    }

    // ��r�B
    xg_constexpr bool __fastcall operator!=(const XG_Pos& pos) const {
        return m_i != pos.m_i || m_j != pos.m_j;
    }

    void clear() {
        m_i = 0; m_j = 0;
    }
};

namespace std
{
    template <>
    inline void swap(XG_Pos& pos1, XG_Pos& pos2) {
        std::swap(pos1.m_i, pos2.m_i);
        std::swap(pos1.m_j, pos2.m_j);
    }
}

struct XG_PosHash
{
    size_t operator()(const XG_Pos& pos) const {
        return static_cast<size_t>((pos.m_i << 16) | pos.m_j);
    }
};

// �L�����b�g�̈ʒu�B
extern XG_Pos xg_caret_pos;

//////////////////////////////////////////////////////////////////////////////
// �q���g�f�[�^�B

struct XG_Hint
{
    int             m_number;
    std::wstring    m_strWord;
    std::wstring    m_strHint;

    // �R���X�g���N�^�B
    XG_Hint() { }

    // �R���X�g���N�^�B
    inline
    XG_Hint(int number, const std::wstring& word, const std::wstring& hint) :
        m_number(number), m_strWord(word), m_strHint(hint)
    {
    }

    // �R���X�g���N�^�B
    inline
    XG_Hint(int number, std::wstring&& word, std::wstring&& hint) :
        m_number(number),
        m_strWord(std::move(word)),
        m_strHint(std::move(hint))
    {
    }

    // �R�s�[�R���X�g���N�^�B
    inline
    XG_Hint(const XG_Hint& info) :
        m_number(info.m_number),
        m_strWord(info.m_strWord),
        m_strHint(info.m_strHint)
    {
    }

    // �R�s�[�R���X�g���N�^�B
    inline
    XG_Hint(XG_Hint&& info) :
        m_number(info.m_number),
        m_strWord(std::move(info.m_strWord)),
        m_strHint(std::move(info.m_strHint))
    {
    }

    inline
    void operator=(const XG_Hint& info)
    {
        m_number = info.m_number;
        m_strWord = info.m_strWord;
        m_strHint = info.m_strHint;
    }

    inline
    void operator=(XG_Hint&& info)
    {
        m_number = info.m_number;
        m_strWord = std::move(info.m_strWord);
        m_strHint = std::move(info.m_strHint);
    }
};

namespace std
{
    inline void swap(XG_Hint& hint1, XG_Hint& hint2)
    {
        std::swap(hint1.m_number, hint2.m_number);
        std::swap(hint1.m_strWord, hint2.m_strWord);
        std::swap(hint1.m_strHint, hint2.m_strHint);
    }
}

extern std::vector<XG_Hint> xg_vecTateHints, xg_vecYokoHints;

//////////////////////////////////////////////////////////////////////////////
// �N���X���[�h �f�[�^�B

// �N���X���[�h�̃T�C�Y�B
extern int xg_nRows, xg_nCols;

class XG_Board
{
public:
    // �R���X�g���N�^�B
    XG_Board();
    XG_Board(const XG_Board& xw);
    XG_Board(XG_Board&& xw);

    // ����B
    void __fastcall operator=(const XG_Board& xw);
    void __fastcall operator=(XG_Board&& xw);

    // �}�X�̓��e���擾����B
    WCHAR __fastcall GetAt(int i) const;
    WCHAR __fastcall GetAt(int iRow, int jCol) const;
    WCHAR __fastcall GetAt(const XG_Pos& pos) const;
    // �}�X�̓��e��ݒ肷��B
    void __fastcall SetAt(int i, WCHAR ch);
    void __fastcall SetAt(int iRow, int jCol, WCHAR ch);
    void __fastcall SetAt(const XG_Pos& pos, WCHAR ch);
    // ��ł͂Ȃ��}�X�̌���Ԃ��B
    WCHAR& __fastcall Count();
    WCHAR __fastcall Count() const;
    // �N���A����B
    void __fastcall clear();
    // ���Z�b�g���ăT�C�Y��ݒ肷��B
    void __fastcall ResetAndSetSize(int nRows, int nCols);
    // �����H
    bool __fastcall IsSolution() const;
    // �����Ȃǂ����H
    bool __fastcall IsValid() const;
    // �����Ȃǂ����H�i�ȗ��Łj
    bool __fastcall IsOK() const;
    // �����Ȃǂ����H�i�ȗ��ŁA���}�X�ǉ��Ȃ��j
    bool __fastcall IsNoAddBlackOK() const;
    // �ԍ�������B
    bool __fastcall DoNumbering();
    // �ԍ�������i�`�F�b�N�Ȃ��j�B
    void __fastcall DoNumberingNoCheck();

    // �N���X���[�h�̕�������擾����B
    void __fastcall GetString(std::wstring& str) const;
    bool __fastcall SetString(const std::wstring& strToBeSet);

    // �q���g��������擾����B
    // hint_type 0: �^�e�B
    // hint_type 1: ���R�B
    // hint_type 2: �^�e�ƃ��R�B
    // hint_type 3: HTML�̃^�e�B
    // hint_type 4: HTML�̃��R�B
    // hint_type 5: HTML�̃^�e�ƃ��R�B
    void __fastcall GetHintsStr(
        std::wstring& str, int hint_type, bool bShowAnswer = true) const;

    // �N���X���[�h���󂩂ǂ����B
    bool __fastcall IsEmpty() const;
    // �N���X���[�h�����ׂĖ��ߐs������Ă��邩�ǂ����B
    bool __fastcall IsFulfilled() const;
    // �l���ɍ��}�X�����邩�ǂ����B
    bool __fastcall CornerBlack() const;
    // ���}�X���ׂ荇���Ă��邩�H
    bool __fastcall DoubleBlack() const;
    // �O���������}�X�ň͂܂ꂽ�}�X�����邩�ǂ����H
    bool __fastcall TriBlackAround() const;
    // ���}�X�ŕ��f����Ă��邩�ǂ����H
    bool __fastcall DividedByBlack() const;
    // ���ׂẴp�^�[�����������ǂ������ׂ�B
    XG_EpvCode __fastcall EveryPatternValid1(
        std::vector<std::wstring>& vNotFoundWords,
        XG_Pos& pos, bool bNonBlackCheckSpace) const;
    // ���ׂẴp�^�[�����������ǂ������ׂ�B
    XG_EpvCode __fastcall EveryPatternValid2(
        std::vector<std::wstring>& vNotFoundWords,
        XG_Pos& pos, bool bNonBlackCheckSpace) const;

    // ���}�X��u���邩�H
    bool __fastcall CanPutBlack(int iRow, int jCol) const;

    // �}�X�̎O���������}�X�ň͂܂�Ă��邩�H
    int __fastcall BlacksAround(int iRow, int jCol) const;

    // ���}�X�����Ώ̂��H
    bool IsLineSymmetry() const;

    // ���}�X���_�Ώ̂��H
    bool IsPointSymmetry() const;

    // ���Ύl�A���H
    bool FourDiagonals() const;

    // �c�Ɖ������ւ���B
    void SwapXandY();

    // �^�e�����Ƀp�^�[����ǂݎ��B
    std::wstring __fastcall GetPatternV(const XG_Pos& pos) const;
    // ���R�����Ƀp�^�[����ǂݎ��B
    std::wstring __fastcall GetPatternH(const XG_Pos& pos) const;

public:
    // �}�X���B
    std::vector<WCHAR> m_vCells;
};

bool __fastcall XgDoSaveStandard(HWND hwnd, LPCWSTR pszFile, const XG_Board& board);

namespace std
{
    inline void swap(XG_Board& xw1, XG_Board& xw2) {
        std::swap(xw1.m_vCells, xw2.m_vCells);
    }
}

inline bool operator==(const XG_Board& xw1, const XG_Board& xw2) {
    for (int i = 0; i < xg_nRows; ++i) {
        for (int j = 0; j < xg_nCols; ++j) {
            if (xw1.GetAt(i, j) != xw2.GetAt(i, j))
                return false;
        }
    }
    return true;
}

inline bool operator!=(const XG_Board& xw1, const XG_Board& xw2) {
    return !(xw1 == xw2);
}

// �N���X���[�h�̕`��T�C�Y���v�Z����B
inline void __fastcall XgGetXWordExtent(LPSIZE psiz)
{
    INT nCellSize;
    if (xg_nForDisplay > 0)
        nCellSize = xg_nCellSize * xg_nZoomRate / 100;
    else
        nCellSize = xg_nCellSize;
    psiz->cx = static_cast<int>(xg_nMargin * 2 + xg_nCols * nCellSize);
    psiz->cy = static_cast<int>(xg_nMargin * 2 + xg_nRows * nCellSize);
}

// �N���X���[�h�̕`��T�C�Y���v�Z����B
inline void __fastcall XgGetMarkWordExtent(int count, LPSIZE psiz)
{
    INT nCellSize;
    if (xg_nForDisplay > 0)
        nCellSize = xg_nCellSize * xg_nZoomRate / 100;
    else
        nCellSize = xg_nCellSize;
    psiz->cx = static_cast<int>(xg_nNarrowMargin * 2 + count * nCellSize);
    psiz->cy = static_cast<int>(xg_nNarrowMargin * 2 + 1 * nCellSize);
}

// �N���X���[�h�̃C���[�W���쐬����B
HBITMAP __fastcall XgCreateXWordImage(XG_Board& xw, LPSIZE psiz, bool bCaret);

// ��d�}�X�P���`�悷��B
void __fastcall XgDrawMarkWord(HDC hdc, LPSIZE psiz);

// �N���X���[�h��`�悷��B
void __fastcall XgDrawXWord(XG_Board& xw, HDC hdc, LPSIZE psiz, bool bCaret);

// �������߂�̂��J�n�B
void __fastcall XgStartSolve_AddBlack(void);

// �������߂�̂��J�n�i���}�X�ǉ��Ȃ��j�B
void __fastcall XgStartSolve_NoAddBlack(void);

// �������߂�̂��J�n�i�X�}�[�g�����j�B
void __fastcall XgStartSolveSmart(void);

// �������߂悤�Ƃ�����̌㏈���B
void __fastcall XgEndSolve(void);

// ���}�X�p�^�[���𐶐�����B
void __fastcall XgStartGenerateBlacks(bool sym);

// �X�e�[�^�X�o�[���X�V����B
void __fastcall XgUpdateStatusBar(HWND hwnd);

// ���}�X���[�����`�F�b�N����B
void __fastcall XgRuleCheck(HWND hwnd);

//////////////////////////////////////////////////////////////////////////////

// �N���X���[�h�̖��B
extern XG_Board         xg_xword;

// �N���X���[�h�̉��B
extern XG_Board         xg_solution;

// �N���X���[�h�̉������邩�ǂ����H
extern bool             xg_bSolved;

// �w�b�_�[������B
extern std::wstring     xg_strHeader;
// ���l������B
extern std::wstring     xg_strNotes;

// ���}�X�摜�B
extern HBITMAP xg_hbmBlackCell;
extern HENHMETAFILE xg_hBlackCellEMF;
extern std::wstring xg_strBlackCellImage;

//////////////////////////////////////////////////////////////////////////////
// inline functions

// �R���X�g���N�^�B
inline XG_Board::XG_Board()
{
}

// �R�s�[�R���X�g���N�^�B
inline XG_Board::XG_Board(const XG_Board& xw) : m_vCells(xw.m_vCells)
{
}

// ����B
inline void __fastcall XG_Board::operator=(const XG_Board& xw)
{
    m_vCells = xw.m_vCells;
}

// �R���X�g���N�^�B
inline XG_Board::XG_Board(XG_Board&& xw) : m_vCells(xw.m_vCells)
{
}

// ����B
inline void __fastcall XG_Board::operator=(XG_Board&& xw)
{
    m_vCells = std::move(xw.m_vCells);
}

// �}�X�̓��e���擾����B
inline WCHAR __fastcall XG_Board::GetAt(int i) const
{
    assert(0 <= i && i < xg_nRows * xg_nCols);
    return m_vCells[i];
}

// �}�X�̓��e���擾����B
inline WCHAR __fastcall XG_Board::GetAt(int iRow, int jCol) const
{
    assert(0 <= iRow && iRow < xg_nRows);
    assert(0 <= jCol && jCol < xg_nCols);
    return m_vCells[iRow * xg_nCols + jCol];
}

// �}�X�̓��e���擾����B
inline WCHAR __fastcall XG_Board::GetAt(const XG_Pos& pos) const
{
    return GetAt(pos.m_i, pos.m_j);
}

// ��}�X����Ȃ��}�X�̌���Ԃ��B
inline WCHAR& __fastcall XG_Board::Count()
{
    return m_vCells[xg_nRows * xg_nCols];
}

// ��}�X����Ȃ��}�X�̌���Ԃ��B
inline WCHAR __fastcall XG_Board::Count() const
{
    return m_vCells[xg_nRows * xg_nCols];
}

// �N���X���[�h���󂩂ǂ����B
inline bool __fastcall XG_Board::IsEmpty() const
{
#if 1
    return Count() == 0;
#else
    for (int i = 0; i < xg_nRows; i++)
    {
        for (int j = 0; j < xg_nCols; j++)
        {
            if (GetAt(i, j) != ZEN_SPACE)
                return false;
        }
    }
    return true;
#endif
}

// �N���X���[�h�����ׂĖ��ߐs������Ă��邩�ǂ����B
inline bool __fastcall XG_Board::IsFulfilled() const
{
#if 1
    return Count() == xg_nRows * xg_nCols;
#else
    for (int i = 0; i < xg_nRows; i++)
    {
        for (int j = 0; j < xg_nCols; j++)
        {
            if (GetAt(i, j) == ZEN_SPACE)
                return false;
        }
    }
    return true;
#endif
}

// ���Z�b�g���ăT�C�Y��ݒ肷��B
inline void __fastcall XG_Board::ResetAndSetSize(int nRows, int nCols)
{
    m_vCells.assign(nRows * nCols + 1, ZEN_SPACE);
    m_vCells[nRows * nCols] = 0;
}

// �N���A����B
inline void __fastcall XG_Board::clear()
{
    ResetAndSetSize(xg_nRows, xg_nCols);
}

// �}�X�̓��e��ݒ肷��B
inline void __fastcall XG_Board::SetAt(int i, WCHAR ch)
{
    assert(0 <= i && i < xg_nRows * xg_nCols);
    WCHAR& ch2 = m_vCells[i];
    if (ch2 != ZEN_SPACE)
    {
        if (ch == ZEN_SPACE)
        {
            assert(Count() != 0);
            Count()--;
        }
    }
    else
    {
        if (ch != ZEN_SPACE)
            Count()++;
    }
    ch2 = ch;
}

// �}�X�̓��e��ݒ肷��B
inline void __fastcall XG_Board::SetAt(int iRow, int jCol, WCHAR ch)
{
    assert(0 <= iRow && iRow < xg_nRows);
    assert(0 <= jCol && jCol < xg_nCols);
    WCHAR& ch2 = m_vCells[iRow * xg_nCols + jCol];
    if (ch2 != ZEN_SPACE)
    {
        if (ch == ZEN_SPACE)
        {
            assert(Count() != 0);
            Count()--;
        }
    }
    else
    {
        if (ch != ZEN_SPACE)
            Count()++;
    }
    ch2 = ch;
}

// �}�X�̓��e��ݒ肷��B
inline void __fastcall XG_Board::SetAt(const XG_Pos& pos, WCHAR ch)
{
    return SetAt(pos.m_i, pos.m_j, ch);
}

// �}�X�̎O���������}�X�ň͂܂�Ă��邩�H
inline int __fastcall XG_Board::BlacksAround(int iRow, int jCol) const
{
    assert(0 <= iRow && iRow < xg_nRows);
    assert(0 <= jCol && jCol < xg_nCols);

    int count = 0;
    if (0 <= iRow - 1) {
        if (GetAt(iRow - 1, jCol) == ZEN_BLACK)
            ++count;
    }
    if (iRow + 1 < xg_nRows) {
        if (GetAt(iRow + 1, jCol) == ZEN_BLACK)
            ++count;
    }
    if (0 <= jCol - 1) {
        if (GetAt(iRow, jCol - 1) == ZEN_BLACK)
            ++count;
    }
    if (jCol + 1 < xg_nCols) {
        if (GetAt(iRow, jCol + 1) == ZEN_BLACK)
            ++count;
    }
    return count;
}

// ���}�X��u���邩�H
inline bool __fastcall XG_Board::CanPutBlack(int iRow, int jCol) const
{
    assert(0 <= iRow && iRow < xg_nRows);
    assert(0 <= jCol && jCol < xg_nCols);

    // �l�����ǂ����H
    if (iRow == 0 && jCol == 0)
        return false;
    if (iRow == xg_nRows - 1 && jCol == 0)
        return false;
    if (iRow == xg_nRows - 1 && jCol == xg_nCols - 1)
        return false;
    if (iRow == 0 && jCol == xg_nCols - 1)
        return false;

    // ���}�X���ׂ荇���Ă��܂����H
    if (0 <= iRow - 1 && GetAt(iRow - 1, jCol) == ZEN_BLACK)
        return false;
    if (iRow + 1 < xg_nRows && GetAt(iRow + 1, jCol) == ZEN_BLACK)
        return false;
    if (0 <= jCol - 1 && GetAt(iRow, jCol - 1) == ZEN_BLACK)
        return false;
    if (jCol + 1 < xg_nCols && GetAt(iRow, jCol + 1) == ZEN_BLACK)
        return false;

    // �O���������}�X�ň͂܂ꂽ�}�X���ł��邩�ǂ����H
    BOOL bBlack = (GetAt(iRow, jCol) == ZEN_BLACK);
    if (0 <= iRow - 1) {
        if (BlacksAround(iRow - 1, jCol) >= 2 + bBlack)
            return false;
    }
    if (iRow + 1 < xg_nRows) {
        if (BlacksAround(iRow + 1, jCol) >= 2 + bBlack)
            return false;
    }
    if (0 <= jCol - 1) {
        if (BlacksAround(iRow, jCol - 1) >= 2 + bBlack)
            return false;
    }
    if (jCol + 1 < xg_nCols) {
        if (BlacksAround(iRow, jCol + 1) >= 2 + bBlack)
            return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef __XWORD_HPP__
