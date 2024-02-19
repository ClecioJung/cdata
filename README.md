# cdata

Header Only Libraries for C data-structures and some examples.

## Usage

```console
$ make
mkdir Release
gcc -pedantic -Wall -Wextra -Werror -std=c99 -O2 -flto main.c -o Release/count

$ ./Release/count The\ Divine\ Comedy.txt Shakespeare.txt 
File: The Divine Comedy.txt
  algorithm: dynamic array
  execution time: 0.895444s
  lines: 6390
  chars: 220001
  words: 38039
  unique words: 8452
  top 10 words:
    01. the               1943
    02. and                896
    03. I                  779
    04. of                 751
    05. to                 723
    06. And                587
    07. that               483
    08. a                  389
    09. in                 376
    10. he                 376
File: The Divine Comedy.txt
  algorithm: sorted dynamic array
  execution time: 0.020293s
  lines: 6390
  chars: 220001
  words: 38039
  unique words: 8452
  top 10 words:
    01. the               1943
    02. and                896
    03. I                  779
    04. of                 751
    05. to                 723
    06. And                587
    07. that               483
    08. a                  389
    09. he                 376
    10. in                 376
File: The Divine Comedy.txt
  algorithm: hash table
  execution time: 0.009593s
  lines: 6390
  chars: 220001
  words: 38039
  unique words: 8452
  top 10 words:
    01. the               1943
    02. and                896
    03. I                  779
    04. of                 751
    05. to                 723
    06. And                587
    07. that               483
    08. a                  389
    09. he                 376
    10. in                 376
File: Shakespeare.txt
  algorithm: dynamic array
  execution time: 102.059s
  lines: 124455
  chars: 5458198
  words: 900987
  unique words: 66597
  top 10 words:
    01. the              23255
    02. I                19593
    03. and              18312
    04. to               15638
    05. of               15545
    06. a                12648
    07. my               10828
    08. in                9581
    09. you               9088
    10. is                7853
File: Shakespeare.txt
  algorithm: sorted dynamic array
  execution time: 0.924233s
  lines: 124455
  chars: 5458198
  words: 900987
  unique words: 66597
  top 10 words:
    01. the              23255
    02. I                19593
    03. and              18312
    04. to               15638
    05. of               15545
    06. a                12648
    07. my               10828
    08. in                9581
    09. you               9088
    10. is                7853
File: Shakespeare.txt
  algorithm: hash table
  execution time: 0.184709s
  lines: 124455
  chars: 5458198
  words: 900987
  unique words: 66597
  top 10 words:
    01. the              23255
    02. I                19593
    03. and              18312
    04. to               15638
    05. of               15545
    06. a                12648
    07. my               10828
    08. in                9581
    09. you               9088
    10. is                7853
```
