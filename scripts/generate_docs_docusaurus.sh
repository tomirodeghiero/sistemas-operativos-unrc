#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

SOURCE_ROOT="resolucion-practicos"

if [ ! -d "$SOURCE_ROOT" ]; then
  echo "No se encontro '$SOURCE_ROOT' en la raiz del proyecto." >&2
  exit 1
fi

upper() {
  echo "$1" | tr '[:lower:]' '[:upper:]'
}

titleize() {
  echo "$1" | sed -E 's/[-_]+/ /g' | awk '{for(i=1;i<=NF;i++){ $i=toupper(substr($i,1,1)) tolower(substr($i,2)) } print}'
}

extract_num() {
  local s="$1"
  echo "$s" | sed -E 's/[^0-9]+/ /g' | awk '{print $1}'
}

practico_title() {
  local practico="$1"
  local pnum
  local year

  if [[ "$practico" =~ ^practico-([0-9]+)-([0-9]{4})$ ]]; then
    pnum="${BASH_REMATCH[1]}"
    year="${BASH_REMATCH[2]}"
    echo "Practico $pnum - $year"
    return
  fi

  pnum="$(extract_num "$practico")"
  if [ -n "$pnum" ]; then
    echo "Practico $pnum"
  else
    echo "$(titleize "$practico")"
  fi
}

practicos=()
while IFS= read -r practico_path; do
  [ -z "$practico_path" ] && continue
  practicos+=("$practico_path")
done < <(find "$SOURCE_ROOT" -mindepth 1 -maxdepth 1 -type d -name 'practico-*' | sort -V)

if [ "${#practicos[@]}" -eq 0 ]; then
  echo "No se encontraron carpetas practico-* dentro de $SOURCE_ROOT." >&2
  exit 1
fi

readmes_list="$(mktemp)"
for practico_path in "${practicos[@]}"; do
  find "$practico_path" -type f \( -name 'README.md' -o -name 'readme.md' \)
done | sort > "$readmes_list"

rm -rf docs
rm -rf static/material
mkdir -p docs static/material

while IFS= read -r src; do
  [ -z "$src" ] && continue

  src_rel="${src#${SOURCE_ROOT}/}"
  practico="${src_rel%%/*}"
  sub="${src_rel#*/}"
  dir="$(dirname "$sub")"

  if [ "$dir" = "." ]; then
    dest="docs/$practico/index.md"
    title="$(practico_title "$practico")"
    slug="/$practico/"
    position=1
  else
    bn="$(basename "$dir")"
    case "$bn" in
      resolucion[0-9]*|resolucion-[0-9]*)
        n="$(extract_num "$bn")"
        if [ -n "$n" ]; then
          title="Resolucion $n"
          position="$n"
        else
          title="$(titleize "$bn")"
          position=99
        fi
        ;;
      ejercicio[0-9]*)
        n="$(extract_num "$bn")"
        if [ -n "$n" ]; then
          title="Ejercicio $n"
          position="$n"
        else
          title="$(titleize "$bn")"
          position=99
        fi
        ;;
      *)
        title="$(titleize "$bn")"
        position=99
        ;;
    esac
    dest="docs/$practico/$dir/index.md"
    slug="/$practico/$dir/"
  fi

  mkdir -p "$(dirname "$dest")"
  {
    echo "---"
    echo "title: \"$title\""
    echo "sidebar_position: $position"
    echo "slug: \"$slug\""
    echo "description: \"Contenido importado desde $src_rel\""
    echo "---"
    echo
    sed -E \
      -e 's#(\[[^]]+\]\([^)]*)README\.md#\1index.md#g' \
      -e 's#(\[[^]]+\]\([^)]*)readme\.md#\1index.md#g' \
      "$src"
  } > "$dest"
done < "$readmes_list"

for practico_path in "${practicos[@]}"; do
  practico="$(basename "$practico_path")"
  idx="docs/$practico/index.md"
  if [ ! -f "$idx" ]; then
    mkdir -p "docs/$practico"
    ptitle="$(practico_title "$practico")"
    {
      echo "---"
      echo "title: \"$ptitle\""
      echo "sidebar_position: 1"
      echo "slug: \"/$practico/\""
      echo "description: \"Indice de documentacion para $practico\""
      echo "---"
      echo
      echo "# $ptitle"
      echo
      echo "Este practico no tenia un README general en la carpeta raiz."
      echo
    } > "$idx"
  fi
done

for practico_path in "${practicos[@]}"; do
  practico="$(basename "$practico_path")"
  idx="docs/$practico/index.md"
  subdocs="$(find "docs/$practico" -type f -name 'index.md' ! -path "$idx" | sort || true)"

  if [ -n "$subdocs" ] && ! grep -q '^## Navegacion interna$' "$idx"; then
    {
      echo
      echo "## Navegacion interna"
      echo
      echo "$subdocs" | while IFS= read -r d; do
        [ -z "$d" ] && continue
        rel="${d#docs/$practico/}"
        rel="${rel%/index.md}"
        bn="$(basename "$rel")"
        case "$bn" in
          resolucion[0-9]*|resolucion-[0-9]*)
            n="$(extract_num "$bn")"
            if [ -n "$n" ]; then
              label="Resolucion $n"
            else
              label="$(titleize "$bn")"
            fi
            ;;
          ejercicio[0-9]*)
            n="$(extract_num "$bn")"
            if [ -n "$n" ]; then
              label="Ejercicio $n"
            else
              label="$(titleize "$bn")"
            fi
            ;;
          *)
            label="$(titleize "$bn")"
            ;;
        esac
        echo "- [$label](./$rel/)"
      done
    } >> "$idx"
  fi
done

for practico_path in "${practicos[@]}"; do
  practico="$(basename "$practico_path")"
  idx="docs/$practico/index.md"
  materials="$(find "$SOURCE_ROOT/$practico" -type f \( -iname '*.pdf' -o -iname '*.odt' \) | sort || true)"
  if [ -n "$materials" ]; then
    {
      echo
      echo "## Material de apoyo"
      echo
      echo "$materials" | while IFS= read -r m; do
        [ -z "$m" ] && continue
        rel="${m#${SOURCE_ROOT}/}"
        mkdir -p "static/material/$(dirname "$rel")"
        cp "$m" "static/material/$rel"
        name="$(basename "$m")"
        echo "- [$name](/material/$rel)"
      done
    } >> "$idx"
  fi
done

{
  echo "---"
  echo "title: \"Documentacion de Practicos\""
  echo "sidebar_position: 1"
  echo "slug: \"/\""
  echo "description: \"Indice principal de la documentacion de Sistemas Operativos\""
  echo "---"
  echo
  echo "# Documentacion de Practicos"
  echo
  echo "Esta documentacion consolida los README de cada practico en formato Docusaurus."
  echo
  echo "## Practicos"
  echo
  for practico_path in "${practicos[@]}"; do
    practico="$(basename "$practico_path")"
    label="$(practico_title "$practico")"
    echo "- [$label](./$practico/)"
  done

  if [ -f "Teoria.pdf" ]; then
    mkdir -p static/material/general
    cp "Teoria.pdf" "static/material/general/Teoria.pdf"
    echo
    echo "## Material general"
    echo
    echo "- [Teoria.pdf](/material/general/Teoria.pdf)"
  fi

  echo
  echo "## Notas"
  echo
  echo "- El contenido se importa desde los README originales sin eliminarlos."
  echo "- Se agrega front matter minimo para compatibilidad con Docusaurus."
  echo "- Los enlaces a README se convierten automaticamente a rutas index.md."
  echo "- Los PDF/ODT se publican dentro de /static/material."
} > docs/intro.md

rm -f "$readmes_list"
echo "Documentacion regenerada en docs/ y static/material/"
