# Plano: campanha STN10 corrigida (vento/ângulo real do CEC), pra revisão antes de rodar

Status: **plano, nada foi executado no supercomputador ainda.** Já
prontos, localmente, read-only: `cec_wind_map.csv` (mapeamento de
vento/ângulo, 200 combinações únicas) e `run_one_windcorrected.sh`
(script de execução, cópia de `run_one.sh` com output pra pasta nova e
`angle`/`wind` obrigatórios em vez de default 30/10). **Escopo:
paridade completa com a campanha original — 600 combinações (200 × 3
valores de P), não só 200** (ver seção 4). Esse documento existe pra
você revisar o desenho experimental antes de eu rodar qualquer coisa —
inclusive se essa correção deve se estender ao RQ1 (esparsas), ver seção
4b do `talking_points_reuniao_26-08.md`, ainda em aberto pro professor.

## 1. Motivação (resumo — detalhes completos em `talking_points_reuniao_26-08.md`)

A campanha STN10 original (`ns41`, `ns48`, `ns101`, `ns178`, `ns192`,
`ns202`, `ns203`, `ns440`, `ns465`, `ns488`) rodou com `angle=30, wind=10`
fixo pra **todas** as execuções. O CEC2026 usou um `(angle, wind)`
diferente sorteado da rosa dos ventos **por execução**. Provamos
empiricamente (`ns101`/`ns178`, seção 6 do talking points) que igualar
esse cenário fecha o gap quase por completo. Essa campanha replica a
correção pras outras 8 instâncias que faltam, pra ter o conjunto STN10
inteiro corrigido.

## 2. Decisão de design: reusar os valores exatos do CEC, não reimplementar o sorteio

**Não vamos** reativar a lógica de sorteio ponderado pela rosa dos ventos
(o bloco comentado em `instance_info.cpp`) e sortear nossos próprios
valores. Em vez disso, **reusamos literalmente o `(angle, wind)` que o
CEC já usou** em cada uma das runs 1-10 de cada (instância, algoritmo) —
extraído direto dos `log.txt` deles.

**Por quê**: reimplementar o sorteio do zero introduz risco de um bug de
amostragem *novo* (ponderação errada, arredondamento, mapeamento errado
de qual arquivo de rosa dos ventos pertence a qual instância — o CEC nem
deixa isso explícito no código deles, só um `windFile` hardcoded que
também está comentado). Reusar os valores exatos deles é **estritamente
mais rigoroso**: comparação run-a-run, sem nenhuma camada de incerteza
nova. É exatamente o mesmo método que já validamos pra `ns101`/`ns178`.

## 3. O que já foi gerado (local, read-only)

`STN_MoWFLOP/tmp_demo/wind_corrected/extract_cec_wind.sh` — lê
`external_pf/wflopcec26/algorithms_raw_results/{MOEAD,NSGA2}/<inst>/
<1..10>/log.txt` pras 10 instâncias, extrai `Wind:`/`Angle:` de cada uma,
mapeia run CEC 1→`run_id=0`, ..., run CEC 10→`run_id=9`. Saída:
`cec_wind_map.csv` (200 linhas de dados, `instance,algo,run_id,angle,wind`).
Primeiras linhas:
```
instance,algo,run_id,angle,wind
41,moead,0,330.000000000000,13.000000000000
41,moead,1,0.000000000000,2.000000000000
41,moead,2,330.000000000000,3.000000000000
```

## 4. Escopo — paridade completa com a campanha original (600, não 200)

**Atualizado**: a campanha original não é só 200 combinações, é
**10 instâncias × 2 algoritmos × 10 execuções × 3 valores de P (10/50/100)
= 600** (confirmado em `COMO_RODAR_CAMPANHA.md`, mesma contagem que
`find raw_results/meta_heuristics_stn -name '*_stn.csv' | wc -l` espera
no original). Pra ter paridade de verdade com o que já existe, a
correção precisa cobrir os 3 P's também, não só P=10.

**P não afeta `(angle, wind)`** — é só a densidade de vetores observadores
da STN (instrumento externo, per `STN_MoWFLOP.pdf` §10.1: "independente
dos vetores de busca do MOEA/D"), não influencia a busca em si. Então
`cec_wind_map.csv` (200 linhas, uma por combinação instância×algo×run_id)
já serve pros 3 P's — cada linha é reusada 3 vezes, uma por P, com o
mesmo `(angle, wind)`. `stop_criteria=10^6` igual à campanha original e
ao CEC.

## 5. Onde os resultados vão

**Pasta nova, não sobrescreve nada**: proponho
`raw_results/meta_heuristics_stn_windcorrected/` (mesma estrutura interna
de `raw_results/meta_heuristics_stn/`, só a raiz diferente) — assim a
campanha original (`angle=30/wind=10` fixo) e essa corrigida coexistem,
dá pra comparar as duas diretamente, e nada do que já existe é tocado.

## 6. Tamanho das instâncias (todas pequenas, nada como as esparsas r1e-05)

| instância | posições | referência |
|---|---|---|
| ns48 | 4.229 | |
| ns192 | 4.113 | |
| ns178 | 3.963 | (já medida) |
| ns101 | 4.727 | (já medida) |
| ns488 | 6.610 | |
| ns203 | 6.734 | |
| ns202 | 8.797 | |
| ns41 | 13.248 | |
| ns465 | 14.709 | |
| ns440 | 28.050 | (maior, ~6x ns101) |

## 7. Estimativa de tempo

Calibrado nas medições reais de `ns101` (4.727 posições: moead ~5,5min,
nsga2 ~16min por execução — feito hoje). Escalando linearmente por
número de posições (O(|P|) por avaliação, confirmado no código):

- **CPU-tempo total agregado** (soma das 600 combinações — 200 únicas ×
  3 P's, execução sequencial hipotética): ≈ **216h** (72h × 3, já que
  cada P é uma execução completa independente do algoritmo, não reusa
  trajetória).
- **Wall-clock com paralelismo**: com `-P 30`, ≈ 216h/30 ≈ **7-10h**
  (contando overhead de fila/contenção com a campanha r1e-04/r1e-05 que
  ainda tá rodando). O piso de uma execução única (`ns440` nsga2, ~1,5h)
  continua o mesmo, não muda com mais combinações.
- **Concorrência sugerida**: mesmo `-P 30` de antes — processos pequenos
  (<20MB cada), ~380GB livre de RAM, sem risco de memória mesmo somando
  às 100 já rodando. Isso dá **~7-10h de wall-clock**.
- **Urgência real**: são 20:58 agora (25/08), a reunião é 8h de amanhã —
  **~11h de janela**. Com estimativa de 7-10h, a margem de segurança é de
  só 1-4h. **Precisa lançar assim que aprovado, sem esperar** — não dá
  pra tratar isso como "roda de manhã antes da reunião".

## 8. Comandos que seriam rodados (ainda não executados)

`run_one_windcorrected.sh` já existe (`source_code/meta_heuristics/
scripts/`) — uso: `<instance> <moead|nsga2> <run_id> <angle> <wind>
[stop_criteria] [stn_p] [stn_interval]`. Só essa variante escreve em
`raw_results/meta_heuristics_stn_windcorrected/` (pasta nova); `angle`/
`wind` são obrigatórios (sem default 30/10), forçando vir do mapa do CEC
sempre.

```bash
# 1. transferir cec_wind_map.csv e o script novo pro supercomputador
scp -P 2004 tmp_demo/wind_corrected/cec_wind_map.csv \
  gpu@200.128.51.124:~/STN_MoWFLOP/source_code/
scp -P 2004 source_code/meta_heuristics/scripts/run_one_windcorrected.sh \
  gpu@200.128.51.124:~/STN_MoWFLOP/source_code/meta_heuristics/scripts/

# 2. gerar a lista de combinações com angle/wind por linha, UMA vez por
#    valor de P (10/50/100) -- reusa o mesmo (angle,wind) por linha nas
#    3, já que P não afeta o cenário de vento
cd ~/STN_MoWFLOP/source_code
> /tmp/windcorrected_combos.txt
for p in 10 50 100; do
  tail -n +2 cec_wind_map.csv | awk -v P="$p" -F, '{print "ns"$1, $2, $3, $4, $5, P}' >> /tmp/windcorrected_combos.txt
done
wc -l /tmp/windcorrected_combos.txt   # espera 600

# 3. lançar em fila com paralelismo limitado (P=30 núcleos concorrentes --
#    não confundir com o "P"/stn_p da STN, mesmo símbolo, coisas diferentes),
#    saída pra pasta nova
setsid nohup xargs -a /tmp/windcorrected_combos.txt -n 6 -P 30 \
  bash -c './meta_heuristics/scripts/run_one_windcorrected.sh "$0" "$1" "$2" "$3" "$4" 1000000 "$5" 50' \
  > logs/windcorrected_launcher.log 2>&1 < /dev/null &
disown
```

## 9. Como acompanhar o progresso (uma vez rodando)

```bash
# combinações completas
find ~/STN_MoWFLOP/raw_results/meta_heuristics_stn_windcorrected -name '*_stn.csv' | wc -l
# espera 600 no final (mesma contagem que a campanha original)

# por P (10/50/100)
find ~/STN_MoWFLOP/raw_results/meta_heuristics_stn_windcorrected -name '*_stn.csv' | grep -oE 'p[0-9]+_i50' | sort | uniq -c
# espera 200 em cada um de p10_i50, p50_i50, p100_i50

# por instância/algoritmo (soma dos 3 P's, espera 30 cada: 10 runs x 3 P's)
find ~/STN_MoWFLOP/raw_results/meta_heuristics_stn_windcorrected -name '*_stn.csv' | sed -E 's#.*/(moead|nsga2)/(ns[0-9]+)/.*#\1 \2#' | sort | uniq -c

# erros
grep -liE 'error|erro' ~/STN_MoWFLOP/source_code/logs/windcorrected_launcher.log

# progresso de uma run específica em andamento
tail -1 ~/STN_MoWFLOP/raw_results/meta_heuristics_stn_windcorrected/<algo>/<inst>/p10_i50/<run_id>/infoRun.txt
```

## 10. Depois de rodar

Gerar os 8 gráficos que faltam (mesmo script de
`STNs-MOCO-MoWFLOP/scripts/mowflop_comparison/plot_corrected_comparison.py`,
só trocando a lista de instâncias de `["101","178"]` pras 10 completas) —
já temos o padrão pronto, é só reaplicar.
