#pragma once

#include "Photo.g.h"

namespace winrt::PhotoEditor::implementation
{
	struct Photo : PhotoT<Photo>
	{
		Photo() = default;

		Photo(Windows::Storage::FileProperties::ImageProperties const& props,
			Windows::Storage::StorageFile const& imageFile,
			hstring const& name,
			hstring const& type) :
			m_imageProperties(props),
			image_name_(name),
			image_file_type_(type),
			m_imageFile(imageFile)
		{
		}

		/// <summary>
		/// 异步获取图片略缩图
		/// </summary>
		/// <returns>略缩图</returns>
		Windows::Foundation::IAsyncOperation<Windows::UI::Xaml::Media::Imaging::BitmapImage> GetImageThumbnailAsync() const;

		/// <summary>
		/// 异步获取原图片
		/// </summary>
		/// <returns>图片源</returns>
		Windows::Foundation::IAsyncOperation<Windows::UI::Xaml::Media::Imaging::BitmapImage> GetImageSourceAsync() const;

		/// <summary>
		/// 图片文件属性
		/// </summary>
		/// <returns></returns>
		Windows::Storage::StorageFile ImageFile() const
		{
			return m_imageFile;
		}

		/// <summary>
		/// 图片信息属性
		/// </summary>
		/// <returns></returns>
		Windows::Storage::FileProperties::ImageProperties ImageProperties() const
		{
			return m_imageProperties;
		}

		/// <summary>
		/// 图片名称
		/// </summary>
		/// <returns></returns>
		hstring ImageName() const
		{
			return image_name_;
		}

		/// <summary>
		/// 图片文件类型
		/// </summary>
		/// <returns></returns>
		hstring ImageFileType() const
		{
			return image_file_type_;
		}

		/// <summary>
		/// 获取或者设置图片标题
		/// </summary>
		/// <returns></returns>
		hstring ImageTitle() const
		{
			return m_imageProperties.Title() == L"" ? image_name_ : m_imageProperties.Title();
		}

		void ImageTitle(hstring const& value);

		hstring ImageDimensions() const;

		/// <summary>
		/// 获取曝光度
		/// </summary>
		/// <returns>曝光度值</returns>
		float Exposure() const
		{
			return m_exposure;
		}

		/// <summary>
		/// 设置曝光度
		/// </summary>
		/// <param name="value">曝光度值</param>
		void Exposure(float value)
		{
			UpdateValue(L"Exposure", m_exposure, value);
		}


		/// <summary>
		/// 获取色温
		/// </summary>
		/// <returns>色温值</returns>
		float Temperature() const
		{
			return m_temperature;
		}

		/// <summary>
		/// 设置色温
		/// </summary>
		/// <param name="value">色温值</param>
		void Temperature(float value)
		{
			UpdateValue(L"Temperature", m_temperature, value);
		}

		/// <summary>
		/// 获取色调
		/// </summary>
		/// <returns>色调值</returns>
		float Tint() const
		{
			return m_tint;
		}

		/// <summary>
		/// 设置色调
		/// </summary>
		/// <param name="value">色调值</param>
		void Tint(float value)
		{
			UpdateValue(L"Tint", m_tint, value);
		}

		/// <summary>
		/// 获取对比度
		/// </summary>
		/// <returns>对比度值</returns>
		float Contrast() const
		{
			return m_contrast;
		}

		/// <summary>
		/// 设置对比度
		/// </summary>
		/// <param name="value">对比度值</param>
		void Contrast(float value)
		{
			UpdateValue(L"Contrast", m_contrast, value);
		}

		/// <summary>
		/// 获取饱和度
		/// </summary>
		/// <returns>饱和度值</returns>
		float Saturation() const
		{
			return m_saturation;
		}

		/// <summary>
		/// 设置饱和度
		/// </summary>
		/// <param name="value">饱和度值</param>
		void Saturation(float value)
		{
			UpdateValue(L"Saturation", m_saturation, value);
		}

		/// <summary>
		/// 获取模糊强度
		/// </summary>
		/// <returns>模糊强度值</returns>
		float BlurAmount() const
		{
			return m_blur;
		}

		/// <summary>
		/// 设置模糊强度
		/// </summary>
		/// <param name="value">模糊强度值</param>
		void BlurAmount(float value)
		{
			UpdateValue<float>(L"BlurAmount", m_blur, value);
		}

		/// <summary>
		/// 获取变旧的强度
		/// </summary>
		/// <returns>强度值</returns>
		float Intensity() const
		{
			return m_sepiaIntensity;
		}

		/// <summary>
		/// 设置变旧的强度
		/// </summary>
		/// <param name="value">强度值</param>
		void Intensity(float value)
		{
			UpdateValue<float>(L"Intensity", m_sepiaIntensity, value);
		}
		
		/// <summary>
		/// 属性更新通知
		/// </summary>
		/// <param name="value">事件句柄</param>
		/// <returns></returns>
		event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value)
		{
			return m_propertyChanged.add(value);
		}

		void PropertyChanged(event_token const& token)
		{
			m_propertyChanged.remove(token);
		}

	private:
		// 文件和信息字段
		Windows::Storage::FileProperties::ImageProperties m_imageProperties{ nullptr };
		Windows::Storage::StorageFile m_imageFile{ nullptr };
		hstring image_name_;
		hstring image_file_type_;
		hstring m_imageTitle;

		// 图片效果字段
		float m_exposure{ 0 };
		float m_temperature{ 0 };
		float m_tint{ 0 };
		float m_contrast{ 0 };
		float m_saturation{ 1 };
		float m_blur{ 0 };
		float m_sepiaIntensity{ .5f };

		// 首页上的图片标题大小字段

		double m_size{ 250 };

		// 属性更改通知
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

		template <class T>
		void UpdateValue(hstring const& propertyName, T& var, T value)
		{
			if (var != value)
			{
				var = value;
				RaisePropertyChanged(propertyName);
			}
		}

		void RaisePropertyChanged(hstring const& propertyName)
		{
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(propertyName));
		}
	};
}

namespace winrt::PhotoEditor::factory_implementation
{
	struct Photo : PhotoT<Photo, implementation::Photo>
	{
	};
}