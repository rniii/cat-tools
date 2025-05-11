% xd(1) cat-tools 0.0
%
% May 2025

# NAME

xd - hexdump

# SYNOPSIS


**xd** [*options*] [*file*]  
**xd** [*options*] -[01248] -[*base*] [*file*]

# DESCRIPTION

**xd** dumps a file in hexadecimal or other formats, and prints as many characters as possible
visibly.

Low bytes (control characters) are printed in caret notation (XOR 0x40), middle
bytes are printed as ASCII, with the exception of space being shown as ‘␣’
(U+2423 OPEN BOX), while high bytes are binary-encoded in braille characters
(top-down, left-to-right).  Each group has a different color to tell them apart.

# OPTIONS

If *file* is not given, **a.out** will be read instead.

**-w** *width*

:   Specify amount of bytes per line. By default will attempt to fit as many bytes as possible based
on terminal width.

**-s** [-]*seek*

:   Start reading file at specified offset.

**-c** [-]*count*

:   Read *count* bytes from file. Negative values are relative to the file's size.

**-[*base*]**

:   Output byte values numerically in the specified base

    **-n**  none, disable numeric output
    **-x**  hexadecimal  
    **-d**  decimal  
    **-o**  octal  

**-[01248]**

:   Group bytes in specified unit. Default is -2 and -8, if this option isn't used. -0 disables
grouping.


# EXAMPLES

Default output style (-x -2 -8)

    $ xd xd
    7f45 4c46 0101 01·   · ·  · ·  · ·  · ·   ?ELFAAA·········
    02·  3e·  01·  · ·   d602 20·  34·  · ·   B·>·A···⢮B␣·4···
    740c · ·  · ·  · ·   34·  20·  02·  28·   tL······4·␣·B·(·
    06·  05·  01·  · ·   80·  · ·  80·  20·   F·E·A···⢀···⢀·␣·

Group four byte words

    $ xd -4 xd
    7f454c46 010101·  · · · ·  · · · ·  ?ELFAAA·········
    02· 3e·  01· · ·  d60220·  34· · ·  B·>·A···⢮B␣·4···
    740c· ·  · · · ·  34· 20·  02· 28·  tL······4·␣·B·(·
    06· 05·  01· · ·  80· · ·  80· 20·  F·E·A···⢀···⢀·␣·

Just pretty-print bytes

    $ xd -n xd
    ?ELFAAA·········B·>·A···⢮B␣·4···tL······4·␣·B·(·F·E·A···⢀···⢀·␣·
    ⢀·␣·⣦K··⢼K··G···P···Q⢵td····················F···················
    !C␣·⢎C␣·⣐C␣·⢹E␣·⣝C␣·⢹E␣·⢹E␣·⢹E␣·⢭C␣·_C␣·⣏C␣·⢹E␣·⢹E␣·⢹E␣·⢙C␣·⢹E␣·
    ⢹E␣·⢹E␣·⣡C␣·⢱C␣·[[90m[[36m[[37m[[97m[[00m·······Usage:␣xd␣[optio

Every byte

    $ ruby -e '$><<[*0..255].pack("C"*256)' >test
    $ xd test
    · 01 0203 0405 0607  0809 0a0b 0c0d 0e0f  ·ABCDEFGHIJKLMNO
    1011 1213 1415 1617  1819 1a1b 1c1d 1e1f  PQRSTUVWXYZ[\]^_
    2021 2223 2425 2627  2829 2a2b 2c2d 2e2f  ␣!"#$%&'()*+,-./
    3031 3233 3435 3637  3839 3a3b 3c3d 3e3f  0123456789:;<=>?
    4041 4243 4445 4647  4849 4a4b 4c4d 4e4f  @ABCDEFGHIJKLMNO
    5051 5253 5455 5657  5859 5a5b 5c5d 5e5f  PQRSTUVWXYZ[\]^_
    6061 6263 6465 6667  6869 6a6b 6c6d 6e6f  `abcdefghijklmno
    7071 7273 7475 7677  7879 7a7b 7c7d 7e7f  pqrstuvwxyz{|}~?
    8081 8283 8485 8687  8889 8a8b 8c8d 8e8f  ⢀⢁⢂⢃⢄⢅⢆⢇⣀⣁⣂⣃⣄⣅⣆⣇
    9091 9293 9495 9697  9899 9a9b 9c9d 9e9f  ⢈⢉⢊⢋⢌⢍⢎⢏⣈⣉⣊⣋⣌⣍⣎⣏
    a0a1 a2a3 a4a5 a6a7  a8a9 aaab acad aeaf  ⢐⢑⢒⢓⢔⢕⢖⢗⣐⣑⣒⣓⣔⣕⣖⣗
    b0b1 b2b3 b4b5 b6b7  b8b9 babb bcbd bebf  ⢘⢙⢚⢛⢜⢝⢞⢟⣘⣙⣚⣛⣜⣝⣞⣟
    c0c1 c2c3 c4c5 c6c7  c8c9 cacb cccd cecf  ⢠⢡⢢⢣⢤⢥⢦⢧⣠⣡⣢⣣⣤⣥⣦⣧
    d0d1 d2d3 d4d5 d6d7  d8d9 dadb dcdd dedf  ⢨⢩⢪⢫⢬⢭⢮⢯⣨⣩⣪⣫⣬⣭⣮⣯
    e0e1 e2e3 e4e5 e6e7  e8e9 eaeb eced eeef  ⢰⢱⢲⢳⢴⢵⢶⢷⣰⣱⣲⣳⣴⣵⣶⣷
    f0f1 f2f3 f4f5 f6f7  f8f9 fafb fcfd feff  ⢸⢹⢺⢻⢼⢽⢾⢿⣸⣹⣺⣻⣼⣽⣾⣿

# SEE ALSO

**hexdump**(1), **xxd**(1), https://justine.lol/braille/

# AUTHORS

rini \<rini@rinici.de\>, some bit-magic courtesy of lily \<https://87flowers.com/\>

**xd** is part of the **cat-tools** package \<https://github.com/rniii/cat-tools\>
