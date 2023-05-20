using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace drawingBoard
{
    public partial class SerialConnectForm : Form
    {
        public SerialConnectForm()
        {
            InitializeComponent();

            string[] ports = SerialPort.GetPortNames();
            cbPort.Items.Clear();

            foreach (string comport in ports)
                cbPort.Items.Add(comport);
        }

        public string SelectedPort()
        {
            return cbPort.SelectedItem.ToString();
        }

        private void cbPort_Validating(object sender, CancelEventArgs e)
        {
            if (cbPort.SelectedIndex == -1)
            {
                e.Cancel = true;
                cbPort.Focus();
                errorProvider1.SetError(cbPort, "Please select a COM port");
            }
            else
            {
                e.Cancel = false;
                errorProvider1.Clear();
                this.DialogResult = DialogResult.OK;
            }
        }

        private void cbPort_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                Validate();
        }
    }
}
