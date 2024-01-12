#include "lexer.h"

struct Token *makeToken(enum TOKEN kind, char *value) {
  struct Token *p = malloc(sizeof(struct Token));

  p->kind = kind;
  p->value = value;

  return p;
}

struct Lexer *makeLexer(char *src) {
  struct Lexer *p = malloc(sizeof(struct Lexer));

  p->src = src;
  p->idx = 0;

  return p;
}

struct Token *Lexer_next(struct Lexer *l) {
  while (l->src[l->idx] && isspace(l->src[l->idx])) {
    l->idx++;
  }

  if (l->src[l->idx] == '\0') {
    return NULL;
  }

  if (isalpha(l->src[l->idx])) {
    char *value = malloc(80);
    char *vp = value;

    while (l->src[l->idx] && isalpha(l->src[l->idx])) {
      *vp++ = l->src[l->idx++];
    }
    *vp = '\0';

    if (strcmp(value, "setup") == 0) {
      return makeToken(TOKEN_setup, value);
    } else if (strcmp(value, "draw") == 0) {
      return makeToken(TOKEN_draw, value);
    } else if (strcmp(value, "title") == 0) {
      return makeToken(TOKEN_title, value);
    } else if (strcmp(value, "window") == 0) {
      return makeToken(TOKEN_window, value);
    } else if (strcmp(value, "clear") == 0) {
      return makeToken(TOKEN_clear, value);
    } else if (strcmp(value, "fill") == 0) {
      return makeToken(TOKEN_fill, value);
    } else if (strcmp(value, "rect") == 0) {
      return makeToken(TOKEN_rect, value);
    }

    return makeToken(TOKEN_ident, value);
  }

  if (isdigit(l->src[l->idx])) {
    char *value = malloc(80);
    char *vp = value;

    while (l->src[l->idx] && isdigit(l->src[l->idx])) {
      *vp++ = l->src[l->idx++];
    }
    *vp = '\0';

    return makeToken(TOKEN_integer, value);
  }

  if (l->src[l->idx] == '"') {
    char *value = malloc(80);
    char *vp = value;

    l->idx++;
    while (l->src[l->idx] && l->src[l->idx] != '"') {
      *vp++ = l->src[l->idx++];
    }
    *vp = '\0';
    l->idx++;

    return makeToken(TOKEN_string, value);
  }

  if (l->src[l->idx] == '{') {
    l->idx++;
    return makeToken(TOKEN_leftCurly, NULL);
  } else if (l->src[l->idx] == '}') {
    l->idx++;
    return makeToken(TOKEN_rightCurly, NULL);
  }

  die("unknown token, found %c", l->src[l->idx]);
}

struct Token *Lexer_eat(struct Lexer *l, enum TOKEN kind) {
  struct Token *next = Lexer_next(l);
  if (next == NULL) {
    die("ParsingErr: expected %d, found NULL", kind);
  }

  if (next->kind != kind) {
    die("ParsingErr: expected %d, found %d", kind, next->kind);
  }

  return next;
}
