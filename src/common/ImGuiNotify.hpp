/**
 * @file ImGuiNotify.hpp
 * @brief A header-only library for creating toast notifications with ImGui.
 *
 * Based on imgui-notify by patrickcjk
 * https://github.com/patrickcjk/imgui-notify
 *
 * @version 0.0.3 by TyomaVader
 * @date 07.07.2024
 */

#pragma once

#include <vector> // Vector for storing notifications list
#include <string>
#include <chrono>     // For the notifications timed dissmiss
#include <functional> // For storing the code, which executest on the button click in the notification

#include <imgui_internal.h>

/**
 * CONFIGURATION SECTION Start
 */

#define NOTIFY_MAX_MSG_LENGTH 4096      // Max message content length
#define NOTIFY_PADDING_X 20.f           // Bottom-left X padding
#define NOTIFY_PADDING_Y 20.f           // Bottom-left Y padding
#define NOTIFY_PADDING_MESSAGE_Y 10.f   // Padding Y between each message
#define NOTIFY_FADE_IN_OUT_TIME 150     // Fade in and out duration
#define NOTIFY_DEFAULT_DISMISS 5000     // Auto dismiss after X ms (default, applied only of no data provided in constructors)
#define NOTIFY_OPACITY 0.8f             // 0-1 Toast opacity
#define NOTIFY_USE_SEPARATOR false      // If true, a separator will be rendered between the title and the content
#define NOTIFY_USE_DISMISS_BUTTON true // If true, a dismiss button will be rendered in the top right corner of the toast
#define NOTIFY_RENDER_LIMIT 5           // Max number of toasts rendered at the same time. Set to 0 for unlimited

// Warning: Requires ImGui docking with multi-viewport enabled
#define NOTIFY_RENDER_OUTSIDE_MAIN_WINDOW false // If true, the notifications will be rendered in the corner of the monitor, otherwise in the corner of the main window

/**
 * CONFIGURATION SECTION End
 */

static const ImGuiWindowFlags NOTIFY_DEFAULT_TOAST_FLAGS =
    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;

#define NOTIFY_NULL_OR_EMPTY(str) (!str || !strlen(str))
#define NOTIFY_FORMAT(fn, format, ...)   \
    if (format)                          \
    {                                    \
        va_list args;                    \
        va_start(args, format);          \
        fn(format, args, ##__VA_ARGS__); \
        va_end(args);                    \
    }

enum class ToastNotificationType : uint8_t
{
    None,
    Success,
    Warning,
    Error,
    Info,
    COUNT
};

enum class ImNotifyToastPhase : uint8_t
{
    FadeIn,
    Wait,
    FadeOut,
    Expired,
    COUNT
};

enum class ImNotifyToastPos : uint8_t
{
    TopLeft,
    TopCenter,
    TopRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    Center,
    COUNT
};

/**
 * @brief A class for creating toast notifications with ImGui.
 */
class ToastNotification
{
private:
    ImGuiWindowFlags flags = NOTIFY_DEFAULT_TOAST_FLAGS;

    ToastNotificationType type = ToastNotificationType::None;
    char title[NOTIFY_MAX_MSG_LENGTH];
    char content[NOTIFY_MAX_MSG_LENGTH];

    int dismissTime = NOTIFY_DEFAULT_DISMISS;
    std::chrono::system_clock::time_point creationTime = std::chrono::system_clock::now();

    std::function<void()> onButtonPress = nullptr; // A lambda variable, which will be executed when button in notification is pressed
    char buttonLabel[NOTIFY_MAX_MSG_LENGTH];

private:
    // Setters

    inline void setTitle(const char* format, va_list args) { vsnprintf(this->title, sizeof(this->title), format, args); }

    inline void setContent(const char* format, va_list args) { vsnprintf(this->content, sizeof(this->content), format, args); }

    inline void setButtonLabel(const char* format, va_list args) { vsnprintf(this->buttonLabel, sizeof(this->buttonLabel), format, args); }

public:
    /**
     * @brief Set the title of the toast notification.
     *
     * @param format The format string for the title.
     * @param ... The arguments for the format string.
     */
    inline void setTitle(const char* format, ...) { NOTIFY_FORMAT(this->setTitle, format); }

    inline void setTitle(const std::string& message) { this->setTitle(message.c_str()); }

    /**
     * @brief Set the content of the toast notification.
     *
     * @param format The format string for the content.
     * @param ... The arguments for the format string.
     */
    inline void setContent(const char* format, ...) { NOTIFY_FORMAT(this->setContent, format); }

    inline void setContent(const std::string& message) { this->setContent(message.c_str()); }

    /**
     * @brief Set the type of the toast notification.
     *
     * @param type The type of the toast notification.
     */
    inline void setType(const ToastNotificationType& type)
    {
        IM_ASSERT(type < ToastNotificationType::COUNT);
        this->type = type;
    };

    /**
     * @brief Set the ImGui window flags for the notification.
     *
     * @param flags ImGui window flags to set.
     */
    inline void setWindowFlags(const ImGuiWindowFlags& flags) { this->flags = flags; }

    /**
     * @brief Set the function to run on the button click in the notification.
     *
     * @param onButtonPress std::fuction or lambda expression, which contains the code for execution.
     */
    inline void setOnButtonPress(const std::function<void()>& onButtonPress) { this->onButtonPress = onButtonPress; }

    /**
     * @brief Set the label for the button in the notification.
     *
     * @param format The format string for the label.
     * @param ... The arguments for the format string.
     */
    inline void setButtonLabel(const char* format, ...) { NOTIFY_FORMAT(this->setButtonLabel, format); }

public:
    // Getters

    /**
     * @brief Get the title of the toast notification.
     *
     * @return const char* The title of the toast notification.
     */
    inline const char* getTitle() { return this->title; };

    /**
     * @brief Get the default title of the toast notification based on its type.
     *
     * @return const char* The default title of the toast notification.
     */
    inline const char* getDefaultTitle()
    {
        if (!strlen(this->title))
        {
            switch (this->type)
            {
            case ToastNotificationType::None: return nullptr;
            case ToastNotificationType::Success: return "Success";
            case ToastNotificationType::Warning: return "Warning";
            case ToastNotificationType::Error: return "Error";
            case ToastNotificationType::Info: return "Info";
            default: return nullptr;
            }
        }

        return this->title;
    };

    /**
     * @brief Get the type of the toast notification.
     *
     * @return ImGuiToastType The type of the toast notification.
     */
    inline ToastNotificationType getType() { return this->type; };

    /**
     * @brief Get the color of the toast notification based on its type.
     *
     * @return ImVec4 The color of the toast notification.
     */
    inline ImVec4 getColor()
    {
        switch (this->type)
        {
        case ToastNotificationType::None: return {255, 255, 255, 255};  // White
        case ToastNotificationType::Success: return {0, 255, 0, 255};   // Green
        case ToastNotificationType::Warning: return {255, 255, 0, 255}; // Yellow
        case ToastNotificationType::Error: return {255, 0, 0, 255};     // Error
        case ToastNotificationType::Info: return {0, 157, 255, 255};    // Blue
        default: return {255, 255, 255, 255};                    // White
        }
    }

    /**
     * @brief Get the icon of the toast notification based on its type.
     *
     * @return const char* The icon of the toast notification.
     */
    inline const char* getIcon()
    {
        switch (this->type)
        {
        case ToastNotificationType::None: return nullptr;
        case ToastNotificationType::Success: return ICON_MD_CHECK_CIRCLE; // Font Awesome 6
        case ToastNotificationType::Warning: return ICON_MD_ALERT;        // Font Awesome 6
        case ToastNotificationType::Error: return ICON_MD_ALERT_DECAGRAM; // Font Awesome 6
        case ToastNotificationType::Info: return ICON_MD_INFORMATION;     // Font Awesome 6
        default: return nullptr;
        }
    }

    /**
     * @brief Get the content of the toast notification.
     *
     * @return char* The content of the toast notification.
     */
    inline char* getContent() { return this->content; };

    /**
     * @brief Get the elapsed time in milliseconds since the creation of the object.
     *
     * @return int64_t The elapsed time in milliseconds.
     * @throws An exception with the message "Unsupported platform" if the platform is not supported.
     */
    inline std::chrono::nanoseconds getElapsedTime() { return std::chrono::system_clock::now() - this->creationTime; }

    /**
     * @brief Get the current phase of the toast notification based on the elapsed time since its creation.
     *
     * @return ImNotifyToastPhase The current phase of the toast notification.
     *         - ImNotifyToastPhase::FadeIn: The notification is fading in.
     *         - ImNotifyToastPhase::Wait: The notification is waiting to be dismissed.
     *         - ImNotifyToastPhase::FadeOut: The notification is fading out.
     *         - ImNotifyToastPhase::Expired: The notification has expired and should be removed.
     */
    inline ImNotifyToastPhase getPhase()
    {
        const int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(getElapsedTime()).count();

        if (elapsed > NOTIFY_FADE_IN_OUT_TIME + this->dismissTime + NOTIFY_FADE_IN_OUT_TIME)
        {
            return ImNotifyToastPhase::Expired;
        }
        else if (elapsed > NOTIFY_FADE_IN_OUT_TIME + this->dismissTime)
        {
            return ImNotifyToastPhase::FadeOut;
        }
        else if (elapsed > NOTIFY_FADE_IN_OUT_TIME)
        {
            return ImNotifyToastPhase::Wait;
        }
        else
        {
            return ImNotifyToastPhase::FadeIn;
        }
    }

    /**
     * Returns the percentage of fade for the notification.
     * @return The percentage of fade for the notification.
     */
    inline float getFadePercent()
    {
        const ImNotifyToastPhase phase = getPhase();
        const int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(getElapsedTime()).count();

        if (phase == ImNotifyToastPhase::FadeIn)
        {
            return ((float)elapsed / (float)NOTIFY_FADE_IN_OUT_TIME) * NOTIFY_OPACITY;
        }
        else if (phase == ImNotifyToastPhase::FadeOut)
        {
            return (1.f - (((float)elapsed - (float)NOTIFY_FADE_IN_OUT_TIME - (float)this->dismissTime) / (float)NOTIFY_FADE_IN_OUT_TIME)) * NOTIFY_OPACITY;
        }

        return 1.f * NOTIFY_OPACITY;
    }

    /**
     * @return ImGui window flags for the notification.
     */
    inline ImGuiWindowFlags getWindowFlags() { return this->flags; }

    /**
     * @return The function, which is executed on the button click in the notification.
     */
    inline std::function<void()> getOnButtonPress() { return this->onButtonPress; }

    /**
     * @return The label on the button in notification.
     */
    inline const char* getButtonLabel() { return this->buttonLabel; }

public:
    // Constructors

    /**
     * @brief Creates a new ToastNotification object with the specified type and dismiss time.
     *
     * @param type The type of the toast.
     * @param dismissTime The time in milliseconds after which the toast should be dismissed. Default is NOTIFY_DEFAULT_DISMISS.
     */
    ToastNotification(ToastNotificationType type, int dismissTime = NOTIFY_DEFAULT_DISMISS)
    {
        IM_ASSERT(type < ToastNotificationType::COUNT);

        this->type = type;
        this->dismissTime = dismissTime;

        this->creationTime = std::chrono::system_clock::now();

        memset(this->title, 0, sizeof(this->title));
        memset(this->content, 0, sizeof(this->content));
    }

    /**
     * @brief Constructor for creating an ToastNotification object with a specified type and message format.
     *
     * @param type The type of the toast message.
     * @param format The format string for the message.
     * @param ... The variable arguments to be formatted according to the format string.
     */
    ToastNotification(ToastNotificationType type, const char* format, ...)
        : ToastNotification(type)
    {
        NOTIFY_FORMAT(this->setContent, format);
    }

    /* @brief Constructor for creating an ToastNotification object with a specified type and std::string message.
     * @param dismissTime The time in milliseconds before the toast message is dismissed.
     * @param message The message to be displayed in the toast.
     */
    ToastNotification(ToastNotificationType type, const std::string& message)
        : ToastNotification(type)
    {
        this->setContent(message);
    }

    /**
     * @brief Constructor for creating a new ToastNotification object with a specified type, dismiss time, and content format.
     *
     * @param type The type of the toast message.
     * @param dismissTime The time in milliseconds before the toast message is dismissed.
     * @param format The format string for the content of the toast message.
     * @param ... The variable arguments to be formatted according to the format string.
     */
    ToastNotification(ToastNotificationType type, int dismissTime, const char* format, ...)
        : ToastNotification(type, dismissTime)
    {
        NOTIFY_FORMAT(this->setContent, format);
    }

    /* @brief Constructor for creating an ToastNotification object with a specified type, dismiss time, and std::string message.
     * @param type The type of the toast message.
     * @param dismissTime The time in milliseconds before the toast message is dismissed.
     * @param message The message to be displayed in the toast.
     */
    ToastNotification(ToastNotificationType type, int dismissTime, const std::string& message)
        : ToastNotification(type, dismissTime)
    {
        this->setContent(message);
    }

    /**
     * @brief Constructor for creating a new ToastNotification object with a specified type, dismiss time, title format, content format and a button.
     *
     * @param type The type of the toast message.
     * @param dismissTime The time in milliseconds before the toast message is dismissed.
     * @param buttonLabel The label for the button.
     * @param onButtonPress The lambda function to be executed when the button is pressed.
     * @param format The format string for the content of the toast message.
     * @param ... The variable arguments to be formatted according to the format string.
     */
    ToastNotification(ToastNotificationType type, int dismissTime, const char* buttonLabel, const std::function<void()>& onButtonPress, const char* format, ...)
        : ToastNotification(type, dismissTime)
    {
        NOTIFY_FORMAT(this->setContent, format);

        this->onButtonPress = onButtonPress;
        this->setButtonLabel(buttonLabel);
    }
};

namespace ImGui
{
inline std::vector<ToastNotification> notifications;

/**
 * Inserts a new notification into the notification queue.
 * @param toast The notification to be inserted.
 */
inline void InsertNotification(const ToastNotification& toast)
{
    notifications.push_back(toast);
}

/**
 * @brief Removes a notification from the list of notifications.
 *
 * @param index The index of the notification to remove.
 */
inline void RemoveNotification(int index)
{
    notifications.erase(notifications.begin() + index);
}

/**
 * Renders all notifications in the notifications vector.
 * Each notification is rendered as a toast window with a title, content and an optional icon.
 * If a notification is expired, it is removed from the vector.
 */
inline void RenderNotifications()
{
    const ImVec2 mainWindowSize = GetMainViewport()->Size;

    float height = 0.f;

    for (size_t i = 0; i < notifications.size(); ++i)
    {
        ToastNotification* currentToast = &notifications[i];

        // Remove toast if expired
        if (currentToast->getPhase() == ImNotifyToastPhase::Expired)
        {
            RemoveNotification(i);
            continue;
        }

#if NOTIFY_RENDER_LIMIT > 0
        if (i > NOTIFY_RENDER_LIMIT)
        {
            continue;
        }
#endif

        // Get icon, title and other data
        const char* icon = currentToast->getIcon();
        const char* title = currentToast->getTitle();
        const char* content = currentToast->getContent();
        const char* defaultTitle = currentToast->getDefaultTitle();
        const float opacity = currentToast->getFadePercent(); // Get opacity based of the current phase

        // Window rendering
        ImVec4 textColor = currentToast->getColor();
        textColor.w = opacity;

        // Generate new unique name for this toast
        char windowName[50];
#ifdef _WIN32
        sprintf_s(windowName, "##TOAST%d", (int)i);
#elif defined(__linux__) || defined(__EMSCRIPTEN__)
        std::sprintf(windowName, "##TOAST%d", (int)i);
#elif defined(__APPLE__)
        std::snprintf(windowName, 50, "##TOAST%d", (int)i);
#else
        throw "Unsupported platform";
#endif

        // PushStyleColor(ImGuiCol_Text, textColor);
        SetNextWindowBgAlpha(opacity);

#if NOTIFY_RENDER_OUTSIDE_MAIN_WINDOW
        short mainMonitorId = static_cast<ImGuiViewportP*>(GetMainViewport())->PlatformMonitor;

        ImGuiPlatformIO& platformIO = GetPlatformIO();
        ImGuiPlatformMonitor& monitor = platformIO.Monitors[mainMonitorId];

        // Set notification window position to bottom right corner of the monitor
        SetNextWindowPos(
            ImVec2(monitor.WorkPos.x + monitor.WorkSize.x - NOTIFY_PADDING_X, monitor.WorkPos.y + monitor.WorkSize.y - NOTIFY_PADDING_Y - height), ImGuiCond_Always,
            ImVec2(1.0f, 1.0f));
#else
        // Set notification window position to bottom right corner of the main window, considering the main window size and location in relation to the display
        ImVec2 mainWindowPos = GetMainViewport()->Pos;
        SetNextWindowPos(
            ImVec2(mainWindowPos.x + mainWindowSize.x - NOTIFY_PADDING_X, mainWindowPos.y + mainWindowSize.y - NOTIFY_PADDING_Y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
#endif

        // Set notification window flags
        if (!NOTIFY_USE_DISMISS_BUTTON && currentToast->getOnButtonPress() == nullptr)
        {
            currentToast->setWindowFlags(NOTIFY_DEFAULT_TOAST_FLAGS | ImGuiWindowFlags_NoInputs);
        }

        Begin(windowName, nullptr, currentToast->getWindowFlags());

        // Render over all other windows
        BringWindowToDisplayFront(GetCurrentWindow());

        // Here we render the toast content
        {
            PushTextWrapPos(mainWindowSize.x / 3.f); // We want to support multi-line text, this will wrap the text after 1/3 of the screen width

            bool wasTitleRendered = false;

            // If an icon is set
            if (!NOTIFY_NULL_OR_EMPTY(icon))
            {
                // Text(icon); // Render icon text
                TextColored(textColor, "%s", icon);
                wasTitleRendered = true;
            }

            // If a title is set
            if (!NOTIFY_NULL_OR_EMPTY(title))
            {
                // If a title and an icon is set, we want to render on same line
                if (!NOTIFY_NULL_OR_EMPTY(icon))
                    SameLine();

                Text("%s", title); // Render title text
                wasTitleRendered = true;
            }
            else if (!NOTIFY_NULL_OR_EMPTY(defaultTitle))
            {
                if (!NOTIFY_NULL_OR_EMPTY(icon))
                    SameLine();

                Text("%s", defaultTitle); // Render default title text (ImGuiToastType_Success -> "Success", etc...)
                wasTitleRendered = true;
            }

            // If a dismiss button is enabled
            if (NOTIFY_USE_DISMISS_BUTTON)
            {
                // If a title or content is set, we want to render the button on the same line
                if (wasTitleRendered || !NOTIFY_NULL_OR_EMPTY(content))
                {
                    SameLine();
                }

                // Render the dismiss button on the top right corner
                // NEEDS TO BE REWORKED
                float scale = 0.8f;

                if (CalcTextSize(content).x > GetContentRegionAvail().x)
                {
                    scale = 0.8f;
                }

                SetCursorPosX(GetCursorPosX() + (GetWindowSize().x - GetCursorPosX()) * scale);

                // If the button is pressed, we want to remove the notification
                if (Button(ICON_MD_CLOSE_BOX))
                {
                    RemoveNotification(i);
                }
            }

            // In case ANYTHING was rendered in the top, we want to add a small padding so the text (or icon) looks centered vertically
            if (wasTitleRendered && !NOTIFY_NULL_OR_EMPTY(content))
            {
                SetCursorPosY(GetCursorPosY() + 5.f); // Must be a better way to do this!!!!
            }

            // If a content is set
            if (!NOTIFY_NULL_OR_EMPTY(content))
            {
                if (wasTitleRendered)
                {
#if NOTIFY_USE_SEPARATOR
                    Separator();
#endif
                }

                Text("%s", content); // Render content text
            }

            // If a button is set
            if (currentToast->getOnButtonPress() != nullptr)
            {
                // If the button is pressed, we want to execute the lambda function
                if (Button(currentToast->getButtonLabel()))
                {
                    currentToast->getOnButtonPress()();
                }
            }

            PopTextWrapPos();
        }

        // Save height for next toasts
        height += GetWindowHeight() + NOTIFY_PADDING_MESSAGE_Y;

        // End
        End();
    }
}
} // namespace ImGui

namespace sol_ToastNotification
{
// rename this to be more user friendly
inline void ShowToast(const ToastNotification& toast)
{
    ImGui::InsertNotification(toast);
}

inline void BindImNotifyToast(sol::table aTable)
{
    aTable.new_usertype<ToastNotification>(
        "ToastNotification",
        sol::constructors<ToastNotification(ToastNotificationType),ToastNotification(ToastNotificationType, int),ToastNotification(ToastNotificationType,const std::string&),ToastNotification(ToastNotificationType,int,const std::string&)>(),
        "SetTitle", sol::resolve<void(const std::string&)>(&ToastNotification::setTitle),
        "SetContent", sol::resolve<void(const std::string&)>(&ToastNotification::setContent),
        "SetType", &ToastNotification::setType,
        "SetWindowFlags", &ToastNotification::setWindowFlags);

    aTable.new_enum(
        "ToastNotificationType", "None", ToastNotificationType::None, "Success", ToastNotificationType::Success, "Warning", ToastNotificationType::Warning, "Error", ToastNotificationType::Error, "Info", ToastNotificationType::Info);

    aTable.set_function("ShowToast", ShowToast);
}
} // namespace sol_ToastNotification
