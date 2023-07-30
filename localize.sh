set -ue

target="$1"

(
  cd "$(dirname "$0")"
  keys=$(mktemp)
  existing=$(mktemp)
  (git ls-files | grep hpp$) | xargs cat | grep TRANS | sed 's/.*TRANS("\([^+]*\)").*/\1/g' | sed 's/"/\"/g' | sort | uniq > "$keys"
  cat "$target" | grep '^"' | sed 's/^"\(.*\)" = .*/\1/g' | sed 's/"/\"/g' | sort | uniq > "$existing"

  echo "No translation for:"
  cat "$keys" "$existing" "$existing" | sort | uniq -c | grep '^ *1 ' | sed 's/^\( *\)1 /\1/g'
  echo "Extra translation key:"
  cat "$keys" "$existing" "$existing" | sort | uniq -c | grep '^ *2 ' | sed 's/^\( *\)2 /\1/g'

  rm -f "$keys" "$existing"
)
