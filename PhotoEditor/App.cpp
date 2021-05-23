#include "pch.h"

#include "App.h"
#include "MainPage.h"

using namespace winrt;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace PhotoEditor;
using namespace PhotoEditor::implementation;

/// <summary>
/// 初始化程序
/// </summary>
App::App()
{
    InitializeComponent();


}

/// <summary>
/// 当程序被用户启动时委托执行。
/// </summary>
/// <param name="e">具体信息</param>
void App::OnLaunched(LaunchActivatedEventArgs const &e)
{
    // 定义一个新根框架
    Frame rootFrame{nullptr};

    // 获取窗口内容
    auto content = Window::Current().Content();

    if (content)
    {
        // 转换内容为根框架
        rootFrame = content.try_as<Frame>();
    }

    // 无内容
    if (rootFrame == nullptr)
    {
        // 创建
        rootFrame = Frame();

        // 导航失败时候委托执行方法 OnNavigationFailed
        rootFrame.NavigationFailed({this, &App::OnNavigationFailed});


        if (e.PrelaunchActivated() == false)
        {
            if (rootFrame.Content() == nullptr)
            {
                // 导航到首页
                rootFrame.Navigate(xaml_typename<PhotoEditor::MainPage>(), box_value(e.Arguments()));
            }
            // 在当前窗口中放置框架
            Window::Current().Content(rootFrame);
            // 确保当前窗口被激活
            Window::Current().Activate();
        }
    }
    else
    {
        if (e.PrelaunchActivated() == false)
        {
            if (rootFrame.Content() == nullptr)
            {
                // 导航到首页
                rootFrame.Navigate(xaml_typename<PhotoEditor::MainPage>(), box_value(e.Arguments()));
            }
            // 确保当前窗口被激活
            Window::Current().Activate();
        }
    }
}

/// <summary>
/// 导航到页面失败的时候委托执行
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void App::OnNavigationFailed(IInspectable const &, NavigationFailedEventArgs const &e)
{
    throw hresult_error(E_FAIL, hstring(L"加载页面失败") + e.SourcePageType().Name);
}

/*
 * 根据 MIT 协议，本项目参考了 Microsoft 的部分代码
 */
