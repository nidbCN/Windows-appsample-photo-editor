/*
 * 主页面视图
 */

#include "pch.h"
#include "MainPage.h"
#include "Photo.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Search;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Media::Animation;
using namespace Windows::UI::Xaml::Media::Imaging;

namespace winrt::PhotoEditor::implementation
{
    /// <summary>
    /// 构造函数
    /// </summary>
    MainPage::MainPage() : m_photos(winrt::single_threaded_observable_vector<IInspectable>()),
                           m_compositor(Window::Current().Compositor())
    {
    	// 初始化组件
        InitializeComponent();

    	// 设置视图源
        ParaView().Source(ForegroundElement());
    }

    // 加载用户图库中的图片集合
    IAsyncAction MainPage::OnNavigatedTo(NavigationEventArgs e)
    {
        // 如果没有预加载则加载图片
        if (Photos().Size() == 0)
        {
            m_elementImplicitAnimation = m_compositor.CreateImplicitAnimationCollection();

            // Define trigger and animation that should play when the trigger is triggered.
            m_elementImplicitAnimation.Insert(L"Offset", CreateOffsetAnimation());

        	// 加载图片元素
            co_await GetItemsAsync();
        }
    }

    /// <summary>
    /// 容器内容改变时候的异步事件
    /// </summary>
    /// <param name="sender">委托者</param>
    /// <param name="args">参数</param>
    /// <returns></returns>
    IAsyncAction MainPage::OnContainerContentChanging(ListViewBase sender, ContainerContentChangingEventArgs args)
    {
        // 获取元素视图
        const auto element_visual = ElementCompositionPreview::GetElementVisual(args.ItemContainer());
        // 获取图片
    	const auto image = args.ItemContainer().ContentTemplateRoot().as<Image>();

    	// 在回收队列中
        if (args.InRecycleQueue())
        {
        	// 取消引用
            element_visual.ImplicitAnimations(nullptr);

            image.Source(nullptr);
        }

    	// 阶段0（隐藏图片）
        if (args.Phase() == 0)
        {
            //对每个元素添加隐藏动画
            element_visual.ImplicitAnimations(m_elementImplicitAnimation);

        	// 回调
            args.RegisterUpdateCallback([&](auto sender, auto args) {
                OnContainerContentChanging(sender, args);
            });

        	// 设置句柄
            args.Handled(true);
        }

    	// 阶段1（显示图片）
        if (args.Phase() == 1)
        {

        	// 设置透明度为100，显示图片
            image.Opacity(100);
        	
            // 对元素拆箱
            const auto item = unbox_value<PhotoEditor::Photo>(args.Item());
            // 将类型转换为图片
            Photo *converted_photo_type = from_abi<Photo>(item);

            try
            {
            	// 获取略缩图
                const auto photo_small = co_await converted_photo_type->GetImageThumbnailAsync();
                // 设置显示的图片为略缩图
            	image.Source(photo_small);
            }
            catch (winrt::hresult_error)
            {
            	// 文件无法正常转换为Bitmap略缩图（也就是文件不是正常图片）
                const BitmapImage error_image{};

            	// 默认图片的URI
                const Uri error_image_uri{image.BaseUri().AbsoluteUri(), L"Assets/StoreLogo.png"};

            	// 设置显示的图片为默认图片
            	error_image.UriSource(error_image_uri);
                image.Source(error_image);
            }
        }
    }

    // Called by the Loaded event of the ImageGridView for animation after back navigation from DetailPage view.
    IAsyncAction MainPage::StartConnectedAnimationForBackNavigation()
    {
        // Run the connected animation for navigation back to the main page from the detail page.
        if (m_persistedItem)
        {
            ImageGridView().ScrollIntoView(m_persistedItem);
            auto animation = ConnectedAnimationService::GetForCurrentView().GetAnimation(L"backAnimation");
            if (animation)
            {
                co_await ImageGridView().TryStartConnectedAnimationAsync(animation, m_persistedItem, L"ItemImage");
            }
        }
    }

    // Registers property changed event handler.
    event_token MainPage::PropertyChanged(PropertyChangedEventHandler const &value)
    {
        return m_propertyChanged.add(value);
    }

    // Unregisters property changed event handler.
    void MainPage::PropertyChanged(event_token const &token)
    {
        m_propertyChanged.remove(token);
    }

    // Loads images from the user's Pictures library.
    IAsyncAction MainPage::GetItemsAsync()
    {
        // Show the loading progress bar.
        LoadProgressIndicator().Visibility(Windows::UI::Xaml::Visibility::Visible);
        NoPicsText().Visibility(Windows::UI::Xaml::Visibility::Collapsed);

        // File type filter.
        QueryOptions options{};
        options.FolderDepth(FolderDepth::Deep);
        options.FileTypeFilter().Append(L".jpg");
        options.FileTypeFilter().Append(L".png");
        options.FileTypeFilter().Append(L".gif");

        // Get the Pictures library.
        StorageFolder picturesFolder = KnownFolders::PicturesLibrary();
        auto result = picturesFolder.CreateFileQueryWithOptions(options);
        auto imageFiles = co_await result.GetFilesAsync();
        auto unsupportedFilesFound = false;

        // Populate Photos collection.
        for (auto &&file : imageFiles)
        {
            // Only files on the local computer are supported.
            // Files on OneDrive or a network location are excluded.
            if (file.Provider().Id() == L"computer")
            {
                // Gaein nidb: Async load image.
                auto image = co_await LoadImageInfoAsync(file);
                // Gaein nidb: Add the image into IVector.
                Photos().Append(image);
            }
            else
            {
                unsupportedFilesFound = true;
            }
        }

        if (Photos().Size() == 0)
        {
            // No pictures were found in the library, so show message.
            NoPicsText().Visibility(Windows::UI::Xaml::Visibility::Visible);
        }

        // Hide the loading progress bar.
        LoadProgressIndicator().Visibility(Windows::UI::Xaml::Visibility::Collapsed);

        if (unsupportedFilesFound)
        {
            // Gaein nidb: Set unsupported dialog.
            ContentDialog unsupportedFilesDialog{};
            unsupportedFilesDialog.Title(box_value(L"存在不支持的图片！"));
            unsupportedFilesDialog.Content(box_value(L"仅支持本地图片，我们查找到了在OneDrive或者其它网络中的图片。我们不会为您加载这些图片。"));
            unsupportedFilesDialog.CloseButtonText(L"确定");

            co_await unsupportedFilesDialog.ShowAsync();
        }
    }

    // Creates a Photo from Storage file for adding to Photo collection.
    IAsyncOperation<PhotoEditor::Photo> MainPage::LoadImageInfoAsync(StorageFile file)
    {
        auto properties = co_await file.Properties().GetImagePropertiesAsync();
        auto info = winrt::make<Photo>(properties, file, file.DisplayName(), file.DisplayType());
        co_return info;
    }

    CompositionAnimationGroup MainPage::CreateOffsetAnimation()
    {
        //Define Offset Animation for the Animation group.
        Vector3KeyFrameAnimation offsetAnimation = m_compositor.CreateVector3KeyFrameAnimation();
        offsetAnimation.InsertExpressionKeyFrame(1.0f, L"this.FinalValue");
        TimeSpan span{std::chrono::milliseconds{400}};
        offsetAnimation.Duration(span);

        //Define Animation Target for this animation to animate using definition.
        offsetAnimation.Target(L"Offset");

        //Add Animations to Animation group.
        CompositionAnimationGroup animationGroup = m_compositor.CreateAnimationGroup();
        animationGroup.Add(offsetAnimation);

        return animationGroup;
    }

    // Photo clicked event handler for navigation to DetailPage view.
    void MainPage::ImageGridView_ItemClick(IInspectable const sender, ItemClickEventArgs const e)
    {
        // Prepare the connected animation for navigation to the detail page.
        m_persistedItem = e.ClickedItem().as<PhotoEditor::Photo>();
        ImageGridView().PrepareConnectedAnimation(L"itemAnimation", e.ClickedItem(), L"ItemImage");

        auto m_suppress = SuppressNavigationTransitionInfo();
        Frame().Navigate(xaml_typename<PhotoEditor::DetailPage>(), e.ClickedItem(), m_suppress);
    }

    // Triggers property changed notification.
    void MainPage::RaisePropertyChanged(hstring const &propertyName)
    {
        m_propertyChanged(*this, PropertyChangedEventArgs(propertyName));
    }
}