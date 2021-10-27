//////////////////////////////////////////////////////////////////////////////
// xword.cpp --- XWord Giver (Japanese Crossword Generator)
// Copyright (C) 2012-2020 Katayama Hirofumi MZ. All Rights Reserved.
// (Japanese, Shift_JIS)

#include "XWordGiver.hpp"
#include "Auto.hpp"

//////////////////////////////////////////////////////////////////////////////
// global variables

// ���O�ɊJ�����N���X���[�h�f�[�^�t�@�C���̃p�X�t�@�C�����B
std::wstring xg_strFileName;

// �Čv�Z���Ȃ����Ă��邩�H
bool xg_bRetrying = false;

// ��̃N���X���[�h�̉��������ꍇ���H
bool xg_bSolvingEmpty = false;

// �v�Z���L�����Z�����ꂽ���H
bool xg_bCancelled = false;

// �N���X���[�h�̉������邩�ǂ����H
bool xg_bSolved = false;

// ������\�����邩�H
bool xg_bShowAnswer = false;

// ���}�X�ǉ��Ȃ����H
bool xg_bNoAddBlack = false;

// �X�}�[�g�������H
bool xg_bSmartResolution = true;

// �X���b�h�̐��B
DWORD               xg_dwThreadCount;

// �X���b�h���B
std::vector<XG_ThreadInfo>    xg_aThreadInfo;

// �X���b�h�̃n���h���B
std::vector<HANDLE>           xg_ahThreads;

// �q���g������B
std::wstring             xg_strHints;

// �w�b�_�[������B
std::wstring             xg_strHeader;
// ���l������B
std::wstring             xg_strNotes;

// �r������̂��߂̃N���e�B�J���Z�N�V�����B
CRITICAL_SECTION    xg_cs;

// ���s�B
const LPCWSTR       xg_pszNewLine = L"\r\n";

// �L�����b�g�̈ʒu�B
XG_Pos              xg_caret_pos = {0, 0};

// �N���X���[�h�̃T�C�Y�B
int                 xg_nRows = 0;
int                 xg_nCols = 0;

// �^�e�ƃ��R�̂����B
std::vector<XG_PlaceInfo> xg_vTateInfo, xg_vYokoInfo;

// �X���ϊ��p�f�[�^�B
const LPCWSTR xg_small[11] = 
{
    L"\x30A1", L"\x30A3", L"\x30A5", L"\x30A7", L"\x30A9", L"\x30C3",
    L"\x30E3", L"\x30E5", L"\x30E7", L"\x30F5", L"\x30F6"
};
const LPCWSTR xg_large[11] = 
{
    L"\x30A2", L"\x30A4", L"\x30A6", L"\x30A8", L"\x30AA", L"\x30C4",
    L"\x30E4", L"\x30E6", L"\x30E8", L"\x30AB", L"\x30B1",
};

// �N���X���[�h�̖��B
XG_Board    xg_xword;

// �N���X���[�h�̉��B
XG_Board    xg_solution;

// �r�b�g�}�b�v�̃n���h���B
HBITMAP     xg_hbmImage = nullptr;

// �q���g�f�[�^�B
std::vector<XG_Hint> xg_vecTateHints, xg_vecYokoHints;

// �Z���̐F�B
COLORREF xg_rgbWhiteCellColor = RGB(255, 255, 255);
COLORREF xg_rgbBlackCellColor = RGB(0x33, 0x33, 0x33);
COLORREF xg_rgbMarkedCellColor = RGB(255, 255, 255);

// ��d�}�X�ɘg��`�����H
bool xg_bDrawFrameForMarkedCell = true;

// ��������H
bool xg_bCharFeed = true;

// �^�e���́H
bool xg_bTateInput = false;

// �����̑傫���i���j�B
INT xg_nCellCharPercents = DEF_SMALL_CHAR_SIZE;

// �����������̑傫���i���j�B
INT xg_nSmallCharPercents = DEF_SMALL_CHAR_SIZE;

// ���}�X�摜�B
HBITMAP xg_hbmBlackCell = NULL;
HENHMETAFILE xg_hBlackCellEMF = NULL;
std::wstring xg_strBlackCellImage;

// �r���[���[�h�B
XG_VIEW_MODE xg_nViewMode = XG_VIEW_NORMAL;

//////////////////////////////////////////////////////////////////////////////
// static variables

// �c�Ɖ��𔽓]���Ă��邩�H
static bool s_bSwapped = false;

// �J�M�̓������͂ޘg�B
static const LPCWSTR s_szBeginWord = L"\x226A", s_szEndWord = L"\x226B";

//////////////////////////////////////////////////////////////////////////////

// ��₪���邩�H
template <bool t_alternative>
bool __fastcall XgAnyCandidateAddBlack(const std::wstring& pattern)
{
    // �p�^�[���̒����B
    const int patlen = static_cast<int>(pattern.size());
    assert(patlen >= 2);

#ifndef NDEBUG
    // �p�^�[���ɍ��}�X���Ȃ����Ƃ����肷��B
    // �p�^�[���ɋ󔒂����邱�Ƃ����肷��B
    bool bSpaceFound = false;
    for (int i = 0; i < patlen; i++) {
        assert(pattern[i] != ZEN_BLACK);
        if (pattern[i] != ZEN_SPACE)
            bSpaceFound = true;
    }
    assert(bSpaceFound);
#endif

    // �p�^�[�����R���ȏォ�H
    if (patlen > 2) {
        // �Ǘ����������}�X�����[���H
        if (pattern[0] != ZEN_SPACE && pattern[1] == ZEN_SPACE) {
            return true;    // �Ǘ����������}�X���������B
        } else {
            bool bCharNotFound = true;  // �����}�X���Ȃ��������H

            // �Ǘ����������}�X�����ɂ��邩�𒲂ׂ�B
            // ���łɕ����}�X���r���ɂ��邩���ׂ�B
            for (int j = 1; j < patlen - 1; j++) {
                if (pattern[j] != ZEN_SPACE) {
                    if (pattern[j - 1] == ZEN_SPACE && pattern[j + 1] == ZEN_SPACE) {
                        return true;    // �Ǘ����������}�X���������B
                    }
                    bCharNotFound = false;
                    break;
                }
            }

            if (bCharNotFound) {
                // �Ǘ����������}�X���E�[���H
                if (pattern[patlen - 1] != ZEN_SPACE && pattern[patlen - 2] == ZEN_SPACE) {
                    return true;    // �Ǘ����������}�X���������B
                }
            }
        }
    }

    // ���ׂĂ̒P��ɂ��Ē��ׂ�B
    for (const auto& data : (t_alternative ? xg_dict_2 : xg_dict_1)) {
        const std::wstring& word = data.m_word;
        const int wordlen = static_cast<int>(word.size());

        // �p�^�[�����P��̕��������ꍇ�A�X�L�b�v����B
        if (wordlen > patlen)
            continue;

        // �P��̒u�����Ԃɂ��Ē��ׂ�B
        const int patlen_minus_wordlen = patlen - wordlen;
        for (int j = 0; j <= patlen_minus_wordlen; j++) {
            // �����P��̈ʒu�̑O��ɕ�������������X�L�b�v����B
            if (j > 0 && pattern[j - 1] != ZEN_SPACE)
                continue;
            if (j < patlen_minus_wordlen && pattern[j + wordlen] != ZEN_SPACE)
                continue;

            // ���[j, j + wordlen - 1]�ɕ����}�X�����邩�H
            // �P�ꂪ�Ƀ}�b�`���邩�H
            bool bCharFound = false;
            for (int k = 0; k < wordlen; k++) {
                if (pattern[j + k] != ZEN_SPACE) {
                    bCharFound = true;
                    if (pattern[j + k] != word[k]) {
                        // �}�b�`���Ȃ������B
                        goto break_continue;
                    }
                }
            }
            // �}�b�`�����B
            if (bCharFound)
                return bCharFound;  // �������B
break_continue:;
        }
    }

    return false;   // �Ȃ������B
}

// ��₪���邩�H�i���}�X�ǉ��Ȃ��j
template <bool t_alternative>
bool __fastcall XgAnyCandidateNoAddBlack(const std::wstring& pattern)
{
    // �p�^�[���̒����B
    const int patlen = static_cast<int>(pattern.size());
    assert(patlen >= 2);

#ifndef NDEBUG
    // �p�^�[���ɍ��}�X���Ȃ����Ƃ����肷��B
    // �p�^�[���ɋ󔒂����邱�Ƃ����肷��B
    bool bSpaceFound = false;
    for (int i = 0; i < patlen; i++) {
        assert(pattern[i] != ZEN_BLACK);
        if (pattern[i] != ZEN_SPACE)
            bSpaceFound = true;
    }
    assert(bSpaceFound);
#endif

    // ���ׂĂ̒P��ɂ��Ē��ׂ�B
    for (const auto& data : (t_alternative ? xg_dict_2 : xg_dict_1)) {
        // �P��̒����B
        const std::wstring& word = data.m_word;
        const int wordlen = static_cast<int>(word.size());

        // �p�^�[���ƒP��̒������قȂ�Ƃ��A�X�L�b�v����B
        if (wordlen != patlen)
            continue;

        // �P�ꂪ�}�b�`���邩�H
        // ���[0, wordlen - 1]�ɕ����}�X�����邩�H
        bool bCharFound = false;
        for (int k = 0; k < wordlen; k++) {
            if (pattern[k] != ZEN_SPACE) {
                bCharFound = true;
                if (pattern[k] != word[k]) {
                    // �}�b�`���Ȃ������B
                    goto break_continue;
                }
            }
        }
        // �}�b�`�����B
        if (bCharFound)
            return bCharFound;      // �������B
break_continue:;
    }

    return false;   // �Ȃ������B
}

// ��₪���邩�H�i���}�X�ǉ��Ȃ��A���ׂċ󔒁j
bool __fastcall XgAnyCandidateWholeSpace(int patlen)
{
    // ���ׂĂ̒P��ɂ��Ē��ׂ�B
    for (const auto& data : xg_dict_1) {
        const std::wstring& word = data.m_word;
        const int wordlen = static_cast<int>(word.size());
        if (wordlen == patlen)
            return true;    // �������B
    }
    for (const auto& data : xg_dict_2) {
        const std::wstring& word = data.m_word;
        const int wordlen = static_cast<int>(word.size());
        if (wordlen == patlen)
            return true;    // �������B
    }
    return false;   // �Ȃ������B
}

// �l���ɍ��}�X�����邩�ǂ����B
bool __fastcall XG_Board::CornerBlack() const
{
    return (GetAt(0, 0) == ZEN_BLACK ||
            GetAt(xg_nRows - 1, 0) == ZEN_BLACK ||
            GetAt(xg_nRows - 1, xg_nCols - 1) == ZEN_BLACK ||
            GetAt(0, xg_nCols - 1) == ZEN_BLACK);
}

// ���}�X���ׂ荇���Ă��邩�H
bool __fastcall XG_Board::DoubleBlack() const
{
    const int n1 = xg_nCols - 1;
    const int n2 = xg_nRows - 1;
    int i = xg_nRows;
    for (--i; i >= 0; --i) {
        for (int j = 0; j < n1; j++) {
            if (GetAt(i, j) == ZEN_BLACK && GetAt(i, j + 1) == ZEN_BLACK)
                return true;    // �ׂ荇���Ă����B
        }
    }
    int j = xg_nCols;
    for (--j; j >= 0; --j) {
        for (int i = 0; i < n2; i++) {
            if (GetAt(i, j) == ZEN_BLACK && GetAt(i + 1, j) == ZEN_BLACK)
                return true;    // �ׂ荇���Ă����B
        }
    }
    return false;   // �ׂ荇���Ă��Ȃ������B
}

// �O���������}�X�ň͂܂ꂽ�}�X�����邩�ǂ����H
bool __fastcall XG_Board::TriBlackAround() const
{
    for (int i = xg_nRows - 2; i >= 1; --i) {
        for (int j = xg_nCols - 2; j >= 1; --j) {
            if ((GetAt(i - 1, j) == ZEN_BLACK) + (GetAt(i + 1, j) == ZEN_BLACK) + 
                (GetAt(i, j - 1) == ZEN_BLACK) + (GetAt(i, j + 1) == ZEN_BLACK) >= 3)
            {
                return true;    // �������B
            }
        }
    }
    return false;   // �Ȃ������B
}

// ���}�X�ŕ��f����Ă��邩�ǂ����H
bool __fastcall XG_Board::DividedByBlack() const
{
    const INT nRows = xg_nRows, nCols = xg_nCols;
    INT nCount = nRows * nCols;

    // �e�}�X�ɑΉ�����t���O�Q�B
    std::vector<BYTE> pb(nCount, 0);

    // �ʒu�̃L���[�B
    // ���}�X�ł͂Ȃ��}�X��T���Apositions�ɒǉ�����B
    std::queue<XG_Pos> positions;
    if (GetAt(0, 0) != ZEN_BLACK) {
        positions.emplace(0, 0);
    } else {
        for (INT i = 0; i < nRows; ++i) {
            for (INT j = 0; j < nCols; ++j) {
                if (GetAt(i, j) != ZEN_BLACK) {
                    positions.emplace(i, j);
                    i = nRows;
                    j = nCols;
                    break;
                }
            }
        }
    }

    // �A���̈�̓h��Ԃ��B
    while (!positions.empty()) {
        // �ʒu���L���[�̈�ԏォ����o���B
        XG_Pos pos = positions.front();
        positions.pop();
        // �t���O�������Ă��Ȃ����H
        if (!pb[pos.m_i * nCols + pos.m_j]) {
            // �t���O�𗧂Ă�B
            pb[pos.m_i * nCols + pos.m_j] = 1;
            // ��B
            if (pos.m_i > 0 && GetAt(pos.m_i - 1, pos.m_j) != ZEN_BLACK)
                positions.emplace(pos.m_i - 1, pos.m_j);
            // ���B
            if (pos.m_i < nRows - 1 && GetAt(pos.m_i + 1, pos.m_j) != ZEN_BLACK)
                positions.emplace(pos.m_i + 1, pos.m_j);
            // ���B
            if (pos.m_j > 0 && GetAt(pos.m_i, pos.m_j - 1) != ZEN_BLACK)
                positions.emplace(pos.m_i, pos.m_j - 1);
            // �E�B
            if (pos.m_j < nCols - 1 && GetAt(pos.m_i, pos.m_j + 1) != ZEN_BLACK)
                positions.emplace(pos.m_i, pos.m_j + 1);
        }
    }

    // ���ׂẴ}�X�ɂ��āB
    while (nCount-- > 0) {
        // �t���O�������Ă��Ȃ��̂ɁA���}�X�ł͂Ȃ��}�X����������A���s�B
        if (pb[nCount] == 0 && GetAt(nCount) != ZEN_BLACK) {
            // �t���O�Q������B
            return true;    // ���f����Ă���B
        }
    }

    return false;   // ���f����Ă��Ȃ��B
}

// ���ׂẴp�^�[�����������ǂ������ׂ�B
XG_EpvCode __fastcall XG_Board::EveryPatternValid1(
    std::vector<std::wstring>& vNotFoundWords,
    XG_Pos& pos, bool bNonBlackCheckSpace) const
{
    const int nRows = xg_nRows, nCols = xg_nCols;

    // �g�����P��̏W���B
    std::unordered_set<std::wstring> used_words;
    // �X�s�[�h�̂��߂ɁA�\��B
    used_words.reserve(nRows * nCols / 4);

    // �P��x�N�^�[����ł��邱�Ƃ����肷��B
    assert(vNotFoundWords.empty());
    //vNotFoundWords.clear();

    // �e�s�ɂ��āA�������ɃX�L��������B
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols - 1; j++) {
            // ���}�X�ł͂Ȃ��A�������܂܂�Ă���A�������Q�}�X���H
            const WCHAR ch1 = GetAt(i, j), ch2 = GetAt(i, j + 1);
            if (ch1 != ZEN_BLACK && ch2 != ZEN_BLACK &&
                (ch1 != ZEN_SPACE || ch2 != ZEN_SPACE))
            {
                // �������u���郈�R�����̋��[lo, hi]�����߂�B
                int lo = j;
                while (lo > 0) {
                    if (GetAt(i, lo - 1) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (j + 1 < nCols) {
                    if (GetAt(i, j + 1) == ZEN_BLACK)
                        break;
                    j++;
                }
                const int hi = j;
                j++;

                // �p�^�[���𐶐�����B
                bool bSpaceFound = false;
                std::wstring pattern;
                for (int k = lo; k <= hi; k++) {
                    WCHAR ch = GetAt(i, k);
                    if (ch == ZEN_SPACE) {
                        bSpaceFound = true;
                        for (; k <= hi; k++)
                            pattern += GetAt(i, k);
                        break;
                    }
                    pattern += ch;
                }

                // �X�y�[�X�����������H
                if (bSpaceFound) {
                    // �p�^�[���Ƀ}�b�`�����₪���݂��Ȃ���Ύ��s����B
                    if (xg_bNoAddBlack) {
                        if (!XgAnyCandidateNoAddBlack<false>(pattern) &&
                            !XgAnyCandidateNoAddBlack<true>(pattern))
                        {
                            // �}�b�`���Ȃ������ʒu�B
                            pos = XG_Pos(i, lo);
                            return xg_epv_PATNOTMATCH;  // �}�b�`���Ȃ������B
                        }
                    } else {
                        if (!XgAnyCandidateAddBlack<false>(pattern) &&
                            !XgAnyCandidateAddBlack<true>(pattern))
                        {
                            // �}�b�`���Ȃ������ʒu�B
                            pos = XG_Pos(i, lo);
                            return xg_epv_PATNOTMATCH;  // �}�b�`���Ȃ������B
                        }
                    }
                } else {
                    // ���łɎg�p�����P�ꂪ���邩�H
                    if (used_words.count(pattern))
                        return xg_epv_DOUBLEWORD;   // �P��̏d���̂��߁A���s�B

                    // �񕪒T���ŒP�ꂪ���邩�ǂ������ׂ�B
                    if (!XgWordDataExists(xg_dict_1, XG_WordData(pattern)) &&
                        !XgWordDataExists(xg_dict_2, XG_WordData(pattern)))
                    {
                        // �o�^����Ă��Ȃ��P��Ȃ̂ŁA�o�^����B
                        vNotFoundWords.emplace_back(pattern);
                    }

                    // ��x�g�����P��Ƃ��ēo�^�B
                    used_words.emplace(pattern);
                }
            } else if (bNonBlackCheckSpace && ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // ���R�����̋��[lo, hi]�����߂�B
                int lo = j;
                while (lo > 0) {
                    if (GetAt(i, lo - 1) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (j + 1 < nCols) {
                    if (GetAt(i, j + 1) == ZEN_BLACK)
                        break;
                    j++;
                }
                const int hi = j;
                j++;

                // �p�^�[���𐶐�����B
                // ��󔒂����邩�H
                bool bNonSpaceFound = false;
                int patlen = 0;
                for (int k = lo; k <= hi; k++) {
                    WCHAR ch = GetAt(i, k);
                    if (ch != ZEN_SPACE) {
                        bNonSpaceFound = true;
                        break;
                    }
                    patlen++;
                }

                if (!bNonSpaceFound) {
                    // ��󔒂��Ȃ��B
                    // �p�^�[���Ƀ}�b�`�����₪���݂��Ȃ���Ύ��s����B
                    if (!XgAnyCandidateWholeSpace(patlen)) {
                        // �}�b�`���Ȃ������ʒu�B
                        pos = XG_Pos(i, lo);
                        return xg_epv_LENGTHMISMATCH;   // �}�b�`���Ȃ������B
                    }
                }
            }
        }
    }

    // �e��ɂ��āA�c�����ɃX�L��������B
    for (int j = 0; j < nCols; j++) {
        for (int i = 0; i < nRows - 1; i++) {
            // ���}�X�ł͂Ȃ��A�������܂܂�Ă���A�������Q�}�X���H
            const WCHAR ch1 = GetAt(i, j), ch2 = GetAt(i + 1, j);
            if (ch1 != ZEN_BLACK && ch2 != ZEN_BLACK &&
                (ch1 != ZEN_SPACE || ch2 != ZEN_SPACE))
            {
                // �������u����^�e�����̋��[lo, hi]�����߂�B
                int lo = i;
                while (lo > 0) {
                    if (GetAt(lo - 1, j) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (i + 1 < nRows) {
                    if (GetAt(i + 1, j) == ZEN_BLACK)
                        break;
                    i++;
                }
                const int hi = i;
                i++;

                // �p�^�[���𐶐�����B
                bool bSpaceFound = false;
                std::wstring pattern;
                for (int k = lo; k <= hi; k++) {
                    WCHAR ch = GetAt(k, j);
                    if (ch == ZEN_SPACE) {
                        bSpaceFound = true;
                        for (; k <= hi; k++)
                            pattern += GetAt(k, j);
                        break;
                    }
                    pattern += ch;
                }

                // �X�y�[�X�����������H
                if (bSpaceFound) {
                    // �p�^�[���Ƀ}�b�`�����₪���݂��Ȃ���Ύ��s����B
                    if (xg_bNoAddBlack) {
                        if (!XgAnyCandidateNoAddBlack<false>(pattern) &&
                            !XgAnyCandidateNoAddBlack<true>(pattern))
                        {
                            // �}�b�`���Ȃ������ʒu�B
                            pos = XG_Pos(lo, j);
                            return xg_epv_PATNOTMATCH;  // �}�b�`���Ȃ������B
                        }
                    } else {
                        if (!XgAnyCandidateAddBlack<false>(pattern) &&
                            !XgAnyCandidateAddBlack<true>(pattern))
                        {
                            // �}�b�`���Ȃ������ʒu�B
                            pos = XG_Pos(lo, j);
                            return xg_epv_PATNOTMATCH;  // �}�b�`���Ȃ������B
                        }
                    }
                } else {
                    // ���łɎg�p�����P�ꂪ���邩�H
                    if (used_words.count(pattern)) {
                        return xg_epv_DOUBLEWORD;   // �P��̏d���̂��߁A���s�B
                    }

                    // �񕪒T���ŒP�ꂪ���邩�ǂ������ׂ�B
                    if (!XgWordDataExists(xg_dict_1, XG_WordData(pattern)) &&
                        !XgWordDataExists(xg_dict_2, XG_WordData(pattern)))
                    {
                        // �o�^����Ă��Ȃ��P��Ȃ̂ŁA�o�^����B
                        vNotFoundWords.emplace_back(pattern);
                    }

                    // ��x�g�����P��Ƃ��ēo�^�B
                    used_words.emplace(pattern);
                }
            } else if (bNonBlackCheckSpace && ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // �^�e�����̋��[lo, hi]�����߂�B
                int lo = i;
                while (lo > 0) {
                    if (GetAt(lo - 1, j) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (i + 1 < nRows) {
                    if (GetAt(i + 1, j) == ZEN_BLACK)
                        break;
                    i++;
                }
                const int hi = i;
                i++;

                // �p�^�[���𐶐�����B
                // ��󔒂����邩�H
                bool bNonSpaceFound = false;
                int patlen = 0;
                for (int k = lo; k <= hi; k++) {
                    WCHAR ch = GetAt(k, j);
                    if (ch != ZEN_SPACE) {
                        bNonSpaceFound = true;
                        break;
                    }
                    patlen++;
                }

                if (!bNonSpaceFound) {
                    // ��󔒂��Ȃ��B
                    // �p�^�[���Ƀ}�b�`�����₪���݂��Ȃ���Ύ��s����B
                    if (!XgAnyCandidateWholeSpace(patlen)) {
                        // �}�b�`���Ȃ������ʒu�B
                        pos = XG_Pos(lo, j);
                        return xg_epv_LENGTHMISMATCH;   // ��������v���Ȃ������B
                    }
                }
            }
        }
    }

    return xg_epv_SUCCESS;      // �����B
}

// ���ׂẴp�^�[�����������ǂ������ׂ�B
XG_EpvCode __fastcall XG_Board::EveryPatternValid2(
    std::vector<std::wstring>& vNotFoundWords,
    XG_Pos& pos, bool bNonBlackCheckSpace) const
{
    const int nRows = xg_nRows;
    const int nCols = xg_nCols;
    XG_Pos pos2 = pos;
    int& i = pos2.m_i;
    int& j = pos2.m_j;

    // �g�����P��̏W���B
    std::unordered_set<std::wstring> used_words;
    // �X�s�[�h�̂��߁A�\��B
    used_words.reserve(nRows * nCols / 4);

    // �P��x�N�^�[����ł��邱�Ƃ����肷��B
    assert(vNotFoundWords.empty());
    //vNotFoundWords.clear();

    // �e�s�ɂ��āA�������ɃX�L��������B
    const int nColsMinusOne = nCols - 1;
    for (i = 0; i < nRows; i++) {
        for (j = 0; j < nColsMinusOne; j++) {
            // ���}�X�ł͂Ȃ��A�������܂܂�Ă���A�������Q�}�X���H
            const WCHAR ch1 = GetAt(i, j), ch2 = GetAt(i, j + 1);
            if (ch1 != ZEN_BLACK && ch2 != ZEN_BLACK &&
                (ch1 != ZEN_SPACE || ch2 != ZEN_SPACE))
            {
                // �������u���郈�R�����̋��[lo, hi]�����߂�B
                int lo = j;
                while (lo > 0) {
                    if (GetAt(i, --lo) == ZEN_BLACK) {
                        ++lo;
                        break;
                    }
                }
                while (j + 1 < nCols) {
                    if (GetAt(i, ++j) == ZEN_BLACK) {
                        --j;
                        break;
                    }
                }
                const int hi = j;
                j++;

                // �p�^�[���𐶐�����B
                bool bSpaceFound = false;
                std::wstring pattern;
                for (int k = lo; k <= hi; k++) {
                    WCHAR ch = GetAt(i, k);
                    if (ch == ZEN_SPACE) {
                        bSpaceFound = true;
                        for (; k <= hi; k++)
                            pattern += GetAt(i, k);
                        break;
                    }
                    pattern += ch;
                }

                // �X�y�[�X�����������H
                if (bSpaceFound) {
                    // �p�^�[���Ƀ}�b�`�����₪���݂��Ȃ���Ύ��s����B
                    if (xg_bNoAddBlack) {
                        if (!XgAnyCandidateNoAddBlack<false>(pattern) &&
                            !XgAnyCandidateNoAddBlack<true>(pattern))
                        {
                            // �}�b�`���Ȃ������ʒu�B
                            j = lo;
                            pos = pos2;
                            return xg_epv_PATNOTMATCH;  // �}�b�`���Ȃ������B
                        }
                    } else {
                        if (!XgAnyCandidateAddBlack<false>(pattern) &&
                            !XgAnyCandidateAddBlack<true>(pattern))
                        {
                            // �}�b�`���Ȃ������ʒu�B
                            j = lo;
                            pos = pos2;
                            return xg_epv_PATNOTMATCH;  // �}�b�`���Ȃ������B
                        }
                    }
                } else {
                    // ���łɎg�p�����P�ꂪ���邩�H
                    if (used_words.count(pattern)) {
                        pos = pos2;
                        return xg_epv_DOUBLEWORD;   // �P��̏d���̂��߁A���s�B
                    }

                    // �񕪒T���ŒP�ꂪ���邩�ǂ������ׂ�B
                    if (!XgWordDataExists(xg_dict_1, XG_WordData(pattern)) &&
                        !XgWordDataExists(xg_dict_2, XG_WordData(pattern)))
                    {
                        // �o�^����Ă��Ȃ��P��Ȃ̂ŁA�o�^����B
                        vNotFoundWords.emplace_back(std::move(pattern));
                        pos = pos2;
                        return xg_epv_NOTFOUNDWORD; // �o�^����Ă��Ȃ��P�ꂪ�������B
                    }

                    // ��x�g�����P��Ƃ��ēo�^�B
                    used_words.emplace(std::move(pattern));
                }
            } else if (bNonBlackCheckSpace && ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // ���R�����̋��[lo, hi]�����߂�B
                int lo = j;
                while (lo > 0) {
                    if (GetAt(i, --lo) == ZEN_BLACK) {
                        ++lo;
                        break;
                    }
                    lo--;
                }
                while (j + 1 < nCols) {
                    if (GetAt(i, ++j) == ZEN_BLACK) {
                        --j;
                        break;
                    }
                }
                const int hi = j;
                j++;

                // �p�^�[���𐶐�����B
                // ��󔒂����邩�H
                bool bNonSpaceFound = false;
                int patlen = 0;
                for (int k = lo; k <= hi; k++) {
                    WCHAR ch = GetAt(i, k);
                    if (ch != ZEN_SPACE) {
                        bNonSpaceFound = true;
                        break;
                    }
                    patlen++;
                }

                if (!bNonSpaceFound) {
                    // ��󔒂��Ȃ��B
                    // �p�^�[���Ƀ}�b�`�����₪���݂��Ȃ���Ύ��s����B
                    if (!XgAnyCandidateWholeSpace(patlen)) {
                        // �}�b�`���Ȃ������ʒu�B
                        j = lo;
                        pos = pos2;
                        return xg_epv_LENGTHMISMATCH;   // �������}�b�`���Ȃ������B
                    }
                }
            }
        }
    }

    // �e��ɂ��āA�c�����ɃX�L��������B
    const int nRowsMinusOne = nRows - 1;
    for (j = 0; j < nCols; j++) {
        for (i = 0; i < nRowsMinusOne; i++) {
            // ���}�X�ł͂Ȃ��A�������܂܂�Ă���A�������Q�}�X���H
            const WCHAR ch1 = GetAt(i, j), ch2 = GetAt(i + 1, j);
            if (ch1 != ZEN_BLACK && ch2 != ZEN_BLACK &&
                (ch1 != ZEN_SPACE || ch2 != ZEN_SPACE))
            {
                // �������u����^�e�����̋��[lo, hi]�����߂�B
                int lo = i;
                while (lo > 0) {
                    if (GetAt(--lo, j) == ZEN_BLACK) {
                        ++lo;
                        break;
                    }
                }
                while (i + 1 < nRows) {
                    if (GetAt(++i, j) == ZEN_BLACK) {
                        --i;
                        break;
                    }
                }
                const int hi = i;
                i++;

                // �p�^�[���𐶐�����B
                bool bSpaceFound = false;
                std::wstring pattern;
                for (int k = lo; k <= hi; k++) {
                    WCHAR ch = GetAt(k, j);
                    if (ch == ZEN_SPACE) {
                        bSpaceFound = true;
                        for (; k <= hi; k++)
                            pattern += GetAt(k, j);
                        break;
                    }
                    pattern += ch;
                }

                // �X�y�[�X�����������H
                if (bSpaceFound) {
                    // �p�^�[���Ƀ}�b�`�����₪���݂��Ȃ���Ύ��s����B
                    if (xg_bNoAddBlack) {
                        if (!XgAnyCandidateNoAddBlack<false>(pattern) &&
                            !XgAnyCandidateNoAddBlack<true>(pattern))
                        {
                            // �}�b�`���Ȃ������ʒu�B
                            i = lo;
                            pos = pos2;
                            return xg_epv_PATNOTMATCH;  // �}�b�`���Ȃ������B
                        }
                    } else {
                        if (!XgAnyCandidateAddBlack<false>(pattern) &&
                            !XgAnyCandidateAddBlack<true>(pattern))
                        {
                            // �}�b�`���Ȃ������ʒu�B
                            i = lo;
                            pos = pos2;
                            return xg_epv_PATNOTMATCH;  // �}�b�`���Ȃ������B
                        }
                    }
                } else {
                    // ���łɎg�p�����P�ꂪ���邩�H
                    if (used_words.count(pattern)) {
                        pos = pos2;
                        return xg_epv_DOUBLEWORD;
                    }

                    // �񕪒T���ŒP�ꂪ���邩�ǂ������ׂ�B
                    if (!XgWordDataExists(xg_dict_1, XG_WordData(pattern)) &&
                        !XgWordDataExists(xg_dict_2, XG_WordData(pattern)))
                    {
                        // �o�^����Ă��Ȃ��P��Ȃ̂ŁA�o�^����B
                        vNotFoundWords.emplace_back(std::move(pattern));
                        pos = pos2;
                        return xg_epv_NOTFOUNDWORD; // �o�^����Ă��Ȃ��P�ꂪ�������Bs
                    }

                    // ��x�g�����P��Ƃ��ēo�^�B
                    used_words.emplace(std::move(pattern));
                }
            } else if (bNonBlackCheckSpace && ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // �^�e�����̋��[lo, hi]�����߂�B
                int lo = i;
                while (lo > 0) {
                    if (GetAt(--lo, j) == ZEN_BLACK) {
                        ++lo;
                        break;
                    }
                }
                while (i + 1 < nRows) {
                    if (GetAt(++i, j) == ZEN_BLACK) {
                        --i;
                        break;
                    }
                }
                const int hi = i;
                i++;

                // �p�^�[���𐶐�����B
                // ��󔒂����邩�H
                bool bNonSpaceFound = false;
                int patlen = 0;
                for (int k = lo; k <= hi; k++) {
                    WCHAR ch = GetAt(k, j);
                    if (ch != ZEN_SPACE) {
                        bNonSpaceFound = true;
                        break;
                    }
                    patlen++;
                }

                if (!bNonSpaceFound) {
                    // ��󔒂��Ȃ��B
                    // �p�^�[���Ƀ}�b�`�����₪���݂��Ȃ���Ύ��s����B
                    if (!XgAnyCandidateWholeSpace(patlen)) {
                        // �}�b�`���Ȃ������ʒu�B
                        i = lo;
                        pos = pos2;
                        return xg_epv_LENGTHMISMATCH;   // �������}�b�`���Ȃ������B
                    }
                }
            }
        }
    }

    return xg_epv_SUCCESS;      // �����B
}

// �����Ȃǂ����H
inline bool __fastcall XG_Board::IsValid() const
{
    if ((xg_nRules & RULE_DONTCORNERBLACK) && CornerBlack())
        return false;
    if ((xg_nRules & RULE_DONTDOUBLEBLACK) && DoubleBlack())
        return false;
    if ((xg_nRules & RULE_DONTTRIDIRECTIONS) && TriBlackAround())
        return false;
    if ((xg_nRules & RULE_DONTDIVIDE) && DividedByBlack())
        return false;
    if (xg_nRules & RULE_DONTTHREEDIAGONALS) {
        if (ThreeDiagonals())
            return false;
    } else if (xg_nRules & RULE_DONTFOURDIAGONALS) {
        if (FourDiagonals())
            return false;
    }

    // �N���X���[�h�Ɋ܂܂��P��̃`�F�b�N�B
    XG_Pos pos;
    std::vector<std::wstring> vNotFoundWords;
    vNotFoundWords.reserve(xg_nRows * xg_nCols / 4);
    XG_EpvCode code = EveryPatternValid2(vNotFoundWords, pos, false);
    if (code != xg_epv_SUCCESS || !vNotFoundWords.empty())
        return false;

    // ��̃N���X���[�h�������Ă���Ƃ��́A���f�ւ��`�F�b�N����K�v�͂Ȃ��B
    // ���f�ցB
    if (!xg_bSolvingEmpty && DividedByBlack())
        return false;

    return true;    // �����B
}

// �����Ȃǂ����H�i�ȗ��ŁA���}�X�ǉ��Ȃ��j
bool __fastcall XG_Board::IsNoAddBlackOK() const
{
    // �N���X���[�h�Ɋ܂܂��P��̃`�F�b�N�B
    XG_Pos pos;
    std::vector<std::wstring> vNotFoundWords;
    vNotFoundWords.reserve(xg_nRows * xg_nCols / 4);
    XG_EpvCode code = EveryPatternValid2(vNotFoundWords, pos, false);
    if (code != xg_epv_SUCCESS || !vNotFoundWords.empty())
        return false;

    // ��̃N���X���[�h�������Ă���Ƃ��́A���f�ւ��`�F�b�N����K�v�͂Ȃ��B

    return true;    // �����B
}

// �ԍ�������B
bool __fastcall XG_Board::DoNumbering()
{
    const int nRows = xg_nRows;
    const int nCols = xg_nCols;

    // �g�����P��̏W���B
    std::unordered_set<std::wstring> used_words;
    // �X�s�[�h�̂��߁A�\��B
    used_words.reserve(nRows * nCols / 4);

    // �P��f�[�^�B
    XG_WordData wd;

#ifndef NDEBUG
    // ��}�X���Ȃ����Ƃ����肷��B
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            assert(GetAt(i, j) != ZEN_SPACE);
        }
    }
#endif

    // �J�M���N���A����B
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();

    // �e��ɂ��āA�c�����ɃX�L��������B
    for (int j = 0; j < nCols; j++) {
        for (int i = 0; i < nRows - 1; i++) {
            // �����}�X�̘A���������������H
            if (GetAt(i, j) != ZEN_BLACK && GetAt(i + 1, j) != ZEN_BLACK) {
                int lo, hi;

                // �P�ꂪ�u�����Ԃ����߂�B
                lo = hi = i;
                while (hi + 1 < nRows) {
                    if (GetAt(hi + 1, j) == ZEN_BLACK)
                        break;
                    hi++;
                }
                assert(hi == nRows - 1 || GetAt(hi + 1, j) == ZEN_BLACK);

                // ���̈ʒu��ݒ肷��B
                i = hi + 1;

                // �󔒂����邩�𒲂ׂ�Ɠ����ɁA���̋�Ԃɂ���P����擾����B
                std::wstring word;
                for (int k = lo; k <= hi; k++) {
                    if (GetAt(k, j) == ZEN_SPACE)
                        goto space_found_1;

                    word += GetAt(k, j);
                }

                // �󔒂�������Ȃ������B
                // ���łɎg�p�����P�ꂪ���邩�H
                if (used_words.count(word)) {
                    return false;   // ���łɎg�p�����P�ꂪ�g��ꂽ�̂ŁA���s�B
                }

                // �񕪒T���ŒP�ꂪ���邩�ǂ������ׂ�B
                wd.m_word = word;
                if (XgWordDataExists(xg_dict_1, wd) || XgWordDataExists(xg_dict_2, wd)) {
                    // ��x�g�����P��Ƃ��ēo�^�B
                    used_words.emplace(word);

                    // �c�̃J�M�ɓo�^�B
                    xg_vTateInfo.emplace_back(lo, j, std::move(word));
                } else {
                    return false;   // �o�^����Ă��Ȃ��P�ꂪ�������̂Ŏ��s�B
                }
space_found_1:;
            }
        }
    }

    // �e�s�ɂ��ĉ������ɃX�L��������B
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols - 1; j++) {
            // �����}�X�̘A���������������H
            if (GetAt(i, j) != ZEN_BLACK && GetAt(i, j + 1) != ZEN_BLACK) {
                int lo, hi;

                // �P�ꂪ�u�����Ԃ����߂�B
                lo = hi = j;
                while (hi + 1 < nCols) {
                    if (GetAt(i, hi + 1) == ZEN_BLACK)
                        break;
                    hi++;
                }
                assert(hi == nCols - 1 || GetAt(i, hi + 1) == ZEN_BLACK);

                // ���̈ʒu��ݒ肷��B
                j = hi + 1;

                // �󔒂����邩�𒲂ׂ�Ɠ����ɁA���̋�Ԃɂ���P����擾����B
                std::wstring word;
                for (int k = lo; k <= hi; k++) {
                    if (GetAt(i, k) == ZEN_SPACE)
                        goto space_found_2;

                    word += GetAt(i, k);
                }

                // �󔒂�������Ȃ������B
                // ���łɎg�p�����P�ꂪ���邩�H
                if (used_words.count(word)) {
                    return false;   // �g�p�����P�ꂪ�g��ꂽ���߁A���s�B
                }

                // �񕪒T���ŒP�ꂪ���邩�ǂ������ׂ�B
                wd.m_word = word;
                if (XgWordDataExists(xg_dict_1, wd) || XgWordDataExists(xg_dict_2, wd)) {
                    // ��x�g�����P��Ƃ��ēo�^�B
                    used_words.emplace(word);

                    // ���̃J�M�Ƃ��ēo�^�B
                    xg_vYokoInfo.emplace_back(i, lo, std::move(word));
                } else {
                    return false;   // �o�^����Ă��Ȃ��P��̂��߁A���s�B
                }
space_found_2:;
            }
        }
    }

    // �J�M�̊i�[���ɏ���������B
    std::vector<XG_PlaceInfo *> data;
    const int size1 = static_cast<int>(xg_vTateInfo.size());
    const int size2 = static_cast<int>(xg_vYokoInfo.size());
    data.reserve(size1 + size2);
    for (int k = 0; k < size1; k++) {
        data.emplace_back(&xg_vTateInfo[k]);
    }
    for (int k = 0; k < size2; k++) {
        data.emplace_back(&xg_vYokoInfo[k]);
    }
    sort(data.begin(), data.end(), xg_placeinfo_compare_position());

    // �����t����ꂽ�J�M�̊i�[���ɔԍ���ݒ肷��B
    int number = 1;
    {
        const int size = static_cast<int>(data.size());
        for (int k = 0; k < size; k++) {
            // �ԍ���ݒ肷��B
            data[k]->m_number = number;
            if (k + 1 < size) {
                // ���̊i�[��񂪓����ʒu�Ȃ��
                if (data[k]->m_iRow == data[k + 1]->m_iRow &&
                    data[k]->m_jCol == data[k + 1]->m_jCol)
                {
                    // ��������B
                    ;
                } else {
                    // �Ⴄ�ʒu�Ȃ�A�ԍ��𑝂₷�B
                    number++;
                }
            }
        }
    }

    // �J�M�̊i�[����ԍ����ɕ��בւ���B
    sort(xg_vTateInfo.begin(), xg_vTateInfo.end(), xg_placeinfo_compare_number());
    sort(xg_vYokoInfo.begin(), xg_vYokoInfo.end(), xg_placeinfo_compare_number());

    return true;
}

// �ԍ�������i�`�F�b�N�Ȃ��j�B
void __fastcall XG_Board::DoNumberingNoCheck()
{
    // �P��f�[�^�B
    XG_WordData wd;

    const int nRows = xg_nRows;
    const int nCols = xg_nCols;

#ifndef NDEBUG
    // ��}�X���Ȃ��Ɖ��肷��B
    for (int i = 0; i < nRows * nCols; i++) {
        assert(GetAt(i) != ZEN_SPACE);
    }
#endif

    // �J�M���N���A����B
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();

    // �e��ɂ��āA�c�����ɃX�L��������B
    for (int j = 0; j < nCols; j++) {
        for (int i = 0; i < nRows - 1; i++) {
            // �����}�X�̘A���������������H
            if (GetAt(i, j) != ZEN_BLACK && GetAt(i + 1, j) != ZEN_BLACK) {
                int lo, hi;

                // �P�ꂪ�u�����Ԃ����߂�B
                lo = hi = i;
                while (hi + 1 < nRows) {
                    if (GetAt(hi + 1, j) == ZEN_BLACK)
                        break;
                    hi++;
                }
                assert(hi == nRows - 1 || GetAt(hi + 1, j) == ZEN_BLACK);

                // ���̈ʒu��ݒ肷��B
                i = hi + 1;

                // �󔒂����邩�𒲂ׂ�Ɠ����ɁA���̋�Ԃɂ���P����擾����B
                bool bFound = false;
                std::wstring word;
                for (int k = lo; k <= hi; k++) {
                    if (GetAt(k, j) == ZEN_SPACE) {
                        bFound = true;
                        break;
                    }
                    word += GetAt(k, j);
                }

                // �󔒂�������Ȃ��������H
                if (!bFound) {
                    // �P���o�^����B
                    xg_vTateInfo.emplace_back(lo, j, std::move(word));
                }
            }
        }
    }

    // �e�s�ɂ��āA�������ɃX�L��������B
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols - 1; j++) {
            // �����}�X�̘A���������������H
            if (GetAt(i, j) != ZEN_BLACK && GetAt(i, j + 1) != ZEN_BLACK) {
                int lo, hi;

                // �P�ꂪ�u�����Ԃ����߂�B
                lo = hi = j;
                while (hi + 1 < nCols) {
                    if (GetAt(i, hi + 1) == ZEN_BLACK)
                        break;
                    hi++;
                }
                assert(hi == nCols - 1 || GetAt(i, hi + 1) == ZEN_BLACK);

                // ���̈ʒu��ݒ肷��B
                j = hi + 1;

                // �󔒂����邩�𒲂ׂ�Ɠ����ɁA���̋�Ԃɂ���P����擾����B
                std::wstring word;
                bool bFound = false;
                for (int k = lo; k <= hi; k++) {
                    if (GetAt(i, k) == ZEN_SPACE) {
                        bFound = true;
                        break;
                    }
                    word += GetAt(i, k);
                }

                // �󔒂�������Ȃ��������H
                if (!bFound) {
                    // ���̃J�M�Ƃ��ēo�^�B
                    xg_vYokoInfo.emplace_back(i, lo, std::move(word));
                }
            }
        }
    }

    // �J�M�̊i�[���ɏ���������B
    std::vector<XG_PlaceInfo *> data;
    {
        const int size = static_cast<int>(xg_vTateInfo.size());
        for (int k = 0; k < size; k++) {
            data.emplace_back(&xg_vTateInfo[k]);
        }
    }
    {
        const int size = static_cast<int>(xg_vYokoInfo.size());
        for (int k = 0; k < size; k++) {
            data.emplace_back(&xg_vYokoInfo[k]);
        }
    }
    sort(data.begin(), data.end(), xg_placeinfo_compare_position());

    // �����t����ꂽ�J�M�̊i�[���ɔԍ���ݒ肷��B
    int number = 1;
    {
        const int size = static_cast<int>(data.size());
        for (int k = 0; k < size; k++) {
            // �ԍ���ݒ肷��B
            data[k]->m_number = number;
            if (k + 1 < size) {
                // ���̊i�[��񂪓����ʒu�Ȃ��
                if (data[k]->m_iRow == data[k + 1]->m_iRow &&
                    data[k]->m_jCol == data[k + 1]->m_jCol)
                {
                    // ��������B
                    ;
                } else {
                    // �Ⴄ�ʒu�Ȃ�A�ԍ��𑝂₷�B
                    number++;
                }
            }
        }
    }

    // �J�M�̊i�[����ԍ����ɕ��בւ���B
    sort(xg_vTateInfo.begin(), xg_vTateInfo.end(), xg_placeinfo_compare_number());
    sort(xg_vYokoInfo.begin(), xg_vYokoInfo.end(), xg_placeinfo_compare_number());
}

// �����擾����B
template <bool t_alternative> bool __fastcall
XgGetCandidatesAddBlack(
    std::vector<std::wstring>& cands, const std::wstring& pattern, int& nSkip,
    bool left_black_check, bool right_black_check)
{
    // �p�^�[���̒����B
    const int patlen = static_cast<int>(pattern.size());
    assert(patlen >= 2);

#ifndef NDEBUG
    // �p�^�[���ɍ��}�X���Ȃ����Ƃ����肷��B
    // �p�^�[���ɋ󔒂����邱�Ƃ����肷��B
    bool bSpaceFound = false;
    for (int i = 0; i < patlen; i++) {
        assert(pattern[i] != ZEN_BLACK);
        if (pattern[i] == ZEN_SPACE)
            bSpaceFound = true;
    }
    assert(bSpaceFound);
#endif

    // �����N���A����B
    cands.clear();
    // �X�s�[�h�̂��߂ɗ\�񂷂�B
    if (t_alternative)
        cands.reserve(xg_dict_2.size() / 32);
    else
        cands.reserve(xg_dict_1.size() / 32);

    // ����������B
    nSkip = 0;
    std::wstring result(pattern);

    // �p�^�[����3���ȏォ�H
    if (patlen > 2) {
        // �Ǘ����������}�X�����[���H
        if (result[0] != ZEN_SPACE && result[1] == ZEN_SPACE) {
            // ���̉E�Ƀu���b�N��u�������̂����ɂ���B
            result[1] = ZEN_BLACK;
            cands.emplace_back(result);
            result = pattern;
            // ����͐�ɏ��������ׂ��Ȃ̂ŁA�����_�������珜�O����B
            nSkip++;
        } else {
            bool bCharNotFound = true;  // �����}�X���Ȃ��������H

            // �Ǘ����������}�X�����ɂ��邩�H
            for (int j = 1; j < patlen - 1; j++) {
                if (result[j] != ZEN_SPACE) {
                    if (result[j - 1] == ZEN_SPACE && result[j + 1] == ZEN_SPACE) {
                        // �Ǘ����������}�X���������B
                        // ���̗��[�Ƀu���b�N��u���B
                        result[j - 1] = result[j + 1] = ZEN_BLACK;
                        cands.emplace_back(result);
                        result = pattern;
                        // ����͐�ɏ��������ׂ��Ȃ̂ŁA�����_�������珜�O����B
                        nSkip++;
                    }
                    bCharNotFound = false;
                    break;
                }
            }

            if (bCharNotFound) {
                // �Ǘ����������}�X���E�[���H
                if (result[patlen - 1] != ZEN_SPACE && result[patlen - 2] == ZEN_SPACE) {
                    // ���̍��Ƀu���b�N��u���B
                    result[patlen - 2] = ZEN_BLACK;
                    cands.emplace_back(result);
                    result = pattern;
                    // ����͐�ɏ��������ׂ��Ȃ̂ŁA�����_�������珜�O����B
                    nSkip++;
                }
            }
        }
    }

    // ���ׂĂ̓o�^����Ă���P��ɂ��āB
    for (const auto& data : (t_alternative ? xg_dict_2 : xg_dict_1)) {
        // �p�^�[�����P��̕��������ꍇ�A�X�L�b�v����B
        const std::wstring& word = data.m_word;
        const int wordlen = static_cast<int>(word.size());
        if (wordlen > patlen)
            continue;

        // �P��̒u�����Ԃɂ��Ē��ׂ�B
        const int patlen_minus_wordlen = patlen - wordlen;
        for (int j = 0; j <= patlen_minus_wordlen; j++) {
            // ���[j, j + wordlen - 1]�̑O��ɕ�������������X�L�b�v����B
            if (j > 0 && pattern[j - 1] != ZEN_SPACE)
                continue;
            if (j < patlen_minus_wordlen && pattern[j + wordlen] != ZEN_SPACE)
                continue;

            // ���[j, j + wordlen - 1]�ɕ����}�X�����邩�H
            bool bCharFound = false;
            const int j_plus_wordlen = j + wordlen;
            for (int m = j; m < j_plus_wordlen; m++) {
                assert(pattern[m] != ZEN_BLACK);
                if (pattern[m] != ZEN_SPACE) {
                    bCharFound = true;
                    break;
                }
            }
            if (!bCharFound)
                continue;

            // �p�^�[�����P��Ƀ}�b�`���邩�H
            bool bMatched = true;
            for (int m = j, n = 0; n < wordlen; m++, n++) {
                if (pattern[m] != ZEN_SPACE && pattern[m] != word[n]) {
                    bMatched = false;
                    break;
                }
            }
            if (!bMatched)
                continue;

            // �}�b�`�����B
            result = pattern;

            // ���[j, j + wordlen - 1]�̑O��Ɂ��������B
            if (j > 0)
                result[j - 1] = ZEN_BLACK;
            if (j < patlen_minus_wordlen)
                result[j + wordlen] = ZEN_BLACK;

            // ���[j, j + wordlen - 1]�ɒP���K�p����B
            for (int k = 0, m = j; k < wordlen; k++, m++)
                result[m] = word[k];

            // ���}�X�̘A�������O����B
            if (left_black_check && result[0] == ZEN_BLACK)
                continue;
            if (right_black_check && result[patlen - 1] == ZEN_BLACK)
                continue;

            // �ǉ�����B
            cands.emplace_back(result);
        }
    }

    // ��₪��łȂ���ΐ����B
    return !cands.empty();
}

// �����擾����i���}�X�ǉ��Ȃ��j�B
template <bool t_alternative> bool __fastcall
XgGetCandidatesNoAddBlack(std::vector<std::wstring>& cands, const std::wstring& pattern)
{
    // �P��̒����B
    const int patlen = static_cast<int>(pattern.size());
    assert(patlen >= 2);

    // �����N���A����B
    cands.clear();
    // �X�s�[�h�̂��߁A�\�񂷂�B
    if (t_alternative)
        cands.reserve(xg_dict_2.size() / 32);
    else
        cands.reserve(xg_dict_1.size() / 32);

    // ���ׂĂ̓o�^���ꂽ�P��ɂ��āB
    for (const auto& data : (t_alternative ? xg_dict_2 : xg_dict_1)) {
        // �p�^�[���ƒP��̒������������Ȃ���΁A�X�L�b�v����B
        const std::wstring& word = data.m_word;
        const int wordlen = static_cast<int>(word.size());
        if (wordlen != patlen)
            continue;

        // ���[0, wordlen - 1]�ɕ����}�X�����邩�H
        bool bCharFound = false;
        for (int k = 0; k < wordlen; k++) {
            assert(pattern[k] != ZEN_BLACK);
            if (pattern[k] != ZEN_SPACE) {
                bCharFound = true;
                break;
            }
        }
        if (!bCharFound)
            continue;

        // �p�^�[�����P��Ƀ}�b�`���邩�H
        bool bMatched = true;
        for (int k = 0; k < wordlen; k++) {
            if (pattern[k] != ZEN_SPACE && pattern[k] != word[k]) {
                bMatched = false;
                break;
            }
        }
        if (!bMatched)
            continue;

        // �ǉ�����B
        cands.emplace_back(word);
    }

    // ��₪��łȂ���ΐ����B
    return !cands.empty();
}

// �����H
bool __fastcall XG_Board::IsSolution() const
{
    const int nRows = xg_nRows;
    const int nCols = xg_nCols;

    // �󂫃}�X������Ή��ł͂Ȃ��B
#if 1
    if (Count() != nRows * nCols)
        return false;
#else
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            if (GetAt(i, j) == ZEN_SPACE)
                return false;
        }
    }
#endif

    return IsValid();
}

// �q���g���������͂���B
bool __fastcall XgParseHints(
    std::vector<XG_Hint>& hints, const std::wstring& str)
{
    // �q���g���N���A����B
    hints.clear();

    int count = 0;
    for (size_t i = 0;;) {
        if (count++ > 1000) {
            return false;
        }

        size_t i0 = str.find(XgLoadStringDx1(IDS_KEYLEFT), i);
        if (i0 == std::wstring::npos) {
            break;
        }

        size_t i1 = str.find_first_of(L"0123456789", i0);
        if (i1 == std::wstring::npos) {
            return false;
        }

        int number = _wtoi(str.data() + i1);
        if (number <= 0) {
            return false;
        }

        size_t i2 = str.find(XgLoadStringDx1(IDS_KEYRIGHT), i0);
        if (i2 == std::wstring::npos) {
            return false;
        }

        std::wstring word;
        size_t i3, i4;
        i3 = str.find(s_szBeginWord, i2);
        if (i3 != std::wstring::npos) {
            i3 += wcslen(s_szBeginWord);

            i4 = str.find(s_szEndWord, i3);
            if (i4 == std::wstring::npos) {
                return false;
            }
            word = str.substr(i3, i4 - i3);
            i4 += wcslen(s_szEndWord);
        } else {
            i4 = i2 + wcslen(XgLoadStringDx1(IDS_KEYRIGHT));
        }

        size_t i5 = str.find(XgLoadStringDx1(IDS_KEYLEFT), i4);
        if (i5 == std::wstring::npos) {
            std::wstring hint = str.substr(i4);
            xg_str_replace_all(hint, L"\r", L"");
            xg_str_replace_all(hint, L"\n", L"");
            xg_str_replace_all(hint, L"\t", L"");
            xg_str_trim(hint);
            hints.emplace_back(number, std::move(word), std::move(hint));
            break;
        } else {
            std::wstring hint = str.substr(i4, i5 - i4);
            xg_str_replace_all(hint, L"\r", L"");
            xg_str_replace_all(hint, L"\n", L"");
            xg_str_replace_all(hint, L"\t", L"");
            xg_str_trim(hint);
            hints.emplace_back(number, std::move(word), std::move(hint));
            i = i5;
        }
    }

    return true;
}

// �q���g���������͂���B
bool __fastcall XgParseHintsStr(HWND hwnd, const std::wstring& strHints)
{
    // �q���g���N���A����B
    xg_vecTateHints.clear();
    xg_vecYokoHints.clear();

    // �q���g������̑O��̋󔒂���菜���B
    std::wstring str(strHints);
    xg_str_trim(str);

    // strCaption1��strCaption2�ɂ��Atate��yoko�ɕ�����B
    std::wstring strCaption1 = XgLoadStringDx1(IDS_DOWN);
    std::wstring strCaption2 = XgLoadStringDx1(IDS_ACROSS);
    size_t i1 = str.find(strCaption1);
    if (i1 == std::wstring::npos)
        return false;
    i1 += strCaption1.size();
    size_t i2 = str.find(strCaption2);
    if (i2 == std::wstring::npos)
        return false;
    std::wstring tate = str.substr(i1, i2 - i1);
    i2 += strCaption2.size();
    std::wstring yoko = str.substr(i2);

    // ���l������菜���B
    size_t i3 = yoko.find(XgLoadStringDx1(IDS_HEADERSEP2));
    if (i3 != std::wstring::npos) {
        yoko = yoko.substr(0, i3);
    }

    // �O��̋󔒂���菜���B
    xg_str_trim(tate);
    xg_str_trim(yoko);

    // ���ꂼ��ɂ��ĉ�͂���B
    return XgParseHints(xg_vecTateHints, tate) &&
           XgParseHints(xg_vecYokoHints, yoko);
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

// ������̃��[������͂���B
INT __fastcall XgParseRules(const std::wstring& str)
{
    INT nRules = 0;
    std::vector<std::wstring> rules;
    if (str.find(L" / ") != str.npos) { // ����" / "���܂܂�Ă�����
        mstr_split(rules, str, L"/"); // "/"�ŕ�������B
    } else { // �܂܂�Ă��Ȃ����
        mstr_split(rules, str, L" \t"); // �󔒂ŕ�������B
    }
    for (auto& rule : rules) {
        xg_str_trim(rule); // �O��̋󔒂���菜���B
        if (rule.empty())
            continue;
        if (rule == XgLoadStringDx1(IDS_RULE_DONTDOUBLEBLACK)) {
            nRules |= RULE_DONTDOUBLEBLACK;
        } else if (rule == XgLoadStringDx1(IDS_RULE_DONTCORNERBLACK)) {
            nRules |= RULE_DONTCORNERBLACK;
        } else if (rule == XgLoadStringDx1(IDS_RULE_DONTTRIDIRECTIONS)) {
            nRules |= RULE_DONTTRIDIRECTIONS;
        } else if (rule == XgLoadStringDx1(IDS_RULE_DONTDIVIDE)) {
            nRules |= RULE_DONTDIVIDE;
        } else if (rule == XgLoadStringDx1(IDS_RULE_DONTTHREEDIAGONALS)) {
            nRules |= RULE_DONTTHREEDIAGONALS;
        } else if (rule == XgLoadStringDx1(IDS_RULE_DONTFOURDIAGONALS)) {
            nRules |= RULE_DONTFOURDIAGONALS;
        } else if (rule == XgLoadStringDx1(IDS_RULE_POINTSYMMETRY)) {
            nRules |= RULE_POINTSYMMETRY;
        } else if (rule == XgLoadStringDx1(IDS_RULE_LINESYMMETRYV)) {
            nRules |= RULE_LINESYMMETRYV;
        } else if (rule == XgLoadStringDx1(IDS_RULE_LINESYMMETRYH)) {
            nRules |= RULE_LINESYMMETRYH;
        } else {
            if (XgIsUserJapanese())
                nRules = DEFAULT_RULES_JAPANESE;
            else
                nRules = DEFAULT_RULES_ENGLISH;
            break;
        }
    }
    return nRules;
}

// ���[���𕶎���ɂ���B
std::wstring __fastcall XgGetRulesString(INT rules)
{
    // �����F�p��Ή��̂��߁A�󔒋�؂肩��" / "��؂�ɕύX���܂����B
    std::wstring ret;

    if (rules & RULE_DONTDOUBLEBLACK) {
        if (ret.size())  {
            ret += L" / ";
        }
        ret += XgLoadStringDx1(IDS_RULE_DONTDOUBLEBLACK);
    }
    if (rules & RULE_DONTCORNERBLACK) {
        if (ret.size())  {
            ret += L" / ";
        }
        ret += XgLoadStringDx1(IDS_RULE_DONTCORNERBLACK);
    }
    if (rules & RULE_DONTTRIDIRECTIONS) {
        if (ret.size())  {
            ret += L" / ";
        }
        ret += XgLoadStringDx1(IDS_RULE_DONTTRIDIRECTIONS);
    }
    if (rules & RULE_DONTDIVIDE) {
        if (ret.size())  {
            ret += L" / ";
        }
        ret += XgLoadStringDx1(IDS_RULE_DONTDIVIDE);
    }
    if (rules & RULE_DONTTHREEDIAGONALS) {
        if (ret.size())  {
            ret += L" / ";
        }
        ret += XgLoadStringDx1(IDS_RULE_DONTTHREEDIAGONALS);
    } else if (rules & RULE_DONTFOURDIAGONALS) {
        if (ret.size())  {
            ret += L" / ";
        }
        ret += XgLoadStringDx1(IDS_RULE_DONTFOURDIAGONALS);
    }
    if (rules & RULE_POINTSYMMETRY) {
        if (ret.size())  {
            ret += L" / ";
        }
        ret += XgLoadStringDx1(IDS_RULE_POINTSYMMETRY);
    }
    if (rules & RULE_LINESYMMETRYV) {
        if (ret.size())  {
            ret += L" / ";
        }
        ret += XgLoadStringDx1(IDS_RULE_LINESYMMETRYV);
    }
    if (rules & RULE_LINESYMMETRYH) {
        if (ret.size())  {
            ret += L" / ";
        }
        ret += XgLoadStringDx1(IDS_RULE_LINESYMMETRYH);
    }
    return ret;
}

// JSON�������ݒ肷��B
bool __fastcall XgSetJsonString(HWND hwnd, const std::wstring& str)
{
    ::DestroyWindow(xg_hHintsWnd);
    xg_hHintsWnd = NULL;

    std::string utf8 = XgUnicodeToUtf8(str);

    try {
        json j = json::parse(utf8);
        int row_count = j["row_count"];
        int column_count = j["column_count"];
        auto cell_data = j["cell_data"];
        bool is_solved = j["is_solved"];
        bool has_mark = j["has_mark"];
        bool has_hints = j["has_hints"];
        INT rules;
        if (XgIsUserJapanese())
            rules = DEFAULT_RULES_JAPANESE;
        else
            rules = DEFAULT_RULES_ENGLISH;
        if (j["rules"].is_string()) {
            auto str = XgUtf8ToUnicode(j["rules"].get<std::string>());
            rules = XgParseRules(str);
        }
        std::wstring dictionary;
        if (j["dictionary"].is_string()) {
            dictionary = XgUtf8ToUnicode(j["dictionary"].get<std::string>());
        }

        if (row_count <= 0 || column_count <= 0) {
            return false;
        }

        if (row_count != int(cell_data.size())) {
            return false;
        }

        XG_Board xw;
        int nRowsSave = xg_nRows, nColsSave = xg_nCols;
        xg_nRows = row_count;
        xg_nCols = column_count;
        xw.ResetAndSetSize(row_count, column_count);

        bool success = true;

        for (int i = 0; i < row_count; ++i) {
            std::wstring row = XgUtf8ToUnicode(cell_data[i]);
            if (int(row.size()) != column_count) {
                success = false;
                break;
            }
            for (int j = 0; j < column_count; ++j) {
                xw.SetAt(i, j, row[j]);
            }
        }

        std::vector<XG_Pos> mark_positions;
        if (has_mark) {
            auto marks = j["marks"];
            for (size_t k = 0; k < marks.size(); ++k) {
                auto mark = marks[k];
                int i = int(mark[0]) - 1;
                int j = int(mark[1]) - 1;
                if (i < 0 || row_count < i) {
                    success = false;
                    break;
                }
                if (j < 0 || column_count < j) {
                    success = false;
                    break;
                }
                mark_positions.emplace_back(i, j);
            }
        }

        std::vector<XG_Hint> tate, yoko;
        if (has_hints) {
            auto hints = j["hints"];
            auto v = hints["v"];
            auto h = hints["h"];
            for (size_t i = 0; i < v.size(); ++i) {
                auto data = v[i];
                int number = int(data[0]);
                if (number <= 0) {
                    success = false;
                    break;
                }
                auto word = XgUtf8ToUnicode(data[1]);
                auto hint = XgUtf8ToUnicode(data[2]);
                tate.emplace_back(number, word, hint);
            }
            for (size_t i = 0; i < h.size(); ++i) {
                auto data = h[i];
                int number = int(data[0]);
                if (number <= 0) {
                    success = false;
                    break;
                }
                auto word = XgUtf8ToUnicode(data[1]);
                auto hint = XgUtf8ToUnicode(data[2]);
                yoko.emplace_back(number, word, hint);
            }
        }

        auto header = XgUtf8ToUnicode(j["header"]);
        auto notes = XgUtf8ToUnicode(j["notes"]);
        if (j["theme"].is_string()) {
            xg_strTheme = XgUtf8ToUnicode(j["theme"]);
        } else {
            xg_strTheme = xg_strDefaultTheme;
        }

        if (success) {
            // �J�M���N���A����B
            xg_vTateInfo.clear();
            xg_vYokoInfo.clear();

            if (is_solved) {
                xg_bSolved = true;
                xg_solution = xw;
                xg_xword.ResetAndSetSize(row_count, column_count);
                for (int i = 0; i < xg_nRows; i++) {
                    for (int j = 0; j < xg_nCols; j++) {
                        // ���ɍ��킹�āA���ɍ��}�X��u���B
                        if (xg_solution.GetAt(i, j) == ZEN_BLACK)
                            xg_xword.SetAt(i, j, ZEN_BLACK);
                    }
                }
                if (has_hints) {
                    xg_solution.DoNumberingNoCheck();
                }
                xg_solution.GetHintsStr(xg_strHints, 2, true);
            } else {
                xg_bSolved = false;
                xg_xword = xw;
            }
            xg_vMarks = mark_positions;
            xg_vecTateHints = tate;
            xg_vecYokoHints = yoko;
            xg_strHeader = header;
            xg_str_trim(xg_strHeader);
            xg_strNotes = notes;
            xg_str_trim(xg_strNotes);

            LPCWSTR psz = XgLoadStringDx1(IDS_BELOWISNOTES);
            if (xg_strNotes.empty()) {
                ;
            } else if (xg_strNotes.find(psz) == 0) {
                xg_strNotes = xg_strNotes.substr(std::wstring(psz).size());
                xg_str_trim(xg_strNotes);
            }

            // �q���g�ǉ��t���O���N���A����B
            xg_bHintsAdded = false;

            if (is_solved) {
                // �q���g��\������B
                XgShowHints(hwnd);
            }

            // ���[����ݒ肷��B
            xg_nRules = rules;

            // �������̎�����ǂݍ��ށB
            if (dictionary.size()) {
                for (auto& file : xg_dict_files) {
                    if (file.find(dictionary) != std::wstring::npos) {
                        if (XgLoadDictFile(file.c_str())) {
                            XgSetDict(file.c_str());
                            XgSetInputModeFromDict(xg_hMainWnd);
                            break;
                        }
                    }
                }
            }
        } else {
            xg_nRows = nRowsSave;
            xg_nCols = nColsSave;
        }

        return success;
    }
    catch (json::exception&)
    {
    }

    return false;
}

// �������ݒ肷��B
bool __fastcall
XgSetString(HWND hwnd, const std::wstring& str, bool json)
{
    ::DestroyWindow(xg_hHintsWnd);
    xg_hHintsWnd = NULL;

    if (json) {
        // JSON�`���B
        return XgSetJsonString(hwnd, str);
    }

    // �N���X���[�h�f�[�^�B
    XG_Board xword;

    // �������ǂݍ��ށB
    if (!xword.SetString(str))
        return false;

    // �󂫃}�X�����݂��Ȃ����H
    bool bFulfilled = xword.IsFulfilled();
    if (bFulfilled) {
        // �󂫃}�X���Ȃ������B

        // �q���g��ݒ肷��B
        size_t i = str.find(ZEN_LRIGHT, 0);
        if (i != std::wstring::npos) {
            // �t�b�^�[���擾����B
            std::wstring s = str.substr(i + 1);

            // �t�b�^�[�̔��l�����擾���āA��菜���B
            std::wstring strFooterSep = XgLoadStringDx1(IDS_HEADERSEP2);
            size_t i3 = s.find(strFooterSep);
            if (i3 != std::wstring::npos) {
                xg_strNotes = s.substr(i3 + strFooterSep.size());
                s = s.substr(0, i3);
            }

            LPCWSTR psz = XgLoadStringDx1(IDS_BELOWISNOTES);
            if (xg_strNotes.find(psz) == 0) {
                xg_strNotes = xg_strNotes.substr(std::wstring(psz).size());
                xg_str_trim(xg_strNotes);
            }

            // �q���g�̑O��̋󔒂���菜���B
            xg_str_trim(s);

            // �q���g�������ݒ肷��B
            if (!s.empty()) {
                xg_strHints = s;
                xg_strHints += xg_pszNewLine;
            }
        } else {
            // �q���g���Ȃ��B
            xg_strHints.clear();
        }

        // �q���g���������͂���B
        if (xg_strHints.empty() || !XgParseHintsStr(hwnd, xg_strHints)) {
            // ���s�����B
            xg_strHints.clear();
            xg_vecTateHints.clear();
            xg_vecYokoHints.clear();
        } else {
            // �J�M�ɒP�ꂪ������Ă��Ȃ������ꍇ�̏����B
            for (auto& hint : xg_vecTateHints) {
                if (hint.m_strWord.size())
                    continue;

                for (const auto& info : xg_vTateInfo) {
                    if (info.m_number == hint.m_number) {
                        std::wstring word;
                        for (int k = info.m_iRow; k < xg_nRows; ++k) {
                            WCHAR ch = xword.GetAt(k, info.m_jCol);
                            if (ch == ZEN_BLACK)
                                break;
                            word += ch;
                        }
                        hint.m_strWord = std::move(word);
                        break;
                    }
                }
            }
            for (auto& hint : xg_vecYokoHints) {
                if (hint.m_strWord.size())
                    continue;

                for (const auto& info : xg_vYokoInfo) {
                    if (info.m_number == hint.m_number) {
                        std::wstring word;
                        for (int k = info.m_jCol; k < xg_nCols; ++k) {
                            WCHAR ch = xword.GetAt(info.m_iRow, k);
                            if (ch == ZEN_BLACK)
                                break;
                            word += ch;
                        }
                        hint.m_strWord = std::move(word);
                        break;
                    }
                }
            }

            // �q���g��\������B
            XgShowHints(hwnd);
        }

        // �q���g�����邩�H
        if (xg_strHints.empty()) {
            // �q���g���Ȃ������B���ł͂Ȃ��B
            xg_xword = xword;
            xg_bSolved = false;
            xg_bShowAnswer = false;
        } else {
            // �󂫃}�X���Ȃ��A�q���g���������B����͉��ł���B
            xg_solution = xword;
            xg_bSolved = true;
            xg_bShowAnswer = false;

            xg_xword.clear();
            for (int i = 0; i < xg_nRows; i++) {
                for (int j = 0; j < xg_nCols; j++) {
                    // ���ɍ��킹�āA���ɍ��}�X��u���B
                    if (xword.GetAt(i, j) == ZEN_BLACK)
                        xg_xword.SetAt(i, j, ZEN_BLACK);
                }
            }

            // �ԍ��t�����s���B
            xg_solution.DoNumberingNoCheck();
        }
    } else {
        // �󂫃}�X���������B���ł͂Ȃ��B
        xg_xword = xword;
        xg_bSolved = false;
        xg_bShowAnswer = false;
    }

    // �q���g�ǉ��t���O���N���A����B
    xg_bHintsAdded = false;

    // �}�[�N�������ǂݍ��ށB
    XgSetStringOfMarks(str.data());

    return true;
}

// �q���g���擾����B
void __fastcall XG_Board::GetHintsStr(
    std::wstring& str, int hint_type, bool bShowAnswer) const
{
    // ������o�b�t�@�B
    WCHAR sz[64];

    // �������B
    str.clear();

    // �܂�������Ă��Ȃ��ꍇ�́A�����Ԃ��Ȃ��B
    if (!xg_bSolved)
        return;

    // �q���g�ɕύX������΁A�X�V����B
    if (XgAreHintsModified()) {
        XgUpdateHintsData();
    }

    assert(0 <= hint_type && hint_type < 6);

    if (hint_type == 0 || hint_type == 2) {
        // �^�e�̃J�M�̕�������\������B
        str += XgLoadStringDx1(IDS_DOWN);
        str += xg_pszNewLine;

        for (const auto& info : xg_vTateInfo) {
            // �ԍ����i�[����B
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_DOWNNUMBER), info.m_number);
            str += sz;

            // �����������邩�ǂ����H
            if (bShowAnswer) {
                str += s_szBeginWord;
                str += info.m_word;
                str += s_szEndWord;
            }

            // �q���g���͂�ǉ�����B
            bool added = false;
            for (const auto& data : xg_dict_1) {
                if (_wcsicmp(data.m_word.data(),
                             info.m_word.data()) == 0)
                {
                    str += data.m_hint;
                    added = true;
                    break;
                }
            }
            if (!added) {
                for (const auto& data : xg_dict_2) {
                    if (_wcsicmp(data.m_word.data(),
                                 info.m_word.data()) == 0)
                    {
                        str += data.m_hint;
                        added = true;
                        break;
                    }
                }
            }
            str += xg_pszNewLine;   // ���s�B
        }
        str += xg_pszNewLine;
    }
    if (hint_type == 1 || hint_type == 2) {
        // ���R�̃J�M�̕�������\������B
        str += XgLoadStringDx1(IDS_ACROSS);
        str += xg_pszNewLine;
        for (const auto& info : xg_vYokoInfo) {
            // �ԍ����i�[����B
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_ACROSSNUMBER), info.m_number);
            str += sz;

            // �����������邩�ǂ����H
            if (bShowAnswer) {
                str += s_szBeginWord;
                str += info.m_word;
                str += s_szEndWord;
            }

            // �q���g���͂�ǉ�����B
            bool added = false;
            for (const auto& data : xg_dict_1) {
                if (_wcsicmp(data.m_word.data(), info.m_word.data()) == 0) {
                    str += data.m_hint;
                    added = true;
                    break;
                }
            }
            if (!added) {
                for (const auto& data : xg_dict_2) {
                    if (_wcsicmp(data.m_word.data(), info.m_word.data()) == 0) {
                        str += data.m_hint;
                        added = true;
                        break;
                    }
                }
            }
            str += xg_pszNewLine;   // ���s�B
        }
        str += xg_pszNewLine;
    }
    if (hint_type == 3 || hint_type == 5) {
        // �^�e�̃J�M�̕�������\������B
        str += XgLoadStringDx1(IDS_PARABOLD);     // <p><b>
        str += XgLoadStringDx1(IDS_DOWNLABEL);
        str += XgLoadStringDx1(IDS_ENDPARABOLD);    // </b></p>
        str += xg_pszNewLine;
        str += XgLoadStringDx1(IDS_OL);    // <ol>
        str += xg_pszNewLine;

        for (const auto& info : xg_vTateInfo) {
            // <li>
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_LI), info.m_number);
            str += sz;

            // �q���g���͂�ǉ�����B
            bool added = false;
            for (const auto& data : xg_dict_1) {
                if (_wcsicmp(data.m_word.data(), info.m_word.data()) == 0) {
                    str += data.m_hint;
                    added = true;
                    break;
                }
            }
            if (!added) {
                for (const auto& data : xg_dict_2) {
                    if (_wcsicmp(data.m_word.data(), info.m_word.data()) == 0) {
                        str += data.m_hint;
                        added = true;
                        break;
                    }
                }
            }
            str += XgLoadStringDx1(IDS_ENDLI);    // </li>
            str += xg_pszNewLine;           // ���s�B
        }
        str += XgLoadStringDx1(IDS_ENDOL);    // </ol>
        str += xg_pszNewLine;           // ���s�B
    }
    if (hint_type == 4 || hint_type == 5) {
        // ���R�̃J�M�̕�������\������B
        str += XgLoadStringDx1(IDS_PARABOLD);     // <p><b>
        str += XgLoadStringDx1(IDS_ACROSSLABEL);
        str += XgLoadStringDx1(IDS_ENDPARABOLD);    // </b></p>
        str += xg_pszNewLine;
        str += XgLoadStringDx1(IDS_OL);    // <ol>
        str += xg_pszNewLine;

        for (const auto& info : xg_vYokoInfo) {
            // <li>
            StringCbPrintf(sz, sizeof(sz), XgLoadStringDx1(IDS_LI), info.m_number);
            str += sz;

            // �q���g���͂�ǉ�����B
            bool added = false;
            for (const auto& data : xg_dict_1) {
                if (_wcsicmp(data.m_word.data(), info.m_word.data()) == 0) {
                    str += data.m_hint;
                    added = true;
                    break;
                }
            }
            if (!added) {
                for (const auto& data : xg_dict_2) {
                    if (_wcsicmp(data.m_word.data(), info.m_word.data()) == 0) {
                        str += data.m_hint;
                        added = true;
                        break;
                    }
                }
            }
            str += XgLoadStringDx1(IDS_ENDLI);    // </li>
            str += xg_pszNewLine;           // ���s�B
        }
        str += XgLoadStringDx1(IDS_ENDOL);    // </ol>
        str += xg_pszNewLine;           // ���s�B
    }
}

// �X���b�h�����擾����B
XG_ThreadInfo *__fastcall XgGetThreadInfo(void)
{
    const DWORD threadid = ::GetCurrentThreadId();
    for (DWORD i = 0; i < xg_dwThreadCount; i++) {
        if (xg_aThreadInfo[i].m_threadid == threadid)
            return &xg_aThreadInfo[i];
    }
    return nullptr;
}

// �ċA����B
void __fastcall XgSolveXWord_AddBlackRecurse(const XG_Board& xw)
{
    // ���łɉ�����Ă���Ȃ�A�I���B
    if (xg_bSolved)
        return;

    // �X���b�h�����擾����B
    XG_ThreadInfo *info = XgGetThreadInfo();
    if (info == nullptr)
        return;

    // ��ł͂Ȃ��}�X�̌����Z�b�g����B
    info->m_count = xw.Count();

    // �L�����Z������Ă���Ȃ�A�I���B
    // �Čv�Z���ׂ��Ȃ�A�I������B
    if (xg_bCancelled || xg_bRetrying)
        return;

    // �����ł���΁A�I���B
    if (!xw.IsValid())
        return;

    const int nRows = xg_nRows, nCols = xg_nCols;

    // �e�s�ɂ��āA�������ɃX�L��������B
    const int nColsMinusOne = nCols - 1;
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nColsMinusOne; j++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂ƕ������ׂ荇���Ă��邩�H
            WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i, j + 1);
            if ((ch1 == ZEN_SPACE && ch2 != ZEN_BLACK && ch2 != ZEN_SPACE) ||
                (ch1 != ZEN_SPACE && ch1 != ZEN_BLACK && ch2 == ZEN_SPACE))
            {
                int lo, hi;

                // �������u������[lo, hi]�����߂�B
                lo = hi = j;
                while (lo > 0) {
                    if (xw.GetAt(i, lo - 1) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (hi + 1 < nCols) {
                    if (xw.GetAt(i, hi + 1) == ZEN_BLACK)
                        break;
                    hi++;
                }

                // �p�^�[���𐶐�����B
                std::wstring pattern;
                for (int k = lo; k <= hi; k++) {
                    pattern += xw.GetAt(i, k);
                }

                const bool left_black_check = (lo != 0);
                const bool right_black_check = (hi + 1 != nCols);

                // �p�^�[���Ƀ}�b�`��������擾����B
                int nSkip;
                std::vector<std::wstring> cands;
                if (XgGetCandidatesAddBlack<false>(cands, pattern, nSkip,
                                                   left_black_check, right_black_check))
                {
                    // ���̈ꕔ������������B
                    auto it = cands.begin();
                    std::advance(it, nSkip);
                    std::random_shuffle(it, cands.end());

                    for (const auto& cand : cands) {
                        // ���łɉ�����Ă���Ȃ�A�I���B
                        // �L�����Z������Ă���Ȃ�A�I���B
                        // �Čv�Z���ׂ��Ȃ�A�I������B
                        if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                            return;

                        bool bCanPutBlack = true;
                        for (int k = lo; k <= hi; k++) {
                            if (cand[k - lo] == ZEN_BLACK && !xw.CanPutBlack(i, k)) {
                                bCanPutBlack = false;
                                break;
                            }
                        }
                        if (bCanPutBlack) {
                            // ����K�p���čċA����B
                            XG_Board copy(xw);

                            assert(cand.size() == pattern.size());

                            for (int k = lo; k <= hi; k++) {
                                assert(copy.GetAt(i, k) == ZEN_SPACE ||
                                       copy.GetAt(i, k) == cand[k - lo]);

                                if (cand[k - lo] == ZEN_BLACK && !copy.CanPutBlack(i, k)) {
                                    bCanPutBlack = false;
                                    break;
                                }

                                copy.SetAt(i, k, cand[k - lo]);
                            }

                            if (!bCanPutBlack) {
                                continue;
                            }

                            // �ċA����B
                            XgSolveXWord_AddBlackRecurse(copy);
                        }

                        // ��ł͂Ȃ��}�X�̌����Z�b�g����B
                        info->m_count = xw.Count();
                    }
                }
                if (XgGetCandidatesAddBlack<true>(cands, pattern, nSkip,
                                                  left_black_check, right_black_check))
                {
                    // ���̈ꕔ������������B
                    auto it = cands.begin();
                    std::advance(it, nSkip);
                    std::random_shuffle(it, cands.end());

                    for (const auto& cand : cands) {
                        // ���łɉ�����Ă���Ȃ�A�I���B
                        // �L�����Z������Ă���Ȃ�A�I���B
                        // �Čv�Z���ׂ��Ȃ�A�I������B
                        if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                            return;

                        bool bCanPutBlack = true;
                        for (int k = lo; k <= hi; k++) {
                            if (cand[k - lo] == ZEN_BLACK && !xw.CanPutBlack(i, k)) {
                                bCanPutBlack = false;
                                break;
                            }
                        }
                        if (bCanPutBlack) {
                            // ����K�p���čċA����B
                            XG_Board copy(xw);

                            assert(cand.size() == pattern.size());

                            for (int k = lo; k <= hi; k++) {
                                assert(copy.GetAt(i, k) == ZEN_SPACE ||
                                       copy.GetAt(i, k) == cand[k - lo]);

                                if (cand[k - lo] == ZEN_BLACK && !copy.CanPutBlack(i, k)) {
                                    bCanPutBlack = false;
                                    break;
                                }

                                copy.SetAt(i, k, cand[k - lo]);
                            }

                            if (!bCanPutBlack) {
                                continue;
                            }

                            // �ċA����B
                            XgSolveXWord_AddBlackRecurse(copy);
                        }

                        // ��ł͂Ȃ��}�X�̌����Z�b�g����B
                        info->m_count = xw.Count();
                    }
                }
                return;
            }
        }
    }

    // �e��ɂ��āA�c�����ɃX�L��������B
    const int nRowsMinusOne = nRows - 1;
    for (int j = 0; j < nCols; j++) {
        for (int i = 0; i < nRowsMinusOne; i++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂ƕ������ׂ荇���Ă��邩�H
            WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i + 1, j);
            if ((ch1 == ZEN_SPACE && ch2 != ZEN_BLACK && ch2 != ZEN_SPACE) ||
                (ch1 != ZEN_SPACE && ch1 != ZEN_BLACK && ch2 == ZEN_SPACE))
            {
                int lo, hi;

                // �������u������[lo, hi]�����߂�B
                lo = hi = i;
                while (lo > 0) {
                    if (xw.GetAt(lo - 1, j) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (hi + 1 < nRows) {
                    if (xw.GetAt(hi + 1, j) == ZEN_BLACK)
                        break;
                    hi++;
                }

                // �p�^�[���𐶐�����B
                std::wstring pattern;
                for (int k = lo; k <= hi; k++) {
                    pattern += xw.GetAt(k, j);
                }

                const bool left_black_check = (lo != 0);
                const bool right_black_check = (hi + 1 != nRows);

                // �p�^�[���Ƀ}�b�`��������擾����B
                int nSkip;
                std::vector<std::wstring> cands;
                if (XgGetCandidatesAddBlack<false>(cands, pattern, nSkip,
                                                   left_black_check, right_black_check))
                {
                    // ���̈ꕔ������������B
                    auto it = cands.begin();
                    std::advance(it, nSkip);
                    std::random_shuffle(it, cands.end());

                    for (const auto& cand : cands) {
                        // ���łɉ�����Ă���Ȃ�A�I���B
                        // �L�����Z������Ă���Ȃ�A�I���B
                        // �Čv�Z���ׂ��Ȃ�A�I������B
                        if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                            return;

                        bool bCanPutBlack = true;
                        for (int k = lo; k <= hi; k++) {
                            if (cand[k - lo] == ZEN_BLACK && !xw.CanPutBlack(k, j)) {
                                bCanPutBlack = false;
                                break;
                            }
                        }
                        if (bCanPutBlack) {
                            // ����K�p���čċA����B
                            XG_Board copy(xw);

                            assert(cand.size() == pattern.size());

                            for (int k = lo; k <= hi; k++) {
                                assert(copy.GetAt(k, j) == ZEN_SPACE ||
                                       copy.GetAt(k, j) == cand[k - lo]);

                                if (cand[k - lo] == ZEN_BLACK && !copy.CanPutBlack(k, j)) {
                                    bCanPutBlack = false;
                                    break;
                                }

                                copy.SetAt(k, j, cand[k - lo]);
                            }

                            if (!bCanPutBlack) {
                                continue;
                            }

                            // �ċA����B
                            XgSolveXWord_AddBlackRecurse(copy);
                        }

                        // ��ł͂Ȃ��}�X�̌����Z�b�g����B
                        info->m_count = xw.Count();
                    }
                }
                if (XgGetCandidatesAddBlack<true>(cands, pattern, nSkip,
                                                  left_black_check, right_black_check))
                {
                    // ���̈ꕔ������������B
                    auto it = cands.begin();
                    std::advance(it, nSkip);
                    std::random_shuffle(it, cands.end());

                    for (const auto& cand : cands) {
                        // ���łɉ�����Ă���Ȃ�A�I���B
                        // �L�����Z������Ă���Ȃ�A�I���B
                        // �Čv�Z���ׂ��Ȃ�A�I������B
                        if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                            return;

                        bool bCanPutBlack = true;
                        for (int k = lo; k <= hi; k++) {
                            if (cand[k - lo] == ZEN_BLACK && !xw.CanPutBlack(k, j)) {
                                bCanPutBlack = false;
                                break;
                            }
                        }
                        if (bCanPutBlack) {
                            // ����K�p���čċA����B
                            XG_Board copy(xw);

                            assert(cand.size() == pattern.size());

                            for (int k = lo; k <= hi; k++) {
                                assert(copy.GetAt(k, j) == ZEN_SPACE ||
                                       copy.GetAt(k, j) == cand[k - lo]);

                                if (cand[k - lo] == ZEN_BLACK && !copy.CanPutBlack(k, j)) {
                                    bCanPutBlack = false;
                                    break;
                                }

                                copy.SetAt(k, j, cand[k - lo]);
                            }

                            if (!bCanPutBlack) {
                                continue;
                            }

                            // �ċA����B
                            XgSolveXWord_AddBlackRecurse(copy);
                        }

                        // ��ł͂Ȃ��}�X�̌����Z�b�g����B
                        info->m_count = xw.Count();
                    }
                }
                return;
            }
        }
    }

    // �����ǂ����H
    EnterCriticalSection(&xg_cs);
    bool ok = xw.IsSolution();
    ::LeaveCriticalSection(&xg_cs);
    if (ok) {
        // ���������B
        ::EnterCriticalSection(&xg_cs);
        xg_bSolved = true;
        xg_solution = xw;
        xg_solution.DoNumbering();

        // ���ɍ��킹�āA���ɍ��}�X��u���B
        const int nCount = nRows * nCols;
        for (int i = 0; i < nCount; i++) {
            if (xw.GetAt(i) == ZEN_BLACK)
                xg_xword.SetAt(i, ZEN_BLACK);
        }
        ::LeaveCriticalSection(&xg_cs);
    }
}

// �ċA����i���}�X�ǉ��Ȃ��j�B
void __fastcall XgSolveXWord_NoAddBlackRecurse(const XG_Board& xw)
{
    // ���łɉ�����Ă���Ȃ�A�I���B
    if (xg_bSolved)
        return;

    // �X���b�h�����擾����B
    XG_ThreadInfo *info = XgGetThreadInfo();
    if (info == nullptr)
        return;

    // ��ł͂Ȃ��}�X�̌����Z�b�g����B
    info->m_count = xw.Count();

    // �L�����Z������Ă���Ȃ�A�I���B
    // �Čv�Z���ׂ��Ȃ�A�I������B
    if (xg_bCancelled || xg_bRetrying)
        return;

    // �����ł���΁A�I���B
    if (!xw.IsNoAddBlackOK())
        return;

    const int nRows = xg_nRows, nCols = xg_nCols;

    // �e�s�ɂ��āA�������ɃX�L��������B
    const int nColsMinusOne = nCols - 1;
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nColsMinusOne; j++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂ƕ������ׂ荇���Ă��邩�H
            WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i, j + 1);
            if ((ch1 == ZEN_SPACE && ch2 != ZEN_BLACK && ch2 != ZEN_SPACE) ||
                (ch1 != ZEN_SPACE && ch1 != ZEN_BLACK && ch2 == ZEN_SPACE))
            {
                int lo, hi;

                // �������u������[lo, hi]�����߂�B
                lo = hi = j;
                while (lo > 0) {
                    if (xw.GetAt(i, lo - 1) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (hi + 1 < nCols) {
                    if (xw.GetAt(i, hi + 1) == ZEN_BLACK)
                        break;
                    hi++;
                }

                // �p�^�[���𐶐�����B
                std::wstring pattern;
                for (int k = lo; k <= hi; k++) {
                    pattern += xw.GetAt(i, k);
                }

                // �p�^�[���Ƀ}�b�`��������擾����i���}�X�ǉ��Ȃ��j�B
                std::vector<std::wstring> cands;
                if (XgGetCandidatesNoAddBlack<false>(cands, pattern)) {
                    // ��������������B
                    std::random_shuffle(cands.begin(), cands.end());

                    for (const auto& cand : cands) {
                        // ���łɉ�����Ă���Ȃ�A�I���B
                        // �L�����Z������Ă���Ȃ�A�I���B
                        // �Čv�Z���ׂ��Ȃ�A�I������B
                        if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                            return;

                        // ����K�p���čċA����B
                        XG_Board copy(xw);
                        assert(cand.size() == pattern.size());
                        for (int k = lo; k <= hi; k++) {
                            assert(copy.GetAt(i, k) == ZEN_SPACE ||
                                   copy.GetAt(i, k) == cand[k - lo]);
                            copy.SetAt(i, k, cand[k - lo]);
                        }

                        // �ċA����B
                        XgSolveXWord_NoAddBlackRecurse(copy);

                        // ��ł͂Ȃ��}�X�̌����Z�b�g����B
                        info->m_count = xw.Count();
                    }
                }
                if (XgGetCandidatesNoAddBlack<true>(cands, pattern)) {
                    // ��������������B
                    std::random_shuffle(cands.begin(), cands.end());

                    for (const auto& cand : cands) {
                        // ���łɉ�����Ă���Ȃ�A�I���B
                        // �L�����Z������Ă���Ȃ�A�I���B
                        // �Čv�Z���ׂ��Ȃ�A�I������B
                        if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                            return;

                        // ����K�p���čċA����B
                        XG_Board copy(xw);
                        assert(cand.size() == pattern.size());
                        for (int k = lo; k <= hi; k++) {
                            assert(copy.GetAt(i, k) == ZEN_SPACE ||
                                   copy.GetAt(i, k) == cand[k - lo]);
                            copy.SetAt(i, k, cand[k - lo]);
                        }

                        // �ċA����B
                        XgSolveXWord_NoAddBlackRecurse(copy);

                        // ��ł͂Ȃ��}�X�̌����Z�b�g����B
                        info->m_count = xw.Count();
                    }
                }
                return;
            }
        }
    }

    // �e��ɂ��āA�c�����ɃX�L��������B
    const int nRowsMinusOne = nRows - 1;
    for (int j = 0; j < nCols; j++) {
        for (int i = 0; i < nRowsMinusOne; i++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂ƕ������ׂ荇���Ă��邩�H
            WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i + 1, j);
            if ((ch1 == ZEN_SPACE && ch2 != ZEN_BLACK && ch2 != ZEN_SPACE) ||
                (ch1 != ZEN_SPACE && ch1 != ZEN_BLACK && ch2 == ZEN_SPACE))
            {
                int lo, hi;

                // �������u������[lo, hi]�����߂�B
                lo = hi = i;
                while (lo > 0) {
                    if (xw.GetAt(lo - 1, j) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (hi + 1 < nRows) {
                    if (xw.GetAt(hi + 1, j) == ZEN_BLACK)
                        break;
                    hi++;
                }

                // �p�^�[���𐶐�����B
                std::wstring pattern;
                for (int k = lo; k <= hi; k++) {
                    pattern += xw.GetAt(k, j);
                }

                // �p�^�[���Ƀ}�b�`��������擾����B
                std::vector<std::wstring> cands;
                if (XgGetCandidatesNoAddBlack<false>(cands, pattern)) {
                    // ��������������B
                    std::random_shuffle(cands.begin(), cands.end());

                    for (const auto& cand : cands) {
                        // ���łɉ�����Ă���Ȃ�A�I���B
                        // �L�����Z������Ă���Ȃ�A�I���B
                        // �Čv�Z���ׂ��Ȃ�A�I������B
                        if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                            return;

                        // ����K�p���čċA����B
                        XG_Board copy(xw);
                        assert(cand.size() == pattern.size());
                        for (int k = lo; k <= hi; k++) {
                            assert(copy.GetAt(k, j) == ZEN_SPACE ||
                                   copy.GetAt(k, j) == cand[k - lo]);
                            copy.SetAt(k, j, cand[k - lo]);
                        }

                        // �ċA����B
                        XgSolveXWord_NoAddBlackRecurse(copy);

                        // ��ł͂Ȃ��}�X�̌����Z�b�g����B
                        info->m_count = xw.Count();
                    }
                }
                if (XgGetCandidatesNoAddBlack<false>(cands, pattern)) {
                    // ��������������B
                    std::random_shuffle(cands.begin(), cands.end());

                    for (const auto& cand : cands) {
                        // ���łɉ�����Ă���Ȃ�A�I���B
                        // �L�����Z������Ă���Ȃ�A�I���B
                        // �Čv�Z���ׂ��Ȃ�A�I������B
                        if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                            return;

                        // ����K�p���čċA����B
                        XG_Board copy(xw);
                        assert(cand.size() == pattern.size());
                        for (int k = lo; k <= hi; k++) {
                            assert(copy.GetAt(k, j) == ZEN_SPACE ||
                                   copy.GetAt(k, j) == cand[k - lo]);
                            copy.SetAt(k, j, cand[k - lo]);
                        }

                        // �ċA����B
                        XgSolveXWord_NoAddBlackRecurse(copy);

                        // ��ł͂Ȃ��}�X�̌����Z�b�g����B
                        info->m_count = xw.Count();
                    }
                }
                return;
            }
        }
    }

    // �����ǂ����H
    ::EnterCriticalSection(&xg_cs);
    bool ok = xw.IsSolution();
    ::LeaveCriticalSection(&xg_cs);
    if (ok) {
        // ���������B
        xg_bSolved = true;
        ::EnterCriticalSection(&xg_cs);
        xg_solution = xw;
        xg_solution.DoNumbering();

        // ���ɍ��킹�āA���ɍ��}�X��u���B
        const int nCount = nRows * nCols;
        for (int i = 0; i < nCount; i++) {
            if (xw.GetAt(i) == ZEN_BLACK)
                xg_xword.SetAt(i, ZEN_BLACK);
        }
        ::LeaveCriticalSection(&xg_cs);
    }
}

// �c�Ɖ������ւ���B
void XG_Board::SwapXandY()
{
    const int nRows = xg_nRows;
    const int nCols = xg_nCols;

    std::vector<WCHAR> vCells;
    vCells.assign(nRows * nCols + 1, ZEN_SPACE);

    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            vCells[j * nRows + i] = m_vCells[i * nCols + j];
        }
    }
    vCells[nRows * nCols] = Count();
    m_vCells = vCells;
}

// �����B
void __fastcall XgSolveXWord_AddBlack(const XG_Board& xw)
{
    const int nRows = xg_nRows, nCols = xg_nCols;

    // �����}�X�����邩�H
    bool bCharFound = false;
    for (int i = 0; i < nRows * nCols; i++) {
        const WCHAR ch1 = xw.GetAt(i);
        if (ch1 != ZEN_SPACE && ch1 != ZEN_BLACK)
        {
            bCharFound = true;
        }
    }
    if (bCharFound) {
        // �����}�X���������ꍇ�́A���̂܂܉����B
        XgSolveXWord_AddBlackRecurse(xw);
        return;
    }

    // �����ł���΁A�I���B
    if (!xw.IsValid())
        return;

    // �����_���ȏ����̒P��x�N�^�[���쐬����B
    std::vector<XG_WordData> words(xg_dict_1);
    std::random_shuffle(words.begin(), words.end());

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols - 1; j++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂̘A�������邩�H
            const WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i, j + 1);
            if (ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // �������u������[lo, hi]�����߂�B
                int lo = j;
                while (lo > 0) {
                    if (xw.GetAt(i, lo - 1) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (j + 1 < nCols) {
                    if (xw.GetAt(i, j + 1) == ZEN_BLACK)
                        break;
                    j++;
                }
                const int hi = j;

                // �p�^�[���̒��������߂�B
                const int patlen = hi - lo + 1;
                for (const auto& word_data : words) {
                    // ���łɉ�����Ă���Ȃ�A�I���B
                    // �L�����Z������Ă���Ȃ�A�I���B
                    // �Čv�Z���ׂ��Ȃ�A�I������B
                    if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                        return;

                    // �P��̒������p�^�[���̒����ȉ����H
                    const std::wstring& word = word_data.m_word;
                    const int wordlen = static_cast<int>(word.size());
                    if (wordlen > patlen)
                        continue;

                    // �K�v�ȍ��}�X�͒u���邩�H
                    if ((lo == 0 || xw.CanPutBlack(i, lo - 1)) &&
                        (hi + 1 >= nCols || xw.CanPutBlack(i, hi + 1)))
                    {
                        // �P��Ƃ��̗��[�̊O���̍��}�X���Z�b�g���čċA����B
                        XG_Board copy(xw);
                        for (int k = 0; k < wordlen; k++) {
                            copy.SetAt(i, lo + k, word[k]);
                        }

                        if (lo > 0 && copy.GetAt(i, lo - 1) != ZEN_BLACK) {
                            if (!copy.CanPutBlack(i, lo - 1))
                                continue;

                            copy.SetAt(i, lo - 1, ZEN_BLACK);
                        }
                        if (hi + 1 < nCols && copy.GetAt(i, hi + 1) != ZEN_BLACK) {
                            if (!copy.CanPutBlack(i, hi + 1))
                                continue;

                            copy.SetAt(i, hi + 1, ZEN_BLACK);
                        }

                        //if (copy.TriBlackAround()) {
                        //    continue;
                        //}

                        // �ċA����B
                        XgSolveXWord_AddBlackRecurse(copy);
                    }
                }
                goto retry_1;
            }
        }
    }

    for (int j = 0; j < nCols; j++) {
        for (int i = 0; i < nRows - 1; i++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂̘A�������邩�H
            const WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i + 1, j);
            if (ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // �������u������[lo, hi]�����߂�B
                int lo = i;
                while (lo > 0) {
                    if (xw.GetAt(lo - 1, j) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (i + 1 < nRows) {
                    if (xw.GetAt(i + 1, j) == ZEN_BLACK)
                        break;
                    j++;
                }
                const int hi = i;

                // �p�^�[���̒��������߂�B
                const int patlen = hi - lo + 1;
                for (const auto& word_data : words) {
                    // ���łɉ�����Ă���Ȃ�A�I���B
                    // �L�����Z������Ă���Ȃ�A�I���B
                    // �Čv�Z���ׂ��Ȃ�A�I������B
                    if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                        return;

                    // �P��̒������p�^�[���̒����ȉ����H
                    const std::wstring& word = word_data.m_word;
                    const int wordlen = static_cast<int>(word.size());
                    if (wordlen > patlen)
                        continue;

                    // �K�v�ȍ��}�X�͒u���邩�H
                    if ((lo == 0 || xw.CanPutBlack(lo - 1, j)) &&
                        (hi + 1 >= nRows || xw.CanPutBlack(hi + 1, j)))
                    {
                        // �P��Ƃ��̗��[�̊O���̍��}�X���Z�b�g���čċA����B
                        XG_Board copy(xw);
                        for (int k = 0; k < wordlen; k++) {
                            copy.SetAt(lo + k, j, word[k]);
                        }

                        if (lo > 0 && copy.GetAt(lo - 1, j) != ZEN_BLACK) {
                            if (!copy.CanPutBlack(lo - 1, j))
                                continue;

                            copy.SetAt(lo - 1, j, ZEN_BLACK);
                        }
                        if (hi + 1 < nRows && copy.GetAt(hi + 1, j) != ZEN_BLACK) {
                            if (!copy.CanPutBlack(hi + 1, j))
                                continue;

                            copy.SetAt(hi + 1, j, ZEN_BLACK);
                        }

                        //if (copy.TriBlackAround()) {
                        //    continue;
                        //}

                        // �ċA����B
                        XgSolveXWord_AddBlackRecurse(copy);
                    }
                }
                goto retry_1;
            }
        }
    }

    // �����_���ȏ����̒P��x�N�^�[���쐬����B
retry_1:;
    words = xg_dict_2;
    std::random_shuffle(words.begin(), words.end());

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols - 1; j++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂̘A�������邩�H
            const WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i, j + 1);
            if (ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // �������u������[lo, hi]�����߂�B
                int lo = j;
                while (lo > 0) {
                    if (xw.GetAt(i, lo - 1) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (j + 1 < nCols) {
                    if (xw.GetAt(i, j + 1) == ZEN_BLACK)
                        break;
                    j++;
                }
                const int hi = j;

                // �p�^�[���̒��������߂�B
                const int patlen = hi - lo + 1;
                for (const auto& word_data : words) {
                    // ���łɉ�����Ă���Ȃ�A�I���B
                    // �L�����Z������Ă���Ȃ�A�I���B
                    // �Čv�Z���ׂ��Ȃ�A�I������B
                    if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                        return;

                    // �P��̒������p�^�[���̒����ȉ����H
                    const std::wstring& word = word_data.m_word;
                    const int wordlen = static_cast<int>(word.size());
                    if (wordlen > patlen)
                        continue;

                    // �K�v�ȍ��}�X�͒u���邩�H
                    if ((lo == 0 || xw.CanPutBlack(i, lo - 1)) &&
                        (hi + 1 >= nCols || xw.CanPutBlack(i, hi + 1)))
                    {
                        // �P��Ƃ��̗��[�̊O���̍��}�X���Z�b�g���čċA����B
                        XG_Board copy(xw);
                        for (int k = 0; k < wordlen; k++) {
                            copy.SetAt(i, lo + k, word[k]);
                        }

                        if (lo > 0 && copy.GetAt(i, lo - 1) != ZEN_BLACK) {
                            if (!copy.CanPutBlack(i, lo - 1))
                                continue;

                            copy.SetAt(i, lo - 1, ZEN_BLACK);
                        }
                        if (hi + 1 < nCols && copy.GetAt(i, hi + 1) != ZEN_BLACK) {
                            if (!copy.CanPutBlack(i, hi + 1))
                                continue;

                            copy.SetAt(i, hi + 1, ZEN_BLACK);
                        }

                        //if (copy.TriBlackAround()) {
                        //    continue;
                        //}

                        // �ċA����B
                        XgSolveXWord_AddBlackRecurse(copy);
                    }
                }
                goto retry_2;
            }
        }
    }

    for (int j = 0; j < nCols; j++) {
        for (int i = 0; i < nRows - 1; i++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂̘A�������邩�H
            const WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i + 1, j);
            if (ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // �������u������[lo, hi]�����߂�B
                int lo = i;
                while (lo > 0) {
                    if (xw.GetAt(lo - 1, j) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (i + 1 < nRows) {
                    if (xw.GetAt(i + 1, j) == ZEN_BLACK)
                        break;
                    j++;
                }
                const int hi = i;

                // �p�^�[���̒��������߂�B
                const int patlen = hi - lo + 1;
                for (const auto& word_data : words) {
                    // ���łɉ�����Ă���Ȃ�A�I���B
                    // �L�����Z������Ă���Ȃ�A�I���B
                    // �Čv�Z���ׂ��Ȃ�A�I������B
                    if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                        return;

                    // �P��̒������p�^�[���̒����ȉ����H
                    const std::wstring& word = word_data.m_word;
                    const int wordlen = static_cast<int>(word.size());
                    if (wordlen > patlen)
                        continue;

                    // �K�v�ȍ��}�X�͒u���邩�H
                    if ((lo == 0 || xw.CanPutBlack(lo - 1, j)) &&
                        (hi + 1 >= nRows || xw.CanPutBlack(hi + 1, j)))
                    {
                        // �P��Ƃ��̗��[�̊O���̍��}�X���Z�b�g���čċA����B
                        XG_Board copy(xw);
                        for (int k = 0; k < wordlen; k++) {
                            copy.SetAt(lo + k, j, word[k]);
                        }

                        if (lo > 0 && copy.GetAt(lo - 1, j) != ZEN_BLACK) {
                            if (!copy.CanPutBlack(lo - 1, j))
                                continue;

                            copy.SetAt(lo - 1, j, ZEN_BLACK);
                        }
                        if (hi + 1 < nRows && copy.GetAt(hi + 1, j) != ZEN_BLACK) {
                            if (!copy.CanPutBlack(hi + 1, j))
                                continue;

                            copy.SetAt(hi + 1, j, ZEN_BLACK);
                        }

                        //if (copy.TriBlackAround()) {
                        //    continue;
                        //}

                        // �ċA����B
                        XgSolveXWord_AddBlackRecurse(copy);
                    }
                }
                goto retry_2;
            }
        }
    }
retry_2:;
}

// �����i���}�X�ǉ��Ȃ��j�B
void __fastcall XgSolveXWord_NoAddBlack(const XG_Board& xw)
{
    const int nRows = xg_nRows, nCols = xg_nCols;

    // �����}�X�����邩�H
    bool bCharFound = false;
    for (int i = 0; i < nRows * nCols; i++) {
        const WCHAR ch1 = xw.GetAt(i);
        if (ch1 != ZEN_SPACE && ch1 != ZEN_BLACK) {
            bCharFound = true;
        }
    }
    if (bCharFound) {
        // �����}�X���������ꍇ�́A���̂܂܉����B
        XgSolveXWord_NoAddBlackRecurse(xw);
        return;
    }

    // �����ł���΁A�I���B
    if (!xw.IsNoAddBlackOK())
        return;

    // �����_���ȏ����̒P��x�N�^�[���쐬����B
    std::vector<XG_WordData> words(xg_dict_1);
    std::random_shuffle(words.begin(), words.end());

    // �����}�X���Ȃ������ꍇ�B
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols - 1; j++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂̘A�������邩�H
            const WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i, j + 1);
            if (ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // �������u������[lo, hi]�����߂�B
                int lo = j;
                while (lo > 0) {
                    if (xw.GetAt(i, lo - 1) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (j + 1 < nCols) {
                    if (xw.GetAt(i, j + 1) == ZEN_BLACK)
                        break;
                    j++;
                }
                const int hi = j;

                // �p�^�[���̒��������߂�B
                const int patlen = hi - lo + 1;
                for (const auto& word_data : words) {
                    // ���łɉ�����Ă���Ȃ�A�I���B
                    // �L�����Z������Ă���Ȃ�A�I���B
                    // �Čv�Z���ׂ��Ȃ�A�I������B
                    if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                        return;

                    // �P��ƃp�^�[���̒��������������H
                    const std::wstring& word = word_data.m_word;
                    const int wordlen = static_cast<int>(word.size());
                    if (wordlen != patlen)
                        continue;

                    // �P����Z�b�g����B
                    XG_Board copy(xw);
                    for (int k = 0; k < wordlen; k++) {
                        copy.SetAt(i, lo + k, word[k]);
                    }

                    // �ċA����B
                    XgSolveXWord_NoAddBlackRecurse(copy);
                }
                goto retry_1;
            }
        }
    }

    // �����_���ȏ����̒P��x�N�^�[���쐬����B
retry_1:;
    words = xg_dict_2;
    std::random_shuffle(words.begin(), words.end());

    // �����}�X���Ȃ������ꍇ�B
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols - 1; j++) {
            // ���łɉ�����Ă���Ȃ�A�I���B
            // �L�����Z������Ă���Ȃ�A�I���B
            // �Čv�Z���ׂ��Ȃ�A�I������B
            if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                return;

            // �󔒂̘A�������邩�H
            const WCHAR ch1 = xw.GetAt(i, j), ch2 = xw.GetAt(i, j + 1);
            if (ch1 == ZEN_SPACE && ch2 == ZEN_SPACE) {
                // �������u������[lo, hi]�����߂�B
                int lo = j;
                while (lo > 0) {
                    if (xw.GetAt(i, lo - 1) == ZEN_BLACK)
                        break;
                    lo--;
                }
                while (j + 1 < nCols) {
                    if (xw.GetAt(i, j + 1) == ZEN_BLACK)
                        break;
                    j++;
                }
                const int hi = j;

                // �p�^�[���̒��������߂�B
                const int patlen = hi - lo + 1;
                for (const auto& word_data : words) {
                    // ���łɉ�����Ă���Ȃ�A�I���B
                    // �L�����Z������Ă���Ȃ�A�I���B
                    // �Čv�Z���ׂ��Ȃ�A�I������B
                    if (xg_bSolved || xg_bCancelled || xg_bRetrying)
                        return;

                    // �P��ƃp�^�[���̒��������������H
                    const std::wstring& word = word_data.m_word;
                    const int wordlen = static_cast<int>(word.size());
                    if (wordlen != patlen)
                        continue;

                    // �P����Z�b�g����B
                    XG_Board copy(xw);
                    for (int k = 0; k < wordlen; k++) {
                        copy.SetAt(i, lo + k, word[k]);
                    }

                    // �ċA����B
                    XgSolveXWord_NoAddBlackRecurse(copy);
                }
                goto retry_2;
            }
        }
    }

retry_2:;
}

#ifdef NO_RANDOM
    int xg_random_seed = 0;
#endif

// �}���`�X���b�h�p�̊֐��B
unsigned __stdcall XgSolveProc_AddBlack(void *param)
{
    // �X���b�h�����擾����B
    XG_ThreadInfo *info = reinterpret_cast<XG_ThreadInfo *>(param);

    // ��ł͂Ȃ��}�X�̌����Z�b�g����B
    info->m_count = xg_xword.Count();

    // �X���b�h�̗D��x���グ��B
    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    // �����������[�`��������������B
    #ifndef NO_RANDOM
        srand(::GetTickCount() ^ info->m_threadid);
    #else
        srand(xg_random_seed++);
    #endif

    // �����B
    XgSolveXWord_AddBlack(xg_xword);
    return 0;
}

// �}���`�X���b�h�p�̊֐��i���}�X�ǉ��Ȃ��j�B
unsigned __stdcall XgSolveProc_NoAddBlack(void *param)
{
    // �X���b�h�����擾����B
    XG_ThreadInfo *info = reinterpret_cast<XG_ThreadInfo *>(param);

    // ��ł͂Ȃ��}�X�̌����Z�b�g����B
    info->m_count = xg_xword.Count();

    // �X���b�h�̗D��x���グ��B
    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    // �����������[�`��������������B
    #ifndef NO_RANDOM
        srand(::GetTickCount() ^ info->m_threadid);
    #else
        srand(xg_random_seed++);
    #endif

    // �����i���}�X�ǉ��Ȃ��j�B
    XgSolveXWord_NoAddBlack(xg_xword);
    return 0;
}

// �}���`�X���b�h�p�̊֐��i�X�}�[�g�����j�B
unsigned __stdcall XgSolveProcSmart(void *param)
{
    // �X���b�h�����擾����B
    XG_ThreadInfo *info = reinterpret_cast<XG_ThreadInfo *>(param);

    // ��ł͂Ȃ��}�X�̌����Z�b�g����B
    info->m_count = xg_xword.Count();

    // �X���b�h�̗D��x���グ��B
    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    // �����������[�`��������������B
    #ifndef NO_RANDOM
        srand(::GetTickCount() ^ info->m_threadid);
    #else
        srand(xg_random_seed++);
    #endif

    // ���}�X�𐶐�����B
    XgGenerateBlacksSmart(NULL);

    // ��ł͂Ȃ��}�X�̌����Z�b�g����B
    info->m_count = xg_xword.Count();

    // �����i���}�X�ǉ��Ȃ��j�B
    XgSolveXWord_NoAddBlack(xg_xword);
    return 0;
}

//#define SINGLE_THREAD_MODE

// �������߂�̂��J�n�B
void __fastcall XgStartSolve_AddBlack(void)
{
    // �t���O������������B
    xg_bSolved = xg_bCancelled = xg_bRetrying = false;

    if (xg_bSolvingEmpty)
        xg_xword.clear();

    // �����c�̕��������ꍇ�A�v�Z���Ԃ����炷���߂ɁA
    // �c�Ɖ������ւ��A��ł�����x�c�Ɖ������ւ���B
    if (xg_nRows > xg_nCols) {
        s_bSwapped = true;
        xg_xword.SwapXandY();
        std::swap(xg_nRows, xg_nCols);
    }

#ifdef SINGLE_THREAD_MODE
    XgSolveProc_AddBlack(&xg_aThreadInfo[0]);
#else
    // �X���b�h���J�n����B
    for (DWORD i = 0; i < xg_dwThreadCount; i++) {
        xg_aThreadInfo[i].m_count = static_cast<DWORD>(xg_xword.Count());
        xg_ahThreads[i] = reinterpret_cast<HANDLE>(
            _beginthreadex(nullptr, 0, XgSolveProc_AddBlack, &xg_aThreadInfo[i], 0,
                &xg_aThreadInfo[i].m_threadid));
        assert(xg_ahThreads[i] != nullptr);
    }
#endif
}

// �������߂�̂��J�n�i���}�X�ǉ��Ȃ��j�B
void __fastcall XgStartSolve_NoAddBlack(void)
{
    // �t���O������������B
    xg_bSolved = xg_bCancelled = xg_bRetrying = false;

#ifdef SINGLE_THREAD_MODE
    XgSolveProc_NoAddBlack(&xg_aThreadInfo[0]);
#else
    // �X���b�h���J�n����B
    for (DWORD i = 0; i < xg_dwThreadCount; i++) {
        xg_aThreadInfo[i].m_count = static_cast<DWORD>(xg_xword.Count());
        xg_ahThreads[i] = reinterpret_cast<HANDLE>(
            _beginthreadex(nullptr, 0, XgSolveProc_NoAddBlack, &xg_aThreadInfo[i], 0,
                &xg_aThreadInfo[i].m_threadid));
        assert(xg_ahThreads[i] != nullptr);
    }
#endif
}

// �������߂�̂��J�n�i�X�}�[�g�����j�B
void __fastcall XgStartSolve_Smart(void)
{
    // �t���O������������B
    xg_bSolved = xg_bCancelled = xg_bRetrying = false;

    // �܂��u���b�N�������Ă��Ȃ��B
    xg_bBlacksGenerated = FALSE;

#ifdef SINGLE_THREAD_MODE
    XgSolveProcSmart(&xg_aThreadInfo[0]);
#else
    // �X���b�h���J�n����B
    for (DWORD i = 0; i < xg_dwThreadCount; i++) {
        xg_aThreadInfo[i].m_count = static_cast<DWORD>(xg_xword.Count());
        xg_ahThreads[i] = reinterpret_cast<HANDLE>(
            _beginthreadex(nullptr, 0, XgSolveProcSmart, &xg_aThreadInfo[i], 0,
                &xg_aThreadInfo[i].m_threadid));
        assert(xg_ahThreads[i] != nullptr);
    }
#endif
}

// �������߂悤�Ƃ�����̌㏈���B
void __fastcall XgEndSolve(void)
{
    if (s_bSwapped) {
        xg_xword.SwapXandY();
        if (xg_bSolved) {
            xg_solution.SwapXandY();
        }
        std::swap(xg_nRows, xg_nCols);
        if (xg_bSolved) {
            xg_solution.DoNumbering();
            if (!XgParseHintsStr(xg_hMainWnd, xg_strHints)) {
                xg_strHints.clear();
            }
        }
        s_bSwapped = false;
    }
}

// ��d�}�X�P���`�悷��B
void __fastcall XgDrawMarkWord(HDC hdc, LPSIZE psiz)
{
    int nCount = static_cast<int>(xg_vMarks.size());
    if (nCount == 0) {
        ::MessageBeep(0xFFFFFFFF);
        return;
    }

    // �u���V���쐬����B
    HBRUSH hbrBlack = ::CreateSolidBrush(xg_rgbBlackCellColor);
    HBRUSH hbrWhite = ::CreateSolidBrush(xg_rgbWhiteCellColor);
    HBRUSH hbrMarked = ::CreateSolidBrush(xg_rgbMarkedCellColor);

    // �ׂ��y�����쐬���A�I������B
    HPEN hThinPen = ::CreatePen(PS_SOLID, 1, xg_rgbBlackCellColor);

    // �������ƍ����u���V���쐬����B
    LOGBRUSH lbBlack;
    ::GetObject(hbrBlack, sizeof(lbBlack), &lbBlack);
    int c_nWide = 4;
    HPEN hWidePen = ::ExtCreatePen(
        PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_SQUARE | PS_JOIN_BEVEL,
        c_nWide, &lbBlack, 0, NULL);

    LOGFONTW lf;

    // �����}�X�̃t�H���g���쐬����B
    ZeroMemory(&lf, sizeof(lf));
    StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), XgLoadStringDx1(IDS_MONOFONT));
    if (xg_szCellFont[0])
        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szCellFont);
    lf.lfHeight = -xg_nCellSize * xg_nCellCharPercents / 100;
    lf.lfWidth = 0;
    lf.lfWeight = FW_NORMAL;
    lf.lfQuality = ANTIALIASED_QUALITY;
    HFONT hFont = ::CreateFontIndirectW(&lf);

    // �����������̃t�H���g���쐬����B
    ZeroMemory(&lf, sizeof(lf));
    if (xg_szSmallFont[0])
        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szSmallFont);
    lf.lfHeight = -xg_nCellSize * xg_nSmallCharPercents / 100;
    lf.lfWidth = 0;
    lf.lfWeight = FW_NORMAL;
    lf.lfQuality = ANTIALIASED_QUALITY;
    HFONT hFontSmall = ::CreateFontIndirectW(&lf);

    // �S�̂𔒂œh��Ԃ��B
    RECT rc;
    ::SetRect(&rc, 0, 0, psiz->cx, psiz->cy);
    ::FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH)));

    // ��d�}�X��`�悷��B
    WCHAR sz[32];
    SIZE siz;
    HGDIOBJ hFontOld = ::SelectObject(hdc, hFontSmall);
    HGDIOBJ hPenOld = ::SelectObject(hdc, hThinPen);
    for (int i = 0; i < nCount; i++) {
        ::SetRect(&rc,
            static_cast<int>(xg_nNarrowMargin + i * xg_nCellSize), 
            static_cast<int>(xg_nNarrowMargin + 0 * xg_nCellSize),
            static_cast<int>(xg_nNarrowMargin + (i + 1) * xg_nCellSize - 1), 
            static_cast<int>(xg_nNarrowMargin + 1 * xg_nCellSize));
        ::FillRect(hdc, &rc, hbrMarked);
        ::InflateRect(&rc, -4, -4);
        if (xg_bDrawFrameForMarkedCell) {
            ::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
            ::Rectangle(hdc, rc.left, rc.top, rc.right + 1, rc.bottom + 1);
        }
        ::InflateRect(&rc, 4, 4);

        // ��d�}�X�̕�����`���B
        ::SetTextColor(hdc, xg_rgbBlackCellColor);
        ::SetBkMode(hdc, OPAQUE);
        ::SetBkColor(hdc, xg_rgbMarkedCellColor);
        StringCbPrintf(sz, sizeof(sz), L"%c", ZEN_LARGE_A + i);
        ::GetTextExtentPoint32W(hdc, sz, int(wcslen(sz)), &siz);
        ::DrawTextW(hdc, sz, -1, &rc, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM);
    }
    ::SelectObject(hdc, hFontOld);
    ::SelectObject(hdc, hPenOld);

    // �}�X�̕�����`�悷��B
    hFontOld = ::SelectObject(hdc, hFont);
    for (int i = 0; i < nCount; i++) {
        ::SetRect(&rc,
            static_cast<int>(xg_nNarrowMargin + i * xg_nCellSize), 
            static_cast<int>(xg_nNarrowMargin + 0 * xg_nCellSize),
            static_cast<int>(xg_nNarrowMargin + (i + 1) * xg_nCellSize), 
            static_cast<int>(xg_nNarrowMargin + 1 * xg_nCellSize));

        WCHAR ch;
        XG_Pos& pos = xg_vMarks[i];
        if (xg_bSolved && xg_bShowAnswer) {
            ch = xg_solution.GetAt(pos.m_i, pos.m_j);
        } else {
            ch = xg_xword.GetAt(pos.m_i, pos.m_j);
        }

        // ������ϊ�����B
        if (xg_bHiragana) {
            WCHAR new_ch;
            LCMapStringW(JPN_LOCALE, LCMAP_FULLWIDTH | LCMAP_HIRAGANA, &ch, 1, &new_ch, 1);
            ch = new_ch;
        }
        if (xg_bLowercase) {
            WCHAR new_ch;
            LCMapStringW(JPN_LOCALE, LCMAP_FULLWIDTH | LCMAP_LOWERCASE, &ch, 1, &new_ch, 1);
            ch = new_ch;
        }

        // �}�X�̕�����`�悷��B
        ::SetTextColor(hdc, xg_rgbBlackCellColor);
        ::SetBkMode(hdc, TRANSPARENT);
        ::DrawTextW(hdc, &ch, 1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    }
    ::SelectObject(hdc, hFontOld);

    // ���������B�h��Ԃ��Ȃ��B
    ::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
    hPenOld = ::SelectObject(hdc, hThinPen);
    for (int i = 0; i < nCount; i++) {
        ::Rectangle(hdc,
                static_cast<int>(xg_nNarrowMargin + i * xg_nCellSize),
                static_cast<int>(xg_nNarrowMargin + 0 * xg_nCellSize),
                static_cast<int>(xg_nNarrowMargin + (i + 1) * xg_nCellSize + 1),
                static_cast<int>(xg_nNarrowMargin + 1 * xg_nCellSize) + 1);
    }
    ::SelectObject(hdc, hPenOld);

    // ����ɑ�������`���B
    if (xg_bAddThickFrame) {
        hPenOld = ::SelectObject(hdc, hWidePen);
        c_nWide /= 2;
        ::MoveToEx(hdc, xg_nNarrowMargin - c_nWide, xg_nNarrowMargin - c_nWide, nullptr);
        ::LineTo(hdc, psiz->cx - xg_nNarrowMargin + c_nWide, xg_nNarrowMargin - c_nWide);
        ::LineTo(hdc, psiz->cx - xg_nNarrowMargin + c_nWide, psiz->cy - xg_nNarrowMargin + c_nWide);
        ::LineTo(hdc, xg_nNarrowMargin - c_nWide, psiz->cy - xg_nNarrowMargin + c_nWide);
        ::LineTo(hdc, xg_nNarrowMargin - c_nWide, xg_nNarrowMargin - c_nWide);
        ::SelectObject(hdc, hPenOld);
    }

    // �j������B
    ::DeleteObject(hWidePen);
    ::DeleteObject(hThinPen);
    ::DeleteObject(hFont);
    ::DeleteObject(hFontSmall);
    ::DeleteObject(hbrBlack);
    ::DeleteObject(hbrWhite);
    ::DeleteObject(hbrMarked);
}

// �N���X���[�h��`�悷��i�ʏ�r���[�j�B
void __fastcall XgDrawXWord_NormalView(XG_Board& xw, HDC hdc, LPSIZE psiz, bool bCaret)
{
    INT nCellSize;
    if (xg_nForDisplay > 0) {
        nCellSize = xg_nCellSize * xg_nZoomRate / 100;
    } else {
        nCellSize = xg_nCellSize;
    }

    // �S�̂𔒂œh��Ԃ��B
    RECT rc;
    ::SetRect(&rc, 0, 0, psiz->cx, psiz->cy);
    ::FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH)));

    LOGFONTW lf;

    // �����}�X�̃t�H���g���쐬����B
    ZeroMemory(&lf, sizeof(lf));
    // ���̑��B
    StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), XgLoadStringDx1(IDS_MONOFONT));
    if (xg_szCellFont[0])
        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szCellFont);
    lf.lfHeight = -nCellSize * xg_nCellCharPercents / 100;
    lf.lfWidth = 0;
    lf.lfQuality = ANTIALIASED_QUALITY;
    HFONT hFont = ::CreateFontIndirectW(&lf);

    // �����������̃t�H���g���쐬����B
    ZeroMemory(&lf, sizeof(lf));
    if (xg_szSmallFont[0])
        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szSmallFont);
    lf.lfHeight = -nCellSize * xg_nSmallCharPercents / 100;
    lf.lfWidth = 0;
    lf.lfQuality = ANTIALIASED_QUALITY;
    HFONT hFontSmall = ::CreateFontIndirectW(&lf);

    // �u���V���쐬����B
    HBRUSH hbrBlack = ::CreateSolidBrush(xg_rgbBlackCellColor);
    HBRUSH hbrWhite = ::CreateSolidBrush(xg_rgbWhiteCellColor);
    HBRUSH hbrMarked = ::CreateSolidBrush(xg_rgbMarkedCellColor);

    // ���ׂ̍��y�����쐬����B
    HPEN hThinPen = ::CreatePen(PS_SOLID, 1, xg_rgbBlackCellColor);

    // �Ԃ��L�����b�g�y�����쐬����B
    LOGBRUSH lbRed;
    lbRed.lbStyle = BS_SOLID;
    lbRed.lbColor = RGB(255, 0, 0);
    HPEN hCaretPen = ::ExtCreatePen(
        PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_BEVEL,
        1, &lbRed, 0, NULL);

    // ���������y�����쐬����B
    LOGBRUSH lbBlack;
    ::GetObject(hbrBlack, sizeof(lbBlack), &lbBlack);
    int c_nWide = 4;
    HPEN hWidePen = ::ExtCreatePen(
        PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_SQUARE | PS_JOIN_BEVEL,
        c_nWide, &lbBlack, 0, NULL);

    WCHAR sz[32];
    SIZE siz;
    HGDIOBJ hFontOld, hPenOld;

    BITMAP bm;
    GetObject(xg_hbmBlackCell, sizeof(bm), &bm);

    HDC hdcMem = ::CreateCompatibleDC(NULL);
    SelectObject(hdcMem, xg_hbmBlackCell);
    SetStretchBltMode(hdcMem, STRETCH_HALFTONE);

    // �Z����`�悷��B
    for (int i = 0; i < xg_nRows; i++) {
        for (int j = 0; j < xg_nCols; j++) {
            // �Z���̍��W���Z�b�g����B
            ::SetRect(&rc,
                static_cast<int>(xg_nMargin + j * nCellSize), 
                static_cast<int>(xg_nMargin + i * nCellSize),
                static_cast<int>(xg_nMargin + (j + 1) * nCellSize), 
                static_cast<int>(xg_nMargin + (i + 1) * nCellSize));

            // ��d�}�X���H
            int nMarked = XgGetMarked(i, j);

            // �h��Ԃ��B
            WCHAR ch = xw.GetAt(i, j);
            if (ch == ZEN_BLACK) {
                // ���}�X�B
                if (xg_hbmBlackCell)
                {
                    StretchBlt(hdc,
                               rc.left, rc.top,
                               rc.right - rc.left, rc.bottom - rc.top,
                               hdcMem,
                               0, 0, bm.bmWidth, bm.bmHeight,
                               SRCCOPY);
                }
                else if (xg_hBlackCellEMF)
                {
                    ::PlayEnhMetaFile(hdc, xg_hBlackCellEMF, &rc);
                }
                else
                {
                    ::FillRect(hdc, &rc, hbrBlack);
                }
            } else if (nMarked != -1) {
                // ��d�}�X�B
                ::FillRect(hdc, &rc, hbrMarked);
            } else {
                // ���̑��̃}�X�B
                ::FillRect(hdc, &rc, hbrWhite);
            }

            // �����̔w�i�͓����B�h��Ԃ��Ȃ��B
            ::SetBkMode(hdc, TRANSPARENT);

            // ������ϊ�����B
            if (xg_bHiragana) {
                WCHAR new_ch;
                LCMapStringW(JPN_LOCALE, LCMAP_FULLWIDTH | LCMAP_HIRAGANA, &ch, 1, &new_ch, 1);
                ch = new_ch;
            }
            if (xg_bLowercase) {
                WCHAR new_ch;
                LCMapStringW(JPN_LOCALE, LCMAP_FULLWIDTH | LCMAP_LOWERCASE, &ch, 1, &new_ch, 1);
                ch = new_ch;
            }

            if (ch != ZEN_BLACK)
            {
                // �����������B
                hFontOld = ::SelectObject(hdc, hFont);
                ::SetTextColor(hdc, xg_rgbBlackCellColor);
                ::DrawTextW(hdc, &ch, 1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                ::SelectObject(hdc, hFontOld);
            }
        }
    }

    ::DeleteDC(hdcMem);

    // �����������̃t�H���g��I������B
    hFontOld = ::SelectObject(hdc, hFontSmall);

    // ��d�}�X��`�悷��B
    for (int i = 0; i < xg_nRows; i++) {
        for (int j = 0; j < xg_nCols; j++) {
            // �Z���̍��W���Z�b�g����B
            ::SetRect(&rc,
                static_cast<int>(xg_nMargin + j * nCellSize),
                static_cast<int>(xg_nMargin + i * nCellSize),
                static_cast<int>(xg_nMargin + (j + 1) * nCellSize) - 1,
                static_cast<int>(xg_nMargin + (i + 1) * nCellSize));

            // ��d�}�X���H
            int nMarked = XgGetMarked(i, j);
            if (nMarked == -1) {
                continue;
            }

            // ��d�}�X�̓����̘g��`���B
            if (xg_bDrawFrameForMarkedCell) {
                ::InflateRect(&rc, -4, -4);
                ::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
                hPenOld = ::SelectObject(hdc, hThinPen);
                ::Rectangle(hdc, rc.left, rc.top, rc.right + 1, rc.bottom + 1);
                ::SelectObject(hdc, hPenOld);
                ::InflateRect(&rc, 4, 4);
            }

            StringCbPrintf(sz, sizeof(sz), L"%c", L'A' + nMarked);

            // ��d�}�X�̉E���[�̕����̔w�i��h��Ԃ��B
            RECT rcText;
            GetTextExtentPoint32(hdc, sz, lstrlen(sz), &siz);
            rcText = rc;
            rcText.left = rc.right - std::max(siz.cx, siz.cy);
            rcText.top = rc.bottom - std::max(siz.cx, siz.cy);

            HBRUSH hbr = CreateSolidBrush(xg_rgbMarkedCellColor);
            FillRect(hdc, &rcText, hbr);
            DeleteObject(hbr);

            // ��d�}�X�̉E���[�̕�����`���B
            ::SetBkMode(hdc, TRANSPARENT);
            ::DrawTextW(hdc, sz, -1, &rcText, DT_CENTER | DT_SINGLELINE | DT_BOTTOM);
        }
    }

    // �^�e�̃J�M�̐擪�}�X�B
    {
        const int size = static_cast<int>(xg_vTateInfo.size());
        for (int k = 0; k < size; k++) {
            const int i = xg_vTateInfo[k].m_iRow;
            const int j = xg_vTateInfo[k].m_jCol;
            StringCbPrintf(sz, sizeof(sz), L"%u", xg_vTateInfo[k].m_number);

            // �����̔w�i��h��Ԃ��B
            ::SetBkMode(hdc, OPAQUE);
            int nMarked = XgGetMarked(i, j);
            if (nMarked != -1) {
                ::SetBkColor(hdc, xg_rgbMarkedCellColor);
            } else {
                ::SetBkColor(hdc, xg_rgbWhiteCellColor);
            }

            if (xg_bShowNumbering) {
                // ������`���B
                ::SetRect(&rc,
                    static_cast<int>(xg_nMargin + j * nCellSize), 
                    static_cast<int>(xg_nMargin + i * nCellSize),
                    static_cast<int>(xg_nMargin + (j + 1) * nCellSize), 
                    static_cast<int>(xg_nMargin + (i + 1) * nCellSize));
                ::OffsetRect(&rc, 2, 1);
                ::DrawTextW(hdc, sz, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_TOP);
            }
        }
    }
    // ���R�̃J�M�̐擪�}�X�B
    {
        const int size = static_cast<int>(xg_vYokoInfo.size());
        for (int k = 0; k < size; k++) {
            const int i = xg_vYokoInfo[k].m_iRow;
            const int j = xg_vYokoInfo[k].m_jCol;
            StringCbPrintf(sz, sizeof(sz), L"%u", xg_vYokoInfo[k].m_number);

            // �����̔w�i��h��Ԃ��B
            int nMarked = XgGetMarked(i, j);
            if (nMarked != -1) {
                ::SetBkColor(hdc, xg_rgbMarkedCellColor);
            } else {
                ::SetBkColor(hdc, xg_rgbWhiteCellColor);
            }

            if (xg_bShowNumbering) {
                // ������`���B
                ::SetRect(&rc,
                    static_cast<int>(xg_nMargin + j * nCellSize), 
                    static_cast<int>(xg_nMargin + i * nCellSize),
                    static_cast<int>(xg_nMargin + (j + 1) * nCellSize), 
                    static_cast<int>(xg_nMargin + (i + 1) * nCellSize));
                ::OffsetRect(&rc, 2, 1);
                ::DrawTextW(hdc, sz, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_TOP);
            }
        }
    }

    // �t�H���g�̑I������������B
    ::SelectObject(hdc, hFontOld);

    // �L�����b�g��`�悷��B
    if (bCaret && xg_bShowCaret) {
        const int i = xg_caret_pos.m_i;
        const int j = xg_caret_pos.m_j;
        ::SetRect(&rc,
            static_cast<int>(xg_nMargin + j * nCellSize), 
            static_cast<int>(xg_nMargin + i * nCellSize),
            static_cast<int>(xg_nMargin + (j + 1) * nCellSize), 
            static_cast<int>(xg_nMargin + (i + 1) * nCellSize));

        const int cxyMargin = nCellSize / 10;
        const int cxyLine = nCellSize / 3;
        const int cxyCross = nCellSize / 10;

        hPenOld = ::SelectObject(hdc, hCaretPen);
        ::MoveToEx(hdc, rc.left + cxyMargin, rc.top + cxyMargin, nullptr);
        ::LineTo(hdc, rc.left + cxyMargin, rc.top + cxyLine);
        ::MoveToEx(hdc, rc.left + cxyMargin, rc.top + cxyMargin, nullptr);
        ::LineTo(hdc, rc.left + cxyLine, rc.top + cxyMargin);
        ::MoveToEx(hdc, rc.right - cxyMargin, rc.top + cxyMargin, nullptr);
        ::LineTo(hdc, rc.right - cxyMargin, rc.top + cxyLine);
        ::MoveToEx(hdc, rc.right - cxyMargin, rc.top + cxyMargin, nullptr);
        ::LineTo(hdc, rc.right - cxyLine, rc.top + cxyMargin);
        ::MoveToEx(hdc, rc.right - cxyMargin, rc.bottom - cxyMargin, nullptr);
        ::LineTo(hdc, rc.right - cxyMargin, rc.bottom - cxyLine);
        ::MoveToEx(hdc, rc.right - cxyMargin, rc.bottom - cxyMargin, nullptr);
        ::LineTo(hdc, rc.right - cxyLine, rc.bottom - cxyMargin);
        ::MoveToEx(hdc, rc.left + cxyMargin, rc.bottom - cxyMargin, nullptr);
        ::LineTo(hdc, rc.left + cxyMargin, rc.bottom - cxyLine);
        ::MoveToEx(hdc, rc.left + cxyMargin, rc.bottom - cxyMargin, nullptr);
        ::LineTo(hdc, rc.left + cxyLine, rc.bottom - cxyMargin);

        ::MoveToEx(hdc, (rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2 - cxyCross, nullptr);
        ::LineTo(hdc, (rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2 + cxyCross);
        ::MoveToEx(hdc, (rc.left + rc.right) / 2 - cxyCross, (rc.top + rc.bottom) / 2, nullptr);
        ::LineTo(hdc, (rc.left + rc.right) / 2 + cxyCross, (rc.top + rc.bottom) / 2);
        ::SelectObject(hdc, hPenOld);
    }

    // ���������B
    hPenOld = ::SelectObject(hdc, hThinPen);
    for (int i = 0; i <= xg_nRows; i++) {
        ::MoveToEx(hdc, xg_nMargin, static_cast<int>(xg_nMargin + i * nCellSize), nullptr);
        ::LineTo(hdc, psiz->cx - xg_nMargin, static_cast<int>(xg_nMargin + i * nCellSize));
    }
    for (int j = 0; j <= xg_nCols; j++) {
        ::MoveToEx(hdc, static_cast<int>(xg_nMargin + j * nCellSize), xg_nMargin, nullptr);
        ::LineTo(hdc, static_cast<int>(xg_nMargin + j * nCellSize), psiz->cy - xg_nMargin);
    }
    ::SelectObject(hdc, hPenOld);

    // ����ɑ�������`���B
    hPenOld = ::SelectObject(hdc, hWidePen);
    if (xg_bAddThickFrame) {
        c_nWide /= 2;
        ::MoveToEx(hdc, xg_nMargin - c_nWide, xg_nMargin - c_nWide, nullptr);
        ::LineTo(hdc, psiz->cx - xg_nMargin + c_nWide, xg_nMargin - c_nWide);
        ::LineTo(hdc, psiz->cx - xg_nMargin + c_nWide, psiz->cy - xg_nMargin + c_nWide);
        ::LineTo(hdc, xg_nMargin - c_nWide, psiz->cy - xg_nMargin + c_nWide);
        ::LineTo(hdc, xg_nMargin - c_nWide, xg_nMargin - c_nWide);
    }
    ::SelectObject(hdc, hPenOld);

    // �j������B
    ::DeleteObject(hFont);
    ::DeleteObject(hFontSmall);
    ::DeleteObject(hThinPen);
    ::DeleteObject(hWidePen);
    ::DeleteObject(hCaretPen);
    ::DeleteObject(hbrBlack);
    ::DeleteObject(hbrWhite);
    ::DeleteObject(hbrMarked);
}

// �N���X���[�h��`�悷��i�X�P���g���r���[�j�B
void __fastcall XgDrawXWord_SkeltonView(XG_Board& xw, HDC hdc, LPSIZE psiz, bool bCaret)
{
    INT nCellSize;
    if (xg_nForDisplay > 0) {
        nCellSize = xg_nCellSize * xg_nZoomRate / 100;
    } else {
        nCellSize = xg_nCellSize;
    }

    // �S�̂𔒂œh��Ԃ��B
    RECT rc;
    ::SetRect(&rc, 0, 0, psiz->cx, psiz->cy);
    ::FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH)));

    LOGFONTW lf;

    // �����}�X�̃t�H���g���쐬����B
    ZeroMemory(&lf, sizeof(lf));
    // ���̑��B
    StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), XgLoadStringDx1(IDS_MONOFONT));
    if (xg_szCellFont[0])
        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szCellFont);
    lf.lfHeight = -nCellSize * xg_nCellCharPercents / 100;
    lf.lfWidth = 0;
    lf.lfQuality = ANTIALIASED_QUALITY;
    HFONT hFont = ::CreateFontIndirectW(&lf);

    // �����������̃t�H���g���쐬����B
    ZeroMemory(&lf, sizeof(lf));
    if (xg_szSmallFont[0])
        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), xg_szSmallFont);
    lf.lfHeight = -nCellSize * xg_nSmallCharPercents / 100;
    lf.lfWidth = 0;
    lf.lfQuality = ANTIALIASED_QUALITY;
    HFONT hFontSmall = ::CreateFontIndirectW(&lf);

    // �u���V���쐬����B
    HBRUSH hbrBlack = ::CreateSolidBrush(xg_rgbBlackCellColor);
    HBRUSH hbrWhite = ::CreateSolidBrush(xg_rgbWhiteCellColor);
    HBRUSH hbrMarked = ::CreateSolidBrush(xg_rgbMarkedCellColor);

    // ���ׂ̍��y�����쐬����B
    HPEN hThinPen = ::CreatePen(PS_SOLID, 1, xg_rgbBlackCellColor);

    // �Ԃ��L�����b�g�y�����쐬����B
    LOGBRUSH lbRed;
    lbRed.lbStyle = BS_SOLID;
    lbRed.lbColor = RGB(255, 0, 0);
    HPEN hCaretPen = ::ExtCreatePen(
        PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_BEVEL,
        1, &lbRed, 0, NULL);

    // ���������y�����쐬����B
    LOGBRUSH lbBlack;
    ::GetObject(hbrBlack, sizeof(lbBlack), &lbBlack);
    int c_nWide = 4;
    HPEN hWidePen = ::ExtCreatePen(
        PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_SQUARE | PS_JOIN_BEVEL,
        c_nWide, &lbBlack, 0, NULL);

    WCHAR sz[32];
    SIZE siz;
    HGDIOBJ hFontOld, hPenOld;

    BITMAP bm;
    GetObject(xg_hbmBlackCell, sizeof(bm), &bm);

    HDC hdcMem = ::CreateCompatibleDC(NULL);
    SelectObject(hdcMem, xg_hbmBlackCell);
    SetStretchBltMode(hdcMem, STRETCH_HALFTONE);

    // �Z���̔w�i��`�悷��B
    for (int i = 0; i < xg_nRows; i++) {
        for (int j = 0; j < xg_nCols; j++) {
            // �Z���̍��W���Z�b�g����B
            ::SetRect(&rc,
                static_cast<int>(xg_nMargin + j * nCellSize), 
                static_cast<int>(xg_nMargin + i * nCellSize),
                static_cast<int>(xg_nMargin + (j + 1) * nCellSize), 
                static_cast<int>(xg_nMargin + (i + 1) * nCellSize));

            RECT rcExtended = rc;
            InflateRect(&rcExtended, c_nWide, c_nWide);

            WCHAR ch = xw.GetAt(i, j);
            if (ch != ZEN_BLACK) {
                // �w�i��h��Ԃ��B
                ::FillRect(hdc, &rcExtended, hbrBlack);
            }
        }
    }

    for (int i = 0; i < xg_nRows; i++) {
        for (int j = 0; j < xg_nCols; j++) {
            // �Z���̍��W���Z�b�g����B
            ::SetRect(&rc,
                static_cast<int>(xg_nMargin + j * nCellSize), 
                static_cast<int>(xg_nMargin + i * nCellSize),
                static_cast<int>(xg_nMargin + (j + 1) * nCellSize), 
                static_cast<int>(xg_nMargin + (i + 1) * nCellSize));

            // ��d�}�X���H
            int nMarked = XgGetMarked(i, j);

            WCHAR ch = xw.GetAt(i, j);
            if (ch == ZEN_BLACK)
                continue;

            // �h��Ԃ��B
            if (nMarked != -1) {
                // ��d�}�X�B
                ::FillRect(hdc, &rc, hbrMarked);
            } else {
                // ���̑��̃}�X�B
                ::FillRect(hdc, &rc, hbrWhite);
            }

            // �����̔w�i�͓����B�h��Ԃ��Ȃ��B
            ::SetBkMode(hdc, TRANSPARENT);

            // ������ϊ�����B
            if (xg_bHiragana) {
                WCHAR new_ch;
                LCMapStringW(JPN_LOCALE, LCMAP_FULLWIDTH | LCMAP_HIRAGANA, &ch, 1, &new_ch, 1);
                ch = new_ch;
            }
            if (xg_bLowercase) {
                WCHAR new_ch;
                LCMapStringW(JPN_LOCALE, LCMAP_FULLWIDTH | LCMAP_LOWERCASE, &ch, 1, &new_ch, 1);
                ch = new_ch;
            }

            if (ch != ZEN_BLACK) {
                // �����������B
                hFontOld = ::SelectObject(hdc, hFont);
                ::SetTextColor(hdc, xg_rgbBlackCellColor);
                ::DrawTextW(hdc, &ch, 1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                ::SelectObject(hdc, hFontOld);

                // ���������B
                hPenOld = ::SelectObject(hdc, hThinPen);
                {
                    ::MoveToEx(hdc, static_cast<int>(xg_nMargin + j * nCellSize), static_cast<int>(xg_nMargin + i * nCellSize), nullptr);
                    ++i;
                    ::LineTo(hdc, static_cast<int>(xg_nMargin + j * nCellSize), static_cast<int>(xg_nMargin + i * nCellSize));
                    --i;

                    ::MoveToEx(hdc, static_cast<int>(xg_nMargin + j * nCellSize), static_cast<int>(xg_nMargin + i * nCellSize), nullptr);
                    ++j;
                    ::LineTo(hdc, static_cast<int>(xg_nMargin + j * nCellSize), static_cast<int>(xg_nMargin + i * nCellSize));
                    --j;
                }
                ::SelectObject(hdc, hPenOld);
            }
        }
    }

    ::DeleteDC(hdcMem);

    // �����������̃t�H���g��I������B
    hFontOld = ::SelectObject(hdc, hFontSmall);

    // ��d�}�X��`�悷��B
    for (int i = 0; i < xg_nRows; i++) {
        for (int j = 0; j < xg_nCols; j++) {
            // �Z���̍��W���Z�b�g����B
            ::SetRect(&rc,
                static_cast<int>(xg_nMargin + j * nCellSize),
                static_cast<int>(xg_nMargin + i * nCellSize),
                static_cast<int>(xg_nMargin + (j + 1) * nCellSize) - 1,
                static_cast<int>(xg_nMargin + (i + 1) * nCellSize));

            // ��d�}�X���H
            int nMarked = XgGetMarked(i, j);
            if (nMarked == -1) {
                continue;
            }

            // ��d�}�X�̓����̘g��`���B
            if (xg_bDrawFrameForMarkedCell) {
                ::InflateRect(&rc, -4, -4);
                ::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
                hPenOld = ::SelectObject(hdc, hThinPen);
                ::Rectangle(hdc, rc.left, rc.top, rc.right + 1, rc.bottom + 1);
                ::SelectObject(hdc, hPenOld);
                ::InflateRect(&rc, 4, 4);
            }

            StringCbPrintf(sz, sizeof(sz), L"%c", L'A' + nMarked);

            // ��d�}�X�̉E���[�̕����̔w�i��h��Ԃ��B
            RECT rcText;
            GetTextExtentPoint32(hdc, sz, lstrlen(sz), &siz);
            rcText = rc;
            rcText.left = rc.right - std::max(siz.cx, siz.cy);
            rcText.top = rc.bottom - std::max(siz.cx, siz.cy);

            HBRUSH hbr = CreateSolidBrush(xg_rgbMarkedCellColor);
            FillRect(hdc, &rcText, hbr);
            DeleteObject(hbr);

            // ��d�}�X�̉E���[�̕�����`���B
            ::SetBkMode(hdc, TRANSPARENT);
            ::DrawTextW(hdc, sz, -1, &rcText, DT_CENTER | DT_SINGLELINE | DT_BOTTOM);
        }
    }

    // �^�e�̃J�M�̐擪�}�X�B
    {
        const int size = static_cast<int>(xg_vTateInfo.size());
        for (int k = 0; k < size; k++) {
            const int i = xg_vTateInfo[k].m_iRow;
            const int j = xg_vTateInfo[k].m_jCol;
            StringCbPrintf(sz, sizeof(sz), L"%u", xg_vTateInfo[k].m_number);

            // �����̔w�i��h��Ԃ��B
            ::SetBkMode(hdc, OPAQUE);
            int nMarked = XgGetMarked(i, j);
            if (nMarked != -1) {
                ::SetBkColor(hdc, xg_rgbMarkedCellColor);
            } else {
                ::SetBkColor(hdc, xg_rgbWhiteCellColor);
            }

            if (xg_bShowNumbering) {
                // ������`���B
                ::SetRect(&rc,
                    static_cast<int>(xg_nMargin + j * nCellSize), 
                    static_cast<int>(xg_nMargin + i * nCellSize),
                    static_cast<int>(xg_nMargin + (j + 1) * nCellSize), 
                    static_cast<int>(xg_nMargin + (i + 1) * nCellSize));
                ::OffsetRect(&rc, 2, 1);
                ::DrawTextW(hdc, sz, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_TOP);
            }
        }
    }
    // ���R�̃J�M�̐擪�}�X�B
    {
        const int size = static_cast<int>(xg_vYokoInfo.size());
        for (int k = 0; k < size; k++) {
            const int i = xg_vYokoInfo[k].m_iRow;
            const int j = xg_vYokoInfo[k].m_jCol;
            StringCbPrintf(sz, sizeof(sz), L"%u", xg_vYokoInfo[k].m_number);

            // �����̔w�i��h��Ԃ��B
            int nMarked = XgGetMarked(i, j);
            if (nMarked != -1) {
                ::SetBkColor(hdc, xg_rgbMarkedCellColor);
            } else {
                ::SetBkColor(hdc, xg_rgbWhiteCellColor);
            }

            if (xg_bShowNumbering) {
                // ������`���B
                ::SetRect(&rc,
                    static_cast<int>(xg_nMargin + j * nCellSize), 
                    static_cast<int>(xg_nMargin + i * nCellSize),
                    static_cast<int>(xg_nMargin + (j + 1) * nCellSize), 
                    static_cast<int>(xg_nMargin + (i + 1) * nCellSize));
                ::OffsetRect(&rc, 2, 1);
                ::DrawTextW(hdc, sz, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_TOP);
            }
        }
    }

    // �t�H���g�̑I������������B
    ::SelectObject(hdc, hFontOld);

    // �L�����b�g��`�悷��B
    if (bCaret && xg_bShowCaret) {
        const int i = xg_caret_pos.m_i;
        const int j = xg_caret_pos.m_j;
        ::SetRect(&rc,
            static_cast<int>(xg_nMargin + j * nCellSize), 
            static_cast<int>(xg_nMargin + i * nCellSize),
            static_cast<int>(xg_nMargin + (j + 1) * nCellSize), 
            static_cast<int>(xg_nMargin + (i + 1) * nCellSize));

        const int cxyMargin = nCellSize / 10;
        const int cxyLine = nCellSize / 3;
        const int cxyCross = nCellSize / 10;

        hPenOld = ::SelectObject(hdc, hCaretPen);
        ::MoveToEx(hdc, rc.left + cxyMargin, rc.top + cxyMargin, nullptr);
        ::LineTo(hdc, rc.left + cxyMargin, rc.top + cxyLine);
        ::MoveToEx(hdc, rc.left + cxyMargin, rc.top + cxyMargin, nullptr);
        ::LineTo(hdc, rc.left + cxyLine, rc.top + cxyMargin);
        ::MoveToEx(hdc, rc.right - cxyMargin, rc.top + cxyMargin, nullptr);
        ::LineTo(hdc, rc.right - cxyMargin, rc.top + cxyLine);
        ::MoveToEx(hdc, rc.right - cxyMargin, rc.top + cxyMargin, nullptr);
        ::LineTo(hdc, rc.right - cxyLine, rc.top + cxyMargin);
        ::MoveToEx(hdc, rc.right - cxyMargin, rc.bottom - cxyMargin, nullptr);
        ::LineTo(hdc, rc.right - cxyMargin, rc.bottom - cxyLine);
        ::MoveToEx(hdc, rc.right - cxyMargin, rc.bottom - cxyMargin, nullptr);
        ::LineTo(hdc, rc.right - cxyLine, rc.bottom - cxyMargin);
        ::MoveToEx(hdc, rc.left + cxyMargin, rc.bottom - cxyMargin, nullptr);
        ::LineTo(hdc, rc.left + cxyMargin, rc.bottom - cxyLine);
        ::MoveToEx(hdc, rc.left + cxyMargin, rc.bottom - cxyMargin, nullptr);
        ::LineTo(hdc, rc.left + cxyLine, rc.bottom - cxyMargin);

        ::MoveToEx(hdc, (rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2 - cxyCross, nullptr);
        ::LineTo(hdc, (rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2 + cxyCross);
        ::MoveToEx(hdc, (rc.left + rc.right) / 2 - cxyCross, (rc.top + rc.bottom) / 2, nullptr);
        ::LineTo(hdc, (rc.left + rc.right) / 2 + cxyCross, (rc.top + rc.bottom) / 2);
        ::SelectObject(hdc, hPenOld);
    }

    // �j������B
    ::DeleteObject(hFont);
    ::DeleteObject(hFontSmall);
    ::DeleteObject(hThinPen);
    ::DeleteObject(hWidePen);
    ::DeleteObject(hCaretPen);
    ::DeleteObject(hbrBlack);
    ::DeleteObject(hbrWhite);
    ::DeleteObject(hbrMarked);
}

// �N���X���[�h��`�悷��B
void __fastcall XgDrawXWord(XG_Board& xw, HDC hdc, LPSIZE psiz, bool bCaret)
{
    switch (xg_nViewMode)
    {
    case XG_VIEW_NORMAL:
    default:
        XgDrawXWord_NormalView(xw, hdc, psiz, bCaret);
        break;
    case XG_VIEW_SKELTON:
        XgDrawXWord_SkeltonView(xw, hdc, psiz, bCaret);
        break;
    }
}

// �N���X���[�h�̃C���[�W���쐬����B
HBITMAP __fastcall XgCreateXWordImage(XG_Board& xw, LPSIZE psiz, bool bCaret)
{
    // �݊�DC���쐬����B
    HDC hdc = ::CreateCompatibleDC(nullptr);
    if (hdc == nullptr)
        return nullptr;

    // DIB���쐬����B
    BITMAPINFO bi;
    LPVOID pvBits;
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = psiz->cx;
    bi.bmiHeader.biHeight = psiz->cy;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    HBITMAP hbm;
    hbm = ::CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    if (hbm == nullptr) {
        DeleteDC(hdc);
        return nullptr;
    }

    // �`�悷��B
    HGDIOBJ hbmOld = ::SelectObject(hdc, hbm);
    XgDrawXWord(xw, hdc, psiz, bCaret);
    ::SelectObject(hdc, hbmOld);

    // �݊�DC��j������B
    ::DeleteDC(hdc);
    return hbm;
}

// �`��C���[�W���X�V����B
void __fastcall XgUpdateImage(HWND hwnd, int x, int y)
{
    ForDisplay for_display;

    // �C���[�W������Δj������B
    if (xg_hbmImage)
        ::DeleteObject(xg_hbmImage);

    // �`��T�C�Y���擾���A�C���[�W���쐬����B
    SIZE siz;
    XgGetXWordExtent(&siz);
    if (xg_bSolved && xg_bShowAnswer)
        xg_hbmImage = XgCreateXWordImage(xg_solution, &siz, true);
    else
        xg_hbmImage = XgCreateXWordImage(xg_xword, &siz, true);

    // �X�N���[�������X�V����B
    XgUpdateScrollInfo(hwnd, x, y);

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

    // �ĕ`�悷��B
    ::InvalidateRect(hwnd, &rcClient, TRUE);
}

// CRP�t�@�C�����J���B
bool __fastcall XgDoLoadCrpFile(HWND hwnd, LPCWSTR pszFile)
{
    INT i, nWidth, nHeight;
    std::vector<std::wstring> rows;
    WCHAR szName[32], szText[128];
    XG_Board xword;
    std::vector<XG_Hint> tate, yoko;
    bool bOK = false;

    nWidth = GetPrivateProfileIntW(L"Cross", L"Width", -1, pszFile);
    nHeight = GetPrivateProfileIntW(L"Cross", L"Height", -1, pszFile);

    static const WCHAR sz1[] = { ZEN_UNDERLINE, 0 };
    static const WCHAR sz2[] = { ZEN_SPACE, 0 };

    if (nWidth > 0 && nHeight > 0) {
        for (i = 0; i < nHeight; ++i) {
            StringCbPrintf(szName, sizeof(szName), L"Line%u", i + 1);
            GetPrivateProfileStringW(L"Cross", szName, L"", szText, ARRAYSIZE(szText), pszFile);

            std::wstring str = szText;
            xg_str_trim(str);
            xg_str_replace_all(str, L",", L"");
            xg_str_replace_all(str, sz1, sz2);
            str = XgNormalizeString(str);

            if (INT(str.size()) != nWidth)
                break;

            rows.push_back(str);
        }
        if (i == nHeight) {
            std::wstring str;

            str += ZEN_ULEFT;
            for (i = 0; i < nWidth; ++i) {
                str += ZEN_HLINE;
            }
            str += ZEN_URIGHT;
            str += L"\r\n";

            for (auto& item : rows) {
                str += ZEN_VLINE;
                for (i = 0; i < nWidth; ++i) {
                    str += item[i];
                }
                str += ZEN_VLINE;
                str += L"\r\n";
            }

            str += ZEN_LLEFT;
            for (i = 0; i < nWidth; ++i) {
                str += ZEN_HLINE;
            }
            str += ZEN_LRIGHT;
            str += L"\r\n";

            // �������ǂݍ��ށB
            bOK = xword.SetString(str);
        }
    }

    if (bOK) {
        xg_nCols = nWidth;
        xg_nRows = nHeight;
        xg_vecTateHints.clear();
        xg_vecYokoHints.clear();
        XgDestroyCandsWnd();
        XgDestroyHintsWnd();
        xg_bSolved = false;

        if (xword.IsFulfilled()) {
            // �ԍ��t�����s���B
            xword.DoNumberingNoCheck();

            INT nClueCount = GetPrivateProfileIntW(L"Clue", L"Count", 0, pszFile);
            if (nClueCount) {
                for (i = 0; i < nClueCount; ++i) {
                    StringCbPrintf(szName, sizeof(szName), L"Clue%u", i + 1);
                    GetPrivateProfileStringW(L"Clue", szName, L"", szText, ARRAYSIZE(szText), pszFile);

                    std::wstring str = szText;
                    xg_str_trim(str);
                    if (str.empty())
                        break;
                    size_t icolon = str.find(L':');
                    if (icolon != str.npos) {
                        std::wstring word = str.substr(0, icolon);
                        std::wstring hint = str.substr(icolon + 1);
                        word = XgNormalizeString(word);
                        for (XG_PlaceInfo& item : xg_vTateInfo) {
                            if (item.m_word == word) {
                                tate.emplace_back(item.m_number, word, hint);
                                break;
                            }
                        }
                        for (XG_PlaceInfo& item : xg_vYokoInfo) {
                            if (item.m_word == word) {
                                yoko.emplace_back(item.m_number, word, hint);
                                break;
                            }
                        }
                    }
                }
            } else {
                for (i = 0; i < 256; ++i) {
                    StringCbPrintf(szName, sizeof(szName), L"Down%u", i + 1);
                    GetPrivateProfileStringW(L"Clue", szName, L"", szText, ARRAYSIZE(szText), pszFile);

                    std::wstring str = szText;
                    xg_str_trim(str);
                    if (str.empty())
                        break;
                    if (str == L"{N/A}")
                        continue;
                    for (XG_PlaceInfo& item : xg_vTateInfo) {
                        if (item.m_number == i + 1) {
                            tate.emplace_back(item.m_number, item.m_word, str);
                            break;
                        }
                    }
                }

                for (i = 0; i < 256; ++i) {
                    StringCbPrintf(szName, sizeof(szName), L"Across%u", i + 1);
                    GetPrivateProfileStringW(L"Clue", szName, L"", szText, ARRAYSIZE(szText), pszFile);

                    std::wstring str = szText;
                    xg_str_trim(str);
                    if (str.empty())
                        break;
                    if (str == L"{N/A}")
                        continue;
                    for (XG_PlaceInfo& item : xg_vYokoInfo) {
                        if (item.m_number == i + 1) {
                            yoko.emplace_back(item.m_number, item.m_word, str);
                            break;
                        }
                    }
                }
            }
        }

        // �s������ǉ��B
        for (XG_PlaceInfo& item : xg_vYokoInfo) {
            bool found = false;
            for (auto& info : yoko) {
                if (item.m_number == info.m_number) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                yoko.emplace_back(item.m_number, item.m_word, L"");
            }
        }
        for (XG_PlaceInfo& item : xg_vTateInfo) {
            bool found = false;
            for (auto& info : tate) {
                if (item.m_number == info.m_number) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                tate.emplace_back(item.m_number, item.m_word, L"");
            }
        }

        // �\�[�g�B
        std::sort(tate.begin(), tate.end(),
            [](const XG_Hint& a, const XG_Hint& b) {
                return a.m_number < b.m_number;
            }
        );
        std::sort(yoko.begin(), yoko.end(),
            [](const XG_Hint& a, const XG_Hint& b) {
                return a.m_number < b.m_number;
            }
        );

        // �����B
        xg_xword = xword;
        xg_solution = xword;
        if (tate.size() && yoko.size()) {
            xg_bSolved = true;
            xg_bShowAnswer = false;
            XgClearNonBlocks(hwnd);
            xg_vecTateHints = tate;
            xg_vecYokoHints = yoko;
            XgShowHints(hwnd);
        }

        XgUpdateImage(hwnd, 0, 0);
        XgMarkUpdate();

        // �t�@�C���p�X���Z�b�g����B
        WCHAR szFileName[MAX_PATH];
        ::GetFullPathNameW(pszFile, MAX_PATH, szFileName, NULL);
        xg_strFileName = szFileName;
    }

    return bOK;
}

// �t�@�C�����J���B
bool __fastcall XgDoLoadFile(HWND hwnd, LPCWSTR pszFile, bool json)
{
    DWORD i, cbFile, cbRead;
    bool bOK = false;

    // ��d�}�X�P�����ɂ���B
    XgSetMarkedWord();

    // �t�@�C�����J���B
    AutoCloseHandle hFile(::CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ,
                                        nullptr, OPEN_EXISTING, 0, nullptr));
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    // �t�@�C���T�C�Y���擾�B
    cbFile = ::GetFileSize(hFile, nullptr);
    if (cbFile == 0xFFFFFFFF)
        return false;

    try {
        // ���������m�ۂ��ăt�@�C������ǂݍ��ށB
        std::vector<BYTE> pbFile(cbFile + 4, 0);
        i = cbFile;
        if (!::ReadFile(hFile, &pbFile[0], cbFile, &cbRead, nullptr))
            return false;

        // BOM�`�F�b�N�B
        if (pbFile[0] == 0xFF && pbFile[1] == 0xFE) {
            // Unicode
            std::wstring str = reinterpret_cast<LPWSTR>(&pbFile[2]);
            bOK = XgSetString(hwnd, str, json);
            i = 0;
        } else if (pbFile[0] == 0xFE && pbFile[1] == 0xFF) {
            // Unicode BigEndian
            XgSwab(&pbFile[0], cbFile);
            std::wstring str = reinterpret_cast<LPWSTR>(&pbFile[2]);
            bOK = XgSetString(hwnd, str, json);
            i = 0;
        } else if (pbFile[0] == 0xEF && pbFile[1] == 0xBB && pbFile[2] == 0xBF)
        {
            // UTF-8
            std::wstring str = XgUtf8ToUnicode(reinterpret_cast<LPCSTR>(&pbFile[3]));
            bOK = XgSetString(hwnd, str, json);
            i = 0;
        } else {
            for (i = 0; i < cbFile; i++) {
                // �i�������������Unicode�Ɣ��f����B
                if (pbFile[i] == 0) {
                    // �G���f�B�A���̔���B
                    if (i & 1) {
                        // Unicode
                        std::wstring str = reinterpret_cast<LPWSTR>(&pbFile[0]);
                        bOK = XgSetString(hwnd, str, json);
                    } else {
                        // Unicode BE
                        XgSwab(&pbFile[0], cbFile);
                        std::wstring str = reinterpret_cast<LPWSTR>(&pbFile[0]);
                        bOK = XgSetString(hwnd, str, json);
                    }
                    break;
                }
            }
        }

        if (i == cbFile) {
            if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                    reinterpret_cast<LPCSTR>(&pbFile[0]),
                                    static_cast<int>(cbFile), nullptr, 0))
            {
                // UTF-8
                std::wstring str = XgUtf8ToUnicode(reinterpret_cast<LPCSTR>(&pbFile[0]));
                bOK = XgSetString(hwnd, str, json);
            } else {
                // ANSI
                std::wstring str = XgAnsiToUnicode(reinterpret_cast<LPCSTR>(&pbFile[0]));
                bOK = XgSetString(hwnd, str, json);
            }
        }

        if (bOK) {
            // �����B
            XgUpdateImage(hwnd, 0, 0);
            XgMarkUpdate();

            // �t�@�C���p�X���Z�b�g����B
            WCHAR szFileName[MAX_PATH];
            ::GetFullPathNameW(pszFile, MAX_PATH, szFileName, NULL);
            xg_strFileName = szFileName;
            return true;
        }
    } catch (...) {
        // ��O�����������B
    }

    // ���s�B
    return false;
}

// �t�@�C���iCRP�`���j��ۑ�����B
bool __fastcall XgDoSaveCrpFile(HWND /*hwnd*/, LPCWSTR pszFile)
{
    // �t�@�C�����쐬����B
    FILE *fout = _wfopen(pszFile, L"w");
    if (fout == NULL)
        return false;

    XG_Board *xw = (xg_bSolved ? &xg_solution : &xg_xword);

    try
    {
        fprintf(fout,
            "[Version]\n"
            "Ver=0.3.0\n"
            "\n"
            "[Puzzle]\n"
            "Puzzle=0\n"
            "\n"
            "[Cross]\n"
            "Width=%u\n"
            "Height=%u\n", xg_nCols, xg_nRows);

        // �}�X�B
        for (int i = 0; i < xg_nRows; ++i) {
            std::wstring row;
            for (int j = 0; j < xg_nCols; ++j) {
                if (row.size())
                    row += L",";
                WCHAR ch = xw->GetAt(i, j);
                if (ch == ZEN_SPACE)
                    row += 0xFF3F; // '�Q'
                else
                    row += ch;
            }
            fprintf(fout, "Line%u=%s\n", i + 1, XgUnicodeToAnsi(row).c_str());
        }

        // ��d�}�X�B
        std::vector<std::vector<INT> > marks;
        marks.resize(xg_nRows);
        for (auto& mark : marks) {
            mark.resize(xg_nCols);
        }

        std::wstring answer;
        if (xg_vMarks.size()) {
            for (size_t i = 0; i < xg_vMarks.size(); ++i) {
                answer += xw->GetAt(xg_vMarks[i].m_i, xg_vMarks[i].m_j);
                marks[xg_vMarks[i].m_i][xg_vMarks[i].m_j] = INT(i) + 1;
            }
        }

        for (int i = 0; i < xg_nRows; ++i) {
            std::string row;
            for (int j = 0; j < xg_nCols; ++j) {
                if (row.size())
                    row += ",";
                row += std::to_string(marks[i][j]);
            }
            fprintf(fout, "MarkUpLine%u=%s\n", i + 1, row.c_str());
        }
        fprintf(fout,
            "\n"
            "[Property]\n"
            "Symmetry=None\n"
            "\n"
            "[Clue]\n");

        // �q���g�B
        if (xg_vecTateHints.size() && xg_vecYokoHints.size()) {
            fprintf(fout, "Count=%u\n", int(xg_vecTateHints.size() + xg_vecYokoHints.size()));
            int iHint = 1;
            // �^�e�̃J�M�B
            for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
                auto& tate_hint = xg_vecTateHints[i];
                fprintf(fout, "Clue%u=%s:%s\n", iHint++,
                    XgUnicodeToAnsi(tate_hint.m_strWord).c_str(),
                    XgUnicodeToAnsi(tate_hint.m_strHint).c_str());
            }
            // ���R�̃J�M�B
            for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
                auto& yoko_hint = xg_vecYokoHints[i];
                fprintf(fout, "Clue%u=%s:%s\n", iHint++,
                    XgUnicodeToAnsi(yoko_hint.m_strWord).c_str(),
                    XgUnicodeToAnsi(yoko_hint.m_strHint).c_str());
            }
        }

        fprintf(fout,
            "\n"
            "[Numbering]\n"
            "Hint=%s\n"
            "Answer=%s\n"
            "Theme=%s\n"
            "CantUseChar=%s\n",
            "",
            XgUnicodeToAnsi(answer).c_str(),
            XgUnicodeToAnsi(xg_strTheme).c_str(),
            "");

        fclose(fout);

        // �t�@�C���p�X���Z�b�g����B
        WCHAR szFileName[MAX_PATH];
        ::GetFullPathNameW(pszFile, MAX_PATH, szFileName, NULL);
        xg_strFileName = szFileName;
        XgMarkUpdate();
        return true;
    }
    catch(...)
    {
        ;
    }

    // �������������߂Ȃ������B�s���ȃt�@�C���������B
    ::DeleteFileW(pszFile);
    return false;
}

// .xwj�t�@�C���iJSON�`���j��ۑ�����B
bool __fastcall XgDoSaveJson(HWND /*hwnd*/, LPCWSTR pszFile)
{
    HANDLE hFile;
    std::wstring str, strTable, strMarks, hints;
    DWORD size;

    // �t�@�C�����쐬����B
    hFile = ::CreateFileW(pszFile, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
        CREATE_ALWAYS, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    try
    {
        json j;
        // �쐬�ҏ��B
        j["creator_info"] = XgUnicodeToUtf8(XgLoadStringDx1(IDS_APPINFO));
        // �s�̐��B
        j["row_count"] = xg_nRows;
        // ��̐��B
        j["column_count"] = xg_nCols;
        // ���[���B
        j["rules"] = XgUnicodeToUtf8(XgGetRulesString(xg_nRules));
        // �������B
        j["dictionary"] = XgUnicodeToUtf8(PathFindFileNameW(xg_dict_name.c_str()));

        // �Ղ̐؂�ւ��B
        XG_Board *xw = (xg_bSolved ? &xg_solution : &xg_xword);
        j["is_solved"] = !!xg_bSolved;

        // �}�X�B
        for (int i = 0; i < xg_nRows; ++i) {
            std::wstring row;
            for (int j = 0; j < xg_nCols; ++j) {
                WCHAR ch = xw->GetAt(i, j);
                row += ch;
            }
            j["cell_data"].push_back(XgUnicodeToUtf8(row));
        }

        // ��d�}�X�B
        if (xg_vMarks.size()) {
            j["has_mark"] = true;

            std::wstring mark_word;
            for (size_t i = 0; i < xg_vMarks.size(); ++i) {
                WCHAR ch = xw->GetAt(xg_vMarks[i].m_i, xg_vMarks[i].m_j);
                mark_word += ch;
            }

            j["mark_word"] = XgUnicodeToUtf8(mark_word);

            str += L"\t\"marks\": [\r\n";
            for (size_t i = 0; i < xg_vMarks.size(); ++i) {
                json mark;
                mark.push_back(xg_vMarks[i].m_i + 1);
                mark.push_back(xg_vMarks[i].m_j + 1);
                WCHAR sz[2] = { mark_word[i] , 0 };
                mark.push_back(XgUnicodeToUtf8(sz));
                j["marks"].push_back(mark);
            }
        } else {
            j["has_mark"] = false;
        }

        // �q���g�B
        if (xg_vecTateHints.size() && xg_vecYokoHints.size()) {
            j["has_hints"] = true;

            json hints;

            // �^�e�̃J�M�B
            json v;
            for (size_t i = 0; i < xg_vecTateHints.size(); ++i) {
                json hint;
                auto& tate_hint = xg_vecTateHints[i];
                hint.push_back(tate_hint.m_number);
                hint.push_back(XgUnicodeToUtf8(tate_hint.m_strWord));
                hint.push_back(XgUnicodeToUtf8(tate_hint.m_strHint));
                v.push_back(hint);
            }
            hints["v"] = v;

            // ���R�̃J�M�B
            json h;
            for (size_t i = 0; i < xg_vecYokoHints.size(); ++i) {
                json hint;
                auto& yoko_hint = xg_vecYokoHints[i];
                hint.push_back(yoko_hint.m_number);
                hint.push_back(XgUnicodeToUtf8(yoko_hint.m_strWord));
                hint.push_back(XgUnicodeToUtf8(yoko_hint.m_strHint));
                h.push_back(hint);
            }
            hints["h"] = h;

            j["hints"] = hints;
        } else {
            j["has_hints"] = false;
        }

        // �w�b�_�[�B
        xg_str_trim(xg_strHeader);
        j["header"] = XgUnicodeToUtf8(xg_strHeader);

        // ���l���B
        xg_str_trim(xg_strNotes);
        LPCWSTR psz = XgLoadStringDx1(IDS_BELOWISNOTES);
        if (xg_strNotes.find(psz) == 0) {
            xg_strNotes = xg_strNotes.substr(std::wstring(psz).size());
        }
        j["notes"] = XgUnicodeToUtf8(xg_strNotes);

        // �e�[�}�B
        j["theme"] = XgUnicodeToUtf8(xg_strTheme);

        // UTF-8�֕ϊ�����B
        std::string utf8 = j.dump(1, '\t');
        utf8 += '\n';

        std::string replaced;
        for (auto ch : utf8)
        {
            if (ch == '\n')
                replaced += '\r';
            replaced += ch;
        }
        utf8 = std::move(replaced);

        // �t�@�C���ɏ�������ŁA�t�@�C�������B
        size = static_cast<DWORD>(utf8.size()) * sizeof(CHAR);
        if (::WriteFile(hFile, utf8.data(), size, &size, nullptr)) {
            ::CloseHandle(hFile);

            // �t�@�C���p�X���Z�b�g����B
            WCHAR szFileName[MAX_PATH];
            ::GetFullPathNameW(pszFile, MAX_PATH, szFileName, NULL);
            xg_strFileName = szFileName;
            XgMarkUpdate();
            return true;
        }
        ::CloseHandle(hFile);
    }
    catch(...)
    {
        ;
    }

    // �������������߂Ȃ������B�s���ȃt�@�C���������B
    ::DeleteFileW(pszFile);
    return false;
}

// .xwd/.xwj�t�@�C����ۑ�����B
bool __fastcall XgDoSaveStandard(HWND hwnd, LPCWSTR pszFile, const XG_Board& board)
{
    HANDLE hFile;
    std::wstring str, strTable, strMarks, hints;
    DWORD size;

    // �t�@�C�����쐬����B
    hFile = ::CreateFileW(pszFile, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
        CREATE_ALWAYS, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    // �}�[�N���擾����B
    if (!xg_vMarks.empty())
        XgGetStringOfMarks(strMarks);

    // �t�@�C���ɏ������ޕ���������߂�B
    xg_str_trim(xg_strHeader);

    if (xg_bSolved) {
        // �q���g����B
        board.GetString(strTable);
        board.GetHintsStr(hints, 2, true);
        str += xg_strHeader;        // �w�b�_�[������B
        str += xg_pszNewLine;       // ���s�B
        str += XgLoadStringDx1(IDS_HEADERSEP1); // �w�b�_�[�������B
        str += XgLoadStringDx1(IDS_APPINFO); // �A�v�����B
        str += xg_pszNewLine;       // ���s�B
        str += strMarks;            // �}�[�N�B
        str += strTable;            // �{�́B
        str += xg_pszNewLine;       // ���s�B
        str += hints;               // �q���g�B
    } else {
        // �q���g�Ȃ��B
        board.GetString(strTable);
        str += xg_strHeader;        // �w�b�_�[������B
        str += xg_pszNewLine;       // ���s�B
        str += XgLoadStringDx1(IDS_HEADERSEP1); // �w�b�_�[�������B
        str += XgLoadStringDx1(IDS_APPINFO); // �A�v�����B
        str += xg_pszNewLine;       // ���s�B
        str += strMarks;            // �}�[�N�B
        str += strTable;            // �{�́B
    }
    str += XgLoadStringDx1(IDS_HEADERSEP2);     // �t�b�^�[�������B

    // ���l���B
    LPCWSTR psz = XgLoadStringDx1(IDS_BELOWISNOTES);
    str += psz;
    str += xg_pszNewLine;
    if (xg_strNotes.find(psz) == 0) {
        xg_strNotes = xg_strNotes.substr(std::wstring(psz).size());
    }
    xg_str_trim(xg_strNotes);
    str += xg_strNotes;
    str += xg_pszNewLine;

    // �t�@�C���ɏ�������ŁA�t�@�C�������B
    size = 2;
    if (::WriteFile(hFile, "\xFF\xFE", size, &size, nullptr)) {
        size = static_cast<DWORD>(str.size()) * sizeof(WCHAR);
        if (::WriteFile(hFile, str.data(), size, &size, nullptr)) {
            ::CloseHandle(hFile);
            return true;
        }
    }
    ::CloseHandle(hFile);

    // �������������߂Ȃ������B�s���ȃt�@�C���������B
    ::DeleteFileW(pszFile);
    return false;
}

bool __fastcall XgDoSave(HWND hwnd, LPCWSTR pszFile)
{
    bool ret;
    LPCWSTR pchDotExt = PathFindExtensionW(pszFile);
    if (lstrcmpiW(pchDotExt, L".xwj") == 0 ||
        lstrcmpiW(pchDotExt, L".json") == 0 ||
        lstrcmpiW(pchDotExt, L".jso") == 0)
    {
        // JSON�`���ŕۑ��B
        ret = XgDoSaveJson(hwnd, pszFile);
    } else if (lstrcmpiW(pchDotExt, L".crp") == 0) {
        // Crossword Builder (*.crp) �`���ŕۑ��B
        ret = XgDoSaveCrpFile(hwnd, pszFile);
    } else if (xg_bSolved) {
        // �q���g����B
        ret = XgDoSaveStandard(hwnd, pszFile, xg_solution);
    } else {
        ret = XgDoSaveStandard(hwnd, pszFile, xg_xword);
    }
    if (ret) {
        // �t�@�C���p�X���Z�b�g����B
        WCHAR szFileName[MAX_PATH];
        ::GetFullPathNameW(pszFile, MAX_PATH, szFileName, NULL);
        xg_strFileName = szFileName;
        XgMarkUpdate();
    }
    return ret;
}

// BITMAPINFOEX�\���́B
typedef struct tagBITMAPINFOEX
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[256];
} BITMAPINFOEX, FAR * LPBITMAPINFOEX;

// �r�b�g�}�b�v���t�@�C���ɕۑ�����B
bool __fastcall XgSaveBitmapToFile(LPCWSTR pszFileName, HBITMAP hbm)
{
    bool f;
    BITMAPFILEHEADER bf;
    BITMAPINFOEX bi;
    BITMAPINFOHEADER *pbmih;
    DWORD cb;
    DWORD cColors, cbColors;
    HDC hDC;
    HANDLE hFile;
    LPVOID pBits;
    BITMAP bm;
    DWORD dwError = 0;

    // �r�b�g�}�b�v�̏����擾����B
    if (!::GetObject(hbm, sizeof(BITMAP), &bm))
        return false;

    // BITMAPINFO�\���̂�ݒ肷��B
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

    // BITMAPFILEHEADER�\���̂�ݒ肷��B
    bf.bfType = 0x4d42;
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    cb = sizeof(BITMAPFILEHEADER) + pbmih->biSize + cbColors;
    bf.bfOffBits = cb;
    bf.bfSize = cb + pbmih->biSizeImage;

    // �r�b�g�i�[�p�̃��������m�ۂ���B
    pBits = ::HeapAlloc(::GetProcessHeap(), 0, pbmih->biSizeImage);
    if (pBits == nullptr)
        return false;

    // DC���擾����B
    f = false;
    hDC = ::GetDC(nullptr);
    if (hDC != nullptr) {
        // �r�b�g���擾����B
        if (::GetDIBits(hDC, hbm, 0, bm.bmHeight, pBits,
                      reinterpret_cast<BITMAPINFO*>(&bi),
                      DIB_RGB_COLORS))
        {
            // �t�@�C�����쐬����B
            hFile = ::CreateFileW(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL |
                                FILE_FLAG_WRITE_THROUGH, nullptr);
            if (hFile != INVALID_HANDLE_VALUE) {
                // �t�@�C���ɏ������ށB
                f = ::WriteFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, nullptr) &&
                    ::WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &cb, nullptr) &&
                    ::WriteFile(hFile, bi.bmiColors, cbColors, &cb, nullptr) &&
                    ::WriteFile(hFile, pBits, pbmih->biSizeImage, &cb, nullptr);
                if (!f)
                    dwError = ::GetLastError();
                // �t�@�C�������B
                ::CloseHandle(hFile);

                if (!f)
                    ::DeleteFileW(pszFileName);
            } else {
                dwError = ::GetLastError();
            }
        } else {
            dwError = ::GetLastError();
        }

        // DC���������B
        ::ReleaseDC(nullptr, hDC);
    } else {
        dwError = ::GetLastError();
    }

    // �m�ۂ������������������B
    ::HeapFree(::GetProcessHeap(), 0, pBits);
    // �G���[�R�[�h��ݒ肷��B
    ::SetLastError(dwError);
    return f;
}

#ifndef CDSIZEOF_STRUCT
    #define CDSIZEOF_STRUCT(structname,member) \
        (((INT_PTR)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif
#ifndef OPENFILENAME_SIZE_VERSION_400W
    #define OPENFILENAME_SIZE_VERSION_400W CDSIZEOF_STRUCT(OPENFILENAMEW,lpTemplateName)
#endif

// �����摜�t�@�C���Ƃ��ĕۑ�����B
void __fastcall XgSaveProbAsImage(HWND hwnd)
{
    OPENFILENAMEW ofn;
    WCHAR szFileName[MAX_PATH] = L"";

    // �u�����摜�t�@�C���Ƃ��ĕۑ��v�_�C�A���O��\���B
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = XgMakeFilterString(XgLoadStringDx2(IDS_IMGFILTER));
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = ARRAYSIZE(szFileName);
    ofn.lpstrTitle = XgLoadStringDx1(IDS_SAVEPROBASIMG);
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"bmp";
    if (::GetSaveFileNameW(&ofn)) {
        // �`��T�C�Y���擾����B
        SIZE siz;
        XgGetXWordExtent(&siz);

        if (ofn.nFilterIndex <= 1) {
            // �r�b�g�}�b�v��ۑ�����B
            HBITMAP hbm = XgCreateXWordImage(xg_xword, &siz, false);
            if (hbm != nullptr) {
                if (!XgSaveBitmapToFile(szFileName, hbm))
                {
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTSAVE2), nullptr, MB_ICONERROR);
                }
                ::DeleteObject(hbm);
            } else {
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTSAVE2), nullptr, MB_ICONERROR);
            }
        } else {
            // EMF��ۑ�����B
            HDC hdcRef = ::GetDC(hwnd);
            HDC hdc = ::CreateEnhMetaFileW(hdcRef, szFileName, nullptr, XgLoadStringDx1(IDS_APPNAME));
            if (hdc) {
                XgDrawXWord(xg_xword, hdc, &siz, false);
                ::CloseEnhMetaFile(hdc);
            } else {
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTSAVE2), nullptr, MB_ICONERROR);
            }
            ::ReleaseDC(hwnd, hdcRef);
        }
    }
}

// �𓚂��摜�t�@�C���Ƃ��ĕۑ�����B
void __fastcall XgSaveAnsAsImage(HWND hwnd)
{
    OPENFILENAMEW ofn;
    WCHAR szFileName[MAX_PATH] = L"";

    if (!xg_bSolved) {
        ::MessageBeep(0xFFFFFFFF);
        return;
    }

    // �u�𓚂��摜�t�@�C���Ƃ��ĕۑ��v�_�C�A���O��\���B
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = XgMakeFilterString(XgLoadStringDx2(IDS_IMGFILTER));
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = ARRAYSIZE(szFileName);
    ofn.lpstrTitle = XgLoadStringDx1(IDS_SAVEANSASIMG);
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"bmp";
    if (::GetSaveFileNameW(&ofn)) {
        // �`��T�C�Y���擾����B
        SIZE siz;
        XgGetXWordExtent(&siz);

        if (ofn.nFilterIndex <= 1) {
            // �r�b�g�}�b�v��ۑ�����B
            HBITMAP hbm = XgCreateXWordImage(xg_solution, &siz, false);
            if (hbm != nullptr) {
                if (!XgSaveBitmapToFile(szFileName, hbm)) {
                    XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTSAVE2), nullptr, MB_ICONERROR);
                }
                ::DeleteObject(hbm);
            } else {
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTSAVE2), nullptr, MB_ICONERROR);
            }
        } else {
            // EMF��ۑ�����B
            HDC hdcRef = ::GetDC(hwnd);
            HDC hdc = ::CreateEnhMetaFileW(hdcRef, szFileName, nullptr, XgLoadStringDx1(IDS_APPNAME));
            if (hdc) {
                XgDrawXWord(xg_solution, hdc, &siz, false);
                ::CloseEnhMetaFile(hdc);
            } else {
                XgCenterMessageBoxW(hwnd, XgLoadStringDx1(IDS_CANTSAVE2), nullptr, MB_ICONERROR);
            }
            ::ReleaseDC(hwnd, hdcRef);
        }
    }
}

// �N���X���[�h�̕�������擾����B
void __fastcall XG_Board::GetString(std::wstring& str) const
{
    str.clear();

    str += ZEN_ULEFT;
    for (int j = 0; j < xg_nCols; j++)
        str += ZEN_HLINE;
    str += ZEN_URIGHT;
    str += L"\r\n";

    for (int i = 0; i < xg_nRows; i++) {
        str += ZEN_VLINE;
        for (int j = 0; j < xg_nCols; j++)
        {
            str += GetAt(i, j);
        }
        str += ZEN_VLINE;
        str += L"\r\n";
    }

    str += ZEN_LLEFT;
    for (int j = 0; j < xg_nCols; j++)
        str += ZEN_HLINE;
    str += ZEN_LRIGHT;
    str += L"\r\n";
}

// �N���X���[�h�ɕ������ݒ肷��B
bool __fastcall XG_Board::SetString(const std::wstring& strToBeSet)
{
    int i, nRows, nCols;
    std::vector<WCHAR> v;
    std::wstring str(strToBeSet);

    // �w�b�_�[���擾����B
    xg_strHeader.clear();
    std::wstring strHeaderSep = XgLoadStringDx1(IDS_HEADERSEP1);
    size_t i0 = str.find(strHeaderSep);
    if (i0 != std::wstring::npos) {
        xg_strHeader = str.substr(0, i0);
        str = str.substr(i0 + strHeaderSep.size());
    }
    xg_str_trim(xg_strHeader);

    // ����������B
    const int size = static_cast<int>(str.size());
    nRows = nCols = 0;

    // ����̊p�����邩�H
    for (i = 0; i < size; i++) {
        if (str[i] == ZEN_ULEFT)
            break;
    }
    if (i == size) {
        // ������Ȃ������B���s�B
        return false;
    }

    // �������ǂݍ��ށB
    bool bFoundLastCorner = false;
    for (; i < size; i++) {
        if (str[i] == ZEN_VLINE) {
            // ���̋��E�������������B
            i++;    // ���̕����ցB

            // �����ȊO�̕������i�[����B
            while (i < size && str[i] != ZEN_VLINE) {
                v.emplace_back(str[i]);
                i++;    // ���̕����ցB
            }

            // ������̒����𒴂�����A���f����B
            if (i >= size)
                break;

            // �E�̋��E�������������B�񐔂����i�[�Ȃ�A�i�[����B
            if (nCols == 0)
                nCols = static_cast<int>(v.size());

            nRows++;    // ���̍s�ցB
            i++;    // ���̕����ցB
        } else if (str[i] == ZEN_LRIGHT) {
            // �E���̊p�����������B
            bFoundLastCorner = true;
            break;
        }
        // ���̑��̕����͓ǂݎ̂āB
    }

    // ������Ɠǂݍ��߂����m�F����B
    if (nRows == 0 || nCols == 0 || !bFoundLastCorner) {
        // ���s�B
        return false;
    }

    // �N���X���[�h������������B
    xg_bSolved = false;
    xg_bShowAnswer = false;
    ResetAndSetSize(nRows, nCols);
    xg_nRows = nRows;
    xg_nCols = nCols;

    // �󔒂���Ȃ��}�X�̌��𐔂���B
    WCHAR ch = 0;
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            if (v[i * xg_nCols + j] != ZEN_SPACE)
                ch++;
        }
    }
    v.emplace_back(ch);

    // �}�X�����i�[����B
    m_vCells = v;

    // �}�X�̕����̎�ނɉ����ē��̓��[�h��؂�ւ���B
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            ch = GetAt(i, j);
            if (XgIsCharHankakuAlphaW(ch) || XgIsCharZenkakuAlphaW(ch)) {
                xg_imode = xg_im_ABC;
                goto break2;
            }
            if (XgIsCharKanaW(ch)) {
                xg_imode = xg_im_KANA;
                goto break2;
            }
            if (XgIsCharKanjiW(ch)) {
                xg_imode = xg_im_KANJI;
                goto break2;
            }
            if (XgIsCharZenkakuCyrillicW(ch)) {
                xg_imode = xg_im_RUSSIA;
                goto break2;
            }
            if (XgIsCharZenkakuNumericW(ch) || XgIsCharHankakuNumericW(ch)) {
                xg_imode = xg_im_DIGITS;
                goto break2;
            }
        }
    }
break2:;

    // �J�M���N���A����B
    xg_vTateInfo.clear();
    xg_vYokoInfo.clear();

    // �����B
    return true;
}

//////////////////////////////////////////////////////////////////////////////

// ���}�X�����Ώ̂��H
bool XG_Board::IsLineSymmetry() const
{
    const int nRows = xg_nRows;
    const int nCols = xg_nCols;

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            if ((GetAt(i, j) == ZEN_BLACK) !=
                (GetAt(nRows - i - 1, j) == ZEN_BLACK))
            {
                goto skip01;
            }
        }
    }
    return true;
skip01:;

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            if ((GetAt(i, j) == ZEN_BLACK) !=
                (GetAt(i, xg_nCols - j - 1) == ZEN_BLACK))
            {
                goto skip02;
            }
        }
    }
    return true;
skip02:;

    if (nRows != nCols)
        return false;

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            if ((GetAt(i, j) == ZEN_BLACK) !=
                (GetAt(j, i) == ZEN_BLACK))
            {
                goto skip03;
            }
        }
    }
    return true;
skip03:;

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            if ((GetAt(i, j) == ZEN_BLACK) !=
                (GetAt(nRows - j - 1, nCols - i - 1) == ZEN_BLACK))
            {
                goto skip04;
            }
        }
    }
    return true;
skip04:;

    return false;
}

// ���}�X���_�Ώ̂��H
bool XG_Board::IsPointSymmetry() const
{
    const int nRows = xg_nRows;
    const int nCols = xg_nCols;
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            if ((GetAt(i, j) == ZEN_BLACK) !=
                (GetAt(nRows - (i + 1), nCols - (j + 1)) == ZEN_BLACK))
            {
                return false;
            }
        }
    }
    return true;
}

// ���}�X�����Ώ́i�^�e�j���H
bool XG_Board::IsLineSymmetryV() const
{
    const int nRows = xg_nRows;
    const int nHalfRows = nRows / 2;
    const int nCols = xg_nCols;
    for (int i = 0; i < nHalfRows; i++) {
        for (int j = 0; j < nCols; j++) {
            if ((GetAt(i, j) == ZEN_BLACK) != (GetAt(nRows - (i + 1), j) == ZEN_BLACK))
                return false;
        }
    }
    return true;
}

// ���}�X�����Ώ́i���R�j���H
bool XG_Board::IsLineSymmetryH() const
{
    const int nRows = xg_nRows;
    const int nCols = xg_nCols;
    const int nHalfCols = nCols / 2;
    for (int j = 0; j < nHalfCols; j++) {
        for (int i = 0; i < nRows; i++) {
            if ((GetAt(i, j) == ZEN_BLACK) != (GetAt(i, nCols - (j + 1)) == ZEN_BLACK))
                return false;
        }
    }
    return true;
}

// ���ΎO�A���H
bool XG_Board::ThreeDiagonals() const
{
    const int nRows = xg_nRows;
    const int nCols = xg_nCols;
    for (int i = 0; i < nRows - 2; i++) {
        for (int j = 0; j < nCols - 2; j++) {
            if (GetAt(i, j) != ZEN_BLACK)
                continue;
            if (GetAt(i + 1, j + 1) != ZEN_BLACK)
                continue;
            if (GetAt(i + 2, j + 2) != ZEN_BLACK)
                continue;
            return true;
        }
    }
    for (int i = 0; i < nRows - 2; i++) {
        for (int j = 2; j < nCols; j++) {
            if (GetAt(i, j) != ZEN_BLACK)
                continue;
            if (GetAt(i + 1, j - 1) != ZEN_BLACK)
                continue;
            if (GetAt(i + 2, j - 2) != ZEN_BLACK)
                continue;
            return true;
        }
    }
    return false;
}

// ���Ύl�A���H
bool XG_Board::FourDiagonals() const
{
    const int nRows = xg_nRows;
    const int nCols = xg_nCols;
    for (int i = 0; i < nRows - 3; i++) {
        for (int j = 0; j < nCols - 3; j++) {
            if (GetAt(i, j) != ZEN_BLACK)
                continue;
            if (GetAt(i + 1, j + 1) != ZEN_BLACK)
                continue;
            if (GetAt(i + 2, j + 2) != ZEN_BLACK)
                continue;
            if (GetAt(i + 3, j + 3) != ZEN_BLACK)
                continue;
            return true;
        }
    }
    for (int i = 0; i < nRows - 3; i++) {
        for (int j = 3; j < nCols; j++) {
            if (GetAt(i, j) != ZEN_BLACK)
                continue;
            if (GetAt(i + 1, j - 1) != ZEN_BLACK)
                continue;
            if (GetAt(i + 2, j - 2) != ZEN_BLACK)
                continue;
            if (GetAt(i + 3, j - 3) != ZEN_BLACK)
                continue;
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// ���}�X�p�^�[���𐶐�����B

// ���}�X�p�^�[�����������ꂽ���H
bool xg_bBlacksGenerated = false;
// �z�u�ł���ő�P�꒷�B
INT xg_nMaxWordLen = 4;

// ���}�X�p�^�[���𐶐�����B
bool __fastcall XgGenerateBlacksRecurse(const XG_Board& xword, LONG iRowjCol)
{
    // ���[���̓K�������`�F�b�N����B
    if ((xg_nRules & RULE_DONTCORNERBLACK) && xword.CornerBlack())
        return false;
    if ((xg_nRules & RULE_DONTDOUBLEBLACK) && xword.DoubleBlack())
        return false;
    if ((xg_nRules & RULE_DONTTRIDIRECTIONS) && xword.TriBlackAround())
        return false;
    if ((xg_nRules & RULE_DONTDIVIDE) && xword.DividedByBlack())
        return false;
    if (xg_nRules & RULE_DONTTHREEDIAGONALS) {
        if (xword.ThreeDiagonals())
            return false;
    } else if (xg_nRules & RULE_DONTFOURDIAGONALS) {
        if (xword.FourDiagonals())
            return false;
    }

    const int nRows = xg_nRows, nCols = xg_nCols;
    INT iRow = LOWORD(iRowjCol), jCol = HIWORD(iRowjCol);
    // �I�������B
    if (iRow == nRows && jCol == 0) {
        EnterCriticalSection(&xg_cs);
        if (!xg_bBlacksGenerated) {
            xg_bBlacksGenerated = true;
            xg_xword = xword;
        }
        ::LeaveCriticalSection(&xg_cs);
        return xg_bBlacksGenerated || xg_bCancelled;
    }

    // �I�������B
    if (xg_bBlacksGenerated || xg_bCancelled)
        return true;

    // �}�X�̍���������B
    INT jLeft;
    for (jLeft = jCol; jLeft > 0; --jLeft) {
        if (xword.GetAt(iRow, jLeft - 1) == ZEN_BLACK) {
            break;
        }
    }
    if (jCol - jLeft + 1 > xg_nMaxWordLen) {
        // �󔒂��ő咷���������B���}�X���Z�b�g���čċA�B
        XG_Board copy(xword);
        copy.SetAt(iRow, jCol, ZEN_BLACK);
        if (jCol + 1 < nCols) {
            if (XgGenerateBlacksRecurse(copy, MAKELONG(iRow, jCol + 1)))
                return true;
        } else {
            if (XgGenerateBlacksRecurse(copy, MAKELONG(iRow + 1, 0)))
                return true;
        }
        return false;
    }
    // �}�X�̏㑤������B
    INT iTop;
    for (iTop = iRow; iTop > 0; --iTop) {
        if (xword.GetAt(iTop - 1, jCol) == ZEN_BLACK) {
            break;
        }
    }
    if (iRow - iTop + 1 > xg_nMaxWordLen) {
        // �󔒂��ő咷���������B���}�X���Z�b�g���čċA�B
        XG_Board copy(xword);
        copy.SetAt(iRow, jCol, ZEN_BLACK);
        if (jCol + 1 < nCols) {
            if (XgGenerateBlacksRecurse(copy, MAKELONG(iRow, jCol + 1)))
                return true;
        } else {
            if (XgGenerateBlacksRecurse(copy, MAKELONG(iRow + 1, 0)))
                return true;
        }
        return false;
    }

    if (rand() < RAND_MAX / 2) { // 1/2�̊m���ŁB�B�B
        // ���}�X���Z�b�g���čċA�B
        if (jCol + 1 < nCols) {
            if (XgGenerateBlacksRecurse(xword, MAKELONG(iRow, jCol + 1)))
                return true;
            XG_Board copy(xword);
            copy.SetAt(iRow, jCol, ZEN_BLACK);
            if (XgGenerateBlacksRecurse(copy, MAKELONG(iRow, jCol + 1)))
                return true;
        } else {
            if (XgGenerateBlacksRecurse(xword, MAKELONG(iRow + 1, 0)))
                return true;
            XG_Board copy(xword);
            copy.SetAt(iRow, jCol, ZEN_BLACK);
            if (XgGenerateBlacksRecurse(copy, MAKELONG(iRow + 1, 0)))
                return true;
        }
    } else {
        // ���}�X���Z�b�g���čċA�B
        XG_Board copy(xword);
        copy.SetAt(iRow, jCol, ZEN_BLACK);
        if (jCol + 1 < nCols) {
            if (XgGenerateBlacksRecurse(copy, MAKELONG(iRow, jCol + 1)))
                return true;
            if (XgGenerateBlacksRecurse(xword, MAKELONG(iRow, jCol + 1)))
                return true;
        } else {
            if (XgGenerateBlacksRecurse(copy, MAKELONG(iRow + 1, 0)))
                return true;
            if (XgGenerateBlacksRecurse(xword, MAKELONG(iRow + 1, 0)))
                return true;
        }
    }
    return false;
}

// ���}�X�p�^�[���𐶐�����i�_�Ώ́j�B
bool __fastcall XgGenerateBlacksPointSymRecurse(const XG_Board& xword, LONG iRowjCol)
{
    // ���[���̓K�������`�F�b�N����B
    if ((xg_nRules & RULE_DONTCORNERBLACK) && xword.CornerBlack())
        return false;
    if ((xg_nRules & RULE_DONTDOUBLEBLACK) && xword.DoubleBlack())
        return false;
    if ((xg_nRules & RULE_DONTTRIDIRECTIONS) && xword.TriBlackAround())
        return false;
    if ((xg_nRules & RULE_DONTDIVIDE) && xword.DividedByBlack())
        return false;
    if (xg_nRules & RULE_DONTTHREEDIAGONALS)
    {
        if (xword.ThreeDiagonals())
            return false;
    }
    else if (xg_nRules & RULE_DONTFOURDIAGONALS)
    {
        if (xword.FourDiagonals())
            return false;
    }

    const int nRows = xg_nRows, nCols = xg_nCols;
    INT iRow = LOWORD(iRowjCol), jCol = HIWORD(iRowjCol);

    // �I�������B
    if (iRow == nRows && jCol == 0) {
        EnterCriticalSection(&xg_cs);
        if (!xg_bBlacksGenerated) {
            xg_bBlacksGenerated = true;
            xg_xword = xword;
        }
        ::LeaveCriticalSection(&xg_cs);
        return xg_bBlacksGenerated || xg_bCancelled;
    }

    // �I�������B
    if (xg_bBlacksGenerated || xg_bCancelled)
        return true;

    // �}�X�̍���������B
    INT jLeft;
    for (jLeft = jCol; jLeft > 0; --jLeft) {
        if (xword.GetAt(iRow, jLeft - 1) == ZEN_BLACK) {
            break;
        }
    }
    if (jCol - jLeft + 1 > xg_nMaxWordLen) {
        // �󔒂��ő咷���������B���}�X���Z�b�g���čċA�B
        XG_Board copy(xword);
        copy.SetAt(iRow, jCol, ZEN_BLACK);
        copy.SetAt(nRows - (iRow + 1), nCols - (jCol + 1), ZEN_BLACK);
        if (jCol + 1 < nCols) {
            if (XgGenerateBlacksPointSymRecurse(copy, MAKELONG(iRow, jCol + 1)))
                return true;
        } else {
            if (XgGenerateBlacksPointSymRecurse(copy, MAKELONG(iRow + 1, 0)))
                return true;
        }
        return false;
    }
    // �}�X�̏㑤������B
    INT iTop;
    for (iTop = iRow; iTop > 0; --iTop) {
        if (xword.GetAt(iTop - 1, jCol) == ZEN_BLACK) {
            break;
        }
    }
    if (iRow - iTop + 1 > xg_nMaxWordLen) {
        // �󔒂��ő咷���������B���}�X���Z�b�g���čċA�B
        XG_Board copy(xword);
        copy.SetAt(iRow, jCol, ZEN_BLACK);
        copy.SetAt(nRows - (iRow + 1), nCols - (jCol + 1), ZEN_BLACK);
        if (jCol + 1 < nCols) {
            if (XgGenerateBlacksPointSymRecurse(copy, MAKELONG(iRow, jCol + 1)))
                return true;
        } else {
            if (XgGenerateBlacksPointSymRecurse(copy, MAKELONG(iRow + 1, 0)))
                return true;
        }
        return false;
    }

    if (rand() < RAND_MAX / 2) { // 1/2�̊m���ŁB�B�B
        // ���}�X���Z�b�g���čċA�B
        if (jCol + 1 < nCols) {
            if (XgGenerateBlacksPointSymRecurse(xword, MAKELONG(iRow, jCol + 1)))
                return true;
            XG_Board copy(xword);
            copy.SetAt(iRow, jCol, ZEN_BLACK);
            copy.SetAt(nRows - (iRow + 1), nCols - (jCol + 1), ZEN_BLACK);
            if (XgGenerateBlacksPointSymRecurse(copy, MAKELONG(iRow, jCol + 1)))
                return true;
        } else {
            if (XgGenerateBlacksPointSymRecurse(xword, MAKELONG(iRow + 1, 0)))
                return true;
            XG_Board copy(xword);
            copy.SetAt(iRow, jCol, ZEN_BLACK);
            copy.SetAt(nRows - (iRow + 1), nCols - (jCol + 1), ZEN_BLACK);
            if (XgGenerateBlacksPointSymRecurse(copy, MAKELONG(iRow + 1, 0)))
                return true;
        }
    } else {
        // ���}�X���Z�b�g���čċA�B
        XG_Board copy(xword);
        copy.SetAt(iRow, jCol, ZEN_BLACK);
        copy.SetAt(nRows - (iRow + 1), nCols - (jCol + 1), ZEN_BLACK);
        if (jCol + 1 < nCols) {
            if (XgGenerateBlacksPointSymRecurse(copy, MAKELONG(iRow, jCol + 1)))
                return true;
            if (XgGenerateBlacksPointSymRecurse(xword, MAKELONG(iRow, jCol + 1)))
                return true;
        } else {
            if (XgGenerateBlacksPointSymRecurse(copy, MAKELONG(iRow + 1, 0)))
                return true;
            if (XgGenerateBlacksPointSymRecurse(xword, MAKELONG(iRow + 1, 0)))
                return true;
        }
    }
    return false;
}

// ���}�X�p�^�[���𐶐�����i�^�e���Ώ́j�B
bool __fastcall XgGenerateBlacksLineSymVRecurse(const XG_Board& xword, LONG iRowjCol)
{
    // TODO: XgGenerateBlacksPointSymRecurse�ċA�֐����Q�l�ɂ��̍ċA�֐����������ĉ������B
    // XG_Board�N���X�̐錾�ƒ�`���悭�ǂނ��ƁB
    // �p�^�[��������������A������O���[�o���ϐ�xg_xword�ɏ�������ŉ������B
    // xg_xword�ɏ������ޑO�ɃN���e�B�J���Z�N�V����xg_cs���g�p���Ĕr�����䂵�ĉ������B
    // xg_nRows�͔Ղ̍s���Axg_nCols�͔Ղ̗񐔂ł��B
    // ����iRowjCol�͎��R�ɂ��g���������B�������x�D��B
    assert(0);
    return false;
}

// ���}�X�p�^�[���𐶐�����i���R���Ώ́j�B
bool __fastcall XgGenerateBlacksLineSymHRecurse(const XG_Board& xword, LONG iRowjCol)
{
    // TODO: XgGenerateBlacksPointSymRecurse�ċA�֐����Q�l�ɂ��̍ċA�֐����������ĉ������B
    // XG_Board�N���X�̐錾�ƒ�`���悭�ǂނ��ƁB
    // �p�^�[��������������A������O���[�o���ϐ�xg_xword�ɏ�������ŉ������B
    // xg_xword�ɏ������ޑO�ɃN���e�B�J���Z�N�V����xg_cs���g�p���Ĕr�����䂵�ĉ������B
    // xg_nRows�͔Ղ̍s���Axg_nCols�͔Ղ̗񐔂ł��B
    // ����iRowjCol�͎��R�ɂ��g���������B�������x�D��B
    assert(0);
    return false;
}

// �}���`�X���b�h�p�̊֐��B
unsigned __stdcall XgGenerateBlacks(void *param)
{
    XG_Board xword;
    xg_solution.clear();

    // ����������������B
    srand(::GetTickCount() ^ ::GetCurrentThreadId());

    // �ċA�����֐��ɓ˓�����B
    do {
        if (xg_bBlacksGenerated || xg_bCancelled)
            break;
        xword.clear();
    } while (!XgGenerateBlacksRecurse(xword, 0));
    return 1;
}

// �}���`�X���b�h�p�̊֐��B
unsigned __stdcall XgGenerateBlacksSmart(void *param)
{
    if (xg_bBlacksGenerated)
        return 1;

    XG_Board xword;

    // ����������������B
    srand(::GetTickCount() ^ ::GetCurrentThreadId());

    if (xg_nRules & RULE_POINTSYMMETRY) {
        // �ċA�����֐��ɓ˓�����B
        do {
            if (xg_bCancelled)
                break;
            if (xg_bBlacksGenerated) {
                break;
            }
            xword.clear();
        } while (!XgGenerateBlacksPointSymRecurse(xword, 0));
    } else if (xg_nRules & RULE_LINESYMMETRYV) {
        // �ċA�����֐��ɓ˓�����B
        do {
            if (xg_bCancelled)
                break;
            if (xg_bBlacksGenerated) {
                break;
            }
            xword.clear();
        } while (!XgGenerateBlacksLineSymVRecurse(xword, 0));
    } else if (xg_nRules & RULE_LINESYMMETRYH) {
        // �ċA�����֐��ɓ˓�����B
        do {
            if (xg_bCancelled)
                break;
            if (xg_bBlacksGenerated) {
                break;
            }
            xword.clear();
        } while (!XgGenerateBlacksLineSymHRecurse(xword, 0));
    } else {
        // �ċA�����֐��ɓ˓�����B
        do {
            if (xg_bCancelled)
                break;
            if (xg_bBlacksGenerated) {
                break;
            }
            xword.clear();
        } while (!XgGenerateBlacksRecurse(xword, 0));
    }
    return 1;
}

// �}���`�X���b�h�p�̊֐��B
unsigned __stdcall XgGenerateBlacksPointSym(void *param)
{
    srand(::GetTickCount() ^ ::GetCurrentThreadId());
    xg_solution.clear();
    XG_Board xword;
    do {
        if (xg_bBlacksGenerated || xg_bCancelled)
            break;
        xword.clear();
    } while (!XgGenerateBlacksPointSymRecurse(xword, 0));
    return 1;
}

// �}���`�X���b�h�p�̊֐��B
unsigned __stdcall XgGenerateBlacksLineSymV(void *param)
{
    srand(::GetTickCount() ^ ::GetCurrentThreadId());
    xg_solution.clear();
    XG_Board xword;
    do {
        if (xg_bBlacksGenerated || xg_bCancelled)
            break;
        xword.clear();
    } while (!XgGenerateBlacksLineSymVRecurse(xword, 0));
    return 1;
}

// �}���`�X���b�h�p�̊֐��B
unsigned __stdcall XgGenerateBlacksLineSymH(void *param)
{
    srand(::GetTickCount() ^ ::GetCurrentThreadId());
    xg_solution.clear();
    XG_Board xword;
    do {
        if (xg_bBlacksGenerated || xg_bCancelled)
            break;
        xword.clear();
    } while (!XgGenerateBlacksLineSymHRecurse(xword, 0));
    return 1;
}

void __fastcall XgStartGenerateBlacks(void)
{
    xg_bBlacksGenerated = false;
    xg_bCancelled = false;

    // �X���b�h���J�n����B
    if (xg_nRules & RULE_POINTSYMMETRY) {
#ifdef SINGLE_THREAD_MODE
        XgGenerateBlacksPointSym(&xg_aThreadInfo[0]);
#else
        for (DWORD i = 0; i < xg_dwThreadCount; i++) {
            xg_ahThreads[i] = reinterpret_cast<HANDLE>(
                _beginthreadex(nullptr, 0, XgGenerateBlacksPointSym, &xg_aThreadInfo[i], 0,
                    &xg_aThreadInfo[i].m_threadid));
            assert(xg_ahThreads[i] != nullptr);
        }
#endif
    } else if (xg_nRules & RULE_LINESYMMETRYV) {
#ifdef SINGLE_THREAD_MODE
        XgGenerateBlacksLineSymV(&xg_aThreadInfo[0]);
#else
        for (DWORD i = 0; i < xg_dwThreadCount; i++) {
            xg_ahThreads[i] = reinterpret_cast<HANDLE>(
                _beginthreadex(nullptr, 0, XgGenerateBlacksLineSymV, &xg_aThreadInfo[i], 0,
                    &xg_aThreadInfo[i].m_threadid));
            assert(xg_ahThreads[i] != nullptr);
        }
#endif
    } else if (xg_nRules & RULE_LINESYMMETRYV) {
#ifdef SINGLE_THREAD_MODE
        XgGenerateBlacksLineSymH(&xg_aThreadInfo[0]);
#else
        for (DWORD i = 0; i < xg_dwThreadCount; i++) {
            xg_ahThreads[i] = reinterpret_cast<HANDLE>(
                _beginthreadex(nullptr, 0, XgGenerateBlacksLineSymH, &xg_aThreadInfo[i], 0,
                    &xg_aThreadInfo[i].m_threadid));
            assert(xg_ahThreads[i] != nullptr);
        }
#endif
    } else {
#ifdef SINGLE_THREAD_MODE
        XgGenerateBlacks(&xg_aThreadInfo[0]);
#else
        for (DWORD i = 0; i < xg_dwThreadCount; i++) {
            xg_ahThreads[i] = reinterpret_cast<HANDLE>(
                _beginthreadex(nullptr, 0, XgGenerateBlacks, &xg_aThreadInfo[i], 0,
                    &xg_aThreadInfo[i].m_threadid));
            assert(xg_ahThreads[i] != nullptr);
        }
#endif
    }
}

// �N���X���[�h�Ŏg�������ɕϊ�����B
std::wstring __fastcall XgNormalizeString(const std::wstring& text) {
    WCHAR szText[512];
    LCMapStringW(JPN_LOCALE, LCMAP_FULLWIDTH | LCMAP_KATAKANA | LCMAP_UPPERCASE,
        text.c_str(), -1, szText, 512);
    std::wstring ret = szText;
    for (auto& ch : ret) {
        // �����Ȏ���傫�Ȏ��ɂ���B
        for (size_t i = 0; i < ARRAYSIZE(xg_small); i++) {
            if (ch == xg_small[i][0]) {
                ch = xg_large[i][0];
                break;
            }
        }
    }
    return ret;
}

// �^�e�����Ƀp�^�[����ǂݎ��B
std::wstring __fastcall XG_Board::GetPatternV(const XG_Pos& pos) const
{
    int lo, hi;
    std::wstring pattern;

    lo = hi = pos.m_i;
    while (lo > 0) {
        if (GetAt(lo - 1, xg_caret_pos.m_j) != ZEN_BLACK)
            --lo;
        else
            break;
    }
    while (hi + 1 < xg_nRows) {
        if (GetAt(hi + 1, xg_caret_pos.m_j) != ZEN_BLACK)
            ++hi;
        else
            break;
    }

    for (int i = lo; i <= hi; ++i) {
        pattern += GetAt(i, pos.m_j);
    }

    return pattern;
}

// ���R�����Ƀp�^�[����ǂݎ��B
std::wstring __fastcall XG_Board::GetPatternH(const XG_Pos& pos) const
{
    int lo, hi;
    std::wstring pattern;

    lo = hi = pos.m_j;
    while (lo > 0) {
        if (GetAt(pos.m_i, lo - 1) != ZEN_BLACK)
            --lo;
        else
            break;
    }
    while (hi + 1 < xg_nCols) {
        if (GetAt(pos.m_i, hi + 1) != ZEN_BLACK)
            ++hi;
        else
            break;
    }

    for (int j = lo; j <= hi; ++j) {
        pattern += GetAt(pos.m_i, j);
    }

    return pattern;
}
