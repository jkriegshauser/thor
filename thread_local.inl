/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * thread_local.inl
 *
 * Platform independent thread-local variable support implementation
 */

namespace thor
{

template<typename T> thread_local<T>::thread_local() THOR_NOTHROW
    : internal::thread_local_base()
{
    THOR_COMPILETIME_ASSERT(sizeof(T) <= sizeof(internal::thread_local_base::base_value_type), InvalidAssumption);
}

template<typename T> thread_local<T>::~thread_local() THOR_NOTHROW
{
}

template<typename T> typename thread_local<T>::value_type thread_local<T>::get() const THOR_NOTHROW
{
    internal::thread_local_base::base_value_type v = internal::thread_local_base::get();
    return *reinterpret_cast<pointer>(&v);
}

template<typename T> typename thread_local<T>::value_type thread_local<T>::set(value_type t) THOR_NOTHROW
{
    internal::thread_local_base::base_value_type v =
        *reinterpret_cast<internal::thread_local_base::base_value_type*>(&t);
    internal::thread_local_base::set(v);
    return t;
}

}