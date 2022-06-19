/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INIFILE_H
#define INIFILE_H

#include <misc/SDL2pp.h>

#include <array>
#include <charconv>
#include <deque>
#include <filesystem>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>

inline constexpr auto INVALID_LINE = 0u;

//!  A class for reading and writing *.ini configuration files.
/*!
    This class can be used to read or write to a *.ini file. An ini-File has a very simple format.<br>
    Example:<br>
        <br>
        ; Comments start with ; or #<br>
        ; start of the first section with name ""<br>
        key1 = value1<br>
        key2 = value2<br>
        ; start of a section with name "Section1"<br>
        [Section1]<br>
        key3 = value3<br>
        key4 = value4<br>
    <br>
    The section names and key names are treated case insensitive.
*/
class INIFile {
public:
    class substring final {
    public:
        constexpr substring(std::string::size_type offset, std::string::size_type size)
            : offset_{offset}, size_{size} { }

        [[nodiscard]] constexpr std::string::size_type offset() const { return offset_; }
        [[nodiscard]] constexpr std::string::size_type size() const { return size_; }

        [[nodiscard]] constexpr bool empty() const { return size_ == 0; }
        [[nodiscard]] constexpr std::string_view apply(std::string_view s) const { return s.substr(offset_, size_); }

    private:
        std::string::size_type offset_;
        std::string::size_type size_;
    };

    class Line {
    public:
        explicit Line(std::string completeLine);
        virtual ~Line();

        Line(const Line&)            = default;
        Line(Line&&)                 = default;
        Line& operator=(const Line&) = default;
        Line& operator=(Line&&)      = default;

        [[nodiscard]] const auto& line() const { return line_; }
        [[nodiscard]] bool empty() const { return line_.empty(); }

    protected:
        std::string line_;
    };

    class Key final : public Line {
    private:
        struct initializer {
            std::string completeLine;
            substring key;
            substring value;
        };
        static initializer
        createLine(std::string_view keyname, std::string value, bool bEscapeIfNeeded = true, bool bWhitespace = true);

        Key(initializer&& init);

    public:
        Key(std::string completeLine, substring key, substring value);
        Key(std::string_view keyname, std::string_view value, bool bEscapeIfNeeded = true, bool bWhitespace = true);
        ~Key() override;

        Key(const Key&)            = default;
        Key(Key&&)                 = default;
        Key& operator=(const Key&) = default;
        Key& operator=(Key&&)      = default;

        [[nodiscard]] std::string_view getKeyName() const;
        [[nodiscard]] std::string_view getStringView() const;

        [[nodiscard]] bool getBoolValue(bool defaultValue = false) const;

        [[nodiscard]] auto getValue(auto defaultValue = {}) const noexcept {
            auto value = getStringView();
            if (value.empty())
                return defaultValue;

            if (value.front() == '+')
                value.remove_prefix(1);

            auto ret = defaultValue;
            std::from_chars(value.data(), value.data() + value.size(), ret);
            return ret;
        }

#ifdef NEED_FLOAT_FROM_CHARS
        [[nodiscard]] auto getValue(float defaultValue) const noexcept -> float {
            auto view = getStringView();
            if (view.empty())
                return defaultValue;

            if (view.front() == '+')
                view.remove_prefix(1);

            const std::string str{view};

            char* end;
            auto result = std::strtof(str.c_str(), &end);

            if (HUGE_VALF == result || end == str.c_str())
                return defaultValue;

            return result;
        }
#endif

#ifdef NEED_DOUBLE_FROM_CHARS
        [[nodiscard]] auto getValue(double defaultValue) const noexcept -> double {
            auto view = getStringView();
            if (view.empty())
                return defaultValue;

            if (view.front() == '+')
                view.remove_prefix(1);

            const std::string str{view};

            char* end;
            auto result = std::strtod(str.c_str(), &end);

            if (HUGE_VAL == result || end == str.c_str())
                return defaultValue;

            return result;
        }
#endif

        void setStringValue(std::string_view newValue, bool bEscapeIfNeeded = true);
        void setBoolValue(bool newValue);

        void setValue(auto newValue) {
            std::array<char, 128> buffer{};
            const auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), newValue);

            setStringValue(std::string_view{&buffer[0], static_cast<size_t>(ptr - &buffer[0])});
        }

#ifdef NEED_FLOAT_TO_CHARS
        void setValue(float newValue) {
            const auto str = std::to_string(newValue);

            setStringValue(str);
        }
#endif

#ifdef NEED_DOUBLE_TO_CHARS
        void setValue(double newValue) {
            const auto str = std::to_string(newValue);

            setStringValue(str);
        }
#endif

    protected:
        static bool escapingValueNeeded(std::string_view value);
        static std::string escapeValue(std::string value);

        substring key_;
        substring value_;
    };

    class Section final : public Line {
    public:
        Section(std::string completeLine, substring section, bool bWhitespace = true);
        Section(std::string_view sectionname, bool bWhitespace = true);
        ~Section() override;

        Section(const Section&)            = default;
        Section(Section&&)                 = default;
        Section& operator=(const Section&) = default;
        Section& operator=(Section&&)      = default;

        [[nodiscard]] std::string_view getSectionName() const;

    protected:
        substring section_;
        bool bWhitespace;
    };

public:
    INIFile(bool bWhitespace, std::string_view firstLineComment);
    INIFile(const std::filesystem::path& filename, bool bWhitespace = true);
    INIFile(SDL_RWops* RWopsFile, bool bWhitespace = true);
    INIFile(const INIFile&) = delete;
    INIFile(INIFile&&)      = delete;
    ~INIFile();

    auto lines() const {
        return lines_.size();
    }

    [[nodiscard]] size_t getLineNumber(std::string_view sectionname) const;
    [[nodiscard]] size_t getLineNumber(std::string_view sectionname, std::string_view keyname) const;

    [[nodiscard]] bool hasSection(std::string_view section) const;
    [[nodiscard]] const Section& getSection(std::string_view sectionname) const;
    bool removeSection(std::string_view sectionname);
    bool clearSection(std::string_view sectionname, bool bBlankLineAtSectionEnd = true);
    [[nodiscard]] bool hasKey(std::string_view section, std::string_view key) const;
    [[nodiscard]] const Key* getKey(std::string_view section, std::string_view key) const;
    bool removeKey(std::string_view section, std::string_view key);

    [[nodiscard]] std::string
    getStringValue(std::string_view section, std::string_view key, std::string_view defaultValue = {}) const;
    [[nodiscard]] int getIntValue(std::string_view section, std::string_view key, int defaultValue = 0) const;
    [[nodiscard]] bool getBoolValue(std::string_view section, std::string_view key, bool defaultValue = false) const;
    [[nodiscard]] double
    getDoubleValue(std::string_view section, std::string_view key, double defaultValue = 0.0) const;

    void
    setStringValue(std::string_view section, std::string_view key, std::string_view value, bool bEscapeIfNeeded = true);
    void setIntValue(std::string_view section, std::string_view key, int value);
    void setBoolValue(std::string_view section, std::string_view key, bool value);
    void setDoubleValue(std::string_view section, std::string_view key, double value);

    [[nodiscard]] bool saveChangesTo(const std::filesystem::path& filename, bool bDOSLineEnding = false) const;
    bool saveChangesTo(SDL_RWops* file, bool bDOSLineEnding = false) const;

    auto sections() const {
        return lines_ | std::views::filter([](auto& v) { return std::holds_alternative<Section>(v); })
             | std::views::transform([](auto& v) { return std::get<Section>(v); });
    }

    [[nodiscard]] auto keys(std::string_view sectionname) const {
        auto section = getSectionInternal(sectionname);

        return section | std::views::filter([](auto& v) { return std::holds_alternative<Key>(v); })
             | std::views::transform([](auto& v) { return std::get<Key>(v); });
    }

private:
    using line_type  = std::variant<Line, Key, Section>;
    using lines_type = std::deque<line_type>;

    lines_type lines_;

    bool bWhitespace;

    void flush() const;
    void readfile(SDL_RWops* file);

    [[nodiscard]] lines_type::iterator findSectionInternal(std::string_view sectionname);
    [[nodiscard]] lines_type::const_iterator findSectionInternal(std::string_view sectionname) const;

    [[nodiscard]] std::ranges::subrange<lines_type::iterator> getSectionInternal(std::string_view sectionname);
    [[nodiscard]] std::ranges::subrange<lines_type::const_iterator>
    getSectionInternal(std::string_view sectionname) const;

    [[nodiscard]] lines_type::iterator
    getKeyInternal(std::ranges::subrange<lines_type::iterator> section, std::string_view key);
    [[nodiscard]] lines_type::const_iterator
    getKeyInternal(std::ranges::subrange<lines_type::const_iterator> section, std::string_view key) const;
    [[nodiscard]] lines_type::iterator getKeyInternal(std::string_view section, std::string_view key);
    [[nodiscard]] lines_type::const_iterator getKeyInternal(std::string_view section, std::string_view key) const;

    [[nodiscard]] std::ranges::subrange<lines_type::iterator> getSectionOrCreate(std::string_view sectionname);

    static bool isValidSectionName(std::string_view sectionname);
    static bool isValidKeyName(std::string_view keyname);

    static int getNextChar(const unsigned char* line, int startpos);
    static int skipName(const unsigned char* line, int startpos);
    static int skipValue(const unsigned char* line, int startpos);
    static int skipKey(const unsigned char* line, int startpos);
    static int getNextQuote(const unsigned char* line, int startpos);

    static bool isWhitespace(unsigned char s);
    static bool isNormalChar(unsigned char s);

    static bool lower_compare(std::string_view s1, std::string_view s2);
};

#endif // INIFILE_H
