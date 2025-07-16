> [!NOTE]
> **Current comparison state**
> - [x] IO neutralized during PDE solutions
> - [x] Compare worse-case scenario (slowest solvers/PCs)
> - [x] The PDEs are coupled through their Source Terms; relatively strongly
> - [x] MPI speedup and scaling
> - [ ] Result validation, enforcing different implementations to give similar fields

[HPCToolkit](https://hpctoolkit.org/) is used to conduct and analyse measurements:

```bash
# May need multiple runs to measure incompatible events... eg. cycles and L-cache misses
hpcrun \
    -e cycles -e instructions \
    -e PAPI_TOT_CYC -e PAPI_TOT_INS \
    -e PAPI_VEC_INS -e PAPI_FP_INS \
    -e PAPI_BR_INS \
    -e PAPI_L1_DCM -e PAPI_L2_DCM \
    testDriver
# For more events:
hpcrun -L | grep -i PAPI
# or just
papi_avail 
```

>[!IMPORTANT]
> Access to CPU cycle counts is restricted on most systems; you can **temporarily** enable it with:
> `sudo sysctl -w kernel.perf_event_paranoid=0`

A few metrics that are important enough to look at:
- `IPC = PAPI_TOT_INS / PAPI_TOT_CYC`: -> For OpenFOAM code this typically hovers around 3.0
    - IPC > 2: Good
    - IPC < 1: Stalling CPU -> Mem latency? Bad caching?
- SIMD usage:
    - How much of ops are vectorized? `SIMD_ratio = PAPI_VEC_INS / PAPI_FP_INS`
- Amount of branching: `PAPI_BR_INS`
- Cache misses:
    - `PAPI_L1_DCM, PAPI_L2_DCM` for L1 and L2 data/instruction cache misses
    - L3 if your CPU provides event trackers for it though PAPI

## Measure CPU performance

Do the initial setup, briefly explained in [README](./README.md)

```bash
cd /tmp/ut
sed -i 's/timeout "\$timeOut"/& hpcrun -e cycles -e instructions -e PAPI_TOT_ENS -e PAPI_TOT_CYC -e PAPI_VEC_INS -e PAPI_FP_INS -e PAPI_BR_INS -e PAPI_L1_DCM -e PAPI_L2_DCM/' Alltest
apptainer run <container>.sif 'wclean tests/blockMatrices; ./Alltest --no-parallel'
apptainer run <container>.sif 'hpcstruct hpctoolkit-testDriver-measurements'
apptainer run <container>.sif 'hpcprof hpctoolkit-testDriver-measurements'
apptainer run <container>.sif 'hpcviewer hpctoolkit-testDriver-database'
# Compute metrics above in the GUI, look at most expensive functions...
```

### Findings
- IPC: 3.0 as expected
- GMRES Loop at `gmres.C:231`
  - 247 - matrixMul is very slow
  - 250 - lusgs::precondition() is kind of slow
  - lusgs:233 33% branching happens here
  - lusgs:307 33% branching happens here

- Original time baseline: 450ms
- 1.1 Treat cell-based processing in lusgs::forwardSweep and lusgs::reveseSweep
      and component existence checks: 350ms (~22.22% gain)

## Compare to GPU-accelerated performance

Provided Apptainer containers compile PetSc in CUDA mode, but by default the CPU is used.
Adjust PetSC settings in `cases/theCase/system/petscDict`:
```bash
-T-Ts_vec_type cuda
-T-Ts_mat_type aijcusparse
```

Then run the tests (You may want to add more cells to the mesh to feel a difference and push GPU memory limits):
```bash
apptainer run --nv <container>.sif 'wclean tests/blockMatrices; ./Alltest --no-parallel'
```


> [!WARNING]
> The provided containers' MPI are not GPU aware so staging GPU memory happens through the CPU; which slower
> and scales way worse than Zero-copy GPU-to-GPU MPI routines.


## MPI scaling studies

### Weak scaling

`scripts/weak_scaling.py` script manages a weak scaling study on all benchmarked implementations. It requires
feeding it baseline data for serial runs and the command to run, as well as the number of processors to gather
scaling data for. The preferred way to use the script is:
```bash
cd scripts
uv sync
uv run weak_scaling.py --help
```
