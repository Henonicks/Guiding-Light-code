#include "guiding_light/database/markdown.hpp"

#include "guiding_light/generic/table_mapper.hpp"

dpp::coroutine <markdown_t> markdown_t::generate_md(const std::string_view table_name) {
	markdown_t res;
#ifdef GL_USE_LOCAL_DB
	nlohmann::json json = nlohmann::json::array();
	simple_json_types v;
	sqlite3* raw_sql = db::sql.connection().get();
	sqlite3_stmt* stmt{};
	if (sqlite3_prepare_v2(raw_sql, std::format("SELECT * FROM {};", table_name).c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		db::sql << std::format("SELECT * FROM {} -- Trying to generate a Markdown file", table_name);
		co_return res;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		json.emplace_back(nlohmann::json::object());
		auto& fields = get_field_list(table_name);
		for (int i = 0; i < fields.size(); i++) {
			json.back()[fields[i]] = reinterpret_cast <const char*>(sqlite3_column_text(stmt, i));
		}
	}
	sqlite3_finalize(stmt);
#else
	const auto callback = co_await gor_client->poke <nlohmann::json>(std::string(table_name) + "/all", dpp::m_get);
	if (!callback) {
		error_log("Finished erroneous request trying to generate a Markdown file.");
		log(std::format("Response body: {}", callback.error().body));
		co_return {false};
	}
	const nlohmann::json& json = callback.value();
#endif
	const size_t value_count = json.size();
	std::unordered_map <std::string_view, std::vector <std::string>> values;
	std::unordered_map <std::string_view, size_t> max_size;
	for (const nlohmann::json& object : json) {
		for (const auto& x : object.items()) {
			const std::string value = get_as_string(x.value());
			values[x.key()].push_back(value);
			max_size[x.key()] = std::max(max_size[x.key()], value.size());
		}
	}
	for (const std::string_view field : values | std::views::keys) {
		max_size[field] = std::max(max_size[field], field.size());
	}
	res.content = "|";
	for (const std::string_view field : get_field_list(table_name)) {
		res.content += std::format(" {:^{}} |", field, max_size[field]);
	}
	res.content += "\n|";
	for (const std::string_view field : get_field_list(table_name)) {
		res.content += std::format("{:-^{}}|", "", max_size[field] + 2);
	}
	for (size_t i = 0; i < value_count; i++) {
		res.content += "\n|";
		for (const std::string_view field : get_field_list(table_name)) {
			res.content += std::format(" {:{}} |", values[field][i], max_size[field]);
		}
	}
	co_return res;
}
