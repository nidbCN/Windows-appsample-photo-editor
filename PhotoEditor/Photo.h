#pragma once

#include "Photo.g.h"

namespace winrt::PhotoEditor::implementation
{
	struct Photo : PhotoT<Photo>
	{
		Photo() = default;

		Photo(
			Windows::Storage::FileProperties::ImageProperties const& props,
			Windows::Storage::StorageFile const& image_file,
			hstring const& name,
			hstring const& type
		) :
			image_properties_(props),
			image_name_(name),
			image_file_type_(type),
			image_file_(image_file)
		{
		}

		/// <summary>
		/// 异步获取图片略缩图
		/// </summary>
		/// <returns>略缩图</returns>
		Windows::Foundation::IAsyncOperation<Windows::UI::Xaml::Media::Imaging::BitmapImage> [[nodiscard]] GetImageThumbnailAsync() const;

		/// <summary>
		/// 异步获取原图片
		/// </summary>
		/// <returns>图片源</returns>
		Windows::Foundation::IAsyncOperation<Windows::UI::Xaml::Media::Imaging::BitmapImage> [[nodiscard]] GetImageSourceAsync() const;

		/// <summary>
		/// 图片文件属性
		/// </summary>
		/// <returns></returns>
		Windows::Storage::StorageFile [[nodiscard]] ImageFile() const
		{
			return image_file_;
		}

		/// <summary>
		/// 图片信息属性
		/// </summary>
		/// <returns></returns>
		Windows::Storage::FileProperties::ImageProperties [[nodiscard]] ImageProperties() const
		{
			return image_properties_;
		}

		/// <summary>
		/// 图片名称
		/// </summary>
		/// <returns></returns>
		hstring [[nodiscard]] ImageName() const
		{
			return image_name_;
		}

		/// <summary>
		/// 图片文件类型
		/// </summary>
		/// <returns></returns>
		hstring [[nodiscard]] ImageFileType() const
		{
			return image_file_type_;
		}

		/// <summary>
		/// 获取或者设置图片标题
		/// </summary>
		/// <returns></returns>
		hstring [[nodiscard]] ImageTitle() const
		{
			return image_properties_.Title() == L"" ? image_name_ : image_properties_.Title();
		}

		void ImageTitle(hstring const& value);

		hstring [[nodiscard]] ImageDimensions() const;

		/// <summary>
		/// 获取曝光度
		/// </summary>
		/// <returns>曝光度值</returns>
		float [[nodiscard]] Exposure() const
		{
			return exposure_;
		}

		/// <summary>
		/// 设置曝光度
		/// </summary>
		/// <param name="value">曝光度值</param>
		void Exposure(float value)
		{
			update_value(L"Exposure", exposure_, value);
		}


		/// <summary>
		/// 获取色温
		/// </summary>
		/// <returns>色温值</returns>
		float [[nodiscard]] Temperature() const
		{
			return temperature_;
		}

		/// <summary>
		/// 设置色温
		/// </summary>
		/// <param name="value">色温值</param>
		void Temperature(float value)
		{
			update_value(L"Temperature", temperature_, value);
		}

		/// <summary>
		/// 获取色调
		/// </summary>
		/// <returns>色调值</returns>
		float [[nodiscard]] Tint() const
		{
			return tint_;
		}

		/// <summary>
		/// 设置色调
		/// </summary>
		/// <param name="value">色调值</param>
		void Tint(float value)
		{
			update_value(L"Tint", tint_, value);
		}

		/// <summary>
		/// 获取对比度
		/// </summary>
		/// <returns>对比度值</returns>
		float [[nodiscard]] Contrast() const
		{
			return contrast_;
		}

		/// <summary>
		/// 设置对比度
		/// </summary>
		/// <param name="value">对比度值</param>
		void Contrast(float value)
		{
			update_value(L"Contrast", contrast_, value);
		}

		/// <summary>
		/// 获取饱和度
		/// </summary>
		/// <returns>饱和度值</returns>
		float [[nodiscard]] Saturation() const
		{
			return saturation_;
		}

		/// <summary>
		/// 设置饱和度
		/// </summary>
		/// <param name="value">饱和度值</param>
		void Saturation(float value)
		{
			update_value(L"Saturation", saturation_, value);
		}

		/// <summary>
		/// 获取模糊强度
		/// </summary>
		/// <returns>模糊强度值</returns>
		float [[nodiscard]] BlurAmount() const
		{
			return blur_;
		}

		/// <summary>
		/// 设置模糊强度
		/// </summary>
		/// <param name="value">模糊强度值</param>
		void BlurAmount(float value)
		{
			update_value<float>(L"BlurAmount", blur_, value);
		}

		/// <summary>
		/// 获取变旧的强度
		/// </summary>
		/// <returns>强度值</returns>
		float [[nodiscard]] Intensity() const
		{
			return sepia_intensity_;
		}

		/// <summary>
		/// 设置变旧的强度
		/// </summary>
		/// <param name="value">强度值</param>
		void Intensity(float const value)
		{
			update_value<float>(L"Intensity", sepia_intensity_, value);
		}

		/// <summary>
		/// 属性更新通知
		/// </summary>
		/// <param name="value">事件句柄</param>
		/// <returns></returns>
		event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value)
		{
			return property_changed_.add(value);
		}

		void PropertyChanged(event_token const& token)
		{
			property_changed_.remove(token);
		}

	private:
		// 文件和信息字段
		Windows::Storage::FileProperties::ImageProperties image_properties_{ nullptr };
		Windows::Storage::StorageFile image_file_{ nullptr };
		hstring image_name_;
		hstring image_file_type_;
		hstring image_title_;

		// 图片效果字段
		float exposure_{ 0 };
		float temperature_{ 0 };
		float tint_{ 0 };
		float contrast_{ 0 };
		float saturation_{ 1 };
		float blur_{ 0 };
		float sepia_intensity_{ .5f };

		// 首页上的图片标题大小字段

		double size_{ 250 };

		// 属性更改通知
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> property_changed_;

		template <class T>
		void update_value(hstring const& property_name, T& var, T value)
		{
			if (var != value)
			{
				var = value;
				raise_property_changed(property_name);
			}
		}

		void raise_property_changed(hstring const& property_name)
		{
			property_changed_(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(property_name));
		}
	};
}

namespace winrt::PhotoEditor::factory_implementation
{
	struct Photo final : PhotoT<Photo, implementation::Photo>
	{
	};
}