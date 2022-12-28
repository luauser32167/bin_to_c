# bin_to_c.h - single header library for embedding binary data in C source files

```bin_to_c.h``` can be used to generate C source files that have binary data embedded in them: default fonts, icon sets, sounds, etc. It does this by converting the binary data to a string literal which can then be "lexed" back by a compiler into the binary data.
Generating a string literal, instead of a static array ```unsigned char binary_data[] = { 0x00, 0x01, 0x02 ... };``` (```xxd --include```), is preferable because it is easier for the compiler to lex/parse.

```C
#include <stdio.h>
#define BIN_TO_C_IMPLEMENTATION
#include "bin_to_c.h"

int
main(void) {
  enum { ndata = 256, };
  unsigned char data[ndata];
  for (int a = 0; a < ndata; ++a) {
    data[a] = (unsigned char)a;
  }

  int nstr1 = 0;
  char const* str1 = NULL;
  b2cSingleLine(ndata, data, &nstr1, &str1);

  int nstr2 = 0;
  char const* str2 = NULL;
  b2cMultipleLines(47, ndata, data, &nstr2, &str2);

  printf("%.*s\n", nstr1, str1);
  printf("%.*s\n", nstr2, str2);

  return 0;
}

// output:
"\0\1\2\3\4\5\6\a\b\t\n\v\f\r\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37 !\"#$\45&'()*+,-./\60\61\62\63\64\65\66\67\70\71:;<=>\77@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377"
"\0\1\2\3\4\5\6\a\b\t\n\v\f\r\16\17\20\21\22\23\24"
"\25\26\27\30\31\32\33\34\35\36\37 !\"#$\45&'()*"
"+,-./\60\61\62\63\64\65\66\67\70\71:;<=>\77@ABC"
"DEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopq"
"rstuvwxyz{|}~\177\200\201\202\203\204\205\206\207"
"\210\211\212\213\214\215\216\217\220\221\222\223"
"\224\225\226\227\230\231\232\233\234\235\236\237"
"\240\241\242\243\244\245\246\247\250\251\252\253"
"\254\255\256\257\260\261\262\263\264\265\266\267"
"\270\271\272\273\274\275\276\277\300\301\302\303"
"\304\305\306\307\310\311\312\313\314\315\316\317"
"\320\321\322\323\324\325\326\327\330\331\332\333"
"\334\335\336\337\340\341\342\343\344\345\346\347"
"\350\351\352\353\354\355\356\357\360\361\362\363"
"\364\365\366\367\370\371\372\373\374\375\376\377"
""
```
