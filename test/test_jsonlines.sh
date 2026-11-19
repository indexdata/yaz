#!/bin/sh
# Verify JSON Lines output and compatibility with the existing JSON writer.
set -eu
srcdir=${srcdir:-.}
marcdump=../util/yaz-marcdump
jsonparse=../util/yaz-json-parse
tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/yaz-jsonlines.XXXXXXXX")
trap 'rm -rf "$tmpdir"' 0
trap 'exit 1' 1 2 15

check_record() {
    "$marcdump" -o json "$@" > "$tmpdir/pretty.json"
    "$jsonparse" -p < "$tmpdir/pretty.json" > "$tmpdir/expected.jsonl"
    "$marcdump" -o jsonlines "$@" > "$tmpdir/record.jsonl"
    cmp "$tmpdir/expected.jsonl" "$tmpdir/record.jsonl"
    test "$(wc -l < "$tmpdir/record.jsonl")" -eq 1
}

# ISO2709 input, including character conversion, and JSON input.
for f in "$srcdir"/marc-files/marc[0-9].marc "$srcdir"/marc-files/marc[1-9][0-9].marc; do
    charset=$(cat "${f%.marc}.chr")
    check_record -f "$charset" -t utf-8 -i marc "$f"
    cp "$tmpdir/pretty.json" "$tmpdir/input.json"
    check_record -f utf-8 -t utf-8 -i json "$tmpdir/input.json"
    "$marcdump" -f utf-8 -t utf-8 -i jsonlines -o jsonlines "$tmpdir/record.jsonl" > "$tmpdir/actual.jsonl"
    cmp "$tmpdir/record.jsonl" "$tmpdir/actual.jsonl"
done

record="$srcdir/marc-files/marc0.marc"
charset=$(cat "$srcdir/marc-files/marc0.chr")
check_record -f "$charset" -t utf-8 -i marc "$record"
cp "$tmpdir/record.jsonl" "$tmpdir/one.jsonl"
cat "$tmpdir/one.jsonl" "$tmpdir/one.jsonl" > "$tmpdir/two.jsonl"

# A record stream and multiple input files both produce one line per record.
cat "$record" "$record" > "$tmpdir/two.marc"
"$marcdump" -f "$charset" -t utf-8 -o jsonlines "$tmpdir/two.marc" > "$tmpdir/actual.jsonl"
cmp "$tmpdir/two.jsonl" "$tmpdir/actual.jsonl"
"$marcdump" -f "$charset" -t utf-8 -o jsonlines "$record" "$record" > "$tmpdir/actual.jsonl"
cmp "$tmpdir/two.jsonl" "$tmpdir/actual.jsonl"
"$marcdump" -f "$charset" -t utf-8 -o jsonlines -O 1 -L 1 "$tmpdir/two.marc" > "$tmpdir/actual.jsonl"
cmp "$tmpdir/one.jsonl" "$tmpdir/actual.jsonl"

# Empty input must not produce a blank line or an empty array.
: > "$tmpdir/empty.marc"
"$marcdump" -o jsonlines "$tmpdir/empty.marc" > "$tmpdir/actual.jsonl"
test ! -s "$tmpdir/actual.jsonl"

# Read a stream and multiple input files, resetting MARC state for each record.
"$marcdump" -i jsonlines -o jsonlines -r "$tmpdir/two.jsonl" > "$tmpdir/actual.jsonl" 2> "$tmpdir/report"
cmp "$tmpdir/two.jsonl" "$tmpdir/actual.jsonl"
grep -F 'records read: 2' "$tmpdir/report"
"$marcdump" -i jsonlines -o jsonlines "$tmpdir/one.jsonl" "$tmpdir/one.jsonl" > "$tmpdir/actual.jsonl"
cmp "$tmpdir/two.jsonl" "$tmpdir/actual.jsonl"
"$marcdump" -i jsonlines -o jsonlines -O 1 -L 1 "$tmpdir/two.jsonl" > "$tmpdir/actual.jsonl"
cmp "$tmpdir/one.jsonl" "$tmpdir/actual.jsonl"
"$marcdump" -i jsonlines -o jsonlines -L 0 "$tmpdir/two.jsonl" > "$tmpdir/actual.jsonl"
test ! -s "$tmpdir/actual.jsonl"
"$marcdump" -i jsonlines -o jsonlines "$tmpdir/empty.marc" > "$tmpdir/actual.jsonl"
test ! -s "$tmpdir/actual.jsonl"

# CRLF and a final line without a terminator are accepted.
awk '{ printf "%s\r\n", $0 }' "$tmpdir/two.jsonl" > "$tmpdir/crlf.jsonl"
"$marcdump" -i jsonlines -o jsonlines "$tmpdir/crlf.jsonl" > "$tmpdir/actual.jsonl"
cmp "$tmpdir/two.jsonl" "$tmpdir/actual.jsonl"
printf '%s' "$(cat "$tmpdir/one.jsonl")" > "$tmpdir/unterminated.jsonl"
"$marcdump" -i jsonlines -o jsonlines "$tmpdir/unterminated.jsonl" > "$tmpdir/actual.jsonl"
cmp "$tmpdir/one.jsonl" "$tmpdir/actual.jsonl"

# Escaped newlines and other control characters remain inside one record.
printf '%s\n' '{"leader":"00000nam a22000000a 4500","fields":[{"001":"  first\nsecond\r\t\"\\  "}]}' \
    > "$tmpdir/escaped.json"
"$marcdump" -i json -o jsonlines "$tmpdir/escaped.json" > "$tmpdir/escaped.jsonl"
"$marcdump" -i jsonlines -o jsonlines "$tmpdir/escaped.jsonl" > "$tmpdir/actual.jsonl"
cmp "$tmpdir/escaped.jsonl" "$tmpdir/actual.jsonl"

# Output may also be ISO2709, using the existing JSON reader as a reference.
"$marcdump" -i json -o marc "$tmpdir/pretty.json" > "$tmpdir/one.marc"
cat "$tmpdir/one.marc" "$tmpdir/one.marc" > "$tmpdir/expected.marc"
"$marcdump" -i jsonlines -o marc "$tmpdir/two.jsonl" > "$tmpdir/actual.marc"
cmp "$tmpdir/expected.marc" "$tmpdir/actual.marc"

expect_error() {
    status=0
    "$marcdump" -i jsonlines -o jsonlines "$1" > "$tmpdir/actual.jsonl" 2> "$tmpdir/error" || status=$?
    test "$status" -eq 5
    grep -F "$1:$2:" "$tmpdir/error"
}

# Bad lines report their physical line number, then resume at the next record.
cat "$tmpdir/one.jsonl" > "$tmpdir/bad.jsonl"
printf '{bad json}\n' >> "$tmpdir/bad.jsonl"
cat "$tmpdir/one.jsonl" >> "$tmpdir/bad.jsonl"
expect_error "$tmpdir/bad.jsonl" 2
cmp "$tmpdir/two.jsonl" "$tmpdir/actual.jsonl"
printf '\n' > "$tmpdir/blank.jsonl"
expect_error "$tmpdir/blank.jsonl" 1
printf '[]\n' > "$tmpdir/array.jsonl"
expect_error "$tmpdir/array.jsonl" 1
printf '%s%s\n' "$(cat "$tmpdir/one.jsonl")" "$(cat "$tmpdir/one.jsonl")" > "$tmpdir/joined.jsonl"
expect_error "$tmpdir/joined.jsonl" 1
printf '%s\000trailing\n' "$(cat "$tmpdir/one.jsonl")" > "$tmpdir/nul.jsonl"
expect_error "$tmpdir/nul.jsonl" 1

# The line limit is inclusive. Oversized lines are discarded, never truncated
# into a valid JSON prefix, and do not consume the following record.
LC_ALL=C awk 'NR == 1 { printf "%s", $0; for (i = length($0); i < 1048576; i++) printf " "; printf "\n" }' \
    "$tmpdir/one.jsonl" > "$tmpdir/limit.jsonl"
"$marcdump" -i jsonlines -o jsonlines "$tmpdir/limit.jsonl" > "$tmpdir/actual.jsonl"
cmp "$tmpdir/one.jsonl" "$tmpdir/actual.jsonl"
sed 's/$/ /' "$tmpdir/limit.jsonl" > "$tmpdir/oversized.jsonl"
cat "$tmpdir/one.jsonl" >> "$tmpdir/oversized.jsonl"
expect_error "$tmpdir/oversized.jsonl" 1
grep -F 'exceeds 1048576 bytes' "$tmpdir/error"
cmp "$tmpdir/one.jsonl" "$tmpdir/actual.jsonl"

# Line input uses the same output writer.
"$marcdump" -f "$charset" -t utf-8 -o line "$record" > "$tmpdir/record.line"
check_record -f utf-8 -t utf-8 -i line "$tmpdir/record.line"

# XML input is optional in minimal builds.
if "$marcdump" -i marcxml > /dev/null 2>&1; then
    "$marcdump" -f "$charset" -t utf-8 -o marcxml "$record" > "$tmpdir/record.xml"
    check_record -f utf-8 -t utf-8 -i marcxml "$tmpdir/record.xml"
    "$marcdump" -i jsonlines -o marcxml "$tmpdir/two.jsonl" > "$tmpdir/two.xml"
    "$marcdump" -i marcxml -o jsonlines "$tmpdir/two.xml" > "$tmpdir/actual.jsonl"
    cmp "$tmpdir/two.jsonl" "$tmpdir/actual.jsonl"
fi
