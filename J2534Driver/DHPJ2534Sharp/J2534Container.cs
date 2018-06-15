using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHPJ2534Sharp
{
    /// <summary>
    /// Contains a static J2534 object. This class
    /// allows the unmanged CPP code to access the managed J2534 class
    /// </summary>
 public static class J2534Container
    {
        public static J2534 J2534Handle;
        public static void CreateJ5234Object()
        {
            J2534Handle = new J2534();
        }
    }
}
