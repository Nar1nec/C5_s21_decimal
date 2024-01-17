#include "s21_decimal.h"

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  int ans = OK;
  if (setDefault(dst) && src <= INT_MAX) {
    if (src < 0) setSign(dst, 1);
    dst->bits[0] = abs(src);
  } else
    ans = ERROR;
  return ans;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  int ans = OK;
  ans = s21_truncate(src, &src);
  if (dst && !ans && !src.bits[2] && !src.bits[1] &&
      (src.bits[0] <= 2147483647 ||
       (src.bits[0] == 2147483648 && getBit(src, 127)))) {
    *dst = src.bits[0];
    if (getBit(src, 127)) *dst *= -1;
  } else {
    ans = ERROR;
  }
  return ans;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  int ans = OK;
  if (setDefault(dst) && src && (fabsf(src) <= MAX_DECIMAL) &&
      (fabs(src) >= 1e-28)) {
    int scale = 0;
    int sign = src < 0.0;
    if (sign) src *= -1;

    int exp = getExp(src);
    if (exp > 95) {
      ans = ERROR;
    } else if (exp < 95) {
      while (!(int)src) {
        src *= 10;
        scale++;
      }
    }
    if (scale <= 28 && !ans) {
      double dbl = (double)src;
      while ((dbl - (long)dbl != 0) && !(int)(dbl / 1000000) && scale < 28) {
        dbl *= 10.0;
        scale++;
      }
      dbl = round(dbl);
      while (fmod(dbl, 10) == 0 && scale > 0) {
        dbl = dbl / 10.0;
        scale--;
      }
      int i = 0;
      while ((dbl / 10000000) > 1) {
        dbl /= 10.0;
        dbl = round(dbl);
        i++;
      }
      dbl *= pow(10, i);
      dbl = round(dbl);
      exp = getExp(dbl);
      setBit(dst, exp, 1);
      mantissaToDecimal(dst, dbl, exp);
      setScale(dst, scale);
      if (sign) setSign(dst, 1);
    } else
      ans = ERROR;
  } else
    ans = ERROR;
  return ans;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  int ans = OK;
  if (dst) {
    double tmp = 0;
    int scale = getScale(src);
    int sign = getBit(src, 127);
    for (int i = 0; i < 96; i++) {
      if (getBit(src, i)) tmp += pow(2, i);
    }
    if (scale > 0) {
      for (int i = scale; i > 0; i--) tmp /= 10.0;
    }
    if (sign) tmp *= -1;
    *dst = (float)tmp;
  } else {
    ans = ERROR;
  }
  return ans;
}

void mantissaToDecimal(s21_decimal *dst, double num, int exp) {
  char binFloat[64] = "";
  getBinaryFloat(num, binFloat);
  for (int i = exp - 1, j = 12; i >= 0 && j < 64; i--, j++) {
    if (binFloat[j]) setBit(dst, i, 1);
  }
}

void getBinaryFloat(double num, char *binFloat) {
  unsigned long long float_bits = *((unsigned long long *)&num);
  for (unsigned long long mask = 0x8000000000000000, i = 0; mask;
       mask >>= 1, i++) {
    binFloat[i] = !!(float_bits & mask);
  }
}

int getExp(float float_num) {
  unsigned int mask = 0xFF << 23;
  unsigned int binary_float = *((unsigned int *)&float_num);
  return ((binary_float & mask) >> 23) - 127;
}