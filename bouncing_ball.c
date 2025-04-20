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
SpritePositioning update_sprite_position(const Sprite *sprite,
                                         SpritePositioning current_position,
                                         int max_row, int max_col);
void draw_sprite_recursive(const Sprite *sprite,
                           const SpritePositioning *spritePosition,
                           unsigned short line_index);
struct winsize get_terminal_size(void);
void main_animation(Sprite *sprite, SpritePositioning *spritePosition);

int main(void) {
  SpritePositioning soccerBallPosition = {
      .position_column = 1,
      .position_row = 1,
      .direction = DIRECTION_DOWN_RIGHT,
  };

  Sprite soccerBallSprite = load_sprite_from_file("./assets/soccer_ball.txt");

  perform_terminal_setup_for_animation();
  main_animation(&soccerBallSprite, &soccerBallPosition);

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

    short line_length = strlen(linesArr[line_index]);
    if (linesArr[line_index][line_length - 1] == '\n' && line_length > 0)
      linesArr[line_index][line_length - 1] = '\0';

    if (line_length == 0)
      continue;

    line_length = strlen(linesArr[line_index]);
    if (*max_width < line_length)
      *max_width = line_length;

    line_index++;
  }

  return line_index--;
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

SpritePositioning update_sprite_position(const Sprite *sprite,
                                         SpritePositioning current_position,
                                         int max_row, int max_col) {
  if (current_position.position_row <= 0 ||
      current_position.position_row + sprite->height >= max_row)
    current_position.direction =
        flip_vertical_direction(current_position.direction);

  if (current_position.position_column <= 0 ||
      current_position.position_column + sprite->width >= max_col)
    current_position.direction =
        flip_horizontal_direction(current_position.direction);

  const MovementVector movement_vector =
      get_movement_vector(current_position.direction);

  current_position.position_row += movement_vector.dy;
  current_position.position_column += movement_vector.dx;

  return current_position;
}

void draw_sprite(const Sprite *sprite,
                 const SpritePositioning *spritePosition) {

  printf("\033[2J\033[H");
  draw_sprite_recursive(sprite, spritePosition, 0);
  fflush(stdout);
}

void draw_sprite_recursive(const Sprite *sprite,
                           const SpritePositioning *spritePosition,
                           unsigned short line_index) {
  if (line_index >= sprite->height)
    return;

  printf("\033[%d;%dH%s\n", spritePosition->position_row + line_index,
         spritePosition->position_column, sprite->lines[line_index]);

  draw_sprite_recursive(sprite, spritePosition, line_index + 1);
}

struct winsize get_terminal_size(void) {
  struct winsize terminal_size;

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_size);

  return terminal_size;
}

void main_animation(Sprite *sprite, SpritePositioning *spritePosition) {
  while (true) {
    struct winsize term_size = get_terminal_size();
    const int term_max_avaliable_columns = term_size.ws_col;
    const int term_max_avaliable_rows = term_size.ws_row;

    draw_sprite(sprite, spritePosition);

    *spritePosition =
        update_sprite_position(sprite, *spritePosition, term_max_avaliable_rows,
                               term_max_avaliable_columns);

    usleep(40000);
  }
}
