#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>

#if defined(__unix__)
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#error "Platform not supported!"
#endif

int get_termsize(int *width, int *height) {
    if (!width || !height)
        return -1;
#if defined(__unix__)
    struct winsize ws;
    if (ioctl(0, TIOCGWINSZ, &ws) < 0)
        return -1;
    *width = ws.ws_col;
    *height = ws.ws_row;
#elif defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return -1;
    *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#endif
    return 0;
}

long long get_time(void) {
#if defined(__unix__)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#elif defined(_WIN32)
    return GetTickCount();
#endif
}

typedef double real_t;

typedef struct {
    real_t real;
    real_t imag;
} complex_t;

#define COMPLEX(real, imag) (complex_t) { (real), (imag) }

#define re(z) (z).real
#define im(z) (z).imag

static inline complex_t cadd(complex_t lhs, complex_t rhs) {
    return COMPLEX(lhs.real + rhs.real, lhs.imag + rhs.imag);
}

static inline complex_t csquared(complex_t z) {
    return COMPLEX(re(z) * re(z) - im(z) * im(z), 2 * re(z) * im(z));
}

static inline real_t clength_squared(complex_t z) {
    return re(z) * re(z) - im(z) * im(z);
}

static inline real_t clength(complex_t z) {
    return sqrt(clength_squared(z));
}

int mandelbrot(complex_t z, complex_t c, int n) {
    if (n < 0)
        
return -1;
    if (n == 0 || clength_squared(z) > 4)
        return 0;
    z = cadd(csquared(z), c);
    return mandelbrot(z, c, n - 1) + 1;
}

#define remap(x, dim, l, r) (real_t) (x) * ((r) - (l)) / ((dim) - 1) + (l)

#define NCHR 12
char chr[NCHR] = ".,-~:;=!*#$@";

void render(int width, int height, int n, char *buffer) {
    long long t = get_time();
    float sine = sin(t / 2500.0) * 0.5 + 0.5;
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            complex_t c = COMPLEX(
                remap(x, width, -2 + sine + 0.75 * sine, 1 - sine + 0.75 * sine),
                remap(y, height, -1 + sine * 0.67, 1 - sine * 0.67)
            );
            int i = mandelbrot(COMPLEX(0, 0), c, n);
            size_t idx = (size_t) ((real_t) i / n * (NCHR - 1));
            buffer[y * width + x] = chr[idx];
        }
    }
    for (int i = 0; i < width * height; ++i) {
        printf("%c", buffer[i]);
    }
}

int main() {
    int width = 0, height = 0;
    if (get_termsize(&width, &height) < 0 || width <= 1 || height <= 1)
        return -1;
    char *buffer = malloc(width * height * sizeof(char));
    for (;;) {
        printf("\033[H");
        render(width, height, 500, buffer);
    }
    return 0;
}
