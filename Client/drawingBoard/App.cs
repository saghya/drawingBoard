using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.IO.Ports;

namespace drawingBoard
{
    public class App
    {
        private IView activeView;
        private List<Document> documents = new List<Document>();
        private MainForm mainForm;
        private static App theApp;

        public static App Instance
        {
            get { return theApp; }
        }

        public static void Initialize(MainForm form)
        {
            theApp = new App();
            theApp.mainForm = form;
        }

        public MainForm MainForm
        {
            get { return mainForm; }
        }

        public Document ActiveDocument
        {
            get
            {
                if (activeView == null)
                    return null;

                return activeView.GetDocument();
            }
        }

        public void NewDocument()
        {
            NewDocForm form = new NewDocForm();
            if (form.ShowDialog() != DialogResult.OK)
                return;

            DrawingDocument doc = new DrawingDocument(form.DrawingName);
            documents.Add(doc);
            createView(doc, true);
        }

        public void OpenDocument()
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "bin files (*.bin)|*.bin|All files (*.*)|*.*";
            openFileDialog.RestoreDirectory = true;
            if (openFileDialog.ShowDialog() != DialogResult.OK)
                return;
            string path = openFileDialog.FileName;
            DrawingDocument doc = new DrawingDocument(Path.GetFileName(path));

            try
            {
                documents.Add(doc);
                doc.LoadDocument(path);
                createView(doc, true);
            }
            catch
            {
                MessageBox.Show("Error opening drawing.", "Document", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public void CloseActiveView()
        {
            if (mainForm.TabControl.TabPages.Count == 0)
                return;

            Document docToClose = ActiveDocument;

            docToClose.DetachView(activeView);
            try
            {
                mainForm.TabControl.TabPages.Remove(getTabPageForView(activeView));
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString());
            }
            if (!docToClose.HasAnyView())
                documents.Remove(docToClose);
        }

        public void SaveActiveDocument()
        {
            if (ActiveDocument == null)
                return;

            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.RestoreDirectory = true;
            saveFileDialog.Filter = "bin files (*.bin)|*.bin|All files (*.*)|*.*";
            saveFileDialog.FileName = ActiveDocument.Name;
            if (saveFileDialog.ShowDialog() != DialogResult.OK)
                return;

            try
            {
                ActiveDocument.SaveDocument(saveFileDialog.FileName);
            }
            catch
            {
                MessageBox.Show("Error saving drawing.", "Document", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public void SendActiveDocument()
        {
            SerialConnectForm serialConnectForm = new SerialConnectForm();
            if (ActiveDocument == null)
            {
                MessageBox.Show("Open or create a drawing before sending.", "Document", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            else if (serialConnectForm.ShowDialog() != DialogResult.OK)
                return;

            try
            {
                ActiveDocument.SendDocument(serialConnectForm.SelectedPort());
            }
            catch
            {
                MessageBox.Show("Connection lost.", "Serial", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public void ReceiveDocument()
        {
            SerialConnectForm serialConnectForm = new SerialConnectForm();
            NewDocForm form = new NewDocForm();

            if (serialConnectForm.ShowDialog() != DialogResult.OK ||
                form.ShowDialog() != DialogResult.OK)
                return;

            try
            {
                DrawingDocument doc = new DrawingDocument(form.DrawingName);
                doc.ReceiveDocument(serialConnectForm.SelectedPort());
                documents.Add(doc);
                createView(doc, true);
            }
            catch
            {
                MessageBox.Show("Connection lost.", "Serial", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public void UpdateActiveView()
        {
            if (mainForm.TabControl.TabPages.Count == 0)
                activeView = null;
            else
                activeView = (IView)mainForm.TabControl.SelectedTab.Tag;
        }

        IView createView(Document doc, bool activateView)
        {
            TabPage tp = new TabPage(doc.Name);
            mainForm.TabControl.TabPages.Add(tp);
            DrawingView view = new DrawingView(doc);
            tp.Controls.Add(view);
            tp.Tag = view;
            view.Dock = DockStyle.Fill;

            doc.AttachView(view);

            if (activateView)
            {
                mainForm.TabControl.SelectTab(tp);
                activeView = view;
            }
            return view;
        }

        TabPage getTabPageForView(IView view)
        {
            foreach (TabPage page in mainForm.TabControl.TabPages)
                if (page.Tag == activeView)
                    return page;
            throw new Exception("Page for view not found.");
        }
    }
}
