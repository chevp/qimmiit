#include <coregfx/imstudio/imstudio_debug.hpp>
#include <coregfx/core/ocean_config.hpp>

// Function to apply styles if provided in the Proto message
void ImStudio::applyStyle(const cgfx::Style& style) {
    if (style.has_text_color()) {
        const auto& color = style.text_color();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r(), color.g(), color.b(), color.a()));
    }

    if (style.has_background_color()) {
        const auto& color = style.background_color();
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(color.r(), color.g(), color.b(), color.a()));
    }

    if (style.has_button_color()) {
        const auto& color = style.button_color();
        ImVec4 base = ImVec4(color.r(), color.g(), color.b(), color.a());
        ImGui::PushStyleColor(ImGuiCol_Button, base);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(base.x * 1.1f, base.y * 1.1f, base.z * 1.1f, base.w));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(base.x * 0.9f, base.y * 0.9f, base.z * 0.9f, base.w));
    }

    if (style.padding() != 0.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.padding(), style.padding()));
    }

    if (style.border_radius() != 0.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.border_radius());
    }
    if (style.has_button_color()) {
        const auto& color = style.button_color();
        ImVec4 base = ImVec4(color.r(), color.g(), color.b(), color.a());
        ImGui::PushStyleColor(ImGuiCol_Button, base);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(base.x * 1.1f, base.y * 1.1f, base.z * 1.1f, base.w));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(base.x * 0.9f, base.y * 0.9f, base.z * 0.9f, base.w));
    }

    if (style.padding() != 0.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.padding(), style.padding()));
    }

    if (style.border_radius() != 0.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.border_radius());
    }

    if (style.border_size() > 0.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.border_size());
    }

    if (style.has_border_color()) {
        const auto& color = style.border_color();
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(color.r(), color.g(), color.b(), color.a()));
    }
}

// Function to reset any applied styles
void ImStudio::resetStyle(const cgfx::Style& style) {
    int popColorCount = 0;

    if (style.has_text_color()) popColorCount++;
    if (style.has_background_color()) popColorCount++;

    // Button colors: Button, Hovered, Active
    if (style.has_button_color()) popColorCount += 3;

    // Border color
    if (style.has_border_color()) popColorCount++;

    if (popColorCount > 0) {
        ImGui::PopStyleColor(popColorCount);
    }

    int popVarCount = 0;

    if (style.padding() != 0.0f) popVarCount++;
    if (style.border_radius() != 0.0f) popVarCount++;

    // Border size (FrameBorderSize)
    if (style.border_size() > 0.0f) popVarCount++;

    if (popVarCount > 0) {
        ImGui::PopStyleVar(popVarCount);
    }
}

// Load a font with a specific size
ImFont* ImStudio::loadFontWithSize(float size) {
    // Load a font with a specific size using ConfigProvider for asset path
    std::string fontPath = ConfigProvider::getAssetRoot() + "/fonts/Roboto-Medium.ttf";
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath.c_str(), size);
    return font;
}

void ImStudio::renderImGuiFromProto(const cgfx::Layout& layout,
    const std::function<void(const std::string&)>& callback) {
    const cgfx::Window& windowConfig = layout.window();

    // Set window size and position
    ImGui::SetNextWindowSize(ImVec2(windowConfig.size().x(), windowConfig.size().y()));
    ImGui::SetNextWindowPos(ImVec2(windowConfig.position().x(), windowConfig.position().y()));

    // Apply the window style if provided
    if (windowConfig.has_style()) {
        applyStyle(windowConfig.style());
    }

    // Begin window rendering
    ImGui::Begin(windowConfig.title().c_str());

    // Iterate over elements and render each UI element
    for (const auto& element : windowConfig.elements()) {
        // Apply styles if present for each element
        if (element.has_style()) {
            applyStyle(element.style());
        }

        // Text element
        if (element.has_text()) {
            ImGui::Text("%s", element.text().value().c_str());
        }

        // Button element
        if (element.has_button()) {
            const cgfx::ButtonElement& buttonConfig = element.button();
            ImVec2 buttonSize(buttonConfig.size().x(), buttonConfig.size().y());
            if (ImGui::Button(buttonConfig.label().c_str(), buttonSize)) {
                if (buttonConfig.callback() == "onButtonClick") {
                    // onButtonClick();  // Call button click handler
                }
                else if (buttonConfig.callback() == "sendEvent") {
                    if (!buttonConfig.uri().empty())
                    {
                        // Call the provided callback function
                        callback(buttonConfig.uri());
                    }
                }
            }
        }

        // InputText element
        if (element.has_input_text()) {
            const cgfx::InputTextElement& inputConfig = element.input_text();
            //ImGui::InputText(inputConfig.label().c_str(), usernameBuffer, inputConfig.buffer_size());
        }

        // SliderFloat element
        if (element.has_slider_float()) {
            const cgfx::SliderFloatElement& sliderConfig = element.slider_float();
            //ImGui::SliderFloat(sliderConfig.label().c_str(),
            //    &sliderValue,
            //    sliderConfig.range().min(),
            //    sliderConfig.range().max());
        }

        // Reset any style changes after rendering an element
        if (element.has_style()) {
            resetStyle(element.style());
        }
    }

    // Reset any style changes for the window
    if (windowConfig.has_style()) {
        resetStyle(windowConfig.style());
    }

    ImGui::End();
}

// Function to initialize a default Layout
/*cgfx::Layout ImStudio::createDefaultLayout(void) {
    cgfx::Layout layout;

    // Initialize the default Window
    cgfx::Window* window = layout.mutable_window();
    window->set_title("Default Window");

    // Set default size and position
    cgfx::Vec2f* size = window->mutable_size();
    size->set_x(200.0f); // Default width
    size->set_y(400.0f); // Default height

    cgfx::Vec2f* position = window->mutable_position();
    position->set_x(100.0f); // Default X position
    position->set_y(300.0f); // Default Y position

    // Initialize default Style for Window
    cgfx::Style* windowStyle = window->mutable_style();
    cgfx::Color* backgroundColor = windowStyle->mutable_background_color();
    backgroundColor->set_r(0.0863f);
    backgroundColor->set_g(0.149f);
    backgroundColor->set_b(0.1961f);
    backgroundColor->set_a(1.0f);

    // Default padding and border radius
    windowStyle->set_padding(10.0f);
    windowStyle->set_border_radius(5.0f);

    // Add default UI elements
    cgfx::UIElement* textElement = window->add_elements();
    cgfx::TextElement* text = textElement->mutable_text();
    text->set_value("Welcome to the default layout!");

    // Set default Style for TextElement
    cgfx::Style* textStyle = textElement->mutable_style();
    cgfx::Color* textColor = textStyle->mutable_text_color();
    textColor->set_r(1.0f); // White
    textColor->set_g(1.0f);
    textColor->set_b(1.0f);
    textColor->set_a(1.0f);
    textStyle->set_font_size(30.0f);

    // Add and configure a button
    cgfx::UIElement* buttonElement = window->add_elements();
    cgfx::ButtonElement* button = buttonElement->mutable_button();
    button->set_label("Ready");

    cgfx::Vec2f* buttonSize = button->mutable_size();
    buttonSize->set_x(150.0f);
    buttonSize->set_y(50.0f);
    button->set_callback("sendEvent");
    button->set_uri("ready/952a9391695f35b03369b3");

    // Set style for the button
    cgfx::Style* buttonStyle = buttonElement->mutable_style();

    // Button background color (#6CEA75)
    cgfx::Color* buttonColor = buttonStyle->mutable_button_color();
    buttonColor->set_r(0.4235f); // 108 / 255
    buttonColor->set_g(0.9176f); // 234 / 255
    buttonColor->set_b(0.4588f); // 117 / 255
    buttonColor->set_a(1.0f);    // fully opaque

    // Border color
    cgfx::Color* borderColor = buttonStyle->mutable_border_color();
    borderColor->set_r(1.0f); // white border for contrast
    borderColor->set_g(1.0f);
    borderColor->set_b(1.0f);
    borderColor->set_a(1.0f);

    // Border size
    buttonStyle->set_border_size(2.0f); // 2px thick border

    // Optional: Rounded corners
    buttonStyle->set_border_radius(5.0f);

    // button->set_callback("onButtonClick");
    button->set_callback("sendEvent");
    button->set_uri("ready/952a9391695f35b03369b3");

    // Set style for the button
    cgfx::Style* buttonStyle2 = buttonElement->mutable_style();

    // Button background color (#6CEA75)
    cgfx::Color* buttonColor2 = buttonStyle2->mutable_button_color();
    buttonColor2->set_r(0.4235f); // 108 / 255
    buttonColor2->set_g(0.9176f); // 234 / 255
    buttonColor2->set_b(0.4588f); // 117 / 255
    buttonColor2->set_a(1.0f);    // fully opaque

    // Border color
    cgfx::Color* borderColor2 = buttonStyle2->mutable_border_color();
    borderColor2->set_r(1.0f); // white border for contrast
    borderColor2->set_g(1.0f);
    borderColor2->set_b(1.0f);
    borderColor2->set_a(1.0f);

    // Border size
    buttonStyle2->set_border_size(2.0f); // 2px thick border

    // Optional: Rounded corners
    buttonStyle2->set_border_radius(5.0f);

    cgfx::UIElement* buttonElement2 = window->add_elements();
    cgfx::ButtonElement* button2 = buttonElement2->mutable_button();
    button2->set_label("Clear");
    cgfx::Vec2f* buttonSize2 = button2->mutable_size();
    buttonSize2->set_x(150.0f); // Default button width
    buttonSize2->set_y(50.0f);  // Default button height
    button2->set_callback("sendEvent");
    button2->set_uri("clear");

    cgfx::UIElement* buttonElement3 = window->add_elements();
    cgfx::ButtonElement* button3 = buttonElement3->mutable_button();
    button3->set_label("Load");
    cgfx::Vec2f* buttonSize3 = button3->mutable_size();
    buttonSize3->set_x(150.0f); // Default button width
    buttonSize3->set_y(50.0f);  // Default button height
    button3->set_callback("sendEvent");
    button3->set_uri("load/952a9391695f35b03369b3");

    // Set default Style for ButtonElement
    cgfx::Style* buttonStyle4 = buttonElement->mutable_style();
    cgfx::Color* buttonBgColor4 = buttonStyle2->mutable_background_color();
    buttonBgColor4->set_r(0.1686f);
    buttonBgColor4->set_g(0.4f);
    buttonBgColor4->set_b(0.298f);
    buttonBgColor4->set_a(1.0f);
    buttonStyle4->set_border_radius(5.0f);

    return layout;
}*/

cgfx::Layout ImStudio::createDefaultLayout2(void) {
    cgfx::Layout layout;

    // Initialize the default Window
    cgfx::Window* window = layout.mutable_window();
    window->set_title("Default Window");

    // Set default size and position
    cgfx::Vec2f* size = window->mutable_size();
    size->set_x(200.0f); // Default width
    size->set_y(400.0f); // Default height

    cgfx::Vec2f* position = window->mutable_position();
    position->set_x(100.0f); // Default X position
    position->set_y(100.0f); // Default Y position

    // Initialize default Style for Window
    cgfx::Style* windowStyle = window->mutable_style();
    cgfx::Color* backgroundColor = windowStyle->mutable_background_color();
    backgroundColor->set_r(0.0863f);
    backgroundColor->set_g(0.149f);
    backgroundColor->set_b(0.1961f);
    backgroundColor->set_a(1.0f);

    // Default padding and border radius
    windowStyle->set_padding(10.0f);
    windowStyle->set_border_radius(5.0f);

    // Add default UI elements
    cgfx::UIElement* textElement = window->add_elements();
    cgfx::TextElement* text = textElement->mutable_text();
    text->set_value("Welcome to the default layout!");

    // Set default Style for TextElement
    cgfx::Style* textStyle = textElement->mutable_style();
    cgfx::Color* textColor = textStyle->mutable_text_color();
    textColor->set_r(1.0f); // White
    textColor->set_g(1.0f);
    textColor->set_b(1.0f);
    textColor->set_a(1.0f);
    textStyle->set_font_size(30.0f);

    cgfx::UIElement* buttonElement = window->add_elements();
    cgfx::ButtonElement* button = buttonElement->mutable_button();
    button->set_label("Ready");
    cgfx::Vec2f* buttonSize = button->mutable_size();
    buttonSize->set_x(150.0f); // Default button width
    buttonSize->set_y(50.0f);  // Default button height
    // button->set_callback("onButtonClick");
    button->set_callback("sendEvent");
    button->set_uri("ready/952a9391695f35b03369b3");

    cgfx::UIElement* buttonElement2 = window->add_elements();
    cgfx::ButtonElement* button2 = buttonElement2->mutable_button();
    button2->set_label("Clear");
    cgfx::Vec2f* buttonSize2 = button2->mutable_size();
    buttonSize2->set_x(150.0f); // Default button width
    buttonSize2->set_y(50.0f);  // Default button height
    button2->set_callback("sendEvent");
    button2->set_uri("clear");

    cgfx::UIElement* buttonElement3 = window->add_elements();
    cgfx::ButtonElement* button3 = buttonElement3->mutable_button();
    button3->set_label("Load Scene");
    cgfx::Vec2f* buttonSize3 = button3->mutable_size();
    buttonSize3->set_x(150.0f); // Default button width
    buttonSize3->set_y(50.0f);  // Default button height
    button3->set_callback("sendEvent");
    button3->set_uri("load/952a9391695f35b03369b3");

    // Set default Style for ButtonElement
    cgfx::Style* buttonStyle = buttonElement->mutable_style();
    cgfx::Color* buttonBgColor = buttonStyle->mutable_background_color();
    buttonBgColor->set_r(0.1686f);
    buttonBgColor->set_g(0.4f);
    buttonBgColor->set_b(0.298f);
    buttonBgColor->set_a(1.0f);
    buttonStyle->set_border_radius(5.0f);

    return layout;
}