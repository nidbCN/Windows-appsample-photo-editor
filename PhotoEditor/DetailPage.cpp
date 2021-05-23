/*
 * 图片详情视图代码
 */

// ReSharper disable CppExpressionWithoutSideEffects
#include "pch.h"
#include "DetailPage.h"
#include "Photo.h"

using namespace winrt;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Effects;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::UI::Xaml;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Graphics::Effects;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Storage;
using namespace Windows::Storage::Search;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Media::Animation;
using namespace Windows::UI::Xaml::Navigation;

namespace winrt::PhotoEditor::implementation
{
	DetailPage::DetailPage() : compositor_(Window::Current().Compositor())
	{
		// 初始化组件
		InitializeComponent();
		// 选中编辑按钮
		EditButton().IsChecked(true);
	}

	void DetailPage::FitToScreen()
	{
		// 计算缩放比例
		const auto width = MainImageScroller().ActualWidth() / Item().ImageProperties().Width();
		const auto height = MainImageScroller().ActualHeight() / Item().ImageProperties().Height();
		const auto zoom_factor = static_cast<float>(std::min(width, height));

		// 更改视图
		// ReSharper disable once CppExpressionWithoutSideEffects
		MainImageScroller().ChangeView(nullptr, nullptr, zoom_factor);
	}

	void DetailPage::ShowActualSize()
	{
		// 缩放比例更改为1
		MainImageScroller().ChangeView(nullptr, nullptr, 1);
	}

	void DetailPage::UpdateZoomState()
	{
		if (MainImageScroller().ZoomFactor() == 1)
		{
			FitToScreen();
		}
		else
		{
			ShowActualSize();
		}
	}

	void DetailPage::SelectEffectsButton_Click(IInspectable const&, RoutedEventArgs const&)
	{
		// 显示效果预览
		VisualStateManager().GoToState(*this, L"ChooseEffects", true);

		// 保存选中的效果，以便操作取消还可以还原
		selected_effects_temp_.clear();
		for (auto&& effect_item : EffectPreviewGrid().SelectedItems())
		{
			selected_effects_temp_.push_back(effect_item);
		}
	}

	void DetailPage::UpdatePanelState()
	{
		if (show_controls_)
		{
			VisualStateManager().GoToState(*this, L"EditEffects", true);
		}
		else if (EffectPreviewGrid().SelectedItems().Size() == 0)
		{
			// 没有选择任何效果
			VisualStateManager().GoToState(*this, L"Normal", true);
		}
		else
		{
			// 仅选中了没有调节的效果
			VisualStateManager().GoToState(*this, L"SaveEffects", true);
		}
	}

	void DetailPage::PrepareSelectedEffects()
	{
		// 清空效果
		effects_list_.clear();
		animatable_properties_list_.clear();

		// 折叠效果控制面板
		sepiaControlsGrid().Visibility(Visibility::Collapsed);
		blurControlsGrid().Visibility(Visibility::Collapsed);
		colorControlsGrid().Visibility(Visibility::Collapsed);
		lightControlsGrid().Visibility(Visibility::Collapsed);

		// 取消显示效果控制面板
		show_controls_ = false;
		const auto epg = EffectPreviewGrid().as<GridView>();

		// 添加选中的效果
		for (auto&& item : epg.SelectedItems())
		{
			auto preview = item.as<Grid>();
			const auto tag = unbox_value<hstring>(preview.Tag());

			// "switch-case"
			if (tag == L"sepia")
			{
				// 仅用于按钮预览
				sepia_effect_.Intensity(1.0f);
				effects_list_.push_back(sepia_effect_);
				animatable_properties_list_.push_back(L"SepiaEffect.Intensity");

				// 显示控制面板
				sepiaControlsGrid().Visibility(Visibility::Visible);
				show_controls_ = true;
			}
			else if (tag == L"invert")
			{
				// 无预览
				effects_list_.push_back(invert_effect_);
			}
			else if (tag == L"grayscale")
			{
				// 无预览
				effects_list_.push_back(grayscale_effect_);
			}
			else if (tag == L"blur")
			{
				// 仅用于按钮预览
				blur_effect_.BlurAmount(2.5f);
				effects_list_.push_back(blur_effect_);
				animatable_properties_list_.push_back(L"BlurEffect.BlurAmount");

				blurControlsGrid().Visibility(Visibility::Visible);
				show_controls_ = true;
			}
			else if (tag == L"color")
			{
				// 仅用于按钮预览
				temperature_and_tint_effect_.Temperature(0.25f);
				temperature_and_tint_effect_.Tint(-0.25f);
				effects_list_.push_back(temperature_and_tint_effect_);

				animatable_properties_list_.push_back(L"TemperatureAndTintEffect.Temperature");
				animatable_properties_list_.push_back(L"TemperatureAndTintEffect.Tint");

				effects_list_.push_back(saturation_effect_);
				animatable_properties_list_.push_back(L"SaturationEffect.Saturation");

				colorControlsGrid().Visibility(Visibility::Visible);
				show_controls_ = true;
			}
			else if (tag == L"light")
			{
				// 仅用于按钮预览
				contrast_effect_.Contrast(.25f);
				effects_list_.push_back(contrast_effect_);
				animatable_properties_list_.push_back(L"ContrastEffect.Contrast");

				exposure_effect_.Exposure(-0.25f);
				effects_list_.push_back(exposure_effect_);
				animatable_properties_list_.push_back(L"ExposureEffect.Exposure");

				lightControlsGrid().Visibility(Visibility::Visible);
				show_controls_ = true;
			}
		}

		effects_list_.push_back(graphics_effect_);
	}

	void DetailPage::ApplyEffects()
	{
		// 准备选中的效果
		PrepareSelectedEffects();
		// 更新
		UpdateMainImageBrush();

		// 遍历效果列表，添加效果
		for (auto&& item : animatable_properties_list_)
		{
			// 将列表中的元素转化为字符串
			std::wstring effect_name = static_cast<std::wstring>(item);

			// 获取点的索引号
			const auto index = effect_name.find_last_of(L'.', effect_name.size());

			// 属性为以点分隔的子字符串
			hstring prop = static_cast<hstring>(effect_name.substr(index + 1, effect_name.size()));

			// 更新效果笔刷
			UpdateEffectBrush(prop);
		}
	}

	void DetailPage::ApplyEffectsButton_Click(IInspectable const&, RoutedEventArgs const&)
	{
		// 更新面板状态
		UpdatePanelState();

		// 应用效果
		ApplyEffects();
	}

	void DetailPage::CancelEffectsButton_Click(IInspectable const&, RoutedEventArgs const&)
	{
		// 更新面板状态
		UpdatePanelState();

		// 情况选中的预览的效果
		EffectPreviewGrid().SelectedItems().Clear();

		// 从临时列表中添加效果，恢复以前选择的
		for (auto&& effectItem : selected_effects_temp_)
		{
			EffectPreviewGrid().SelectedItems().Append(effectItem);
		}
	}

	void DetailPage::UpdateButtonImageBrush()
	{
		// 设置图片预览背景
		ButtonPreviewImage().Source(image_source_);
		ButtonPreviewImage().InvalidateArrange();

		// 创建效果图形
		CreateEffectsGraph();

		// 创建目标笔刷
		auto destination_brush = compositor_.CreateBackdropBrush();
		// 创建笔刷工厂
		auto graphics_effect_factory = compositor_.CreateEffectFactory(graphics_effect_);

		// 从笔刷工厂创建笔刷
		auto preview_brush = graphics_effect_factory.CreateBrush();

		preview_brush.SetSourceParameter(L"Backdrop", destination_brush);

		auto effect_sprite = compositor_.CreateSpriteVisual();
		effect_sprite.Size(float2{ 232, 64 });
		effect_sprite.Brush(preview_brush);

		ElementCompositionPreview::SetElementChildVisual(ButtonPreviewImage(), effect_sprite);
	}

	void DetailPage::UpdateEffectBrush(hstring const& propertyName)
	{
		if (combined_brush_)
		{
			// "switch-case"
			if (propertyName == L"Exposure")
			{
				// 曝光
				combined_brush_.Properties().InsertScalar(L"ExposureEffect.Exposure", Item().Exposure());
			}
			else if (propertyName == L"Temperature")
			{
				// 色温
				combined_brush_.Properties().InsertScalar(L"TemperatureAndTintEffect.Temperature", Item().Temperature());
			}
			else if (propertyName == L"Tint")
			{
				// 色调
				combined_brush_.Properties().InsertScalar(L"TemperatureAndTintEffect.Tint", Item().Tint());
			}
			else if (propertyName == L"Contrast")
			{
				// 对比度
				combined_brush_.Properties().InsertScalar(L"ContrastEffect.Contrast", Item().Contrast());
			}
			else if (propertyName == L"Saturation")
			{
				// 饱和度
				combined_brush_.Properties().InsertScalar(L"SaturationEffect.Saturation", Item().Saturation());
			}
			else if (propertyName == L"BlurAmount")
			{
				// 模糊强度
				combined_brush_.Properties().InsertScalar(L"BlurEffect.BlurAmount", Item().BlurAmount());
			}
			else if (propertyName == L"Intensity")
			{
				// 变旧强度
				combined_brush_.Properties().InsertScalar(L"SepiaEffect.Intensity", Item().Intensity());
			}
		}
	}

	void DetailPage::ResetEffects()
	{
		Item().Exposure(0);
		Item().BlurAmount(0);
		Item().Tint(0);
		Item().Temperature(0);
		Item().Contrast(0);
		Item().Saturation(1);
		Item().Intensity(0.5F);
	}

	IAsyncAction DetailPage::OnNavigatedTo(NavigationEventArgs e)
	{

		Item(e.Parameter().as<PhotoEditor::Photo>());

		if (auto item = Item())
		{
			Photo* impleType = from_abi<Photo>(item);
			image_source_ = co_await impleType->GetImageSourceAsync();

			// 监听属性更改
			property_changed_token_ = item.PropertyChanged(auto_revoke, [weak{ get_weak() }](auto&&, auto&& args)
			{
				if (auto strong = weak.get())
				{
					strong->UpdateEffectBrush(args.PropertyName());
				}
			});

			// 设置图片源
			targetImage().Source(image_source_);

			// 连接动画
			auto image_animation = ConnectedAnimationService::GetForCurrentView().GetAnimation(L"itemAnimation");
			if (image_animation)
			{
				image_animation.Completed([weak{ get_weak() }](auto&&, auto&&)
				{
					if (auto strong = weak.get())
					{
						strong->MainImage().Source(strong->image_source_);
						strong->MainImage().Visibility(Visibility::Visible);
						strong->targetImage().Source(nullptr);

						strong->InitializeEffects();
						strong->UpdateMainImageBrush();
						strong->InitializeEffectPreviews();
						strong->UpdateButtonImageBrush();
					}
				});

				image_animation.TryStart(targetImage());
			}
			if (image_source_.PixelHeight() == 0 && image_source_.PixelWidth() == 0)
			{
				// 没有加载的图片，禁用编辑和缩放功能
				EditButton().IsEnabled(false);
				ZoomButton().IsEnabled(false);
			}
		}

		BackButton().IsEnabled(Frame().CanGoBack());
	}

	void DetailPage::OnNavigatingFrom(NavigatingCancelEventArgs const& e)
	{
		if (e.NavigationMode() == NavigationMode::Back)
		{
			// 重置效果
			ResetEffects();
			// 播放连接动画
			ConnectedAnimationService::GetForCurrentView().PrepareToAnimate(L"backAnimation", MainImage());
		}
	}

	void DetailPage::InitializeEffects()
	{
		saturation_effect_.Name(L"SaturationEffect");
		saturation_effect_.Saturation(Item().Saturation());

		sepia_effect_.Name(L"SepiaEffect");
		sepia_effect_.Intensity(Item().Intensity());

		invert_effect_.Source(CompositionEffectSourceParameter{ L"source" });

		grayscale_effect_.Source(CompositionEffectSourceParameter{ L"source" });

		contrast_effect_.Name(L"ContrastEffect");
		contrast_effect_.Contrast(Item().Contrast());

		exposure_effect_.Name(L"ExposureEffect");
		exposure_effect_.Exposure(Item().Exposure());

		temperature_and_tint_effect_.Name(L"TemperatureAndTintEffect");
		temperature_and_tint_effect_.Temperature(Item().Temperature());

		blur_effect_.Name(L"BlurEffect");
		blur_effect_.BlurAmount(Item().BlurAmount());
		blur_effect_.BorderMode(EffectBorderMode::Hard);

		graphics_effect_.Sources().Append(CompositionEffectSourceParameter{ L"Backdrop" });

		effects_list_.push_back(graphics_effect_);
	}

	void DetailPage::InitializeEffectPreviews()
	{
		SepiaEffect sepiaEffect{};
		sepiaEffect.Intensity(0.5f);
		sepiaEffect.Source(CompositionEffectSourceParameter{ L"source" });
		InitializeEffectPreview(sepiaEffect, sepiaImage());

		GrayscaleEffect grayscaleEffect{};
		grayscaleEffect.Source(CompositionEffectSourceParameter{ L"source" });
		InitializeEffectPreview(grayscaleEffect, grayscaleImage());

		GaussianBlurEffect blurEffect{};
		blurEffect.BlurAmount(3.0f);
		blurEffect.Source(CompositionEffectSourceParameter{ L"source" });
		InitializeEffectPreview(blurEffect, blurImage());

		InvertEffect invertEffect{};
		invertEffect.Source(CompositionEffectSourceParameter{ L"source" });
		InitializeEffectPreview(invertEffect, invertImage());

		ExposureEffect lightEffect{};
		lightEffect.Exposure(1.0f);
		lightEffect.Source(CompositionEffectSourceParameter{ L"source" });
		InitializeEffectPreview(lightEffect, lightImage());

		SaturationEffect colorEffect{};
		colorEffect.Saturation(0.5f);
		colorEffect.Source(CompositionEffectSourceParameter{ L"source" });
		InitializeEffectPreview(colorEffect, colorImage());
	}

	IAsyncAction DetailPage::InitializeEffectPreview(IInspectable compEffect, Image image)
	{
		Photo* implType = from_abi<Photo>(Item());
		image.Source(co_await implType->GetImageThumbnailAsync());
		image.InvalidateArrange();

		auto destinationBrush = compositor_.CreateBackdropBrush();
		auto graphicsEffectFactory = compositor_.CreateEffectFactory(compEffect.as<IGraphicsEffect>());
		auto combinedBrush = graphicsEffectFactory.CreateBrush();
		auto effectSprite = compositor_.CreateSpriteVisual();

		combinedBrush.SetSourceParameter(L"source", destinationBrush);
		effectSprite.Size(float2{ 188,88 });
		effectSprite.Brush(combinedBrush);
		ElementCompositionPreview::SetElementChildVisual(image, effectSprite);
	}

	void DetailPage::CreateEffectsGraph()
	{
		auto as_source = [](auto&& arg) -> IGraphicsEffectSource
		{
			return arg;
		};

		// 创建效果列表
		for (size_t i = 0; i < effects_list_.size(); i++)
		{
			const auto& effect = effects_list_[i];

			if (i == 0)
			{
				// 第一个效果
				std::visit([&graphicsEffect = graphics_effect_](auto&& effect)
					{
						CompositionEffectSourceParameter source{ L"Backdrop" };
						if constexpr (std::is_same_v<CompositeEffect, std::decay_t<decltype(effect)>>)
						{
							auto const& sources = graphicsEffect.Sources();
							sources.Clear();
							sources.Append(source);
						}
						else
						{
							effect.Source(source);
						}
					}, effect);
			}
			else if (i < effects_list_.size() - 1)
			{
				// 不是最后一个效果
				std::visit([source = std::visit(as_source, effects_list_[i - 1])](auto&& effect)
				{
					if constexpr (!std::is_same_v<CompositeEffect, std::decay_t<decltype(effect)>>)
					{
						effect.Source(source);
					}
				}, effect);
			}
			else
			{
				// 符合效果是列表的最后一项
				auto const& sources = graphics_effect_.Sources();
				sources.Clear();
				sources.Append(std::visit(as_source, effects_list_[i - 1]));
			}
		}
	}

	void DetailPage::UpdateMainImageBrush()
	{
		MainImage().Source(image_source_);
		MainImage().InvalidateArrange();

		// 创建效果图像
		CreateEffectsGraph();

		// 目标笔刷
		auto destination_brush = compositor_.CreateBackdropBrush();
		// 效果工厂
		auto graphics_effect_factory = compositor_.CreateEffectFactory(graphics_effect_, animatable_properties_list_);

		// 创建复合笔刷
		combined_brush_ = graphics_effect_factory.CreateBrush();
		combined_brush_.SetSourceParameter(L"Backdrop", destination_brush);

		const auto effect_sprite = compositor_.CreateSpriteVisual();
		effect_sprite.Size(float2{ static_cast<float>(image_source_.PixelWidth()), static_cast<float>(image_source_.PixelHeight()) });
		effect_sprite.Brush(combined_brush_);

		// 设置显示效果
		ElementCompositionPreview::SetElementChildVisual(MainImage(), effect_sprite);
	}

	void DetailPage::BackButton_ItemClick(IInspectable const&, RoutedEventArgs const&)
	{
		const auto suppress = SuppressNavigationTransitionInfo();

		if (Frame().CanGoBack())
		{
			Frame().GoBack(suppress);
		}
	}

	void DetailPage::EditButton_Check(IInspectable const&, RoutedEventArgs const&)
	{
		// 显示编辑界面
		EditPanel().Visibility(Visibility::Visible);
	}

	void DetailPage::EditButton_Uncheck(IInspectable const&, RoutedEventArgs const&)
	{
		// 折叠编辑界面
		EditPanel().Visibility(Visibility::Collapsed);
	}

	void DetailPage::Effects_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&)
	{
		// 准备选择的效果
		PrepareSelectedEffects();
		// 更新按钮图片笔刷
		UpdateButtonImageBrush();
	}

	void DetailPage::ZoomSlider_ValueChanged(IInspectable const&, Primitives::RangeBaseValueChangedEventArgs const& e)
	{
		if (MainImageScroller())
		{
			MainImageScroller().ChangeView(nullptr, nullptr, static_cast<float>(e.NewValue()));
		}
	}

	void DetailPage::MainImageScroller_ViewChanged(IInspectable const& sender, ScrollViewerViewChangedEventArgs const&)
	{
		ZoomSlider().Value(sender.as<ScrollViewer>().ZoomFactor());
	}

	void DetailPage::TextBlock_Tapped(IInspectable const& sender, TappedRoutedEventArgs const&)
	{
		// 初始化变量
		bool wasFound = false;
		uint32_t indexOf = 0;

		// 遍历选中的效果
		for (auto&& effect_item : EffectPreviewGrid().SelectedItems())
		{
			// 拆箱，获取标签
			const auto effect_tag = unbox_value<hstring>(effect_item.as<FrameworkElement>().Tag());

			// 遍历到当前选中的元素的标签
			if (effect_tag == unbox_value<hstring>(sender.as<FrameworkElement>().Tag()))
			{
				wasFound = EffectPreviewGrid().SelectedItems().IndexOf(effect_item, indexOf);

				// "switch-case"
				if (effect_tag == L"color")
				{
					ResetColorEffects();
				}
				else if (effect_tag == L"light")
				{
					ResetLightEffects();
				}
				else if (effect_tag == L"blur")
				{
					ResetBlurEffects();
				}
				else if (effect_tag == L"sepia")
				{
					ResetSepiaEffects();
				}
			}
		}

		// 上面遍历中找到过这个效果
		if (wasFound)
		{
			// 从选中中删除
			EffectPreviewGrid().SelectedItems().RemoveAt(indexOf);
		}

		// 应用效果并更新
		ApplyEffects();
		UpdatePanelState();
		UpdateButtonImageBrush();
	}

	void DetailPage::RemoveAllEffectsButton_Click(IInspectable const&, RoutedEventArgs const&)
	{
		// 清空选中的项
		EffectPreviewGrid().SelectedItems().Clear();
		// 重置效果
		ResetEffects();
		// 应用效果
		ApplyEffects();
		// 更新
		UpdatePanelState();
		UpdateButtonImageBrush();
	}

	IAsyncAction DetailPage::SaveButton_Click(IInspectable const&, RoutedEventArgs const&)
	{
		// 创建一个选择器
		const auto picker = FileSavePicker{};
		// 设置默认路径
		picker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
		// 设置默认文件名
		picker.SuggestedFileName(L"New Image");

		// 获取文件类型
		const auto file_type = static_cast<std::wstring>(Item().ImageFileType());
		const auto file_ext_index = file_type.find_last_of(L' ', file_type.size());
		auto file_ext = L"." + file_type.substr(0, file_ext_index);

		// 大小写转换
		transform(
			file_ext.begin(), file_ext.end(),
			file_ext.begin(),
			::towlower);

		// 转换为hString
		const auto file_ext_h = hstring(file_ext);


		// 文件类型 JPEG、PNG、TIF、BMP
		picker.FileTypeChoices().Insert(file_type, winrt::single_threaded_vector<hstring>({ file_ext_h }));

		if (const auto& file = co_await picker.PickSaveFileAsync())
		{
			// 创建文件读写流
			if (const auto& stream = co_await file.OpenAsync(Windows::Storage::FileAccessMode::ReadWrite))
			{
				const BitmapEncoder* encoder_ptr = nullptr;

				// 从流创建编码器
				if (file_ext == L".jpg")
				{
					const auto& encoder = co_await BitmapEncoder::CreateAsync(BitmapEncoder::JpegEncoderId(), stream);
					encoder_ptr = &encoder;
				}
				else if (file_ext == L".png")
				{
					const auto& encoder = co_await BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId(), stream);
					encoder_ptr = &encoder;
				}
				else if (file_ext == L".gif")
				{
					const auto& encoder = co_await BitmapEncoder::CreateAsync(BitmapEncoder::GifEncoderId(), stream);
					encoder_ptr = &encoder;
				}
				else
				{
					// 不被支持的文件类型

					// 设置对话框
					const ContentDialog unsupported_files_dialog{};
					unsupported_files_dialog.Title(box_value(L"无法保存图片！"));
					unsupported_files_dialog.Content(box_value(L"不支持的图片编码类型，无法保存这种文件。"));
					unsupported_files_dialog.CloseButtonText(L"确定");

					// 弹出对话框
					co_await unsupported_files_dialog.ShowAsync();
				}

				// 渲染图片
				const RenderTargetBitmap render_target_bitmap{};
				co_await render_target_bitmap.RenderAsync(MainImage());

				// 获取像素到缓存
				const IBuffer pixels = co_await render_target_bitmap.GetPixelsAsync();
				const auto new_buffer = SoftwareBitmap::CreateCopyFromBuffer(
					pixels,
					BitmapPixelFormat::Bgra8,
					Item().ImageProperties().Width(),
					Item().ImageProperties().Height()
				);

				co_await(*encoder_ptr).FlushAsync();

				co_await Windows::Storage::CachedFileManager::CompleteUpdatesAsync(file);
			}
		}
	}
}
