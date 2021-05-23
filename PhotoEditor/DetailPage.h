#pragma once
#include "DetailPage.g.h"
#include <variant>

namespace winrt::PhotoEditor::implementation
{
	struct DetailPage : DetailPageT<DetailPage>, std::enable_shared_from_this<DetailPage>
	{
		DetailPage();

		/// <summary>
		/// 获取照片对象
		/// </summary>
		/// <returns>照片对象</returns>
		PhotoEditor::Photo Item() const
		{
			return item_;
		}

		/// <summary>
		/// 设置照片对象
		/// </summary>
		/// <param name="value"></param>
		void Item(PhotoEditor::Photo const& value)
		{
			item_ = value;
		}

		/// <summary>
		/// 缩放图片以适应屏幕
		/// </summary>
		void FitToScreen();

		/// <summary>
		/// 缩放图片到实际大小
		/// </summary>
		void ShowActualSize();

		/// <summary>
		/// 更新图片缩放
		/// </summary>
		void UpdateZoomState();

		/// <summary>
		/// 更新效果笔刷
		/// </summary>
		/// <param name=""></param>
		void UpdateEffectBrush(hstring const&);

		/// <summary>
		/// 设置效果为默认值
		/// </summary>
		void ResetEffects();

		/// <summary>
		/// 重置颜色效果
		/// </summary>
		void ResetColorEffects()
		{
			Item().Tint(0);
			Item().Temperature(0);
			Item().Saturation(1);
		}

		/// <summary>
		/// 重置亮度效果
		/// </summary>
		void ResetLightEffects()
		{
			Item().Contrast(0);
			Item().Exposure(0);
		}

		/// <summary>
		/// 重置模糊效果
		/// </summary>
		void ResetBlurEffects()
		{
			Item().BlurAmount(0);
		}

		/// <summary>
		/// 重置泛黄效果
		/// </summary>
		void ResetSepiaEffects()
		{
			Item().Intensity(.5f);
		}

		/// <summary>
		/// 导航到详情页面时的事件
		/// </summary>
		/// <param name=""></param>
		/// <returns></returns>
		Windows::Foundation::IAsyncAction OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs);

		/// <summary>
		/// 返回主页时候的动画
		/// </summary>
		/// <param name=""></param>
		void OnNavigatingFrom(Windows::UI::Xaml::Navigation::NavigatingCancelEventArgs const&);

		/// <summary>
		/// 从详情页面返回主页面按钮事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void BackButton_ItemClick(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);	

		/// <summary>
		/// 缩放等级改变事件句柄
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void ZoomSlider_ValueChanged(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const&);
		
		/// <summary>
		/// 缩放等级改变事件句柄
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void MainImageScroller_ViewChanged(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::Controls::ScrollViewerViewChangedEventArgs const&);
		
		/// <summary>
		/// 改变效果选择的事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void Effects_SelectionChanged(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const&);
		
		/// <summary>
		/// 选中编辑按钮事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void EditButton_Check(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);
		
		/// <summary>
		/// 取消选中编辑按钮事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void EditButton_Uncheck(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);
		
		/// <summary>
		/// 关闭效果名称的文本块事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void TextBlock_Tapped(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::Input::TappedRoutedEventArgs const&);
		
		/// <summary>
		/// 移除所有效果的点击事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void RemoveAllEffectsButton_Click(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);
		
		/// <summary>
		/// 点击保存按钮的事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		/// <returns></returns>
		Windows::Foundation::IAsyncAction SaveButton_Click(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);

		/// <summary>
		/// 选择效果的按钮点击事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void SelectEffectsButton_Click(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);
		
		/// <summary>
		/// 点击应用效果的事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void ApplyEffectsButton_Click(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);
		
		/// <summary>
		/// 点击取消效果的事件
		/// </summary>
		/// <param name=""></param>
		/// <param name=""></param>
		void CancelEffectsButton_Click(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&);

	private:
		
		/// <summary>
		/// 初始化所有图片效果
		/// </summary>
		void InitializeEffects();

		/// <summary>
		/// 初始化图片预览
		/// </summary>
		void InitializeEffectPreviews();

		/// <summary>
		/// 初始化图片预览
		/// </summary>
		/// <param name="">效果</param>
		/// <param name="">图片</param>
		/// <returns></returns>
		Windows::Foundation::IAsyncAction InitializeEffectPreview(Windows::Foundation::IInspectable, Windows::UI::Xaml::Controls::Image);

		/// <summary>
		/// 从选中的效果创建效果图像
		/// </summary>
		void CreateEffectsGraph();

		/// <summary>
		/// 配置并且生成用于渲染的资源
		/// </summary>
		void UpdateMainImageBrush();

		/// <summary>
		/// 更新面板状态
		/// </summary>
		void UpdatePanelState();

		/// <summary>
		/// 准备选中的效果
		/// </summary>
		void PrepareSelectedEffects();

		/// <summary>
		/// 应用更改
		/// </summary>
		void ApplyEffects();

		/// <summary>
		/// 更新按钮图片笔刷
		/// </summary>
		void UpdateButtonImageBrush();

		// 图片元素的字段
		PhotoEditor::Photo item_{ nullptr };

		// 是否显示效果面板
		bool show_controls_ = false;

		event_revoker<Windows::UI::Xaml::Data::INotifyPropertyChanged> property_changed_token_;

		// 存储 Windows.UI.Composition 名称空间的页的字段
		Windows::UI::Composition::Compositor compositor_{ nullptr };

		// 图片效果、动画的集合字段
		Microsoft::Graphics::Canvas::Effects::ContrastEffect contrast_effect_{};
		Microsoft::Graphics::Canvas::Effects::ExposureEffect exposure_effect_{};
		Microsoft::Graphics::Canvas::Effects::TemperatureAndTintEffect temperature_and_tint_effect_{};
		Microsoft::Graphics::Canvas::Effects::GaussianBlurEffect blur_effect_{};
		Microsoft::Graphics::Canvas::Effects::SaturationEffect saturation_effect_{};
		Microsoft::Graphics::Canvas::Effects::SepiaEffect sepia_effect_{};
		Microsoft::Graphics::Canvas::Effects::GrayscaleEffect grayscale_effect_{};
		Microsoft::Graphics::Canvas::Effects::InvertEffect invert_effect_{};
		Microsoft::Graphics::Canvas::Effects::CompositeEffect graphics_effect_{};
		Windows::UI::Composition::CompositionEffectBrush combined_brush_{ nullptr };

		std::vector<Windows::Foundation::IInspectable> selected_effects_temp_{};
		std::vector<hstring> animatable_properties_list_{};

		std::vector<std::variant<Microsoft::Graphics::Canvas::Effects::ContrastEffect,
			Microsoft::Graphics::Canvas::Effects::ExposureEffect,
			Microsoft::Graphics::Canvas::Effects::TemperatureAndTintEffect,
			Microsoft::Graphics::Canvas::Effects::GaussianBlurEffect,
			Microsoft::Graphics::Canvas::Effects::SaturationEffect,
			Microsoft::Graphics::Canvas::Effects::SepiaEffect,
			Microsoft::Graphics::Canvas::Effects::GrayscaleEffect,
			Microsoft::Graphics::Canvas::Effects::InvertEffect,
			Microsoft::Graphics::Canvas::Effects::CompositeEffect>> effects_list_{};

		// 照片图像源
		Windows::UI::Xaml::Media::Imaging::BitmapImage image_source_{ nullptr };

	};
}

namespace winrt::PhotoEditor::factory_implementation
{
	struct DetailPage : DetailPageT<DetailPage, implementation::DetailPage>
	{
	};
}