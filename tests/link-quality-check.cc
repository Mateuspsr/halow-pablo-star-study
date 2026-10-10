#include "../implementation/link-quality.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace halowstudy;
namespace {
int checks = 0;
void Check(bool condition, const std::string& message) {
  ++checks;
  if (!condition) throw std::runtime_error(message);
}
void Near(double value, double expected, const std::string& message) {
  Check(std::isfinite(value) && std::abs(value - expected) < 1e-9, message);
}
template <typename Function> void Throws(Function action, const std::string& message) {
  bool raised = false;
  try { action(); } catch (const std::logic_error&) { raised = true; }
  Check(raised, message);
}
const LinkKey ul{2, 0, 0, 0};
const LinkKey dl{0, 0, 2, 0};
}

int main() {
  try {
    // Incomplete cohorts do not label queued/delayed probes as lost.
    LinkQualityWindow cohort(10, 12, 15, 2);
    Check(!cohort.Sent(ul, 99, 9.999), "pre-window generation excluded");
    Check(cohort.Sent(ul, 1, 10), "left endpoint admitted");
    Check(cohort.Sent(ul, 2, 11.999), "last generated probe admitted");
    Check(!cohort.Sent(ul, 3, 12), "right endpoint generation excluded");
    Throws([&] { cohort.Sent(ul, 1, 10.1); }, "logical identity reuse rejected");
    Check(cohort.Attempt(ul, 1, 100, 10), "first physical attempt");
    Check(!cohort.Attempt(ul, 1, 100, 10), "duplicate trace callback deduplicated");
    Check(cohort.Attempt(ul, 1, 101, 10.2), "retry physical attempt");
    Check(cohort.Attempt(ul, 1, 102, 10.4), "second retry physical attempt");
    Check(cohort.Received(ul, 1, 10.5), "first logical receipt");
    Check(!cohort.Received(ul, 1, 10.7), "duplicate logical receipt");
    LinkQuality early = cohort.Snapshot(ul);
    Check(early.sentUnique == 2 && early.receivedUnique == 1, "cohort denominator");
    Check(early.pending == 1 && early.notDeliveredByDeadline == 0, "pending is not loss");
    Check(!early.final && !early.experimentalLqiDefined, "unclosed score withheld");
    Check(early.duplicateReceives == 1, "duplicates separated");
    Check(early.physicalAttempts == 3 && early.retries == 2, "retry denominator isolated");
    Check(early.attemptedProbes == 1 && early.duplicateAttemptCallbacks == 1,
          "physical trace coverage and duplicates visible");
    Check(!cohort.Received(ul, 2, 15), "receipt at exclusive deadline excluded");
    Check(!cohort.Attempt(ul, 2, 200, 15), "attempt at deadline excluded");
    Throws([&] { cohort.Close(14.99); }, "early closure forbidden");
    cohort.Close(15);
    LinkQuality final = cohort.Snapshot(ul);
    Check(final.final && final.pending == 0 && final.notDeliveredByDeadline == 1,
          "terminal deadline resolves cohort");
    Check(final.experimentalLqiDefined, "minimum admitted cohort has score");
    Near(final.pdr, 0.5, "unique PDR");
    Near(final.experimentalLqiPercent, 50, "explicit percent scale");
    Throws([&] { cohort.Received(ul, 2, 14); }, "backdated mutation after closure rejected");
    cohort.Close(16);  // Idempotent terminal snapshot, no changed counts.
    Check(cohort.Snapshot(ul).notDeliveredByDeadline == 1, "idempotent closure");

    // Drain receipts belong to the generation cohort, directions/radios don't pool.
    LinkQualityWindow drain(1, 2, 4, 1);
    drain.Sent(ul, 7, 1);
    drain.Sent(dl, 7, 1);
    LinkKey otherRadio{2, 1, 0, 0};
    drain.Sent(otherRadio, 7, 1);
    Check(drain.Received(ul, 7, 3.9), "late valid drain delivery");
    drain.Close(4);
    Near(drain.Snapshot(ul).pdr, 1, "UL received");
    Near(drain.Snapshot(dl).pdr, 0, "DL failed independently");
    Near(drain.Snapshot(otherRadio).pdr, 0, "radio interfaces kept independent");
    Check(drain.Snapshots().size() == 3, "three directed interface links");

    // No transmissions and too little evidence never become a perfect quality.
    LinkQualityWindow noData(1, 2, 3, 2);
    noData.Close(3);
    const LinkQuality empty = noData.Snapshot(ul);
    Check(!empty.pdrDefined && !empty.experimentalLqiDefined && std::isnan(empty.pdr),
          "empty link undefined");
    Check(empty.rfSuccessSamples == 0 && std::isnan(empty.meanSignalDbm),
          "missing RSSI distinct from zero dBm");
    LinkQualityWindow small(1, 2, 3, 2);
    small.Sent(ul, 1, 1); small.Received(ul, 1, 1.1); small.Close(3);
    Check(small.Snapshot(ul).pdrDefined && !small.Snapshot(ul).experimentalLqiDefined,
          "insufficient sample score withheld, counts preserved");
    LinkQualityWindow unknown(1, 2, 3, 1);
    unknown.Sent(ul, 1, 1); unknown.Received(ul, 1, 1.1);
    Check(!unknown.Received(ul, 99, 1.2), "orphan receipt not credited");
    unknown.Close(3);
    Check(unknown.Snapshot(ul).unknownProbeReceives == 1 &&
          !unknown.Snapshot(ul).experimentalLqiDefined, "identity inconsistency gates score");

    // The fork's noise trace omits noise figure: restore effective reception SNR.
    LinkQualityWindow rf(1, 2, 3, 1);
    Check(rf.SuccessfulRf(ul, 1, -80, -95, 6.8), "successful RF sample one");
    Check(rf.SuccessfulRf(ul, 2.9, -60, -95, 6.8), "successful RF sample in drain");
    Check(!rf.SuccessfulRf(ul, 3, -50, -95, 6.8), "RF deadline excluded");
    rf.Close(3);
    const LinkQuality r = rf.Snapshot(ul);
    Check(r.rfSuccessSamples == 2 && !r.pdrDefined, "RF evidence doesn't invent traffic");
    Near(r.meanSignalDbm, -70, "mean of dBm values explicit");
    Near(r.meanPowerDbm, 10 * std::log10((1e-8 + 1e-6) / 2), "mean linear power separate");
    Near(r.meanNoiseTraceDbm, -95, "legacy noise retained unchanged");
    Near(r.meanSnrTraceDifferenceDb, 25, "reported difference preserved");
    Near(r.meanEffectiveSnrDb, 18.2, "noise figure correction restores SNR");

    // Analytical validation uses exact fork formula, asymmetric roles and frequency.
    Near(ExpectedLogDistanceSignalDbm(30, 3, 0, 100, 1, 8, 3.76, 900e6), -50.2,
         "Pablo DL at 100m");
    Near(ExpectedLogDistanceSignalDbm(0, 0, 3, 100, 1, 8, 3.76, 900e6), -80.2,
         "Pablo UL at 100m");
    Near(ExpectedLogDistanceSignalDbm(30, 3, 0, 1000, 1, 8, 3.76, 900e6), -87.8,
         "Pablo DL at 1km");
    Near(ExpectedLogDistanceSignalDbm(0, 0, 3, 1000, 1, 8, 3.76, 900e6), -117.8,
         "Pablo UL at 1km");
    Near(ExpectedLogDistanceSignalDbm(30, 3, 0, 100, 1, 8, 3.76, 1800e6),
         -50.2 - 21 * std::log10(2.0), "fork frequency term retained");
    Near(ExpectedLogDistanceSignalDbm(30, 3, 0, 1, 1, 8, 3.76, 900e6), 33,
         "reference-distance no-loss branch retained");
    Near(ExpectedLogDistanceSignalDbm(30, 3, 0, 0, 1, 8, 3.76, 900e6), 33,
         "coincident-position fork branch retained");

    const double nan = std::numeric_limits<double>::quiet_NaN();
    Throws([&] { LinkQualityWindow invalid(1, 1, 3); }, "empty generation rejected");
    Throws([&] { LinkQualityWindow invalid(1, 3, 2); }, "negative drain rejected");
    Throws([&] { LinkQualityWindow invalid(1, 2, 3, 0); }, "zero sample requirement rejected");
    Throws([&] { LinkQualityWindow invalid(nan, 2, 3); }, "non-finite window rejected");
    LinkQualityWindow invalidEvents(1, 2, 3, 1);
    invalidEvents.Sent(ul, 1, 1.5);
    Throws([&] { invalidEvents.Received(ul, 1, 1.4); }, "receipt before generation rejected");
    Throws([&] { invalidEvents.Attempt(ul, 1, 1, 1.4); }, "attempt before generation rejected");
    Throws([&] { invalidEvents.Attempt(dl, 1, 1, 1.5); }, "attempt without sent probe rejected");
    Throws([&] { invalidEvents.SuccessfulRf(ul, 1.5, nan, -95, 7); }, "NaN RF rejected");
    Throws([&] { invalidEvents.SuccessfulRf(ul, 1.5, -80, -95, -1); }, "invalid noise figure rejected");
    Throws([&] { invalidEvents.Sent(LinkKey{1, 0, 1, 0}, 1, 1.5); }, "self-endpoint rejected");
    Throws([&] { ExpectedLogDistanceSignalDbm(0, 0, 0, 1, 0, 8, 3.76, 900e6); },
           "zero reference distance rejected");
    Throws([&] { ExpectedLogDistanceSignalDbm(0, 0, 0, 1, 1, 8, 3.76, 0); },
           "zero frequency rejected");
    std::cout << "PASS: " << checks << " link-quality checks (synthetic unit inputs only)\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAIL after " << checks << " checks: " << error.what() << '\n';
    return 1;
  }
}
