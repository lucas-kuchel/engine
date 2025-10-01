#pragma once

#include <fstream>
#include <span>
#include <string>
#include <vector>

namespace filesystem {
    template <typename T>
    requires(std::is_integral_v<T>)
    class BinaryFile {
    public:
        BinaryFile(const std::string& path) {
            file_.open(path, std::ios::binary | std::ios::in | std::ios::out);

            if (!file_) {
                throw std::runtime_error("Construction failed: filesystem::BinaryFile: Failed to open file");
            }
        }

        ~BinaryFile() = default;

        std::size_t getSize() {
            file_.seekg(0, std::ios::end);

            if (!file_) {
                throw std::runtime_error("Call failed: filesystem::BinaryFile::getSize(): Failed to move read pointer");
            }

            return static_cast<std::size_t>(file_.tellg());
        }

        std::vector<T> read() {
            file_.seekg(0, std::ios::end);

            if (!file_) {
                throw std::runtime_error("Call failed: filesystem::BinaryFile::read(): Failed to move read pointer");
            }

            std::size_t size = static_cast<std::size_t>(file_.tellg());

            if (size % sizeof(T) != 0) {
                throw std::runtime_error("Call failed: filesystem::BinaryFile::read(): File size does not match provided integer size");
            }

            file_.seekg(0, std::ios::beg);

            if (!file_) {
                throw std::runtime_error("Call failed: filesystem::BinaryFile::read(): Failed to move read pointer");
            }

            std::vector<T> data(size / sizeof(T));

            if (!file_.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size))) {
                throw std::runtime_error("Call failed: filesystem::BinaryFile::read(): Error reading file");
            }

            return data;
        }

        void write(std::span<T> data, std::size_t offsetBytes) {
            file_.seekp(offsetBytes, std::ios::beg);

            if (!file_) {
                throw std::runtime_error("Call failed: filesystem::BinaryFile::write(): Failed to move write pointer");
            }

            file_.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));

            if (!file_) {
                throw std::runtime_error("Call failed: filesystem::BinaryFile::write(): Failed to write to file");
            }

            file_.flush();

            if (!file_) {
                throw std::runtime_error("Call failed: filesystem::BinaryFile::write(): Failed to flush file");
            }
        }

    private:
        std::fstream file_;
    };
}