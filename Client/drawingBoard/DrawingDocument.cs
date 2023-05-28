using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace drawingBoard
{
    internal class DrawingDocument : Document
    {
        public Drawing Drawing { get; set; }

        public DrawingDocument(string name) : base(name)
        {
            Drawing = new Drawing();
        }

        private byte[,] createBitmapFromDrawing()
        {
            byte[,] d = new byte[Drawing.Width, Drawing.Height / 8];
            for (int y = 0; y < Drawing.Height / 8; y++)
            {
                for (int x = 0; x < Drawing.Width; x++)
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
            return d;
        }

        private void createDrawingFromBitmap(byte[,] d)
        {
            for (int y = 0; y < Drawing.Height / 8; y++)
            {
                for (int x = 0; x < Drawing.Width; x++)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        if ((d[x, y] & (1 << i)) != 0)
                        {
                            Drawing.Pixels[x, y * 8 + i] = true;
                        }
                    }
                }
            }
        }

        public override void SaveDocument(string filePath)
        {
            byte[,] d = createBitmapFromDrawing();

            using (FileStream fs = new FileStream(filePath, FileMode.Create, FileAccess.ReadWrite))
            {
                BinaryWriter bw = new BinaryWriter(fs);
                for (int y = 0; y < Drawing.Height / 8; y++)
                {
                    for (int x = 0; x < Drawing.Width; x++)
                    {
                        bw.Write(d[x, y]);
                    }
                }
                bw.Close();
            }
        }

        public override void LoadDocument(string filePath)
        {
            byte[,] d = new byte[Drawing.Width, Drawing.Height / 8];

            using (FileStream fs = new FileStream(filePath, FileMode.Open, FileAccess.Read))
            {
                BinaryReader br = new BinaryReader(fs);
                for (int y = 0; y < Drawing.Height / 8; y++)
                {
                    for (int x = 0; x < Drawing.Width; x++)
                    {
                        d[x, y] = br.ReadByte();
                    }
                }
                br.Close();
            }
            createDrawingFromBitmap(d);
        }

        public override void ReceiveDocument(string COM)
        {
            byte[,] d = new byte[Drawing.Width, Drawing.Height / 8];
            using (SerialPort serialPort = new SerialPort(COM, 115200))
            {
                serialPort.Open();
                serialPort.ReadTimeout = serialPort.WriteTimeout = 1000;
                serialPort.Write("@SAVE");
                if (serialPort.ReadChar() != 'O' || serialPort.ReadChar() != 'K')
                {
                    serialPort.Close();
                    return;
                }

                for (int y = 0; y < Drawing.Height / 8; y++)
                {
                    for (int x = 0; x < Drawing.Width; x++)
                    {
                        var b = new byte[1];
                        serialPort.Read(b, 0, 1);
                        d[x, y] = b[0];
                    }
                }
            }

            createDrawingFromBitmap(d);
        }

        public override void SendDocument(string COM)
        {
            byte[,] d = createBitmapFromDrawing();
            using (SerialPort serialPort = new SerialPort(COM, 115200))
            {
                serialPort.Open();
                serialPort.ReadTimeout = serialPort.WriteTimeout = 1000;
                serialPort.Write("@LOAD");
                if (serialPort.ReadChar() != 'O' || serialPort.ReadChar() != 'K')
                {
                    serialPort.Close();
                    return;
                }

                for (int y = 0; y < Drawing.Height / 8; y++)
                {
                    for (int x = 0; x < Drawing.Width; x++)
                    {
                        var b = new byte[] { d[x, y] };
                        serialPort.Write(b, 0, 1);
                    }
                }
                serialPort.Close();
            }
        }
    }
}
