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

typedef struct {
  short direction_x, direction_y;
  unsigned short position_column, position_row;
} SoccerBallPositioning;

typedef struct {
  char lines[MAX_ARR_LENGHT][MAX_STRING_LENGTH];
  unsigned short width, height;
} Sprite;

const Sprite load_sprite_from_file(const char *file_path);
const unsigned short load_sprite_lines(char linesArr[][MAX_STRING_LENGTH],
                                       FILE *opened_file,
                                       unsigned short *max_width);

int main() { return 0; }

const Sprite load_sprite_from_file(const char *file_path) {
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

const unsigned short load_sprite_lines(char linesArr[][MAX_STRING_LENGTH],
                                       FILE *opened_file,
                                       unsigned short *max_width) {

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
