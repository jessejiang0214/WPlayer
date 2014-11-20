using System.Linq;
using Windows.Storage;

namespace WPlayerUIShared.Base
{
    static class VideoStorageFileExtensions
    {
        private static readonly string[] VideoExt = { ".mp4", ".mkv", ".wmv", ".avi", ".mov", ".m4v", ".asf", ".wmv", ".3gp", ".rm", ".rmvb", ".flv" };

        public static bool IsVideoFile(this StorageFile file)
        {
            var type = file.FileType.ToLower();
            return VideoExt.Any(e => e.Equals(type));
        }
    }
}
