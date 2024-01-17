#include "s21_decimal.h"

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int ans = OK;
  setDefault(result);
  s21_decimal_long cp_value_1, cp_value_2;
  s21_decimal_long cp_result = {0};
  moveToBig(&cp_value_1, value_1);
  moveToBig(&cp_value_2, value_2);
  int sign_1 = getBitLong(cp_value_1, 255);
  int sign_2 = getBitLong(cp_value_2, 255);
  int res_sign = sign_1;
  int scale_1 = getScaleLong(cp_value_1);
  int scale_2 = getScaleLong(cp_value_2);
  int res_scale = (scale_1 < scale_2) ? scale_2 : scale_1;
  normalizeLong(&cp_value_1, &cp_value_2);
  int compare = compareMantissa(cp_value_1, cp_value_2);
  if (sign_1 == sign_2 || compare != 0) {
    if (sign_1 == sign_2) {
      adDivision(cp_value_1, cp_value_2, &cp_result);
    } else {
      subtract(cp_value_1, cp_value_2, &cp_result);
    }
    setScaleLong(&cp_result, res_scale);
    if (compare == -1) {
      res_sign = sign_2;
    }
    setBitLong(&cp_result, 255, res_sign);
    ans = simplify_and_transform(cp_result, result);
  }
  return ans;
}

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int ans = 0;
  setDefault(result);
  s21_decimal_long cp_value_1, cp_value_2;
  s21_decimal_long cp_result = {0};
  moveToBig(&cp_value_1, value_1);
  moveToBig(&cp_value_2, value_2);
  int sign_1 = getBitLong(cp_value_1, 255);
  int sign_2 = getBitLong(cp_value_2, 255);
  int res_sign = sign_1;
  int scale_1 = getScaleLong(cp_value_1);
  int scale_2 = getScaleLong(cp_value_2);
  int res_scale = scale_1;
  int compare;
  if (scale_1 < scale_2) {
    res_scale = scale_2;
  }
  normalizeLong(&cp_value_1, &cp_value_2);
  compare = compareMantissa(cp_value_1, cp_value_2);
  if ((sign_1 == sign_2) && (compare == 0)) {
    ;
  } else {
    if (sign_1 != sign_2) {
      adDivision(cp_value_1, cp_value_2, &cp_result);
    } else if (sign_1 == sign_2) {
      subtract(cp_value_1, cp_value_2, &cp_result);
      if (((compare == -1) && (!sign_1)) || ((compare == 1) && (sign_1))) {
        res_sign = 1;
      } else {
        res_sign = 0;
      }
    }
    setScaleLong(&cp_result, res_scale);
    setBitLong(&cp_result, 255, res_sign);
    ans = simplify_and_transform(cp_result, result);
  }
  return ans;
}

int s21_div(s21_decimal dividend, s21_decimal divisor, s21_decimal* quotient) {
  int code = OK;
  setDefault(quotient);

  if (!checkNullDecimal2(divisor)) {
    code = DEVISION_BY_ZERO;
  } else {
    s21_decimal_long div_1, div_2, int_part, remainder;
    s21_decimal_long div_result = {0};
    moveToBig(&div_1, dividend);
    moveToBig(&div_2, divisor);
    int sign_result = getSignResult(dividend, divisor);
    int scale_result = 0;

    normalizeLong(&div_1, &div_2);
    division(div_1, div_2, &int_part, &remainder);
    div_result = int_part;

    if (decimalLong_check_null(remainder)) {
      div_1 = remainder;
      int scale_remainder = scale_result;

      while (decimalLong_check_null(remainder) &&
             (scale_remainder < 29)) {  // 27
        multByTen(&div_1);
        ++scale_remainder;
        division(div_1, div_2, &int_part, &remainder);
      }

      setScaleLong(&int_part, scale_remainder);
      normalizeLong(&div_result, &int_part);
      s21_decimal_long tmp;
      adDivision(div_result, int_part, &tmp);
      div_result = tmp;
      setScaleLong(&div_result, scale_remainder);
    }
    setBitLong(&div_result, 255, sign_result);
    code = simplify_and_transform(div_result, quotient);
  }
  return code;
}

int s21_mul(s21_decimal decimal_1, s21_decimal decimal_2, s21_decimal* res) {
  int code = 0;
  for (int i = 0; i < 4; ++i) {
    res->bits[i] = 0;
  }
  if (checkNonZeroDecimal(decimal_1, decimal_2)) {
    s21_decimal_long cp_value_1, cp_value_2;
    s21_decimal_long cp_result = {0};
    moveToBig(&cp_value_1, decimal_1);
    moveToBig(&cp_value_2, decimal_2);
    int res_sign = getSignResult(decimal_1, decimal_2);
    int res_scale;
    res_scale = getScaleLong(cp_value_1) + getScaleLong(cp_value_2);
    multiply(cp_value_1, cp_value_2, &cp_result);
    setScaleLong(&cp_result, res_scale);
    setBitLong(&cp_result, 255, res_sign);
    code = simplify_and_transform(cp_result, res);
  }
  return code;
}