using System;
using System.Collections.Generic;
using System.Text;

namespace WPlayerUIShared.ViewModels
{
    class ViewModelLocator
    {
        private static readonly MainViewModel _mainViewModel = new MainViewModel();
        public MainViewModel MainViewModel { get { return _mainViewModel; } }
    }
}
