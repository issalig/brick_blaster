#!/bin/bash
# Script para lanzar WinAPE con el proyecto actual
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
"$SCRIPT_DIR/cpctelera/cpctelera/tools/scripts/cpct_winape" "$@"
