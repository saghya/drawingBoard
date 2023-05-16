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

        public bool[,] Pixels { get; set; }
        public Drawing()
        {
            Pixels = new bool[width, height];
            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    //Pixels[x, y] = (x % 2) == 0 && (y % 2) == 0? true : false;
                    Pixels[x, y] = x == 2*y || x == 0 || y == 0 || x == 127-2*y || x == 127 || y == 63 ? true : false;
                }
            }
        }
    }
}
