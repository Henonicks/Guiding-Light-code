#ifndef HMAC_SHA256_H
#define HMAC_SHA256_H

#include <openssl/sha.h>
#include <openssl/hmac.h>

#include <sstream>

std::string get_hmac_sha256_hex(std::string_view key, std::string_view message);

#endif
