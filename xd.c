#include "nolibc/nolibc.h"

static char    base = 16;
static char    bytes, words, dwords, qwords, docolor;
static int     octets;
static ssize_t count, skip;

#define CSI    "\033["
#define SGR(c) CSI c "m"

static const char c_null[5]  = SGR("90");
static const char c_lo[5]    = SGR("36");
static const char c_md[5]    = SGR("37");
static const char c_hi[5]    = SGR("32");
static const char c_reset[3] = SGR();

static const char ch_space[3] = "⍽";
static const char ch_null[2]  = "·";
static const char sp[6]       = "      ";

static const char usage[] = {
  #embed "xd.txt"
};

static void color(uint8_t c) {
  if (!docolor) return;

  const char *h;

  if (c == 0)       h = c_null;
  else if (c < ' ') h = c_lo;
  else if (c < 127) h = c_md;
  else              h = c_hi;

  sys_write(1, h, 5);
}

static char *alpha = "0123456789abcdef";

static void numview(char *data, char *end) {
  uint8_t c;
  int octal = base != 16;

  for (int i = octets; i--; ) {
    int pad = 0;

    if (data > end) {
      pad += 2 + octal;
    } else {
      c = *data++;
      color(c);

      char b[3] = "000";
      if (octal) {
        b[0] += c>>6;
        b[1] += c>>3 & 7;
        b[2] += c&7;
      } else {
        b[0] = alpha[c>>4];
        b[1] = alpha[c&15];
      }

      if (c == 0) {
        sys_write(1, octal ? "╶─╴" : "╶╴", octal ? 9 : 6);
      } else {
        sys_write(1, b, 2 + octal);
      }
    }

    bytes && pad++;
    i&1 || words  && pad++;
    i&3 || dwords && pad++;
    i&7 || qwords && pad++;
    sys_write(1, sp, pad);
  }
}

static void rawview(char *data, char *end) {
  uint8_t c;

  for (int i = octets; i-- && data < end; ) {
    c = *data++;
    color(c);
    if (c) {
      if (c < ' ' || c == 127) c ^= 0x40;

      if (c > 127) {
        char braille[] = "\u2800";
        braille[1] |= (c&0010)>>3 | (c&0200)>>6;
        braille[2] |= c&0007 | (c>>1)&0070;
        sys_write(1, braille, 3);
      }
      else if (c == ' ') sys_write(1, ch_space, 3);
      else               sys_write(1, &c, 1);
    }
    else sys_write(1, ch_null, 2);
  }
}

static void xd(char *file) {
  int   fd;
  char *data, *end;

  if (fd = sys_open(file ?: "a.out", O_RDONLY, 0), fd < 0) {
    static const char msg[] = "invalid file: ";
    sys_write(2, msg, sizeof(msg));
    sys_write(2, file, strlen(file));
    exit(2);
  }

  if (count < 1) count += sys_lseek(fd, 0, SEEK_END);
  data  = sys_mmap(0, count, PROT_READ, MAP_PRIVATE, fd, 0);
  data += skip;

  for (end = data + count; data < end; data += octets) {
    if (base) numview(data, end);
    rawview(data, end);

    sys_write(1, "\n", 1);
  }

  docolor && sys_write(1, c_reset, 3);
}

int main(int argc, char *argv[]) {
  char *file = 0;

  while (*++argv) {
    #define optarg argv[0][2] ? &argv[0][2] : *++argv
    if (**argv == '-') switch (argv[0][1]) {
      case 'x': base   = 16; break;
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

  if (base == 16 && !bytes && !words && !dwords && !qwords)
    words = qwords = 1;
  if (base != 16 && bytes != -1)
    bytes = 1;
  if (bytes == -1)
    bytes = 0;

  if (octets < 1) {
    int width;

    for (octets = 0x80; octets > 8; octets /= 2) {
      width = octets;

      if (base == 16)     width += octets*2;
      if (base == 8)      width += octets*3;
      if (base && bytes)  width += octets;
      if (base && words)  width += octets/2;
      if (base && dwords) width += octets/4;
      if (base && qwords) width += octets/8;

      if (width < winsz.ws_col) break;
    }
  }

  xd(file);

  exit(0);
}
