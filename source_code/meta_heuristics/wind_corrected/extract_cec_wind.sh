#!/usr/bin/env bash
# Extrai (angle, wind) exatos que o CEC usou nas runs 1-10 de cada
# (instância, algoritmo), pra reusar nos nossos runs corrigidos.
#
# Já rodado -- cec_wind_map.csv gerado por este script vive agora em
# STNs-MOCO-MoWFLOP/raw_results/wind_corrected/, não mais aqui.
set -euo pipefail
# Caminhos relativos a este script: <STN_MoWFLOP>/source_code/meta_heuristics/wind_corrected/.
# Os logs do CEC ficam no repo irmao STNs-MOCO-MoWFLOP (mesma pasta pai que STN_MoWFLOP),
# no layout raw_results/wflopcec26/<moead|nsga2>/ns<ID>/<run>/log.txt.
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CEC_ROOT="${CEC_ROOT:-$HERE/../../../../STNs-MOCO-MoWFLOP/raw_results/wflopcec26}"
OUT="${OUT:-$HERE/cec_wind_map.csv}"
echo "instance,algo,run_id,angle,wind" > "$OUT"

instances="41 48 101 178 192 202 203 440 465 488"
for inst in $instances; do
  for our_algo in moead nsga2; do
    for cec_run in $(seq 1 10); do
      log="$CEC_ROOT/$our_algo/ns$inst/$cec_run/log.txt"
      if [[ ! -f "$log" ]]; then
        echo "MISSING: $log" >&2
        continue
      fi
      wind=$(grep -oP '(?<=Wind: )[0-9.]+' "$log" | head -1)
      angle=$(grep -oP '(?<=Angle: )[0-9.]+' "$log" | head -1)
      run_id=$((cec_run - 1))
      echo "$inst,$our_algo,$run_id,$angle,$wind" >> "$OUT"
    done
  done
done
echo "wrote $(wc -l < "$OUT") lines to $OUT"
