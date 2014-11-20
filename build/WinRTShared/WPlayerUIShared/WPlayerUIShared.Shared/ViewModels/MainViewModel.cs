using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Input;
using Windows.Storage;
using GalaSoft.MvvmLight;
using WPlayerUI.Utils;
using WPlayerUIShared.Base;

namespace WPlayerUIShared.ViewModels
{
    class MainViewModel : ViewModelBase
    {
        public MainViewModel()
        {
            FolderVideos = new ObservableCollection<StorageFile>();
            UserVideoFolders = new ObservableCollection<StorageFolder> { KnownFolders.VideosLibrary };

            OpenFolderCommand = new ActionCommand<StorageFolder>(ExecuteOpenFolder);
        }

        private async void ExecuteOpenFolder(StorageFolder folder)
        {
            var files = await folder.GetFilesAsync();

            foreach (var video in files.Where(f => f.IsVideoFile()))
            {
                FolderVideos.Add(video);
            }
        }

        public ObservableCollection<StorageFolder> UserVideoFolders { get; private set; }
        public ObservableCollection<StorageFile> FolderVideos { get; private set; }

        public ICommand OpenFolderCommand { get; private set; }
    }
}
