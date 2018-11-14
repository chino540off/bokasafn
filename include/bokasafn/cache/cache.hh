/**
 *  @file cache.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef BOKASAFN_CACHE_SIZED_HH_
# define BOKASAFN_CACHE_SIZED_HH_

# include <mutex>
# include <unordered_map>

# include <bokasafn/literals/size.hh>

# include <bokasafn/cache/policies.hh>

namespace bokasafn {
namespace cache {

template <typename V>
struct sized
{
  typedef literals::size type_value;

  template <typename T>
  static type_value value(T const & n)
  {
    return n * sizeof (V);
  }
};

template <typename V>
struct numbered
{
  typedef unsigned int type_value;

  template <typename T>
  static type_value value(T const & n)
  {
    return n;
  }
};
/**
 * @brief 
 */
template <typename K, typename V,
          template <typename> class S,
          template <typename> class P = policies::nopolicy>
class _cache
{
  public:
    _cache(typename S<V>::type_value s): size_(s) { }

  public:
    /**
     * @brief Put a new key/value in the cache store
     *
     * @param k a K key
     * @param v a V value
     */
    void put(K const & k, V const & v)
    {
      std::lock_guard<std::mutex>{mutex_};

      auto it = map_.find(k);

      if (it == map_.end())
      {
        if (S<V>::value(map_.size() + 1) > size_)
        {
          auto to_remove = policy_.candidate();

          erase(to_remove);
        }
        insert(k, v);
      }
      else
      {
        update(k, v);
      }
    }

    /**
     * @brief Get a V value from a 
     *
     * @param k a K key
     *
     * @return a V value
     */
    V const & get(K const & k) const
    {
      std::lock_guard<std::mutex>{mutex_};

      auto it = map_.find(k);

      if (it == map_.end())
        throw std::range_error("no such key in cache");

      policy_.touch(k);

      return it->second;
    }

  private:
    void insert(K const & k, V const & v)
    {
      policy_.insert(k);

      map_.emplace(std::make_pair(k, v));
    }

    void erase(K const & k)
    {
      policy_.erase(k);

      auto elem_it = map_.find(k);
      map_.erase(elem_it);
    }

    void update(K const & k, V const & v)
    {
      policy_.touch(k);
      map_[k] = v;
    }

  private:
    mutable P<K> policy_;
    mutable std::mutex mutex_;

    std::unordered_map<K, V> map_;
    typename S<V>::type_value size_;
};

template <typename K, typename V>
using nno = _cache<K, V, numbered>;
template <typename K, typename V>
using nlru = _cache<K, V, numbered, policies::lru>;
template <typename K, typename V>
using nlfu = _cache<K, V, numbered, policies::lfu>;

template <typename K, typename V>
using sno = _cache<K, V, sized>;
template <typename K, typename V>
using slru = _cache<K, V, sized, policies::lru>;
template <typename K, typename V>
using slfu = _cache<K, V, sized, policies::lfu>;

} /** !cache */
} /** !bokasafn */

#endif /** !BOKASAFN_CACHE_SIZED_HH_ */
