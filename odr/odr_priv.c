#define ber_enclen_short(b, len) ((*(b) = (len) & 0X7F), 1)
