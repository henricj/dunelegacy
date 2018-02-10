#pragma once

namespace Dune
{
    namespace implementation
    {
        template <typename T>
        struct reverse_adapter { T& iterable; };

        template <typename T>
        auto begin(reverse_adapter<T> w) { return std::rbegin(w.iterable); }

        template <typename T>
        auto end(reverse_adapter<T> w) { return std::rend(w.iterable); }
    }

    template <typename T>
    implementation::reverse_adapter<T> reverse(T&& iterable) { return { iterable }; }
}


