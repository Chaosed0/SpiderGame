#pragma once

#include <vector>
#include <cinttypes>

/*! "mask size type" - type used when specifying the index of a bit in the mask.
	Implicitly defines the maximum number of bits that the mask can contain. */
typedef uint32_t msize_t;

/*! Arbitrary length bitmask. */
class ComponentBitmask
{
public:
	ComponentBitmask();

	/*!
	 * \brief Sets or unsets the given bit within the bitmask.
	 * \param bit Bit index to set.
	 * \param set True to set, false to unset.
	 */
	void setBit(msize_t bit, bool set);

	/*!
	 * \brief Checks if a single bit is set.
	 * \param bit The bit index to check. */
	bool isBitSet(msize_t bit);

	/*!
	 * \brief Checks if all the bits in other are set in this.
	 */
	bool hasComponents(const ComponentBitmask& other);
private:
	/*! "mask unit type" - size of a single "unit" in the vector. A unit is the type
		we actually operate upon, rather than dealing bit-by-bit. */
	typedef uint32_t munit_t;

	/*! The actual bitmask we use. If a unit doesn't exist, it is considered to be
		set to all zeroes. */
	std::vector<munit_t> mask;
};