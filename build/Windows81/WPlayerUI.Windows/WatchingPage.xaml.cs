using System;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace WPlayerUI
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class WatchingPage : Page
    {
        public WatchingPage()
        {
            InitializeComponent();
        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            var file = e.Parameter as StorageFile;
            if (file == null) return;

            Player.SetSource(await file.OpenAsync(FileAccessMode.Read), string.Empty);
        }

        private void BackButtonOnClick(object sender, RoutedEventArgs e)
        {
            if (!Frame.CanGoBack) return;
            Frame.GoBack();
        }
    }
}
