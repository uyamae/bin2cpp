#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <regex>

/**
 * @brief ���̓p�X�𐳋K�\���ɕϊ�
 */
std::wregex regex_from_path(const std::filesystem::path& expected)
{
    auto pattern = expected.wstring();
    if (pattern.find(L'.') != decltype(pattern)::npos) {
        pattern = std::regex_replace(pattern, std::wregex(L"\\."), L"\\.");
    }
    if (pattern.find(L'?') != decltype(pattern)::npos) {
        pattern = std::regex_replace(pattern, std::wregex(L"\\?"), L".");
    }
    if (pattern.find(L'*') != decltype(pattern)::npos) {
        pattern = std::regex_replace(pattern, std::wregex(L"\\*"), L".+");
    }
    return std::move(std::wregex(pattern));
}
/**
 * @brief ���͂��p�^�[���Ɉ�v���邩�ǂ���
 */
bool is_match(const std::wregex& expected, const std::filesystem::path& path)
{
    return std::regex_match(path.wstring(), expected);
}
/**
 * @brief �o�C�i���f�[�^�t�@�C������cpp �t�@�C���𐶐�
 */
int convert_cpp(const std::filesystem::path& path)
{
    // �t�@�C���T�C�Y�擾
    auto size = std::filesystem::file_size(path);
    // �t�@�C�����J��
    std::ifstream ifs(path, std::ios_base::in | std::ios_base::binary);
    if (ifs.fail()) {
        return 2;
    }
    // �o�̓t�@�C����
    auto stem = path.stem().wstring();
    auto out_cpp_name = stem + L".cpp";
    // �ꎞ�o�̓t�@�C��
    auto tmp_cpp_path = std::filesystem::temp_directory_path() / out_cpp_name;
    std::wofstream ofs(tmp_cpp_path, std::ios_base::out);
    // �w�b�_�t�@�C���p�X
    auto hpp_path = stem + L".h";

    ofs << L"#include <cstdint>" << std::endl;
    ofs << L"#include \"" << hpp_path << L"\"" << std::endl;
    ofs << L"const uint8_t " << stem << L"[] {" << std::endl;
    decltype(size) pos = 0;
    constexpr decltype(size) default_read_size = 4096;
    char buf[default_read_size];
    while (pos < size) {
        auto readsize = std::min<decltype(size)>(default_read_size, size - pos);
        ifs.read(buf, readsize);
        for (decltype(size) i = 0; i < readsize; ++i) {
            ofs << " 0x" << std::hex << std::setw(2) << std::setfill(L'0') << static_cast<uint8_t>(buf[i]) << L",";
            if (i % 32 == 31) {
                ofs << std::endl;
            }
        }
        ofs << std::endl;
        pos += readsize;
    }
    ofs << L"};" << std::endl;
    ofs.close();
    // �ꎞ�t�@�C�����o�̓t�@�C���Ɉړ�
    std::filesystem::rename(tmp_cpp_path, path.parent_path() / out_cpp_name);

    return 0;
}
/**
 * @brief �w�b�_�t�@�C�����o��
 */
int convert_hpp(const std::filesystem::path& path)
{
    // �o�̓t�@�C����
    auto stem = path.stem().wstring();
    auto out_hpp_name = stem + L".h";
    // �t�@�C���T�C�Y
    auto size = std::filesystem::file_size(path);
    // �ꎞ�o�̓t�@�C��
    auto tmp_hpp_path = std::filesystem::temp_directory_path() / out_hpp_name;
    std::wofstream ofs(tmp_hpp_path, std::ios_base::out);
    // �w�b�_�̓��e���o��
    ofs << L"#pragma once" << std::endl;
    ofs << L"constexpr size_t " << stem << L"_size{ " << size << L" };" << std::endl;
    ofs << L"extern const uint8_t " << stem << "[];" << std::endl;
    ofs.close();
    // �ꎞ�t�@�C�����o�̓t�@�C���Ɉړ�
    std::filesystem::rename(tmp_hpp_path, path.parent_path() / out_hpp_name);
    return 0;
}
/**
 * @brief ���̓p�X��ϊ�
 */
int convert(const std::filesystem::path& path)
{
    // �t�@�C���̑��݃`�F�b�N
    if (!std::filesystem::exists(path)) {
        return 1;
    }
    // cpp �t�@�C��
    if (auto err = convert_cpp(path)) {
        return err;
    }
    // hpp �t�@�C��
    if (auto err = convert_hpp(path)) {
        return err;
    }
    return 0;
}
/**
 * @brief ���͂�����
 */
int process_input(std::string_view input)
{
    int result = 0;
    std::filesystem::path input_path(input);
    // �f�B���N�g���̑��݊m�F
    std::filesystem::path dir = input_path.has_parent_path() ? input_path.parent_path() : ".";
    if (!std::filesystem::exists(dir)) {
        result = 1;
        return result;
    }
    // �t�@�C�������p�^�[����
    auto pattern = regex_from_path(input_path.has_filename() ? input_path.filename() : L"*");
    // �f�B���N�g�����̃t�@�C��������
    for (auto& entry : std::filesystem::directory_iterator(dir)) {
        std::cout << entry << std::endl;
        // �t�@�C�������w��p�^�[���Ƀ}�b�`���Ă�����
        if (is_match(pattern, entry)) {
            // �R���o�[�g����
            if (auto err = convert(entry)) {
                return err;
            }
        }
    }
    return result;
}

int main(int argc, char ** argv)
{
    if (argc < 2) {
        std::cout << "usage" << std::endl;
        std::cout << "bin2cpp input" << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (auto err = process_input(argv[i])) {
            return err;
        }
    }

    return 0;
}
