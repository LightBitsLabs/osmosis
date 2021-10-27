#ifndef __OSMOSIS_COMMON_UTILS_H__
#define __OSMOSIS_COMMON_UTILS_H__

#include <boost/filesystem.hpp>
#include "Osmosis/ObjectStore/DirectoryNames.h"

namespace Osmosis {
namespace CommonUtils {
    static void rename(const boost::filesystem::path & src, const boost::filesystem::path & dst)
    {
        try{
            boost::filesystem::rename( src, dst );
        }
        catch(...){
                //better not to log it as this exception is thrown for every
                //chunk during checkin phase (i.e tens-thousands logs per checkin)
                boost::filesystem::copy_file(src, dst);
                boost::filesystem::remove(src);
        }
    }
}
}
#endif