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
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        public TabControl TabControl
        {
            get { return tcDrawings; }
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            App.Instance.NewDocument();
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            App.Instance.OpenDocument();
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            App.Instance.SaveActiveDocument();
        }

        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            App.Instance.CloseActiveView();
        }

        private void tcDrawings_SelectedIndexChanged(object sender, EventArgs e)
        {
            App.Instance.UpdateActiveView();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void sendToolStripMenuItem_Click(object sender, EventArgs e)
        {
            App.Instance.SendActiveDocument();
        }

        private void receiveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            App.Instance.ReceiveDocument();
        }
    }
}
