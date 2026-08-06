/* main.c
 *
 * C driver for imgCvtGrayIntToDouble (implemented in imgcvt.asm).
 *   1. Collects height, width, and pixel values from stdin.
 *   2. Allocates memory, calls the asm routine, and prints input/output.
 *   3. Verifies correctness against an independently computed expected
 *      value (f = i/255) for every pixel.
 *   4. Benchmarks the asm function (only the call itself is timed) over
 *      30 trials each for 10x10, 100x100, and 1000x1000 random images.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <windows.h>

extern void imgCvtGrayIntToDouble(unsigned char *intImg, double *dblImg,
                                   int height, int width);

#define EPSILON 1e-9
#define TRIALS  30

/* Recomputes the expected value for every pixel in C (independently of
 * the asm result) and returns the number of mismatches (0 = correct). */
static long verify(const unsigned char *intImg, const double *dblImg, int n)
{
    long mismatches = 0;
    for (int i = 0; i < n; i++) {
        double expected = intImg[i] * (1.0 / 255.0);
        if (fabs(expected - dblImg[i]) > EPSILON) mismatches++;
    }
    return mismatches;
}

/* Times imgCvtGrayIntToDouble on a random height x width image, averaged
 * over TRIALS runs, and checks correctness of the result. */
static void benchmark(int height, int width)
{
    int n = height * width;
    unsigned char *intImg = malloc(n);
    double *dblImg = malloc(n * sizeof(double));

    for (int i = 0; i < n; i++) intImg[i] = (unsigned char)(rand() % 256);

    LARGE_INTEGER freq, t0, t1;
    QueryPerformanceFrequency(&freq);

    double total = 0.0;
    for (int t = 0; t < TRIALS; t++) {
        QueryPerformanceCounter(&t0);
        imgCvtGrayIntToDouble(intImg, dblImg, height, width);  /* timed */
        QueryPerformanceCounter(&t1);
        total += (double)(t1.QuadPart - t0.QuadPart) / (double)freq.QuadPart;
    }
    double avg_us = (total / TRIALS) * 1e6;

    long mismatches = verify(intImg, dblImg, n);
    printf("%5d x %-5d avg = %10.3f us over %d runs   correctness = %s\n",
           height, width, avg_us, TRIALS, mismatches == 0 ? "PASS" : "FAIL");

    free(intImg);
    free(dblImg);
}

int main(void)
{
    srand((unsigned)time(NULL));

    /*Collect inputs from stdin */
    int height, width;
    if (scanf("%d %d", &height, &width) != 2) {
        fprintf(stderr, "Error: could not read height and width\n");
        return 1;
    }

    int n = height * width;
    unsigned char *intImg = malloc(n * sizeof(unsigned char));
    double *dblImg = malloc(n * sizeof(double));
    if (!intImg || !dblImg) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return 1;
    }

    /* Read pixel values. The while-loop skips commas, spaces, and newlines
       so the assignment's example format (with or without commas) works. */
    for (int i = 0; i < n; i++) {
        int val;
        while (scanf("%d", &val) != 1) {
            scanf("%*c");          /* discard one non-numeric character */
        }
        intImg[i] = (unsigned char)val;
    }

    imgCvtGrayIntToDouble(intImg, dblImg, height, width);

    printf("Input:\n");
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) printf("%3u ", intImg[r * width + c]);
        printf("\n");
    }

    printf("\nOutput:\n");
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) printf("%.2f ", dblImg[r * width + c]);
        printf("\n");
    }

    long mismatches = verify(intImg, dblImg, n);
    printf("\nCorrectness: %s (%ld mismatched pixel(s) out of %d)\n\n",
           mismatches == 0 ? "PASS" : "FAIL", mismatches, n);

    free(intImg);
    free(dblImg);

    /* Performance benchmark */
    printf("Benchmark (asm function only, %d trials each):\n", TRIALS);
    benchmark(10, 10);
    benchmark(100, 100);
    benchmark(1000, 1000);

    return 0;
}