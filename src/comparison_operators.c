#include "s21_decimal.h"

int s21_is_less(s21_decimal decimal_1, s21_decimal decimal_2) {
  int result = 0;
  int compare_result = s21_compare(decimal_1, decimal_2);
  if (compare_result == -1) {
    result = 1;
  }
  return result;
}

// Функция для сравнения двух чисел Decimal: a <= b
int s21_is_less_or_equal(s21_decimal decimal_1, s21_decimal decimal_2) {
  return s21_is_equal(decimal_1, decimal_2) ||
         s21_is_less(decimal_1, decimal_2);
}

// Функция для сравнения двух чисел Decimal: a > b
int s21_is_greater(s21_decimal decimal_1, s21_decimal decimal_2) {
  int result = 0;
  int compare_result = s21_compare(decimal_1, decimal_2);
  if (compare_result == 1) {
    result = 1;
  }
  return result;
}

// Функция для сравнения двух чисел Decimal: a >= b
int s21_is_greater_or_equal(s21_decimal decimal_1, s21_decimal decimal_2) {
  return s21_is_equal(decimal_1, decimal_2) ||
         s21_is_greater(decimal_1, decimal_2);
}

// Функция для сравнения двух чисел Decimal: a != b
int s21_is_not_equal(s21_decimal decimal_1, s21_decimal decimal_2) {
  return !s21_is_equal(decimal_1, decimal_2);
}

// Функция для сравнения двух чисел Decimal: a == b
int s21_is_equal(s21_decimal decimal_1, s21_decimal decimal_2) {
  int return_code = 1;
  int x;
  x = s21_compare(decimal_1, decimal_2);
  if (x != 0) {
    return_code = 0;
  }
  return return_code;
}

int s21_compare(s21_decimal value_1, s21_decimal value_2) {
  int is_decimal_nonzero = checkNonZeroDecimal(value_1, value_2);
  int sign_comparison = compareSigns(value_1, value_2);
  int common_sign = getCommonSign(value_1, value_2);

  return is_decimal_nonzero
             ? (sign_comparison != 0
                    ? sign_comparison
                    : s21_compare_mantissa(value_1, value_2, common_sign))
             : 0;
}

int checkNonZeroDecimal(s21_decimal value_1, s21_decimal value_2) {
  int is_nonzero = 0;
  for (int i = 2; i >= 0; --i) {
    if (value_1.bits[i] || value_2.bits[i]) {
      is_nonzero = 1;
      break;
    }
  }
  return is_nonzero;
}

int compareSigns(s21_decimal value_1, s21_decimal value_2) {
  int code = 0;
  int sign_1 = getBit(value_1, 127);
  int sign_2 = getBit(value_2, 127);

  if (sign_1 && !sign_2) {
    code = -1;
  } else if (!sign_1 && sign_2) {
    code = 1;
  }
  return code;
}

int getCommonSign(s21_decimal value_1, s21_decimal value_2) {
  int sign_1 = getBit(value_1, 127);
  int sign_2 = getBit(value_2, 127);
  return sign_1 + sign_2;
}

int s21_compare_mantissa(s21_decimal value_1, s21_decimal value_2,
                         int common_sign) {
  int return_code = -1;

  s21_normalize(&value_1, &value_2);
  int equal = countEqualBits(value_1, value_2);

  if (common_sign == 2) {
    return_code = compareDifferentSign(value_1, value_2);
  } else if (common_sign == 0) {
    return_code = compareSameSign(value_1, value_2);
  }

  if (equal == 4) {
    return_code = 0;
  }

  return return_code;
}

// Функция подсчета количества равных битов в мантиссе двух чисел
int countEqualBits(s21_decimal value_1, s21_decimal value_2) {
  int equal = 0;
  for (int i = 3; i >= 0; --i) {
    if (value_1.bits[i] == value_2.bits[i]) {
      ++equal;
    }
  }
  return equal;
}

// Функция сравнения мантисс двух чисел при разных знаках
int compareDifferentSign(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  for (int i = 3; i >= 0; --i) {
    if (value_1.bits[i] != value_2.bits[i]) {
      result = (value_1.bits[i] > value_2.bits[i]) ? -1 : 1;
      break;
    }
  }
  return result;
}

// Функция сравнения мантисс двух чисел при одинаковых знаках
int compareSameSign(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  for (int i = 3; i >= 0; --i) {
    if (value_1.bits[i] != value_2.bits[i]) {
      result = (value_1.bits[i] < value_2.bits[i]) ? -1 : 1;
      break;
    }
  }
  return result;
}

void s21_normalize(s21_decimal* value_1, s21_decimal* value_2) {
  int scale1 = getScale(*value_1);
  int scale2 = getScale(*value_2);

  value_1->bits[3] = 0;
  value_2->bits[3] = 0;
  if (scale1 > scale2) {
    int diff_scale = scale1 - scale2;
    for (int i = 0; i < diff_scale; ++i) {
      multiply_by_ten(value_2);
    }
  } else if (scale2 > scale1) {
    int diff_scale = scale2 - scale1;
    for (int i = 0; i < diff_scale; ++i) {
      multiply_by_ten(value_1);
    }
  }
}

void multiply_by_ten(s21_decimal* value) {
  s21_decimal tmp = {0};
  s21_decimal copy_value = {0};
  s21_shift_left(value);
  copy_value = *value;
  s21_shift_left(value);
  s21_shift_left(value);
  s21_add_for_compare(*value, copy_value, &tmp);
  *value = tmp;
}

int s21_shift_left(s21_decimal* value) {
  s21_decimal result = {0};
  for (int i = 1; i < 128; i++) {
    if (getBit(*value, i - 1)) {
      setBit(&result, i, 1);
    }
  }
  *value = result;
  return result.bits[3];
}

void s21_add_for_compare(s21_decimal value_1, s21_decimal value_2,
                         s21_decimal* result) {
  unsigned int carry = 0;
  for (int i = 0; i < 128; ++i) {
    unsigned int value_1_bit = getBit(value_1, i);
    unsigned int value_2_bit = getBit(value_2, i);
    unsigned int sum = value_1_bit + value_2_bit + carry;
    unsigned int result_bit = sum & 1;
    carry = sum >> 1;
    setBit(result, i, result_bit);
  }
}