#include "lexer.h"

struct Color {
  int r, g, b;
};

struct Setup {
  int width, height;
  char *title;
};

enum CMD {
  CMD_clear,
  CMD_fill,
  CMD_rect,
};

struct Rect {
  int x, y, w, h;
};

struct Cmd {
  enum CMD kind;

  union {
    struct Color clear;
    struct Color fill;
    struct Rect rect;
  };
};

struct Draw {
  struct Cmd *cmds;
  int cmdsLen;
};

struct Program {
  struct Setup setup;
  struct Draw draw;
};

struct Setup parseSetup(struct Lexer *l) {
  Lexer_eat(l, TOKEN_setup);
  Lexer_eat(l, TOKEN_leftCurly);

  Lexer_eat(l, TOKEN_window);
  int width = atoi(Lexer_eat(l)->value);
  int height = atoi(Lexer_eat(l)->value);

  Lexer_eat(l, TOKEN_title);
  char *title = Lexer_eat(l, TOKEN_string)->value;

  Lexer_eat(l, TOKEN_rightCurly);

  return (struct Setup){
      .width = width,
      .height = height,
      .title = title,
  };
}

struct Cmd parseCmdClear(struct Lexer *l)
{}

struct Draw parseDraw(struct Lexer *l) {
  struct Cmd *cmds = calloc(sizeof(struct Cmd));

  cmds[0] = parseCmdClear(l);
  cmds[1] = parseCmdFill(l);
  cmds[2] = parseCmdRect(l);

  return (struct Draw){
      .cmds = cmds,
      .cmdsLen = 3,
  };
}

struct Program parse(struct Lexer *l) {
  struct Setup setup = parseSetup(l);
  struct Draw draw = parseDraw(l);

  return (struct Program){
      .setup = setup,
      .draw = draw,
  };
}

int main(int argc, char **argv) {
  char *prg = argv[0];

  argc--;
  argv++;

  if (argc != 1) {
    die("usage: %s [file]", prg);
  }

  char *path = argv[0];
  struct FileContents source = readFileContents(path);

  struct Lexer *lexer = makeLexer(source.data);
  struct Token *token;
  while ((token = Lexer_next(lexer)) != NULL) {
    printf("token(%d, '%s')\n", token->kind, token->value);
  }
}
