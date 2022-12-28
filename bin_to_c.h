#ifndef BIN_TO_C_H
#define BIN_TO_C_H

void b2cSingleLine(int ndata, unsigned char const* data, int* outLen, char const** outStr);
void b2cMultipleLines(int maxLineLen, int ndata, unsigned char const* data, int* outLen, char const** outStr);

#ifdef BIN_TO_C_IMPLEMENTATION

#include <stdlib.h> // malloc, realloc
#include <string.h> // memcpy

unsigned char const b2cCharBits[256] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static inline int b2cIsCharPrintable(int c) { return 0 != (1 & b2cCharBits[c]); }
static inline int b2cIsCharDigit(int c) { return 0 != (2 & b2cCharBits[c]); }

typedef struct B2cBuf {
  int cap;
  int len;
  char* buf;
} B2cBuf;

static void
b2cCheckN(B2cBuf* b, int n) {
  if (b->cap < b->len + n) {
    int newCap = b->cap;
    if (0 == newCap) { newCap = 8192; }
    while (newCap < b->len + n) { newCap *= 2; }
    b->cap = newCap;
    b->buf = realloc(b->buf, (size_t)newCap);
  }
}

static void
b2cBufFree(B2cBuf* b) {
  free(b->buf);
  b->buf = NULL;
  b->cap = 0;
  b->len = 0;
}

static void
b2cBufPushc(B2cBuf* b, int c) {
  b2cCheckN(b, 1);
  b->buf[b->len++] = (char)c;
}

void
b2cSingleLine(int ndata, unsigned char const* data, int* outLen, char const** outStr) {
  B2cBuf bb = {0};

  b2cBufPushc(&bb, '"');

  for (int a = 0; a < ndata; ++a) {
    int b = (int)data[a];

    if ('\0' == b) {
      b2cBufPushc(&bb, '\\');
      b2cBufPushc(&bb, '0');
    }
    else if (b2cIsCharPrintable(b)) {
      if (b2cIsCharDigit(b)) {
        // we can't just emit digits because they can make the octal escape codes longer,
        // which would result in wrong bytes when the string is lexed by the compiler
        goto l_to_oct;
      }
      else if ('\\' == b) {
        b2cBufPushc(&bb, '\\');
        b2cBufPushc(&bb, '\\');
      }
      else if ('%' == b) {
        // '%' == 0d37 == 0o45
        b2cBufPushc(&bb, '\\');
        b2cBufPushc(&bb, '4');
        b2cBufPushc(&bb, '5');
      }
      else if ('?' == b) {
        // '?' is used to avoid trigraphs (whaaaaat?)
        // '?' == 0d63 == 0o77
        b2cBufPushc(&bb, '\\');
        b2cBufPushc(&bb, '7');
        b2cBufPushc(&bb, '7');
      }
      else if ('"' == b) {
        b2cBufPushc(&bb, '\\');
        b2cBufPushc(&bb, '"');
      }
      else {
        b2cBufPushc(&bb, b);
      }
    }
    else if ('\a' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'a'); }
    else if ('\b' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'b'); }
    else if ('\t' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 't'); }
    else if ('\n' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'n'); }
    else if ('\v' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'v'); }
    else if ('\f' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'f'); }
    else if ('\r' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'r'); }
    // else if ('\e' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'e'); } // non-standard gcc extension
    else {
      l_to_oct:;
      b2cBufPushc(&bb, '\\');
      char buf[3]; // 0d255 == 0o377
      int nbuf = 0;
      for (;;) {
        int d = b & 7;
        b >>= 3;
        buf[nbuf++] = (char)('0' + d);
        if (0 == b) { break; }
      }
      switch (nbuf) {
      break; case 1: b2cBufPushc(&bb, buf[0]);
      break; case 2: b2cBufPushc(&bb, buf[1]); b2cBufPushc(&bb, buf[0]);
      break; case 3: b2cBufPushc(&bb, buf[2]); b2cBufPushc(&bb, buf[1]); b2cBufPushc(&bb, buf[0]);
      break;default: /*unreachable*/;
      }
    }
  }

  b2cBufPushc(&bb, '"');

  *outLen = bb.len;

  b2cBufPushc(&bb, '\0');
  char* result = malloc((size_t)bb.len * sizeof(char));
  memcpy(result, bb.buf, (size_t)bb.len);
  *outStr = result;

  b2cBufFree(&bb);
}

void
b2cMultipleLines(int maxLineLen, int ndata, unsigned char const* data, int* outLen, char const** outStr) {
  B2cBuf bb = {0};

  int lineLen = 0;
  b2cBufPushc(&bb, '"');

  for (int a = 0; a < ndata; ++a) {
    int b = (int)data[a];

    if ('\0' == b) {
      b2cBufPushc(&bb, '\\');
      b2cBufPushc(&bb, '0');
      lineLen += 2;
    }
    else if (b2cIsCharPrintable(b)) {
      if (b2cIsCharDigit(b)) {
        goto l_to_oct;
      }
      else if ('\\' == b) {
        b2cBufPushc(&bb, '\\');
        b2cBufPushc(&bb, '\\');
        lineLen += 2;
      }
      else if ('%' == b) {
        b2cBufPushc(&bb, '\\');
        b2cBufPushc(&bb, '4');
        b2cBufPushc(&bb, '5');
        lineLen += 3;
      }
      else if ('?' == b) {
        b2cBufPushc(&bb, '\\');
        b2cBufPushc(&bb, '7');
        b2cBufPushc(&bb, '7');
        lineLen += 3;
      }
      else if ('"' == b) {
        b2cBufPushc(&bb, '\\');
        b2cBufPushc(&bb, '"');
        lineLen += 2;
      }
      else {
        b2cBufPushc(&bb, b);
        lineLen += 1;
      }
    }
    else if ('\a' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'a'); lineLen += 2; }
    else if ('\b' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'b'); lineLen += 2; }
    else if ('\t' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 't');  lineLen += 2;}
    else if ('\n' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'n'); lineLen += 2; }
    else if ('\v' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'v'); lineLen += 2; }
    else if ('\f' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'f'); lineLen += 2; }
    else if ('\r' == b) { b2cBufPushc(&bb, '\\'); b2cBufPushc(&bb, 'r'); lineLen += 2; }
    else {
      l_to_oct:;
      b2cBufPushc(&bb, '\\');
      char buf[3];
      int nbuf = 0;
      for (;;) {
        int d = b & 7;
        b >>= 3;
        buf[nbuf++] = (char)('0' + d);
        if (0 == b) { break; }
      }
      switch (nbuf) {
      break; case 1: b2cBufPushc(&bb, buf[0]);
      break; case 2: b2cBufPushc(&bb, buf[1]); b2cBufPushc(&bb, buf[0]);
      break; case 3: b2cBufPushc(&bb, buf[2]); b2cBufPushc(&bb, buf[1]); b2cBufPushc(&bb, buf[0]);
      break;default: /*unreachable*/;
      }
      lineLen += 1 + nbuf;
    }

    if (maxLineLen <= lineLen) {
      lineLen = 0;
      b2cBufPushc(&bb, '"');
      b2cBufPushc(&bb, '\n');
      b2cBufPushc(&bb, '"');
    }
  }

  b2cBufPushc(&bb, '"');
  b2cBufPushc(&bb, '\n');

  *outLen = bb.len;

  b2cBufPushc(&bb, '\0');
  char* result = malloc((size_t)bb.len * sizeof(char));
  memcpy(result, bb.buf, (size_t)bb.len);
  *outStr = result;

  b2cBufFree(&bb);
}

#endif // BIN_TO_C_IMPLEMENTATION

#endif // BIN_TO_C_H
