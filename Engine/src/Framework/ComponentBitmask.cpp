
#include "Framework/ComponentBitmask.h"

ComponentBitmask::ComponentBitmask()
	: mask(0)
{ }

void ComponentBitmask::setBit(msize_t bit, bool set)
{
	const msize_t unit_index = bit / (sizeof(munit_t) * 8);
	const msize_t bit_index = bit % (sizeof(munit_t) * 8);

	while(mask.size() <= unit_index) {
		mask.push_back(0);
	}

	munit_t& unit = mask[unit_index];
	munit_t setter = ((munit_t)1 << bit_index);
	if (set) {
		unit = unit | setter;
	} else {
		unit = unit & (~setter);
	}
}

bool ComponentBitmask::isBitSet(msize_t bit)
{
	const msize_t unit_index = bit / (sizeof(munit_t) * 8);
	const msize_t bit_index = bit % (sizeof(munit_t) * 8);

	if (mask.size() < unit_index) {
		return false;
	}

	munit_t unit = mask[unit_index];
	return (unit & (1 << bit_index)) > 0;
}

bool ComponentBitmask::hasComponents(const ComponentBitmask& other)
{
	// If the other mask has more bits than us, we assume that one of those later bits
	// is set (and that bit isn't set for us because it doesn't exist)
	if (other.mask.size() > this->mask.size()) {
		return false;
	}

	// Otherwise, do a unit-by-unit comparison
	for(unsigned i = 0; i < other.mask.size(); i++) {
		munit_t and_unit = this->mask[i] & other.mask[i];
		if (and_unit != other.mask[i]) {
			return false;
		}
	}

	return true;
}
