#include "shared.h"
#include <ctype.h>

enum TOKEN {
  /* keywords */
  TOKEN_setup,
  TOKEN_draw,

  TOKEN_window,
  TOKEN_title,

  TOKEN_clear,
  TOKEN_fill,
  TOKEN_rect,

  /* values */
  TOKEN_ident,
  TOKEN_integer,
  TOKEN_string,

  /* punctuation */
  TOKEN_leftCurly,
  TOKEN_rightCurly,
};

struct Token {
  enum TOKEN kind;
  char *value;
};

struct Token *makeToken(enum TOKEN kind, char *value);

struct Lexer {
  char *src;
  int idx;
};

struct Lexer *makeLexer(char *src);
struct Token *Lexer_next(struct Lexer *l);
struct Token *Lexer_eat(struct Lexer *l, enum TOKEN kind);
