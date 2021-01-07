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
    inline void swap(XG_WordData& data1, XG_WordData& data2)
    {
        std::swap(data1.m_word, data2.m_word);
        std::swap(data1.m_hint, data2.m_hint);
    }
}

// �����f�[�^�B
extern std::vector<XG_WordData> xg_dict_data;

// �����t�@�C����ǂݍ��ށB
bool __fastcall XgLoadDictFile(LPCWSTR pszFile);

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
inline bool XgWordDataExists(const XG_WordData& wd)
{
    return binary_search(xg_dict_data.begin(), xg_dict_data.end(),
                         wd, xg_word_less());
}

// �����f�[�^���\�[�g����B
inline void XgSortDictData(void)
{
    sort(xg_dict_data.begin(), xg_dict_data.end(), xg_word_less());
}

struct XG_WordData_Equal
{
    bool operator()(const XG_WordData wd1, const XG_WordData wd2)
    {
        return wd1.m_word == wd2.m_word;
    }
};

// �����f�[�^���\�[�g���A��ӓI�ɂ���B
void __fastcall XgSortAndUniqueDictData(void);

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
