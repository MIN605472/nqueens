#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Board {
  int size;
  int shift_diag45;
  int *solution;
  int *col_is_free;
  char *diag45_is_free;
  char *diag135_is_free;
} Board;

Board create_board(int size) {
  Board board;
  board.size = size;
  board.shift_diag45 = size - 1;
  board.solution = malloc(sizeof(int[size]));
  board.col_is_free = malloc(sizeof(int[size]));
  board.diag45_is_free = malloc(sizeof(char[size * 2 - 1]));
  board.diag135_is_free = malloc(sizeof(char[size * 2 - 1]));
  for (int i = 0; i < size; ++i) {
    board.solution[i] = -1;
    board.col_is_free[i] = 1;
  }
  for (int i = 0; i < 2 * size - 1; ++i) {
    board.diag45_is_free[i] = 1;
    board.diag135_is_free[i] = 1;
  }
  return board;
}

void delete_board(Board *board) {
  free(board->solution);
  free(board->col_is_free);
  free(board->diag45_is_free);
  free(board->diag135_is_free);
}

void print_board(Board *board) {
  printf("[");
  for (int i = 0; i < board->size - 1; ++i) printf("%d,", board->solution[i]);
  printf("%d", board->solution[board->size - 1]);
  printf("]\n");
}

/*
 * Check if the board passed is a valid solution to the n-queen problem.
 * @param the board to check
 * @return true if \p board is a solution, false otherwise
 */
char is_valid(Board *board) { return board->solution[board->size - 1] != -1; }

char can_place_queen(Board *board, int x, int y) {
  return board->col_is_free[x] &&
         board->diag45_is_free[y - x + board->shift_diag45] &&
         board->diag135_is_free[y + x];
}

void place_queen(Board *board, int x, int y) {
  board->solution[y] = x;
  board->col_is_free[x] = 0;
  board->diag45_is_free[y - x + board->shift_diag45] = 0;
  board->diag135_is_free[y + x] = 0;
}

void remove_queen(Board *board, int x, int y) {
  board->solution[y] = -1;
  board->col_is_free[x] = 1;
  board->diag45_is_free[y - x + board->shift_diag45] = 1;
  board->diag135_is_free[y + x] = 1;
}

/*
 * Apply a backtrack algorithm to solve the n-queens problem.
 * @param board the board to apply the algorithm to
 * @param y the number of queens that have already been placed on the board \p
 * board
 * @return true if a solution can be found, false otherwise
 */
char backtrack(Board *board, int y) {
  if (is_valid(board)) return 1;

  for (int x = 0; x < board->size; ++x) {
    if (can_place_queen(board, x, y)) {
      place_queen(board, x, y);
      char success = backtrack(board, y + 1);
      if (success)
        return 1;
      else
        remove_queen(board, x, y);
    }
  }
  return 0;
}

/*
 * Apply a Las Vegas algorithm to try to solve the n-queens problem. It first
 * places \p random_queens random queens on the board \p board, and it then
 * applies backtracking.
 * @param board the board to apply the algorithm to
 * @param random_queens the number of queens that are placed randomly
 * @return true if a solution can be found, false otherwise
 */
char solve_lv(Board *board, int random_queens) {
  int available_cols[board->size];
  int index;
  for (int y = 0; y < random_queens; ++y) {
    index = 0;
    for (int x = 0; x < board->size; ++x)
      if (can_place_queen(board, x, y)) available_cols[index++] = x;
    if (index == 0) return 0;
    place_queen(board, available_cols[rand() % index], y);
  }
  return backtrack(board, random_queens);
}

/*
 * Put the board to its initial state, i.e., no queens placed.
 * @param board the board to clean or reset
 */
void reset_board(Board *board) {
  for (int i = 0; i < board->size; ++i) {
    board->solution[i] = -1;
    board->col_is_free[i] = 1;
  }
  for (int i = 0; i < 2 * board->size - 1; ++i) {
    board->diag45_is_free[i] = 1;
    board->diag135_is_free[i] = 1;
  }
}

/*
 * Repeat the Las Vegas algorithm till it succeeds.
 * @param board the board to apply the algorithm to
 * @param random_queens the number of queens to place randomly
 * @param trials the number of tries that took to get a success
 */
void repeat_lv(Board *board, int random_queens, int *trials) {
  *trials = 0;
  do {
    reset_board(board);
    ++*trials;
  } while (!solve_lv(board, random_queens));
}

/*
 * Print some measurements in a CSV format of Las Vegas algorithm applied to all
 * possible values of k (number of random queens). Those measurements are: the
 * execution time in milliseconds (t) and the probability of success (s).
 * @param queens the number of queens; the size of the board
 * @param reps the number of measurements that should be taken to calculate the
 * mean value
 */
void print_stats(int queens, int reps) {
  Board board = create_board(queens);
  printf("k;t;s\n");
  for (int rand_queens = queens; rand_queens > -1; --rand_queens) {
    int total_trials = 0;
    clock_t start = clock();
    for (int i = 0; i < reps; ++i) {
      int trials;
      repeat_lv(&board, rand_queens, &trials);
      total_trials += trials;
    }
    clock_t end = clock();
    printf("%d;%.5f;%.5f\n", rand_queens,
           (end - start) * 1000.0 / CLOCKS_PER_SEC / reps,
           1.0 / ((double)total_trials / reps));
  }
  delete_board(&board);
}

/* Print a solution to the n-queens problem.
 * @param queens the number of queens; the size of the board
 * @param rand_queens the number of random queens to first place randomly
 */
void print_solution(int queens, int rand_queens) {
  Board board = create_board(queens);
  int trials;
  repeat_lv(&board, rand_queens, &trials);
  print_board(&board);
  delete_board(&board);
}

int main() {
  srand(time(NULL));
  printf("Some stats for n = 8:\n\n");
  print_stats(8, 1000000);
  printf("\n\nA solution for n = 100:\n\n");
  print_solution(100, 88);
  printf("\n\nA solution for n = 1000:\n\n");
  print_solution(1000, 983);
  printf("\n\nSome stats for n = 39:\n\n");
  print_stats(39, 100);
}
