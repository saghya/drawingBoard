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
            byte[,] d = new byte[Drawing.Width, Drawing.Height/8];
            for (int x = 0; x < Drawing.Width; x++)
            {
                for (int y = 0; y < Drawing.Height/8; y++)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        if (Drawing.Pixels[x, y * 8 + i])
                        {
                            d[x, y] |= (byte)(1 << i);
                        }
                    }
                }
            }

            using (FileStream fs = new FileStream(filePath, FileMode.Create, FileAccess.ReadWrite))
            {
                BinaryWriter bw = new BinaryWriter(fs);
                for (int x = 0; x < Drawing.Width; x++)
                {
                    for (int y = 0; y < Drawing.Height/8; y++)
                    {
                        bw.Write(d[x, y]);
                    }
                }
                bw.Close();
            }
        }

        public override void LoadDocument(string filePath)
        {
            byte[,] d = new byte[Drawing.Width, Drawing.Height/8];

            using (FileStream fs = new FileStream(filePath, FileMode.Open, FileAccess.Read))
            {
                BinaryReader br = new BinaryReader(fs);
                for (int x = 0; x < Drawing.Width; x++)
                {
                    for (int y = 0; y < Drawing.Height/8; y++)
                    {
                        d[x, y] = br.ReadByte();
                    }
                }
                br.Close();
            }

            for (int x = 0; x < Drawing.Width; x++)
            {
                for (int y = 0; y < Drawing.Height/8; y++)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        if ((d[x,y] & (1 << i)) != 0)
                        {
                            Drawing.Pixels[x, y * 8 + i] = true;
                        }
                    }
                }
            }
        }
    }
}
