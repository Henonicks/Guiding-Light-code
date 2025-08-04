#!/bin/bash

mkdir database
cd database

sqlite3 dev.db "CREATE TABLE jtc_vcs (channel_id BIGINT PRIMARY KEY, guild_id BIGINT);"
sqlite3 dev.db "CREATE TABLE temp_vc_notifications (channel_id BIGINT, guild_id BIGINT PRIMARY KEY);"
sqlite3 dev.db "CREATE TABLE jtc_default_values (channel_id BIGINT PRIMARY KEY, name VARCHAR(100), vc_limit SMALLINT, bitrate SMALLINT);"
sqlite3 dev.db "CREATE TABLE no_temp_ping (user_id BIGINT PRIMARY KEY);"
sqlite3 dev.db "CREATE TABLE topgg_guild_choices (user_id BIGINT PRIMARY KEY, guild_id BIGINT);"
sqlite3 dev.db "CREATE TABLE topgg_guild_votes_amount (guild_id BIGINT PRIMARY KEY, votes INT);"
sqlite3 dev.db "CREATE TABLE no_noguild_reminder (user_id BIGINT PRIMARY KEY);"
sqlite3 dev.db "CREATE TABLE topgg_notifications (channel_id BIGINT PRIMARY KEY, guild_id BIGINT);"
sqlite3 dev.db "CREATE TABLE tickets (user_id BIGINT PRIMARY KEY, channel_id BIGINT);"
sqlite3 dev.db "CREATE TABLE temp_vcs (channel_id BIGINT PRIMARY KEY, guild_id BIGINT, creator_id BIGINT, parent_id BIGINT);"

sqlite3 release.db "CREATE TABLE jtc_vcs (channel_id BIGINT PRIMARY KEY, guild_id BIGINT);"
sqlite3 release.db "CREATE TABLE temp_vc_notifications (channel_id BIGINT, guild_id BIGINT PRIMARY KEY);"
sqlite3 release.db "CREATE TABLE jtc_default_values (channel_id BIGINT PRIMARY KEY, name VARCHAR(100), vc_limit SMALLINT, bitrate SMALLINT);"
sqlite3 release.db "CREATE TABLE no_temp_ping (user_id BIGINT PRIMARY KEY);"
sqlite3 release.db "CREATE TABLE topgg_guild_choices (user_id BIGINT PRIMARY KEY, guild_id BIGINT);"
sqlite3 release.db "CREATE TABLE topgg_guild_votes_amount (guild_id BIGINT PRIMARY KEY, votes INT);"
sqlite3 release.db "CREATE TABLE no_noguild_reminder (user_id BIGINT PRIMARY KEY);"
sqlite3 release.db "CREATE TABLE topgg_notifications (channel_id BIGINT PRIMARY KEY, guild_id BIGINT);"
sqlite3 release.db "CREATE TABLE tickets (user_id BIGINT PRIMARY KEY, channel_id BIGINT);"
sqlite3 release.db "CREATE TABLE temp_vcs (channel_id BIGINT PRIMARY KEY, guild_id BIGINT, creator_id BIGINT, parent_id BIGINT);"

mkdir select
cd select
mkdir dev
mkdir release
