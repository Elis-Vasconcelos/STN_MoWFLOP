#!/usr/bin/env bash
# Variante de run_one.sh pra campanha STN10 corrigida (vento/ângulo real do
# CEC por execução, em vez do angle=30/wind=10 fixo original -- ver
# STN_MoWFLOP/source_code/meta_heuristics/wind_corrected/PLANO_CAMPANHA_CORRIGIDA.md).
#
# ÚNICA diferença em relação a run_one.sh: escreve em
# raw_results/meta_heuristics_stn_windcorrected/ em vez de
# raw_results/meta_heuristics_stn/, pra coexistir com a campanha original
# sem sobrescrever nada. angle/wind aqui não têm default (30/10) -- são
# obrigatórios, porque o objetivo inteiro dessa variante é forçar valores
# explícitos por execução (vindos de cec_wind_map.csv), nunca o default.
#
# Uso: ./run_one_windcorrected.sh <instance_id> <moead|nsga2> <run_id> <angle> <wind> [stop_criteria] [stn_p] [stn_interval]

set -euo pipefail

if [[ $# -lt 5 ]]; then
  echo "uso: $0 <instance_id> <moead|nsga2> <run_id> <angle> <wind> [stop_criteria] [stn_p] [stn_interval]" >&2
  exit 1
fi

instance="$1"
algo="$2"
run_id="$3"
angle="$4"
wind="$5"
stop_criteria="${6:-1000000}"
stn_p="${7:-10}"
stn_interval="${8:-50}"

if [[ "$algo" != "moead" && "$algo" != "nsga2" ]]; then
  echo "algo deve ser 'moead' ou 'nsga2', recebido: $algo" >&2
  exit 1
fi

if [[ ! -x "./meta_heuristics/$algo" ]]; then
  echo "./meta_heuristics/$algo não existe ou não é executável -- rode 'make' em meta_heuristics/ primeiro" >&2
  exit 1
fi

site_link="../instances/site/${instance}"

if [[ "$instance" =~ ^ns([0-9]+)$ ]]; then
  numeric_id="${BASH_REMATCH[1]}"
  if [[ ! -e "$site_link" ]]; then
    new_sites_src="../wflop_instances/New Sites/${numeric_id}"
    if [[ ! -d "$new_sites_src" ]]; then
      echo "instância $instance: '$new_sites_src' não existe -- confira se wflop_instances/ foi clonado/atualizado" >&2
      exit 1
    fi
    ln -sfn "../../wflop_instances/New Sites/${numeric_id}" "$site_link"
    echo "[setup] criado $site_link -> $new_sites_src"
  fi
elif [[ ! -e "$site_link" ]]; then
  mapfile -t matches < <(find "../wflop_instances" -mindepth 1 -maxdepth 4 -type d -name "$instance")
  if [[ ${#matches[@]} -eq 0 ]]; then
    echo "instância $instance: não encontrada em $site_link nem em ../wflop_instances/ -- confira o nome" >&2
    exit 1
  elif [[ ${#matches[@]} -gt 1 ]]; then
    echo "instância $instance: nome ambíguo, encontrado em mais de um lugar sob ../wflop_instances/:" >&2
    printf '  %s\n' "${matches[@]}" >&2
    exit 1
  fi
  rel_target="$(realpath --relative-to="../instances/site" "${matches[0]}")"
  ln -sfn "$rel_target" "$site_link"
  echo "[setup] criado $site_link -> $rel_target"
fi

# ÚNICA linha que difere de run_one.sh: raiz de saída.
out_dir="../raw_results/meta_heuristics_stn_windcorrected/${algo}/${instance}/p${stn_p}_i${stn_interval}/${run_id}/"
mkdir -p "$out_dir"

stn_csv="${out_dir}${instance}_${algo}_stn.csv"
if [[ -s "$stn_csv" ]]; then
  echo "[skip] $stn_csv já existe, run considerada completa"
  exit 0
fi

shared_dir="../raw_results/meta_heuristics_stn_windcorrected/candidates"
mkdir -p "$shared_dir"
shared_candidates="$(cd "$shared_dir" && pwd)/${instance}_candidates.csv"
local_candidates="${out_dir}${instance}_${algo}_candidates.csv"

if [[ -f "$shared_candidates" ]]; then
  rel_target="$(realpath --relative-to="$out_dir" "$shared_candidates")"
  ln -sf "$rel_target" "$local_candidates"
fi

echo "[start] instance=$instance algo=$algo run_id=$run_id angle=$angle wind=$wind stop_criteria=$stop_criteria stn_p=$stn_p stn_interval=$stn_interval $(date -Iseconds)"
./meta_heuristics/"$algo" "$instance" "$out_dir" "$angle" "$wind" "$run_id" "$stop_criteria" "$stn_p" "$stn_interval"
echo "[done]  instance=$instance algo=$algo run_id=$run_id $(date -Iseconds)"

if [[ ! -f "$shared_candidates" && -f "$local_candidates" && ! -L "$local_candidates" ]]; then
  mv "$local_candidates" "$shared_candidates"
  rel_target="$(realpath --relative-to="$out_dir" "$shared_candidates")"
  ln -sf "$rel_target" "$local_candidates"
fi
