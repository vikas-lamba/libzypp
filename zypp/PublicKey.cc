/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PublicKey.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/base/String.h"
#include "zypp/PublicKey.h"
#include "zypp/ExternalProgram.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Logger.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////    
  //
  //	CLASS NAME : PublicKey::Impl
  //
  /** PublicKey implementation. */
  struct PublicKey::Impl
  {
    Impl()
    {}

    Impl(const Pathname &file)
    {
      readFromFile(file);
      MIL << "Done reading key" << std::endl;
    }
    
    public:
      /** Offer default Impl. */
      static shared_ptr<Impl> nullimpl()
      {
        static shared_ptr<Impl> _nullimpl( new Impl );
        return _nullimpl;
      }

     
    std::string asString() const
    {
      return "[" + id() + "] [" + name() + "] [" + fingerprint() + "]";
    }
    
    std::string armoredData() const
    { return _data; }
    
    std::string id() const
    { return _id; }
    
    std::string name() const
    { return _name; }
    
    std::string fingerprint() const
    { return _fingerprint; }
    
    Pathname path() const
    { 
      return _data_file.path();
      //return _data_file;
    }
    
    protected:
      
     void readFromFile( const Pathname &keyfile)
     {
       PathInfo info(keyfile);
       MIL << "Reading pubkey from " << keyfile << " of size " << info.size() << " and sha1 " << filesystem::checksum(keyfile, "sha1")<< endl; 
       if ( !info.isExist() )
         ZYPP_THROW(Exception("Can't read public key from " + keyfile.asString() + ", file not found"));
         
       if ( copy( keyfile, _data_file.path() ) != 0 )
         ZYPP_THROW(Exception("Can't copy public key data from " + keyfile.asString() + " to " +  _data_file.path().asString() ));

       
       filesystem::TmpDir dir;
  
        const char* argv[] =
        {
          "gpg",
          "--no-default-keyring",
          "--homedir",
          dir.path().asString().c_str(),
          "--with-fingerprint",
          "--with-colons",
          "--quiet",
          "--no-tty",
          "--no-greeting",
          "--batch",
          "--status-fd",
          "1",
          _data_file.path().asString().c_str(),
          NULL
        };
  
        ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
  
        std::string line;
        int count = 0;
  
        str::regex rxColons("^([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):\n$");
  
      // pub:-:1024:17:A84EDAE89C800ACA:2000-10-19:2008-06-21::-:SuSE Package Signing Key <build@suse.de>:
  
        for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
        {
        //MIL << "[" << line << "]" << std::endl;
          str::smatch what;
          if(str::regex_match(line, what, rxColons, str::match_extra))
          {
            if ( what[1] == "pub" )
            {
              _id = what[5];
              _name = what[10];
            //return key;
            }
            else if ( what[1] == "fpr" )
            {
                _fingerprint = what[10];
            }
          //dumpRegexpResults(what);
          }
        }
        prog.close();
        
        if (_id.size() == 0 )
          ZYPP_THROW(BadKeyException("File " + keyfile.asString() + " doesn't contain public key data" , keyfile));
     }
    
  private:
    std::string _id;
    std::string _name;
    std::string _fingerprint;
    std::string _data;
    filesystem::TmpFile _data_file;
    //Pathname _data_file;
  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PublicKey::PublicKey
  //	METHOD TYPE : Ctor
  //
  PublicKey::PublicKey()
  : _pimpl( Impl::nullimpl() )
  {}

  PublicKey::PublicKey( const Pathname &file )
  : _pimpl( new Impl(file) )
  {}
  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PublicKey::~PublicKey
  //	METHOD TYPE : Dtor
  //
  PublicKey::~PublicKey()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to implementation:
  //
  ///////////////////////////////////////////////////////////////////

  std::string PublicKey::asString() const
  {
    return _pimpl->asString();
  }
  
  std::string PublicKey::armoredData() const
  { return _pimpl->armoredData(); }
    
  std::string PublicKey::id() const
  { return _pimpl->id(); }
    
  std::string PublicKey::name() const
  { return _pimpl->name(); }
    
  std::string PublicKey::fingerprint() const
  { return _pimpl->fingerprint(); }
  
  Pathname PublicKey::path() const
  { return _pimpl->path(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
