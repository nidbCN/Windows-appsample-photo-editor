#pragma once

// 使用std中的MIN MAX，避免与自定义的宏定义冲突。
#define NOMINMAX

// 避免VS Designer 编译器链接错误
#define _VSDESIGNER_DONT_LOAD_AS_DLL

// 引用的头文件
#include <Unknwn.h>
#include <hstring.h>

// WinRT 头文件
#include <winrt/coroutine.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Search.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.Effects.h>
#include <winrt/Microsoft.Graphics.Canvas.Text.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.Xaml.h>
#include <winrt/Windows.Graphics.Effects.h>
