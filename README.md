# cdata

Header Only Library for C data-structures and some examples.

## Usage

```console
$ make
gcc -pedantic -W -Wall -Wextra -Wconversion -Wswitch-enum -Werror -std=c99 -O0 -g -I.  examples/count-words.c -o examples/count-words

$ ./examples/count-words examples/The\ Divine\ Comedy.txt examples/Shakespeare.txt 
File: examples/The Divine Comedy.txt
  lines: 6390
  chars: 220001
  words: 38039
  algorithm: dynamic array
    execution time: 1.78038s
    unique words: 7783
    top 10 words:
      01. the               2213
      02. and               1483
      03. of                 854
      04. to                 806
      05. i                  779
      06. that               651
      07. he                 450
      08. in                 441
      09. a                  436
      10. with               397
  algorithm: sorted dynamic array
    execution time: 0.039635s
    unique words: 7783
    top 10 words:
      01. the               2213
      02. and               1483
      03. of                 854
      04. to                 806
      05. i                  779
      06. that               651
      07. he                 450
      08. in                 441
      09. a                  436
      10. with               397
  algorithm: hash table
    execution time: 0.020831s
    unique words: 7783
    top 10 words:
      01. the               2213
      02. and               1483
      03. of                 854
      04. to                 806
      05. i                  779
      06. that               651
      07. he                 450
      08. in                 441
      09. a                  436
      10. with               397
File: examples/Shakespeare.txt
  lines: 124455
  chars: 5458198
  words: 900987
  algorithm: dynamic array
    execution time: 188.265s
    unique words: 58580
    top 10 words:
      01. the              27627
      02. and              26063
      03. i                19593
      04. to               18996
      05. of               18011
      06. a                14589
      07. my               12476
      08. in               10682
      09. you              10642
      10. that             10498
  algorithm: sorted dynamic array
    execution time: 1.30934s
    unique words: 58580
    top 10 words:
      01. the              27627
      02. and              26063
      03. i                19593
      04. to               18996
      05. of               18011
      06. a                14589
      07. my               12476
      08. in               10682
      09. you              10642
      10. that             10498
  algorithm: hash table
    execution time: 0.462562s
    unique words: 58580
    top 10 words:
      01. the              27627
      02. and              26063
      03. i                19593
      04. to               18996
      05. of               18011
      06. a                14589
      07. my               12476
      08. in               10682
      09. you              10642
      10. that             10498
```
