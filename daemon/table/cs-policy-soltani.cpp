

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
    this->updateCRF(i);
    this->restoreHeapStructure(true);
    this->moveToHeapList(i);
  }

  else if (entryInfo->queueType == heaplist){
    NFD_LOG_INFO("Already in HeapList");
    this->updateCRF(i);
    this->restoreHeapStructure(false);
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
    this->updateCRF(i);
    this->restoreHeapStructure(true);
    this->moveToHeapList(i);
  }

  else if (entryInfo->queueType == heaplist){
    NFD_LOG_INFO("Already in HeapList");
    this->updateCRF(i);
    this->restoreHeapStructure(false);
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
    entryInfo->crf = 1.0;
    entryInfo->lastReferencedTime=init_currentTime;

    if(this->getCs()->size() == this->getLimit()+1){
      NFD_LOG_INFO("** New Interest **");
      this->restoreHeapStructure(true);
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


  NFD_LOG_DEBUG("Size: "<< this->getCs()->size() <<"CRF : "<< m_entryInfoMap[i]->crf << ", entryInfo: "<< m_entryInfoMap[i]);

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
SoltaniPolicy::updateCRF(iterator i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  EntryInfo* entryInfo = m_entryInfoMap[i];

  double initTime = entryInfo->lastReferencedTime;
  double lastCrf = entryInfo->crf;

  ndn::time::steady_clock::TimePoint now = ::ndn::time::steady_clock::now();
  ndn::time::milliseconds milliseconds = ::ndn::time::duration_cast<::ndn::time::milliseconds>(now.time_since_epoch());
  double time_ms = milliseconds.count();
  double currentTime = time_ms/1000;

  double newReferencedTime = currentTime - initTime;

  entryInfo->crf = 1.0 + pow((1.0/2.0),(0.1 * newReferencedTime)) * lastCrf;
  entryInfo->lastReferencedTime = currentTime;

  NFD_LOG_DEBUG("Update CRF : "<< m_entryInfoMap[i]->crf << " New Referenced: " <<m_entryInfoMap[i]->lastReferencedTime);
  NFD_LOG_DEBUG("currentTime: " <<currentTime << " CRF: " << lastCrf << " lastReferencedTime: " <<initTime);

}

void
SoltaniPolicy::restoreHeapStructure(bool status)
{
  BOOST_ASSERT(!m_queues[heaplist].empty());

  iterator lowestCrfPointer;
  double tempCrf = 9999.0;
  double getCrf;
  int list = 1;

  for(auto it = m_queues[heaplist].begin(); it != m_queues[heaplist].end(); ++it)
  {
      if(m_entryInfoMap[*it]->queueType == heaplist){
        NFD_LOG_INFO( list << ". HeapList CRF Value: " << m_entryInfoMap[*it]->crf);

        getCrf = m_entryInfoMap[*it]->crf;
          if ( tempCrf > getCrf)
          {
            tempCrf = getCrf;
            lowestCrfPointer = *it;
          }
          else if (tempCrf == getCrf){
          	//do nothing
          }
      }
      list ++;
  }
  NFD_LOG_INFO("-- Lowest CRF: " << tempCrf);
  NFD_LOG_INFO("-- Lowest Iterator: " << m_entryInfoMap[lowestCrfPointer]);

  if (status == true){
    this->moveToLinkedList(lowestCrfPointer);
  }

}

void
SoltaniPolicy::moveToLinkedList(iterator i)
{
    EntryInfo* entryInfo = m_entryInfoMap[i];
    m_queues[heaplist].erase(entryInfo->queueIt);

    entryInfo->queueType = linkedlist;
    Queue&queue = m_queues[linkedlist];
    entryInfo->queueIt = queue.insert(queue.end(),i);
    m_entryInfoMap[i] = entryInfo;

    if(m_entryInfoMap[i]->queueType==linkedlist){
      NFD_LOG_INFO("Move to LinkedList (1) ; Crf = " << m_entryInfoMap[i]->crf);
  }
}

void
SoltaniPolicy::moveToHeapList(iterator i)
{
    EntryInfo* entryInfo = m_entryInfoMap[i];
    m_queues[linkedlist].erase(entryInfo->queueIt);

    entryInfo->queueType = heaplist;
    Queue&queue = m_queues[heaplist];
    entryInfo->queueIt = queue.insert(queue.end(),i);
    m_entryInfoMap[i] = entryInfo;

    NFD_LOG_INFO("Move To HeapList (0) ; CRF = " <<m_entryInfoMap[i]->crf);

}

} // namespace lrfu
} // namespace cs
} // namespace nfd