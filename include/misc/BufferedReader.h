#ifndef BUFFEREDREADER_H
#define BUFFEREDREADER_H

#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <ios>
#include <optional>
#include <span>

template<int BufferSize = 32768>
class BufferedReader final {
public:
    explicit BufferedReader(SDL_RWops* rwop) : rwop_{rwop} { }

    size_t read(void* data, size_t size, size_t maxnum) {
        if (size < 1 || maxnum < 1)
            THROW(std::invalid_argument, "Attempting zero-length read from file");

        if (eof_)
            return 0;

        if (pending_.size() < size) {
            if (!fill())
                return 0;
        }

        // We're still short, try to complete a partial read, if possible.
        if (pending_.size() < size)
            return complete_partial_read(data, size);

        auto output = std::span{static_cast<char*>(data), size * maxnum};

        // Complete as much as we can.
        const auto count = std::min(pending_.size(), output.size()) / size;
        assert(count >= 1);

        const auto copy_size = count * size;

        std::copy_n(pending_.data(), copy_size, output.data());

        pending_ = pending_.subspan(copy_size);

        return count;
    }

    bool read_one(void* data, size_t size) { return 1 == read(data, size, 1); }

    template<typename TValue>
    bool read_type(TValue& value) {
        return read_one(&value, sizeof(TValue));
    }

    [[nodiscard]] size_t size() const { return SDL_RWsize(rwop_); }

    [[nodiscard]] size_t position() const { return SDL_RWtell(rwop_) - pending_.size(); }

    void clear() { pending_ = {}; }

    void seek(size_t pos) {
        const auto current_unbuffered = [rwop = rwop_] {
            const auto tell = SDL_RWtell(rwop);

            if (tell < 0)
                THROW(std::runtime_error, "Unable to get the current position: {}!", SDL_GetError());

            return static_cast<size_t>(tell);
        }();

        const auto current = current_unbuffered - pending_.size();

        if (pos == current)
            return;

        if (pos >= current && pos < current_unbuffered) {
            const auto offset = pos - current;

            pending_ = pending_.subspan(offset);
            return;
        }

        // We might be able to do better since anything in the buffer_ to the left
        // of pending_ is still valid.
        const auto previous = static_cast<size_t>(pending_.data() - buffer_.data());
        assert(current >= previous);

        const auto buffer_position = current - previous;
        if (pos >= buffer_position && pos < current_unbuffered) {
            const auto offset = pos - buffer_position;

            pending_ = std::span{buffer_.data() + offset, current_unbuffered - pos};
            return;
        }

        clear();
        if (-1 == SDL_RWseek(rwop_, pos, SEEK_SET))
            THROW(std::runtime_error, "Unable to seek file.");
    }

    void skip(std::streamoff pos) {
        if (0 == pos)
            return;

        if (pos > 0 && static_cast<size_t>(pos) < pending_.size()) {
            pending_ = pending_.subspan(pos);
            return;
        }

        if (pos < 0) {
            const auto previous      = pending_.data() - buffer_.data();
            const auto buffer_offset = previous + pos;

            if (buffer_offset >= 0) {
                pending_ = std::span{buffer_.data() + buffer_offset, static_cast<size_t>(pending_.size() - pos)};
                return;
            }
        }

        // We might be able to do better since anything in the buffer_ to the left
        // of pending_ is still valid.

        pos -= pending_.size();
        clear();

        if (-1 == SDL_RWseek(rwop_, pos, SEEK_CUR))
            THROW(std::runtime_error, "Unable to skip file.");
    }

private:
    bool fill() {
        if (!pending_.empty() && pending_.data() != buffer_.data()) {
            const auto pending_size = pending_.size();

            memmove(buffer_.data(), pending_.data(), pending_size);

            pending_ = std::span{buffer_.data(), pending_size};
        }

        const auto pending_size = pending_.size();
        const auto buffer_size  = buffer_.size();

        if (pending_size >= buffer_size)
            THROW(std::runtime_error, "The buffer is already full ({} >= {})!", pending_size, buffer_size);

        const auto remaining = buffer_size - pending_size;

        assert(pending_size >= 0 && pending_size <= buffer_size);
        assert(remaining > 0);

        const auto actual_read = SDL_RWread(rwop_, buffer_.data() + pending_size, 1, remaining);

        if (0 == actual_read) {
            eof_ = true;
            return false;
        }

        pending_ = std::span{buffer_.data(), pending_size + actual_read};
        return true;
    }

    size_t complete_partial_read(void* data, size_t size) {
        auto partial = size_t{0};
        auto output  = std::span{static_cast<char*>(data), size};

        if (!pending_.empty()) {
            std::ranges::copy(pending_, output.begin());
            output  = output.subspan(pending_.size());
            partial = pending_.size();
        }

        assert(size_t(partial) < size);

        const auto read_length = size - partial;

        const auto actual = SDL_RWread(rwop_, output.data(), read_length, 1);

        if (0 == actual) {
            eof_ = true;
            return 0;
        }

        output   = output.subspan(read_length);
        pending_ = std::span<char>{};

        return 1;
    }

    SDL_RWops* rwop_;
    bool eof_{};
    std::span<char> pending_;
    std::array<char, BufferSize> buffer_{};
};

template<int BufferSize = 32768>
class SimpleBufferedReader final {
public:
    explicit SimpleBufferedReader(SDL_RWops* rwop) : rwop_{rwop} { }

    size_t read(void* data, size_t maxnum) {
        if (maxnum < 1)
            THROW(std::invalid_argument, "Attempting zero-length read from file");

        if (eof_)
            return 0;

        if (pending_.empty()) {
            if (!fill())
                return 0;
        }

        auto output = std::span{static_cast<char*>(data), maxnum};

        // Complete as much as we can.
        const auto count = std::min(pending_.size(), output.size());
        assert(count >= 1);

        const auto copy_size = count;

        std::copy_n(pending_.data(), copy_size, output.data());

        pending_ = pending_.subspan(copy_size);

        return count;
    }

    std::optional<char> getch() {
        if (eof_)
            return std::nullopt;

        if (pending_.empty()) {
            if (!fill())
                return std::nullopt;
        }

        const auto c = pending_[0];

        pending_ = pending_.subspan(1);

        return c;
    }

    [[nodiscard]] size_t size() const { return SDL_RWsize(rwop_); }

    [[nodiscard]] size_t position() const { return SDL_RWtell(rwop_) - pending_.size(); }

    void clear() { pending_ = {}; }

    void seek(size_t pos) {
        clear();
        if (-1 == SDL_RWseek(rwop_, pos, SEEK_SET))
            THROW(std::runtime_error, "Unable to seek file.");
    }

    void skip(int64_t pos) {
        if (pos > 0 && pos < pending_.size()) {
            pending_ = pending_.subspan(pos);
            return;
        }

        pos -= pending_.size();
        clear();

        if (-1 == SDL_RWseek(rwop_, pos, SEEK_CUR))
            THROW(std::runtime_error, "Unable to skip file.");
    }

private:
    bool fill() {
        if (!pending_.empty())
            return true;

        const auto actual_read = SDL_RWread(rwop_, buffer_.data(), 1, buffer_.size());

        if (0 == actual_read) {
            eof_ = true;
            return false;
        }

        pending_ = std::span{buffer_.data(), actual_read};

        return true;
    }

    SDL_RWops* rwop_;
    bool eof_{};
    std::span<char> pending_;
    std::array<char, BufferSize> buffer_;
};

#endif // BUFFEREDREADER_H
