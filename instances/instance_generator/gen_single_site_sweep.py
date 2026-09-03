"""
Single-site sparsity sweep for the Shannon-entropy STN degeneracy test (RQ1).

Builds instances 506_e-02 ... 513_e-05 -- eight instances that all share ONE
synthetic single-zone site: the same polygon, the same obstacles, the same
58 fixed turbines, and the same underlying depth field. The only things that
vary between them are:

  * the number of mobile turbines  tau  (5 for 506-509, 15 for 510-513), and
  * the number of available positions |P| (the density of the candidate grid).

This is deliberate: within each tau family only the sparsity tau/|P| changes,
and between the two families only tau changes, so any effect on the entropy
metric / occupation-grid partitioning / kappa / algorithm behaviour can be
attributed to sparsity rather than to a change of landscape. (Prof. Islame's
instruction, 2026-08-31: "pode gerar todas no mesmo sitio, isto e, a mesma
geometria".)

The eight target |P| values are fixed by the spec CSV
(`WFLOP instances - Instancias esparsas.csv`) and are reproduced EXACTLY, not
approximated: the grid cell size is bracketed so the in-polygon point count
lands at or just above the target, then the surplus points are dropped
uniformly at random (fixed seed) to hit the exact integer. Random drop keeps
the grid spatially uniform; it does not pull the candidate region's outline
inward the way a farthest-from-centroid trim would, which matters because the
site geometry must stay identical across all eight.

Run from this directory (STN_MoWFLOP/instances/instance_generator/):

    python gen_single_site_sweep.py

Output: STN_MoWFLOP/wflop_instances/sparse_instances/single_site_sweep/<name>/
with the usual five files (availablePositions.txt, fixed_wf.txt,
turbines_per_zone.txt, geometry.txt, plot.png).

Needs shapely, numpy, matplotlib, perlin_noise (e.g. the venv at
../../../instancegeneration/.venv).
"""

import os
import sys
import random
import zlib

import numpy as np
import shapely
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from perlin_noise import PerlinNoise

# gen_layout.py lives next to this file; its single-zone layout generator is
# reused verbatim. script.py CANNOT be imported (it runs an instance-
# generation loop at module import time), so the obstacle / fixed-turbine
# helpers below are ported from it byte-for-byte.
from gen_layout import gen_layout, hasIntersections, re_position


# ---------------------------------------------------------------------------
# Spec: the eight instances (name, |P| target, tau).  1 zone, 58 fixed each.
# ---------------------------------------------------------------------------

INSTANCES = [
    ("506_e-02",     60,  5),
    ("507_e-03",    600,  5),
    ("508_e-04",   6000,  5),
    ("509_e-05",  60000,  5),
    ("510_e-02",   1076, 15),
    ("511_e-03",   2548, 15),
    ("512_e-04",  19505, 15),
    ("513_e-05", 185054, 15),
]

NUM_FIXED = 58
NUM_ZONES = 1

# One seed fixes the whole shared site (polygon + obstacles + fixed turbines
# + depth field). Bump only if you deliberately want a different site.
SITE_SEED = 20260831

OUT_PARENT = os.path.normpath(
    os.path.join(
        os.path.dirname(__file__),
        "..", "..", "wflop_instances", "sparse_instances", "single_site_sweep",
    )
)


def stable_seed(*parts) -> int:
    """Deterministic seed from string/number parts (Python's hash() is
    per-process randomised for str/tuple, so it can't be used here)."""
    s = "|".join(str(p) for p in parts).encode()
    return zlib.crc32(s)


def cost(depth: float) -> float:
    return 659933.9999999129 + depth * -72606.60000000268


# ---------------------------------------------------------------------------
# Ported verbatim from instancegeneration / STN_MoWFLOP instance_generator
# script.py (module-level side effects prevent importing it).
# ---------------------------------------------------------------------------

def gen_hole(polygon: shapely.Polygon):
    new_hole = []
    hole_max = random.uniform(polygon.length / 50, polygon.length / 25)

    for i in range(random.randint(3, 7)):
        new_hole.append(tuple([random.uniform(0.0, hole_max), random.uniform(0.0, hole_max)]))

    new_hole = shapely.Polygon(new_hole)
    new_hole = shapely.Polygon(new_hole.convex_hull)

    minx, miny, maxx, maxy = polygon.bounds
    point = shapely.Point(0, 0)

    while not polygon.contains(point):
        point = shapely.Point([random.uniform(minx, maxx), random.uniform(miny, maxy)])

    distance = point - new_hole.centroid
    coords = new_hole.exterior.coords[:]
    for i in range(len(coords)):
        coords[i] = (coords[i][0] + distance.x, coords[i][1] + distance.y)
    new_hole = shapely.Polygon(coords)

    return polygon.difference(new_hole)


def gen_holes(polygon: shapely.Polygon, qtty):
    for i in range(qtty):
        copy = polygon
        while copy.equals(polygon):
            copy = polygon
            copy = gen_hole(copy)
        while copy.geom_type == "MultiPolygon" or copy.equals(polygon):
            copy = polygon
            copy = gen_hole(copy)
        polygon = copy
    return polygon


def gen_structure(polygon: shapely.Polygon):
    minx, miny, maxx, maxy = polygon.bounds

    pointIn = shapely.Point([random.uniform(minx, maxx), random.uniform(miny, maxy)])
    while not polygon.contains(pointIn):
        pointIn = shapely.Point([random.uniform(minx, maxx), random.uniform(miny, maxy)])

    pointOut = shapely.Point([random.uniform(100000.0, 150000.0), random.uniform(100000.0, 150000.0)])
    while polygon.contains(pointOut):
        pointOut = shapely.Point([random.uniform(minx, maxx), random.uniform(miny, maxy)])

    dist = random.uniform(polygon.length / 300, polygon.length / 200)
    line = shapely.LineString([pointIn, pointOut])

    left = [shapely.Point(c) for c in line.parallel_offset(dist / 2, "left").coords]
    right = [shapely.Point(p) for p in line.parallel_offset(dist / 2, "right").coords]

    structure = shapely.Polygon(left + right).convex_hull
    return polygon.difference(structure)


def gen_structures(polygon: shapely.Polygon, qtty):
    for i in range(qtty):
        copy = polygon
        copy = gen_structure(copy)
        while copy.geom_type == "MultiPolygon":
            copy = polygon
            copy = gen_structure(copy)
        polygon = copy
    return polygon


def exact_grid_dims(n: int):
    """(a, b) with a*b == n exactly and a as close to sqrt(n) as possible.

    script.py's create_grid() uses floor division, so create_grid(58) can
    return e.g. (3, 19) -> only 57 turbines placed. The spec CSV fixes the
    fixed-turbine count at 58, so an exact factor pair is used instead
    (58 = 2 x 29)."""
    a = int(n ** 0.5)
    while a > 1 and n % a != 0:
        a -= 1
    return a, n // a


def gen_grid(vec, rows: int, cols: int, dist: float, base: shapely.LineString):
    tVec = dist / (vec[0] ** 2 + vec[1] ** 2) ** 0.5
    vec = [vec[0] * tVec, vec[1] * tVec]
    perpVec = [vec[1], -vec[0]]

    points = []
    jumps = cols - 1
    limit = base.length - jumps * dist if base.length - jumps * dist > 0 else 0
    initial_point = base.interpolate(random.uniform(0, limit))

    for i in range(cols):
        for j in range(1, rows + 1):
            points.append([initial_point.x + vec[0] * i + perpVec[0] * j, initial_point.y + vec[1] * i + perpVec[1] * j])

    return points


def gen_fixed(polygons, num_fixed: int):
    grid_sizes = exact_grid_dims(num_fixed)
    zone = random.randint(0, len(polygons) - 1)
    dist = random.uniform(200, 500)  # noqa: F841  (kept for RNG-stream parity with script.py)

    polyX, polyY = polygons[zone].exterior.xy
    polyX = polyX[:-1]
    polyY = polyY[:-1]

    rlNum = random.randint(0, len(polyX) - 1)
    vector = [polyX[(rlNum + 1) % len(polyX)] - polyX[rlNum], polyY[(rlNum + 1) % len(polyY)] - polyY[rlNum]]
    randomLine = shapely.LineString([[polyX[rlNum], polyY[rlNum]], [polyX[(rlNum + 1) % len(polyX)], polyY[(rlNum + 1) % len(polyY)]]])

    return gen_grid(vector, grid_sizes[1], grid_sizes[0], 1440, randomLine)


# ---------------------------------------------------------------------------
# Build the one shared site
# ---------------------------------------------------------------------------

class Site:
    def __init__(self, raw_polygon, obstacle_polygon, fixed_points, octaves):
        self.raw_polygon = raw_polygon            # boundary only (for geometry.txt / plot)
        self.obstacle_polygon = obstacle_polygon  # boundary minus holes/structures
        self.fixed_points = fixed_points          # list[(x, y)]
        minx, miny, maxx, maxy = raw_polygon.bounds
        self.minx, self.miny = minx, miny
        self.sizex = maxx - minx
        self.sizey = maxy - miny
        self._noise = PerlinNoise(octaves=octaves, seed=stable_seed(SITE_SEED, "perlin"))

    def depth_at(self, xs, ys):
        """Depth at world coords, sampled from ONE Perlin field defined on the
        site bounds -- so a given (x, y) has the same depth in every instance.
        Same -25 +/- 15*noise mapping as script.py's gen_points."""
        out = np.empty(len(xs), dtype=float)
        for k, (x, y) in enumerate(zip(xs, ys)):
            nx = (x - self.minx) / self.sizex
            ny = (y - self.miny) / self.sizey
            out[k] = -25.0 + self._noise([nx, ny]) * 15.0
        return out


def build_site() -> Site:
    random.seed(stable_seed(SITE_SEED, "layout"))
    polygons = gen_layout(NUM_ZONES)
    while hasIntersections(polygons):
        for i in range(len(polygons)):
            if polygons[i].intersects(polygons[(i + 1) % len(polygons)]):
                polygons[i], polygons[(i + 1) % len(polygons)] = re_position(
                    polygons[i], polygons[(i + 1) % len(polygons)]
                )
    raw_polygon = polygons[0]

    # obstacles: same count distribution as script.py's createInstance
    random.seed(stable_seed(SITE_SEED, "obstacles"))
    num_structs = random.randint(5, 8)
    num_holes = random.randint(2, 5)
    obstacle_polygon = gen_holes(raw_polygon, num_holes)
    obstacle_polygon = gen_structures(obstacle_polygon, num_structs - num_holes)

    # 58 fixed turbines, placed the same way script.py / gen_sparse_variant do
    random.seed(stable_seed(SITE_SEED, "fixed"))
    fixed_points = gen_fixed([raw_polygon], NUM_FIXED)
    while raw_polygon.intersects(shapely.MultiPoint(fixed_points)):
        fixed_points = gen_fixed([raw_polygon], NUM_FIXED)

    octaves = random.randint(3, 7)
    return Site(raw_polygon, obstacle_polygon, fixed_points, octaves)


# ---------------------------------------------------------------------------
# Grid density -> exact position count
# ---------------------------------------------------------------------------

def grid_centroids(polygon, cell_size):
    minx, miny, maxx, maxy = polygon.bounds
    sizex, sizey = maxx - minx, maxy - miny
    valuex = max(1, round(sizex / cell_size))
    valuey = max(1, round(sizey / cell_size))
    cellx, celly = sizex / valuex, sizey / valuey

    j = np.arange(valuex)
    i = np.arange(valuey)
    centroidX = (minx + j * cellx + minx + (j + 1) * cellx) / 2
    centroidY = (maxy - i * celly + maxy - (i + 1) * celly) / 2
    xx, yy = np.meshgrid(centroidX, centroidY)
    return xx.ravel(), yy.ravel()


def count_for_cell(polygon, cell_size):
    xs, ys = grid_centroids(polygon, cell_size)
    mask = shapely.contains_xy(polygon, xs, ys)
    return int(mask.sum())


def solve_exact_grid(polygon, target, seed, max_iter=80):
    """Return exactly `target` in-polygon (x, y) points.

    Bracket the cell size so the in-polygon count is >= target and as close
    to it as possible, then drop the surplus uniformly at random."""
    area = polygon.area
    guess = (area / target) ** 0.5
    lo, hi = guess / 50.0, guess * 50.0          # lo cell -> many points, hi cell -> few

    best = None  # (count, cell) with count >= target, smallest such count
    for _ in range(max_iter):
        mid = (lo + hi) / 2.0
        c = count_for_cell(polygon, mid)
        if c >= target and (best is None or c < best[0]):
            best = (c, mid)
        if c > target:
            lo = mid      # need fewer points -> larger cell
        elif c < target:
            hi = mid      # need more points -> smaller cell
        else:
            best = (c, mid)
            break

    if best is None:
        # never reached >= target (shouldn't happen); use the finest grid tried
        cell = lo
    else:
        cell = best[1]

    xs, ys = grid_centroids(polygon, cell)
    mask = shapely.contains_xy(polygon, xs, ys)
    xin, yin = xs[mask], ys[mask]

    if len(xin) < target:
        raise RuntimeError(f"grid solver undershot: {len(xin)} < {target}")
    if len(xin) > target:
        rng = np.random.default_rng(seed)
        keep = np.sort(rng.choice(len(xin), size=target, replace=False))
        xin, yin = xin[keep], yin[keep]

    return xin, yin


# ---------------------------------------------------------------------------
# File writers
# ---------------------------------------------------------------------------

def write_instance(site: Site, name: str, target_positions: int, tau: int):
    out_dir = os.path.join(OUT_PARENT, name)
    os.makedirs(out_dir, exist_ok=True)

    xs, ys = solve_exact_grid(
        site.obstacle_polygon, target_positions, seed=stable_seed(SITE_SEED, name)
    )
    depths = site.depth_at(xs, ys)

    # availablePositions.txt : x y z foundation_cost zone   (zone = 1)
    with open(os.path.join(out_dir, "availablePositions.txt"), "w") as f:
        for x, y, d in zip(xs, ys, depths):
            f.write(f"{x:.11f} {y:.11f} {d:.11f} {cost(d):.11f} 1\n")

    # fixed_wf.txt : same 5-column format; z/cost are dummy, zone = NUM_ZONES+1
    # (matches New Sites/178 and instances/site/* -- the C++ loader reads five
    # whitespace tokens per line and ignores columns 3-5 for fixed turbines).
    sentinel = NUM_ZONES + 1
    with open(os.path.join(out_dir, "fixed_wf.txt"), "w") as f:
        for x, y in site.fixed_points:
            f.write(f"{x:.11f} {y:.11f} 0.0 {sentinel} {sentinel}\n")

    # turbines_per_zone.txt : single value, trailing space (matches 178's "23 ")
    with open(os.path.join(out_dir, "turbines_per_zone.txt"), "w") as f:
        f.write(f"{tau} ")

    # geometry.txt : boundary exterior only (obstacles are not written here --
    # same convention as gen_complete_layout / New Sites instances)
    with open(os.path.join(out_dir, "geometry.txt"), "w") as f:
        xe, ye = site.raw_polygon.exterior.xy
        for x, y in zip(xe, ye):
            f.write(f"{x:11f} {y:11f} \n")
        f.write("\n")

    # plot.png
    plt.figure()
    sc = plt.scatter(xs, ys, 2, c=depths)
    xe, ye = site.obstacle_polygon.exterior.xy
    plt.plot(xe, ye, color="magenta")
    for inner in site.obstacle_polygon.interiors:
        xi, yi = zip(*inner.coords[:])
        plt.plot(xi, yi, color="magenta")
    fx = [p[0] for p in site.fixed_points]
    fy = [p[1] for p in site.fixed_points]
    plt.plot(fx, fy, marker="o", color="orange", markersize=1, linestyle="none")
    plt.colorbar(sc)
    plt.title(f"{name}  |P|={len(xs)}  tau={tau}")
    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, "plot.png"), dpi=195)
    plt.close()

    print(f"{name}: |P|={len(xs)} (target {target_positions}) tau={tau} "
          f"fixed={len(site.fixed_points)} density={tau / len(xs):.3e} -> {out_dir}")
    return len(xs)


def main():
    os.makedirs(OUT_PARENT, exist_ok=True)
    site = build_site()
    print(f"site: boundary area={site.raw_polygon.area / 1e6:.1f} km^2  "
          f"obstacle area={site.obstacle_polygon.area / 1e6:.1f} km^2  "
          f"fixed={len(site.fixed_points)}")

    ok = True
    if len(site.fixed_points) != NUM_FIXED:
        ok = False
        print(f"  !! fixed turbines: {len(site.fixed_points)} != {NUM_FIXED}")
    for name, target, tau in INSTANCES:
        achieved = write_instance(site, name, target, tau)
        if achieved != target:
            ok = False
            print(f"  !! {name}: achieved {achieved} != target {target}")

    print("ALL EXACT" if ok else "MISMATCH -- see above")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
