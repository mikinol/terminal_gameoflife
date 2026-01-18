#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

volatile sig_atomic_t stop = 0;

void handle_signal(int signum) { stop = 1; }

bool **alloc_grid(unsigned short cols, unsigned short rows) {
  bool **grid = malloc(rows * sizeof(bool *));
  for (unsigned short i = 0; i < rows; i++) {
    grid[i] = malloc(cols * sizeof(bool));
  }

  return grid;
}
void print_grid(bool **grid, unsigned short cols, unsigned short rows) {
  printf("\e[3J\033[H\033[J");
  for (unsigned short i = 0; i < rows; i++) {
    for (unsigned short j = 0; j < cols; j++) {
      putchar(grid[i][j] ? '#' : ' ');
    }
    if (i + 1 < rows) {
      putchar('\n');
    }
  }
  fflush(stdout);
}
void fill_random(bool **grid, unsigned short cols, unsigned short rows) {
  for (unsigned short int i = 0; i < rows; i++) {
    for (unsigned short int j = 0; j < cols; j++) {
      grid[i][j] = rand() % 2 == 0;
    }
  }
}
void free_grid(bool **grid, unsigned short int rows) {
  for (unsigned short int i = 0; i < rows; i++) {
    free(grid[i]);
  }
  free(grid);
}

unsigned char **allocate_calculate_grid(unsigned short cols, unsigned short rows) {
  unsigned char **grid = malloc(rows * sizeof(unsigned char *));
  for (unsigned short i = 0; i < rows; i++) {
    grid[i] = malloc(cols * sizeof(unsigned char));
  }

  return grid;
}
void calculate_neighbours(bool **grid, unsigned char **calculate_grid, unsigned short cols, unsigned short rows) {
  for (unsigned short i = 0; i < rows; i++) {
    for (unsigned short j = 0; j < cols; j++) {
      unsigned char val = 0;
      if (i != 0)
        val += grid[i - 1][j];
      if (j != 0)
        val += grid[i][j - 1];
      if (i != rows - 1)
        val += grid[i + 1][j];
      if (j != cols - 1)
        val += grid[i][j + 1];
      if (i != 0 && j != 0)
        val += grid[i - 1][j - 1];
      if (i != rows - 1 && j != 0)
        val += grid[i + 1][j - 1];
      if (i != 0 && j != cols - 1)
        val += grid[i - 1][j + 1];
      if (i != rows - 1 && j != cols - 1)
        val += grid[i + 1][j + 1];

      calculate_grid[i][j] = val;
    }
  }

  for (unsigned short i = 0; i < rows; i++) {
    for (unsigned short j = 0; j < cols; j++) {
      unsigned char val = calculate_grid[i][j];
      if (grid[i][j]) {
        if (val < 2 || val > 3)
          grid[i][j] = false;
      } else {
        if (val == 3)
          grid[i][j] = true;
      }
    }
  }
}
void free_calculatie_grid(unsigned char **calculete_grid, unsigned short int rows) {
  for (unsigned short int i = 0; i < rows; i++) {
    free(calculete_grid[i]);
  }
  free(calculete_grid);
}

int main(int argc, char *argv[]) {
  struct winsize w;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    perror("ioctl");
    return 1;
  }

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  unsigned short int cols = w.ws_col;
  unsigned short int rows = w.ws_row;

  bool **grid = alloc_grid(cols, rows);
  unsigned char **calculate_grid = allocate_calculate_grid(cols, rows);

  srand(time(NULL));
  fill_random(grid, cols, rows);

  while (!stop) {
    calculate_neighbours(grid, calculate_grid, cols, rows);

    print_grid(grid, cols, rows);

    usleep(100 * 1000);
  }

  printf("\n");
  free_grid(grid, rows);
  free_calculatie_grid(calculate_grid, rows);
}
