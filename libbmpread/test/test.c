/******************************************************************************
* libbmpread - tiny, fast bitmap (.bmp) image file loader                     *
*              <https://github.com/chazomaticus/libbmpread>                   *
* Copyright (C) 2005, 2012, 2016, 2018 Charles Lindsay <chaz@chazomatic.us>   *
*                                                                             *
*  This software is provided 'as-is', without any express or implied          *
*  warranty.  In no event will the authors be held liable for any damages     *
*  arising from the use of this software.                                     *
*                                                                             *
*  Permission is granted to anyone to use this software for any purpose,      *
*  including commercial applications, and to alter it and redistribute it     *
*  freely, subject to the following restrictions:                             *
*                                                                             *
*  1. The origin of this software must not be misrepresented; you must not    *
*     claim that you wrote the original software. If you use this software    *
*     in a product, an acknowledgment in the product documentation would be   *
*     appreciated but is not required.                                        *
*  2. Altered source versions must be plainly marked as such, and must not be *
*     misrepresented as being the original software.                          *
*  3. This notice may not be removed or altered from any source distribution. *
******************************************************************************/


#include "bmpread.c"

/* I use assert for simple testing here.  It's not great, because it can be
 * hard to tell which value caused a failure, if the assertion occurs inside a
 * loop, for example.  If you get a failed test and can't figure out exactly
 * what caused it, I might suggest re-running the failed test binary under gdb.
 */
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>


static const char * const test_data = "./test.data";


static void test_CanAdd(void)
{
    assert(CanAdd(0,        0));
    assert(CanAdd(SIZE_MAX, 0));
    assert(CanAdd(0, SIZE_MAX));

    assert(!CanAdd(SIZE_MAX, SIZE_MAX));
    assert(!CanAdd(SIZE_MAX,        1));
    assert(!CanAdd(1,        SIZE_MAX));
}

static void test_CanMultiply(void)
{
    assert(CanMultiply(1,        1));
    assert(CanMultiply(SIZE_MAX, 1));
    assert(CanMultiply(1, SIZE_MAX));

    assert(!CanMultiply(SIZE_MAX, SIZE_MAX));
    assert(!CanMultiply(SIZE_MAX, 2));
    assert(!CanMultiply(2,        SIZE_MAX));
}

static void test_CanMakeSizeT(void)
{
    assert(CanMakeSizeT(0));
#if UINT32_MAX == SIZE_MAX
    assert(CanMakeSizeT(UINT32_MAX));
    assert(CanMakeSizeT(SIZE_MAX));
#elif UINT32_MAX > SIZE_MAX
    assert(!CanMakeSizeT(UINT32_MAX));
    assert(CanMakeSizeT(SIZE_MAX));
#else
    assert(CanMakeSizeT(UINT32_MAX));
#endif
}

static void test_CanMakeLong(void)
{
    assert(CanMakeLong(0));
#if UINT32_MAX == LONG_MAX
    assert(CanMakeLong(UINT32_MAX));
    assert(CanMakeLong(LONG_MAX));
#elif UINT32_MAX > LONG_MAX
    assert(!CanMakeLong(UINT32_MAX));
    assert(CanMakeLong(LONG_MAX));
#else
    assert(CanMakeLong(UINT32_MAX));
#endif
}

static void test_CanNegate(void)
{
    assert(CanNegate(0));
    assert(CanNegate(INT32_MAX));
    assert(!CanNegate(INT32_MIN));
}

static void test_ReadLittleUint32(void)
{
    uint32_t a = 0;
    uint32_t b = 0;
    uint32_t c = 0;
    FILE * fp = fopen(test_data, "rb");

    assert(ReadLittleUint32(&a, fp));
    assert(ReadLittleUint32(&b, fp));
    assert(a == UINT32_C(0x04030201));
    assert(b == UINT32_C(0x80706050));

    assert(!ReadLittleUint32(&c, fp));

    fclose(fp);
}

static void test_ReadLittleInt32(void)
{
    int32_t a = 0;
    int32_t b = 0;
    int32_t c = 0;
    FILE * fp = fopen(test_data, "rb");

    assert(ReadLittleInt32(&a, fp));
    assert(ReadLittleInt32(&b, fp));
    assert(a == INT32_C(   67305985));
    assert(b == INT32_C(-2140118960));

    assert(!ReadLittleInt32(&c, fp));

    fclose(fp);
}

static void test_ReadLittleUint16(void)
{
    uint16_t a = 0;
    uint16_t b = 0;
    uint16_t c = 0;
    uint16_t d = 0;
    uint16_t e = 0;
    FILE * fp = fopen(test_data, "rb");

    assert(ReadLittleUint16(&a, fp));
    assert(ReadLittleUint16(&b, fp));
    assert(ReadLittleUint16(&c, fp));
    assert(ReadLittleUint16(&d, fp));
    assert(a == UINT16_C(0x0201));
    assert(b == UINT16_C(0x0403));
    assert(c == UINT16_C(0x6050));
    assert(d == UINT16_C(0x8070));

    assert(!ReadLittleUint16(&e, fp));

    fclose(fp);
}

static void test_ReadUint8(void)
{
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t c = 0;
    uint8_t d = 0;
    uint8_t e = 0;
    uint8_t f = 0;
    uint8_t g = 0;
    uint8_t h = 0;
    uint8_t i = 0;
    FILE * fp = fopen(test_data, "rb");

    assert(ReadUint8(&a, fp));
    assert(ReadUint8(&b, fp));
    assert(ReadUint8(&c, fp));
    assert(ReadUint8(&d, fp));
    assert(ReadUint8(&e, fp));
    assert(ReadUint8(&f, fp));
    assert(ReadUint8(&g, fp));
    assert(ReadUint8(&h, fp));
    assert(a == UINT8_C(0x01));
    assert(b == UINT8_C(0x02));
    assert(c == UINT8_C(0x03));
    assert(d == UINT8_C(0x04));
    assert(e == UINT8_C(0x50));
    assert(f == UINT8_C(0x60));
    assert(g == UINT8_C(0x70));
    assert(h == UINT8_C(0x80));

    assert(!ReadUint8(&i, fp));

    fclose(fp);
}

static void test_ApplyBitfield(void)
{
    bitfield field = {4, 4};

    assert(ApplyBitfield(0xfafU, field) == 0xa);
    assert(ApplyBitfield(0x0U, field) == 0x0);

    field.start = 0;
    field.span = 0;

    assert(ApplyBitfield(0xffU, field) == 0x0);
}

static void test_ParseBitfield(void)
{
    bitfield field;

    assert(ParseBitfield(&field, 0x0));
    assert(field.start == 0);
    assert(field.span == 0);

    assert(ParseBitfield(&field, 0x1));
    assert(field.start == 0);
    assert(field.span == 1);

    assert(ParseBitfield(&field, 0x2));
    assert(field.start == 1);
    assert(field.span == 1);

    assert(ParseBitfield(&field, 0x3));
    assert(field.start == 0);
    assert(field.span == 2);

    assert(ParseBitfield(&field, 0xf0));
    assert(field.start == 4);
    assert(field.span == 4);

    assert(!ParseBitfield(&field, 0x81));
}

static void test_IsPowerOf2(void)
{
    uint32_t i;
    uint32_t j;

    assert(!IsPowerOf2(0));

    /* Because the values passed to this come from int32_ts which are checked
     * against being negative, the max value necessary to test here is
     * INT32_MAX, not UINT32_MAX.  It wouldn't matter, but this function is so
     * slow that I only want to care about testing values that can be seen in
     * the program.
     */
    for(i = 1; i <= (uint32_t)INT32_MAX / 2; i *= 2)
    {
        assert(IsPowerOf2(i));
        for(j = i + 1; j < i * 2; j++)
            assert(!IsPowerOf2(j));
    }

    assert(IsPowerOf2(i));
    if(i < (uint32_t)INT32_MAX)
    {
        for(j = i + 1; j < (uint32_t)INT32_MAX; j++)
            assert(!IsPowerOf2(j));
        assert(!IsPowerOf2(INT32_MAX));
    }
}

static void test_GetLineLength(void)
{
    size_t i;
    for(i = 1; i <= 32; i++)
        assert(GetLineLength(i, 1) == 4);
    assert(GetLineLength(33, 1) == 8);
    /* Etc.  TODO: test near SIZE_MAX. */

    for(i = 1; i <= 8; i++)
        assert(GetLineLength(i, 4) == 4);
    assert(GetLineLength(9, 4) == 8);

    for(i = 1; i <= 4; i++)
        assert(GetLineLength(i, 8) == 4);
    assert(GetLineLength(5, 8) == 8);

    assert(GetLineLength(1, 24) ==  4);
    assert(GetLineLength(2, 24) ==  8);
    assert(GetLineLength(3, 24) == 12);
    assert(GetLineLength(4, 24) == 12);
    assert(GetLineLength(5, 24) == 16);
    assert(GetLineLength(6, 24) == 20);
    assert(GetLineLength(7, 24) == 24);
    assert(GetLineLength(8, 24) == 24);
    assert(GetLineLength(9, 24) == 28);
    /* Etc. */
}

static void test_Make8Bits(void)
{
    assert(Make8Bits(0x0, 1) ==  0x0);
    assert(Make8Bits(0x1, 1) == 0xff);
    assert(Make8Bits(0x5, 3) == 0xb6);
    assert(Make8Bits(0xa, 4) == 0xaa);

    assert(Make8Bits(0xa5ffffff, 32) == 0xa5);
}

static void test_LoadLittleUint32(void)
{
    uint8_t buf[] = {0x1, 0x2, 0x3, 0x4};
    assert(LoadLittleUint32(buf) == 0x04030201);
}

static void test_LoadLittleUint16(void)
{
    uint8_t buf[] = {0x1, 0x2};
    assert(LoadLittleUint16(buf) == 0x0201);
}

int main(int argc, char * argv[])
{
    printf("%s: running tests\n", argv[0]);

#define TEST(x) do                            \
{                                             \
    printf("%s: testing %s...", argv[0], #x); \
    fflush(stdout);                           \
    test_##x();                               \
    printf("OK\n");                           \
} while(0)

    TEST(CanAdd);
    TEST(CanMultiply);
    TEST(CanMakeSizeT);
    TEST(CanMakeLong);
    TEST(CanNegate);
    TEST(ReadLittleUint32);
    TEST(ReadLittleInt32);
    TEST(ReadLittleUint16);
    TEST(ReadUint8);
    TEST(ApplyBitfield);
    TEST(ParseBitfield);
    TEST(IsPowerOf2);
    TEST(GetLineLength);
    TEST(Make8Bits);
    TEST(LoadLittleUint32);
    TEST(LoadLittleUint16);

#undef TEST

    printf("%s: all tests passed\n", argv[0]);
    return 0;

    (void)argc; /* Unused. */
}
