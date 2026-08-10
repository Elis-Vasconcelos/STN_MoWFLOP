#!/usr/bin/env bash
# Roda a campanha completa: todas as instâncias listadas em <instances_file>
# (uma por linha), os dois algoritmos (moead, nsga2), num_runs repetições
# cada -- sequencialmente, chamando run_one.sh pra cada combinação.
#
# Uso: ./run_campaign.sh <instances_file> <num_runs> [stop_criteria] [angle] [wind]
#
# Ex.: seq 1 300 > instances.txt && ./run_campaign.sh instances.txt 20
#
# Sequencial de propósito, pra ser simples de auditar e retomar (ver
# run_one.sh sobre o skip idempotente). Pra paralelizar num scheduler de
# verdade (ex. array job do supercomputador), não chame este script -- chame
# run_one.sh diretamente, uma combinação (instance, algo, run_id) por task.

set -euo pipefail

if [[ $# -lt 2 ]]; then
  echo "uso: $0 <instances_file> <num_runs> [stop_criteria] [angle] [wind]" >&2
  exit 1
fi

instances_file="$1"
num_runs="$2"
stop_criteria="${3:-1000000}"
angle="${4:-30}"
wind="${5:-10}"

if [[ ! -f "$instances_file" ]]; then
  echo "instances_file não encontrado: $instances_file" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir/../.."   # scripts/ -> meta_heuristics/ -> source_code/ (cwd exigido pelos binários)

total=0
start_ts=$(date -Iseconds)
echo "[campanha] início $start_ts, num_runs=$num_runs, stop_criteria=$stop_criteria"

while IFS= read -r instance; do
  [[ -z "$instance" ]] && continue
  for algo in moead nsga2; do
    for ((run_id=0; run_id<num_runs; run_id++)); do
      ./meta_heuristics/scripts/run_one.sh "$instance" "$algo" "$run_id" "$stop_criteria" "$angle" "$wind"
      total=$((total+1))
    done
  done
done < "$instances_file"

echo "[campanha] fim $(date -Iseconds), início $start_ts, $total execuções"
