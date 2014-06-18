/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * bitset.h
 *
 * Implementation of a C++ Standard Library-compatible bitset
 */

#ifndef THOR_BITSET_H
#define THOR_BITSET_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_BASIC_STRING_H
#include "basic_string.h"
#endif

#ifndef THOR_VECTOR_H
#include "vector.h"
#endif

namespace thor
{

template <unsigned N> class bitset
{
public:
	typedef bool value_type;
	typedef bool* pointer;
	typedef const bool* const_pointer;
	typedef const bool& const_reference;
	typedef thor_size_type size_type;
	typedef thor_diff_type difference_type;

	const static size_type bitsize = (size_type)N;

	// A proxy class that acts as a reference to a single bit
	class reference
	{
        friend class bitset<N>;
		thor_size_type* data_;
		thor_size_type  mask_;

		reference(thor_size_type* d, thor_size_type mask) : data_(d), mask_(mask) {}
	public:
		reference(const reference& rhs) : data_(rhs.data_), mask_(rhs.mask_) {}
		~reference() {}

		operator bool () const
		{
			THOR_ASSERT(data_);
			THOR_ASSERT(mask_ != 0);
			return (*data_ & mask_) == mask_;
		}

		bool operator ~ () const
		{
			return ! operator bool();
		}

		reference& operator = (bool b)
		{
			THOR_ASSERT(data_);
			THOR_ASSERT(mask_ != 0);
			if (b)
			{
				*data_ |= mask_;
			}
			else
			{
				*data_ &= ~mask_;
			}
			return *this;
		}

		reference& operator = (const reference& rhs)
		{
			return operator = ((bool)rhs);
		}

		reference& flip()
		{
			THOR_ASSERT(data_);
			THOR_ASSERT(mask_ != 0);
			*data_ ^= mask_;
			return *this;
		}
	};

	bitset();
	bitset(unsigned long val);
	template <class Ch> explicit bitset(const basic_string<Ch>& s, size_type pos = 0, size_type n = basic_string<Ch>::npos)
        : data_(storage_size)
        , bitsize_(bitsize)
    {
        from_string(s, pos, n);
    }
	bitset(const bitset& rhs);
	~bitset();

    bool operator == (const bitset& rhs) const;

	bitset& operator = (const bitset& rhs);

	bitset& operator &= (const bitset& rhs);
	bitset& operator |= (const bitset& rhs);
	bitset& operator ^= (const bitset& rhs);
	bitset& operator <<= (size_type t);
	bitset& operator >>= (size_type t);
	bitset  operator << (size_type t) const;
	bitset  operator >> (size_type t) const;
	bitset& set();
	bitset& flip();
	bitset  operator ~ () const;
	bitset& reset();
	bitset& set(size_type n, int val = 1);
	bitset& reset(size_type n);
	bitset& flip(size_type n);
    bitset& rotate_left(size_type n);
    bitset& rotate_right(size_type n);
	size_type size() const;
	size_type count() const;
	bool any() const;
    bool all() const;
	bool none() const;
	bool test(size_type n) const;
	reference operator [] (size_type n);
	bool operator [] (size_type n) const;
    void from_ulong(unsigned long ulong);
	unsigned long to_ulong() const;
    template<class Ch> void from_string(const basic_string<Ch>& s, size_type pos = 0, size_type n = basic_string<Ch>::npos);
	template<class Ch> basic_string<Ch> to_string() const;

private:
	const static size_type bits_per_size_type = sizeof(size_type) * 8;
	const static size_type storage_size = (bitsize + (bits_per_size_type - 1)) / bits_per_size_type;

	void ensure(size_type bits);
    size_type remainder_mask() const;
    void check_remainder() const;
    void clear_remainder();

	typedef vector<size_type, (unsigned)storage_size> vector_type;
	vector_type data_;
	size_type bitsize_;
};

}

template<unsigned N> bool operator == (const thor::bitset<N>& lhs, const thor::bitset<N>& rhs)
{
    return lhs.operator == (rhs);
}

template<unsigned N> bool operator != (const thor::bitset<N>& lhs, const thor::bitset<N>& rhs)
{
    return !(lhs.operator == (rhs));
}

template<unsigned N> thor::bitset<N> operator & (const thor::bitset<N>& lhs, const thor::bitset<N>& rhs)
{
    thor::bitset<N> ret(lhs);
    ret &= rhs;
    return ret;
}

template<unsigned N> thor::bitset<N> operator | (const thor::bitset<N>& lhs, const thor::bitset<N>& rhs)
{
    thor::bitset<N> ret(lhs);
    ret |= rhs;
    return ret;
}

template<unsigned N> thor::bitset<N> operator ^ (const thor::bitset<N>& lhs, const thor::bitset<N>& rhs)
{
    thor::bitset<N> ret(lhs);
    ret ^= rhs;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////

namespace thor
{

template<unsigned N> bitset<N>::bitset()
	: data_(storage_size)
	, bitsize_(bitsize)
{}

template<unsigned N> bitset<N>::bitset(unsigned long uval)
	: data_(storage_size)
	, bitsize_(bitsize)
{
    from_ulong(uval);
}

template<unsigned N> bitset<N>::bitset(const bitset& rhs)
	: data_(rhs.data_)
	, bitsize_(rhs.bitsize_)
{
    check_remainder();
}

template<unsigned N> bitset<N>::~bitset()
{}

template<unsigned N> bool bitset<N>::operator == (const bitset& rhs) const
{
    if (size() != rhs.size()) return false;

    for (vector_type::const_iterator iter(data_.begin()), riter(rhs.data_.begin());
         iter != data_.end();
         ++iter, ++riter)
    {
        if (*iter != *riter)
        {
            return false;
        }
    }
    return true;
}

template<unsigned N> bitset<N>& bitset<N>::operator = (const bitset& rhs)
{
	data_ = rhs.data_;
	bitsize_ = rhs.size();
    check_remainder();
	return *this;
}

template<unsigned N> bitset<N>& bitset<N>::operator &= (const bitset& rhs)
{
	if (THOR_SUPPRESS_WARNING(bitsize == 0))
	{
		// resize to match rhs
		ensure(rhs.size());

		// must ensure that we only perform the operation on the number of bits in rhs
		const size_type leftover = rhs.size() % bits_per_size_type;
		size_type iters = rhs.size() / bits_per_size_type;

        vector_type::const_iterator rhsiter(rhs.data_.begin());
        vector_type::iterator iter(data_.begin());
		for (; iters != 0; ++iter, ++rhsiter, --iters)
		{
			*iter &= *rhsiter;
		}

		if (leftover > 0)
		{
			const size_type mask = (*rhsiter & ~((size_type(1) << (bits_per_size_type - leftover)) - 1)) |
								   (*iter    &  ((size_type(1) << (bits_per_size_type - leftover)) - 1));
			*iter &= mask;
		}
	}
	else
	{
        vector_type::const_iterator rhsiter(rhs.data_.begin());
		for (vector_type::iterator iter(data_.begin());
			 iter != data_.end();
			 ++iter, ++rhsiter)
		{
			*iter &= *rhsiter;
		}
	}
    check_remainder();
	return *this;
}

template<unsigned N> bitset<N>& bitset<N>::operator |= (const bitset& rhs)
{
	if (THOR_SUPPRESS_WARNING(bitsize == 0))
	{
		// resize to match rhs
		ensure(rhs.size());
    }

    vector_type::const_iterator rhsiter(rhs.data_.begin());
	for (vector_type::iterator iter(data_.begin());
			iter != data_.end();
			++iter, ++rhsiter)
	{
		*iter |= *rhsiter;
	}
    check_remainder();
	return *this;
}

template<unsigned N> bitset<N>& bitset<N>::operator ^= (const bitset& rhs)
{
	if (THOR_SUPPRESS_WARNING(bitsize == 0))
	{
		// resize to match rhs
		ensure(rhs.size());
    }

    vector_type::const_iterator rhsiter(rhs.data_.begin());
	for (vector_type::iterator iter(data_.begin());
			iter != data_.end();
			++iter, ++rhsiter)
	{
		*iter ^= *rhsiter;
	}
    check_remainder();
	return *this;
}

template<unsigned N> bitset<N>& bitset<N>::operator <<= (size_type t)
{
	if (t >= size())
	{
		reset();    // shifted past all bits
	}
	else if (t > 0)
	{
		const size_type skip = t / bits_per_size_type;
		const size_type shift_bits = t % bits_per_size_type;
		const size_type remainder_bits = (bits_per_size_type - shift_bits);

		vector_type::iterator to = data_.begin();
		vector_type::iterator from = to + skip;
		for (; from != data_.end(); ++to)
		{
			*to = *from;
			*to <<= shift_bits;
			if (++from != data_.end())
			{
				*to |= (*from >> remainder_bits);
			}
		}

		for (; to != data_.end(); ++to)
		{
			*to = size_type(0);
		}
	}
    check_remainder();
	return *this;
}

template<unsigned N> bitset<N>& bitset<N>::operator >>= (size_type t)
{
	if (t >= size())
	{
		reset();    // shifted past all bits
	}
	else if (t > 0)
	{
		const size_type skip = t / bits_per_size_type;
		const size_type shift_bits = t % bits_per_size_type;
		const size_type remainder_bits = (bits_per_size_type - shift_bits);

		vector_type::reverse_iterator to = data_.rbegin();
		vector_type::reverse_iterator from = to + skip;
		for (; from != data_.rend(); ++to)
		{
			*to = *from;
			*to >>= shift_bits;
			if (++from != data_.rend())
			{
				*to |= (*from << remainder_bits);
			}
		}

		for (; to != data_.rend(); ++to)
		{
			*to = size_type(0);
		}
	}
    clear_remainder();
    check_remainder();
	return *this;
}

template<unsigned N> bitset<N>  bitset<N>::operator << (size_type t) const
{
	bitset<N> ret(*this);
	ret <<= t;
	return ret;
}

template<unsigned N> bitset<N>  bitset<N>::operator >> (size_type t) const
{
	bitset<N> ret(*this);
	ret >>= t;
	return ret;
}

template<unsigned N> bitset<N>& bitset<N>::set()
{
    const vector_type::iterator end(data_.end());
    for (vector_type::iterator iter(data_.begin()); iter != end; ++iter)
    {
        *iter = size_type(-1);
    }
    clear_remainder();
    check_remainder();

    return *this;
}

template<unsigned N> bitset<N>& bitset<N>::flip()
{
    const vector_type::iterator end = data_.end();
    for (vector_type::iterator iter(data_.begin()); iter != end; ++iter)
    {
        *iter ^= size_type(-1);
    }
    clear_remainder();
    check_remainder();

    return *this;
}

template<unsigned N> bitset<N> bitset<N>::operator ~ () const
{
	bitset<N> copy(*this);
	copy.flip();
	return copy;
}

template<unsigned N> bitset<N>& bitset<N>::reset()
{
    const vector_type::iterator end(data_.end());
    for (vector_type::iterator iter(data_.begin()); iter != end; ++iter)
    {
        *iter = size_type(0);
    }    
    check_remainder();

    return *this;
}

template<unsigned N> bitset<N>& bitset<N>::set(size_type n, int val /*=1*/)
{
    ensure(n + 1);
    const size_type index = n / bits_per_size_type;
    const size_type bit   = bits_per_size_type - (n % bits_per_size_type) - 1;
    if (val == 0)
    {
        data_[index] &= ~(size_type(1) << bit);
    }
    else
    {
        data_[index] |= (size_type(1) << bit);
    }
    check_remainder();
    return *this;
}

template<unsigned N> bitset<N>& bitset<N>::reset(size_type n)
{
    ensure(n + 1);
    const size_type index = n / bits_per_size_type;
    const size_type bit   = bits_per_size_type - (n % bits_per_size_type);
    data_[index] &= ~(size_type(1) << (bit - 1));
    check_remainder();
    return *this;
}

template<unsigned N> bitset<N>& bitset<N>::flip(size_type n)
{
    ensure(n + 1);
    const size_type index = n / bits_per_size_type;
    const size_type bit   = bits_per_size_type - (n % bits_per_size_type);
    data_[index] ^= (size_type(1) << (bit - 1));
    check_remainder();
    return *this;
}

template<unsigned N> bitset<N>& bitset<N>::rotate_left(size_type n)
{
    n %= size();
    if (n == 0) return *this;

    // Simple; non-optimal
    bitset b1(*this), b2(*this);
    b1 <<= n, b2 >>= (size() - n);
    b1 |= b2;
    b1.data_.swap(data_);
    return *this;
}

template<unsigned N> bitset<N>& bitset<N>::rotate_right(size_type n)
{
    n %= size();
    if (n == 0) return *this;

    // Simple; non-optimal
    bitset b1(*this), b2(*this);
    b1 >>= n, b2 <<= (size() - n);
    b1 |= b2;
    b1.data_.swap(data_);
    return *this;
}

template<unsigned N> typename bitset<N>::size_type bitset<N>::size() const
{
    THOR_ASSERT(bitsize == bitsize_ || bitsize == 0);
    // As a compile-time optimization, return the size parameter if it's
    // not dynamic (equal to zero). Dynamic bitsets return the current size
    return THOR_SUPPRESS_WARNING(N == 0) ? bitsize_ : (size_type)N;
}

template<unsigned N> typename bitset<N>::size_type bitset<N>::count() const
{
    // Algorithm below only works as written for types up to 128 bits according to URL given below
    THOR_COMPILETIME_ASSERT(sizeof(size_type) < 16, InvalidAssumption);
    size_type bits = 0;
    const vector_type::const_iterator end(data_.end());
    for (vector_type::const_iterator iter(data_.begin()); iter != end; ++iter)
    {
        // From: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
        size_type v = *iter - ((*iter >> 1) & (size_type(-1)/3));
        v = (v & (size_type(-1)/15*3)) + ((v >> 2) & (size_type(-1)/15*3));
        v = (v + (v >> 4)) & (size_type(-1)/255*15);
        bits += (v * ((size_type(-1)/255))) >> (sizeof(size_type) - 1) * 8;
    }
    return bits;
}

template<unsigned N> bool bitset<N>::any() const
{
    for (vector_type::const_iterator iter = data_.begin(); iter != data_.end(); ++iter)
    {
        if (*iter != size_type(0))
        {
            return true;
        }
    }
    return false;
}

template<unsigned N> bool bitset<N>::all() const
{
    for (vector_type::const_iterator iter = data_.begin(); iter != data_.end(); ++iter)
    {
        if (*iter != size_type(-1))
        {
            // Could legitmately be the last item
            if ((iter + 1) == data_.end())
            {
                const size_type mask = ~remainder_mask();
                return (*iter & mask) == mask;
            }
            return false;
        }
    }
    return true;
}

template<unsigned N> bool bitset<N>::none() const
{
    return !any();
}

template<unsigned N> bool bitset<N>::test(size_type n) const
{
    if (n >= size()) return false;
    const size_type index = n / bits_per_size_type;
    const size_type bit   = bits_per_size_type - (n % bits_per_size_type);
    return (data_[index] & (size_type(1) << (bit - 1))) != size_type(0);
}

template<unsigned N> typename bitset<N>::reference bitset<N>::operator [] (size_type n)
{
    ensure(n + 1);
    const size_type index = n / bits_per_size_type;
    const size_type bit   = bits_per_size_type - (n % bits_per_size_type);
    return reference(&data_[index], size_type(1) << (bit - 1));
}

template<unsigned N> bool bitset<N>::operator [] (size_type n) const
{
    THOR_ASSERT(n < size());
    const size_type index = n / bits_per_size_type;
    const size_type bit   = bits_per_size_type - (n % bits_per_size_type);
    return n < size() ? (data_[index] & (size_type(1) << (bit - 1))) != 0 : false;
}

template<unsigned N> void bitset<N>::from_ulong(unsigned long uval)
{
    THOR_COMPILETIME_ASSERT(sizeof(uval) <= sizeof(size_type), InvalidAssumption);
	if (THOR_SUPPRESS_WARNING(bitsize == 0))
	{
		// Resize to take the entire value
		ensure(sizeof(uval) * 8);
	}
    else if (THOR_SUPPRESS_WARNING(bitsize < (8 * sizeof(unsigned long))))
    {
        // Set any unused bits to zero
        const unsigned long mask = (1ul << ((sizeof(unsigned long) * 8) - size())) - 1;
        uval &= ~mask;
    }

    // Resize to size_type and shift down
	size_type uvalx = size_type(uval);
	uvalx <<= (8 * (sizeof(size_type) - sizeof(uval)));

    if (THOR_SUPPRESS_WARNING(bitsize > (8 * sizeof(unsigned long)) && sizeof(unsigned long) != sizeof(size_type)))
    {
        // Preserve any bits that would be overwritten
        const size_type mask = (size_type(1) << (8 * (sizeof(size_type) - sizeof(uval)))) - 1;
        uvalx |= (data_[0] & mask);
    }

    data_[0] = uvalx;
    check_remainder();
}

template<unsigned N> unsigned long bitset<N>::to_ulong() const
{
    THOR_COMPILETIME_ASSERT(sizeof(unsigned long) <= sizeof(size_type), InvalidAssumption);
    size_type first = data_.empty() ? 0 : data_[0];
    first >>= (8 * (sizeof(size_type) - sizeof(unsigned long)));
    return (unsigned long)first;
}

template<unsigned N> template<class Ch> void bitset<N>::from_string(const basic_string<Ch>& s, size_type pos /*=0*/, size_type n /*=basic_string<Ch>::npos*/)
{
	if (pos < s.size())
	{
		if (n > (s.size() - pos)) n = s.size() - pos;
		ensure(n); // will assert if string too long for data
        if (n > size()) n = size();

		vector_type::iterator iter = data_.begin();
        THOR_ASSERT(iter != data_.end());
        size_type mask = size_type(1) << (bits_per_size_type - 1);
		const Ch* c = s.c_str() + pos;
		const Ch* end = c + n;
		do
		{
			if (*c == Ch('1'))
			{
				*iter |= mask;
			}
			else if (*c == Ch('0'))
			{
				*iter &= ~mask;
			}
			else
			{
				THOR_ASSERT(0);
			}
			++c;
            mask >>= 1;
			if (mask == 0)
			{
				mask = size_type(1) << (bits_per_size_type - 1);
				++iter;
                THOR_ASSERT(iter != data_.end() || c == end);
			}
		} while (c < end);
	}
    check_remainder();
}

template<unsigned N> template<class Ch> basic_string<Ch> bitset<N>::to_string() const
{
    basic_string<Ch> str;
    if (data_.empty())
    {
        str.assign(size(), Ch('0'));
    }
    else
    {
        str.reserve(size());
        vector_type::const_iterator iter(data_.begin());
        size_type mask = size_type(1) << (bits_per_size_type - 1);
        for (size_type i = 0; i < size(); ++i)
        {
            str.push_back((*iter & mask) != 0 ? Ch('1') : Ch('0'));
            mask >>= 1;
            if (mask == 0)
            {
                mask = size_type(1) << (bits_per_size_type - 1);
                ++iter;
            }
        }
    }
    return str;
}

template<unsigned N> void bitset<N>::ensure(size_type n)
{
	if (size() < n)
	{
		THOR_ASSERT(THOR_SUPPRESS_WARNING(bitsize == 0)); // Only dynamic bitsets can be resized
		const size_type newsize = (n + (bits_per_size_type - 1)) / bits_per_size_type;
		data_.reserve(newsize);
		data_.resize(newsize);
		bitsize_ = n;
	}
}

template<unsigned N> typename bitset<N>::size_type bitset<N>::remainder_mask() const
{
    const size_type remainder = bits_per_size_type - (size() % bits_per_size_type);
    return remainder < bits_per_size_type ? (size_type(1) << remainder) - 1 : 0;
}

template<unsigned N> void bitset<N>::check_remainder() const
{
    THOR_ASSERT((*data_.rbegin() & remainder_mask()) == 0);
}

template<unsigned N> void bitset<N>::clear_remainder()
{
    *data_.rbegin() &= ~remainder_mask();
}

}

#endif