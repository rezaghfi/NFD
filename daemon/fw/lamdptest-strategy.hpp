

#ifndef NDNSIM_NFD_DAEMON_FW_equal_STRATEGY_HPP_
#define NDNSIM_NFD_DAEMON_FW_equal_STRATEGY_HPP_

#include "strategy.hpp"
#include "asf-measurements.hpp"
#include "fw/retx-suppression-exponential.hpp"
// for queue size : calculating load


namespace nfd {
namespace fw {

	typedef time::duration<double, boost::micro> myRtt;

	/**
	 * this is Structure of strategy prameters
	 */

	struct FaceStats{
		Face* outFace;
		Face* inFace;
		int reward;
		int availBW;
		int unsatisfiedInterest;
		int rtt;
		float prob;
		float wprob;
	};


	class lamdptestStrategy: public Strategy {
		public:
		explicit
		lamdptestStrategy(Forwarder& forwarder, const Name& name = getStrategyName());

		static const Name& getStrategyName();

		void afterReceiveInterest(const Face& inFace, const Interest& interest,
							const shared_ptr<pit::Entry>& pitEntry) override;


		void afterReceiveData (const shared_ptr< pit::Entry > &pitEntry,
							const Face &inFace, const Data &data) override;

		private:

		Face* getBestFaceForForwarding(const fib::Entry& fibEntry, const Interest& interest, const Face& inFace);

		void sendNoRouteNack(const Face& inFace, const Interest& interest,
									const shared_ptr<pit::Entry>& pitEntry);

		int selectedFace;
		RetxSuppressionExponential m_retxSuppression;
		asf::AsfMeasurements m_measurements;
		static const time::milliseconds RETX_SUPPRESSION_INITIAL;
		static const time::milliseconds RETX_SUPPRESSION_MAX;

	}; /* equal class*/
	} /* namespace fw */
	} /* namespace nfd */

	#endif /* NDNSIM_NFD_DAEMON_FW_equal_STRATEGY_HPP_ */
