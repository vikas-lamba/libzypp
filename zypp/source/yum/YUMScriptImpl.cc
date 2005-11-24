/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMScriptImpl.cc
*/

#include "zypp/source/yum/YUMScriptImpl.h"
#include "zypp/Arch.h"
#include "zypp/Edition.h"
#include <zypp/detail/ResObjectFactory.h>


using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace yum
    {

      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : YUMScriptImpl
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
      */
      YUMScriptImpl::YUMScriptImpl(
	const zypp::parser::yum::YUMPatchScript & parsed
      )
      {
	_do_script = parsed.do_script;
	_undo_script = parsed.undo_script;
      }
      std::string YUMScriptImpl::do_script() const {
	return _do_script;
      }
      /** Get the script to undo the change */
      std::string YUMScriptImpl::undo_script() const {
	return _undo_script;
      }
      /** Check whether script to undo the change is available */
      bool YUMScriptImpl::undo_available() const {
	return _undo_script != "";
      }
      Label YUMScriptImpl::summary() const
      { return ResObjectImplIf::summary(); }

      Text YUMScriptImpl::description() const
      { return ResObjectImplIf::description(); }

      Text YUMScriptImpl::insnotify() const
      { return ResObjectImplIf::insnotify(); }

      Text YUMScriptImpl::delnotify() const
      { return ResObjectImplIf::delnotify(); }

      FSize YUMScriptImpl::size() const
      { return ResObjectImplIf::size(); }

      bool YUMScriptImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label YUMScriptImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }

      Vendor YUMScriptImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }

    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
