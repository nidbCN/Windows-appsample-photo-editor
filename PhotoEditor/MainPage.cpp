/*
 * 主页面视图代码
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
    MainPage::MainPage() : photos_(winrt::single_threaded_observable_vector<IInspectable>()),
                           compositor_(Window::Current().Compositor())
    {
    	// 初始化组件
        InitializeComponent();

        // 加载图片
        get_items_async();

    	// 设置视图源
        ParaView().Source(ForegroundElement());
    }

    /// <summary>
    /// 加载图库中的用户图片集合
    /// </summary>
    /// <param name="e"></param>
    /// <returns></returns>
    IAsyncAction MainPage::on_navigated_to(NavigationEventArgs e)
    {
        // 如果没有预加载则加载图片
        if (photos().Size() == 0)
        {
            element_implicit_animation_ = compositor_.CreateImplicitAnimationCollection();

            // Define trigger and animation that should play when the trigger is triggered.
            element_implicit_animation_.Insert(L"Offset", create_offset_animation());

        	// 加载图片元素
            co_await get_items_async();
        }
    }

    /// <summary>
    /// 容器内容改变时候的异步事件
    /// </summary>
    /// <param name="sender">委托者</param>
    /// <param name="args">参数</param>
    /// <returns></returns>
    IAsyncAction MainPage::on_container_content_changing(ListViewBase sender, ContainerContentChangingEventArgs args)
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
            element_visual.ImplicitAnimations(element_implicit_animation_);

        	// 回调
            args.RegisterUpdateCallback([&](auto sender, auto args) {
                on_container_content_changing(sender, args);
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

    /// <summary>
    /// 从详情页返回主页面调用的动画
    /// </summary>
    /// <returns></returns>
    IAsyncAction MainPage::start_connected_animation_for_back_navigation()
    {
        // 运行动画
        if (selected_item_)
        {
            ImageGridView().ScrollIntoView(selected_item_);
        	// 创建动画并异步执行
            if (const auto animation = ConnectedAnimationService::GetForCurrentView().GetAnimation(L"backAnimation"); animation)
            {
                co_await ImageGridView().TryStartConnectedAnimationAsync(animation, selected_item_, L"ItemImage");
            }
        }
    }

    // 注册属性改动事件句柄
    event_token MainPage::PropertyChanged(PropertyChangedEventHandler const &value)
    {
        return property_changed_.add(value);
    }

    // 取消注册属性改动事件句柄
    void MainPage::PropertyChanged(event_token const &token)
    {
        property_changed_.remove(token);
    }

    // 从用户图库中加载图片     
    IAsyncAction MainPage::get_items_async()
    {
        // 显示加载进度条
        LoadProgressIndicator().Visibility(Windows::UI::Xaml::Visibility::Visible);
        NoPicsText().Visibility(Windows::UI::Xaml::Visibility::Collapsed);

        // 文件后缀名过滤
        const QueryOptions options{};
        // 设置扫描深度
        options.FolderDepth(FolderDepth::Deep);   
        options.FileTypeFilter().Append(L".jpg");
        options.FileTypeFilter().Append(L".png");
        options.FileTypeFilter().Append(L".gif");

        // 从图库中获取图片
        const auto result = KnownFolders::PicturesLibrary().CreateFileQueryWithOptions(options);
        const auto &image_files = co_await result.GetFilesAsync();
        auto has_unsupported_files = false;

        // 填充图片集合
        for (auto &&file : image_files)
        {
        	// 仅使用本机（computer）的填充，OneDrive和远程目录的图片不算在内
            if (file.Provider().Id() == L"computer")
            {
                // 异步读取图片并添加
                photos().Append(co_await load_image_info_async(file));
            }
            else
            {
            	// 非本机图片，不受支持
                has_unsupported_files = true;
            }
        }

    	// 没有找到文件
        if (photos().Size() == 0)
        {
            // 显示消息
            NoPicsText().Visibility(Windows::UI::Xaml::Visibility::Visible);
        }

        // 隐藏加载进度条
        LoadProgressIndicator().Visibility(Windows::UI::Xaml::Visibility::Collapsed);

    	// 存在不支持的文件，显示对话框
        if (has_unsupported_files)
        {
        	// 设置对话框
            const ContentDialog unsupported_files_dialog{};
            unsupported_files_dialog.Title(box_value(L"存在不支持的图片！"));
            unsupported_files_dialog.Content(box_value(L"仅支持本地图片，查找到了在OneDrive或者其它远程的图片。无法加载这些图片。"));
            unsupported_files_dialog.CloseButtonText(L"确定");

        	// 弹出对话框
            co_await unsupported_files_dialog.ShowAsync();
        }
    }

    /// <summary>
    /// 从StorageFile创建Photo类型
    /// </summary>
    /// <param name="file">存储文件</param>
    /// <returns>Photo</returns>
    IAsyncOperation<PhotoEditor::Photo> MainPage::load_image_info_async(StorageFile file)
    {
        auto properties = co_await file.Properties().GetImagePropertiesAsync();
        co_return winrt::make<Photo>(properties, file, file.DisplayName(), file.DisplayType());
    }

    /// <summary>
    /// 创建偏移动画
    /// </summary>
    /// <returns></returns>
    CompositionAnimationGroup MainPage::create_offset_animation()
    {
        // 定义偏移动画
        const auto offset_animation = compositor_.CreateVector3KeyFrameAnimation();
    	// 时间片段
        const TimeSpan span{ std::chrono::milliseconds{400} };
    	
        offset_animation.InsertExpressionKeyFrame(1.0f, L"this.FinalValue");
        offset_animation.Duration(span);

        // 定义动画目标
        offset_animation.Target(L"Offset");

        // 将动画添加到动画组中
        auto animation_group = compositor_.CreateAnimationGroup();
        animation_group.Add(offset_animation);

        return animation_group;
    }

    // 点击图片导航到详情页的事件
    void MainPage::image_grid_view_item_click(IInspectable const sender, ItemClickEventArgs const e)
    {
        // 过渡动画
        selected_item_ = e.ClickedItem().as<PhotoEditor::Photo>();
    	
        // ReSharper disable once CppExpressionWithoutSideEffects
        ImageGridView().PrepareConnectedAnimation(L"itemAnimation", e.ClickedItem(), L"ItemImage");

        const auto m_suppress = SuppressNavigationTransitionInfo();
        Frame().Navigate(xaml_typename<PhotoEditor::DetailPage>(), e.ClickedItem(), m_suppress);
    }

    /// <summary>
    /// 属性变更事件通知
    /// </summary>
    /// <param name="propertyName">属性名</param>
    void MainPage::raise_property_changed(hstring const &propertyName)
    {
        property_changed_(*this, PropertyChangedEventArgs(propertyName));
    }
}