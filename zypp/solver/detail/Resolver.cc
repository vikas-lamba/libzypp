/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Resolver.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <sys/utsname.h>

#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/Helper.h"

#include "zypp/CapSet.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/ZYppFactory.h"
#include "zypp/SystemResObject.h"



/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

IMPL_PTR_TYPE(Resolver);

//---------------------------------------------------------------------------

std::ostream &
Resolver::dumpOn( std::ostream & os ) const
{
    return os << "<resolver/>";
}

// Generating a system resolvable in the pool in order to trigger
// modaliases and hals	
void assertSystemResObjectInPool()
{
  ResPool pool( getZYpp()->pool() );
  if ( pool.byKindBegin<SystemResObject>()
       == pool.byKindEnd<SystemResObject>() )
    {
      // SystemResObject is missing in the pool ==> insert
      ResStore store;
      store.insert( SystemResObject::instance() ); 
      getZYpp()->addResolvables( store, true ); // true = is installed
    }

  // set lock
  if ( ! pool.byKindBegin<SystemResObject>()
         ->status().setLock( true, ResStatus::USER ) )
    {
      WAR << "Unable to set SystemResObject to lock" << endl;
    }
}	

//---------------------------------------------------------------------------

Resolver::Resolver (const ResPool & pool)
    : _pool (pool)
    , _timeout_seconds (0)
    , _verifying (false)
    , _testing (false)
    , _valid_solution_count (0)
    , _best_context (NULL)
    , _timed_out (false)
    , _forceResolve (false)
    , _upgradeMode (false)
{
    struct utsname buf;
    if (uname (&buf) < 0) {
	ERR << "Can't determine system architecture" << endl;
    }
    else {
	MIL << "System architecture is '" << buf.machine << "'" << endl;
	_architecture = Arch(buf.machine);
    }
}


Resolver::~Resolver()
{
}

//---------------------------------------------------------------------------

ResPool
Resolver::pool (void) const
{
    return _pool;
}

void
Resolver::reset (void)
{
    _timeout_seconds = 0;
    _verifying = false;

    _initial_items.clear();

    _items_to_install.clear();
    _items_to_remove.clear();
    _items_to_verify.clear();
    _items_to_establish.clear();

    _extra_caps.clear();
    _extra_conflicts.clear();

    _pending_queues.clear();
    _pruned_queues.clear();
    _complete_queues.clear();
    _deferred_queues.clear();
    _invalid_queues.clear();

    _valid_solution_count = 0;

    _best_context = NULL;
    _timed_out = false;
}


ResolverContext_Ptr
Resolver::context (void) const
{
    if (_best_context) return _best_context;
    if (_invalid_queues.empty()) return NULL;
    ResolverQueue_Ptr invalid = _invalid_queues.front();
    return invalid->context();
}

//---------------------------------------------------------------------------

void
Resolver::addSubscribedSource (Source_Ref source)
{
    _subscribed.insert(source);
}

void
Resolver::addPoolItemToInstall (PoolItem_Ref item)
{
    bool found = false;
    for (PoolItemList::const_iterator iter = _items_to_remove.begin();
	 iter != _items_to_remove.end(); iter++) {
	if (*iter == item) {
	    _items_to_remove.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found)
	_items_to_install.push_back (item);
}


void
Resolver::addPoolItemsToInstallFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToInstall (*iter);
    }
}


void
Resolver::addPoolItemToRemove (PoolItem_Ref item)
{
    bool found = false;
    for (PoolItemList::const_iterator iter = _items_to_install.begin();
	 iter != _items_to_install.end(); iter++) {
	if (*iter == item) {
	    _items_to_install.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found)
	_items_to_remove.push_back (item);
}


void
Resolver::addPoolItemsToRemoveFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToRemove (*iter);
    }
}


void
Resolver::addPoolItemToEstablish (PoolItem_Ref item)
{
    _items_to_establish.push_back (item);
}


void
Resolver::addPoolItemsToEstablishFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToEstablish (*iter);
    }
}


void
Resolver::addPoolItemToVerify (PoolItem_Ref item)
{
#if 0
  /** Order PoolItems based on name and edition only. */
  struct {
    /** 'less then' based on name and edition */
    bool operator()( PoolItem_Ref lhs, PoolItem_Ref rhs ) const
    {
      int res = lhs->name().compare( rhs->name() );
      if ( res )
        return res == -1; // lhs < rhs ?
      // here: lhs == rhs, so compare edition:
      return lhs->edition() < rhs->edition();
    }
  } order;
#endif

    _items_to_verify.push_back (item);

#warning Should order by name (and probably edition since with zypp we could have multiple editions installed in parallel)
//    _items_to_verify.sort (order);			//(GCompareFunc) rc_item_compare_name);
}


void
Resolver::addExtraCapability (const Capability & capability)
{
    _extra_caps.insert (capability);
}


void
Resolver::addExtraConflict (const Capability & capability)
{
    _extra_conflicts.insert (capability);
}


void
Resolver::addIgnoreConflict (const PoolItem_Ref item,
		   const Capability & capability)
{
    _ignoreConflicts.insert(make_pair(item, capability));
}


void
Resolver::addIgnoreRequires (const PoolItem_Ref item,
			     const Capability & capability)
{
    _ignoreRequires.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreObsoletes (const PoolItem_Ref item,
			      const Capability & capability)
{
    _ignoreObsoletes.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreInstalledItem (const PoolItem_Ref item)
{
    _ignoreInstalledItem.push_back (item);
}

//---------------------------------------------------------------------------

struct VerifySystem : public resfilter::PoolItemFilterFunctor
{
    Resolver & resolver;

    VerifySystem (Resolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem_Ref provider )
    {
	resolver.addPoolItemToVerify (provider);
	return true;
    }
};


bool
Resolver::verifySystem (void)
{
    _DEBUG( "Resolver::verifySystem()" );

    VerifySystem info (*this);

    invokeOnEach( pool().byKindBegin( ResTraits<Package>::kind ),
		  pool().byKindEnd( ResTraits<Package>::kind ),
		  resfilter::ByInstalled ( ),
		  functor::functorRef<bool,PoolItem>(info) );


    _verifying = true;

    return resolveDependencies ();
}


//---------------------------------------------------------------------------

// copy marked item from solution back to pool
// if data != NULL, set as APPL_HIGH (from establishPool())

static void
solution_to_pool (PoolItem_Ref item, const ResStatus & status, void *data)
{
    bool r;

    if (status.isToBeInstalled()) {
	r = item.status().setToBeInstalled( (data != NULL) ? ResStatus::APPL_HIGH : ResStatus::SOLVER );
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") install !" << r);
    }
    else if (status.isToBeUninstalledDueToUpgrade()) {
	r = item.status().setToBeUninstalledDueToUpgrade( (data != NULL) ? ResStatus::APPL_HIGH : ResStatus::SOLVER );
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") upgrade !" << r);
    }
    else if (status.isToBeUninstalled()) {
	r = item.status().setToBeUninstalled( (data != NULL) ? ResStatus::APPL_HIGH : ResStatus::SOLVER );
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") remove !" << r);
    }
    else if (status.isIncomplete()
	     || status.isNeeded()) {
	r = item.status().setIncomplete();
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") incomplete !" << r);
    }
    else if (status.isUnneeded()) {
	r = item.status().setUnneeded();
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") unneeded !" << r);
    }
    else if (status.isSatisfied()) {
	r = item.status().setSatisfied();
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") satisfied !" << r);
    } else {
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") unchanged !");
    }
    return;
}


//---------------------------------------------------------------------------


// establish state

struct EstablishState
{
    Resolver & resolver;

    EstablishState (Resolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem_Ref provider )
    {
	resolver.addPoolItemToEstablish (provider);
	return true;
    }
};


void
Resolver::establishState (ResolverContext_Ptr context)
{
    _DEBUG( "Resolver::establishState ()" );
    typedef list<Resolvable::Kind> KindList; 
    static KindList ordered;
    if (ordered.empty()) {
	ordered.push_back (ResTraits<zypp::Atom>::kind);
	ordered.push_back (ResTraits<zypp::Patch>::kind);
	ordered.push_back (ResTraits<zypp::Pattern>::kind);
	ordered.push_back (ResTraits<zypp::Product>::kind);
    }

    if (context == NULL)
	context = new ResolverContext(_pool, _architecture);

    context->setEstablishing (true);
    context->setIgnoreCababilities (_ignoreConflicts,
				    _ignoreRequires,
				    _ignoreObsoletes,
				    _ignoreInstalledItem);
    context->setForceResolve (_forceResolve);
    context->setUpgradeMode (_upgradeMode);        
    
    for (KindList::const_iterator iter = ordered.begin(); iter != ordered.end(); iter++) {
	const Resolvable::Kind kind = *iter;

	DBG << "establishing state for kind " << kind.asString() << endl;

	//world()->foreachResItemByKind (kind, trial_establish_cb, this);

	EstablishState info (*this);

	invokeOnEach( pool().byKindBegin( kind ),
		      pool().byKindEnd( kind ),
		      functor::functorRef<bool,PoolItem>(info) );

	// process the queue
	resolveDependencies( context );

	reset();
    }

    context->setEstablishing (false);

    _best_context = context;

    return;
}


bool
Resolver::establishPool ()
{
    establishState ();						// establish !
    ResolverContext_Ptr solution = bestContext();

    if (solution) {						// copy solution back to pool
	solution->foreachMarked (solution_to_pool, (void *)1);	// as APPL_HIGH
    }
    else {
	ERR << "establishState did not return a bestContext" << endl;
	return false;
    }

    return true;
}

//---------------------------------------------------------------------------

bool
Resolver::resolveDependencies (const ResolverContext_Ptr context)
{

    time_t t_start, t_now;

    MIL << "Resolver::resolveDependencies()" << endl;

    _pending_queues.clear();
    _pruned_queues.clear();
    _complete_queues.clear();
    _deferred_queues.clear();
    _invalid_queues.clear();
    _valid_solution_count = 0;
    _best_context = NULL;    

#warning local items disabled
#if 0
    bool have_local_items = false;

    /* Walk through are list of to-be-installed packages and see if any of them are local. */

    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	if ((*iter)->local()) {
	    have_local_items = true;
	    break;
	}
    }

    World_Ptr the_world = world();
    StoreWorld_Ptr local_world = NULL;
    MultiWorld_Ptr local_multiworld = NULL;

    Channel_Ptr local_channel = NULL;

    if (have_local_items) {
	local_multiworld = new MultiWorld();
	local_world = new StoreWorld();

	local_channel = new Channel ("", "Local ResItems", "@local", "");

	local_world->addChannel (local_channel);

	local_multiworld->addSubworld (local_world);
	local_multiworld->addSubworld (the_world);

	the_world = local_multiworld;
    }
#endif

    // create initial_queue

    ResolverQueue_Ptr initial_queue = new ResolverQueue(_pool, _architecture, context);

    // Initialize all ignoring dependencies
    initial_queue->context()->setIgnoreCababilities (_ignoreConflicts,
				    _ignoreRequires,
				    _ignoreObsoletes,
				    _ignoreInstalledItem);
    initial_queue->context()->setForceResolve (_forceResolve);
    initial_queue->context()->setUpgradeMode (_upgradeMode);        

    /* If this is a verify, we do a "soft resolution" */

    initial_queue->context()->setVerifying (_verifying);

    /* Add extra items. */

    for (QueueItemList::const_iterator iter = _initial_items.begin(); iter != _initial_items.end(); iter++) {
	initial_queue->addItem (*iter);
    }

    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	PoolItem_Ref r = *iter;

#warning local items disabled
#if 0
	/* Add local packages to our dummy channel. */
	if (r->local()) {
	    assert (local_channel != NULL);
	    ResItem_Ptr r1 = const_pointer_cast<ResItem>(r);
	    r1->setChannel (local_channel);
	    local_world->addPoolItem_Ref (r);
	}
#endif
	initial_queue->addPoolItemToInstall (r);
    }

    for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
	if (!_upgradeMode)
	    initial_queue->addPoolItemToRemove (*iter, true /* remove-only mode */);
	else
	    //   Checking old dependencies for packages which will be updated.
	    //   E.g. foo provides a dependecy which foo-new does not provides anymore.
	    //   So check, if there is a packages installed which requires foo.
	    //   Testcase exercise-bug150844-test.xml
	    //   Testcase Bug156439-test.xml
	    initial_queue->addPoolItemToRemove (*iter, false /* no remove-only mode */);
    }

    for (PoolItemList::const_iterator iter = _items_to_verify.begin(); iter != _items_to_verify.end(); iter++) {
	initial_queue->addPoolItemToVerify (*iter);
    }

    for (PoolItemList::const_iterator iter = _items_to_establish.begin(); iter != _items_to_establish.end(); iter++) {
	initial_queue->addPoolItemToEstablish (*iter);
    }

    for (CapSet::const_iterator iter = _extra_caps.begin(); iter != _extra_caps.end(); iter++) {
	initial_queue->addExtraCapability (*iter);
    }

    for (CapSet::const_iterator iter = _extra_conflicts.begin(); iter != _extra_conflicts.end(); iter++) {
	initial_queue->addExtraConflict (*iter);
    }

    // Adding System resolvable
    assertSystemResObjectInPool();    

    _DEBUG( "Initial Queue: [" << *initial_queue << "]" );

    if (initial_queue->isEmpty()) {
	INT << "Empty Queue, nothing to resolve" << endl;

	return true;
    }

    _best_context = NULL;

    _pending_queues.push_front (initial_queue);

    time (&t_start);

    while (!_pending_queues.empty()) {

	_DEBUG( "Pend " << (long) _pending_queues.size()
			      << " / Cmpl " << (long) _complete_queues.size()
			      << " / Prun " << (long) _pruned_queues.size()
			      << " / Defr " << (long) _deferred_queues.size()
			      << " / Invl " << (long) _invalid_queues.size() );

	      if (_timeout_seconds > 0) {
		    time (&t_now);
		    if (difftime (t_now, t_start) > _timeout_seconds) {
			_timed_out = true;
		    break;
		}
	    }

	    ResolverQueue_Ptr queue = _pending_queues.front();
	    _pending_queues.pop_front();
	    ResolverContext_Ptr context = queue->context();

	    queue->process();

	if (queue->isInvalid ()) {

	    DBG << "Invalid Queue\n" << endl;;
	    _invalid_queues.push_back(queue);

	} else if (queue->isEmpty ()) {

	    DBG <<"Empty Queue\n" << endl;

	    _complete_queues.push_back(queue);

	    ++_valid_solution_count;

	    /* Compare this solution to our previous favorite.  In the case of a tie,
	       the first solution wins --- yeah, I know this is lame, but it shouldn't
	       be an issue too much of the time. */

	    if (_best_context == NULL
		|| _best_context->compare (context) < 0) {
		_best_context = context;
	    }

	} else if (_best_context != NULL
		   && _best_context->partialCompare (context) > 0) {

	    /* If we aren't currently as good as our previous best complete solution,
	       this solution gets pruned. */

	    DBG << "PRUNED!" << endl;

	    _pruned_queues.push_back(queue);

	} else {

	    /* If our queue is isn't empty and isn't invalid, that can only mean
	       one thing: we are down to nothing but branches. */

	    queue->splitFirstBranch (_pending_queues, _deferred_queues);
	}

	/* If we have run out of pending queues w/o finding any solutions,
	   and if we have deferred queues, make the first deferred queue
	   pending. */

	if (_pending_queues.empty()
	    && _complete_queues.empty()
	    && !_deferred_queues.empty()) {
	    _pending_queues.push_back(_deferred_queues.front());
	}
    }
    _DEBUG("Pend " << (long) _pending_queues.size()
		   << " / Cmpl " << (long) _complete_queues.size()
		   << " / Prun " << (long) _pruned_queues.size()
		   << " / Defr " << (long) _deferred_queues.size()
		   << " / Invl " << (long) _invalid_queues.size() );

    return _best_context && _best_context->isValid();
}


//----------------------------------------------------------------------------
// undo

struct UndoTransact : public resfilter::PoolItemFilterFunctor
{
    UndoTransact ()
    { }

    bool operator()( PoolItem_Ref item )		// only transacts() items go here
    {
	item.status().setTransact(false, ResStatus::SOLVER);// clear any solver/establish transactions
	return true;
    }
};

void
Resolver::undo(void)
{
    UndoTransact info;
    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );
    // These conflict should be ignored of the concering item
    _ignoreConflicts.clear();
    // These requires should be ignored of the concering item    
    _ignoreRequires.clear();
    // These obsoletes should be ignored of the concering item    
    _ignoreObsoletes.clear();
    // Ignore architecture of the item
    _ignoreArchitecture.clear();
    // Ignore the status "installed" of the item
    _ignoreInstalledItem.clear();
    
    return;
}

//----------------------------------------------------------------------------
// resolvePool

struct CollectTransact : public resfilter::PoolItemFilterFunctor
{
    Resolver & resolver;

    CollectTransact (Resolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem_Ref item )		// only transacts() items go here
    {
	ResStatus status = item.status();
	_XDEBUG( "CollectTransact(" << item << ")" );
	bool by_solver = status.isBySolver();

	if (by_solver) {
	    item.status().setTransact(false, ResStatus::SOLVER);// clear any solver/establish transactions
	    return true;				// back out here, dont re-queue former solver result
	}

	if (status.isToBeInstalled()) {
	    resolver.addPoolItemToInstall(item);	// -> install! 
	}
	if (status.isToBeUninstalled()) {
	    resolver.addPoolItemToRemove(item);		// -> remove !
	}
	if (status.isIncomplete()) {			// incomplete (re-install needed)
	    PoolItem_Ref reinstall = Helper::findReinstallItem (resolver.pool(), item);
	    if (reinstall) {
		MIL << "Reinstall " << reinstall << " for incomplete " << item << endl;
		resolver.addPoolItemToInstall(reinstall);	// -> install! 
	    }
	    else {
		WAR << "Can't find " << item << " for re-installation" << endl;
	    }
	}
	return true;
    }
};


static void
show_pool( ResPool pool )
{
    int count = 1;
    static bool full_pool_shown = false;

    MIL << "---------------------------------------" << endl;
    for (ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it, ++count) {

	if (!full_pool_shown					// show item if not shown all before
	    || it->status().transacts()				// or transacts
	    || !it->status().isUndetermined())			// or established status
	{
	    MIL << count << ": " << *it << endl;
	}

    }
    MIL << "---------------------------------------" << endl;
    full_pool_shown = true;
}

//  This function loops over the pool and grabs
//  all item.status().transacts() and item.status().byUser()
//  It clears all previous bySolver() states also
//
//  Every toBeInstalled is passed to zypp::solver:detail::Resolver.addPoolItemToInstall()
//  Every toBeUninstalled is passed to zypp::solver:detail::Resolver.addPoolItemToRemove()
//
//  Then zypp::solver:detail::Resolver.resolveDependencies() is called.
//
//  zypp::solver:detail::Resolver then returns a ResolverContext via bestContext() which
//  describes the best solution. If bestContext() is NULL, no solution was found.
//
//  ResolverContext has a foreachMarked() iterator function which loops over all
//  items of the solutions. These must be written back to the pool.


bool
Resolver::resolvePool ()
{
    
    CollectTransact info (*this);
    
    // cleanup before next run
    reset();
    
#if 1

    MIL << "Resolver::resolvePool()" << endl;
    MIL << "Pool before resolve" << endl;
    show_pool( _pool );

#endif
    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );

    bool have_solution = resolveDependencies ();		// resolve !

    if (have_solution) {					// copy solution back to pool
	MIL << "Have solution, copying back to pool" << endl;
	ResolverContext_Ptr solution = bestContext();
	solution->foreachMarked (solution_to_pool, NULL);
#if 1
	MIL << "Pool after resolve" << endl;
	show_pool( _pool );
#endif
    }
    else {
	MIL << "!!! Have NO solution !!!" << endl;
    }
    return have_solution;
}


//
// transact list of capabilities, return false if one couldn't be matched
//

static bool
transactCaps( const ResPool & pool, const CapSet & caps, bool install, bool soft )
{
    bool result = true;

    for (CapSet::const_iterator it = caps.begin(); it != caps.end(); ++it) {

	PoolItem_Ref installed = Helper::findInstalledByNameAndKind( pool, it->index(), it->refers() );
	PoolItem_Ref uninstalled = Helper::findUninstalledByNameAndKind( pool, it->index(), it->refers() );

	if (install) {
	    if (uninstalled
		&& !uninstalled.status().isLocked())
	    {
		if (soft) 
		    uninstalled.status().setSoftTransact( install, ResStatus::SOLVER );
		else
		    uninstalled.status().setTransact( install, ResStatus::SOLVER );
	    }
	    if (installed
		&& !installed.status().isLocked())
	    {
		if (soft) 
		    installed.status().setSoftTransact( false, ResStatus::SOLVER );
		else
		    installed.status().setTransact( false, ResStatus::SOLVER );
	    }
	} else {
	    // uninstall
	    if (uninstalled
		&& !uninstalled.status().isLocked())
	    {
		if (soft) 
		    uninstalled.status().setSoftTransact( false, ResStatus::SOLVER );
		else
		    uninstalled.status().setTransact( false, ResStatus::SOLVER );
	    }
	    if (installed
		&& !installed.status().isLocked())
	    {
		if (soft) 
		    installed.status().setSoftTransact( true, ResStatus::SOLVER );
		else
		    installed.status().setTransact( true, ResStatus::SOLVER );
	    }	    
	}
	if (!uninstalled
	    && !installed)
	{
	    result = false;
	}
    }
    return result;
}

//
// transact a single object
// -> do a 'single step' resolving either installing or removing
//    required and recommended PoolItems

bool
Resolver::transactResObject( ResObject::constPtr robj, bool install)
{
    if (robj == NULL) {
	ERR << "NULL ResObject" << endl;
    }
    DBG << "transactResObject(" << *robj << ", " << (install?"install":"remove") << ")" << endl;

    transactCaps( _pool, robj->dep( Dep::RECOMMENDS ), install, true );
    return transactCaps( _pool, robj->dep( Dep::REQUIRES ), install, false );
}

///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

