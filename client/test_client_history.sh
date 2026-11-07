#!/bin/sh
# This file is part of the YAZ toolkit.
# Copyright (C) Index Data
# See the file LICENSE for details.

set -eu
history_dir=$(mktemp -d "${TMPDIR:-/tmp}/yaz-history.XXXXXX")
trap 'rm -rf "$history_dir"' EXIT
trap 'exit 1' HUP INT TERM
: > "$history_dir/rc"

run_client() {
    env HOME="$history_dir" ./yaz-client -f "$history_dir/rc" \
        </dev/null > "$history_dir/output" 2>&1 || {
        result=$?
        cat "$history_dir/output"
        return "$result"
    }
}

# Previously persisted credentials must disappear, including abbreviations,
# nested history commands, and a final line without a newline.
printf '%s\n' 'find ordinary' 'auth user secret' 'a user other-secret' \
    'push_command pu auth user nested-secret' 'list_all' \
    > "$history_dir/.yazclient.history"
printf '%s' 'authentication user/final-secret' \
    >> "$history_dir/.yazclient.history"
printf '%s\n' 'find ordinary' 'list_all' > "$history_dir/expected"
run_client
cmp "$history_dir/expected" "$history_dir/.yazclient.history"

# An unused client with no previous history need not create a file.
rm "$history_dir/.yazclient.history"
run_client
test ! -e "$history_dir/.yazclient.history"

# An entirely filtered file must be truncated, not left untouched.
printf '%s\n' 'auth user secret' > "$history_dir/.yazclient.history"
run_client
test ! -s "$history_dir/.yazclient.history"

# Preserve an ordinary final line even when the old file lacks a newline.
printf '%s' 'find ordinary' > "$history_dir/.yazclient.history"
printf '%s\n' 'find ordinary' > "$history_dir/expected"
run_client
cmp "$history_dir/expected" "$history_dir/.yazclient.history"
