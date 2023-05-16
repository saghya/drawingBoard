using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

namespace drawingBoard
{
    internal class Drawing
    {
        const int width = 128;
        const int height = 64;
        public int Width { get { return width; } }
        public int Height { get { return height; } }

        public bool[,] Pixels { get; set; } = new bool[width, height];
    }
}
