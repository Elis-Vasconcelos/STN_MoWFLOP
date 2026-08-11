

# Multi-Objective Offshore Wind Farm Layout Optimization (Mo-WFLOP)  

This repository contains the implementation, benchmark instances, and experimental data for the paper:  

> **"On the use of Pareto-based Features in Meta-Learning for Multi-Objective Offshore Wind Farm Layout Optimization"**  

## 📄 Abstract  

The Multi-objective Wind Farm Layout Optimization Problem (MoWFLOP) seeks the optimal turbine placement in offshore wind farms, where construct cost minimization and power generation maximization are simultaneously optimized. As MoWFLOP belongs to NP-hard, previous work has focused on metaheuristic approaches to compute high-quality solutions. However, metaheuristics' results can vary across wind farm instances, making manual algorithm selection impractical. This paper investigates the use of Pareto-based meta-features to guide a landscape-aware method based on meta-learning for automated metaheuristic selection applied to MoWFLOP. To our knowledge, no such investigation exists for this problem. We explore 30 different sampling configurations to extract Pareto-based meta-features from 300 wind farms, balancing computational cost with feature informativeness. We build a dataset for each configuration and train Random Forest models to predict the best-performing algorithm for unseen instances. The analyses comprised the metaheuristics' results regarding Pareto-compliant indicators, the impact of sampling configurations on feature values, and the meta-learning performance for automated metaheuristic selection.

## 📂 Repository Structure  
```
├── /final_dataset/                 # Refined epsilon dataset for each parameter and for all instances
├── /Instances/                 
│   ├── /instance_generator/        # Files for creating additional wind farm sites
│   ├── /site/                      # All Wind farm sites used in the study
│   ├── /wind/                      # Wind files and their probabilities
│   └── /wtg/                       # Power generated per turbine type for specific wind velocities
├── /raw_results/               
│   ├── /meta_features/             # Raw features extracted
│   ├── /meta_heuristics/           # Raw execution data of multi-objective algorithms
│   └── /stat_analysis/             # Raw statistical data for result comparison
├── /refined_results/           
│   ├── /statistical_tables/        # Refined statistical tables for each instance and each statistical measure 
│   └── /meta_learning/             # Final results from the meta-learning stage:
│       ├── features_importance/    # Contains the feature importance of each ML model
│       ├── figures/                # Visualizations showing feature importance
│       ├── logs/                   # Logs of each model run
│       └── /models/                # Trained models (theoretical models + executable formats)
└── /source_code/
    ├── /meta_features_extraction/  # Code used for features extraction
    ├── /meta_heuristics/           # Code of multi-objective algorithms
    └── /metalearning/              # Code that executes the full meta-learning process, including:
                                        # - Construction of ML models (theoretical and executable)
                                        # - Calculation of performance metrics (merit and regression-based)
                                        # - Dataset construction for model training
                                        # - Generation of figures
```
## ▶️ Running the metaheuristics (MOEA/D, NSGA-II)

`source_code/meta_heuristics/` contains the C++ MOEA/D and NSGA-II
implementations used by the paper (population size 100, 10 MOEA/D
neighbors, both hardcoded in `headers/globals.h` / `moead.cpp`). There is
no CoMOLS/D implementation in this repository.

### Build

```bash
cd source_code/meta_heuristics
make            # builds both ./nsga2 and ./moead
```

`make nsga2` / `make moead` build just one of the two; `make clean` /
`make rebuild` act on both. (Originally the `Makefile` only had a single
`nsga2.cpp`-hardcoded target — it's been extended with a proper `moead`
target, sharing the same `SRC_FILES`/flags, since both algorithm
implementations already live in the same source tree.)

### Run

**Run from `source_code/`, not from `meta_heuristics/`.** Instance loading
is hardcoded to look for `../instances/site/<id>/` relative to the current
working directory (`get_instance_info` in `instance_info.cpp` ignores the
`root_folder` CLI arg for this) — `..` only resolves to `instances/`
correctly when the process is launched from `source_code/`:

```bash
cd source_code   # NOT source_code/meta_heuristics
./meta_heuristics/moead <instance_id> [output_dir] [angle] [wind] [run_id] [stop_criteria] [stn_p] [stn_interval]
./meta_heuristics/nsga2 <instance_id> [output_dir] [angle] [wind] [run_id] [stop_criteria] [stn_p] [stn_interval]
```

Arguments are strictly positional — to reach `run_id` you must also pass
`angle` and `wind` (their defaults are `30` and `10`).

- `<instance_id>` — a directory name under `instances/site/` (e.g. `1`,
  `172`, `A`; the repo ships 300 numeric instances, no lettered A–J ones).
- `[output_dir]` — where output files are written; defaults to the current
  directory (`./`). Pass a trailing slash, e.g. `results/1/`. **Two runs
  sharing an output dir at the same time will corrupt each other's
  `infoRun.txt`** (both processes truncate-and-append the same path) —
  give concurrent runs distinct output dirs.
- `[angle]` / `[wind]` — wind angle in degrees and speed in m/s; default
  30° / 10 m/s.
- `[run_id]` — integer stamped on every row of the STN trajectory CSV, so
  independent repetitions stay distinguishable once their CSVs are
  concatenated. Defaults to `0`.
- `[stop_criteria]` — number of solution evaluations to stop after;
  defaults to 1,000,000. Lower it for a quick smoke test.
- `[stn_p]` — number of STN weight vectors (`STN_LOGGER_NUM_VECTORS`);
  defaults to `10`. Runtime arg, not a compile-time constant, so sweeping
  P (e.g. 10/50/100) doesn't require rebuilding.
- `[stn_interval]` — STN sampling interval in generations
  (`STN_LOGGER_INTERVAL`); defaults to `50`.

On a modern desktop core, a full 1,000,000-evaluation run of a
75-mobile-turbine instance (e.g. instance `1`) took **~25 minutes**
end-to-end in testing; larger instances take longer. The `Run time:` line
printed to stdout is just a static header (printed *before* the run
starts, not an actual timer) — prefix the command with `time` yourself if
you want wall-clock time.

### Output

All output files land in `[output_dir]` (default cwd):

| File | Written | Contents |
|---|---|---|
| `infoRun.txt` | continuously, one line per generation | `Generation <g> \| Revalues: <n> \| GridSize: <archive size>` — progress log |
| `<instance>_<algo>_<n>.txt` | every 100,000 evaluations (`n` = 100000, 200000, …, 1000000) | current non-dominated archive snapshot, one solution per line: `<cost> <power>` |
| `<instance>_<algo>_layout.txt` | once, at the final checkpoint (`n` = 1000000) | turbine coordinates of every solution in the final non-dominated archive: one `<x> <y>` line per turbine, solutions separated by a blank line |
| `<instance>_<algo>_stn.csv` | continuously, per generation | Search Trajectory Network raw data — see below |
| `<instance>_<algo>_candidates.csv` | once, at start | `global_index,zone,zone_index,x,y` — decodes the `occupied` column of the STN CSV into coordinates |

`<algo>` is `moead` or `nsga2` depending on which binary you ran. `cost` is
minimized, `power` is maximized — both printed as positive numbers in the
snapshot files.

### Search Trajectory Network logging

`<instance>_<algo>_stn.csv` records, every `STN_LOGGER_INTERVAL`
generations, the representative solution of each of `STN_LOGGER_NUM_VECTORS`
weight vectors — the raw data a Search Trajectory Network is built from
later. Both are **runtime CLI args** (`stn_p`/`stn_interval`, see "Run"
above), defaulting to `10`/`50` when omitted — `headers/globals.h` only
declares them (`extern`); the actual values are set per-run in
`moead.cpp`/`nsga2.cpp`'s `main()`, alongside `SIZE_OF_POPULATION` (which
*is* still a compile-time constant). Columns: `run_id,vector_id,generation,f_cost,f_power,
weight1,weight2,occupied`, where `weight1`/`weight2` are that row's vector's
literal weights (redundant with `vector_id`, since `build_weight_vector` is
deterministic, but avoids a join step before feeding this into `create.R`)
and `occupied` is the space-separated list of global candidate indices
holding a turbine (sorted ascending), decodable through
`<instance>_<algo>_candidates.csv`. `f_cost` is positive and minimized;
`f_power` is positive and maximized.

The occupation-grid signature that partitions the search space into STN
nodes is deliberately *not* computed here — its cell size is a
post-processing choice, and logging raw occupied positions lets any cell
size be recomputed later without rerunning the algorithm.

The STN's `STN_LOGGER_NUM_VECTORS` weight vectors are generated the same
way as MOEA/D's own decomposition (`build_weight_vector`, uniform between
`(1,0)` and `(0,1)`), but are a separate, smaller set — MOEA/D's own
`SIZE_OF_POPULATION` internal vectors are unaffected and still drive its
mating/replacement. Because of that, `population[j]` in MOEA/D is *not*
aligned with the STN's vector `j`: every generation logged, both
algorithms call `select_representatives` (`stn_logger.h`) to pick, for
each STN vector, the population member minimising `calculate_gte` against
it and the current `z_point`. This makes both algorithms observed the
same way, at the STN's coarser resolution — an external instrumentation
layer imposed by the experimenter, not part of either algorithm, and
should be declared as such in any methodology write-up.

Sampling every `STN_LOGGER_INTERVAL`-th generation means the very last
generation of a run is only captured in the log if it happens to be a
multiple of the interval — the trajectory's true terminal state can be
missed. If exact terminal nodes matter for a metric (e.g. `n_end` in the
STN reference model), account for this in post-processing or force one
extra log call after the loop.

After a short local run (pass a small `stop_criteria`, e.g. `20000`, as the
6th CLI arg — see "Run" above), check the output. There's no automated
validator script committed yet (`validate_stn_log.py` was referenced by an
older draft of this doc but never actually added to the repo) — use these
manual checks instead, run from `source_code/`:

```bash
file="<output_dir>/<instance>_<algo>_stn.csv"

# every row should have tau_total turbines occupied (column 8; columns
# shifted right by weight1/weight2)
awk -F',' 'NR>1{n=split($8,a," "); if(n!=EXPECTED_TAU) print "line " NR ": " n " turbines"}' "$file"

# sampled generations should be 0, STN_LOGGER_INTERVAL, 2*STN_LOGGER_INTERVAL, ...
awk -F',' 'NR>1{print $3}' "$file" | sort -n -u

# no vector_id should have a repeated or out-of-order generation
# (BEGIN{prev_v=-1} avoids a false positive on vector_id 0: an
# uninitialized awk variable compares equal to "0" by default, which
# collides with vector 0 specifically)
awk -F',' 'NR>1{print $2","$3}' "$file" | sort -t, -k1,1n -k2,2n | \
  awk -F',' 'BEGIN{prev_v=-1} prev_v==$1 && $2<=prev_g {print "order broken in vector " $1} {prev_v=$1; prev_g=$2}'
```
No output beyond headers on any of the three means the log is consistent.

### Running a campaign

`meta_heuristics/scripts/run_one.sh` runs exactly one `(instance, algo,
stn_p, stn_interval, run_id)` combination and organizes its output under
`raw_results/meta_heuristics_stn/<algo>/<instance>/p<stn_p>_i<stn_interval>/<run_id>/`:

```bash
cd source_code
./meta_heuristics/scripts/run_one.sh 41 moead 0 1000000 30 10 10 50
```

- Idempotent: re-running skips a combination whose `_stn.csv` already
  exists — safe to interrupt and resume. `stn_p`/`stn_interval` are part of
  the output path specifically so this stays safe: re-running the same
  `(instance, algo, run_id)` with a *different* P (e.g. sweeping 10/50/100
  as requested) lands in a different directory and actually runs, instead
  of finding the previous P's `_stn.csv` already there and silently
  skipping.
- `_candidates.csv` is instance-only, so `run_one.sh` keeps a single
  canonical copy per instance under `raw_results/meta_heuristics_stn/
  candidates/` and symlinks (relative, portable across machines) it into
  every run's directory, instead of letting the ~290KB table get
  regenerated in every output directory a full campaign creates.
- This is also the unit to call directly from a real HPC job
  scheduler if one becomes available (one job/task per combination) —
  there's no scheduler-specific logic baked into it.

For the actual supercomputer run, use `batch.sh` below rather than looping
`run_one.sh` yourself — it's the fan-out layer.

### STN experiment instances (Cazzaro/Pisinger "New Sites" dataset)

The instance set selected for the STN experiments (41, 48, 101, 178, 192,
202, 203, 440, 465, 488) refers to the **Cazzaro/Pisinger real-world
instance set** ("New Sites"), **not** the 300 synthetic instances already
bundled under `instances/site/`. The two sets happen to reuse the same
numeric IDs for completely different wind farms (e.g. bundled
`instances/site/41` has 1 zone/48 turbines; New Sites' instance `41` has
3 zones/123 turbines) — using the wrong one silently runs the wrong
experiment.

The full New Sites collection (501 instances, ~700MB, `wflop_instances/`
at the repo root) is committed **in full**, not just this instance set's 10 —
so `git push`/`git pull` alone gets everything onto the supercomputer, no
separate data-transfer step, and running any of the other 490+ instances
later needs no re-setup.

`instance_info.cpp` resolves instances through a fixed path
(`../instances/site/<instance>/...`), so pointing the binaries at
`wflop_instances/` directly isn't possible without a C++ change. Instead,
referencing an instance as **`ns<id>`** anywhere (an instances file,
directly on the CLI) makes `run_one.sh` auto-create the symlink
`instances/site/ns<id>` → `wflop_instances/New Sites/<id>` the first time
that instance is actually used — no separate setup step. The `ns` prefix
just avoids colliding with the bundled instance of the same number; a
plain numeric ID still means the bundled one, unaffected. These symlinks
aren't committed themselves (machine-specific, regenerated automatically
wherever `wflop_instances/` exists — see `.gitignore`).

An instances file (what `batch.sh` reads) can freely mix plain and
`ns`-prefixed IDs on separate lines — nothing validates the format, each
line is just passed straight through, and both kinds can appear in the
same run. `instances_stn10.txt` lists the STN instance set as
`ns`-prefixed IDs.

To run every instance of one dataset (not both mixed) — e.g. if more of
these 500+ real-world instances end up needed later:

```bash
seq 1 300 > instances_all_bundled.txt              # all 300 bundled synthetic instances
seq 0 500 | sed 's/^/ns/' > instances_all_new.txt  # all 501 Cazzaro/Pisinger instances
```

**Concurrency warning**: `batch.sh` launches one process per line, all at
once, with no cap — fine for 10 instances, but 300 or 501 concurrent
processes will likely exceed available cores on any real machine. Check
core availability and consider chunking into smaller batches before
running a full-dataset instances file.

### Running the STN instance batch with `nohup`

`meta_heuristics/scripts/batch.sh` + `run_instance.sh` adapt Gustavo/João's
`MO_WFLOP-experiment-runner` two-file split (`scripts/batch.sh` +
`scripts/main.sh`) directly — same idiom, same layering:

- `run_instance.sh <instance> [algos] [num_runs] ...` — one instance, loops
  over `{algos} × run_id 0..num_runs-1` calling `run_one.sh` for each.
  Equivalent to their `main.sh` (which loops runs calling `comolsd.sh`).
- `batch.sh [instances_file] [algos] [num_runs] ...` — one
  `nohup run_instance.sh <instance> ... &> logs/<instance>_p<stn_p>_i<stn_interval>.log &`
  per instance, no scheduler, no concurrency cap, no final `wait` — the
  exact pattern their `batch.sh` uses (`nohup "$script" $batch &> logfile
  &`) to fan out across instances. The only real differences: the instance
  list comes from a file instead of hardcoded literals (defaults to
  `instances_stn10.txt`, the 10-instance STN set: 41, 48, 101, 178, 192,
  202, 203, 440, 465, 488), and it's one process per instance rather than
  per ~10-instance chunk (their original use case had 300+ instances to
  distribute; this instance set only has 10).

```bash
cd source_code
./meta_heuristics/scripts/batch.sh                      # defaults: instances_stn10.txt, moead+nsga2, 20 runs
./meta_heuristics/scripts/batch.sh my_instances.txt "moead nsga2" 20 1000000 30 10 100 10
```

To sweep P (e.g. 10/50/100) with the interval held fixed, call `batch.sh`
directly, once per P value — each lands in its own output directory and
log file (see above), nothing gets skipped or overwritten between them.
Since `batch.sh` only launches detached `nohup` background jobs and
returns immediately, calling it 3 times back to back like this actually
runs the 3 P's **concurrently**, not sequentially:

```bash
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 20 1000000 30 10 10  1
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 20 1000000 30 10 50  1
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 20 1000000 30 10 100 1
```

This needs enough free cores to run all of it at once (10 instances × 2
algorithms × 3 P values = 60 concurrent single-threaded processes) — check
availability first (`nproc`, `uptime`, `top`) if the machine is shared.
Running fewer P values per invocation, or waiting for one to finish before
launching the next, trades wall-clock time for a smaller concurrent
footprint.

- Logs land in `source_code/logs/<instance>_p<stn_p>_i<stn_interval>.log`;
  check progress with `tail -f logs/*.log` or `ps -p <pid>` (PIDs are
  printed when `batch.sh` launches).
- Composes with `run_one.sh`'s idempotency — safe to re-launch `batch.sh`
  to resume after an interruption, already-complete runs are skipped.
- `run_one.sh`'s idempotent skip-if-done check and its `_candidates.csv`
  dedup/symlink logic have no equivalent in Gustavo/João's scripts — their
  `comolsd` binary doesn't write a shared per-instance file the way our
  `STNLogger` does, so there was nothing to adapt there; both are
  necessary additions specific to our own logger's behavior, not
  deviations from their pattern.

## 👥 Authors  
| Name | Affiliation | Contact |  
|------|-------------|---------|  
| **Gustavo J. N. Silva** | Federal University of Bahia, Institute of Computing | [gustavojorge080@gmail.com](mailto:gustavojorge080@gmail.com) |  
| **João G. Lofiego** | Federal University of Bahia, Institute of Computing | [joao.lofiego@ufba.br](mailto:joao.lofiego@ufba.br) |
| **Islame F. C. Fernandes** | Federal University of Bahia, Institute of Computing | [islame.felipe@ufba.br](mailto:islame.felipe@ufba.br) |

## 📧 Contact  
For questions, contact the lead authors: [gustavojorge080@gmail.com](mailto:gustavojorge080@gmail.com) and [joao.lofiego@ufba.br](mailto:joao.lofiego@ufba.br).  
