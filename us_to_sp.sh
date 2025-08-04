#!/bin/bash

sqlite3 database/dev.db "UPDATE jtc_default_values set name = REPLACE(name, '_', ' ');"
sqlite3 database/release.db "UPDATE jtc_default_values set name = REPLACE(name, '_', ' ');"
