#include "minecraft/Nbt.h"

#include <cstring>
#include <fstream>
#include <limits>
#include <zlib.h>

namespace sanbucraft::minecraft {
namespace {
class Reader {
public:
    Reader(const std::vector<std::uint8_t>& bytes, std::string& error) : bytes_(bytes), error_(error) {}
    bool parseRoot(NbtTag& root) { TagType type; return typeId(type) && type != TagType::End && string(name_) && payload(type, root) && atEnd(); }
private:
    bool atEnd() { if (pos_ == bytes_.size()) return true; error_ = "Unexpected trailing bytes in NBT payload."; return false; }
    bool raw(void* destination, std::size_t count) { if (count > bytes_.size() - pos_) { error_ = "Unexpected end of NBT data."; return false; } std::memcpy(destination, bytes_.data() + pos_, count); pos_ += count; return true; }
    template <typename T> bool number(T& value) { std::uint8_t rawBytes[sizeof(T)]; if (!raw(rawBytes, sizeof(T))) return false; std::uint64_t rawValue = 0; for (auto byte : rawBytes) rawValue = (rawValue << 8U) | byte; if constexpr (std::is_floating_point_v<T>) { if constexpr (sizeof(T) == 4) { std::uint32_t bits = static_cast<std::uint32_t>(rawValue); std::memcpy(&value, &bits, sizeof(value)); } else { std::memcpy(&value, &rawValue, sizeof(value)); } } else value = static_cast<T>(rawValue); return true; }
    bool typeId(TagType& type) { std::uint8_t id = 0; if (!raw(&id, 1)) return false; if (id > static_cast<std::uint8_t>(TagType::LongArray)) { error_ = "Unknown NBT tag id: " + std::to_string(id); return false; } type = static_cast<TagType>(id); return true; }
    bool string(std::string& result) { std::uint16_t length = 0; if (!number(length)) return false; if (length > bytes_.size() - pos_) { error_ = "Invalid NBT string length."; return false; } result.assign(reinterpret_cast<const char*>(bytes_.data() + pos_), length); pos_ += length; return true; }
    bool length(std::int32_t& result) { if (!number(result)) return false; if (result < 0 || result > 16 * 1024 * 1024) { error_ = "Invalid NBT array/list length."; return false; } return true; }
    bool payload(TagType type, NbtTag& tag) {
        tag.type = type; std::int32_t count = 0;
        switch (type) {
            case TagType::Byte: { std::int8_t v; if (!number(v)) return false; tag.value = v; return true; }
            case TagType::Short: { std::int16_t v; if (!number(v)) return false; tag.value = v; return true; }
            case TagType::Int: { std::int32_t v; if (!number(v)) return false; tag.value = v; return true; }
            case TagType::Long: { std::int64_t v; if (!number(v)) return false; tag.value = v; return true; }
            case TagType::Float: { float v; if (!number(v)) return false; tag.value = v; return true; }
            case TagType::Double: { double v; if (!number(v)) return false; tag.value = v; return true; }
            case TagType::String: { std::string v; if (!string(v)) return false; tag.value = std::move(v); return true; }
            case TagType::ByteArray: { if (!length(count)) return false; NbtTag::ByteArray v(count); for (auto& x : v) if (!number(x)) return false; tag.value = std::move(v); return true; }
            case TagType::IntArray: { if (!length(count)) return false; NbtTag::IntArray v(count); for (auto& x : v) if (!number(x)) return false; tag.value = std::move(v); return true; }
            case TagType::LongArray: { if (!length(count)) return false; NbtTag::LongArray v(count); for (auto& x : v) if (!number(x)) return false; tag.value = std::move(v); return true; }
            case TagType::List: { TagType element; if (!typeId(element) || !length(count)) return false; NbtTag::List v; v.reserve(count); for (int i = 0; i < count; ++i) { NbtTag child; if (!payload(element, child)) return false; v.push_back(std::move(child)); } tag.value = std::move(v); return true; }
            case TagType::Compound: { NbtTag::Compound v; while (true) { TagType childType; if (!typeId(childType)) return false; if (childType == TagType::End) break; std::string key; NbtTag child; if (!string(key) || !payload(childType, child)) return false; v.emplace(std::move(key), std::move(child)); } tag.value = std::move(v); return true; }
            case TagType::End: tag.value = std::monostate{}; return true;
        } return false;
    }
    const std::vector<std::uint8_t>& bytes_; std::string& error_; std::size_t pos_ = 0; std::string name_;
};
bool decompress(const std::vector<std::uint8_t>& input, int windowBits, std::vector<std::uint8_t>& output, std::string& error) { z_stream stream{}; stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data())); stream.avail_in = static_cast<uInt>(input.size()); if (inflateInit2(&stream, windowBits) != Z_OK) { error = "Cannot initialize NBT decompressor."; return false; } int result; do { const std::size_t oldSize = output.size(); output.resize(oldSize + 65536); stream.next_out = output.data() + oldSize; stream.avail_out = 65536; result = inflate(&stream, Z_NO_FLUSH); output.resize(oldSize + 65536 - stream.avail_out); } while (result == Z_OK); inflateEnd(&stream); if (result != Z_STREAM_END) { error = "Invalid compressed NBT payload."; return false; } return true; }
void append16(std::vector<std::uint8_t>& out, std::uint16_t v) { out.push_back(static_cast<std::uint8_t>(v >> 8U)); out.push_back(static_cast<std::uint8_t>(v)); }
template <typename T> void appendNumber(std::vector<std::uint8_t>& out, T value) { std::uint64_t v = 0; if constexpr (std::is_floating_point_v<T>) std::memcpy(&v, &value, sizeof(T)); else v = static_cast<std::uint64_t>(value); for (int i = sizeof(T) - 1; i >= 0; --i) out.push_back(static_cast<std::uint8_t>(v >> (8 * i))); }
bool writePayload(const NbtTag& tag, std::vector<std::uint8_t>& out, std::string& error) { auto writeString = [&out](const std::string& s) { if (s.size() > 65535) return false; append16(out, static_cast<std::uint16_t>(s.size())); out.insert(out.end(), s.begin(), s.end()); return true; }; switch (tag.type) { case TagType::Byte: appendNumber(out, std::get<std::int8_t>(tag.value)); break; case TagType::Short: appendNumber(out, std::get<std::int16_t>(tag.value)); break; case TagType::Int: appendNumber(out, std::get<std::int32_t>(tag.value)); break; case TagType::Long: appendNumber(out, std::get<std::int64_t>(tag.value)); break; case TagType::Float: appendNumber(out, std::get<float>(tag.value)); break; case TagType::Double: appendNumber(out, std::get<double>(tag.value)); break; case TagType::String: if (!writeString(std::get<std::string>(tag.value))) return false; break; case TagType::List: { const auto& list = std::get<NbtTag::List>(tag.value); TagType element = list.empty() ? TagType::End : list.front().type; for (const auto& item : list) if (item.type != element) { error = "NBT lists must contain one tag type."; return false; } out.push_back(static_cast<std::uint8_t>(element)); appendNumber(out, static_cast<std::int32_t>(list.size())); for (const auto& item : list) if (!writePayload(item, out, error)) return false; break; } case TagType::Compound: for (const auto& [key, item] : std::get<NbtTag::Compound>(tag.value)) { out.push_back(static_cast<std::uint8_t>(item.type)); if (!writeString(key) || !writePayload(item, out, error)) return false; } out.push_back(0); break; default: error = "NBT writer does not support array tags yet."; return false; } return true; }
}  // namespace
const NbtTag::Compound* NbtTag::compound() const { return std::get_if<Compound>(&value); }
const NbtTag::List* NbtTag::list() const { return std::get_if<List>(&value); }
const NbtTag* NbtTag::find(const std::string& key) const { const auto* values = compound(); if (!values) return nullptr; const auto it = values->find(key); return it == values->end() ? nullptr : &it->second; }
std::string NbtTag::stringOr(const std::string& fallback) const { const auto* result = std::get_if<std::string>(&value); return result ? *result : fallback; }
std::int64_t NbtTag::integerOr(std::int64_t fallback) const { if (const auto* v = std::get_if<std::int8_t>(&value)) return *v; if (const auto* v = std::get_if<std::int16_t>(&value)) return *v; if (const auto* v = std::get_if<std::int32_t>(&value)) return *v; if (const auto* v = std::get_if<std::int64_t>(&value)) return *v; return fallback; }
double NbtTag::numberOr(double fallback) const { if (const auto* v = std::get_if<float>(&value)) return *v; if (const auto* v = std::get_if<double>(&value)) return *v; return static_cast<double>(integerOr(static_cast<std::int64_t>(fallback))); }
bool NbtReader::readFile(const std::string& path, NbtTag& root, std::string& error) { std::ifstream input(path, std::ios::binary); if (!input) { error = "Cannot open NBT file: " + path; return false; } std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)), {}); return read(bytes, root, error); }
bool NbtReader::read(const std::vector<std::uint8_t>& bytes, NbtTag& root, std::string& error) { if (bytes.empty()) { error = "NBT data is empty."; return false; } std::vector<std::uint8_t> payload; const std::vector<std::uint8_t>* raw = &bytes; if (bytes.size() >= 2 && bytes[0] == 0x1f && bytes[1] == 0x8b) { if (!decompress(bytes, 16 + MAX_WBITS, payload, error)) return false; raw = &payload; } else if (bytes.size() >= 2 && bytes[0] == 0x78) { if (!decompress(bytes, MAX_WBITS, payload, error)) return false; raw = &payload; } return Reader(*raw, error).parseRoot(root); }
bool NbtWriter::write(const NbtTag& root, std::vector<std::uint8_t>& bytes, std::string& error) { if (root.type != TagType::Compound) { error = "NBT root must be a compound."; return false; } bytes.clear(); bytes.push_back(static_cast<std::uint8_t>(root.type)); append16(bytes, 0); return writePayload(root, bytes, error); }
}  // namespace sanbucraft::minecraft
