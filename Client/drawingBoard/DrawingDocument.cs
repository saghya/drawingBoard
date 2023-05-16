using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace drawingBoard
{
    internal class DrawingDocument : Document
    {
        public Drawing Drawing { get; set; }

        public DrawingDocument(string name) : base(name)
        {
            Drawing = new Drawing();
        }

        public override void SaveDocument(string filePath)
        {
        }

        public override void LoadDocument(string filePath)
        {
        }
    }
}
