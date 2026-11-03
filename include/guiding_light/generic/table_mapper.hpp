#ifndef TABLE_MAPPER_H
#define TABLE_MAPPER_H

#include <map>
#include <string>
#include <vector>
/**
 * Lists of fields per table
 */
inline std::map <std::string_view, const std::vector <std::string_view>*> field_lists;

void map_field_lists();

inline auto& get_field_list(const std::string_view s) {
	return *field_lists[s];
}

#endif
