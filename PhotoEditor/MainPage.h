/*
 * 主页面视图头文件
 */

#pragma once
#include "MainPage.g.h"

namespace winrt::PhotoEditor::implementation
{
	struct MainPage : MainPageT<MainPage>
	{
		MainPage();

		// 返回图片方法

		Windows::Foundation::Collections::IVector<Windows::Foundation::IInspectable> [[nodiscard]] photos() const
		{
			return photos_;
		}

		// 加载和渲染图片的事件句柄
		Windows::Foundation::IAsyncAction on_navigated_to(Windows::UI::Xaml::Navigation::NavigationEventArgs);
		Windows::Foundation::IAsyncAction on_container_content_changing(Windows::UI::Xaml::Controls::ListViewBase, Windows::UI::Xaml::Controls::ContainerContentChangingEventArgs);

		// 从详情页导航回来的动画
		Windows::Foundation::IAsyncAction start_connected_animation_for_back_navigation();

		// 事件更改的通知
		event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const&);
		void PropertyChanged(event_token const&);

		// 事件句柄
		void image_grid_view_item_click(Windows::Foundation::IInspectable const, Windows::UI::Xaml::Controls::ItemClickEventArgs const);

	private:
		// 加载图片和动画的函数
		Windows::Foundation::IAsyncAction get_items_async();
		Windows::UI::Composition::CompositionAnimationGroup create_offset_animation();
		Windows::Foundation::IAsyncOperation<PhotoEditor::Photo> load_image_info_async(Windows::Storage::StorageFile);

		// 图片集合字段
		Windows::Foundation::Collections::IVector<IInspectable> photos_{ nullptr };

		// 选中图片的字段
		PhotoEditor::Photo selected_item_{ nullptr };

		// 动画集合字段
		Windows::UI::Composition::ImplicitAnimationCollection element_implicit_animation_{ nullptr };

		// 页面Compositor字段
		Windows::UI::Composition::Compositor compositor_{ nullptr };

		// 事件
		event<Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::ListViewBase, Windows::UI::Xaml::Controls::ContainerContentChangingEventArgs>> handler_;

		// 属性更改通知
		void raise_property_changed(hstring const&);
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> property_changed_;

	};
}

namespace winrt::PhotoEditor::factory_implementation
{
	struct MainPage : MainPageT<MainPage, implementation::MainPage>
	{
	};
}