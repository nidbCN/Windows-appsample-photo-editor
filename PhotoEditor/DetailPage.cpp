/*
 * 图片详情视图代码
 */

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
    DetailPage::DetailPage() : m_compositor(Window::Current().Compositor())
    {
    	// 初始化组件
        InitializeComponent();
        // 选中编辑按钮
        EditButton().IsChecked(true);
    }

    /// <summary>
    /// 缩放图片至适应屏幕
    /// </summary>
    void DetailPage::FitToScreen()
    {
    	// 计算缩放比例
        const auto width = MainImageScroller().ActualWidth() / Item().ImageProperties().Width();
        const auto height = MainImageScroller().ActualHeight() / Item().ImageProperties().Height();
        const auto zoom_factor = static_cast<float>(std::min(width, height));

        // 更改视图
    	MainImageScroller().ChangeView(nullptr, nullptr, zoom_factor);
    }

    
    /// <summary>
    /// 显示实际大小
    /// </summary>
    void DetailPage::ShowActualSize()
    {
        // 缩放比例更改为1
        MainImageScroller().ChangeView(nullptr, nullptr, 1);
    }

    /// <summary>
    /// 在点击后更改图片缩放
    /// </summary>
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

    /// <summary>
    /// 选择效果的按钮点击事件
    /// </summary>
    /// <param name=""></param>
    /// <param name=""></param>
    void DetailPage::SelectEffectsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // 显示效果预览
        VisualStateManager().GoToState(*this, L"ChooseEffects", true);

        // 保存选中的效果，以便操作取消还可以还原
        m_selectedEffectsTemp.clear();
        for (auto&& effectItem : EffectPreviewGrid().SelectedItems())
        {
            m_selectedEffectsTemp.push_back(effectItem);
        }
    }

    /// <summary>
    /// 更新面板状态
    /// </summary>
    void DetailPage::UpdatePanelState()
    {
        if (m_showControls)
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

    /// <summary>
    /// 准备选中的效果
    /// </summary>
    void DetailPage::PrepareSelectedEffects()
    {
        // 清空效果
        m_effectsList.clear();
        m_animatablePropertiesList.clear();

        // 折叠效果控制面板
        sepiaControlsGrid().Visibility(Visibility::Collapsed);
        blurControlsGrid().Visibility(Visibility::Collapsed);
        colorControlsGrid().Visibility(Visibility::Collapsed);
        lightControlsGrid().Visibility(Visibility::Collapsed);

        // 取消显示效果控制面板
        m_showControls = false;
        GridView epg = EffectPreviewGrid().as<GridView>();

        // 添加选中的效果
        for (auto&& item : epg.SelectedItems())
        {
            auto preview = item.as<Grid>();
            auto tag = unbox_value<hstring>(preview.Tag());

            // "switch-case"
            if (tag == L"sepia")
            {
                // 仅用于按钮预览
                m_sepiaEffect.Intensity(1.0f);
                m_effectsList.push_back(m_sepiaEffect);
                m_animatablePropertiesList.push_back(L"SepiaEffect.Intensity");
                sepiaControlsGrid().Visibility(Visibility::Visible);
                m_showControls = true;
            }
            else if (tag == L"invert")
            {
                m_effectsList.push_back(m_invertEffect);
            }
            else if (tag == L"grayscale")
            {
                m_effectsList.push_back(m_grayscaleEffect);
            }
            else if (tag == L"blur")
            {
                // This blur amount is applied only to the button preview.
                m_blurEffect.BlurAmount(2.5f);
                m_effectsList.push_back(m_blurEffect);
                m_animatablePropertiesList.push_back(L"BlurEffect.BlurAmount");
                blurControlsGrid().Visibility(Visibility::Visible);
                m_showControls = true;
            }
            else if (tag == L"color")
            {
                // These values are applied only to the button preview.
                m_temperatureAndTintEffect.Temperature(0.25f);
                m_temperatureAndTintEffect.Tint(-0.25f);
                m_effectsList.push_back(m_temperatureAndTintEffect);
                m_animatablePropertiesList.push_back(L"TemperatureAndTintEffect.Temperature");
                m_animatablePropertiesList.push_back(L"TemperatureAndTintEffect.Tint");
                m_effectsList.push_back(m_saturationEffect);
                m_animatablePropertiesList.push_back(L"SaturationEffect.Saturation");
                colorControlsGrid().Visibility(Visibility::Visible);
                m_showControls = true;
            }
            else if (tag == L"light")
            {
                // This contrast amount is applied only to the button preview.
                m_contrastEffect.Contrast(.25f);
                m_effectsList.push_back(m_contrastEffect);
                m_animatablePropertiesList.push_back(L"ContrastEffect.Contrast");
                // This exposure amount is applied only to the button preview.
                m_exposureEffect.Exposure(-0.25f);
                m_effectsList.push_back(m_exposureEffect);
                m_animatablePropertiesList.push_back(L"ExposureEffect.Exposure");
                lightControlsGrid().Visibility(Visibility::Visible);
                m_showControls = true;
            }
        }

        m_effectsList.push_back(m_graphicsEffect);
    }

    void DetailPage::ApplyEffects()
    {
        PrepareSelectedEffects();
        UpdateMainImageBrush();

        for (auto&& item : m_animatablePropertiesList)
        {
            std::wstring str = static_cast<std::wstring>(item);
            auto index = str.find_last_of(L'.', str.size());
            hstring prop = static_cast<hstring>(str.substr(index + 1, str.size()));
            UpdateEffectBrush(prop);
        }
    }

    void DetailPage::ApplyEffectsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        UpdatePanelState();
        ApplyEffects();
    }

    void DetailPage::CancelEffectsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        UpdatePanelState();
        EffectPreviewGrid().SelectedItems().Clear();
        for (auto&& effectItem : m_selectedEffectsTemp)
        {
            EffectPreviewGrid().SelectedItems().Append(effectItem);
        }
    }

    void DetailPage::UpdateButtonImageBrush()
    {
        ButtonPreviewImage().Source(m_imageSource);
        ButtonPreviewImage().InvalidateArrange();

        CreateEffectsGraph();

        auto destinationBrush = m_compositor.CreateBackdropBrush();
        auto graphicsEffectFactory = m_compositor.CreateEffectFactory(m_graphicsEffect);

        auto previewBrush = graphicsEffectFactory.CreateBrush();
        previewBrush.SetSourceParameter(L"Backdrop", destinationBrush);

        auto effectSprite = m_compositor.CreateSpriteVisual();
        effectSprite.Size(float2{ 232, 64 });
        effectSprite.Brush(previewBrush);
        ElementCompositionPreview::SetElementChildVisual(ButtonPreviewImage(), effectSprite);
    }


    // Adds or updates specific effect value within combined brush.
    void DetailPage::UpdateEffectBrush(hstring const& propertyName)
    {
        if (m_combinedBrush)
        {
            if (propertyName == L"Exposure")
            {
                m_combinedBrush.Properties().InsertScalar(L"ExposureEffect.Exposure", Item().Exposure());
            }
            else if (propertyName == L"Temperature")
            {
                m_combinedBrush.Properties().InsertScalar(L"TemperatureAndTintEffect.Temperature", Item().Temperature());
            }
            else if (propertyName == L"Tint")
            {
                m_combinedBrush.Properties().InsertScalar(L"TemperatureAndTintEffect.Tint", Item().Tint());
            }
            else if (propertyName == L"Contrast")
            {
                m_combinedBrush.Properties().InsertScalar(L"ContrastEffect.Contrast", Item().Contrast());
            }
            else if (propertyName == L"Saturation")
            {
                m_combinedBrush.Properties().InsertScalar(L"SaturationEffect.Saturation", Item().Saturation());
            }
            else if (propertyName == L"BlurAmount")
            {
                m_combinedBrush.Properties().InsertScalar(L"BlurEffect.BlurAmount", Item().BlurAmount());
            }
            else if (propertyName == L"Intensity")
            {
                m_combinedBrush.Properties().InsertScalar(L"SepiaEffect.Intensity", Item().Intensity());
            }
        }
    }

    // Resets effects to their default values.
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

    // Retrieves appropriate photo, sets up detail view, and performs animation.
    IAsyncAction DetailPage::OnNavigatedTo(NavigationEventArgs e)
    {
        Item(e.Parameter().as<PhotoEditor::Photo>());

        if (auto item = Item())
        {
            Photo* impleType = from_abi<Photo>(item);
            m_imageSource = co_await impleType->GetImageSourceAsync();

            // Because DetailPage can be destroyed during the life of the event handler, 
            // it is good practice to create a weak_ref to *this, capture it in the lambda, and resolve it before use.
            m_propertyChangedToken = item.PropertyChanged(auto_revoke, [weak{ get_weak() }](auto&&, auto&& args)
            {
                if (auto strong = weak.get())
                {
                    strong->UpdateEffectBrush(args.PropertyName());
                }
            });

            targetImage().Source(m_imageSource);

            ConnectedAnimation imageAnimation = ConnectedAnimationService::GetForCurrentView().GetAnimation(L"itemAnimation");
            if (imageAnimation)
            {
                imageAnimation.Completed([weak{ get_weak() }](auto&&, auto&&)
                {
                    if (auto strong = weak.get())
                    {
                        strong->MainImage().Source(strong->m_imageSource);
                        strong->MainImage().Visibility(Visibility::Visible);
                        strong->targetImage().Source(nullptr);

                        strong->InitializeEffects();
                        strong->UpdateMainImageBrush();
                        strong->InitializeEffectPreviews();
                        strong->UpdateButtonImageBrush();
                    }
                });

                imageAnimation.TryStart(targetImage());
            }
            if (m_imageSource.PixelHeight() == 0 && m_imageSource.PixelWidth() == 0)
            {
                // There is no editable image loaded. Disable zoom and edit
                // to prevent other errors.
                EditButton().IsEnabled(false);
                ZoomButton().IsEnabled(false);
            }
        }

        BackButton().IsEnabled(Frame().CanGoBack());
    }

    // Prepares animation for navigation back to MainPage view.
    void DetailPage::OnNavigatingFrom(NavigatingCancelEventArgs const& e)
    {
        if (e.NavigationMode() == NavigationMode::Back)
        {
            ResetEffects();
            ConnectedAnimationService::GetForCurrentView().PrepareToAnimate(L"backAnimation", MainImage());
        }
    }

    // Initializes image effects prior to creation of effect graph.
    void DetailPage::InitializeEffects()
    {
        m_saturationEffect.Name(L"SaturationEffect");
        m_saturationEffect.Saturation(Item().Saturation());
        m_sepiaEffect.Name(L"SepiaEffect");
        m_sepiaEffect.Intensity(Item().Intensity());
        m_invertEffect.Source(CompositionEffectSourceParameter{ L"source" });
        m_grayscaleEffect.Source(CompositionEffectSourceParameter{ L"source" });
        m_contrastEffect.Name(L"ContrastEffect");
        m_contrastEffect.Contrast(Item().Contrast());
        m_exposureEffect.Name(L"ExposureEffect");
        m_exposureEffect.Exposure(Item().Exposure());
        m_temperatureAndTintEffect.Name(L"TemperatureAndTintEffect");
        m_temperatureAndTintEffect.Temperature(Item().Temperature());
        m_blurEffect.Name(L"BlurEffect");
        m_blurEffect.BlurAmount(Item().BlurAmount());
        m_blurEffect.BorderMode(EffectBorderMode::Hard);
        m_graphicsEffect.Sources().Append(CompositionEffectSourceParameter{ L"Backdrop" });
        m_effectsList.push_back(m_graphicsEffect);
    }

    // Creates all the thumbnail previews for effect selection UI.
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

    // Creates a specified effect thumbnail for the effect preview UI.
    IAsyncAction DetailPage::InitializeEffectPreview(IInspectable compEffect, Image image)
    {
        Photo* implType = from_abi<Photo>(Item());
        image.Source(co_await implType->GetImageThumbnailAsync());
        image.InvalidateArrange();

        auto destinationBrush = m_compositor.CreateBackdropBrush();
        auto graphicsEffectFactory = m_compositor.CreateEffectFactory(compEffect.as<IGraphicsEffect>());
        auto combinedBrush = graphicsEffectFactory.CreateBrush();
        auto effectSprite = m_compositor.CreateSpriteVisual();

        combinedBrush.SetSourceParameter(L"source", destinationBrush);
        effectSprite.Size(float2{ 188,88 });
        effectSprite.Brush(combinedBrush);
        ElementCompositionPreview::SetElementChildVisual(image, effectSprite);
    }

    // Creates the effects graph based on the selected effects.
    void DetailPage::CreateEffectsGraph()
    {
        auto as_source = [](auto&& arg) -> IGraphicsEffectSource
        {
            return arg;
        };

        // Create effects chain from list of effects.
        // The CompositeEffect is always the end of the chain and gets some special handling.
        for (size_t i = 0; i < m_effectsList.size(); i++)
        {
            const auto& effect = m_effectsList[i];

            if (i == 0)
            {
                std::visit([&graphicsEffect = m_graphicsEffect](auto&& effect)
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
            else if (i < m_effectsList.size() - 1)
            {
                std::visit([source = std::visit(as_source, m_effectsList[i - 1])](auto&& effect)
                {
                    if constexpr (!std::is_same_v<CompositeEffect, std::decay_t<decltype(effect)>>)
                    {
                        effect.Source(source);
                    }
                }, effect);
            }
            else
            {
                // CompositeEffect is the last effect in the chain.
                auto const& sources = m_graphicsEffect.Sources();
                sources.Clear();
                sources.Append(std::visit(as_source, m_effectsList[i - 1]));
            }
        }
    }

    void DetailPage::UpdateMainImageBrush()
    {
        MainImage().Source(m_imageSource);
        MainImage().InvalidateArrange();

        CreateEffectsGraph();

        auto destinationBrush = m_compositor.CreateBackdropBrush();
        auto graphicsEffectFactory = m_compositor.CreateEffectFactory(m_graphicsEffect, m_animatablePropertiesList);

        m_combinedBrush = graphicsEffectFactory.CreateBrush();
        m_combinedBrush.SetSourceParameter(L"Backdrop", destinationBrush);

        auto effectSprite = m_compositor.CreateSpriteVisual();
        effectSprite.Size(float2{ static_cast<float>(m_imageSource.PixelWidth()), static_cast<float>(m_imageSource.PixelHeight()) });
        effectSprite.Brush(m_combinedBrush);
        ElementCompositionPreview::SetElementChildVisual(MainImage(), effectSprite);
    }

    // Back button event handler for navigation back to MainPage.
    void DetailPage::BackButton_ItemClick(IInspectable const&, RoutedEventArgs const&)
    {
        auto m_suppress = SuppressNavigationTransitionInfo();
        if (Frame().CanGoBack())
        {
            Frame().GoBack(m_suppress);
        }
    }

    // Edit button event handler for opening edit UI.
    void DetailPage::EditButton_Check(IInspectable const&, RoutedEventArgs const&)
    {
        EditPanel().Visibility(Visibility::Visible);
    }

    // Edit button event handler for closing edit UI.
    void DetailPage::EditButton_Uncheck(IInspectable const&, RoutedEventArgs const&)
    {
        EditPanel().Visibility(Visibility::Collapsed);
    }

    void DetailPage::Effects_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&)
    {
        PrepareSelectedEffects();
        UpdateButtonImageBrush();
    }

    // Event handler for zoom level change.
    void DetailPage::ZoomSlider_ValueChanged(IInspectable const&, Primitives::RangeBaseValueChangedEventArgs const& e)
    {
        if (MainImageScroller())
        {
            MainImageScroller().ChangeView(nullptr, nullptr, static_cast<float>(e.NewValue()));
        }
    }

    // Event handler for zoom level change.
    void DetailPage::MainImageScroller_ViewChanged(IInspectable const& sender, ScrollViewerViewChangedEventArgs const&)
    {
        ZoomSlider().Value(sender.as<ScrollViewer>().ZoomFactor());
    }

    void DetailPage::TextBlock_Tapped(IInspectable const& sender, TappedRoutedEventArgs const&)
    {
        bool wasFound = false;
        uint32_t indexOf = 0;

        for (auto&& effectItem : EffectPreviewGrid().SelectedItems())
        {
            auto effectTag = unbox_value<hstring>(effectItem.as<FrameworkElement>().Tag());

            if (effectTag == unbox_value<hstring>(sender.as<FrameworkElement>().Tag()))
            {
                wasFound = EffectPreviewGrid().SelectedItems().IndexOf(effectItem, indexOf);

                if (effectTag == L"color")
                {
                    ResetColorEffects();
                }
                else if (effectTag == L"light")
                {
                    ResetLightEffects();
                }
                else if (effectTag == L"blur")
                {
                    ResetBlurEffects();
                }
                else if (effectTag == L"sepia")
                {
                    ResetSepiaEffects();
                }
            }
        }

        if (wasFound)
        {
            EffectPreviewGrid().SelectedItems().RemoveAt(indexOf);
        }

        ApplyEffects();
        UpdatePanelState();
        UpdateButtonImageBrush();
    }

    void DetailPage::RemoveAllEffectsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        EffectPreviewGrid().SelectedItems().Clear();
        ResetEffects();
        ApplyEffects();
        UpdatePanelState();
        UpdateButtonImageBrush();
    }

    IAsyncAction DetailPage::SaveButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // Setup the picker.
        auto picker = FileSavePicker{};
        picker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
        picker.SuggestedFileName(L"New Image");
        picker.FileTypeChoices().Insert(L"Images", winrt::single_threaded_vector<hstring>({ L".jpg" }));

        if (auto file = co_await picker.PickSaveFileAsync())
        {
            if (auto stream = co_await file.OpenAsync(Windows::Storage::FileAccessMode::ReadWrite))
            {
                // Create the encoder from the stream.
                auto encoder = co_await BitmapEncoder::CreateAsync(BitmapEncoder::JpegEncoderId(), stream);

                RenderTargetBitmap renderTargetBitmap{};
                co_await renderTargetBitmap.RenderAsync(MainImage());

                IBuffer pixels = co_await renderTargetBitmap.GetPixelsAsync();
                auto newBuffer = SoftwareBitmap::CreateCopyFromBuffer
                (pixels, BitmapPixelFormat::Bgra8, Item().ImageProperties().Width(), Item().ImageProperties().Height());

                encoder.SetSoftwareBitmap(newBuffer);
                co_await encoder.FlushAsync();

                co_await Windows::Storage::CachedFileManager::CompleteUpdatesAsync(file);
            }
        }
    }
}