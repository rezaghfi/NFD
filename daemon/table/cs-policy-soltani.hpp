

#ifndef NFD_DAEMON_TABLE_CS_POLICY_SOLTANI_HPP
#define NFD_DAEMON_TABLE_CS_POLICY_SOLTANI_HPP

#include "cs-policy.hpp"
#include "core/scheduler.hpp"


namespace nfd {
namespace cs {
namespace soltani {

typedef std::list<iterator> Queue;
typedef Queue::iterator QueueIt;

enum QueueType{
  heaplist,
  linkedlist,
  lrfu
};

struct EntryInfo
{
  QueueType queueType;
  double Di;
  double lastReferencedTime;
  QueueIt queueIt;
  scheduler::EventId moveListEventId;
};

struct EntryItComparator
{

  bool
  operator()(const iterator& a, const iterator& b) const
  {
    return *a < *b;
  }

};

typedef std::map<iterator, EntryInfo*, EntryItComparator> EntryInfoMapLrfu;

/** \brief Priority LRFU cs replacement policy
 *
 * The entries that get removed first are unsolicited Data packets,
 * which are the Data packets that got cached opportunistically without preceding
 * forwarding of the corresponding Interest packet.
 * Next, the Data packets with expired freshness are removed.
 * Last, the Data packets are removed from the Content Store on a pure LRFU basis.
 */
class SoltaniPolicy : public Policy
{
public:
  SoltaniPolicy();

  virtual
  ~SoltaniPolicy();

public:
  static const std::string POLICY_NAME;

private:
  virtual void
  doAfterInsert(iterator i) override;

  virtual void
  doAfterRefresh(iterator i) override;

  virtual void
  doBeforeErase(iterator i) override;

  virtual void
  doBeforeUse(iterator i) override;

  virtual void
  evictEntries() override;

private:
  /** \brief evicts one entry
   *  \pre CS is not empty
   */
  void
  evictOne();

  /** \brief attaches the entry to an appropriate queue
   *  \pre the entry is not in any queue
   */
  void
  attachQueue(iterator i);

  /** \brief detaches the entry from its current queue
   *  \post the entry is not in any queue
   */
  void
  detachQueue(iterator i);

  /** \brief moves an entry from LRFU queue to STALE queue
   */
  void
  updateDi(iterator i);

  void
  restoreHeapStructure(bool status);

  void
  moveToLinkedList(iterator i);

  void
  moveToHeapList(iterator i);


private:
  // Queue m_queues[QUEUE_MAX];
  Queue m_queues[lrfu];
  EntryInfoMapLrfu m_entryInfoMap;
};

} // namespace priority_lrfu

using lrfu::SoltaniPolicy;

} // namespace cs
} // namespace nfd

#endif // NFD_DAEMON_TABLE_CS_POLICY_SOLTANI_HPP