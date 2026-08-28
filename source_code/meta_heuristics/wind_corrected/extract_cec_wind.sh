#!/usr/bin/env bash
# Extrai (angle, wind) exatos que o CEC usou nas runs 1-10 de cada
# (instância, algoritmo), pra reusar nos nossos runs corrigidos.
set -euo pipefail
CEC_ROOT="/home/elis/Projects/TCC/external_pf/wflopcec26/algorithms_raw_results"
OUT="/home/elis/Projects/TCC/STN_MoWFLOP/tmp_demo/wind_corrected/cec_wind_map.csv"
echo "instance,algo,run_id,angle,wind" > "$OUT"

instances="41 48 101 178 192 202 203 440 465 488"
for inst in $instances; do
  for algo_pair in "MOEAD:moead" "NSGA2:nsga2"; do
    cec_algo="${algo_pair%%:*}"
    our_algo="${algo_pair##*:}"
    for cec_run in $(seq 1 10); do
      log="$CEC_ROOT/$cec_algo/$inst/$cec_run/log.txt"
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
