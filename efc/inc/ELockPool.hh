/*
 * ELockPool.hh
 *
 *  Created on: 2014-6-4
 *      Author: cxxjava@163.com
 */

#ifndef ELOCKPOOL_HH_
#define ELOCKPOOL_HH_

//@see: sofa-pbrpc-master/src/sofa/pbrpc/smart_ptr/detail/spinlock_pool.hpp

#include "ESpinLock.hh"

namespace efc {

#define POLLSIZE 41

//use spin lock

#define SSYNCBLOCKFOR(i, p) SYNCBLOCK(ESpinLockPool<i>::lockFor(p))

#define SCOPED_SLOCK0(p) SSYNCBLOCKFOR(0, p) //0 for sp!!!
#define SCOPED_SLOCK1(p) SSYNCBLOCKFOR(1, p) //1 for nio!!!

#define SCOPED_SLOCK2(p) SSYNCBLOCKFOR(2, p) //2 for ?
#define SCOPED_SLOCK3(p) SSYNCBLOCKFOR(3, p) //3 for ?
#define SCOPED_SLOCK4(p) SSYNCBLOCKFOR(4, p) //4 for ?
#define SCOPED_SLOCK5(p) SSYNCBLOCKFOR(5, p) //5 for ?

template< int I > class ESpinLockPool
{
private:
    static ELock* pool_[POLLSIZE];

public:
    static ELock* lockFor(void const* pv)
    {
    	es_size_t i = reinterpret_cast<es_size_t>(pv) % POLLSIZE;
        return pool_[i];
    }
};

#define ES_SPINLOCK_INIT new ESpinLock()

template< int I > ELock* ESpinLockPool< I >::pool_[POLLSIZE] =
{
	ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT,
	ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT,
	ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT,
	ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT,
	ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT,
	ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT,
	ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT,
	ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT, ES_SPINLOCK_INIT,
	ES_SPINLOCK_INIT
};

} // namespace efc

#endif /* ELOCKPOOL_HH_ */
