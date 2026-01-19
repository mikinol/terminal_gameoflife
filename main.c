#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

volatile sig_atomic_t stop = 0;

void handle_signal(int signum) { stop = 1; }

volatile sig_atomic_t resized = 0;
volatile sig_atomic_t new_cols = 0;
volatile sig_atomic_t new_rows = 0;

void handle_winch(int sig) {
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
    new_cols = w.ws_col;
    new_rows = w.ws_row;
    resized = 1;
  }
}

#define CELL(grid, i, j, cols) (grid)[(i) * (cols) + (j)]

void print_grid(bool *grid, unsigned short cols, unsigned short rows) {
  printf("\033[H\033[J");
  for (unsigned short i = 0; i < rows; i++) {
    for (unsigned short j = 0; j < cols; j++)
      putchar(CELL(grid, i, j, cols) ? '#' : ' ');
    if (i + 1 < rows)
      putchar('\n');
  }
  fflush(stdout);
}
void fill_random(bool *grid, unsigned short cols, unsigned short rows) {
  for (unsigned short int i = 0; i < rows; i++)
    for (unsigned short int j = 0; j < cols; j++)
      CELL(grid, i, j, cols) = rand() % 2 == 0;
}

bool *resize_grid(bool *grid, unsigned short cols, unsigned short rows, unsigned short new_cols, unsigned short new_rows) {
  bool *newgrid = NULL;
  if (cols < new_cols) {
    newgrid = realloc(grid, new_rows * new_cols * sizeof(bool));
    if (!newgrid) {
      perror("malloc failed");
      exit(1);
    }

    int max_i = (rows < new_rows) ? rows : new_rows;
    for (int i = max_i - 1; i >= 0; i--) {
      for (int j = cols - 1; j >= 0; j--) {
        CELL(newgrid, i, j, new_cols) = CELL(newgrid, i, j, cols);
      }
      for (int j = cols; j < new_cols; j++) {
        CELL(newgrid, i, j, new_cols) = false;
      }
    }

    for (int i = rows; i < new_rows; i++) {
      for (int j = 0; j < new_cols; j++) {
        CELL(newgrid, i, j, new_cols) = false;
      }
    }
  } else if (cols > new_cols) {
    int max_i = (rows < new_rows) ? rows : new_rows;
    for (int i = 1; i < max_i; i++) {
      for (int j = 0; j < cols - (cols - new_cols); j++) {
        CELL(grid, i, j, new_cols) = CELL(grid, i, j, cols);
      }
    }

    newgrid = realloc(grid, new_rows * new_cols * sizeof(bool));
    if (!newgrid) {
      perror("malloc failed");
      exit(1);
    }

    for (int i = rows; i < new_rows; i++) {
      for (int j = 0; j < new_cols; j++) {
        CELL(newgrid, i, j, new_cols) = false;
      }
    }

  } else {
    newgrid = realloc(grid, new_rows * new_cols * sizeof(bool));
    if (!newgrid) {
      perror("malloc failed");
      exit(1);
    }
  }

  return newgrid;
}

void run_tick(bool *grid, bool *temp_grid, unsigned short cols, unsigned short rows) {
  for (unsigned short i = 0; i < rows; i++) {
    for (unsigned short j = 0; j < cols; j++) {
      unsigned char val = 0;
      if (i != 0)
        val += CELL(grid, i - 1, j, cols);
      if (j != 0)
        val += CELL(grid, i, j - 1, cols);
      if (i != rows - 1)
        val += CELL(grid, i + 1, j, cols);
      if (j != cols - 1)
        val += CELL(grid, i, j + 1, cols);
      if (i != 0 && j != 0)
        val += CELL(grid, i - 1, j - 1, cols);
      if (i != rows - 1 && j != 0)
        val += CELL(grid, i + 1, j - 1, cols);
      if (i != 0 && j != cols - 1)
        val += CELL(grid, i - 1, j + 1, cols);
      if (i != rows - 1 && j != cols - 1)
        val += CELL(grid, i + 1, j + 1, cols);

      if (CELL(grid, i, j, cols))
        CELL(temp_grid, i, j, cols) = val == 2 || val == 3;
      else
        CELL(temp_grid, i, j, cols) = val == 3;
    }
  }
}

int main(int argc, char *argv[]) {
  setvbuf(stdout, NULL, _IOFBF, 0);
  struct winsize w;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    perror("ioctl");
    return 1;
  }

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
  signal(SIGWINCH, handle_winch);

  unsigned short int cols = w.ws_col;
  unsigned short int rows = w.ws_row;

  bool *grid = malloc(rows * cols * sizeof(bool));
  if (!grid) {
    perror("malloc failed");
    exit(1);
  }
  bool *temp_grid = malloc(rows * cols * sizeof(bool));
  if (!temp_grid) {
    perror("malloc failed");
    exit(1);
  }

  srand(time(NULL));
  fill_random(grid, cols, rows);

  while (!stop) {

    if (resized) {
      resized = 0;
      if (cols == new_cols && rows == new_rows)
        continue;

      free(temp_grid);
      temp_grid = malloc(new_rows * new_cols * sizeof(bool));
      if (!temp_grid) {
        perror("malloc failed");
        exit(1);
      }

      grid = resize_grid(grid, cols, rows, new_cols, new_rows);

      cols = new_cols;
      rows = new_rows;
    }

    run_tick(grid, temp_grid, cols, rows);

    bool *temp = grid;
    grid = temp_grid;
    temp_grid = temp;

    print_grid(grid, cols, rows);

    usleep(100 * 1000);
  }

  putchar('\n');
  fflush(stdout);

  free(grid);
  free(temp_grid);
}
