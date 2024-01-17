#include "s21_decimal.h"

int s21_floor(s21_decimal value, s21_decimal* result) {
  int ans = OK;
  if (result) {
    setDefault(result);
    s21_truncate(value, result);
    if (getBit(value, 127) && !s21_is_equal(*result, value)) {
      s21_sub(*result, ((s21_decimal){{1, 0, 0, 0}}), result);
    }
  } else {
    ans = ERROR;
  }
  return ans;
}

int s21_round(s21_decimal value, s21_decimal* result) {
  setDefault(result);
  int res = OK;
  int sign = getBit(value, 127);
  setSign(&value, 0);
  s21_decimal left_part = {{0}};
  s21_decimal right_part = {{0}};
  s21_truncate(value, &left_part);
  int even = !getBit(left_part, 0);
  s21_sub(value, left_part, &right_part);
  s21_decimal five = {{5, 0, 0, 0}};
  s21_decimal one = {{1, 0, 0, 0}};
  setScale(&five, 1);
  if (s21_is_greater_or_equal(right_part, five) &&
      !((s21_is_equal(right_part, five)) && even)) {
    res = s21_add(left_part, one, result);
  } else {
    *result = left_part;
  }
  if (sign) setSign(result, sign);
  return res;
}

int s21_truncate(s21_decimal value, s21_decimal* result) {
  int ans = OK;
  if (result) {
    setDefault(result);
    int sign = getBit(value, 127);
    int scale = getScale(value);
    if (scale) {
      for (int i = scale; i > 0; i--) {
        divBy10(&value);
      }
      *result = value;
      result->bits[3] = 0;
      if (sign) setSign(result, 1);
    } else {
      *result = value;
    }
    if (sign) setSign(result, 1);
  } else {
    ans = ERROR;
  }
  return ans;
}

int s21_negate(s21_decimal dec, s21_decimal* result) {
  int ans = OK;
  if (setDefault(result)) {
    int sign = getBit(dec, 127);
    *result = dec;
    if (sign == 1) {
      setSign(result, 0);
    } else {
      setSign(result, 1);
    }
  } else {
    ans = ERROR;
  }
  return ans;
}