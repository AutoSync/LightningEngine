// EditorUI.h - editor-facing UI facade.
//
// The editor should depend on this layer instead of including Titan headers
// directly. Today the implementation is Titan-backed; later this file can grow
// into a true adapter without rewriting every editor panel.
#pragma once

#include "../include/gui/TitanUI.h"
#include "../include/gui/widgets/Backdrop.h"
#include "../include/gui/widgets/Border.h"
#include "../include/gui/widgets/Button.h"
#include "../include/gui/widgets/Checkbox.h"
#include "../include/gui/widgets/ColorPicker.h"
#include "../include/gui/widgets/ContextMenu.h"
#include "../include/gui/widgets/DockSpace.h"
#include "../include/gui/widgets/Dropdown.h"
#include "../include/gui/widgets/FileDialog.h"
#include "../include/gui/widgets/Grid.h"
#include "../include/gui/widgets/HStack.h"
#include "../include/gui/widgets/Image.h"
#include "../include/gui/widgets/Label.h"
#include "../include/gui/widgets/List.h"
#include "../include/gui/widgets/MenuBar.h"
#include "../include/gui/widgets/Modal.h"
#include "../include/gui/widgets/NumericUpDown.h"
#include "../include/gui/widgets/Panel.h"
#include "../include/gui/widgets/ProgressBar.h"
#include "../include/gui/widgets/RichText.h"
#include "../include/gui/widgets/ScrollView.h"
#include "../include/gui/widgets/Separator.h"
#include "../include/gui/widgets/Slider.h"
#include "../include/gui/widgets/Splitter.h"
#include "../include/gui/widgets/StatusBar.h"
#include "../include/gui/widgets/TabStrip.h"
#include "../include/gui/widgets/Table.h"
#include "../include/gui/widgets/Text.h"
#include "../include/gui/widgets/TextField.h"
#include "../include/gui/widgets/TextureViewerWidget.h"
#include "../include/gui/widgets/Toolbar.h"
#include "../include/gui/widgets/TreeView.h"
#include "../include/gui/widgets/Viewport2D.h"
#include "../include/gui/widgets/VStack.h"
#include "../include/gui/widgets/Window.h"

namespace LightningEditor::UI {

using Runtime = Titan::TitanUI;
using Font = Titan::TitanFont;
using Color = Titan::Color;
using Style = Titan::Style;

inline Style& gStyle = Titan::gStyle;

using Widget = Titan::Widget;
using Backdrop = Titan::Backdrop;
using Border = Titan::Border;
using Button = Titan::Button;
using Checkbox = Titan::Checkbox;
using ColorPicker = Titan::ColorPicker;
using ContextMenu = Titan::ContextMenu;
using DockNode = Titan::DockNode;
using DockSpace = Titan::DockSpace;
using Dropdown = Titan::Dropdown;
using FileDialog = Titan::FileDialog;
using Grid = Titan::Grid;
using HStack = Titan::HStack;
using Image = Titan::Image;
using Label = Titan::Label;
using List = Titan::List;
using MenuBar = Titan::MenuBar;
using Modal = Titan::Modal;
using NumericUpDown = Titan::NumericUpDown;
using Panel = Titan::Panel;
using ProgressBar = Titan::ProgressBar;
using RichText = Titan::RichText;
using ScrollView = Titan::ScrollView;
using Separator = Titan::Separator;
using Slider = Titan::Slider;
using Splitter = Titan::Splitter;
using StatusBar = Titan::StatusBar;
using TabStrip = Titan::TabStrip;
using Table = Titan::Table;
using Text = Titan::Text;
using TextField = Titan::TextField;
using TextureViewerWidget = Titan::TextureViewerWidget;
using Toolbar = Titan::Toolbar;
using TreeNode = Titan::TreeNode;
using TreeView = Titan::TreeView;
using Viewport2D = Titan::Viewport2D;
using VStack = Titan::VStack;
using Window = Titan::Window;

using Titan::ClearUIFocusIfMatch;
using Titan::GetUIScrollY;
using Titan::SetUICapture;
using Titan::SetUIFocus;

} // namespace LightningEditor::UI
