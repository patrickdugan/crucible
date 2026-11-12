// Regression for #1923: affine offsets must be interpreted modulo 2^w.
#include <crucible.h>
#include <stdint.h>

static int repro(uint32_t m, uint64_t v0, uint64_t v1)
{
  if (v0 == 0 || v1 == 0) return 1;
  uint64_t a[8];
  __builtin_memset(a, 0, sizeof a);
  a[2] = v0;
  a[3] = v1;
  uint64_t j = (m & 1) ? 6 : 5;
  return a[8 - j] == 0;
}

static uint64_t reverse_load(uint32_t m, uint64_t v0, uint64_t v1)
{
  uint64_t a[8];
  __builtin_memset(a, 0, sizeof a);
  a[2] = v0;
  a[3] = v1;
  uint64_t j = (m & 1) ? 6 : 5;
  return a[8 - j];
}

static uint64_t forward_load(uint32_t m, uint64_t v0, uint64_t v1)
{
  uint64_t a[8];
  __builtin_memset(a, 0, sizeof a);
  a[2] = v0;
  a[3] = v1;
  uint64_t j = (m & 1) ? 6 : 5;
  return a[j - 3];
}

static uint64_t reverse_load_memcpy(uint32_t m, uint64_t v0, uint64_t v1)
{
  uint64_t zero[8] = {0};
  uint64_t a[8];
  __builtin_memcpy(a, zero, sizeof a);
  a[2] = v0;
  a[3] = v1;
  uint64_t j = (m & 1) ? 6 : 5;
  return a[8 - j];
}

static uint64_t reverse_load_struct(uint32_t m, uint64_t v0, uint64_t v1)
{
  struct words { uint64_t a[8]; } zero = {{0}}, out;
  out = zero;
  out.a[2] = v0;
  out.a[3] = v1;
  uint64_t j = (m & 1) ? 6 : 5;
  return out.a[8 - j];
}

static uint64_t reverse_load_no_fill(uint32_t m, uint64_t v0, uint64_t v1)
{
  uint64_t a[8];
  a[2] = v0;
  a[3] = v1;
  uint64_t j = (m & 1) ? 6 : 5;
  return a[8 - j];
}

// Exercise the constant-load / affine-store case.
static uint64_t reverse_store(uint32_t m, uint64_t v)
{
  uint64_t a[8];
  __builtin_memset(a, 0, sizeof a);
  uint64_t j = (m & 1) ? 6 : 5;
  a[8 - j] = v;
  return a[2];
}

// Exercise the affine-load / affine-store case with independent variables.
static uint64_t reverse_both(uint32_t m, uint32_t n, uint64_t v)
{
  uint64_t a[8];
  __builtin_memset(a, 0, sizeof a);
  uint64_t j = (m & 1) ? 6 : 5;
  uint64_t k = (n & 1) ? 7 : 6;
  a[8 - j] = v;
  return a[9 - k];
}

// Positive coefficients also need the modulus: 3*k wraps to 1 or 2.
static uint64_t positive_wrap(uint64_t k, uint64_t v0, uint64_t v1)
{
  uint64_t a[4];
  __builtin_memset(a, 0, sizeof a);
  a[1] = v0;
  a[2] = v1;
  uint64_t i = 3 * k;
  return a[i];
}

int main(void)
{
  uint32_t m = crucible_uint32_t("m");
  uint32_t n = crucible_uint32_t("n");
  uint64_t v0 = crucible_uint64_t("v0");
  uint64_t v1 = crucible_uint64_t("v1");
  uint64_t k = crucible_uint64_t("k");
  assuming(k == UINT64_C(12297829382473034411) ||
           k == UINT64_C(6148914691236517206));

  uint64_t expected = (m & 1) ? v0 : v1;
  check(repro(m, v0, v1) == (v0 == 0 || v1 == 0));
  check(reverse_load(m, v0, v1) == expected);
  // j-3 visits the same two elements in the opposite order.
  check(forward_load(m, v0, v1) == ((m & 1) ? v1 : v0));
  check(reverse_load_memcpy(m, v0, v1) == expected);
  check(reverse_load_struct(m, v0, v1) == expected);
  check(reverse_load_no_fill(m, v0, v1) == expected);
  check(reverse_store(m, v0) == ((m & 1) ? v0 : 0));
  check(reverse_both(m, n, v0) == (((m & 1) == (n & 1)) ? v0 : 0));
  check(positive_wrap(k, v0, v1) ==
        ((k == UINT64_C(12297829382473034411)) ? v0 : v1));
  return 0;
}
