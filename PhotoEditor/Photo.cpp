#include "pch.h"
#include "photo.h"
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
    IAsyncOperation<BitmapImage> Photo::GetImageThumbnailAsync() const
    {
        // 获取略缩图
        const auto thumbnail = co_await image_file_.GetThumbnailAsync(FileProperties::ThumbnailMode::PicturesView);
        
        BitmapImage bitmap_image{};
        
        // 设置源为略缩图
        bitmap_image.SetSource(thumbnail);
        thumbnail.Close();
        
        co_return bitmap_image;
    }

    IAsyncOperation<BitmapImage> Photo::GetImageSourceAsync() const
    {
        // 创建流
        const IRandomAccessStream stream{ co_await ImageFile().OpenAsync(FileAccessMode::Read) };
        
        const BitmapImage bitmap{};
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
        wstringstream string_stream;

        string_stream << image_properties_.Width() << " x " << image_properties_.Height();
        const wstring str = string_stream.str();
        return static_cast<hstring>(str);
    }

    /// <summary>
    /// 获取图片标题
    /// </summary>
    /// <param name="value"></param>
    void Photo::ImageTitle(hstring const& value)
    {
        if (image_properties_.Title() != value)
        {
            image_properties_.Title(value);
            auto ignore_result = image_properties_.SavePropertiesAsync();
            raise_property_changed(L"ImageTitle");
        }
    }
}