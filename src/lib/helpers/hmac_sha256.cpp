#include <guiding_light/helpers/hmac_sha256.hpp>

#include <iomanip>

std::string get_hmac_sha256_hex(const std::string_view key, const std::string_view message) {
	uint8_t hash[EVP_MAX_MD_SIZE];
	uint32_t hash_size;
	HMAC(
		EVP_sha256(),
		key.data(),
		static_cast <int>(key.size()),
		reinterpret_cast <const uint8_t*>(message.data()),
		message.size(),
		hash,
		&hash_size
	);
	std::stringstream res;
	for (uint32_t i = 0; i < hash_size; i++) {
		res << std::setfill('0') << std::setw(2) << std::right << std::hex << static_cast <int>(hash[i]);
	}
	return res.str();
}
