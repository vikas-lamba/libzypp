/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PackageImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_PACKAGEIMPLIF_H
#define ZYPP_DETAIL_PACKAGEIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Package;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PackageImplIf
    //
    /** Abstact Package implementation interface.
    */
    class PackageImplIf : public ResObjectImplIf
    {
    public:
      typedef Package ResType;
#if 0
      class CheckSum
      {
      public:
        CheckSum(std::string type, bool pkgid, std::string checksum) {
          _type = type; _pkgid = pkgid; _checksum = checksum;
        }
        std::string type() { return _type; }
        bool pkgid() { return _pkgid; }
        std::string checksum() { return _checksum; }
      private:
        std::string _type;
        bool _pkgid;
        std::string _checksum;
      };
      /**
      * @short Holds data about how much space will be needed per directory
      **/
      class DirSize {
      public:
        DirSize();
        DirSize(const std::string& path,
                const std::string& size,
                const std::string& fileCount)
	: path(path), sizeKByte(size), fileCount(fileCount)
	{}
        const std::string path;
        const std::string sizeKByte;
        const std::string fileCount;
      };
    
      /**
       * @short Holds Data about file and file type
       *  (directory, plain)
       **/
      class FileData {
      public:
        std::string name;
        std::string type;
        FileData();
        FileData(const std::string &name,
                 const std::string &type)
	: name(name), type(type)
	{}
      };
#endif

    public:
      /** \name Rpm Package Attributes. */
      //@{
      /** */
      virtual Date buildtime() const PURE_VIRTUAL;
      /** */
      virtual std::string buildhost() const PURE_VIRTUAL;
      /** */
      virtual Date installtime() const PURE_VIRTUAL;
      /** */
      virtual std::string distribution() const PURE_VIRTUAL;
      /** */
      virtual Vendor vendor() const PURE_VIRTUAL;
      /** */
      virtual Label license() const PURE_VIRTUAL;
      /** */
      virtual std::string packager() const PURE_VIRTUAL;
      /** */
      virtual PackageGroup group() const PURE_VIRTUAL;
      /** */
      virtual Text changelog() const PURE_VIRTUAL;
      /** Don't ship it as class Url, because it might be
       * in fact anything but a legal Url. */
      virtual std::string url() const PURE_VIRTUAL;
      /** */
      virtual std::string os() const PURE_VIRTUAL;
      /** */
      virtual Text prein() const PURE_VIRTUAL;
      /** */
      virtual Text postin() const PURE_VIRTUAL;
      /** */
      virtual Text preun() const PURE_VIRTUAL;
      /** */
      virtual Text postun() const PURE_VIRTUAL;
      /** */
      virtual FSize sourcesize() const PURE_VIRTUAL;
      /** */
      virtual FSize archivesize() const PURE_VIRTUAL;
      /** */
      virtual Text authors() const PURE_VIRTUAL;
      /** */
      virtual Text filenames() const PURE_VIRTUAL;
      //@}

      /** \name Additional Package Attributes.
       * \todo review what's actually needed here. Maybe worth grouping
       * all the package rertieval related stuff in a class. Easier to ship
       * and handle it.
      */
      //@{
      /** */
      virtual License licenseToConfirm() const PURE_VIRTUAL;
#if 0
      /** */
      virtual std::string sourceloc() const PURE_VIRTUAL;
      /** */
      virtual void du( PkgDu & dudata_r ) const PURE_VIRTUAL;
      /** */
      virtual std::string location() const PURE_VIRTUAL;
      /** */
      virtual unsigned int medianr() const PURE_VIRTUAL;
      /** */
      virtual PackageKeywords keywords() const PURE_VIRTUAL;
      /** */
      virtual std::string md5sum() const PURE_VIRTUAL;
      /** */
      virtual std::string externalUrl() const PURE_VIRTUAL;
      /** */
      virtual std::list<Edition> patchRpmBaseVersions() const PURE_VIRTUAL;
      /** */
      virtual FSize patchRpmSize() const PURE_VIRTUAL;
      /** */
      virtual bool forceInstall() const PURE_VIRTUAL;
      /** */
      virtual std::string patchRpmMD5() const PURE_VIRTUAL;
      /** */
      virtual bool isRemote() const PURE_VIRTUAL;
      /** */
      virtual PMError providePkgToInstall( Pathname& path_r ) const PURE_VIRTUAL;
      /** */
      virtual PMError provideSrcPkgToInstall( Pathname& path_r ) const PURE_VIRTUAL;
      /** */
      virtual constInstSrcPtr source() const PURE_VIRTUAL;
      /** */
      virtual std::list<PMPackageDelta> deltas() const PURE_VIRTUAL;
#endif
      //@}
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PACKAGEIMPLIF_H
