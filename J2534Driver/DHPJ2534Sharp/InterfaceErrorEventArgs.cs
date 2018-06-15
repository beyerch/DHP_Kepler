using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHPJ2534Sharp
{
   public class InterfaceErrorEventArgs : EventArgs
    {
        public KeplerCommand ErrorMessage { get; set; } 
        public InterfaceErrorEventArgs(KeplerCommand ErrorMessage)
        {
            this.ErrorMessage = ErrorMessage;
        }
    }
}
