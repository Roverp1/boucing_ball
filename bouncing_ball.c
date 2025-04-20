#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MAX_ARR_LENGHT 10
#define MAX_STRING_LENGTH 64

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

typedef enum {
  DIRECTION_UP_LEFT,
  DIRECTION_UP_RIGHT,
  DIRECTION_DOWN_LEFT,
  DIRECTION_DOWN_RIGHT
} SpriteDirection;

typedef struct {
  SpriteDirection direction;
  unsigned short position_column, position_row;
} SpritePositioning;

typedef struct {
  char lines[MAX_ARR_LENGHT][MAX_STRING_LENGTH];
  unsigned short width, height;
} Sprite;

typedef struct {
  short dy, dx;
} MovementVector;

Sprite load_sprite_from_file(const char *file_path);
unsigned short load_sprite_lines(char linesArr[][MAX_STRING_LENGTH],
                                 FILE *opened_file, unsigned short *max_width);
void perform_terminal_setup_for_animation();
void restore_terminal();
MovementVector get_movement_vector(SpriteDirection sprite_direction);
SpriteDirection flip_horizontal_direction(SpriteDirection direction);
SpriteDirection flip_vertical_direction(SpriteDirection direction);
SpritePositioning update_sprite_position(SpritePositioning current_position,
                                         int max_row, int max_col);

void test_update_sprite_position(void);

int main(void) {
  struct winsize terminal_size;

  Sprite soccerBallSprite = load_sprite_from_file("./assets/soccer_ball.txt");

  perform_terminal_setup_for_animation();

  test_update_sprite_position();

  return 0;
}

Sprite load_sprite_from_file(const char *file_path) {
  Sprite soccerBallSprite = {
      .width = 0,
      .height = 0,
  };

  FILE *soccer_ball_ptr = fopen(file_path, "r");
  if (!soccer_ball_ptr) {
    fprintf(stderr, "failed to open soccer_ball.txt\n");
    exit(1);
  }

  soccerBallSprite.height = load_sprite_lines(
      soccerBallSprite.lines, soccer_ball_ptr, &soccerBallSprite.width);
  fclose(soccer_ball_ptr);

  return soccerBallSprite;
}

unsigned short load_sprite_lines(char linesArr[][MAX_STRING_LENGTH],
                                 FILE *opened_file, unsigned short *max_width) {

  unsigned short line_index = 0;

  while (fgets(linesArr[line_index], MAX_STRING_LENGTH, opened_file) != NULL &&
         line_index < MAX_ARR_LENGHT) {

    const short line_length = strlen(linesArr[line_index]);
    if (*max_width < line_length)
      *max_width = line_length;

    if (linesArr[line_index][line_length - 1] == '\n' && line_length > 0)
      linesArr[line_index][line_length - 1] = '\0';

    line_index++;
  }

  return line_index;
}

void perform_terminal_setup_for_animation() {
  printf(HIDE_CURSOR);
  atexit(restore_terminal);
}

void restore_terminal() { printf(SHOW_CURSOR); }

MovementVector get_movement_vector(SpriteDirection sprite_direction) {
  switch (sprite_direction) {
  case DIRECTION_UP_LEFT:
    return (MovementVector){-1, -1};
  case DIRECTION_UP_RIGHT:
    return (MovementVector){-1, 1};
  case DIRECTION_DOWN_LEFT:
    return (MovementVector){1, -1};
  case DIRECTION_DOWN_RIGHT:
    return (MovementVector){1, 1};
  default:
    assert(0 && "Ureachable: invalid SpriteDirection");
  }
}

SpriteDirection flip_horizontal_direction(SpriteDirection direction) {
  switch (direction) {
  case DIRECTION_UP_LEFT:
    return DIRECTION_UP_RIGHT;
  case DIRECTION_UP_RIGHT:
    return DIRECTION_UP_LEFT;
  case DIRECTION_DOWN_LEFT:
    return DIRECTION_DOWN_RIGHT;
  case DIRECTION_DOWN_RIGHT:
    return DIRECTION_DOWN_LEFT;
  default:
    assert(0 && "Ureachable: invalid SpriteDirection");
  }
}

SpriteDirection flip_vertical_direction(SpriteDirection direction) {
  switch (direction) {
  case DIRECTION_UP_LEFT:
    return DIRECTION_DOWN_LEFT;
  case DIRECTION_DOWN_LEFT:
    return DIRECTION_UP_LEFT;
  case DIRECTION_UP_RIGHT:
    return DIRECTION_DOWN_RIGHT;
  case DIRECTION_DOWN_RIGHT:
    return DIRECTION_UP_RIGHT;
  default:
    assert(0 && "Ureachable: invalid SpriteDirection");
  }
}

SpritePositioning update_sprite_position(SpritePositioning current_position,
                                         int max_row, int max_col) {
  if (current_position.position_row <= 0 ||
      current_position.position_row >= max_row)
    current_position.direction =
        flip_vertical_direction(current_position.direction);

  if (current_position.position_column <= 0 ||
      current_position.position_column >= max_col)
    current_position.direction =
        flip_horizontal_direction(current_position.direction);

  const MovementVector movement_vector =
      get_movement_vector(current_position.direction);

  current_position.position_row += movement_vector.dy;
  current_position.position_column += movement_vector.dx;

  return current_position;
}

void test_update_sprite_position(void) {
  SpritePositioning testSprite = {
      .position_row = 0, .position_column = 5, .direction = DIRECTION_UP_LEFT};

  SpritePositioning resultSprite = update_sprite_position(testSprite, 20, 40);

  printf("Expected direction: DIRECTION_DOWN_LEFT (%d)\n", DIRECTION_DOWN_LEFT);
  printf("Actual direction:   %d\n", resultSprite.direction);
  printf("Expected row: 1\n");
  printf("Actual row:   %d\n", resultSprite.position_row);
  printf("Expected col: 4\n");
  printf("Actual col:   %d\n", resultSprite.position_column);
}
