#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace sanbucraft::minecraft {

enum class TagType : std::uint8_t { End = 0, Byte, Short, Int, Long, Float, Double, ByteArray, String, List, Compound, IntArray, LongArray };

struct NbtTag {
    using ByteArray = std::vector<std::int8_t>;
    using IntArray = std::vector<std::int32_t>;
    using LongArray = std::vector<std::int64_t>;
    using List = std::vector<NbtTag>;
    using Compound = std::map<std::string, NbtTag>;
    TagType type = TagType::End;
    std::variant<std::monostate, std::int8_t, std::int16_t, std::int32_t, std::int64_t, float, double, ByteArray, std::string, List, Compound, IntArray, LongArray> value;

    const Compound* compound() const;
    const List* list() const;
    const NbtTag* find(const std::string& key) const;
    std::string stringOr(const std::string& fallback = "") const;
    std::int64_t integerOr(std::int64_t fallback = 0) const;
    double numberOr(double fallback = 0) const;
};

class NbtReader {
public:
    static bool readFile(const std::string& path, NbtTag& root, std::string& error);
    static bool read(const std::vector<std::uint8_t>& bytes, NbtTag& root, std::string& error);
};

class NbtWriter {
public:
    static bool write(const NbtTag& root, std::vector<std::uint8_t>& bytes, std::string& error);
};

}  // namespace sanbucraft::minecraft
