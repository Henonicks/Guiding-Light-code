#!/bin/bash

sqlite3 database/dev.db "UPDATE jtc_default_values set vc_limit = REPLACE(vc_limit, 100, 0);"
sqlite3 database/release.db "UPDATE jtc_default_values set vc_limit = REPLACE(vc_limit, 100, 0);"
