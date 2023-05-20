using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace drawingBoard
{
    public partial class DrawingView : UserControl, IView
    {
        private DrawingDocument document;
        private int offset = 10;
        private float scale = 5;
        bool draw = false;
        bool erase = false;

        public DrawingView()
        {
            InitializeComponent();
        }

        public DrawingView(Document document)
        {
            InitializeComponent();
            this.document = (DrawingDocument)document;
        }

        public Document GetDocument()
        {
            return document;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            DoubleBuffered = true;
            scale = ((float)ClientSize.Width - 2 * offset) / document.Drawing.Width;

            // background
            e.Graphics.FillRectangle(Brushes.DodgerBlue, 0, 0, ClientSize.Width, ClientSize.Height);

            // outer lines
            e.Graphics.DrawRectangle(Pens.Black, offset - 1, offset - 1, scale * document.Drawing.Width + 1,
                                     scale * document.Drawing.Height + 1);

            // pixels
            for (int x = 0; x < document.Drawing.Width; x++)
            {
                for (int y = 0; y < document.Drawing.Height; y++)
                {
                    if (document.Drawing.Pixels[x, y])
                    {
                        e.Graphics.FillRectangle(Brushes.White, offset + x * scale, offset + y * scale, scale, scale);
                    }
                }
            }
        }

        private void setPixel(bool value, MouseEventArgs e)
        {
            int x = (int)((e.X - offset) / scale);
            int y = (int)((e.Y - offset) / scale);
            if (x < 0 || y < 0 || x > document.Drawing.Width - 1 || y > document.Drawing.Height - 1)
                return;
            document.Drawing.Pixels[x, y] = value;
        }

        private void DrawingView_MouseMove(object sender, MouseEventArgs e)
        {
            if (draw)
                setPixel(true, e);
            else if (erase)
                setPixel(false, e);

            Invalidate();
        }

        private void DrawingView_MouseDown(object sender, MouseEventArgs e)
        {
            switch (e.Button)
            {
                case MouseButtons.Left:
                    draw = true;
                    setPixel(true, e);
                    break;
                case MouseButtons.Right:
                    erase = true;
                    setPixel(false, e);
                    break;
            }
            Invalidate();
        }

        private void DrawingView_MouseUp(object sender, MouseEventArgs e)
        {
            draw = erase = false;
        }

        private void DrawingView_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.C)
            {
                for (int x = 0; x < document.Drawing.Width; x++)
                {
                    for (int y = 0; y < document.Drawing.Height; y++)
                    {
                        document.Drawing.Pixels[x, y] = false;
                    }
                }
            }
            Invalidate();
        }
    }
}
