#include "link-quality.h"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <tuple>

namespace halowstudy {
namespace {
double Missing() { return std::numeric_limits<double>::quiet_NaN(); }
void Finite(double value) {
  if (!std::isfinite(value)) throw std::invalid_argument("non-finite input");
}
void UpdateMean(double value, std::uint64_t count, double& mean) {
  mean += (value - mean) / static_cast<double>(count);
}
}

bool LinkKey::operator<(const LinkKey& other) const {
  return std::tie(txNode, txInterface, rxNode, rxInterface) <
         std::tie(other.txNode, other.txInterface, other.rxNode, other.rxInterface);
}

LinkQuality::LinkQuality()
  : pdr(Missing()), experimentalLqiPercent(Missing()), meanSignalDbm(Missing()),
    meanPowerDbm(Missing()), meanNoiseTraceDbm(Missing()),
    meanSnrTraceDifferenceDb(Missing()), meanEffectiveSnrDb(Missing()) {}

LinkQualityWindow::LinkQualityWindow(double start, double end, double deadline,
                                     std::uint64_t minimumProbes)
  : m_start(start), m_end(end), m_deadline(deadline),
    m_minimumProbes(minimumProbes) {
  Finite(start); Finite(end); Finite(deadline);
  if (start < 0 || end <= start || deadline < end || minimumProbes == 0)
    throw std::invalid_argument("invalid cohort window or minimum sample size");
}

void LinkQualityWindow::ValidateTime(double time) {
  Finite(time);
  if (time < 0) throw std::invalid_argument("negative time");
}

void LinkQualityWindow::ValidateLink(const LinkKey& link) {
  if (link.txNode == link.rxNode && link.txInterface == link.rxInterface)
    throw std::invalid_argument("a link must have distinct endpoints");
}

void LinkQualityWindow::RequireOpen() const {
  if (m_closed) throw std::logic_error("cohort already closed");
}

bool LinkQualityWindow::Sent(const LinkKey& link, std::uint64_t probeId, double time) {
  ValidateTime(time); ValidateLink(link);
  if (time < m_start || time >= m_end) return false;
  RequireOpen();
  if (!m_links[link].probes.emplace(probeId, Probe(time)).second)
    throw std::invalid_argument("logical probe identifier reused in cohort");
  return true;
}

bool LinkQualityWindow::Attempt(const LinkKey& link, std::uint64_t probeId,
                                std::uint64_t attemptId, double time) {
  ValidateTime(time); ValidateLink(link);
  if (time < m_start || time >= m_deadline) return false;
  RequireOpen();
  auto foundLink = m_links.find(link);
  if (foundLink == m_links.end()) throw std::invalid_argument("attempt without sent probe");
  auto probe = foundLink->second.probes.find(probeId);
  if (probe == foundLink->second.probes.end())
    throw std::invalid_argument("attempt without sent probe");
  if (time < probe->second.sentAt) throw std::invalid_argument("attempt before generation");
  if (!probe->second.attemptIds.insert(attemptId).second) {
    ++foundLink->second.duplicateAttempts;
    return false;
  }
  return true;
}

bool LinkQualityWindow::Received(const LinkKey& link, std::uint64_t probeId, double time) {
  ValidateTime(time); ValidateLink(link);
  if (time < m_start || time >= m_deadline) return false;
  RequireOpen();
  State& state = m_links[link];
  auto probe = state.probes.find(probeId);
  if (probe == state.probes.end()) {
    ++state.unknownReceives;
    return false;
  }
  if (time < probe->second.sentAt) throw std::invalid_argument("receipt before generation");
  if (probe->second.received) {
    ++state.duplicateReceives;
    return false;
  }
  probe->second.received = true;
  return true;
}

bool LinkQualityWindow::SuccessfulRf(const LinkKey& link, double time, double signalDbm,
                                    double noiseTraceDbm, double rxNoiseFigureDb) {
  ValidateTime(time); ValidateLink(link);
  Finite(signalDbm); Finite(noiseTraceDbm); Finite(rxNoiseFigureDb);
  const double powerMw = std::pow(10.0, signalDbm / 10.0);
  const double differenceDb = signalDbm - noiseTraceDbm;
  const double effectiveDb = differenceDb - rxNoiseFigureDb;
  if (!std::isfinite(powerMw) || powerMw <= 0 || !std::isfinite(differenceDb) ||
      !std::isfinite(effectiveDb) || rxNoiseFigureDb < 0)
    throw std::invalid_argument("invalid RF sample");
  if (time < m_start || time >= m_deadline) return false;
  RequireOpen();
  State& state = m_links[link];
  ++state.rfSamples;
  UpdateMean(signalDbm, state.rfSamples, state.signalMean);
  UpdateMean(powerMw, state.rfSamples, state.powerMwMean);
  UpdateMean(noiseTraceDbm, state.rfSamples, state.noiseMean);
  UpdateMean(differenceDb, state.rfSamples, state.traceDifferenceMean);
  UpdateMean(effectiveDb, state.rfSamples, state.effectiveSnrMean);
  return true;
}

void LinkQualityWindow::Close(double time) {
  ValidateTime(time);
  if (time < m_deadline) throw std::invalid_argument("cannot close before reception deadline");
  m_closed = true;
}

LinkQuality LinkQualityWindow::Snapshot(const LinkKey& link) const {
  ValidateLink(link);
  LinkQuality quality;
  quality.final = m_closed;
  const auto found = m_links.find(link);
  if (found == m_links.end()) return quality;
  const State& state = found->second;
  quality.sentUnique = state.probes.size();
  for (const auto& item : state.probes) {
    const Probe& probe = item.second;
    quality.receivedUnique += probe.received ? 1 : 0;
    const std::uint64_t attempts = probe.attemptIds.size();
    quality.attemptedProbes += attempts ? 1 : 0;
    quality.physicalAttempts += attempts;
    quality.retries += attempts ? attempts - 1 : 0;
  }
  quality.duplicateReceives = state.duplicateReceives;
  quality.duplicateAttemptCallbacks = state.duplicateAttempts;
  quality.unknownProbeReceives = state.unknownReceives;
  const std::uint64_t missing = quality.sentUnique - quality.receivedUnique;
  if (m_closed) quality.notDeliveredByDeadline = missing;
  else quality.pending = missing;
  quality.pdrDefined = quality.sentUnique > 0;
  if (quality.pdrDefined)
    quality.pdr = static_cast<double>(quality.receivedUnique) / quality.sentUnique;
  quality.experimentalLqiDefined = m_closed && quality.sentUnique >= m_minimumProbes &&
                                  quality.unknownProbeReceives == 0;
  if (quality.experimentalLqiDefined) quality.experimentalLqiPercent = 100 * quality.pdr;
  quality.rfSuccessSamples = state.rfSamples;
  if (state.rfSamples) {
    quality.meanSignalDbm = state.signalMean;
    quality.meanPowerDbm = 10 * std::log10(state.powerMwMean);
    quality.meanNoiseTraceDbm = state.noiseMean;
    quality.meanSnrTraceDifferenceDb = state.traceDifferenceMean;
    quality.meanEffectiveSnrDb = state.effectiveSnrMean;
  }
  return quality;
}

std::map<LinkKey, LinkQuality> LinkQualityWindow::Snapshots() const {
  std::map<LinkKey, LinkQuality> result;
  for (const auto& item : m_links) result.emplace(item.first, Snapshot(item.first));
  return result;
}

double ExpectedLogDistanceSignalDbm(double txDbm, double txGainDb, double rxGainDb,
                                   double distanceM, double referenceDistanceM,
                                   double referenceLossDb, double exponent,
                                   double frequencyHz) {
  for (double value : {txDbm, txGainDb, rxGainDb, distanceM, referenceDistanceM,
                       referenceLossDb, exponent, frequencyHz}) Finite(value);
  if (distanceM < 0 || referenceDistanceM <= 0 || exponent < 0 || frequencyHz <= 0)
    throw std::invalid_argument("invalid LogDistance parameter");
  const double signalWithoutLoss = txDbm + txGainDb + rxGainDb;
  if (distanceM <= referenceDistanceM) return signalWithoutLoss;
  const double pathLoss = referenceLossDb +
    10 * exponent * std::log10(distanceM / referenceDistanceM) +
    21 * std::log10(frequencyHz / 900e6);
  const double result = signalWithoutLoss - pathLoss;
  Finite(result);
  return result;
}

} // namespace halowstudy
