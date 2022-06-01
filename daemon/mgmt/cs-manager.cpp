/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cs-manager.hpp"
#include "fw/forwarder-counters.hpp"
#include "table/cs.hpp"

#include <ndn-cxx/mgmt/nfd/cs-info.hpp>

namespace nfd {

constexpr size_t CsManager::ERASE_LIMIT;

CsManager::CsManager(Cs& cs, const ForwarderCounters& fwCounters,
                     Dispatcher& dispatcher, CommandAuthenticator& authenticator)
  : ManagerBase("cs", dispatcher, authenticator)
  , m_cs(cs)
  , m_fwCounters(fwCounters)
{
  registerCommandHandler<ndn::nfd::CsConfigCommand>("config",
    std::bind(&CsManager::changeConfig, this, _4, _5));
  registerCommandHandler<ndn::nfd::CsEraseCommand>("erase",
    std::bind(&CsManager::erase, this, _4, _5));

  registerStatusDatasetHandler("info", std::bind(&CsManager::serveInfo, this, _1, _2, _3));
}

void
CsManager::changeConfig(const ControlParameters& parameters,
                        const ndn::mgmt::CommandContinuation& done)
{
  using ndn::nfd::CsFlagBit;

  if (parameters.hasCapacity()) {
    m_cs.setLimit(parameters.getCapacity());
  }

  if (parameters.hasFlagBit(CsFlagBit::BIT_CS_ENABLE_ADMIT)) {
    m_cs.enableAdmit(parameters.getFlagBit(CsFlagBit::BIT_CS_ENABLE_ADMIT));
  }

  if (parameters.hasFlagBit(CsFlagBit::BIT_CS_ENABLE_SERVE)) {
    m_cs.enableServe(parameters.getFlagBit(CsFlagBit::BIT_CS_ENABLE_SERVE));
  }

  ControlParameters body;
  body.setCapacity(m_cs.getLimit());
  body.setFlagBit(CsFlagBit::BIT_CS_ENABLE_ADMIT, m_cs.shouldAdmit(), false);
  body.setFlagBit(CsFlagBit::BIT_CS_ENABLE_SERVE, m_cs.shouldServe(), false);
  done(ControlResponse(200, "OK").setBody(body.wireEncode()));
}

void
CsManager::erase(const ControlParameters& parameters,
                 const ndn::mgmt::CommandContinuation& done)
{
  size_t count = parameters.hasCount() ?
                 parameters.getCount() :
                 std::numeric_limits<size_t>::max();
  m_cs.erase(parameters.getName(), std::min(count, ERASE_LIMIT),
    [=] (size_t nErased) {
      ControlParameters body;
      body.setName(parameters.getName());
      body.setCount(nErased);
      if (nErased == ERASE_LIMIT && count > ERASE_LIMIT) {
        m_cs.find(Interest(parameters.getName()).setCanBePrefix(true),
          [=] (const Interest&, const Data&) mutable {
            body.setCapacity(ERASE_LIMIT);
            done(ControlResponse(200, "OK").setBody(body.wireEncode()));
          },
          [=] (const Interest&) {
            done(ControlResponse(200, "OK").setBody(body.wireEncode()));
          });
      }
      else {
        done(ControlResponse(200, "OK").setBody(body.wireEncode()));
      }
    });
}

void
CsManager::serveInfo(const Name& topPrefix, const Interest& interest,
                     ndn::mgmt::StatusDatasetContext& context) const
{
  ndn::nfd::CsInfo info;
  info.setCapacity(m_cs.getLimit());
  info.setEnableAdmit(m_cs.shouldAdmit());
  info.setEnableServe(m_cs.shouldServe());
  info.setNEntries(m_cs.size());
  info.setNHits(m_fwCounters.nCsHits);
  info.setNMisses(m_fwCounters.nCsMisses);

  context.append(info.wireEncode());
  context.end();
}
void
 CsManager::createClustring(){
  double k[clustersNum];
   int  clustersNodeNum[],  clustersNum,  clustersHead[],  clustersNode[][];
  double c[clustersNum][clustersNodeNum], p[clustersNum][clustersNodeNum], ram[clustersNum][clustersNodeNum];
  double fc[clustersNum][clustersNodeNum];
  double ct[clustersNum][clustersNodeNum], pt[clustersNum][clustersNodeNum], ramt[clustersNum][clustersNodeNum];
  double cnorm[clustersNum][clustersNodeNum], pnorm[clustersNum][clustersNodeNum], ramnorm[clustersNum][clustersNodeNum];
  //read file and create clustring
  ifstream MyReadFile("router.csv");
   ifstream MyReadFile2("clusteringConfig.txt");
  // Use a while loop together with the getline() function to read the file line by line
  while (MyReadFile) {
     c = MyReadFile.c;
     p = MyReadFile.p;
     ram = MyReadFile.ram;
  }
while (MyReadFile2) {
    clustersNum = MyReadFile2.clustersNum;
    clustersHead = MyReadFile2.clustersHead;
    clustersNodeNum = MyReadFile2.clustersNodeNum;
    clustersNode = MyReadFile2.clustersNode;
  }
  // Close the file
  MyReadFile.close();
  MyReadFile2.close();

   for(int i=0; i<clustersNum; i++){
    for(int j=0; j <clustersNodeNum[i]; j++){
      cnorm[i][j] += c[i][j];
      pnorm[i][j] +=p[i][j];
      ramnorm[i][j] += ram[i][j];
    }
   }
  this.sumation(ct,pt,ramt);
  // SET F FOR rounter in any cluster for once
  for(int i=0; i<clustersNum; i++){
    k[i] = (-1)/log(clustersNodeNum);
    for(int j=0; j <clustersNodeNum[i]; j++){
      cnorm[i][j] = c[i][j]/cnorm[i][j]; 
      pnorm[i][j] = c[i][j]/pnorm[i][j]; 
      ramnorm[i][j] = ram[i][j]/ramnorm[i][j]; 
      fc[i][j] = (cnorm[i][j]*(1-k[i]*ct[i][j])+pnorm[i][j]*(1-k[i]*pt[i][j])+ramnorm[i][j]*(1-(k[i]*ramt[i][j])))/((1-k[i]*ct[i][j])+(1-k[i]*pt[i][j])+(1-k[i]*ramt[i][j]));
    }
  }
  
}

int
CsManager::placement(const Interest& interest, const FaceEndpoint& ingress){
  
  double p[routerNum];
  RetxSuppressionResult suppression = m_retxSuppression.decidePerPitEntry(*pitEntry);
  if (suppression == RetxSuppressionResult::SUPPRESS) {
    NFD_LOG_DEBUG(interest << " from=" << ingress << " suppressed");
    return;
  }
it = std::find_if(nodesArounds.begin(), nodesArounds.end(),
                    [&, now = time::steady_clock::now()] (const auto& nexthop) {
                      return isInCluster(ingress.face, interest, nexthop, pitEntry, true, now);
                    });
// if data in fist node from client
  if (suppression == RetxSuppressionResult::NEW) {
    // for node arrounds
    if (it == nodesArounds.end()) {
      NFD_LOG_DEBUG(interest << " from=" << ingress << " noNextHop");

      lp::NackHeader nackHeader;
      nackHeader.setReason(lp::NackReason::NO_ROUTE);
      this->sendNack(nackHeader, ingress.face, pitEntry);
      this->rejectPendingInterest(pitEntry);
      return;
    }
    Face& outFace = it->getFace();
    NFD_LOG_DEBUG(interest << " from=" << ingress << " data is in fist Node" << outFace.getId());
    this->sendInterest(interest, outFace, pitEntry);
    this->updatePi();
    return;
  }
// if in cluster but not first node
  if (DataInCluster()) {
    // for node arrounds
    if (it == nodesArounds.end()) {
      NFD_LOG_DEBUG(interest << " from=" << ingress << " noNextHop");

      lp::NackHeader nackHeader;
      nackHeader.setReason(lp::NackReason::NO_ROUTE);
      this->sendNack(nackHeader, ingress.face, pitEntry);
      this->rejectPendingInterest(pitEntry);
      return;
    }
    Face& outFace = it->getFace();
    NFD_LOG_DEBUG(interest << " from=" << ingress << " data is in fist Node" << outFace.getId());
    this->sendInterest(interest, outFace, pitEntry);
    this->updatePi();
    this->sortPi(status);
    return;
  }
 // if cs not in cluster
  if (!DataInCluster()) {
    Face& outFace = it->getFace();
    this->sendInterest(interest, outFace, pitEntry);
    NFD_LOG_DEBUG(interest << " from=" << ingress << " data is not in my cluster" << outFace.getId());
    sendDataFromAnotherCluster();
    this->updatePi();
    this->sortPi(status);
    return;
  }

  
}
void
CsManager::updatePi()
{
  BOOST_ASSERT(clustringMap.find(i) != clustringMap.end());

  EntryInfo* entryInfo = clustringMap[i];

  double lastPi = entryInfo->pi;
  double A = -1/log(cacheCounter);
  double tnormi, Tt, lnormi, Lt;
  set(nnorm, nt, ht, ft, hnorm, lnormi, Lt,k[i]);

  entryInfo->p[i] = nnorm * (1-k[i]*nt) + hnorm * (1-k[i]*ht) _cnorm*(1-k[i]*ct)+fnorm*(1-k[i]*ft)

  NFD_LOG_DEBUG("Update Pi : "<< clustringMap[i]->pi << " New Referenced: " <<clustringMap[i]->lastReferencedTime);

}

void
SoltaniPolicy::sortPi(bool status)
{
  BOOST_ASSERT(!m_queues[heaplist].empty());

  iterator lowestPiPointer;
  double tempPi = 0;
  double getPi;
  int list = 1;

  for(auto it = m_queues[heaplist].begin(); it != m_queues[heaplist].end(); ++it)
  {
      if(m_entryInfoMap[*it]->queueType == heaplist){
        NFD_LOG_INFO( list << ". HeapList Pi Value: " << m_entryInfoMap[*it]->Pi);

        getPi = m_entryInfoMap[*it]->Pi;
          if ( tempPi < getPi)
          {
            tempPi = getPi;
            lowestPiPointer = *it;
          }

      }
      list ++;
  }
  NFD_LOG_INFO("-- Lowest Di: " << tempDi);
  NFD_LOG_INFO("-- Lowest Iterator: " << m_entryInfoMap[lowestDiPointer]);

}

} // namespace nfd
