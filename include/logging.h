
#ifndef LOGGING_H
#define LOGGING_H

namespace dune {

void logging_initialize();
void logging_configure(bool capture_output);
void logging_complete();

} // namespace dune

#endif // LOGGING_H
