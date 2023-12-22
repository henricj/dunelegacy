#ifndef MACFUNCTIONS_H
#define MACFUNCTIONS_H

#if __cplusplus
extern "C" {
#endif

const char* getMacLanguage();

void getMacApplicationSupportFolder(char* buffer, int len);

#if __cplusplus
}
#endif

#endif //MACFUNCTIONS_H
