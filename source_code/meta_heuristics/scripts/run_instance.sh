#!/usr/bin/env bash
# Roda todas as combinações (algo, run_id) de UMA instância, chamando
# run_one.sh pra cada uma. Papel equivalente ao main.sh do repo do
# Gustavo/João (MO_WFLOP-experiment-runner/scripts/main.sh): lá, main.sh
# recebe um lote de instâncias e faz for instance / for run chamando
# comolsd.sh; aqui cada instância já tem seu próprio processo (ver
# batch.sh), então este script cuida só do for algo / for run_id de uma
# instância só, chamando run_one.sh (nosso equivalente ao comolsd.sh deles).
#
# Uso: ./run_instance.sh <instance_id> [algos] [num_runs] [stop_criteria] [angle] [wind] [stn_p] [stn_interval]
#
# Precisa ser chamado com cwd = source_code/ (mesmo requisito de run_one.sh).

set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "uso: $0 <instance_id> [algos] [num_runs] [stop_criteria] [angle] [wind] [stn_p] [stn_interval]" >&2
  exit 1
fi

instance="$1"
algos="${2:-moead nsga2}"
num_runs="${3:-10}"
stop_criteria="${4:-1000000}"
angle="${5:-30}"
wind="${6:-10}"
stn_p="${7:-10}"
stn_interval="${8:-50}"

for algo in $algos; do
  for ((run_id=0; run_id<num_runs; run_id++)); do
    ./meta_heuristics/scripts/run_one.sh "$instance" "$algo" "$run_id" "$stop_criteria" "$angle" "$wind" "$stn_p" "$stn_interval"
  done
done
