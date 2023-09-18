

#include "cs-policy-soltani.hpp"
#include "cs.hpp"
#include "core/logger.hpp"
#include "math.h"

NFD_LOG_INIT("SoltaniPolicy");

namespace nfd {
namespace cs {
namespace soltani {

const std::string SoltaniPolicy::POLICY_NAME = "soltani";
NFD_REGISTER_CS_POLICY(SoltaniPolicy);

SoltaniPolicy::SoltaniPolicy()
  : Policy(POLICY_NAME)
{
}

SoltaniPolicy::~SoltaniPolicy()
{
  for (auto entryInfoMapPair : m_entryInfoMap) {
    delete entryInfoMapPair.second;
  }
}

//Not Yet
void
SoltaniPolicy::doBeforeErase(iterator i)
{
  NFD_LOG_INFO("Before Erase Function" );

  this->detachQueue(i);
}

void
SoltaniPolicy::doAfterRefresh(iterator i)
{
  NFD_LOG_INFO("After Refresh Function" );

  EntryInfo* entryInfo = m_entryInfoMap[i];

  if(entryInfo->queueType == linkedlist){
    NFD_LOG_INFO("Linked List Location");
    this->updateDI(i);
    this->replaceCs(true);
  }

  else if (entryInfo->queueType == heaplist){
    NFD_LOG_INFO("Already in HeapList");
    this->updateDI(i);
    this->replaceCs(false);
  }
  
}


void
SoltaniPolicy::doAfterInsert(iterator i)
{
  NFD_LOG_INFO("After Insert Function" );

  this->attachQueue(i);
  this->evictEntries();
}

void
SoltaniPolicy::doBeforeUse(iterator i)
{
  NFD_LOG_INFO("Before Use Function" );

  EntryInfo* entryInfo = m_entryInfoMap[i];

  if(entryInfo->queueType == linkedlist){
    NFD_LOG_INFO("Linked List Location");
    this->updateDI(i);
    this->replaceCs(true);
  }

  else if (entryInfo->queueType == heaplist){
    NFD_LOG_INFO("Already in HeapList");
    this->updateDI(i);
    this->replaceCs(false);
  }

}

void
SoltaniPolicy::evictEntries()
{
  BOOST_ASSERT(this->getCs() != nullptr);

  while (this->getCs()->size() > this->getLimit()) {
    this->evictOne();
  }
}

void
SoltaniPolicy::evictOne()
{
  BOOST_ASSERT(!m_queues[heaplist].empty()||
               !m_queues[linkedlist].empty());

   iterator i;
    if(!m_queues[linkedlist].empty()){
       i = m_queues[linkedlist].front();
     }

   this->detachQueue(i);
   this->emitSignal(beforeEvict, i);
}

void
SoltaniPolicy::attachQueue(iterator i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) == m_entryInfoMap.end());

  ndn::time::steady_clock::TimePoint init_now = ::ndn::time::steady_clock::now();
  ndn::time::milliseconds init_milliseconds = ::ndn::time::duration_cast<::ndn::time::milliseconds>(init_now.time_since_epoch());
  double init_time_ms = init_milliseconds.count();
  double init_currentTime = init_time_ms/1000;

  EntryInfo* entryInfo = new EntryInfo();
    entryInfo->Di = 1.0;
    entryInfo->lastReferencedTime=init_currentTime;

    if(this->getCs()->size() == this->getLimit()+1){
      NFD_LOG_INFO("** New Interest **");
      this->replaceCs(true);
      entryInfo->queueType = heaplist;
    }
    else if (this->getCs()->size() > 7) {
      entryInfo->queueType = linkedlist;
      NFD_LOG_INFO("Type : LinkedList");
    }
    else{
      entryInfo->queueType = heaplist;
      NFD_LOG_INFO("Type : HeapList");
    }

  Queue& queue = m_queues[entryInfo->queueType];
  entryInfo->queueIt = queue.insert(queue.end(), i);
  m_entryInfoMap[i] = entryInfo;

  for(auto it = m_queues[heaplist].begin(); it != m_queues[heaplist].end(); ++it)
     {
      if(m_entryInfoMap[*it]->queueType == heaplist){
      NFD_LOG_INFO("EntryInfo HeapList " << m_entryInfoMap[*it]);
     }
  }
 
  for(auto it = m_queues[linkedlist].begin(); it != m_queues[linkedlist].end(); ++it)
  {
    if(m_entryInfoMap[*it]->queueType == linkedlist){
    NFD_LOG_INFO("EntryInfo LinkedList " << m_entryInfoMap[*it]);
    }
  }          


  NFD_LOG_DEBUG("Size: "<< this->getCs()->size() <<"Di : "<< m_entryInfoMap[i]->Di << ", entryInfo: "<< m_entryInfoMap[i]);

}

void
SoltaniPolicy::detachQueue(iterator i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  EntryInfo* entryInfo = m_entryInfoMap[i];

  m_queues[entryInfo->queueType].erase(entryInfo->queueIt);

  NFD_LOG_DEBUG("Erased " << m_entryInfoMap[i]);
  m_entryInfoMap.erase(i);
  delete entryInfo;

}

void
SoltaniPolicy::updateDI(iterator i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  EntryInfo* entryInfo = m_entryInfoMap[i];

  double initTime = entryInfo->lastReferencedTime;
  double favorite = entryInfo->lastReferenceFavorite;
  double lastDi = entryInfo->di;
  double A = -1/log(cacheCounter);
  double tnormi, Tt, lnormi, Lt;
  set(tnormi, Tt, lnormi, Lt);
  ndn::time::steady_clock::TimePoint now = ::ndn::time::steady_clock::now();
  ndn::time::milliseconds milliseconds = ::ndn::time::duration_cast<::ndn::time::milliseconds>(now.time_since_epoch());
  double time_ms = milliseconds.count();
  double currentTime = time_ms/1000;

  double newReferencedTime = currentTime - initTime;

  entryInfo->di = tnorm*(1-A*Tt)+lnorm*(1-A*Lt);
  entryInfo->lastReferencedTime = currentTime;

  NFD_LOG_DEBUG("Update Di : "<< m_entryInfoMap[i]->Di << " New Referenced: " <<m_entryInfoMap[i]->lastReferencedTime);
  NFD_LOG_DEBUG("currentTime: " <<currentTime << " Di: " << lastDi << " lastReferencedTime: " <<initTime);

}

void
SoltaniPolicy::replaceCs(bool status)
{
  BOOST_ASSERT(!m_queues[heaplist].empty());

  iterator lowestDiPointer;
  double tempDi = 9999.0;
  double getDi;
  int list = 1;

  for(auto it = m_queues[heaplist].begin(); it != m_queues[heaplist].end(); ++it)
  {
      if(m_entryInfoMap[*it]->queueType == heaplist){
        NFD_LOG_INFO( list << ". HeapList Di Value: " << m_entryInfoMap[*it]->di);

        getDi = m_entryInfoMap[*it]->Di;
          if ( tempDi > getDi)
          {
            tempDi = getDi;
            lowestDiPointer = *it;
          }
      }
      list ++;
  }
  NFD_LOG_INFO("-- Lowest Di: " << tempDi);
  NFD_LOG_INFO("-- delete data with lowest DI in CS: " << m_entryInfoMap[lowestDiPointer]);

}


void
SoltaniPolicy::replaceCs(iterator i)
{
    EntryInfo* entryInfo = m_entryInfoMap[i];
    m_queues[linkedlist].erase(entryInfo->queueIt);

    entryInfo->queueType = heaplist;
    Queue&queue = m_queues[heaplist];
    entryInfo->queueIt = queue.insert(queue.end(),i);
    m_entryInfoMap[i] = entryInfo;

    NFD_LOG_INFO("Move To HeapList (0) ; Di = " <<m_entryInfoMap[i]->Di);

}

} // namespace lrfu
} // namespace cs
} // namespace nfd