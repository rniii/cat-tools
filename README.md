# cat tools

## Development

<!-- maid-tasks -->

### build

Compile with [nolibc](https://github.com/torvalds/linux/blob/master/tools/include/nolibc/nolibc.h)

```sh
# cc -m32 -fno-asynchronous-unwind-tables -fno-ident -nostdlib -Inolibc \
#   -Os -s xd.c -o xd
nasm -f bin xd.s -l xd.l -o a.out && chmod a+x a.out
```
