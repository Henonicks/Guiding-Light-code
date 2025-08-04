#!/bin/bash

names=("jtc_vcs" "temp_vc_notifications" "jtc_default_values" "no_temp_ping" "topgg_guild_choices" "topgg_guild_votes_amount" "no_noguild_reminder" "topgg_notifications" "tickets" "temp_vcs")

for x in ${names[@]}; do
    while IFS= read -r line; do
        echo $line
        line="'$line"
        line=${line//[ ]/"','"}
        line="$line'"
        echo $line
        sqlite3 database/dev.db "INSERT INTO $x VALUES ($line);"
    done < src/dev/$x.txt
done

for x in ${names[@]}; do
    while IFS= read -r line; do
        echo $line
        line="'$line"
        line=${line//[ ]/"','"}
        line="$line'"
        echo $line
        sqlite3 database/release.db "INSERT INTO $x VALUES ($line);"
    done < src/dev/$x.txt
done
