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

// TODO: Ensure that line number and column number are availble for diagnostics

typedef enum TokenKind {
  tk_ident = 0,
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

uint64_t getInteger(Token tok)
{
  assert(tok.kind_ == tk_int);
}

double getReal(Token tok)
{
  assert(tok.kind_ == tk_real);
  
}

char* getString(Token tok)
{
  assert((tok.kind_ == tk_ident) || (tok.kind_ == tk_str));
  
  char *str = calloc(tok.len_ + 1, sizeof(char));
  if (!str) return NULL;
  
  memcpy(str, tok.start_, tok.len_);
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
  
  fprintf(stderr, "returning identifier\n");
  
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

static void ParseErr(const char *str)
{
  fprintf(stderr, "error: %s\n", str);
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
  
  ParseErr("token 'xxx' expected");
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
      Require(parser, tk_colon);
      Require(parser, tk_ident);
      
    } while (Optional(parser, tk_comma));
  }
}

void ParseVal(ParseState *parser)
{
  Require(parser, tk_ident);
  
  Token ident = lexGetCurrentTok(parser->lexer);

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
    fprintf(stderr, "colon found\n");
    if (Optional(parser, tk_str)) {
      fprintf(stderr, "string??\n");
      Token str = lexGetCurrentTok(parser->lexer);
    } else if (Optional(parser, tk_int)) {
      fprintf(stderr, "int??\n");
      Token integer = lexGetCurrentTok(parser->lexer);
    } else if (Optional(parser, tk_real)) {
      fprintf(stderr, "real??\n");
      Token real = lexGetCurrentTok(parser->lexer);
      if (Optional(parser, tk_ident)) {
        // Got unit
      }
    } else if (Optional(parser, tk_lbrack)) {
      fprintf(stderr, "bracket??\n");
      
      while (!Peek(parser, tk_rbrack)) {
        // Ensure that types are identical for all subvalues
      }
      Require(parser, tk_rbrack);
    }
    fprintf(stderr, "endof val??\n");
    
    Require(parser, tk_semi);
  } else {
    ParseErr("object missing data");
  }
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


HRMLdocument *hrmlParseNew(const char *path)
{
  assert(path != NULL);
  ParseState *parser = newParser(path);
  parser->doc = malloc(sizeof(HRMLdocument));
  parser->doc->rootNode = NULL;
  
  Parse(parser);
  
  HRMLdocument *doc = parser->doc;
  if (parser->errors) { // Incase of parsing errors, return NULL
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

typedef enum HRMLtokenkind{
  HrmlTokenInvalid = 0,
  HrmlTokenSym,
  HrmlTokenStr,
  HrmlTokenInt,
  HrmlTokenFloat,
  HrmlTokenDate,
  HrmlTokenTime,
  HrmlTokenChar,
  HrmlTokenBool
} HRMLtokenkind;

static int gLineCount = 1;
static const char *gFileName = NULL;
static bool gParseErrors = false;

static char *gTokenKinds[] =
{
  [HrmlTokenInvalid] = "invalid",
  [HrmlTokenSym] = "sym",
  [HrmlTokenStr] = "str",
  [HrmlTokenInt] = "int",
  [HrmlTokenFloat] = "real",
  [HrmlTokenDate] = "date",
  [HrmlTokenTime] = "time",
  [HrmlTokenChar] = "char",
  [HrmlTokenBool] = "bool"
};

static inline void
ParseError(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  fprintf(stderr, "%d:%s:parse error ", gLineCount, gFileName);
  vfprintf(stderr, fmt, args);
  fputc('\n', stderr);

  va_end(args);

  gParseErrors = true;
}



typedef struct HRMLtoken {
  HRMLtokenkind kind;
  union {
    char *sym;
    char *str;
    uint64_t integer;
    double real;
    char ch;
    bool boolean;
  } val;
} HRMLtoken;

struct growable_string {
  size_t curr_len;
  size_t a_len;
  char *str;
};

static void gw_init(struct growable_string *s)
{
  s->str = malloc(16);
  s->str[0] = '\0';
  s->a_len = 16;
  s->curr_len = 0;
}

static void gw_push(struct growable_string *s, char c)
{
  if (s->curr_len + 1 >= s->a_len) { // Add one accounts for '\0'
    s->str = realloc(s->str, s->a_len * 2);
    s->a_len *= 2;
  }

  s->str[s->curr_len] = c;
  s->str[s->curr_len+1] = '\0';
  s->curr_len ++;
}

static void gw_clear(struct growable_string *s)
{
  s->curr_len = 0;
  s->str[0] = '\0';
}

static void gw_destroy(struct growable_string *s)
{
  s->curr_len = 0;
  free(s->str);
}

static size_t gw_len(struct growable_string *s) {return s->curr_len;}

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
typedef struct {
  FILE *f;
  size_t lineNo;
  size_t colNo;
} HRMLlexer;

int
hrmlLexGet(HRMLlexer *lex)
{
  int ch = fgetc(lex->f);
  if (ch == '\n') {
    lex->lineNo ++;
    lex->colNo = 0;
  } else if (ch == '\r') {
    ch = fgetc(lex->f);
  } else {
    lex->colNo ++;
  }
  return ch;
}

void
hrmlLexUnGet(HRMLlexer *lex, int ch)
{
  ungetc(ch, lex->f);
  if (ch == '\n') {
    lex->lineNo --;
    lex->colNo = 0; // TODO: this is obviously incorrect, but I think it is ok
                    // for now
  } else {
    lex->colNo --;
  }
}


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

  return obj;
}

static inline HRMLobject*
makeNode(const char * restrict name)
{
  HRMLobject *obj = malloc(sizeof(HRMLobject));
  obj->children = NULL;
  obj->previous = NULL;
  obj->next = NULL;

  obj->val.alen = 0;  
  obj->name = strdup(name);
  obj->val.typ = HRMLNode;

  return obj;
}

static inline void
pushNode(HRMLobject *parent, HRMLobject *child)
{
  // TODO: This inserts in the wrong order, we should fix this by adding a tail node
  child->next = parent->children;
  child->previous = NULL;

  if (parent->children != NULL) {
    parent->children->previous = child;
  }
  parent->children = child;
}

HRMLtoken
hrmlLex(FILE *f)
{
  HRMLtoken errTok = {HrmlTokenInvalid, .val.integer = 0};
  int c;
  struct growable_string s;
  gw_init(&s);

  while (isblank(c = fgetc(f)) || c == '\n' || c == '#') {
    // Skip WS
    if (c == '\n') {
      // Update linecounter
      gLineCount ++;
    } else if (c == '#') {
      // Skip comments
      while ((c = fgetc(f)) != '\n') {
        if (feof(f)) return errTok; // End of file
        if (ferror(f)) {fprintf(stderr, "lexerror\n");return errTok;} // End of file
      } // note update line counter
    }

  }
  if (feof(f)) {
    printf("********\n");

    return errTok; // End of file
  }
  if (ferror(f)) {fprintf(stderr, "lexerror\n");return errTok;} // End of file

  // Special hex or bin numbers
  if (c == '0') {
    int c2 = fgetc(f);
    if (c2 == 'x') {
      // Hexnumber
      while (isxdigit(c = fgetc(f)) || c == '_') {
        if (c != '_') gw_push(&s, c);
      }
      ungetc(c, f);
      if (gw_len(&s)) {
        HRMLtoken tok;
        tok.kind = HrmlTokenInt;
        tok.val.integer = strtoull(s.str, NULL, 16);
        gw_destroy(&s);
        return tok;
      }
  printf("********\n");
      return errTok;
    } else if (c2 == 'b') {
      // Binary number
      while (isbindigit(c = fgetc(f)) || c == '_') {
        if (c != '_') gw_push(&s, c);
      }
      ungetc(c, f);
      if (gw_len(&s)) {
        HRMLtoken tok;
        tok.kind = HrmlTokenInt;
        tok.val.integer = strtoull(s.str, NULL, 2);
        gw_destroy(&s);
        return tok;
      }
  printf("********\n");

      return errTok;
    } else {
      ungetc(c2, f);
    }
  }

  if (isdigit(c)) {
    // Integers or floats
    gw_push(&s, c);
    while (isdigit(c = fgetc(f)) || c == '_') {
      if (c != '_') gw_push(&s, c);
    }
    if (c == '.') {
      // Float
      gw_push(&s, c);
      while (isdigit(c = fgetc(f)) || c == '_') {
        if (c != '_') gw_push(&s, c);
      }
      ungetc(c, f);

      HRMLtoken tok;
      tok.kind = HrmlTokenFloat;
      tok.val.real = strtod(s.str, NULL);
      gw_destroy(&s);
      return tok;
    } else {
      // Integer
      HRMLtoken tok;
      ungetc(c, f);

      tok.kind = HrmlTokenInt;
      tok.val.integer = strtoull(s.str, NULL, 10);
      gw_destroy(&s);

      return tok;
    }
  
    printf("********\n");
    return errTok;
  } else if (isalpha(c)) {
    // Symbols
    gw_push(&s, c);
    while (isalnum(c = fgetc(f)) || c == '-') {
      gw_push(&s, c);
    }
    ungetc(c, f);

    if (!strcmp(s.str, "true")) {
      HRMLtoken tok;
      tok.kind = HrmlTokenBool;
      tok.val.boolean = true;
      gw_destroy(&s);
      return tok;
    } else if (!strcmp(s.str, "false")) {
      HRMLtoken tok;
      tok.kind = HrmlTokenBool;
      tok.val.boolean = false;
      gw_destroy(&s);
      return tok;
    }
    HRMLtoken tok;
    tok.kind = HrmlTokenSym;
    tok.val.sym = strdup(s.str);//TODO: fix leak, should move to Mmap based files
    gw_destroy(&s);
    return tok;
  } else if (c == '"') {
    // String
    while ((c = fgetc(f)) != '"') {
      // TODO: check for errors in reading
      if (c == '\\') {
        // Escapes
        int c2 = fgetc(f);
        if (c2 == '"') gw_push(&s, c2);
        else if (c2 == 't') gw_push(&s, '\t');
        else if (c2 == 'n') gw_push(&s, '\n');
        else if (c2 == '\\') gw_push(&s, '\\');
        else {gw_destroy(&s); return errTok; }
      } else {
        // Non escaped characters
        gw_push(&s, c);
      }
    }

    HRMLtoken tok;
    tok.kind = HrmlTokenStr;
    tok.val.str = strdup(s.str);
    return tok;
  }

  if (isop(c)) {
    HRMLtoken tok;
    tok.kind = HrmlTokenChar;
    tok.val.ch = c;
    gw_destroy(&s);
    return tok;
  }
  // Maybe this should be static
  gw_destroy(&s);
  printf("********\n");

  return errTok;
}
#define IS_CHAR(tok, c) ((tok).kind == HrmlTokenChar && (tok).val.ch == (c))
#define IS_SYM(tok) ((tok).kind == HrmlTokenSym)
#define SYM(tok) ((tok).val.sym)

#define IS_STR(tok) ((tok).kind == HrmlTokenStr)
#define STR(tok) ((tok).val.str)
#define IS_INTEGER(tok) ((tok).kind == HrmlTokenInt)
#define IS_BOUNDED_INTEGER(tok, minVal, maxVal) ((tok).kind == HrmlTokenInt && (tok).val.integer >= (minVal) && (tok).val.integer <= (maxVal))
#define INTEGER(tok) ((tok).val.integer)
#define IS_REAL(tok) ((tok).kind == HrmlTokenFloat)
#define REAL(tok) ((tok).val.real)
#define IS_LEAP_YEAR(y) (((y) % 400 == 0) || ((y) % 100 != 0 && (y) % 4 == 0))
#define IS_VALUE(tok) (IS_STR(tok) || IS_INTEGER(tok) || IS_REAL(tok) || IS_SYM(tok))

#define IS_BOOLEAN(tok) ((tok).kind == HrmlTokenBool)
#define BOOLEAN(tok) ((tok).val.boolean)

// Checks date for validity, asserts that month is between 1 and 12 and that the
// day is in the valid range for that month, taking leap years into account
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

HRMLtoken
requireSym(HRMLlexer *lex)
{
  HRMLtoken tok = hrmlLex(lex->f);
  if (IS_SYM(tok)) {
    return tok;
  } else {
    // longjmp(lex->jump_buffer, 1);
  }
}

bool
isSym(HRMLlexer *lex)
{
  return false;
}


void
hrmlPrintTok(HRMLtoken tok)
{
  switch (tok.kind) {
  case HrmlTokenInvalid:
    break;
  case HrmlTokenSym:
    fprintf(stderr, "sym token %s\n", SYM(tok));
    break;
  case HrmlTokenStr:
    fprintf(stderr, "str token %s\n", STR(tok));
    break;
  case HrmlTokenInt:
    fprintf(stderr, "int token %llu\n", INTEGER(tok));
    break;
  case HrmlTokenFloat:
    fprintf(stderr, "real token %f\n", REAL(tok));
    break;
  case HrmlTokenDate:
    fprintf(stderr, "date token\n");
    break;
  case HrmlTokenTime:
    fprintf(stderr, "time token\n");
    break;
  case HrmlTokenChar:
    fprintf(stderr, "char token %c\n", tok.val.ch);
    break;
  default:
    fprintf(stderr, "unknown token\n");
  }
}

HRMLobject*
hrmlParseArray(FILE *f)
{
  size_t alen;
  // At this point, the left bracket '[' should already be consumed
  HRMLtoken value = hrmlLex(f);
  HRMLtoken comma = hrmlLex(f);

  HRMLtoken rightBracket = hrmlLex(f);
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

HRMLattrlist*
hrmlCreateAttrList(void)
{
  HRMLattrlist *alist = malloc(sizeof(HRMLattrlist));
  alist->names = malloc(sizeof(char*)*4);
  alist->values = malloc(sizeof(HRMLvalue*)*4);
  alist->attrCount = 0;
  alist->allocLen = 4;

  return alist;
}

void
hrmlPushAttr(HRMLattrlist *attrList, const char *name, HRMLvalue *value)
{
  // Expand attr array if needed
  if (attrList->allocLen <= attrList->attrCount) {
    attrList->names = realloc(attrList->names,
                              attrList->allocLen*sizeof(char*)*2);
    attrList->values = realloc(attrList->names,
                               attrList->allocLen*sizeof(HRMLvalue*)*2);
  }

  attrList->names[attrList->attrCount] = strdup(name);
  attrList->values[attrList->attrCount] = *value;
  attrList->attrCount ++;
}

HRMLattrlist*
hrmlParseAttrs(FILE *f)
{
  // Precondition, have already consumed '('
  HRMLtoken commaOrParen;
  do {
    HRMLtoken tok = hrmlLex(f);
    if (IS_CHAR(tok, ')')) {
      // This is an empty attribute list
      //fprintf(stderr, "empty attr list\n");
      return NULL;
    }

    if (IS_SYM(tok)) {
      HRMLtoken colon = hrmlLex(f);
      if (! IS_CHAR(colon, ':')) {
        fprintf(stderr, "attr: missing colon after %s\n", SYM(tok));
        return NULL;
      }
      HRMLtoken value = hrmlLex(f);
      if (!IS_VALUE(value)) {
        fprintf(stderr, "attr: no value after %s\n", SYM(tok));
        return NULL;
      } else {
        //fprintf(stderr, "attr: %s\n", SYM(tok));
      }
    }

    commaOrParen = hrmlLex(f);
    if (!(IS_CHAR(commaOrParen, ')') || IS_CHAR(commaOrParen, ','))) {
      fprintf(stderr, "attr: list terminated by unknown sequence\n");
      return NULL;
    }
  } while (IS_CHAR(commaOrParen, ','));

  return NULL;
}

HRMLobject* hrmlParseObj(FILE *f, const char *sym);

HRMLobject*
hrmlParseObjList(FILE *f, const char *sym)
{
  HRMLobject *node = makeNode(sym);

  HRMLtoken tok;
  // '{' must be consumed
  tok = hrmlLex(f);

  do {
    if (IS_CHAR(tok, '}')) {
      // fprintf(stderr, "end of object list\n");
      return node;
    } else if (IS_SYM(tok)) {
      //  fprintf(stderr, "parse subobj and push on list\n");
      HRMLobject *obj = hrmlParseObj(f, SYM(tok));
      pushNode(node, obj);
    }
    tok = hrmlLex(f);
  } while (IS_SYM(tok));


  if (!IS_CHAR(tok, '}')) {
    ParseError("expected '}', got %s\n", gTokenKinds[tok.kind]);
  }
  //fprintf(stderr, "objlist done\n");
  return node;

}

HRMLobject*
hrmlParseObj(FILE *f, const char *sym)
{
  HRMLtoken tok = hrmlLex(f);
  if (IS_CHAR(tok, '(')) { // this is optional
    //printf("parsing attributes\n");

    HRMLattrlist *attrs = hrmlParseAttrs(f);
    tok = hrmlLex(f);
  }

  if (IS_CHAR(tok, ':')) {
    //fprintf(stderr, "parsing primitive %s\n", sym);
    HRMLobject *obj = hrmlParsePrimitiveValue(f, sym);
    return obj;
  } else if (IS_CHAR(tok, '{')) {
    //fprintf(stderr, "parsing complex %s\n", sym);
    HRMLobject *objList = hrmlParseObjList(f, sym);
    //fprintf(stderr, "done parsing complex %s, %p\n", sym, objList);
    return objList;
  }

  printf("invalid object %s\n", sym);

  return NULL;
}


static inline void indentstdout(int indentlev)
{
  for (int i = 0 ; i < indentlev ; i ++) {
    putchar('\t');
  }
}

static void
hrmlPrintObj(HRMLobject *obj, int indent)
{
  assert(obj != NULL);

  if (obj->val.typ == HRMLNode) {
    indentstdout(indent);
    printf("%s () {\n", obj->name);
    HRMLobject *child = obj->children;
    while (child) {
      hrmlPrintObj(child, indent + 1);
      child = child->next;
    }

    indentstdout(indent);
    printf("}\n");

  } else {
    indentstdout(indent);
    printf("%s (): ;\n", obj->name);
  }
}



static HRMLobject*
hrmlParse2(FILE *f)
{
  HRMLtoken tok = hrmlLex(f);

  HRMLobject *obj = NULL;
  switch (tok.kind) {
  case HrmlTokenSym:
    obj = hrmlParseObj(f, SYM(tok));

    fprintf(stderr, "done root object parsing\n");
    break;

  default:
    return NULL;
  }

  return obj;
}

HRMLdocument*
hrmlParse(FILE *f)
{
  gLineCount = 1;
  gParseErrors = false;

  HRMLdocument *doc = malloc(sizeof(HRMLdocument));
  doc->rootNode = hrmlParse2(f);
  if (gParseErrors) {
    hrmlFreeDocument(doc);
    return NULL;
  }

  fprintf(stderr, "parser done\n");

  hrmlPrintObj(doc->rootNode, 0);

  return doc;
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

