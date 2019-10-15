set terminal png font arial 14 size 1920,1080
set title "Performance comparison of different matrix multiplication implementations"
set label " Intel(R) Core(TM) i5-7440HQ CPU @ 2.80GHz with  6144 KB L3 cache, 256KiB L2 cache, 32KiB L1 cache" at screen 0.35, 0.9 font "arial,10"
set xlabel "Array size in Bytes"
set ylabel "Bandwidth in GiB/s"

plot "data_gmm_bench_mkl/cblas_mkl.dat" w lp lw 2 t "cblas mkl" smooth unique, \
	"data_gmm_bench_mkl/ijk.dat" w lp lw 2 t "ijk" smooth unique, \
	"data_gmm_bench_mkl/ikj.dat" w lp lw 2 t "ikj" smooth unique, \
	"data_gmm_bench_mkl/ikj_OpenMP.dat" w lp lw 2 t "ikj OpenMP" smooth unique, \
	"data_gmm_bench_mkl/ikj_OpenMP_+_vec_+_Unroll.dat" w lp lw 2 t "ikj OpenMP  vec  Unroll" smooth unique, \
