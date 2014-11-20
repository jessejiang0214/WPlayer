using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace WPlayerUI
{
    /// <summary>
    /// 可独立使用或用于导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        private void VideoItemOnClick(object sender, RoutedEventArgs e)
        {
            var file = ((Button) sender).DataContext;
            Frame.Navigate(typeof (WatchingPage), file);
        }
    }
}
