#ifndef HALOW_STUDY_LINK_QUALITY_H
#define HALOW_STUDY_LINK_QUALITY_H

#include <cstdint>
#include <map>
#include <set>

namespace halowstudy {

// Direction and radio interfaces are part of the identity. UL and DL never pool.
struct LinkKey {
  std::uint32_t txNode, txInterface, rxNode, rxInterface;
  bool operator<(const LinkKey& other) const;
};

struct LinkQuality {
  std::uint64_t sentUnique = 0;
  std::uint64_t receivedUnique = 0;
  std::uint64_t duplicateReceives = 0;
  std::uint64_t pending = 0;
  std::uint64_t notDeliveredByDeadline = 0;
  std::uint64_t attemptedProbes = 0;
  std::uint64_t physicalAttempts = 0;
  std::uint64_t retries = 0;
  std::uint64_t duplicateAttemptCallbacks = 0;
  std::uint64_t unknownProbeReceives = 0;
  std::uint64_t rfSuccessSamples = 0;
  bool final = false;
  bool pdrDefined = false;
  bool experimentalLqiDefined = false;
  double pdr;
  double experimentalLqiPercent;
  double meanSignalDbm;
  double meanPowerDbm;  // dBm of mean linear mW; differs from meanSignalDbm.
  double meanNoiseTraceDbm;
  double meanSnrTraceDifferenceDb;
  double meanEffectiveSnrDb;  // Legacy trace difference minus RxNoiseFigure.
  LinkQuality();
};

// A cohort of one-hop probes generated in [start,end), received before deadline.
// Probe identifiers must remain unique per directed link for this whole cohort.
// This class does not know association, RAW validity or actual ns-3 trace types.
class LinkQualityWindow {
public:
  LinkQualityWindow(double start, double end, double deadline,
                    std::uint64_t minimumProbes = 20);
  bool Sent(const LinkKey& link, std::uint64_t probeId, double time);
  bool Attempt(const LinkKey& link, std::uint64_t probeId,
               std::uint64_t uniqueAttemptId, double time);
  bool Received(const LinkKey& link, std::uint64_t probeId, double time);
  // Successful PHY receptions only. Adapter must filter to the same probe class.
  bool SuccessfulRf(const LinkKey& link, double time, double signalDbm,
                    double noiseTraceDbm, double rxNoiseFigureDb);
  void Close(double time);
  LinkQuality Snapshot(const LinkKey& link) const;
  std::map<LinkKey, LinkQuality> Snapshots() const;

private:
  struct Probe {
    double sentAt;
    bool received = false;
    std::set<std::uint64_t> attemptIds;
    explicit Probe(double time) : sentAt(time) {}
  };
  struct State {
    std::map<std::uint64_t, Probe> probes;
    std::uint64_t duplicateReceives = 0, duplicateAttempts = 0;
    std::uint64_t unknownReceives = 0, rfSamples = 0;
    double signalMean = 0, powerMwMean = 0, noiseMean = 0;
    double traceDifferenceMean = 0, effectiveSnrMean = 0;
  };
  static void ValidateTime(double time);
  static void ValidateLink(const LinkKey& link);
  void RequireOpen() const;
  double m_start, m_end, m_deadline;
  std::uint64_t m_minimumProbes;
  bool m_closed = false;
  std::map<LinkKey, State> m_links;
};

// Exact deterministic LogDistance formula of the imported Pablo fork, including
// its 21*log10(f/900MHz) term and its no-loss branch at distance <= reference.
// Only applicable without additional propagation models/fading.
double ExpectedLogDistanceSignalDbm(double txDbm, double txGainDb,
                                   double rxGainDb, double distanceM,
                                   double referenceDistanceM,
                                   double referenceLossDb, double exponent,
                                   double propagationFrequencyHz);

} // namespace halowstudy
#endif
