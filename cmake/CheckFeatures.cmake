include(CheckSymbolExists)
include(CheckCXXSourceCompiles)

check_symbol_exists(strerror_s "string.h" HAVE_STRERROR_S)

check_cxx_source_compiles(
    "#include <time.h>
    int main(int argc, char* argv[]) {
        time_t rawtime{};
        tm timeinfo{};
        int error = localtime_s(&timeinfo, &rawtime);
        return 0;
    }"
    HAVE_MS_LOCALTIME_S)

check_cxx_source_compiles(
    "#include <time.h>
    int main(int argc, char* argv[]) {
        time_t rawtime{};
        tm timeinfo{};
        tm* result = localtime_r(&rawtime, &timeinfo);
        return 0;
    }"
    HAVE_MS_LOCALTIME_R)


check_cxx_source_compiles(
    "#include <string.h>
    char buffer[64];
    int main(int argc, char* argv[]) {
        int error = strerror_r(7, buffer, sizeof(buffer));
        return 0;
    }"
    HAVE_STRERROR_R)

check_cxx_source_compiles(
    "#include <string.h>
    char buffer[64];
    int main(int argc, char* argv[]) {
        char* error = strerror_r(7, buffer, sizeof(buffer));
        return 0;
    }"
    HAVE_GNU_STRERROR_R)

check_cxx_source_compiles(
    "#include <charconv>
    char buffer[64];
    int main(int argc, char* argv[]) {
        auto [ptr, ec] = std::to_chars(&buffer[0], &buffer[sizeof(buffer)], 1.0f);
        return 0;
    }"
    HAVE_FLOAT_TO_CHARS)

check_cxx_source_compiles(
    "#include <charconv>
    char buffer[64];
    int main(int argc, char* argv[]) {
        auto [ptr, ec] = std::to_chars(&buffer[0], &buffer[sizeof(buffer)], 1.0);
        return 0;
    }"
    HAVE_DOUBLE_TO_CHARS)

check_cxx_source_compiles(
    "#include <charconv>
    const char number[] = \"1.0\";
    int main(int argc, char* argv[]) {
        float value;
        auto [ptr, ec] = std::from_chars(&number[0], &number[sizeof(number) - 1], value);
        return 0;
    }"
    HAVE_FLOAT_FROM_CHARS)

check_cxx_source_compiles(
    "#include <charconv>
    const char number[] = \"1.0\";
    int main(int argc, char* argv[]) {
        double value;
        auto [ptr, ec] = std::from_chars(&number[0], &number[sizeof(number) - 1], value);
        return 0;
    }"
    HAVE_DOUBLE_FROM_CHARS)

check_cxx_source_compiles(
    "#include <vector>
    struct Foo { int a; double b; bool c; };
    int main(int argc, char* argv[]) {
        std::vector<Foo> v;
        v.emplace_back(123, -3.456, false);
        return 0;
    }"
    HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES)

check_cxx_source_compiles(
    "#include <future>
    int main(int argc, char* argv[]) {
        auto fut = std::async(std::launch::async | std::launch::deferred, [] { return 42; });
        return fut.get();
    }"
    HAVE_STD_ASYNC)
