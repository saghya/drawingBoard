using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

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

            while (String.IsNullOrWhiteSpace(form.DrawingName))
            {
                MessageBox.Show("Name cannot be empty");
                form.DrawingName = "";
                if (form.ShowDialog() != DialogResult.OK)
                    return;
            }

            DrawingDocument doc = new DrawingDocument(form.DrawingName);
            documents.Add(doc);
            createView(doc, true);
        }

        public void OpenDocument()
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "bin files (*.bin)|*.bin|All files (*.*)|*.*";
            openFileDialog.RestoreDirectory = true;
            string path = "";
            if (openFileDialog.ShowDialog() != DialogResult.OK)
                return;
            path = openFileDialog.FileName;
            DrawingDocument doc = new DrawingDocument(Path.GetFileName(path));
            documents.Add(doc);
            doc.LoadDocument(path);
            createView(doc, true);
        }

        public void CloseActiveView()
        {
            if (mainForm.TabControl.TabPages.Count == 0)
                return;

            Document docToClose = ActiveDocument;

            docToClose.DetachView(activeView);
            mainForm.TabControl.TabPages.Remove(getTabPageForView(activeView));
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
            string path = "";
            if (saveFileDialog.ShowDialog() != DialogResult.OK)
                return;
            path = saveFileDialog.FileName;
            ActiveDocument.SaveDocument(path);
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
