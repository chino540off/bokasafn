/**
 *  @file policies.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_CACHE_POLICIES_HH_
# define BOKASAFN_CACHE_POLICIES_HH_

# include <list>
# include <map>
# include <unordered_map>
# include <unordered_set>

namespace bokasafn {
namespace cache {
namespace policies {

/**
 * @brief 
 */
template <typename K>
class policy
{
  public:
    virtual ~policy() = default;

  public:
    virtual void insert(K const &) = 0;
    virtual void touch(K const &) = 0;
    virtual void erase(K const &) = 0;
    virtual K const & candidate() const = 0;
};

template <typename K>
class nopolicy:
  public policy<K>
{
  public:
    nopolicy() = default;
    ~nopolicy() = default;

  public:
    virtual void insert(K const & k)
    {
      key_store_.insert(k);
    }

    virtual void touch(K const &)
    {
      // nothing to do
    }

    virtual void erase(K const & k)
    {
      key_store_.erase(k);
    }

    virtual K const & candidate() const
    {
      return *key_store_.cbegin();
    }

  private:
    std::set<K> key_store_;
};

template <typename K>
class lru:
  public policy<K>
{
  public:
    lru() = default;
    ~lru() = default;

  public:
    virtual void insert(K const & k)
    {
      key_store_.emplace_front(k);
      key_finder_[k] = key_store_.begin();
    }

    virtual void touch(K const & k)
    {
      key_store_.splice(key_store_.begin(),
                        key_store_,
                        key_finder_[k]);
    }

    virtual void erase(K const &)
    {
      key_finder_.erase(key_store_.back());
      key_store_.pop_back();
    }

    virtual K const & candidate() const
    {
      return key_store_.back();
    }

  private:
    typedef typename std::list<K>::iterator iterator_type;

    std::list<K> key_store_;
    std::unordered_map<K, iterator_type> key_finder_;
};

template <typename K>
class lfu:
  public policy<K>
{
  public:
    lfu() = default;
    ~lfu() = default;

  public:
    virtual void insert(K const & k)
    {
      constexpr std::size_t INIT_VAL = 1;

      // all new value initialized with the frequency 1
      key_finder_[k] = key_store_.emplace_hint(key_store_.cbegin(),
                                               INIT_VAL,
                                               k);
    }

    virtual void touch(K const & k)
    {
      // get the previous frequency value of a key
      auto elem_for_update = key_finder_[k];
      auto updated_elem = std::make_pair(elem_for_update->first + 1,
                                         elem_for_update->second);

      // update the previous value
      key_store_.erase(elem_for_update);
      key_finder_[k] = key_store_.emplace_hint(key_store_.cend(),
                                               std::move(updated_elem));
    }

    virtual void erase(K const & k)
    {
      key_store_.erase(key_finder_[k]);
      key_finder_.erase(k);
    }

    virtual K const & candidate() const
    {
      // at the beginning of the key_store_ we have the
      // least frequency used value
      return key_store_.crbegin()->second;
    }

  private:
    typedef typename std::multimap<std::size_t, K>::iterator iterator_type;

    std::multimap<std::size_t, K> key_store_;
    std::unordered_map<K, iterator_type> key_finder_;
};

} /** !policies */
} /** !cache */
} /** !bokasafn */

#endif /** !BOKASAFN_CACHE_POLICIES_HH_ */
