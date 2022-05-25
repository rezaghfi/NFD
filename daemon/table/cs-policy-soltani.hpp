

#ifndef NFD_DAEMON_TABLE_CS_POLICY_SOLTANI_HPP
#define NFD_DAEMON_TABLE_CS_POLICY_SOLTANI_HPP

#include "cs-policy.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>

namespace nfd {
namespace cs {
namespace soltani {

using Queue = boost::multi_index_container<
                Policy::EntryRef,
                boost::multi_index::indexed_by<
                  boost::multi_index::sequenced<>,
                  boost::multi_index::ordered_unique<boost::multi_index::identity<Policy::EntryRef>>
                >
              >;

/** \brief Least-Recently-Used (Soltani) replacement policy
 */
class SoltaniPolicy final : public Policy
{
public:
  SoltaniPolicy();

public:
  static const std::string POLICY_NAME;

private:
  void
  doAfterInsert(EntryRef i) final;

  void
  doAfterRefresh(EntryRef i) final;

  void
  doBeforeErase(EntryRef i) final;

  void
  doBeforeUse(EntryRef i) final;

  void
  evictEntries() final;

private:
  /** \brief moves an entry to the end of queue
   */
  void
  insertToQueue(EntryRef i, bool isNewEntry);

private:
  Queue m_queue;
};

} // namespace soltani

using soltani::SoltaniPolicy;

} // namespace cs
} // namespace nfd

#endif // NFD_DAEMON_TABLE_CS_POLICY_SOLTANI_HPP
