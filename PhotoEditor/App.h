﻿#pragma once
#include "App.xaml.g.h"

namespace winrt::PhotoEditor::implementation
{
    struct App : public AppT<App>
    {
        App();

        void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&);
        void OnNavigationFailed(IInspectable const&, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs const&);
    };
}