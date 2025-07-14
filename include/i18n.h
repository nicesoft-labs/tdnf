#pragma once
#include <stdint.h>

uint32_t TDNFLoadTranslations(const char *lang);
const char* TDNFTranslate(const char *msg);
void TDNFFreeTranslations(void);
