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

# Line input uses the same output writer.
"$marcdump" -f "$charset" -t utf-8 -o line "$record" > "$tmpdir/record.line"
check_record -f utf-8 -t utf-8 -i line "$tmpdir/record.line"

# XML input is optional in minimal builds.
if "$marcdump" -i marcxml > /dev/null 2>&1; then
    "$marcdump" -f "$charset" -t utf-8 -o marcxml "$record" > "$tmpdir/record.xml"
    check_record -f utf-8 -t utf-8 -i marcxml "$tmpdir/record.xml"
fi

# JSON Lines input is reserved for a separate change.
if "$marcdump" -i jsonlines "$tmpdir/one.jsonl" > /dev/null 2>&1; then
    echo 'Unexpected support for -i jsonlines' >&2
    exit 1
fi
