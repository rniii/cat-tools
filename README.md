# cat tools

## Development

<!-- maid-tasks -->

Tasks use [maid](https://github.com/rniii/maid)

### build

Compile with [nolibc](https://github.com/torvalds/linux/blob/master/tools/include/nolibc/nolibc.h)

```sh
CFLAGS="-mx32 -fno-pic -fno-pie -Os -fno-asynchronous-unwind-tables -fno-ident -fno-stack-protector"
LDFLAGS="-N -s"
alias cc=clang
alias ld=ld.lld

CFLAGS="$CFLAGS -std=c23 -Wunused"

cc $CFLAGS -nostdlib -Inolibc xd.c -c -o xd.o
ld $LDFLAGS xd.o -o xd
strip xd -R .comment

maid manual
```

### manual

```sh
pandoc -st man man/xd.md -o xd.1
```

### package

```sh
install -Dm755 xd   "$dstdir/bin/xd"
install -Dm644 xd.1 "$dstdir/share/man/man1/xd.1"
```
