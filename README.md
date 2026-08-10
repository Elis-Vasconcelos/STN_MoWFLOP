

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
./meta_heuristics/moead <instance_id> [output_dir] [angle] [wind] [run_id] [stop_criteria]
./meta_heuristics/nsga2 <instance_id> [output_dir] [angle] [wind] [run_id] [stop_criteria]
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
later. Both constants live in `headers/globals.h`, next to
`SIZE_OF_POPULATION`. Columns: `run_id,vector_id,generation,f_cost,f_power,
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

### Running a full campaign

`meta_heuristics/scripts/run_one.sh` and `run_campaign.sh` automate running
many instances/runs/algorithms and organize the output under
`raw_results/meta_heuristics_stn/<algo>/<instance>/<run_id>/`:

```bash
cd source_code
seq 1 300 > instances.txt
./meta_heuristics/scripts/run_campaign.sh instances.txt 20   # 20 runs, full stop_criteria (1e6)
```

- Idempotent: re-running skips any `(instance, algo, run_id)` whose
  `_stn.csv` already exists — safe to interrupt and resume.
- `_candidates.csv` is instance-only, so `run_one.sh` keeps a single
  canonical copy per instance under `raw_results/meta_heuristics_stn/
  candidates/` and symlinks (relative, portable across machines) it into
  every run's directory, instead of letting the ~290KB table get
  regenerated in every one of the `instances × runs × 2` output
  directories a full campaign creates.
- Sequential on purpose. To parallelize on the supercomputer's actual job
  scheduler, call `run_one.sh <instance> <algo> <run_id> [stop_criteria]`
  directly, one combination per job/task, instead of `run_campaign.sh`.

## 👥 Authors  
| Name | Affiliation | Contact |  
|------|-------------|---------|  
| **Gustavo J. N. Silva** | Federal University of Bahia, Institute of Computing | [gustavojorge080@gmail.com](mailto:gustavojorge080@gmail.com) |  
| **João G. Lofiego** | Federal University of Bahia, Institute of Computing | [joao.lofiego@ufba.br](mailto:joao.lofiego@ufba.br) |
| **Islame F. C. Fernandes** | Federal University of Bahia, Institute of Computing | [islame.felipe@ufba.br](mailto:islame.felipe@ufba.br) |

## 📧 Contact  
For questions, contact the lead authors: [gustavojorge080@gmail.com](mailto:gustavojorge080@gmail.com) and [joao.lofiego@ufba.br](mailto:joao.lofiego@ufba.br).  
