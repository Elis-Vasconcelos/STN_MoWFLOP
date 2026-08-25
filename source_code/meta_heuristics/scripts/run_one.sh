#!/usr/bin/env bash
# Roda uma única combinação (instância, algoritmo, P, intervalo, run_id) e
# organiza a saída em
# raw_results/meta_heuristics_stn/<algo>/<instance>/p<stn_p>_i<stn_interval>/<run_id>/
# (relativo à raiz do repo).
#
# Uso: ./run_one.sh <instance_id> <moead|nsga2> <run_id> [stop_criteria] [angle] [wind] [stn_p] [stn_interval]
#
# Idempotente: se <out_dir>/<instance>_<algo>_stn.csv já existir e não
# estiver vazio, pula (assume que a run já terminou) -- útil pra retomar uma
# campanha interrompida sem refazer trabalho já feito. Pra forçar uma run de
# novo, apague o diretório de saída correspondente antes de chamar de novo.
#
# Precisa ser chamado com cwd = source_code/ (é isso que batch.sh já faz) --
# os binários resolvem instâncias relativas a esse diretório, ignorando esse
# detalhe se você rodar de outro lugar.

set -euo pipefail

if [[ $# -lt 3 ]]; then
  echo "uso: $0 <instance_id> <moead|nsga2> <run_id> [stop_criteria] [angle] [wind] [stn_p] [stn_interval]" >&2
  exit 1
fi

instance="$1"
algo="$2"
run_id="$3"
stop_criteria="${4:-1000000}"
angle="${5:-30}"
wind="${6:-10}"
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

# instâncias "ns<id>" referem ao dataset real da Cazzaro/Pisinger
# ("New Sites", commitado por inteiro em wflop_instances/ -- ver
# .gitignore), distinto das instâncias sintéticas já embutidas no repo
# que reusam os mesmos números (ex.: instances/site/101 != wflop_instances/
# New Sites/101 -- números diferentes, dados diferentes, confirmado). O
# prefixo "ns" existe só pra resolver essa colisão específica -- por isso
# esse caso continua com path construído direto (sem busca), sem ambiguidade
# possível.
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
  # Fallback genérico pra qualquer outra instância nova sob wflop_instances/
  # (ex.: wflop_instances/sparse_instances/.../<name>/, o sweep de sparsity
  # pra testar a degeneração da entropia de Shannon -- ver
  # wflop_instances/README.md). Busca por um diretório com esse nome exato;
  # erra alto (não escolhe silenciosamente) se achar zero ou mais de um,
  # pra nunca introduzir uma colisão nova do jeito que "101" já tem.
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

# stn_p/stn_interval fazem parte do caminho -- sem isso, rodar a mesma
# combinação (instância, algo, run_id) de novo com um P diferente acharia
# o _stn.csv do P anterior já ali, pularia por engano (idempotência) e a
# run com o novo P nunca aconteceria de verdade
out_dir="../raw_results/meta_heuristics_stn/${algo}/${instance}/p${stn_p}_i${stn_interval}/${run_id}/"
mkdir -p "$out_dir"

stn_csv="${out_dir}${instance}_${algo}_stn.csv"
if [[ -s "$stn_csv" ]]; then
  echo "[skip] $stn_csv já existe, run considerada completa"
  exit 0
fi

# candidates.csv depende só da instância (não do algoritmo nem da run), mas
# cada run tem seu próprio out_dir -- sem isso, o binário regeneraria uma
# cópia completa (~300KB pra instância 1, escala pra ~3GB numa campanha
# inteira: 300 instâncias x 20 runs x 2 algoritmos) a cada run. Em vez
# disso, mantemos UMA cópia canônica por instância e linkamos: o
# STNLogger só escreve candidates.csv se o arquivo ainda não existir no
# out_dir, então o link symlink já resolve isso de graça.
shared_dir="../raw_results/meta_heuristics_stn/candidates"
mkdir -p "$shared_dir"
shared_candidates="$(cd "$shared_dir" && pwd)/${instance}_candidates.csv"
local_candidates="${out_dir}${instance}_${algo}_candidates.csv"

if [[ -f "$shared_candidates" ]]; then
  # link relativo, não absoluto -- o caminho absoluto do repo muda entre
  # esta máquina e o supercomputador (usuário/home diferentes), então um
  # link absoluto ficaria quebrado assim que o campanha rodasse lá
  rel_target="$(realpath --relative-to="$out_dir" "$shared_candidates")"
  ln -sf "$rel_target" "$local_candidates"
fi

echo "[start] instance=$instance algo=$algo run_id=$run_id stop_criteria=$stop_criteria stn_p=$stn_p stn_interval=$stn_interval $(date -Iseconds)"
./meta_heuristics/"$algo" "$instance" "$out_dir" "$angle" "$wind" "$run_id" "$stop_criteria" "$stn_p" "$stn_interval"
echo "[done]  instance=$instance algo=$algo run_id=$run_id $(date -Iseconds)"

# primeira run pra essa instância: promove a cópia recém-gerada a canônica,
# pra próximas runs (qualquer algoritmo) linkarem nela em vez de regerar
if [[ ! -f "$shared_candidates" && -f "$local_candidates" && ! -L "$local_candidates" ]]; then
  mv "$local_candidates" "$shared_candidates"
  rel_target="$(realpath --relative-to="$out_dir" "$shared_candidates")"
  ln -sf "$rel_target" "$local_candidates"
fi
