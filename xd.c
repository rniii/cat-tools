#include "nolibc/nolibc.h"

#define SPC  "␣"
#define NIL  "·  "

static char   *file;
static char    base = 16;
static char    bytes, words, dwords, qwords, docolor;
static int     octets;
static ssize_t page, count, skip;

#define CSI    "\033["
#define SGR(c) CSI c "m"

const char c_null[5]  = SGR("90");
const char c_lo[5]    = SGR("36");
const char c_md[5]    = SGR("37");
const char c_hi[5]    = SGR("97");
const char c_reset[5] = SGR("00");

const char usage[] = {
  #embed "xd.txt"
};

static void color(const char c[5]) { docolor && sys_write(1, c, 5); }

static char *alpha = "0123456789abcdef";

static void numview(char *data, char *end) {
  uint8_t c;
  int     w = base == 16 ? 2 : 3;

  for (int i = octets; i--; ) {
    char b[4] = "000";
    c = *data++;

    switch (base) {
      case 16: b[0]  = alpha[c>>4], b[1]  = alpha[c&15];             break;
      case 10: b[0] += c/100;       b[1] += c/10 % 10; b[2] += c%10; break;
      case 8:  b[0] += c>>6;        b[1] += c>>3 & 7;  b[2] += c&7;  break;
    }

    if (data > end) {
      sys_write(1, "   ", w);
    } else if (c == 0) {
      color(c_null);
      sys_write(1, NIL, sizeof(NIL)-4+w);
    } else {
      if (c < ' ')      color(c_lo);
      else if (c < 127) color(c_md);
      else              color(c_hi);
      sys_write(1, b, w);
    }

    bytes && sys_write(1, " ", 1);
    i&1 || words  && sys_write(1, " ", 1);
    i&3 || dwords && sys_write(1, " ", 1);
    i&7 || qwords && sys_write(1, " ", 1);
  }
}

static void rawview(char *data, char *end) {
  uint8_t c;

  for (int i = octets; i-- && data < end; ) switch (c = *data++) {
    case 0:
      color(c_null);
      sys_write(1, NIL, sizeof(NIL)-3);
      break;
    default:
      if (c < ' ')      color(c_lo);
      else if (c < 127) color(c_md);
      else              color(c_hi);

      if (c < ' ' || c == 127) c ^= 0x40;

      if (c > 127) {
        char braille[] = "\u2800";
        braille[1] |= (c&0010)>>3 | (c&0200)>>6;
        braille[2] |= c&0007 | (c>>1)&0070;
        sys_write(1, braille, 3);
      }
      else if (c == ' ') sys_write(1, SPC, sizeof(SPC)-1);
      else               sys_write(1, &c, 1);

      break;
  };
}

static void xd(void) {
  int   fd;
  char *data, *end;

  if (fd = sys_open(file, O_RDONLY, 0), fd < 0) {
    char *msg;
    switch (errno) {
      case EACCES:  msg = "Permission denied";
      case EIO:     msg = "Input/output error";
      case ENOENT:  msg = "No such file";
      default:      msg = "Failed to open file";
    };
    sys_write(2, msg, strlen(msg)), exit(1);
  }

  if (count < 1) count += sys_lseek(fd, 0, SEEK_END);
  data  = sys_mmap(0, count, PROT_READ, MAP_PRIVATE, fd, 0);
  data += skip;

  for (end = data + count; data < end; data += octets) {
    if (base != 0) numview(data, end);
    rawview(data, end);

    sys_write(1, "\n", 1);
  }

  color(c_reset);
}

int main(int argc, char *argv[]) {
  while (*++argv) {
    #define optarg argv[0][2] ? &argv[0][2] : *++argv
    if (**argv == '-') switch (argv[0][1]) {
      case 'x': base   = 16; break;
      case 'd': base   = 10; break;
      case 'o': base   =  8; break;
      case 'n': base   =  0; break;
      case '0': bytes  = -1; break;
      case '1': bytes  =  1; break;
      case '2': words  =  1; break;
      case '4': dwords =  1; break;
      case '8': qwords =  1; break;
      case 'w': octets = strtol(optarg, 0, 0); break;
      case 's': skip   = strtol(optarg, 0, 0); break;
      case 'c': count  = strtol(optarg, 0, 0); break;
      default:  sys_write(1, usage, sizeof(usage)), exit(1);
    }
    else file = file ?: *argv;
  }

  struct winsize winsz = { .ws_col = 80 };
  docolor = sys_ioctl(1, TIOCGWINSZ, (ssize_t)&winsz) == 0;

  file = file ?: "a.out";
  page = getauxval(AT_PAGESZ);

  if (base == 16 && !bytes && !words && !dwords && !qwords)
    words = qwords = 1;
  if (base != 16 && bytes != -1)
    bytes = 1;
  if (bytes == -1)
    bytes = 0;

  if (octets == 0) {
    int width;

    for (octets = 0x80; octets > 8; octets >>= 1) {
      width = octets;

      if (base == 16)     width += octets*2;
      if (base == 10)     width += octets*3;
      if (base == 8)      width += octets*3;
      if (base && bytes)  width += octets;
      if (base && words)  width += octets/2;
      if (base && dwords) width += octets/4;
      if (base && qwords) width += octets/8;

      if (width < winsz.ws_col) break;
    }
  }

  xd();

  exit(0);
}
