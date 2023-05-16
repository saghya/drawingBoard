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

        //private void DrawingView_MouseWheel(object sender, MouseEventArgs e)
        //{
        //    if (e.Delta > 0)
        //    {
        //        this.scale *= 1.2f;
        //    }
        //    else
        //    {
        //        this.scale /= 1.2f;
        //    }
        //    Invalidate();
        //}

        public DrawingView(Document document)
        {
            InitializeComponent();
            this.document = (DrawingDocument)document;
        }

        public void Update()
        {
            Invalidate();
        }
        
        public Document GetDocument()
        {
            return document;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            DoubleBuffered = true;
            //const int offset = 10;
            scale = ((float)ClientSize.Width - 2*offset) / document.Drawing.Width;

            // background
            e.Graphics.FillRectangle(Brushes.DodgerBlue, 0, 0, ClientSize.Width, ClientSize.Height);

            // outer lines
            e.Graphics.DrawRectangle(Pens.Black, offset - 1, offset - 1, scale * document.Drawing.Width + 1, scale * document.Drawing.Height + 1);

            // pixels
            for (int x = 0; x < document.Drawing.Width; x++)
            {
                for (int y = 0; y < document.Drawing.Height; y++)
                {
                    if (document.Drawing.Pixels[x,y])
                    {
                        e.Graphics.FillRectangle(Brushes.White, offset + x*scale, offset + y*scale, scale, scale);
                    }
                }

            }

        }

        private void DrawingView_MouseMove(object sender, MouseEventArgs e)
        {
            int x = (int)((e.X - offset) / scale);
            int y = (int)((e.Y - offset)/ scale);

            if (x < 0 || y < 0 || x > document.Drawing.Width - 1 || y > document.Drawing.Height - 1)
                return;

            if (draw)
                document.Drawing.Pixels[x, y] = true;
            else if (erase)
                document.Drawing.Pixels[x, y] = false;

            Invalidate();
        }

        private void DrawingView_MouseDown(object sender, MouseEventArgs e)
        {
            int x = (int)((e.X - offset) / scale);
            int y = (int)((e.Y - offset)/ scale);
            switch (e.Button)
            {
                case MouseButtons.Left:
                    draw = true;
                    document.Drawing.Pixels[x, y] = true;
                    break;
                case MouseButtons.Right:
                    erase = true;
                    document.Drawing.Pixels[x, y] = false;
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
