#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MINUS 0b10000000000000000000000000000000
#define MAX_DECIMAL 79228162514264337593543950335.0f

#define OK 0
#define ERROR 1
#define TOO_LARGE 1
#define TOO_SMALL 2
#define DEVISION_BY_ZERO 3

typedef struct {
  unsigned int bits[4];
} s21_decimal;

typedef struct {
  unsigned bits[8];
} s21_decimal_long;

// arithmetic operators
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal* result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal* result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal* result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal* result);

// comparison operators
int s21_is_less(s21_decimal decimal_1, s21_decimal decimal_2);
int s21_is_less_or_equal(s21_decimal decimal_1, s21_decimal decimal_2);
int s21_is_greater(s21_decimal decimal_1, s21_decimal decimal_2);
int s21_is_greater_or_equal(s21_decimal decimal_1, s21_decimal decimal_2);
int s21_is_equal(s21_decimal decimal_1, s21_decimal decimal_2);
int s21_is_not_equal(s21_decimal decimal_1, s21_decimal decimal_2);

void s21_normalize(s21_decimal* value_1, s21_decimal* value_2);
void multiply_by_ten(s21_decimal* value);
int s21_shift_left(s21_decimal* count);
void s21_add_for_compare(s21_decimal value_1, s21_decimal value_2,
                         s21_decimal* result);
int s21_compare(s21_decimal value_1, s21_decimal value_2);
int checkNonZeroDecimal(s21_decimal value_1, s21_decimal value_2);
int compareSigns(s21_decimal value_1, s21_decimal value_2);
int getCommonSign(s21_decimal value_1, s21_decimal value_2);
int s21_compare_mantissa(s21_decimal value_1, s21_decimal value_2,
                         int common_sign);
int countEqualBits(s21_decimal value_1, s21_decimal value_2);
int compareDifferentSign(s21_decimal value_1, s21_decimal value_2);
int compareSameSign(s21_decimal value_1, s21_decimal value_2);

// convertors
int s21_from_float_to_decimal(float src, s21_decimal* dst);
int s21_from_int_to_decimal(int src, s21_decimal* dst);
int s21_from_decimal_to_int(s21_decimal src, int* dst);
int s21_from_decimal_to_float(s21_decimal src, float* dst);

int getExp(float float_num);
void mantissaToDecimal(s21_decimal* dst, double float_num, int exp);
void getBinaryFloat(double num, char* binFloat);

// another functions
int s21_floor(s21_decimal value, s21_decimal* result);
int s21_round(s21_decimal value, s21_decimal* result);
int s21_truncate(s21_decimal value, s21_decimal* result);
int s21_negate(s21_decimal value, s21_decimal* result);

// common
int setDefault(s21_decimal* dec);
void setBit(s21_decimal* dec, int position, int bit);
int getBit(s21_decimal dec, int position);
void setScale(s21_decimal* dec, int scale);
int getScale(s21_decimal dec);
void setSign(s21_decimal* dec, int sign);
void divBy10(s21_decimal* dec);

// common long decimal
int checkNonZeroDecimal(s21_decimal value_1, s21_decimal value_2);
void moveToBig(s21_decimal_long* big_value, s21_decimal value);
int getSignResult(s21_decimal decimal_1, s21_decimal decimal_2);
int getScaleLong(s21_decimal_long src);
int getBitLong(s21_decimal_long dst, int index);
void setScaleLong(s21_decimal_long* dst, int scale);
void setBitLong(s21_decimal_long* dst, int index, int bit);

int divide_ten_un_overfl(s21_decimal_long* value, int* scale_value,
                         int* prev_remainder);
int simplify_and_transform(s21_decimal_long value, s21_decimal* result);
int checkOverflow(s21_decimal_long value);
int divideByTen(s21_decimal_long value, s21_decimal_long* int_part);
int discharge(s21_decimal_long* divide_by_10, s21_decimal_long* get);
int compareMantissa(s21_decimal_long value_1, s21_decimal_long decimal_2);
int shiftDecimalLongLeft(s21_decimal_long* count);
void shiftDecimalRight(s21_decimal_long* count);
void adDivision(s21_decimal_long decimal_1, s21_decimal_long decimal_2,
                s21_decimal_long* result);
void subtract(s21_decimal_long decimal_1, s21_decimal_long decimal_2,
              s21_decimal_long* result);
void getAdditionalCode(s21_decimal_long value, s21_decimal_long* result);
void roundValue(s21_decimal_long* value, int remaind, int prev_remaind);
void moveBack(s21_decimal_long big_value, s21_decimal* value);
void multiply(s21_decimal_long decimal_1, s21_decimal_long value_2,
              s21_decimal_long* result);
int decimal_check_null(s21_decimal value);
int decimalLong_check_null(s21_decimal_long value);
void normalizeLong(s21_decimal_long* value_1, s21_decimal_long* value_2);
void multByTen(s21_decimal_long* value);
void division(s21_decimal_long value_1, s21_decimal_long value_2,
              s21_decimal_long* int_part, s21_decimal_long* remaind);
int divide(s21_decimal_long* dividend, s21_decimal_long divider,
           s21_decimal_long* get);
int checkNullDecimal2(s21_decimal value);
int performShifts(s21_decimal_long* dividend, s21_decimal_long divider,
                  s21_decimal_long* get, int compare, int shift_count);

#endif  // SRC_S21_DECIMAL_H_