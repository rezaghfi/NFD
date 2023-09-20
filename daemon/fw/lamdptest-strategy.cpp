
#include "lamdptest-strategy.hpp"
#include "algorithm.hpp"
#include "common/logger.hpp"

namespace nfd {
namespace fw {

NFD_REGISTER_STRATEGY(lamdptestStrategy);

NFD_LOG_INIT(lamdptestStrategy);

const time::milliseconds lamdptestStrategy::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds lamdptestStrategy::RETX_SUPPRESSION_MAX(250);

lamdptestStrategy::lamdptestStrategy(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , m_retxSuppression(RETX_SUPPRESSION_INITIAL,
                      RetxSuppressionExponential::DEFAULT_MULTIPLIER,
                      RETX_SUPPRESSION_MAX)
{
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    NDN_THROW(std::invalid_argument("lamdptestStrategy does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    NDN_THROW(std::invalid_argument(
      "lamdptestStrategy does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

const Name&
lamdptestStrategy::getStrategyName()
{
  static const auto strategyName = Name("/localhost/nfd/strategy/lamdptesttest").appendVersion(4);
  return strategyName;
}

void
lamdptestStrategy::afterReceiveInterest(const Interest& interest, const FaceEndpoint& ingress,
                                        const shared_ptr<pit::Entry>& pitEntry)
{
  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  for (const auto& nexthop : nexthops) {
    Face& outFace = nexthop.getFace();

    RetxSuppressionResult suppressResult = m_retxSuppression.decidePerUpstream(*pitEntry, outFace);

    if (suppressResult == RetxSuppressionResult::SUPPRESS) {
      NFD_LOG_DEBUG(interest << " from=" << ingress << " to=" << outFace.getId() << " suppressed");
      continue;
    }

    if (!isNextHopEligible(ingress.face, interest, nexthop, pitEntry)) {
      continue;
    }

    NFD_LOG_DEBUG(interest << " from=" << ingress << " pitEntry-to=" << outFace.getId());
    auto* sentOutRecord = this->sendInterest(interest, outFace, pitEntry);
    if (sentOutRecord && suppressResult == RetxSuppressionResult::FORWARD) {
      m_retxSuppression.incrementIntervalForOutRecord(*sentOutRecord);
    }
  }
}

void
lamdptestStrategy::afterNewNextHop(const fib::NextHop& nextHop,
                                   const shared_ptr<pit::Entry>& pitEntry)
{
  // no need to check for suppression, as it is a new next hop

  auto nextHopFaceId = nextHop.getFace().getId();
  auto& interest = pitEntry->getInterest();

  // try to find an incoming face record that doesn't violate scope restrictions
  for (const auto& r : pitEntry->getInRecords()) {
    auto& inFace = r.getFace();
    if (isNextHopEligible(inFace, interest, nextHop, pitEntry)) {

      NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " pitEntry-to=" << nextHopFaceId);
      this->sendInterest(interest, nextHop.getFace(), pitEntry);

      break; // just one eligible incoming face record is enough
    }
  }

  // if nothing found, the interest will not be forwarded
}

} // namespace fw
} // namespace nfd
