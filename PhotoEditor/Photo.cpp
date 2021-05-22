#include "pch.h"
#include "Photo.h"
#include <sstream>

using namespace winrt;
using namespace std;
using namespace Windows::UI::Xaml;
using namespace Windows::Storage;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage::Streams;

namespace winrt::PhotoEditor::implementation
{
    /// <summary>
    /// 异步获取图片略缩图
    /// </summary>
    /// <returns>略缩图</returns>
    IAsyncOperation<BitmapImage> Photo::GetImageThumbnailAsync() const
    {
        const auto thumbnail = co_await m_imageFile.GetThumbnailAsync(FileProperties::ThumbnailMode::PicturesView);
        
        BitmapImage bitmapImage{};
        
        // 设置源为略缩图
        bitmapImage.SetSource(thumbnail);
        thumbnail.Close();
        
        co_return bitmapImage;
    }

    /// <summary>
    /// 异步获取图片源
    /// </summary>
    /// <returns>图片源</returns>
    IAsyncOperation<BitmapImage> Photo::GetImageSourceAsync() const
    {
        // 创建流
        IRandomAccessStream stream{ co_await ImageFile().OpenAsync(FileAccessMode::Read) };
        
        BitmapImage bitmap{};
        bitmap.SetSource(stream);
        
        co_return bitmap;
    }

    /// <summary>
    /// 获取图像大小
    /// </summary>
    /// <returns></returns>
    hstring Photo::ImageDimensions() const
    {
        // 创建字符串流
        wstringstream stringStream;

        stringStream << m_imageProperties.Width() << " x " << m_imageProperties.Height();
        wstring str = stringStream.str();
        return static_cast<hstring>(str);
    }

    /// <summary>
    /// 获取图片标题
    /// </summary>
    /// <param name="value"></param>
    void Photo::ImageTitle(hstring const& value)
    {
        if (m_imageProperties.Title() != value)
        {
            m_imageProperties.Title(value);
            auto ignoreResult = m_imageProperties.SavePropertiesAsync();
            RaisePropertyChanged(L"ImageTitle");
        }
    }
}