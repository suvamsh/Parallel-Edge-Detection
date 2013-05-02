gcc -fopenmp -lm -O3 -msse3 parallel_sobel.c -o parallel_sobel
gcc -lm -O3 serial_sobel.c -o serial_sobel
