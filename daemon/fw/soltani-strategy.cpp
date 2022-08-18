/*
 * topsis-strategy.cpp
 *
 *  Created on: Jun 26, 2019
 *      Author: reza
 */
#define BW 1000000
#define RoutingMetric 1

#include "soltani-strategy.hpp"
#include "algorithm.hpp"
#include "core/logger.hpp"
#include "topsis.hpp"

namespace nfd
{
	namespace fw
	{

		NFD_LOG_INIT(SoltaniStrategy);
		NFD_REGISTER_STRATEGY(SoltaniStrategy);

		const time::milliseconds SoltaniStrategy::RETX_SUPPRESSION_INITIAL(10);
		const time::milliseconds SoltaniStrategy::RETX_SUPPRESSION_MAX(250);

		ns3::QueueSize qz;
		/**
		 * \brief constructor of soltani-strategy
		 * @param forwarder instance for run strategy
		 * @param name of strategy
		 */
		SoltaniStrategy::SoltaniStrategy(Forwarder &forwarder, const Name &name)
				: Strategy(forwarder), m_measurements(getMeasurements()), m_retxSuppression(RETX_SUPPRESSION_INITIAL,
																																										RetxSuppressionExponential::DEFAULT_MULTIPLIER,
																																										RETX_SUPPRESSION_MAX)
		{
			this->countAfterReceiveInterest = 0;
			this->countAfterReceiveNack = 0;
			this->countBeforeSatisfyInterest = 0;

			ParsedInstanceName parsed = parseInstanceName(name);
			if (!parsed.parameters.empty())
			{
				BOOST_THROW_EXCEPTION(std::invalid_argument("soltani strategy does not accept parameters"));
			}
			if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion())
			{
				BOOST_THROW_EXCEPTION(std::invalid_argument(
						"soltani strategy does not support version " + to_string(*parsed.version)));
			}
			this->setInstanceName(makeInstanceName(name, getStrategyName()));
		}

		/**
		 * \brief create name for use in scenario
		 * @return strategyName
		 */
		const Name &
		SoltaniStrategy::getStrategyName()
		{
			static Name strategyName("/localhost/nfd/strategy/soltani/%FD%01");
			return strategyName;
		}

		/**
		 * \brief trigger after receiving Interest
		 * @param inface
		 * @param interest
		 * @param pitEntry
		 */
		void
		SoltaniStrategy::afterReceiveInterest(const Face &inFace, const Interest &interest,
																					const shared_ptr<pit::Entry> &pitEntry)
		{
			/* todo interest can three kinds:
			 * NEW, FORWARD, SUppress
			 */
			RetxSuppressionResult suppressResult = m_retxSuppression.decidePerPitEntry(*pitEntry);

			switch (suppressResult)
			{
			case RetxSuppressionResult::NEW:
			case RetxSuppressionResult::FORWARD:
				break;
			case RetxSuppressionResult::SUPPRESS:
				NFD_LOG_DEBUG("SUPPRESS Interest:" << interest << " from=" << inFace.getId() << "DROPED");
				return;
			}

			// find fibentry with pitEntry Pointer
			const fib::Entry &fibEntry = this->lookupFib(*pitEntry);
			const fib::NextHopList &nexthops = fibEntry.getNextHops();

			// if vector size is zero, send NO_ROUTE NACK and return
			if (nexthops.size() == 0)
			{
				sendNoRouteNack(inFace, interest, pitEntry);
				this->rejectPendingInterest(pitEntry);
				return;
			}

			// call function to set output face to choose best face
			Face *faceToUse = getBestFaceForForwarding(fibEntry, interest, inFace);

			if (faceToUse == nullptr)
			{
				sendNoRouteNack(inFace, interest, pitEntry);
				this->rejectPendingInterest(pitEntry);
				return;
			}

			this->sendInterest(pitEntry, *faceToUse, interest);
			NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
														 << " pitEntry-to=" << faceToUse->getId());
			this->countAfterReceiveInterest++;
		}

		/**
		 * \brief trigger after receiving Nack
		 * @param inface
		 * @param interest
		 * @param pitEntry
		 */
		void
		SoltaniStrategy::afterReceiveNack(const Face &inFace, const lp::Nack &nack,
																			const shared_ptr<pit::Entry> &pitEntry)
		{

			this->countAfterReceiveNack++;
		}

		/**
		 * \brief trigger before satisfy Interest
		 * @param pitEntry
		 * @param data
		 */
		void
		SoltaniStrategy::beforeSatisfyInterest(const shared_ptr<pit::Entry> &pitEntry,
																					 const Face &inFace, const Data &data)
		{

			this->countBeforeSatisfyInterest++;
		}

		/**
		 * \brief for calling topsis class methods
		 * @param states
		 * @param rows the number of faces in fibEntry
		 * @return rows topsis choose best face row.
		 */
		int result(FaceStats *stats, int rows)
		{

			static const myRtt S_RTT_TIMEOUT = time::microseconds::max();
			static const myRtt S_RTT_NO_MEASUREMENT = S_RTT_TIMEOUT / 2;
			int col = 3;
			float *q;
			for (int var = 0; var < rows; ++var)
			{
				B[var] = stats->BW;
				H[var] = stats->hopCount;
				if (stats->rtt == asf::RttStats::RTT_NO_MEASUREMENT)
				{
					T[var] = (float)S_RTT_NO_MEASUREMENT.count();
				}
				else
				{
					T[var] = (float)stats->rtt.count();
				}
			}
			for (int var = 0; var < rows; ++var)
			{
				if (methed == 1)
				{
				float a = (1 - ((-1 / LN[M])*(B[var] * Ln[B[var]]))) + (1 - ((-1/LN(M))*((1-H[var])*(LN(1-H[var]))) + (1-((-1/LN(M))((1-T[var]*[LN(1-T[var])])))))));
				float q[var] = (WB / a) * (B[var] + 1 / ((WT * T[var] + WH * H[var]) * (1 / WT * T[var] + WH * H[v
				}
				else if (method == 2)
				{
					float q[var] = B[var] * (1-((-1/LN(M))*B[var]*(LN(B[var]))) + (1 - H[var])*(1-((-1/LN(M))*(1-H[var])*(LN(1-H[var])))+(1-T[var])*(1-((-1/LN(M))*(1-T[var])*(LN(1-T[var])))));
				}
			}

			float bestValue = 0;
			int bestRow = 0;
			for (int l = 0; l < rows; ++l)
			{
				if (q[l] > bestValue)
				{
					bestRow = l;
					bestValue = q[l];
				}
			}
			return bestRow;
		}

		/**
		 * \brief the main topsis strategy for send interest
		 * @param fibEntry
		 * @param interest
		 * @param inface
		 * @return the best face for sending Interest
		 */
		Face *
		SoltaniStrategy::getBestFaceForForwarding(const fib::Entry &fibEntry, const Interest &interest, const Face &inFace)
		{

			const fib::NextHopList &nexthops = fibEntry.getNextHops();
			FaceStats *stats = new FaceStats[10];
			int i = 0;

			for (const fib::NextHop &hop : nexthops)
			{
				Face &hopFace = hop.getFace();
				if ((hopFace.getId() == inFace.getId() && hopFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
						wouldViolateScope(inFace, interest, hopFace))
				{
					continue;
				}
				float hopCount++;
				asf::FaceInfo *info = m_measurements.getFaceInfo(fibEntry, interest, hopFace.getId());
				if (info == nullptr)
				{
					// set Facestate structure for topsis Rank if info is null
					stats[i] = {&hopFace, hopCount, BW, asf::RttStats::RTT_NO_MEASUREMENT, RoutingMetric};
				}
				else
				{
					// set Facestate structure for topsis Rank
					stats[i] = {&hopFace, hopCount, BW, info->getRtt(), RoutingMetric};
				}
				i++;
			}

			// return best row of states array
			int bestRow = result(stats, nexthops.size());
			// set output face for send interest
			Face *faceToUse = stats[bestRow].face;

			return faceToUse;
		}

		/**
		 * \brief send the NoRoute packet to faces
		 * @param inface
		 * @param interest
		 * @param pitEntity
		 */
		void
		SoltaniStrategy::sendNoRouteNack(const Face &inFace, const Interest &interest,
																		 const shared_ptr<pit::Entry> &pitEntry)
		{
			NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " noNextHop");
			lp::NackHeader nackHeader;
			nackHeader.setReason(lp::NackReason::NO_ROUTE);
			this->sendNack(pitEntry, inFace, nackHeader);
		}

	} /* namespace fw */
} /* namespace nfd */
