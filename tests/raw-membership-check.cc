#include "../implementation/raw-membership.h"
#include <iostream>
#include <stdexcept>

static unsigned failures = 0;
static unsigned assertions = 0;
static void Expect(bool value, const char *name)
{
  ++assertions;
  if (!value) { ++failures; std::cerr << "FAIL " << name << '\n'; }
}

int main()
{
  using namespace halow_study;
  // These are independently specified membership sets: all 2500 stations;
  // first page AIDs 1..2047; second page AIDs 2048..2500. They are regression
  // controls, not simulated observations or packet-delivery results.
  const std::uint32_t firstPage = (2047u << 13) | (1u << 2);
  const std::uint32_t secondPage = (452u << 13) | 1u;
  unsigned allCount = 0, firstCount = 0, secondCount = 0;
  for (std::uint16_t aid = 1; aid <= 8191; ++aid) {
    const bool all = RawMemberD01(aid, true, 0);
    const bool first = RawMemberD01(aid, true, firstPage);
    const bool second = RawMemberD01(aid, true, secondPage);
    Expect(all, "all-zero group crosses every AID page");
    Expect(first == (aid <= 2047), "first page excludes other pages");
    Expect(second == (aid >= 2048 && aid <= 2500), "second page includes local AID zero");
    Expect(!RawMemberD01(aid, false, 0), "unassociated holes are not scheduled");
    if (aid <= 2500) { allCount += all; firstCount += first; secondCount += second; }
  }
  Expect(allCount == 2500 && firstCount == 2047 && secondCount == 453,
         "one all-stations group covers the confirmed population");
  Expect(!RawMemberD01(0, true, 0) && !RawMemberD01(8192, true, 0),
         "invalid AIDs do not acquire membership");
  const std::uint16_t aids[] = {1023, 1024, 2047, 2048, 2500, 8191};
  const std::uint16_t slots3[] = {0, 1, 1, 2, 1, 1};
  const std::uint16_t slots7[] = {1, 2, 3, 4, 1, 1};
  for (unsigned i = 0; i != 6; ++i) {
    Expect(RawSlotFullAid(aids[i], 3) == slots3[i], "three slots across mask boundaries");
    Expect(RawSlotFullAid(aids[i], 7) == slots7[i], "seven slots across mask boundaries");
    Expect(RawSlotFullAid(aids[i], 1) == 0, "one slot contains all members");
  }
  Expect(RawSlotFullAid(2048, 3, 1) == 0, "coordinated offset mapping");
  bool rejected = false;
  try { RawSlotFullAid(1, 0); } catch (const std::invalid_argument &) { rejected = true; }
  Expect(rejected, "zero slots are rejected before division");
  rejected = false;
  try { RawMemberD01(1, true, (10u << 2) | (9u << 13)); }
  catch (const std::invalid_argument &) { rejected = true; }
  Expect(rejected, "reversed page intervals are rejected");
  rejected = false;
  try { IsAllStationsGroupD01(0x1000000u); }
  catch (const std::invalid_argument &) { rejected = true; }
  Expect(rejected, "overwide group fields are rejected");
  std::cout << "assertions=" << assertions << " failures=" << failures
            << " plannedAllStations=" << allCount << '\n';
  return failures ? 1 : 0;
}
