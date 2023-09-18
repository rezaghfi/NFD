

#define RoutingMetric 1

#include "equal-strategy.hpp"
#include "algorithm.hpp"
#include "core/logger.hpp"


namespace nfd {
namespace fw {

NFD_LOG_INIT(equalStrategy);
NFD_REGISTER_STRATEGY(equalStrategy);

const time::milliseconds equalStrategy::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds equalStrategy::RETX_SUPPRESSION_MAX(250);

ns3::QueueSize faceInterests;
/**
 * \brief constructor of equal-strategy
 * @param forwarder instance for run strategy
 * @param name of strategy
 */
equalStrategy::equalStrategy(Forwarder& forwarder, const Name& name)
:Strategy(forwarder)
, m_measurements(getMeasurements())
, m_retxSuppression(RETX_SUPPRESSION_INITIAL,
        RetxSuppressionExponential::DEFAULT_MULTIPLIER,
        RETX_SUPPRESSION_MAX)
{

	ParsedInstanceName parsed = parseInstanceName(name);
	 if (!parsed.parameters.empty()) {
	    BOOST_THROW_EXCEPTION(std::invalid_argument("equal strategy does not accept parameters"));
	 }
	 if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
	    BOOST_THROW_EXCEPTION(std::invalid_argument(
	      "equal strategy does not support version " + to_string(*parsed.version)));
	 }
	 this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

/**
 * \brief create name for use in scenario
 * @return strategyName
 */
const Name&
equalStrategy::getStrategyName(){
	static Name strategyName("/localhost/nfd/strategy/equal/%FD%01");
	return strategyName;
}

/**
 * \brief trigger after receiving Interest
 * @param inface
 * @param interest
 * @param pitEntry
 */
void
equalStrategy::afterReceiveInterest(const Face& inFace, const Interest& interest,
                       const shared_ptr<pit::Entry>& pitEntry){
  /* todo interest can three kinds:
   * NEW, FORWARD, SUppress
	*/
  RetxSuppressionResult suppressResult = m_retxSuppression.decidePerPitEntry(*pitEntry);

  switch (suppressResult) {
  case RetxSuppressionResult::NEW:
  case RetxSuppressionResult::FORWARD:
	break;
  case RetxSuppressionResult::SUPPRESS:
	NFD_LOG_DEBUG("SUPPRESS Interest:" << interest << " from=" << inFace.getId() << "DROPED");
	return;
  }

  // find fibentry with pitEntry Pointer
  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  // if vector size is zero, send NO_ROUTE NACK and return
  if (nexthops.size() == 0) {
	sendNoRouteNack(inFace, interest, pitEntry);
	this->rejectPendingInterest(pitEntry);
	return;
  }

  // call function to set output face to choose best face
  Face* faceToUse = getBestFaceForForwarding(fibEntry, interest, inFace);

  if (faceToUse == nullptr) {
		sendNoRouteNack(inFace, interest, pitEntry);
		this->rejectPendingInterest(pitEntry);
		return;
	  }

  this->sendInterest(pitEntry, *faceToUse, interest);
  NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                         << " pitEntry-to=" << faceToUse->getId());

}

/**
 * \brief trigger after receiving Data
 * @param pitEntry
 * @param inface
 * @param data
 */
void
equalStrategy::afterReceiveData (const shared_ptr< pit::Entry > &pitEntry,
					const Face &inFace, const Data &data){
	
	int select = this->selectedFace;
    int lambda = 0.5;
	int i = 0;
	 // find fibentry with pitEntry Pointer
  	const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
	const fib::NextHopList& nexthops = fibEntry.getNextHops();
	FaceStats* stats = new FaceStats[10]; 
	stats[select].unsatisfiedInterest = stats[select].unsatisfiedInterest - 1;
		// prob[select] + x //nokay
	stats[select].prob = 1 - lambda*(Math.sum(prob) - stats[select].prob);

	for (const fib::NextHop& hop : nexthops) {
		if(i != select){
			// 			prob[i] - x/(num - 1)
			stats[i].prob = lambda*stats[i].prob;
		}
		i++;
	}
	// send data to inFace Interest
	Face* faceToUse = stats[i].inFace;
	//sendData(pitEntry, data, faceToUse);

}

/**
 * \brief this function find output interface for sendind interest with Learning automata and MDP
 * @param FaceStats
 * @param rows
 * @return actionNum
 */
int equalresult(FaceStats* stats, int rows) {
	int actionNum = 0;
      for (int i = 0; i < 0; i++){
        if (stats[i].wprob == 0)
        {
          actionNum = i;
          stats[i].wprob = 1;
          break;
        }
      }
      return actionNum;
}

/**
 * \brief the main function for send interest
 * @param fibEntry
 * @param interest
 * @param inface
 * @return the best face for sending Interest
 */
Face*
equalStrategy::getBestFaceForForwarding(const fib::Entry& fibEntry, const Interest& interest, const Face& inFace){


	const fib::NextHopList& nexthops = fibEntry.getNextHops();
	FaceStats* stats = new FaceStats[10]; 

	int i = 0;
	int bw = 0;
	int sumBW = 0;
	int sumUnsatisfied = 0;
	int sumRTT = 0;

	for (const fib::NextHop& hop : nexthops) {
		
		Face& hopFace = hop.getFace();
		// set band width of interface
		bw = hopFace.getQueue.Size();
		// set rtt for delay
		asf::FaceInfo* info = m_measurements.getFaceInfo(fibEntry, interest, hopFace.getId());
		// set Facestate structure
		stats[i].outFace = &hopFace;
		stats[i].availBW = bw;
		stats[bestRow].unsatisfiedInterest = stats[bestRow].unsatisfiedInterest + 1;
		// set rtt info is nullptr or no
		if(info == nullptr){
			stats[i].rtt = 0;
		}else{
			stats[i].rtt = info->getRtt();
		}
		sumBW += bw;
		sumUnsatisfied += stats[i].unsatisfiedInterest;
		sumRTT += stats[i].rtt;
		i++; 
	}
	
	int bwn = 0;
	int unsatisfiedn = 0;
	int rttn = 0;
	int sumProReward = 0;

	for (const fib::NextHop& hop : nexthops) {
		Face& hopFace = hop.getFace();
		// normalization
		bwn = stats[i].availBW /(sumBW+1);
		unsatisfiedn = stats[i].unsatisfiedInterest /(sumUnsatisfied+1);
		rttn = stats[i].rtt/ (sumRTT + 1);
		// calculate reward
		stats[i].reward = bwn/(rttn + unsatisfiedn + 1);

		// calculate sumProReward from  simple probality and reward
		sumProReward = sumProReward + stats[i].prob * stats[i].reward;
		i++;
	}

	for (const fib::NextHop& hop : nexthops) {
		// calculate wpro for each output interface
		stats[i].wprob = 0;
		i++;
	}
	// return best wpro of states array
	int bestRow = equalresult(stats, nexthops.size());
	// set output face for send interest
	Face* faceToUse = stats[bestRow].outFace;
	this->selectedFace = bestRow;
	// for save inFace in FaceStates //nokay
	stats[bestRow].inFace = &inFace;
	return faceToUse;
}


/**
 * \brief send the NoRoute packet to faces
 * @param inface
 * @param interest
 * @param pitEntity
 */
void
equalStrategy::sendNoRouteNack(const Face& inFace, const Interest& interest,
                             const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " noNextHop");
  lp::NackHeader nackHeader;
  nackHeader.setReason(lp::NackReason::NO_ROUTE);
  this->sendNack(pitEntry, inFace, nackHeader);
}

} /* namespace fw */
} /* namespace nfd */

