using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace drawingBoard
{
    public interface IView
    {
        void Update();

        Document GetDocument();
    }
}
