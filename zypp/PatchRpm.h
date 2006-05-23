/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file        zypp/PatchRpm.h
 *
*/
#ifndef ZYPP_PATCHRPM_H
#define ZYPP_PATCHRPM_H

#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/BaseVersion.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class PatchRpm
  {
  public:
    PatchRpm(const Arch & arch,
             const Pathname & filename, 
             const ByteCount & downloadsize,
             const CheckSum & checksum,
             const Date & buildtime,
             const std::list<BaseVersion> & base_versions,
	     const unsigned media_nr)
    : _arch(arch)
    , _filename(filename)
    , _downloadsize(downloadsize)
    , _checksum(checksum)
    , _buildtime(buildtime)
    , _base_versions(base_versions)
    , _media_nr(media_nr)
    {}
    Arch arch() const { return _arch; }
    Pathname filename() const { return _filename; }
    ByteCount downloadsize() const { return _downloadsize; }
    CheckSum checksum() const { return _checksum; }
    Date buildtime() const { return _buildtime; }
    std::list<BaseVersion> baseVersions() const { return _base_versions; }
    unsigned mediaNr() const { return _media_nr; }
  private:
    Arch _arch;
    Pathname _filename;
    ByteCount _downloadsize;
    CheckSum _checksum;
    Date _buildtime;
    std::list<BaseVersion> _base_versions;
    unsigned _media_nr;
  };


} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATCHRPM_H
