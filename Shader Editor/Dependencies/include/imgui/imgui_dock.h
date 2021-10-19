#pragma once

// Forward declarations
typedef int ImGuiWindowFlags;
    
typedef enum ImGuiDockSlot {
    ImGuiDockSlot_Left,
    ImGuiDockSlot_Right,
    ImGuiDockSlot_Top,
    ImGuiDockSlot_Bottom,
    ImGuiDockSlot_Tab,

    ImGuiDockSlot_Float,
    ImGuiDockSlot_None
} ImGuiDockSlot;

namespace ImGui{

bool BeginDockspace();
void EndDockspace();
void ShutdownDock();
void SetNextDock( const char* dock_panel , ImGuiDockSlot slot );
bool BeginDock(const char* label, bool* opened = NULL, ImGuiWindowFlags extra_flags = 0);
void EndDock();
void DockDebugWindow(const char* dock_panel);
void InitDock();

};