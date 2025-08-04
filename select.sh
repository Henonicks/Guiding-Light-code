#!/bin/bash

sqlite3 database/$1.db '.mode markdown' ".output database/select/$1/$2.md" "SELECT * FROM $2;"
