# Campanha no supercomputador — script, verificação e recuperação de erro

Estado atual: revisado contra `STN_MoWFLOP.pdf` §9/§10 (formato do log,
número de execuções, intervalo de gravação) — tudo abaixo já reflete isso.

## 0. As instâncias são de outro dataset

As 10 instâncias do conjunto STN (41, 48, 101, 178, 192, 202, 203, 440,
465, 488) são do dataset real da Cazzaro/Pisinger ("New Sites"), **não**
das 300 instâncias sintéticas já embutidas no repo em `instances/site/` —
os dois conjuntos coincidentemente reusam os mesmos números pra fazendas
eólicas completamente diferentes (ex.: `instances/site/41` embutida tem 1
zona/48 turbinas; a `41` real da Cazzaro/Pisinger tem 3 zonas/123
turbinas). Usar a errada roda o experimento errado sem avisar.

O dataset completo (501 instâncias, ~700MB, `wflop_instances/` na raiz do
repo) está **commitado no repo** — um `git pull` já traz tudo, nada pra
copiar/baixar à parte. Pra usar qualquer uma delas, referencia com o
prefixo `ns` (ex. `ns41`) em qualquer lugar (arquivo de instâncias, linha
de comando) — `run_one.sh` cria o link simbólico sozinho, na primeira vez
que aquela instância é usada, sem precisar de nenhum passo manual antes:

```bash
./meta_heuristics/scripts/run_one.sh ns41 moead 0 1000000 30 10 10 50
# [setup] criado ../instances/site/ns41 -> ../wflop_instances/New Sites/41
```

Dá pra misturar instância antiga (número puro) e nova (`ns<número>`) no
mesmo arquivo de instâncias, e também dá pra rodar só de um tipo ou só do
outro — testado, funciona: cada linha é só um ID passado direto pro
binário, sem validação de formato.

`instances_stn10.txt` já lista as 10 do conjunto STN como `ns41`, `ns48`,
etc.

### Rodar todas as instâncias de um dataset só (se precisar mais pra frente)

```bash
cd source_code
seq 1 300 > instances_all_bundled.txt              # as 300 sintéticas embutidas
seq 0 500 | sed 's/^/ns/' > instances_all_new.txt  # as 501 da Cazzaro/Pisinger (New Sites)
```
Cada arquivo é independente — usa um ou outro dependendo de qual dataset
quer rodar por inteiro.

**Cuidado com concorrência**: `batch.sh` lança um processo por linha do
arquivo, todos ao mesmo tempo, sem limite. Pro conjunto de 10 instâncias
isso é ótimo. Pra rodar um dataset inteiro (300 ou 501 instâncias) isso
tentaria lançar 300 ou 501 processos simultâneos — o supercomputador tem
60 núcleos (AMD EPYC 7763, confirmado), então isso estouraria bastante.
Quebra em lotes menores antes de tentar (me chama que ajudo a montar
quando for a hora).

## 1. Onde tá o script e como funciona

Três scripts, em `source_code/meta_heuristics/scripts/`, empilhados um em cima do outro:

**`run_one.sh`** — a unidade mínima: roda **uma única combinação** (instância, algoritmo, `run_id`, P, intervalo). Faz:
- checa se `moead`/`nsga2` foi compilado (`make` em `meta_heuristics/`)
- monta o diretório de saída: `raw_results/meta_heuristics_stn/<algo>/<instância>/p<P>_i<intervalo>/<run_id>/`
- **pula se já rodou** (se `<instância>_<algo>_stn.csv` já existe e não tá vazio ali) — isso é o que permite retomar sem refazer trabalho
- cuida do `candidates.csv` (tabela instância→coordenadas): mantém UMA cópia por instância e linka via symlink nas outras runs, em vez de duplicar ~300KB por run
- só então chama o binário de verdade: `./meta_heuristics/moead <instância> <out_dir> <angle> <wind> <run_id> <stop_criteria> <P> <intervalo>`

**`run_instance.sh`** — pra **uma instância**, roda todas as combinações de algoritmo × `run_id` chamando `run_one.sh` pra cada uma, em sequência.

**`batch.sh`** — o que vocês realmente vão chamar. Lê um arquivo de instâncias (padrão: `instances_stn10.txt`) e lança, via `nohup`, **um processo por instância**, cada um rodando `run_instance.sh` daquela instância. Todos em paralelo, sem esperar um terminar pro outro começar.

Uso:
```bash
cd source_code
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 10 1000000 30 10 10 50
#                                  ^instâncias                ^algos       ^runs ^stop  ^ang^wind ^P ^intervalo
```
Isso dispara 10 processos (um por instância), cada um rodando moead+nsga2 × **10 runs** (conforme §10.2 do PDF: "Execute cada par 10 vezes"), sequencialmente dentro do processo daquela instância.

**Pra testar os 3 valores de P (10/50/100)**: chama `batch.sh` de novo, uma vez por P, com o **intervalo fixo em 50** (§10.5: k≈50, ~100-300 pontos por trajetória — não usar 1, isso produziria trajetórias de até 10.000 nós). Não colide, cada P vai pra uma pasta e log diferentes (`p10_i50/`, `p50_i50/`, `p100_i50/`). Como `batch.sh` só lança processos `nohup` em background e retorna na hora, chamar as 3 vezes seguidas roda os 3 P's **concorrentemente**:

```bash
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 10 1000000 30 10 10  50
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 10 1000000 30 10 50  50
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 10 1000000 30 10 100 50
```
`batch.sh` lança um processo **por instância** (cada um rodando seus `{algos} × num_runs` combinações em sequência por dentro), então 3 chamadas = 10 instâncias × 3 P's = 30 processos simultâneos, cada um usando um núcleo — bem dentro dos 60 núcleos do supercomputador. Confere que nada mais tá rodando lá antes (`uptime`, `top`, `who`) já que a máquina é compartilhada.

## 2. Formato do CSV (`<instância>_<algo>_stn.csv`)

Colunas: `algorithm,instance,run_id,vector_id,generation,iteration,f_cost,f_power,weight1,weight2,occupied`.

`algorithm`/`instance` são colunas literais (não só no nome do arquivo) e
`iteration` é um índice sequencial de gravação (0,1,2,...) — os campos
mínimos que §10.2 do PDF exige. `generation` continua guardando a geração
bruta (0, 50, 100, ... se intervalo=50) além do mínimo exigido, útil pra
depuração.

## 3. Como verificar a execução

Cada instância escreve seu próprio log:
```bash
tail -f source_code/logs/*.log            # acompanha tudo em tempo real
tail -f source_code/logs/ns41_p10_i50.log # só a instância 41 nesse P
```
O `batch.sh` imprime o PID de cada processo lançado na hora que roda — guarda isso ou confere depois:
```bash
ps aux | grep meta_heuristics    # todos os binários moead/nsga2 rodando agora
ps -p <pid>                      # um processo específico ainda vivo?
```
Dentro de cada run, `infoRun.txt` mostra o progresso geração a geração:
```bash
tail -5 raw_results/meta_heuristics_stn/moead/ns41/p10_i50/0/infoRun.txt
# Generation 342 | Revalues: 68420 | GridSize: 19
```
`Revalues` sobe até bater o `stop_criteria` (1000000 na campanha real) — dá pra estimar quanto falta.

Uma visão geral rápida de quanto já terminou:
```bash
find raw_results/meta_heuristics_stn -name "*_stn.csv" | wc -l
# compara com o total esperado: 10 instâncias x 2 algos x 10 runs x 3 P's = 600
```

## 4. Se der algum BO

**A resposta curta: relança `batch.sh` de novo, exatamente com os mesmos argumentos.** Ele é idempotente — qualquer combinação cujo `_stn.csv` já existe é pulada automaticamente, só o que faltou (ou travou pela metade) roda de novo. Não precisa descobrir manualmente o que já terminou.

Casos específicos:
- **Processo morreu no meio (sessão caiu, nó reiniciou, etc.)**: só relança `batch.sh`. O que tinha `_stn.csv` completo fica intocado; o resto roda.
- **Uma run travou/corrompeu pela metade** (`_stn.csv` existe mas tá incompleto ou zoado) — isso o skip automático não pega, porque ele só checa "o arquivo existe e não tá vazio", não valida o conteúdo. Apaga a pasta daquela combinação específica e relança:
  ```bash
  rm -rf raw_results/meta_heuristics_stn/moead/ns41/p10_i50/3/   # ex.: instância 41, moead, P=10, run 3
  ./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 10 1000000 30 10 10 50
  ```
- **Quer forçar tudo de novo do zero** (não deveria ser preciso, mas): `rm -rf raw_results/meta_heuristics_stn/` inteiro antes de relançar.
- **Erro de compilação/binário não existe**: `cd source_code/meta_heuristics && make rebuild` antes de tentar de novo.

## 5. Teste antes da campanha real (fazer sempre primeiro)

**Importante**: `instances/` e `raw_results/` são irmãos de `source_code/`
(não ficam dentro dele) -- rodando com cwd = `source_code/`, os caminhos
precisam do `../` na frente. `logs/` é a exceção, essa sim fica direto
dentro de `source_code/`. E o `tail` desse servidor não aceita `-N`
(precisa `-n N`).

**Roda em blocos separados, não cole tudo de uma vez** -- se colar o
bloco inteiro (incluindo a limpeza do passo 7) de uma vez, ele executa
até o fim sem parar pra você conferir nada no meio, e a limpeza pode
rodar antes das execuções em background terminarem de verdade.

```bash
# 1. código + dados (git pull já traz tudo, incluindo wflop_instances/)
git pull
cd source_code/meta_heuristics
make rebuild
cd ..
```

```bash
# 2. confere que a máquina tá livre (é compartilhada)
uptime
top -bn1 | head -15
who
```

```bash
# 3. teste real pequeno -- cria o symlink ns41 sozinho, roda as 10
#    instâncias reais x 2 algoritmos, stop_criteria baixo só pra validar
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 1 2000 30 10 5 500
sleep 30
```

```bash
# 4. confere -- SÓ depois que o passo 3 realmente terminou
tail -n 30 logs/ns41_p5_i500.log
grep -iE "error|erro" logs/*.log
ls -la ../instances/site/ns41
find ../raw_results/meta_heuristics_stn -name "*_stn.csv" | wc -l   # espera 20 (10 instâncias x 2 algos)
head -2 ../raw_results/meta_heuristics_stn/moead/ns41/p5_i500/0/ns41_moead_stn.csv
```

```bash
# 5. só limpa depois de conferir que o passo 4 tá tudo certo
rm -rf ../raw_results/meta_heuristics_stn logs
```

