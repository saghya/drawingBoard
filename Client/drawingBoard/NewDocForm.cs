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
    public partial class NewDocForm : Form
    {
        public NewDocForm()
        {
            InitializeComponent();
        }

        public string DrawingName
        {
            get { return tbDrawingName.Text; }
            set { tbDrawingName.Text = value; }
        }

        private void tbNewDocForm_Validating(object sender, CancelEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(tbDrawingName.Text))
            {
                e.Cancel = true;
                tbDrawingName.Focus();
                errorProvider.SetError(tbDrawingName, "Name cannot be empty!");
            }
            else
            {
                e.Cancel = false;
                errorProvider.Clear();
                this.DialogResult = DialogResult.OK;
            }
        }

        private void tbNewDocForm_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                Validate();
        }
    }
}
