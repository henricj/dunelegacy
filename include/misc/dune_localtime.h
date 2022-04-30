#ifndef DUNE_LOCALTIME_H
#define DUNE_LOCALTIME_H

#include <ctime>
#include <optional>
#include <string>

namespace dune {

std::optional<std::tm> dune_localtime();

std::string dune_localtime_string();

} // namespace dune

#endif // DUNE_LOCALTIME
