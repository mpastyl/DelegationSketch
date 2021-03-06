Delegation Sketch
------------------------------------
A parallel design for sketches, that supports concurrent insertions and queries at high rates. Uses Augmented Sketch as the underlying sketch.

Publication: "Delegation sketch: a parallel design with support for fast and accurate concurrent operations", Charalampos Stylianopoulos, Ivan Walulya, Magnus Almgren, Olad Landsiedel, Marina Papatriantafilou, EuroSys'20, Heraklion, Greece, [pdf](https://dl.acm.org/doi/abs/10.1145/3342195.3387542)

Preliminary Instructions
----------------------------------
To build:

```
mkdir bin
cd src
make ITHACA=2
```
It will generate a different binary in bin for every version.

test_performance.sh is a script that builds and runs Delegation Sketch and the other baselines, against different parameters. Can be useful as a usage example.

Set ITHACA=2 for a straightforward thread-to-cpu pinning (i.e. thread 0 is pinned to CPU0). You might want to search for "ITHACA" in src/cm_benchark.cpp and change the mapping there for a more NUMA aware mapping.

Main source files
------------------------------
src/cm_benchmark.cpp contains the benchmark entry point, input and data structure creation, as well as the main processing loop. 
Also contains the main routines of Delegation Sketch.

src/sketches.cpp contains insert/query implementations of the underlying sketch.

inlude/filter.h contains operations regarding filters, used by Delegation Sketch and Augmented Sketch.

Notes
--------------------------
A refactoring is on its way...

Acknowledgments
------------------------------
Part of the project was based on sketch, hash function and input distribution creation from Florin Rusu https://faculty.ucmerced.edu/frusu/Projects/Sketches/sketches.html  Some documentation on those parts can be found there.

