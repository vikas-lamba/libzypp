/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoObsoletes.cc
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

#include <map>
#include <sstream>

#include "zypp/solver/detail/ResolverInfo.h"
#include "zypp/solver/detail/ResolverInfoObsoletes.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

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
      
      IMPL_PTR_TYPE(ResolverInfoObsoletes);
      
      //---------------------------------------------------------------------------
      
      ostream&
      operator<<( ostream& os, const ResolverInfoObsoletes & obsoletes)
      {
	  const ResolverInfo & info = obsoletes;
	  ostringstream info_str;
	  info_str << info;

	  // Translator: all.%s = name of package,patch,....
	  os << str::form (_("%s is replaced by %s"),
			   info_str.str().c_str(),
			   obsoletes.itemsToString(false).c_str());
	  return os;
      }
      
      //---------------------------------------------------------------------------
      
      ResolverInfoObsoletes::ResolverInfoObsoletes (PoolItem_Ref item, PoolItem_Ref obsoletes)
          : ResolverInfoContainer (RESOLVER_INFO_TYPE_OBSOLETES, item, RESOLVER_INFO_PRIORITY_USER, obsoletes)
      {
      }
      
      
      ResolverInfoObsoletes::~ResolverInfoObsoletes ()
      {
      }
      
      //---------------------------------------------------------------------------
      
      ResolverInfo_Ptr
      ResolverInfoObsoletes::copy (void) const
      {
	  // leave the second item empty, it will be copied with the container below

          ResolverInfoObsoletes_Ptr cpy = new ResolverInfoObsoletes(affected(), PoolItem_Ref());
      
          ((ResolverInfoContainer_Ptr)cpy)->copy (this);
      
          return cpy;
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

