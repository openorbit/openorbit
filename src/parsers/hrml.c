/*
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2009 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)openorbit.org>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
 */

#include "hrml.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <setjmp.h>
#include <execinfo.h>
#include <inttypes.h>



static inline HRMLobject*
makeInt(const char * restrict name, uint64_t i)
{
  HRMLobject *obj = malloc(sizeof(HRMLobject));
  obj->name = strdup(name);
  obj->val.alen = 0;
  obj->val.typ = HRMLInt;
  obj->val.u.integer = i;
  obj->children = NULL;
  obj->previous = NULL;
  obj->next = NULL;
  obj->parent = NULL;

  return obj;
}

static inline HRMLobject*
makeIntArray(const char * restrict name, uint64_t *i, size_t len)
{
  HRMLobject *obj = malloc(sizeof(HRMLobject));
  obj->name = strdup(name);
  obj->val.alen = len;
  obj->val.typ = HRMLIntArray;
  obj->val.u.intArray = calloc(len, sizeof(uint64_t));
  memcpy(obj->val.u.intArray, i, len*sizeof(uint64_t));

  obj->children = NULL;
  obj->previous = NULL;
  obj->next = NULL;
  obj->parent = NULL;

  return obj;
}


static inline HRMLobject*
makeFloatArray(const char * restrict name, double *d, size_t len)
{
  HRMLobject *obj = malloc(sizeof(HRMLobject));
  obj->name = strdup(name);
  obj->val.alen = len;
  obj->val.typ = HRMLFloatArray;
  obj->val.u.realArray = calloc(len, sizeof(double));
  memcpy(obj->val.u.realArray, d, len*sizeof(double));
  obj->children = NULL;
  obj->previous = NULL;
  obj->next = NULL;
  obj->parent = NULL;

  return obj;
}

static inline HRMLobject*
makeFloat(const char * restrict name, double d)
{
  HRMLobject *obj = malloc(sizeof(HRMLobject));
  obj->name = strdup(name);
  obj->val.alen = 0;
  obj->val.typ = HRMLFloat;
  obj->val.u.real = d;
  obj->children = NULL;
  obj->previous = NULL;
  obj->next = NULL;
  obj->parent = NULL;

  return obj;
}


static inline HRMLobject*
makeStr(const char * restrict name, const char * restrict s)
{
  HRMLobject *obj = malloc(sizeof(HRMLobject));
  obj->name = strdup(name);

  obj->val.alen = 0;
  obj->val.typ = HRMLStr;
  obj->val.u.str = strdup(s);

  obj->children = NULL;
  obj->previous = NULL;
  obj->next = NULL;
  obj->parent = NULL;

  return obj;
}

static inline HRMLobject*
makeBool(const char * restrict name, bool b)
{
  HRMLobject *obj = malloc(sizeof(HRMLobject));
  obj->name = strdup(name);

  obj->val.alen = 0;
  obj->val.typ = HRMLBool;
  obj->val.u.boolean = b;

  obj->children = NULL;
  obj->previous = NULL;
  obj->next = NULL;
  obj->parent = NULL;

  return obj;
}

static inline HRMLobject*
makeBoolArray(const char * restrict name, bool *b, size_t len)
{
  HRMLobject *obj = malloc(sizeof(HRMLobject));
  obj->name = strdup(name);

  obj->val.alen = len;
  obj->val.typ = HRMLBool;
  obj->val.u.boolArray = calloc(len, sizeof(bool));
  memcpy(obj->val.u.boolArray, b, len*sizeof(bool));

  obj->children = NULL;
  obj->previous = NULL;
  obj->next = NULL;
  obj->parent = NULL;

  return obj;
}

static inline HRMLobject*
makeNode(const char * restrict name)
{
  HRMLobject *obj = malloc(sizeof(HRMLobject));
  obj->children = NULL;
  obj->previous = NULL;
  obj->next = NULL;
  obj->parent = NULL;

  obj->val.alen = 0;  
  obj->name = strdup(name);
  obj->val.typ = HRMLNode;

  return obj;
}


// TODO: Ensure that line number and column number are availble for diagnostics

typedef enum TokenKind {
  tk_ident = 0,
  tk_true,
  tk_false,
  tk_int,
  tk_real,
  tk_str,
  tk_comment,
  tk_colon,
  tk_semi,
  tk_comma,
  tk_eq,
  tk_lbrace,
  tk_rbrace,
  tk_lparen,
  tk_rparen,
  tk_lbrack,
  tk_rbrack,
  tk_eof,
  tk_count
} TokenKind;

char * toks [] = {
  [tk_ident] = "ident",
  [tk_true] = "true",
  [tk_false] = "false",
  [tk_int] = "int",
  [tk_real] = "real",
  [tk_str] = "str",
  [tk_comment] = "comment",
  [tk_colon] = "colon",
  [tk_semi] = "semi",
  [tk_comma] = "comma",
  [tk_eq] = "eq",
  [tk_lbrace] = "lbrace",
  [tk_rbrace] = "rbrace",
  [tk_lparen] = "lparen",
  [tk_rparen] = "rparen",
  [tk_lbrack] = "lbrack",
  [tk_rbrack] = "rbrack",
  [tk_eof] = "eof",
  [tk_count] = "count"
};

typedef struct Token {
  TokenKind kind_;
  const char *start_;
  size_t len_;
} Token;

int64_t getInteger(Token tok)
{
  assert(tok.kind_ == tk_int);
  char str[tok.len_ + 1];
  memset(str, 0, tok.len_ + 1);
  // TODO: Fix support for negative values
  if (tok.start_[0] == '-') {
    
  }
  
  if (tok.len_ > 2) {
    if (tok.start_[0] == '0' && tok.start_[1] == 'b') {
      // Binary
      for (int i = 2, j = 0; i < tok.len_; ++ i) {
        // Copy string to temporary buffer, skipping any '_' that cannot be handled
        // by strtol
        if (tok.start_[i] != '_') {
          str[j] = tok.start_[i];
          ++ j;
        }
      }
      intmax_t val = strtoimax(str, NULL, 2);
      return val;
    } else if (tok.start_[0] == '0' && tok.start_[1] == 'o') {
      // Octal
      for (int i = 2, j = 0; i < tok.len_; ++ i) {
        // Copy string to temporary buffer, skipping any '_' that cannot be handled
        // by strtol
        if (tok.start_[i] != '_') {
          str[j] = tok.start_[i];
          ++ j;
        }
      }
      intmax_t val = strtoimax(str, NULL, 8);
      return val;
    } else if (tok.start_[0] == '0' && tok.start_[1] == 'x') {
      // Hex
      for (int i = 2, j = 0; i < tok.len_; ++ i) {
        // Copy string to temporary buffer, skipping any '_' that cannot be handled
        // by strtol
        if (tok.start_[i] != '_') {
          str[j] = tok.start_[i];
          ++ j;
        }
      }
      intmax_t val = strtoimax(str, NULL, 16);
      return val;
    }
  }
  
  // Normal base 10 value
  for (int i = 0, j = 0; i < tok.len_; ++ i) {
    // Copy string to temporary buffer, skipping any '_' that cannot be handled
    // by strtol
    if (tok.start_[i] != '_') {
      str[j] = tok.start_[i];
      ++ j;
    }
  }
  
  intmax_t val = strtoimax(str, NULL, 10);
  return val;
}

double getReal(Token tok)
{
  assert(tok.kind_ == tk_real);
  char str[tok.len_ + 1];
  memcpy(str, tok.start_, tok.len_);
  str[tok.len_] = '\0';
  double val = strtod(str, NULL);
  return val;
}

char* getString(Token tok)
{
  assert((tok.kind_ == tk_ident) || (tok.kind_ == tk_str));
  
  char *str = calloc(tok.len_ + 1, sizeof(char));
  if (!str) return NULL;
  
  // If this is a quoted string, strip the quotes
  if (*tok.start_ == '"') {
    memcpy(str, tok.start_ + 1, tok.len_ - 2);
  } else {
    memcpy(str, tok.start_, tok.len_);
  }
  return str;
}

void printToken(Token tok)
{
  char str[tok.len_ + 1];
  
  memcpy(str, tok.start_, tok.len_);
  str[tok.len_] = '\0';
  
  fprintf(stderr, "'%s'", str);
}


Token makeTok(TokenKind kind, const char *start, size_t len)
{
  Token tok = {kind, start, len}; 
  return tok;
}

HRMLvalue makeVal(Token tok)
{
  HRMLvalue val;
  
  switch (tok.kind_) {
  case tk_ident:
  case tk_str:
    val.typ = HRMLStr;
    val.u.str = getString(tok);
    break;
  case tk_int:
    val.typ = HRMLInt;
    val.u.integer = getInteger(tok);
    break;
  case tk_real:
    val.typ = HRMLFloat;
    val.u.real = getReal(tok);
    break;
  //    HRMLDate,
//      HRMLBool,
//      HRMLIntArray,
//      HRMLFloatArray,
//      HRMLBoolArray,
//      HRMLNull
  case tk_true:
    val.typ = HRMLBool;
    val.u.boolean = true;
    break;
  case tk_false:
    val.typ = HRMLBool;
    val.u.boolean = false;
    break;
  case tk_comment:
  case tk_colon:
  case tk_semi:
  case tk_comma:
  case tk_eq:
  case tk_lbrace:
  case tk_rbrace:
  case tk_lparen:
  case tk_rparen:
  case tk_lbrack:
  case tk_rbrack:
  case tk_eof:
  case tk_count:
  default:
    assert(0 && "invalid token, cannot make value");
  }
  
  return val;
}

static jmp_buf gParseError;

typedef struct LexState {
  const char *startPtr_; // Start of file
  const char *endPtr_; // End of file
  const char *rdPtr_; // Current read pointer for file
  size_t fileLen_; // Total length of file in bytes

  const char *tokPtr_; // Pointer to current token
  const char *nextTokPtr_; // Pointer to next token pointer
  
  Token currTok_;
  Token nextTok_;
  int fd;
  int line_;
  int col_;
  wchar_t currCh_;
} LexState;

typedef struct ParseState {
  LexState *lexer;
  bool errors;
  HRMLdocument *doc; //!< Available so that we do not lose this entry
  HRMLobject *obj; //!< Need to know where parser inserts the entries
} ParseState;

Token createNextToken(TokenKind kind, LexState *lex) {
  Token next = makeTok(kind, lex->nextTokPtr_,
                       (size_t)(lex->rdPtr_ - lex->nextTokPtr_));
  lex->nextTok_ = next;
  
  return next;
}
  
static inline size_t min(size_t a, size_t b) {
  return (a < b) ? a : b;
}

static inline wchar_t lexGetCurrChar(LexState *lex)
{
  return lex->currCh_;
}

off_t getFileLen(const char *path)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    return 0;
  }
  
  off_t len = lseek(fd, 0, SEEK_END);
  close(fd);
  
  fprintf(stderr, "file %s: %u B\n", path, len);
  
  return len;
}

LexState*
newLex(const char *file)
{
  off_t len = getFileLen(file);
  if (len == 0) {
    // No data or no file with that name
    return NULL;
  }
  
  LexState *lex = malloc(sizeof(LexState));
  int fd = open(file, O_RDONLY);
  if (fd == -1) {
    perror(NULL);
    assert(0 && "file not opened");
  }
  lex->fd = fd;
  lex->startPtr_ = mmap(NULL, (size_t)len, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
  if (lex->startPtr_ == (char*)-1) {
    perror(NULL);
    assert(lex->startPtr_ != (char*)-1);
  }

  lex->endPtr_ = lex->startPtr_ + len;
  lex->fileLen_ = len;
  lex->rdPtr_ = lex->startPtr_;
  lex->tokPtr_ = NULL;
  lex->nextTokPtr_ = NULL;

  lex->line_ = 1;
  lex->col_ = 0;

  return lex;
}

void
deleteLex(LexState *lex)
{
  munmap((void*)lex->startPtr_, lex->fileLen_);
  close(lex->fd);
  memset(lex, 0, sizeof(LexState)); // invalidate all pointers, though in principle the 
                                    // unmap does this, but if someone does another mmap,
                                    // they may become valid again
  free(lex);
}
Token lexToken(LexState *lex);

// Reads next character and consumes it
wchar_t
lexGetChar(LexState *lex)
{
  wchar_t ch;
  if (lex->fileLen_ - (lex->rdPtr_ - lex->startPtr_) <= 0) {
    return WEOF;
  }
  
  int count = mbtowc(&ch, lex->rdPtr_ , min(16, lex->fileLen_ - (lex->rdPtr_ - lex->startPtr_)));
  if (count == -1) {
    fprintf(stderr, "invalid file, for now lets just die\n");
    exit(1);
  }
  
  lex->rdPtr_ += count;
  lex->col_ ++;
  
  if (ch == '\n') {
    lex->line_ ++;
    lex->col_ = 0;
  }
  
  lex->currCh_ = ch;
  
  return ch;
}

// Reads next character without consuming it
wchar_t
lexPeekChar(LexState *lex)
{
  wchar_t ch;
  if (lex->fileLen_ - (lex->rdPtr_ - lex->startPtr_) <= 0) {
    return WEOF;
  }

  int count = mbtowc(&ch, lex->rdPtr_ , min(16, lex->fileLen_ - (lex->rdPtr_ - lex->startPtr_)));
  if (count == -1) {
    fprintf(stderr, "invalid chars in file, for now lets just die\n");
    exit(1);
  }
  
  return ch;
}

void
lexConsumeTok(LexState *lex)
{
  lex->tokPtr_ = lex->nextTokPtr_;
  lex->currTok_ = lex->nextTok_;
}

Token
lexPeekTok(LexState *lex)
{
  if (lex->tokPtr_ == lex->nextTokPtr_) {
    do {
      lex->nextTokPtr_ = lex->rdPtr_;
      lex->nextTok_ = lexToken(lex);
    } while (lex->nextTok_.kind_ == tk_comment); // Ignore comments
    
  }
  return lex->nextTok_;
}

Token lexGetNextTok(LexState *lex)
{
  Token tok = lexPeekTok(lex);
  lexConsumeTok(lex);
  return tok;
}

Token lexGetCurrentTok(LexState *lex)
{
  return lex->currTok_;
}

void
lexConsumeWS(LexState *lex)
{
  while (iswspace(lexPeekChar(lex))) {
    lexGetChar(lex);
    lex->nextTokPtr_ = lex->rdPtr_;
  }
}

Token lexIdent(LexState *lex)
{
  while (iswalnum(lexPeekChar(lex)) || lexPeekChar(lex) == '_') {
    lexGetChar(lex);
  }

  if ((lex->rdPtr_ - lex->nextTokPtr_ == 4)
      && !memcmp(lex->nextTokPtr_, "true", 4)) {
    return createNextToken(tk_true, lex);
  } else if ((lex->rdPtr_ - lex->nextTokPtr_ == 5)
             && !memcmp(lex->nextTokPtr_, "false", 5)) {
    return createNextToken(tk_false, lex);
  }

  return createNextToken(tk_ident, lex);
}

Token lexString(LexState *lex)
{
  while ((lexPeekChar(lex) != '"') && (lexPeekChar(lex) != WEOF)) {
    wchar_t ch = lexGetChar(lex);
    if (ch == '\\') {
      lexGetChar(lex);
    }
  }

  if (lexPeekChar(lex) == '"') {
    lexGetChar(lex); // consume last quote in string
    return createNextToken(tk_str, lex);
  } else {
    return createNextToken(tk_eof, lex);
  }
  
  assert(0 && "invalid string");
}
Token lexReal(LexState *lex)
{
  lexGetChar(lex);
  while (iswnumber(lexPeekChar(lex)) || lexPeekChar(lex) == '_') {
    lexGetChar(lex);
  }
  if (lexPeekChar(lex) == 'e') {
    lexGetChar(lex);
    if (lexPeekChar(lex) == '-') lexGetChar(lex);
    if (!iswnumber(lexPeekChar(lex))) {
      // unget
    } else {
      while (iswnumber(lexPeekChar(lex))) {
        lexGetChar(lex);
      }
    }
  }

  return createNextToken(tk_real, lex);
}

Token lexNum(LexState *lex)
{
  if ( lexGetCurrChar(lex) == '0') {
    if ( lexPeekChar(lex) == 'x') {
       lexGetChar(lex);
      while (iswxdigit( lexPeekChar(lex)) ||  lexPeekChar(lex) == '_') {
         lexGetChar(lex);
      }
      return createNextToken(tk_int, lex);
    } else if ( lexPeekChar(lex) == 'b') {
      // binary number
       lexGetChar(lex);
      while (( lexPeekChar(lex) == '0') || ( lexPeekChar(lex) == '1') || ( lexPeekChar(lex) == '_')) {
         lexGetChar(lex);
      }
      return createNextToken(tk_int, lex);
    } else if ( lexPeekChar(lex) == 'o') {
      // octal
       lexGetChar(lex);
      while ((( lexPeekChar(lex) >= '0') && ( lexPeekChar(lex) <= '7')) || ( lexPeekChar(lex) == '_')) {
         lexGetChar(lex);
      }
      return createNextToken(tk_int, lex);
    }
  }

  while (iswnumber(lexPeekChar(lex)) ||  lexPeekChar(lex) == '_') {
    lexGetChar(lex);
  }

  if (lexPeekChar(lex) == '.') {
    return lexReal(lex);
  }
  
  return createNextToken(tk_int, lex); // Otherwise this is an integer
  

  assert(0 && "invalid number");
}

Token lexComment(LexState *lex)
{
  if (lexPeekChar(lex) == '*') {
    int nestLevel = 1;

    while ((lexPeekChar(lex) != WEOF)) {
      wchar_t ch = lexGetChar(lex);
      if (ch == '/') {
        if (lexPeekChar(lex) == '*') {
          lexGetChar(lex);
          nestLevel ++;
        }
      }
      if (ch == '*') {
        if (lexPeekChar(lex) == '/') {
          lexGetChar(lex);
          nestLevel --;
          if (nestLevel == 0) return createNextToken(tk_comment, lex);
        }
      }
    }
  } else if (lexPeekChar(lex) == '/') {
    while ((lexPeekChar(lex) != '\n') && (lexPeekChar(lex) != WEOF)) lexGetChar(lex);
    return createNextToken(tk_comment, lex);
  } else if (lexGetCurrChar(lex) == '#') {
    while ((lexPeekChar(lex) != '\n') && (lexPeekChar(lex) != WEOF)) lexGetChar(lex);
    return createNextToken(tk_comment, lex);
  }

  return createNextToken(tk_eof, lex);
}

Token lexPunct(LexState *lex)
{
  switch (lexGetCurrChar(lex)) {
  case '[':
    return createNextToken(tk_lbrack, lex);
  case ']':
    return createNextToken(tk_rbrack, lex);
  case '{':
    return createNextToken(tk_lbrace, lex);
  case '}':
    return createNextToken(tk_rbrace, lex);
  case '(':
    return createNextToken(tk_lparen, lex);
  case ')':
    return createNextToken(tk_rparen, lex);
  case ':':
    return createNextToken(tk_colon, lex);
  case ';':
    return createNextToken(tk_semi, lex);
  case ',':
    return createNextToken(tk_comma, lex);
  case '/':
  case '#':
    // Comments
    return lexComment(lex);
  default:
    // Error
    ;
  }

  return createNextToken(tk_eof, lex);
}


Token lexToken(LexState *lex)
{  
  lexConsumeWS(lex);

  wchar_t ch = lexGetChar(lex);

  if (iswalpha(ch)) {
    return lexIdent(lex);
  } else if (iswdigit(ch)) {
    return lexNum(lex);
  } else if (ch == '"') {
    return lexString(lex);
  } else if (iswpunct(ch)) {
    return lexPunct(lex);
  } else if (ch == WEOF) {
    fprintf(stderr, "eof\n");
    return createNextToken(tk_eof, lex);
  } else {
    fprintf(stderr, "unknown character '"); fputwc(ch, stderr); fprintf(stderr, "'\n");
    assert(0 && "unknown ch in stream");
  }
}


//typedef struct ParseState {
//  LexState *lexer;
//  bool errors;
//  HRMLdocument *doc; //!< Available so that we do not lose this entry
//  HRMLobject *obj; //!< Need to know where parser inserts the entries
//} ParseState;


void
InsertNewNode(ParseState *parser, const char *name)
{
  HRMLvalue val;  
  
  
  if (parser->obj == NULL) {
    
    parser->doc->rootNode = malloc(sizeof(HRMLobject));
    parser->obj = parser->doc->rootNode;
    parser->obj->name = strdup(name);
    parser->obj->parent = NULL;
    parser->obj->next = NULL;
    parser->obj->previous = NULL;
    parser->obj->children = NULL;
    parser->obj->attr = NULL;
  } else {
    HRMLobject *parent = parser->obj;
    HRMLobject *newObj = malloc(sizeof(HRMLobject));
    
    newObj->name = strdup(name);
    newObj->parent = parent;
    newObj->next = parent->children;    
    newObj->previous = NULL;
    newObj->attr = NULL;
    newObj->children = NULL;
    
    if (parent->children) {
      parent->children->previous = newObj;
    }
    parent->children = newObj;
    parser->obj = newObj;
  }
}

void
GotoParentNode(ParseState *parser)
{
  parser->obj = parser->obj->parent;
}


void
InsertAttribute(ParseState *parser, const char *name, Token tok)
{
  HRMLattr *attr = malloc(sizeof(HRMLattr));
  attr->name = strdup(name);

  if (tok.kind_ == tk_int) {
    attr->val.alen = 0;
    attr->val.typ = HRMLInt;
    attr->val.u.integer = getInteger(tok);
  } else if (tok.kind_ == tk_int) {
    attr->val.alen = 0;
    attr->val.typ = HRMLFloat;
    attr->val.u.real = getReal(tok);
  } else if (tok.kind_ == tk_str) {
    attr->val.alen = 0;
    attr->val.typ = HRMLStr;
    attr->val.u.str = getString(tok);
  } else if (tok.kind_ == tk_ident) {
    attr->val.alen = 0;
    attr->val.typ = HRMLStr;
    attr->val.u.str = getString(tok);
  }

  attr->next = parser->obj->attr;
  parser->obj->attr = attr;
}

void
InsertValueInNode(ParseState *parser, HRMLvalue val)
{
  parser->obj->val = val;
}


static void ParseErr(const char *str, ...)
{
  char msg[strlen(str)+sizeof("error: ")+2];
  strncpy(msg, "error: ", 8);
  strncat(msg, str, strlen(str));
  strncat(msg, "\n", 1);
  va_list args;
  va_start(args, str);

  vfprintf(stderr, msg, args);
  va_end(args);
  
  longjmp(gParseError, 1);
}

static void ParseWarning(const char *str)
{
  fprintf(stderr, "warning: %s\n", str);  
}

// Checks if next token is of the given kind, if not, longjump to parser error handler
static void Require(ParseState *parser, TokenKind kind)
{
  
  
  if (lexPeekTok(parser->lexer).kind_ == kind) {
    fprintf(stderr,
            "Require(%s) == %s: ",
            toks[kind], toks[lexPeekTok(parser->lexer).kind_]);
    printToken(lexPeekTok(parser->lexer));
    fprintf(stderr, "\n");

    lexGetNextTok(parser->lexer);
    return;
  }

  fprintf(stderr,
          "Require(%s) != %s: ",
          toks[kind], toks[lexPeekTok(parser->lexer).kind_]);
  printToken(lexPeekTok(parser->lexer));
  fprintf(stderr, "\n");
  
  void *callstack[128];
  int frames = backtrace(callstack, 128);
  char **strs = backtrace_symbols(callstack, frames);
  for (int i = 0 ; i < frames; ++ i) {
    fprintf(stderr, "%s\n", strs[i]);
  }
  
  ParseErr("token '%s' expected", toks[kind]);
}

// Checks if next token is of the given kind, if so consume token and return true
static bool Optional(ParseState *parser, TokenKind kind)
{
  if (lexPeekTok(parser->lexer).kind_ == kind) {
    fprintf(stderr,
            "Optional(%s) == %s: ",
            toks[kind], toks[lexPeekTok(parser->lexer).kind_]);
    printToken(lexPeekTok(parser->lexer));
    fprintf(stderr, "\n");
    
    lexGetNextTok(parser->lexer);
    return true;
  }


  fprintf(stderr,
          "Optional(%s) != %s: ",
          toks[kind], toks[lexPeekTok(parser->lexer).kind_]);
  printToken(lexPeekTok(parser->lexer));
  fprintf(stderr, "\n");

  return false;
}

// Peeks for next token without consuming
static bool Peek(ParseState *parser, TokenKind kind)
{
  fprintf(stderr,
          "Peek(%s) ?? %s: ",
          toks[kind], toks[lexPeekTok(parser->lexer).kind_]);
  printToken(lexPeekTok(parser->lexer));
  fprintf(stderr, "\n");


  if (lexPeekTok(parser->lexer).kind_ == kind) {
    return true;
  }

  if (lexPeekTok(parser->lexer).kind_ == tk_eof) {
    ParseErr("premature eof");
  }
  return false;
}

void 
ParseAttributes(ParseState *parser)
{
  // May be an empty attribute list
  if (!Peek(parser, tk_rparen)) {
    do {
      Require(parser, tk_ident);
      Token nameTok = lexGetCurrentTok(parser->lexer);

      Require(parser, tk_colon);
      Require(parser, tk_ident);
      Token valueTok = lexGetCurrentTok(parser->lexer);

      char *name = getString(nameTok);
      InsertAttribute(parser, name, valueTok);
      free(name);
    } while (Optional(parser, tk_comma));
  }
}

void ParseVal(ParseState *parser)
{
  Require(parser, tk_ident);

  Token ident = lexGetCurrentTok(parser->lexer);
  char *valName = getString(ident);
  InsertNewNode(parser, valName);
  free(valName);

  if (Optional(parser, tk_lparen)) {
    ParseAttributes(parser);
    Require(parser, tk_rparen);
  }

  if (Optional(parser, tk_lbrace)) {
    // Aggregated value
    // Parse each sub value by recursive call
    while (!Peek(parser, tk_rbrace)) {
      ParseVal(parser);
    }
    Require(parser, tk_rbrace);
  } else if (Optional(parser, tk_colon)) {
    // Primitive value    
    if (Optional(parser, tk_str)) {
      Token str = lexGetCurrentTok(parser->lexer);
      HRMLvalue val = makeVal(str);
      InsertValueInNode(parser, val);      
    } else if (Optional(parser, tk_int)) {
      Token integer = lexGetCurrentTok(parser->lexer);
      HRMLvalue val = makeVal(integer);
      InsertValueInNode(parser, val);
    } else if (Optional(parser, tk_real)) {
      Token real = lexGetCurrentTok(parser->lexer);
      if (Optional(parser, tk_ident)) {
        // Got unit
      }
      HRMLvalue val = makeVal(real);
      InsertValueInNode(parser, val);
    } else if (Optional(parser, tk_lbrack)) {

      while (!Peek(parser, tk_rbrack)) {
        // Ensure that types are identical for all subvalues
      }
      Require(parser, tk_rbrack);
    }
    
    Require(parser, tk_semi);
  } else {
    ParseErr("object missing data");
  }
  
  GotoParentNode(parser);
}

void Parse2(ParseState *parser)
{
  ParseVal(parser);
}

// New parser function
void Parse(ParseState *parser)
{
  int res = 0;
  if (res = setjmp(gParseError)) {
    parser->errors = true;
    // Longjumped here, error code is in res
  } else {
    Parse2(parser);
  }
}

ParseState *newParser(const char *path)
{
  assert(path != NULL);
  
  ParseState *parser = malloc(sizeof(ParseState));
  parser->lexer = newLex(path);
  parser->errors = false;
  parser->doc = NULL;
  parser->obj = NULL;
  return parser;
}


void deleteParser(ParseState *parser)
{  
  deleteLex(parser->lexer);
  free(parser);
}

HRMLdocument *hrmlParse(const char *path)
{
  assert(path != NULL);
  ParseState *parser = newParser(path);
  parser->doc = malloc(sizeof(HRMLdocument));
  parser->doc->rootNode = NULL;
  
  Parse(parser);
  
  HRMLdocument *doc = parser->doc;
  if (parser->errors) { // Incase of parsing errors, return NULL
    fprintf(stderr, "parse errors, freeing doc\n");
    hrmlFreeDocument(doc);
    doc = NULL;
  }
  deleteParser(parser);

  return doc;
}

// Used to build up parsing tables for the si-unit parser
static const char * gPrefixTable[HRML_siprefix_size] = {
    [HRML_yocto] = "y",
    [HRML_zepto] = "z",
    [HRML_atto] = "a",
    [HRML_femto] = "f",
    [HRML_pico] = "p",
    [HRML_nano] = "n",
    [HRML_micro] = "u",
    [HRML_milli] = "m",
    [HRML_centi] = "c",
    [HRML_deci] = "d",
    [HRML_none] = "",
    [HRML_deca] = "da",
    [HRML_hecto] = "h",
    [HRML_kilo] = "k",
    [HRML_mega] = "M",
    [HRML_giga] = "G",
    [HRML_tera] = "T",
    [HRML_peta] = "P",
    [HRML_exa] = "E",
    [HRML_zetta] = "Z",
    [HRML_yotta] = "Y"
};

// Used to build up parsing tables for the si-unit parser
static const char * gUnitTable[HRML_siunit_size] = {
  [HRML_metre] = "m",
  [HRML_gram] = "g",
  [HRML_second] = "s",
  [HRML_ampere] = "A",
  [HRML_kelvin] = "K",
  [HRML_mole] = "mol",
  [HRML_candela] = "cd",

  [HRML_hertz] = "Hz",
  [HRML_radian] = "rad",
  [HRML_steradian] = "sr",
  [HRML_newton] = "N",
  [HRML_pascal] = "Pa",
  [HRML_joule] = "J",
  [HRML_watt] = "W",
  [HRML_coulomb] = "C",
  [HRML_volt] = "V",
  [HRML_farad] = "F",
  [HRML_ohm] = "Ohm",
  [HRML_siemens] = "S",
  [HRML_weber] = "Wb",
  [HRML_tesla] = "T",
  [HRML_henry] = "H",
  [HRML_celsius] = "deg C",
  [HRML_lumen] = "lm",
  [HRML_lux] = "lx",
  [HRML_becquerel] = "Bq",
  [HRML_gray] = "Gy",
  [HRML_sievert] = "Sv",
  [HRML_katal] = "kat",

  [HRML_minute] = "min",
  [HRML_hour] = "h",
  [HRML_day] = "d",
  [HRML_arcdeg] = "deg arc",
  [HRML_arcminute] = "'",
  [HRML_arcsec] = "''",
  [HRML_hectare] = "ha",
  [HRML_litre] = "L",
  [HRML_tonne] = "t",
  [HRML_electronvolt] = "eV",
  [HRML_atomic_mass_unit] = "u",
  [HRML_astronomical_unit] = "AU",
  [HRML_parsec] = "pc",
  [HRML_lightyear] = "ly",
  [HRML_bar] = "bar"
};

/* Parsing the si units is a bit tricky as it in principle requires that
  we parse the unit first and then the prefix due to ambiguities with the
  derived units. E.g. dd is in principle deci-day, though this is in princple
  not allowed, we have to treat this case if we want a general parser.

  We build up parse tables that start from the back of the strings and then
  fork of in sparse trees to resolve the units
*/

typedef struct parse_tree_t {
  char ch;
  HRMLsitype sitype; // If this is the final entry for this, otherwise invalid
  size_t childCount;
  struct parse_tree_t **children;
} parse_tree_t;

parse_tree_t *si_unit_trees[28];

static void
build_si_unit_parse_tables(void)
{
  memset(si_unit_trees, 0, sizeof(si_unit_trees));
  for (size_t i = 0 ; i < HRML_siunit_size ; i ++ ) {
    size_t slen = strlen(gUnitTable[i]);
    const char *endc = gUnitTable[i] + slen - 1;

    // Create new root if neccissary
    if (si_unit_trees[*endc] == NULL) {
      si_unit_trees[*endc] = malloc(sizeof(parse_tree_t));
      si_unit_trees[*endc]->ch = *endc;
      si_unit_trees[*endc]->sitype = HRML_siunit_invalid;
      si_unit_trees[*endc]->childCount = 0;
      si_unit_trees[*endc]->children = NULL;
    }

    // Go down in the tree, adding additional nodes if neccissary
    parse_tree_t *final_entry = si_unit_trees[*endc];
    do {
      endc --;
    } while (endc >= gUnitTable[i]);
    final_entry->sitype = i;
  }
}

static void
parse_si_unit(const char *unit)
{

}


static int
isbindigit(int c)
{
  if (c == '0') return 1;
  if (c == '1') return 1;
  return 0;
}

static int
isop(int c)
{
  if (c == '[') return 1;
  if (c == ']') return 1;
  if (c == '{') return 1;
  if (c == '}') return 1;
  if (c == '(') return 1;
  if (c == ')') return 1;
  if (c == ':') return 1;
  if (c == ';') return 1;
  if (c == ',') return 1;

  return 0;
}


// Checks date for validity, asserts that month is between 1 and 12 and that the
// day is in the valid range for that month, taking leap years into account
#if 0
static inline bool
isValidDate(int64_t year, int month, int day)
{
  if (month == 2 && IS_LEAP_YEAR(year)) {
    if (day <= 29) {
      return true;
    } else {
      return false;
    }
  } else if (month == 2) {
    if (day <= 28) {
      return true;
    } else {
      return false;
    }
  } else if (month == 1 || month == 3 || month == 5 ||
             month == 7 || month == 8 || month == 10 ||
             month == 12) {
    if (day <= 31) {
      return true;
    } else {
      return false;
    }
  } else if (month == 4 || month == 6 || month == 9 ||
             month == 11) {
    if (day <= 31) {
      return true;
    } else {
      return false;
    }
  }
  return false;
}



HRMLobject*
hrmlParsePrimitiveValue(FILE *f, const char *sym)
{
  HRMLtoken firstTok = hrmlLex(f);
  if (IS_INTEGER(firstTok)) {
    HRMLtoken minusOrSemi = hrmlLex(f);

    if (IS_CHAR(minusOrSemi, ';')) {
      // Normal integer
      return makeInt(sym, INTEGER(firstTok));
    } else if (IS_CHAR(minusOrSemi, '-')) {
      // This should be a date object
      HRMLtoken month = hrmlLex(f);
      if (IS_BOUNDED_INTEGER(month, 1, 12)) {
        HRMLtoken minus1 = hrmlLex(f);
        if (IS_CHAR(minus1, '-')) {
          HRMLtoken day = hrmlLex(f);
          if (IS_BOUNDED_INTEGER(day, 1, 31)) {
            HRMLtoken semiOrHour = hrmlLex(f);
            if (IS_CHAR(semiOrHour, ';')) {
              // Date ok... sort of, we still need to verify the days in the
              // month
              int64_t yearNum = INTEGER(firstTok);
              int monthNum = INTEGER(month);
              int dayNum = INTEGER(day);
              if (isValidDate(yearNum, monthNum, dayNum)) {
                fprintf(stderr, "found date: %lld-%d-%d\n",
                                 yearNum, monthNum, dayNum);
              }
            } else if (IS_BOUNDED_INTEGER(semiOrHour, 0, 23)) {

            }
          }
        }
      }
    } else {
      // Parse error
      ParseError("after int %llx\n", INTEGER(firstTok));
    }
  } else if (IS_REAL(firstTok)) {
    HRMLtoken unitOrSemi = hrmlLex(f);
    if (IS_CHAR(unitOrSemi, ';')) {
      return makeFloat(sym, REAL(firstTok));
      //fprintf(stderr, "found real: %f\n", REAL(firstTok));
    } else if (IS_SYM(unitOrSemi)) {
      //fprintf(stderr, "found real with unit: %f %s\n",
      //                REAL(firstTok), SYM(unitOrSemi));
      HRMLtoken semi = hrmlLex(f);
      if (!IS_CHAR(semi, ';')) {
        ParseError("expected ';'");
        return NULL;
      }
      fprintf(stderr, "WARNING: Units not supported, returning float\n");
      return makeFloat(sym, REAL(firstTok));
    }
  } else if (IS_STR(firstTok)) {
    HRMLtoken semi = hrmlLex(f);
    if (IS_CHAR(semi, ';')) {
      return makeStr(sym, STR(firstTok));
    }
  } else if (IS_BOOLEAN(firstTok)) {
    HRMLtoken semi = hrmlLex(f);
    if (IS_CHAR(semi, ';')) {
      return makeBool(sym, BOOLEAN(firstTok));
    }
  } else if (IS_CHAR(firstTok, '[')) {
    // Parse primitive value array (only bools, reals and integers)
  } else {
    ParseError("unknown token detected\n");
  }


  return NULL;
}
#endif


static inline void indent(FILE *fp, int indentlev)
{
  for (int i = 0 ; i < indentlev ; i ++) {
    fputc('\t', fp);
  }
}


bool
hrmlValidate(HRMLdocument *doc, HRMLschema *sc)
{

}

void
hrmlFreeObj(HRMLobject *obj)
{
  // TODO: Free strings as well
  assert(obj != NULL);

  if (obj->val.typ == HRMLNode) {
    HRMLobject *child = obj->children;
    while (child) {
      hrmlFreeObj(child);
      child = child->next;
    }
  }
  
  free(obj);
}

void
hrmlFreeDocument(HRMLdocument *doc)
{
  hrmlFreeObj(doc->rootNode);
  free(doc);
}


HRMLobject*
hrmlGetObject(HRMLdocument *doc, const char *docPath)
{
  char str[strlen(docPath)+1];
  strcpy(str, docPath); // TODO: We do not trust the user, should probably
                        // check alloca result

  HRMLobject *obj = hrmlGetRoot(doc);
  char *strp = str;
  char *strTok = strsep(&strp, "/");
  while (obj) {
    if (!strcmp(obj->name, strTok)) {
      if (strp == NULL) {
        // At the end of the doc path
        return obj;
      }

      // If this is not the lowest level, go one level down
      strTok = strsep(&strp, "/");
      obj = obj->children;
    } else {
      obj = obj->next;
    }
  }
  return NULL;
}

void
writeNode(HRMLobject *obj, FILE *fp, int lev)
{
  indent(fp, lev);
  fprintf(fp, "%s", obj->name);
  
  // Print out attributes
  if (obj->attr) {
    fprintf(fp, " (");
    for (HRMLattr *attr = obj->attr; attr != NULL; attr = attr->next) {
      fprintf(fp, "%s: ", attr->name);
      switch (attr->val.typ) {
      case HRMLInt:
        fprintf(fp, "%"PRIi64, attr->val.u.integer);
        break;
      case HRMLFloat:
        fprintf(fp, "%f", attr->val.u.real);
        break;
      case HRMLStr:
        fprintf(fp, "\"%s\"", attr->val.u.str);
        break;
      case HRMLDate:
        assert(0 && "date support not yet implemented");
        break;
      case HRMLBool:
        if (attr->val.u.boolean) fprintf(fp, "true");
        else fprintf(fp, "false");
        break;
      case HRMLIntArray:
        fprintf(fp, "[");
        fprintf(fp, "]");
        assert(0 && "array support not yet implemented");
        break;
      case HRMLFloatArray:
        fprintf(fp, "[");
        fprintf(fp, "]");
        assert(0 && "array support not yet implemented");
        break;
      case HRMLBoolArray:
        fprintf(fp, "[");
        fprintf(fp, "]");
        assert(0 && "array support not yet implemented");
        break;
      default:
        assert(0 && "invalid attribute");
      }
      
      if (attr->next) fprintf(fp, ", ");
    }
    fprintf(fp, ")");
  }

  
  if (obj->children) {
    fprintf(fp, " {\n");
    for (HRMLobject *child = obj->children; child != NULL; child = child->next) {
      writeNode(child, fp, lev + 1);
    }
    indent(fp, lev);
    fprintf(fp, "}\n");
  } else {
    fprintf(fp, ": ");
    switch (obj->val.typ) {
    case HRMLInt:
      fprintf(fp, "%"PRIi64, obj->val.u.integer);
      break;
    case HRMLFloat:
      fprintf(fp, "%f", obj->val.u.real);
      break;
    case HRMLStr:
      fprintf(fp, "\"%s\"", obj->val.u.str);
      break;
    case HRMLDate:
      assert(0 && "date support not yet implemented");
      break;
    case HRMLBool:
      if (obj->val.u.boolean) fprintf(fp, "true");
      else fprintf(fp, "false");
      break;
    case HRMLIntArray:
      fprintf(fp, "[");
      fprintf(fp, "]");
      assert(0 && "array support not yet implemented");
      break;
    case HRMLFloatArray:
      fprintf(fp, "[");
      fprintf(fp, "]");
      assert(0 && "array support not yet implemented");
      break;
    case HRMLBoolArray:
      fprintf(fp, "[");
      fprintf(fp, "]");
      assert(0 && "array support not yet implemented");
      break;
    default:
      assert(0 && "invalid attribute");
    }
    fprintf(fp, ";\n");
  }
}
void
hrmlWriteDocument(HRMLdocument *doc, FILE *file)
{
  HRMLobject *obj = doc->rootNode;  
  writeNode(obj, file, 0);
}
