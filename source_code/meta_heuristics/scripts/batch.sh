#!/usr/bin/env bash
# Adapta scripts/batch.sh do repo do Gustavo/João
# (MO_WFLOP-experiment-runner): mesmo idioma -- um `nohup <script> ... &>
# logfile &` por instância/lote, sem scheduler, sem limite de concorrência,
# sem `wait` final. Only diferenças reais: a lista de instâncias vem de um
# arquivo em vez de literais hardcoded no script, e um processo por
# instância em vez de lotes de ~10 (eles tinham 300+ instâncias pra
# distribuir; este conjunto tem só 10, não precisa agrupar). Chama
# run_instance.sh (nosso main.sh) em vez do main.sh deles.
#
# Uso: ./batch.sh [instances_file] [algos] [num_runs] [stop_criteria] [angle] [wind] [stn_p] [stn_interval]
#
# Ex.: ./batch.sh                                   # 10 instâncias do conjunto STN, moead+nsga2, 20 runs, defaults
#      ./batch.sh instances_stn10.txt "moead" 20 1000000 30 10 100 10
#
# Precisa ser chamado com cwd = source_code/ (mesmo requisito de run_one.sh).
# Logs em source_code/logs/<instance>.log. Idempotente via run_one.sh: seguro
# relançar pra retomar depois de uma interrupção.

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir/../.."   # scripts/ -> meta_heuristics/ -> source_code/

instances_file="${1:-instances_stn10.txt}"
algos="${2:-moead nsga2}"
num_runs="${3:-10}"
stop_criteria="${4:-1000000}"
angle="${5:-30}"
wind="${6:-10}"
stn_p="${7:-10}"
stn_interval="${8:-50}"

if [[ ! -f "$instances_file" ]]; then
  echo "instances_file não encontrado: $instances_file" >&2
  exit 1
fi

mkdir -p logs

while IFS= read -r instance; do
  [[ -z "$instance" ]] && continue
  # stn_p/stn_interval no nome do log também -- senão relançar batch.sh com
  # um P diferente sobrescreve o log da rodada anterior, mesmo que os dados
  # (agora em diretórios separados, ver run_one.sh) fiquem corretos
  log="logs/${instance}_p${stn_p}_i${stn_interval}.log"
  nohup ./meta_heuristics/scripts/run_instance.sh "$instance" "$algos" "$num_runs" "$stop_criteria" "$angle" "$wind" "$stn_p" "$stn_interval" \
    &> "$log" &
  echo "[batch] instance=$instance pid=$! log=$log"
done < "$instances_file"
