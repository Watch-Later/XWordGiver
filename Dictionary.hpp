//////////////////////////////////////////////////////////////////////////////
// Dictionary.hpp --- XWord Giver (Japanese Crossword Generator)
// Copyright (C) 2012-2020 Katayama Hirofumi MZ. All Rights Reserved.
// (Japanese, Shift_JIS)

#ifndef __XG_DICTIONARY_HPP__
#define __XG_DICTIONARY_HPP__

//////////////////////////////////////////////////////////////////////////////
// �P��f�[�^�B

struct XG_WordData
{
    // �P��B
    std::wstring     m_word;
    // �q���g�B
    std::wstring     m_hint;

    // �R���X�g���N�^�B
    XG_WordData() { }

    // �R���X�g���N�^�B
    XG_WordData(const std::wstring& word_) : m_word(word_)
    {
    }

    // �R���X�g���N�^�B
    XG_WordData(const std::wstring& word_, const std::wstring& hint_) :
        m_word(word_), m_hint(hint_)
    {
    }

    // �R���X�g���N�^�B
    XG_WordData(std::wstring&& word_, std::wstring&& hint_) :
        m_word(std::move(word_)), m_hint(std::move(hint_))
    {
    }

    // �R�s�[�R���X�g���N�^�B
    XG_WordData(const XG_WordData& wd) :
        m_word(wd.m_word), m_hint(wd.m_hint)
    {
    }

    // �R�s�[�R���X�g���N�^�B
    XG_WordData(XG_WordData&& wd) :
        m_word(std::move(wd.m_word)), m_hint(std::move(wd.m_hint))
    {
    }

    // ����B
    void __fastcall operator=(const XG_WordData& wd) {
        m_word = wd.m_word;
        m_hint = wd.m_hint;
    }

    // ����B
    void __fastcall operator=(XG_WordData&& wd) {
        m_word = std::move(wd.m_word);
        m_hint = std::move(wd.m_hint);
    }

    void clear() {
        m_word.clear();
        m_hint.clear();
    }
};

namespace std
{
    template <>
    inline void swap(XG_WordData& data1, XG_WordData& data2) {
        std::swap(data1.m_word, data2.m_word);
        std::swap(data1.m_hint, data2.m_hint);
    }
}

// �����f�[�^�B�D��^�O���ۂ��ŕ�����B
extern std::vector<XG_WordData> xg_dict_1, xg_dict_2;
// �^�O�t���f�[�^�B
extern std::unordered_map<std::wstring, std::unordered_set<std::wstring> > xg_word_to_tags_map;
// �^�O�̃q�X�g�O�����B
extern std::unordered_map<std::wstring, size_t> xg_tag_histgram;
// �P��̒����̃q�X�g�O�����B
extern std::unordered_map<size_t, size_t> xg_word_length_histgram;
// �D��^�O�B
extern std::unordered_set<std::wstring> xg_priority_tags;
// ���O�^�O�B
extern std::unordered_set<std::wstring> xg_forbidden_tags;
// �e�[�}������B
extern std::wstring xg_strTheme;
// ����̃e�[�}������B
extern std::wstring xg_strDefaultTheme;
// �e�[�}���ύX���ꂽ���H
extern bool xg_bThemeModified;

// �����t�@�C����ǂݍ��ށB
bool __fastcall XgLoadDictFile(LPCWSTR pszFile);
// �e�[�}�����Z�b�g����B
void __fastcall XgResetTheme(HWND hwnd);
// �e�[�}��ݒ肷��B
void XgSetThemeString(const std::wstring& strTheme);
// �e�[�}��������p�[�X����B
void XgParseTheme(std::unordered_set<std::wstring>& priority,
                  std::unordered_set<std::wstring>& forbidden,
                  const std::wstring& strTheme);

// �~�j�������쐬����B
std::vector<XG_WordData> XgCreateMiniDict(void);

//////////////////////////////////////////////////////////////////////////////
// XG_WordData�\���̂��r����t�@���N�^�B

class xg_word_less
{
public:
    bool __fastcall operator()(const XG_WordData& wd1, const XG_WordData& wd2)
    {
        return wd1.m_word < wd2.m_word;
    }
};

// �P��f�[�^�����݂��邩�H
inline bool XgWordDataExists(const std::vector<XG_WordData>& data, const XG_WordData& wd)
{
    return binary_search(data.begin(), data.end(), wd, xg_word_less());
}

struct XG_WordData_Equal
{
    bool operator()(const XG_WordData wd1, const XG_WordData wd2)
    {
        return wd1.m_word == wd2.m_word;
    }
};

// �����f�[�^���\�[�g���A��ӓI�ɂ���B
void __fastcall XgSortAndUniqueDictData(std::vector<XG_WordData>& data);

//////////////////////////////////////////////////////////////////////////////
// �P�ꂪ�������邩�ǂ������m�F����t�@���N�^�B

class xg_word_toolong
{
public:
    xg_word_toolong(int n)
    {
        m_n = n;
    }

    bool __fastcall operator()(const XG_WordData& wd) const
    {
        return static_cast<int>(wd.m_word.size()) > m_n;
    }

protected:
    int m_n;    // �ő咷�B
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef __XG_DICTIONARY_HPP__
