#include "s21_decimal.h"

void setSign(s21_decimal* dec, int sign) {
  unsigned int mask = 1u << 31;
  if (sign == 1) {
    dec->bits[3] |= mask;
  }
  if (sign == 0) {
    dec->bits[3] &= ~mask;
  }
}

void setScale(s21_decimal* dec, int scale) {
  dec->bits[3] = 0;
  scale <<= 16;
  dec->bits[3] |= scale;
}

int getScale(s21_decimal dec) {
  unsigned int mask = 0xFF << 16;
  return ((dec.bits[3] & mask) >> 16);
}

void setBit(s21_decimal* dec, int position, int bit) {
  unsigned int mask = 1 << (position % 32);
  if (bit)
    dec->bits[position / 32] |= mask;
  else
    dec->bits[position / 32] &= ~(mask);
}

int getBit(s21_decimal dec, int position) {
  unsigned int mask = 1 << (position % 32);
  return ((dec.bits[position / 32] &= mask) != 0);
}

int setDefault(s21_decimal* dec) {
  int ans = 1;
  if (dec) {
    dec->bits[0] = dec->bits[1] = dec->bits[2] = dec->bits[3] = 0;
  } else
    ans = 0;
  return ans;
}

void divBy10(s21_decimal* dec) {
  unsigned long long remainder = 0, quotient;
  for (int i = 2; i >= 0; i--) {
    quotient = 0;
    quotient = (unsigned long long)dec->bits[i] + remainder;
    dec->bits[i] = (unsigned int)(quotient / 10);
    remainder = (quotient % 10) << 32;
  }
}

// FUNCTIONS FOR BIG DECIMAL

void moveToBig(s21_decimal_long* big_value, s21_decimal value) {
  memset(big_value->bits, 0, sizeof(big_value->bits));
  for (int i = 0; i <= 2; ++i) {
    big_value->bits[i] = value.bits[i];
  }
  big_value->bits[7] = value.bits[3];
}

int getSignResult(s21_decimal decimal_1, s21_decimal decimal_2) {
  return (getBit(decimal_1, 127) == getBit(decimal_2, 127)) ? 0 : 1;
}

int getScaleLong(s21_decimal_long src) {
  int result = 0;
  for (int i = 0; i <= 7; ++i) {
    result |= (getBitLong(src, i + 240) << i);
  }
  return result;
}

int getBitLong(s21_decimal_long dst, int index) {
  unsigned int i = index / 32;
  index %= 32;
  return (dst.bits[i] >> index) & 1;
}

void setScaleLong(s21_decimal_long* dst, int scale) {
  for (int i = 0; i < 8; ++i) {
    setBitLong(dst, i + 240, scale & (1 << i));
  }
}

void setBitLong(s21_decimal_long* dst, int index, int bit) {
  unsigned int mask = 1 << (index % 32);
  if (bit)
    dst->bits[index / 32] |= mask;
  else
    dst->bits[index / 32] &= ~(mask);
}

int simplify_and_transform(s21_decimal_long value, s21_decimal* result) {
  int check = checkOverflow(value);

  if (check) {
    int scale_value = getScaleLong(value);
    int sign_value = getBitLong(value, 255);
    int remainder = 0;
    int prev_remainder = 0;

    if (scale_value) {
      remainder = divide_ten_un_overfl(&value, &scale_value, &prev_remainder);
    }

    check = checkOverflow(value);

    if (check) {
      if (sign_value) {
        check = TOO_SMALL;
      }
      memset(result->bits, 0, sizeof(result->bits));
    } else {
      roundValue(&value, remainder, prev_remainder);
      setScaleLong(&value, scale_value);
      setBitLong(&value, 255, sign_value);
      moveBack(value, result);
    }
  } else {
    moveBack(value, result);
  }

  return check;
}

int divide_ten_un_overfl(s21_decimal_long* value, int* scale_value,
                         int* prev_remainder) {
  int remainder = 0;
  s21_decimal_long int_part = {0};

  while (checkOverflow(*value) && *scale_value) {
    *prev_remainder += remainder;
    remainder = divideByTen(*value, &int_part);
    --(*scale_value);
    *value = int_part;
  }

  return remainder;
}

int checkOverflow(s21_decimal_long value) {  // ??
  int hasOverflow = 0;

  for (int i = 96; i < 224; ++i) {
    hasOverflow |= getBitLong(value, i);
  }

  return hasOverflow;
}

int divideByTen(s21_decimal_long value, s21_decimal_long* int_part) {
  memset(int_part->bits, 0, sizeof(int_part->bits));
  s21_decimal_long copy_value = value;
  s21_decimal_long tmp = {0};
  s21_decimal_long get;
  int division_is_possible = discharge(&copy_value, &get);
  while (division_is_possible) {
    adDivision(*int_part, get, &tmp);
    *int_part = tmp;
    subtract(value, copy_value, &tmp);
    value = tmp;
    copy_value = value;
    division_is_possible = discharge(&copy_value, &get);
  }
  return value.bits[0];
}

int discharge(s21_decimal_long* divide_value, s21_decimal_long* result) {
  memset(result->bits, 0, sizeof(result->bits));
  s21_decimal_long ten = {0};
  ten.bits[0] = 10;
  int is_less = 1;
  int shift_count = 0;
  int is_division_possible = 0;
  int comparison = 0;

  while (is_less) {
    comparison = compareMantissa(*divide_value, ten);

    if (comparison == 0) {
      setBitLong(result, 0, 1);
      is_less = 0;
      is_division_possible = 1;
    } else if (comparison == -1) {
      is_less = 0;
    } else {
      shiftDecimalLongLeft(&ten);
      ++shift_count;
    }
  }

  if (shift_count > 0) {
    if (comparison == -1) {
      shiftDecimalRight(&ten);
      setBitLong(result, shift_count - 1, 1);
    } else if (comparison == 0) {
      memset(result->bits, 0, sizeof(result->bits));
      setBitLong(result, shift_count, 1);
    }

    *divide_value = ten;
    is_division_possible = 1;
  }

  return is_division_possible;
}

int compareMantissa(s21_decimal_long value1, s21_decimal_long value2) {
  int result = 0;

  for (int i = 6; i >= 0; --i) {
    if (value1.bits[i] < value2.bits[i]) {
      result = -1;
      break;
    } else if (value1.bits[i] > value2.bits[i]) {
      result = 1;
      break;
    }
  }

  return result;
}

int shiftDecimalLongLeft(s21_decimal_long* value) {
  s21_decimal_long shifted = {0};
  for (int i = 1; i < 224; i++) {
    if (getBitLong(*value, i - 1)) {
      setBitLong(&shifted, i, 1);
    }
  }
  *value = shifted;
  return 0;
}

void shiftDecimalRight(s21_decimal_long* value) {
  for (int i = 0; i < 223; i++) {
    setBitLong(value, i, getBitLong(*value, i + 1));
  }
  setBitLong(value, 223, 0);
}

void adDivision(s21_decimal_long decimal_1, s21_decimal_long decimal_2,
                s21_decimal_long* result) {
  memset(result->bits, 0, sizeof(result->bits));
  int carry = 0;

  for (int i = 0; i < 224; ++i) {
    int bit1 = getBitLong(decimal_1, i);
    int bit2 = getBitLong(decimal_2, i);
    int sum = bit1 ^ bit2 ^ carry;
    carry = (bit1 & bit2) | (bit1 & carry) | (bit2 & carry);
    setBitLong(result, i, sum);
  }
}

void subtract(s21_decimal_long value1, s21_decimal_long value2,
              s21_decimal_long* result) {
  memset(result->bits, 0, sizeof(result->bits));
  s21_decimal_long additionalCodeValue2 = {0};
  s21_decimal_long additionalCodeResult = {0};
  int comparison = compareMantissa(value1, value2);
  getAdditionalCode(value2, &additionalCodeValue2);
  adDivision(value1, additionalCodeValue2, result);
  if (comparison == -1) {
    getAdditionalCode(*result, &additionalCodeResult);
    for (int i = 6; i >= 0; --i) {
      result->bits[i] = additionalCodeResult.bits[i];
    }
  }
}

void getAdditionalCode(s21_decimal_long value, s21_decimal_long* result) {
  for (int i = 6; i >= 0; --i) {
    result->bits[i] = ~value.bits[i];
  }

  s21_decimal_long one = {0};
  s21_decimal_long tmp = {0};
  one.bits[0] = 1;

  adDivision(*result, one, &tmp);
  *result = tmp;
}

void roundValue(s21_decimal_long* value, int remainder, int prev_remainder) {
  int is_even = getBitLong(*value, 0);
  int should_round = 0;

  if (is_even && remainder > 4) {
    should_round = 1;
  } else if (!is_even && remainder == 5 && prev_remainder) {
    should_round = 1;
  }

  if (should_round) {
    s21_decimal_long rounded_value;
    s21_decimal_long unit = {0};
    unit.bits[0] = 1;
    adDivision(*value, unit, &rounded_value);
    *value = rounded_value;
  }
}

void moveBack(s21_decimal_long source, s21_decimal* destination) {
  for (int i = 0; i < 3; ++i) {
    destination->bits[i] = source.bits[i];
  }
  destination->bits[3] = source.bits[7];
}

void multiply(s21_decimal_long decimal_1, s21_decimal_long decimal_2,
              s21_decimal_long* res) {
  s21_decimal_long cp_dec = decimal_2;
  s21_decimal_long tmp = {0};
  for (int i = 0; i < 96; ++i) {
    if (getBitLong(decimal_1, i)) {
      for (int j = 0; j < i; ++j) {
        shiftDecimalLongLeft(&cp_dec);
      }
      adDivision(*res, cp_dec, &tmp);
      *res = tmp;
      cp_dec = decimal_2;
    }
  }
}

int decimal_check_null(s21_decimal value) {
  s21_decimal zero = {0};
  return memcmp(&value, &zero, sizeof(s21_decimal));
}

void division(s21_decimal_long dividend, s21_decimal_long divisor,
              s21_decimal_long* quotient, s21_decimal_long* remainder) {
  memset(quotient->bits, 0, sizeof(quotient->bits));
  memset(remainder->bits, 0, sizeof(remainder->bits));

  s21_decimal_long tmp_quotient = {0};
  s21_decimal_long tmp_remainder = dividend;
  s21_decimal_long quotient_get;

  int division_possible = divide(&tmp_remainder, divisor, &quotient_get);

  if (!division_possible) {
    *remainder = dividend;
  }

  while (division_possible) {
    adDivision(*quotient, quotient_get, &tmp_quotient);
    *quotient = tmp_quotient;

    subtract(dividend, tmp_remainder, remainder);
    dividend = *remainder;
    tmp_remainder = dividend;

    division_possible = divide(&tmp_remainder, divisor, &quotient_get);
  }
}

void normalizeLong(s21_decimal_long* decimal_1, s21_decimal_long* decimal_2) {
  int diff = getScaleLong(*decimal_1) - getScaleLong(*decimal_2);

  if (diff > 0) {
    for (int i = 0; i < diff; ++i) {
      multByTen(decimal_2);
    }
  } else if (diff < 0) {
    diff = -diff;
    for (int i = 0; i < diff; ++i) {
      multByTen(decimal_1);
    }
  }
}

void multByTen(s21_decimal_long* value) {
  s21_decimal_long result = {0};
  s21_decimal_long copy_decimal = {0};
  shiftDecimalLongLeft(value);
  copy_decimal = *value;
  shiftDecimalLongLeft(value);
  shiftDecimalLongLeft(value);
  adDivision(*value, copy_decimal, &result);
  *value = result;
}

int decimalLong_check_null(s21_decimal_long value) {
  return memcmp(value.bits, &value.bits[1],
                sizeof(value.bits) - sizeof(value.bits[0]));
}

int divide(s21_decimal_long* dividend, s21_decimal_long divider,
           s21_decimal_long* get) {
  memset(get->bits, 0, sizeof(get->bits));
  int is_less = 1;
  int division_is_possible = 0;
  int compare = 0;
  int shift_count = 0;

  while (is_less) {
    compare = compareMantissa(*dividend, divider);

    if (!compare) {
      setBitLong(get, 0, 1);
      is_less = 0;
      division_is_possible = 1;
    } else if (compare == -1) {
      is_less = 0;
    } else {
      shiftDecimalLongLeft(&divider);
      ++shift_count;
    }
  }

  if (shift_count)
    division_is_possible =
        performShifts(dividend, divider, get, compare, shift_count);

  return division_is_possible;
}

int performShifts(s21_decimal_long* dividend, s21_decimal_long divider,
                  s21_decimal_long* get, int compare, int shift_count) {
  if (compare == -1) {
    shiftDecimalRight(&divider);
    setBitLong(get, (shift_count - 1), 1);
  } else if (!compare) {
    memset(get->bits, 0, sizeof(get->bits));
    setBitLong(get, (shift_count), 1);
  }
  *dividend = divider;
  return 1;
}

int checkNullDecimal2(s21_decimal value) {
  int is_nonzero = 0;
  for (int i = 0; i <= 2; i++) {
    if (value.bits[i]) {
      is_nonzero = 1;
      break;
    }
  }
  return is_nonzero;
}