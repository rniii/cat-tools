#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <unistd.h>

#define CSI    "\033["
#define SGR(c) CSI c "m"

#define LOW  SGR("36")
#define MID  SGR("37")
#define HIGH SGR("97")

#define SPC  "␣"
#define NIL  SGR("90") "·  "

int octets    = 0;
int no_caret  = 0;
int no_ascii  = 0;
int bytes     = 0;
int words     = 0;
int dwords    = 0;
int qwords    = 0;
int base      = 16;

ssize_t page, count = 0, off = 0;

const char *usage =
  "Usage: xd [options] [file]\n"
  "\n"
  "Options:\n"
  "  -h     display this message\n"
  "  -x     hexadecimal output (default)\n"
  "  -d     decimal output\n"
  "  -o     octal output\n"
  "  -n     ascii-only output\n"
  "  -0     group all bytes into one\n"
  "  -1     group single bytes (default: oct, dec)\n"
  "  -2     group 16-byte units (default: hex)\n"
  "  -4     group 32-byte units\n"
  "  -8     group 64-byte units (default: hex)\n"
  "  -w {8,16,32,..}  display this many bytes per line (default: terminal width)\n"
  "  -s [-]seek  start at this position\n"
  "  -c count  output this many bytes\n"
;

static void fatal(char *msg) { write(2, msg, strlen(msg)), exit(1); }

static short ctoa16(char c) {
  static char *alpha = "0123456789abcdef";

  short i;
  char *b = (char *)&i;

  b[0] = alpha[c>>4];
  b[1] = alpha[c>>0 & 0x0f];

  return i;
}

static int ctoa10(char c) {
  int   i = 0x30303030;
  char *b = (char *)&i;

  b[0] |= c/100;
  b[1] |= c/10  % 10;
  b[2] |= c/1   % 10;

  return i;
}

static int ctoa8(char c) {
  int   i = 0x30303030;
  char *b = (char *)&i;

  b[0] += c>>6;
  b[1] += c>>3 & 07;
  b[2] += c>>0 & 07;

  return i;
}

static void numview(char *data, char *end) {
  uint8_t c;

  for (int i = octets; i--; ) {
    int a = ctoa16(c = *data++);

    if (data > end) {
      write(2, "  ", 2);
    } else if (c == 0) {
      write(2, NIL, sizeof(NIL)-2);
    } else {
      if (c < ' ')      write(2, LOW,  sizeof(LOW)-1);
      else if (c < 127) write(2, MID,  sizeof(MID)-1);
      else              write(2, HIGH, sizeof(HIGH)-1);
      write(2, &a, 2);
    }

    // bytes && write(2, " ", 1);
    // i&1 || words  && write(2, " ", 1);
    // i&3 || dwords && write(2, " ", 1);
    // i&7 || qwords && write(2, " ", 1);
  }
}

static void rawview(char *data, char *end) {
  uint8_t c;

  for (int i = octets; i-- && data < end; ) switch (c = *data++) {
    case 0:
      write(2, NIL, sizeof(NIL)-3);
      break;
    default:
      if (c < ' ')      write(2, LOW,  sizeof(LOW)-1);
      else if (c < 127) write(2, MID,  sizeof(MID)-1);
      else              write(2, HIGH, sizeof(HIGH)-1);

      if (c < ' ' || c == 127) c ^= 0x40;

      if (c > 127 || no_ascii) {
        char braille[] = "\u2800";
        braille[1] |= (c&0010)>>3 | (c&0200)>>6;
        braille[2] |= c&0007 | (c>>1)&0070;
        write(2, braille, 3);
      } 
      else if (c == ' ') write(2, SPC, sizeof(SPC)-1);
      else               write(2, &c, 1);

      break;
  };
}

static void dump(char *data, size_t len) {
  for (char *end = data + len; data < end; data += octets) {
    if (base != 0) numview(data, end);
    rawview(data, end);

    write(2, "\n", 1);
  }
}

void dumpfile(char *path) {
  int   fd;
  char *data;

  if (fd = open(path, O_RDONLY), fd < 0) switch (errno) {
    case EACCES:  fatal("Permission denied");
    case EIO:     fatal("Input/output error");
    case ENOENT:  fatal("No such file");
    default:      fatal("Failed to open file");
  };

  count = count ?: lseek(fd, 0, SEEK_END);

  while (count > 0) {
    ssize_t size = count < page ? count : page;
    data = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, off);

    dump(data, size);

    munmap(data, size);
    off += page, count -= page;
  }
}

void help() {
  write(1, usage, strlen(usage));
}

int main(int argc, char *argv[]) {
  int opt;

  while (opt = getopt(argc, argv, "xdon01248w:s:c:"), opt != -1) switch (opt) {
    case 'x': base   = 16; break;
    case 'd': base   = 10; break;
    case 'o': base   = 8;  break;
    case 'n': base   = 0;  break;
    case '0': bytes  = -1; break;
    case '1': bytes  = 1;  break;
    case '2': words  = 1;  break;
    case '4': dwords = 1;  break;
    case '8': qwords = 1;  break;
    case 'w': octets = atoi(optarg); break;
    case 's': off    = atoi(optarg); break;
    case 'c': count    = atoi(optarg); break;
    case '?': write(1, usage, strlen(usage)), exit(1);
  }

  page = getpagesize();

  if (base == 16 && !bytes && !words && !dwords && !qwords)
    words = qwords = 1;
  if (base != 16 && bytes != -1)
    bytes = 1;
  if (bytes == -1) 
    bytes = 0;

  if (octets == 0) {
    struct winsize winsz;
    ioctl(0, TIOCGWINSZ, &winsz);

    octets = 0x40;
    while (octets > 8) {
      int width = octets * 3;
      if (base != 16) width += octets;
      if (bytes)      width += octets;
      if (words)      width += octets/2;
      if (dwords)     width += octets/4;
      if (qwords)     width += octets/8;

      if (width < winsz.ws_col) break;

      octets /= 2;
    }
  }

  dumpfile(argv[optind] ?: "a.out");

  exit(0);
}
