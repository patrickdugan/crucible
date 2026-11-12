// Regression for #1923: the false assertion must produce a counterexample.
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

int main(void)
{
  uint32_t m = crucible_uint32_t("m");
  uint64_t v0 = crucible_uint64_t("v0");
  uint64_t v1 = crucible_uint64_t("v1");
  check(repro(m, v0, v1) == 1);
  return 0;
}
