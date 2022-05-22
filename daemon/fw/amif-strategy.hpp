/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef NFD_DAEMON_FW_RFA_STRATEGY_HPP
#define NFD_DAEMON_FW_RFA_STRATEGY_HPP

#include "strategy.hpp"
#include "retx-suppression-exponential.hpp"

namespace nfd {
namespace fw {

/** \brief A forwarding strategy that forwards Interests to all FIB nexthops
 */
class AMIFStrategy : public Strategy
{
public:
  explicit
  AMIFStrategy(Forwarder& forwarder, const Name& name = getStrategyName());

  static const Name&
  getStrategyName();

public: // triggers
  void
  afterReceiveInterest(const Interest& interest, const FaceEndpoint& ingress,
                       const shared_ptr<pit::Entry>& pitEntry) override;

  void
  afterNewNextHop(const fib::NextHop& nextHop, const shared_ptr<pit::Entry>& pitEntry) override;

private:
  RetxSuppressionExponential m_retxSuppression;

NFD_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  static const time::milliseconds RETX_SUPPRESSION_INITIAL;
  static const time::milliseconds RETX_SUPPRESSION_MAX;
};

} // namespace fw
} // namespace nfd

#endif // NFD_DAEMON_FW_RFA_STRATEGY_HPP
