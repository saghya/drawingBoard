using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace drawingBoard
{
    public abstract class Document
    {
        private string name;
        List<IView> views = new List<IView>();

        public string Name
        {
            get { return name; }
        }

        public Document(string name)
        {
            this.name = name;
        }

        public void AttachView(IView v)
        {
            views.Add(v);
            v.Update();
        }

        public void DetachView(IView v)
        {
            views.Remove(v);
        }

        public bool HasAnyView()
        {
            return views.Count > 0;
        }

        protected void UpdateAllViews()
        {
            foreach (IView view in views)
                view.Update();
        }

        public virtual void LoadDocument(string filePath) { }

        public virtual void SaveDocument(string filePath) { }

        public virtual void ReceiveDocument(string COM) { }

        public virtual void SendDocument(string COM) { }
    }
}
