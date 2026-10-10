// Experimental RAW membership contract, based on the historical TGah D0.1
// all-zero group encoding. This is not an ns-3 adapter or a claim of final
// IEEE 802.11ah conformance. See study/RAW_GLOBAL_IMPLEMENTATION.md.
#ifndef HALOW_STUDY_RAW_MEMBERSHIP_H
#define HALOW_STUDY_RAW_MEMBERSHIP_H

#include <cstdint>
#include <stdexcept>

namespace halow_study {

inline bool IsAllStationsGroupD01(std::uint32_t group24)
{
  if (group24 > 0xffffffu)
    throw std::invalid_argument("RAW group exceeds the legacy 24-bit field");
  return group24 == 0;
}

inline bool RawMemberD01(std::uint16_t aid, bool associated,
                         std::uint32_t group24)
{
  const bool all = IsAllStationsGroupD01(group24);
  const std::uint16_t start = (group24 >> 2) & 0x7ffu;
  const std::uint16_t end = (group24 >> 13) & 0x7ffu;
  if (!all && start > end)
    throw std::invalid_argument("RAW interval start exceeds its end");
  if (!associated || aid == 0 || aid > 0x1fffu)
    return false;
  if (all)
    return true;
  return ((aid >> 11) == (group24 & 3u)) &&
         start <= (aid & 0x7ffu) && (aid & 0x7ffu) <= end;
}

// AP and STA must both use this mapping. Offset zero is a declared research
// profile; introducing random offsets needs coordinated signaling and RNG.
inline std::uint16_t RawSlotFullAid(std::uint16_t aid,
                                   std::uint16_t slotCount,
                                   std::uint16_t offset = 0)
{
  if (aid == 0 || aid > 0x1fffu)
    throw std::invalid_argument("Invalid AID for this fork's 13-bit identity");
  if (slotCount == 0 || slotCount > 63)
    throw std::invalid_argument("RAW slot count must be between 1 and 63");
  return (static_cast<std::uint32_t>(aid) + offset) % slotCount;
}

} // namespace halow_study
#endif
