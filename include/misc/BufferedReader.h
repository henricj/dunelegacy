#ifndef BUFFEREDREADER_H
#define BUFFEREDREADER_H

#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <optional>
#include <span>

template<int BufferSize = 4096>
class BufferedReader final {
public:
    BufferedReader(SDL_RWops* rwop) : rwop_{rwop} { }

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

private:
    bool fill() {
        if (!pending_.empty() && pending_.data() != buffer_.data()) {
            const auto pending_size = pending_.size();

            memmove(buffer_.data(), pending_.data(), pending_size);

            pending_ = std::span{buffer_.data(), pending_size};
        }

        if (pending_.size() < buffer_.size()) {
            const auto pending_size = pending_.size();
            const auto remaining    = buffer_.size() - pending_size;

            assert(pending_size >= 0 && pending_size <= buffer_.size());
            assert(remaining > 0);

            const auto actual_read = SDL_RWread(rwop_, buffer_.data() + pending_size, 1, remaining);

            if (0 == actual_read) {
                eof_ = true;
                return false;
            }

            pending_ = std::span{buffer_.data(), pending_size + actual_read};
        }

        return true;
    }

    size_t complete_partial_read(void* data, size_t size) {
        auto partial = 0;
        auto output  = std::span{static_cast<char*>(data), size};

        if (!pending_.empty()) {
            std::ranges::copy(pending_, output.begin());
            output  = output.subspan(pending_.size());
            partial = pending_.size();
        }

        assert(partial < size);

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
    std::array<char, BufferSize> buffer_;
};

template<int BufferSize = 4096>
class SimpleBufferedReader final {
public:
    SimpleBufferedReader(SDL_RWops* rwop) : rwop_{rwop} { }

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
