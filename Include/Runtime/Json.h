#ifndef _RT_JSON_H_
#define _RT_JSON_H_

#include <string.h>

#define JSMN_STATIC
#include <jsmn.h>

#define JSON_STRING(str, tok, data) (tok.type == JSMN_STRING && !strncmp(data + tok.start, str, (size_t)tok.end - (size_t)tok.start))

#endif /* _RT_JSON_H_ */