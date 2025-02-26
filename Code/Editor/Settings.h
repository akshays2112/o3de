/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


#pragma once
#ifndef CRYINCLUDE_EDITOR_SETTINGS_H
#define CRYINCLUDE_EDITOR_SETTINGS_H
#include "SettingsManager.h"

#include <QColor>
#include <QFont>
#include <QRect>
#include <QSettings>

#include <AzToolsFramework/Editor/EditorSettingsAPIBus.h>
#include <AzToolsFramework/Prefab/PrefabLoaderInterface.h>
#include <AzCore/JSON/document.h>

#include <AzQtComponents/Components/Widgets/ToolBar.h>

//////////////////////////////////////////////////////////////////////////
// Settings for snapping in the viewports.
//////////////////////////////////////////////////////////////////////////
struct SSnapSettings
{
    SSnapSettings()
    {
        constructPlaneDisplay = false;
        constructPlaneSize = 5;

        markerDisplay = false;
        markerColor = QColor(0, 200, 200);
        markerSize = 1.0f;

        bGridUserDefined = false;
        bGridGetFromSelected = false;
    }

    // Display settings for construction plane.
    bool  constructPlaneDisplay;
    float constructPlaneSize;

    // Display settings for snapping marker.
    bool  markerDisplay;
    QColor markerColor;
    float markerSize;

    bool bGridUserDefined;
    bool bGridGetFromSelected;
};


//////////////////////////////////////////////////////////////////////////
// Settings for View of Tools
//////////////////////////////////////////////////////////////////////////
struct SToolViewSettings
{
    SToolViewSettings()
    {
    }

    int nFrameRate;
    QString codec;
};

//////////////////////////////////////////////////////////////////////////
// Settings for deep selection.
//////////////////////////////////////////////////////////////////////////
struct SDeepSelectionSettings
{
    SDeepSelectionSettings()
        : fRange(1.f)
        , bStickDuplicate(false) {}

    //! If there are other objects hit within this value, one of them needs
    //! to be selected by user.
    //! If this value is 0.f, then deep selection mode won't work.
    float fRange;
    bool bStickDuplicate;
};

//////////////////////////////////////////////////////////////////////////
struct SObjectColors
{
    SObjectColors()
    {
        groupHighlight = QColor(0, 255, 0);
        entityHighlight = QColor(112, 117, 102);
        fBBoxAlpha = 0.3f;
        geometryHighlightColor = QColor(192, 0, 192);
        solidBrushGeometryColor = QColor(192, 0, 192);
        fGeomAlpha = 0.2f;
        fChildGeomAlpha = 0.4f;
    }

    QColor groupHighlight;
    QColor entityHighlight;
    QColor geometryHighlightColor;
    QColor solidBrushGeometryColor;

    float fBBoxAlpha;
    float fGeomAlpha;
    float fChildGeomAlpha;
};

//////////////////////////////////////////////////////////////////////////
struct SViewportsSettings
{
    //! If enabled always show entity radiuse.
    bool bAlwaysShowRadiuses;
    //! True if 2D viewports will be synchronized with same view and origin.
    bool bSync2DViews;
    //! Camera FOV for perspective View.
    float fDefaultFov;
    //! Camera Aspect Ratio for perspective View.
    float fDefaultAspectRatio;
    //! Show safe frame.
    bool bShowSafeFrame;
    //! To highlight selected geometry.
    bool bHighlightSelectedGeometry;
    //! To highlight selected vegetation.
    bool bHighlightSelectedVegetation;

    //! To highlight selected geometry.
    int bHighlightMouseOverGeometry;

    //! Show triangle count on mouse over
    bool bShowMeshStatsOnMouseOver;

    //! If enabled will always display entity labels.
    bool bDrawEntityLabels;
    //! Show Trigger bounds.
    bool bShowTriggerBounds;
    //! Show Icons in viewport.
    bool bShowIcons;
    //! Scale icons with distance, so they aren't a fixed size no matter how far away you are
    bool bDistanceScaleIcons;

    //! Show Size-based Icons in viewport.
    bool bShowSizeBasedIcons;

    //! Show Helpers in viewport for frozen objects.
    int nShowFrozenHelpers;
    //! Fill Selected Shapes.
    bool bFillSelectedShapes;

    // Swap X/Y in map viewport.
    bool bTopMapSwapXY;
    // Texture resolution in the Top map viewport.
    int nTopMapTextureResolution;

    // Whether the grid guide (for move/rotate tool) will be shown or not.
    bool bShowGridGuide;

    // Whether the mouse cursor will be hidden when capturing the mouse.
    bool bHideMouseCursorWhenCaptured;

    // Size of square which the mouse must leave before a drag operation begins.
    int nDragSquareSize;

    // Enable context menu in the viewport
    bool bEnableContextMenu;

    //! Warning icons draw distance
    float fWarningIconsDrawDistance;
    //! Warnings for scale != 1
    bool bShowScaleWarnings;
    //! Warnings for rotation != 0
    bool bShowRotationWarnings;
};

struct SSelectObjectDialogSettings
{
    QString columns;
    int nLastColumnSortDirection;

    SSelectObjectDialogSettings()
        : nLastColumnSortDirection(0)
    {
    }
};

//////////////////////////////////////////////////////////////////////////
struct SGUI_Settings
{
    bool bWindowsVista;        // true when running on windows Vista
    QFont hSystemFont;         // Default system GUI font.
    QFont hSystemFontBold;     // Default system GUI bold font.
    QFont hSystemFontItalic;   // Default system GUI italic font.
    int nDefaultFontHieght;    // Default font height for 8 logical units.

    int nToolbarIconSize;      // Override size of the toolbar icons
};

//////////////////////////////////////////////////////////////////////////
struct STextureBrowserSettings
{
    int nCellSize; // Stores the default texture maximum cell size for
};

//////////////////////////////////////////////////////////////////////////
struct SExperimentalFeaturesSettings
{
    bool bTotalIlluminationEnabled;
};

//////////////////////////////////////////////////////////////////////////
struct SSliceSettings
{
    bool dynamicByDefault;
};

struct SLevelSaveSettings
{
    AzToolsFramework::Prefab::SaveAllPrefabsPreference saveAllPrefabsPreference;
};

//////////////////////////////////////////////////////////////////////////
struct SAssetBrowserSettings
{
    // stores the default thumb size
    int nThumbSize;
    // the current filename search filter in the search edit box
    QString sFilenameSearch,
    // the current filter preset name
            sPresetName;
    // current selected/checked/visible databases, db names separated by comma (,), Ex: "Textures,Models,Sounds"
    QString sVisibleDatabaseNames;
    // current visible columns in asset list
    QString sVisibleColumnNames;
    // current columns in asset list, in their actual order (including visible and hidden columns)
    QString sColumnNames;
    // check to show only the assets used in current level
    bool bShowUsedInLevel,
    // check to show only the assets loaded in the current level
         bShowLoadedInLevel,
    // check to filter only the favorite assets
         bShowFavorites,
    // hide LODs from thumbs view, for CGFs for example
         bHideLods,
    // true when the edited filter preset was changed, it will be saved automatically, without the user to push the "Save" button
         bAutoSaveFilterPreset,
    // true when we want sync between asset browser selection and viewport selection
         bAutoChangeViewportSelection,
    // true when we want sync between viewport selection and asset browser visible thumbs
         bAutoFilterFromViewportSelection;
};

struct SSmartOpenDialogSettings
{
    QRect rect;
    QString lastSearchTerm;
};

//////////////////////////////////////////////////////////////////////////
/** Various editor settings.
*/
AZ_PUSH_DISABLE_DLL_EXPORT_BASECLASS_WARNING
struct SANDBOX_API SEditorSettings
    : AzToolsFramework::EditorSettingsAPIBus::Handler
{
AZ_POP_DISABLE_DLL_EXPORT_BASECLASS_WARNING
    SEditorSettings();
    ~SEditorSettings() = default;
    void    Save();
    void    Load();
    void    LoadCloudSettings();

    void Connect();
    void Disconnect();

    // EditorSettingsAPIBus...
    AZStd::vector<AZStd::string> BuildSettingsList() override;
    SettingOutcome GetValue(const AZStd::string_view path) override;
    SettingOutcome SetValue(const AZStd::string_view path, const AZStd::any& value) override;
    AzToolsFramework::ConsoleColorTheme GetConsoleColorTheme() const override;
    int GetMaxNumberOfItemsShownInSearchView() const override;

    void ConvertPath(const AZStd::string_view sourcePath, AZStd::string& category, AZStd::string& attribute);

    // needs to be called after crysystem has been loaded
    void    LoadDefaultGamePaths();

    // need to expose updating of the source control enable/disable flag
    // because its state is updateable through the main status bar
    void SaveEnableSourceControlFlag(bool triggerUpdate = false);

    void LoadEnableSourceControlFlag();

    void PostInitApply();

    bool BrowseTerrainTexture(bool bIsSave);

    //////////////////////////////////////////////////////////////////////////
    // Variables.
    //////////////////////////////////////////////////////////////////////////
    int undoLevels;
    bool m_undoSliceOverrideSaveValue;
    bool bShowDashboardAtStartup;
    bool m_showCircularDependencyError;
    bool bAutoloadLastLevelAtStartup;
    bool bMuteAudio;
    bool bEnableGameModeVR;

    //! Speed of camera movement.
    float cameraMoveSpeed;
    float cameraRotateSpeed;
    float cameraFastMoveSpeed;
    float wheelZoomSpeed;
    bool invertYRotation;
    bool invertPan;
    bool stylusMode; // if stylus mode is enabled, no setCursorPos will be performed (WACOM tablets, etc)
    bool restoreViewportCamera; // When true, restore the original editor viewport camera when exiting game mode

    //! Hide mask for objects.
    int objectHideMask;

    //! Selection mask for objects.
    int objectSelectMask;

    //////////////////////////////////////////////////////////////////////////
    // Viewport settings.
    //////////////////////////////////////////////////////////////////////////
    SViewportsSettings viewports;

    AZ_PUSH_DISABLE_DLL_EXPORT_MEMBER_WARNING
    SToolViewSettings toolViewSettings;

    //////////////////////////////////////////////////////////////////////////
    // Files.
    //////////////////////////////////////////////////////////////////////////

    //! Before saving, make a backup into a subfolder
    bool bBackupOnSave;
    //! how many save backups to keep
    int backupOnSaveMaxCount;

    int useLowercasePaths;

    //////////////////////////////////////////////////////////////////////////
    // Autobackup.
    //////////////////////////////////////////////////////////////////////////
    //! Save auto backup file every autoSaveTime minutes.
    int autoBackupTime;
    //! Keep only several last autobackups.
    int autoBackupMaxCount;
    //! When this variable set to true automatic file backup is enabled.
    bool autoBackupEnabled;
    //! After this amount of minutes message box with reminder to save will pop on.
    int autoRemindTime;
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // Asset Browser Search View.
    //////////////////////////////////////////////////////////////////////////
    //! Current maximum number of items that can be displayed in the AssetBrowser Search View.
    int maxNumberOfItemsShownInSearch;
    //////////////////////////////////////////////////////////////////////////


    //! If true preview windows is displayed when browsing geometries.
    bool bPreviewGeometryWindow;

    int showErrorDialogOnLoad;

    //! Keeps the editor active even if no focus is set
    int keepEditorActive;

    // Settings of the snapping.
    SSnapSettings snap;

    //! Source Control Enabling.
    bool enableSourceControl;
    bool clearConsoleOnGameModeStart;

    //! Text editor.
    QString textEditorForScript;
    QString textEditorForShaders;
    QString textEditorForBspaces;

    //! Asset editors
    QString textureEditor;
    QString animEditor;

    //////////////////////////////////////////////////////////////////////////
    //! Editor data search paths.
    QStringList searchPaths[10]; // EDITOR_PATH_LAST

    // This directory is related to the editor root.
    QString strStandardTempDirectory;

    SGUI_Settings gui;

    bool              bApplyConfigSpecInEditor;

    ESystemConfigSpec editorConfigSpec;

    //! Terrain Texture Export/Import filename.
    QString terrainTextureExport;

    // Read only parameter.
    // Refects the status of GetIEditor()->GetOperationMode
    // To change current operation mode use GetIEditor()->SetOperationMode
    // see EOperationMode
    int operationMode;

    // For the texture browser configurations.
    STextureBrowserSettings sTextureBrowserSettings;

    // Experimental features configurations.
    SExperimentalFeaturesSettings sExperimentalFeaturesSettings;

    // For the asset browser configurations.
    SAssetBrowserSettings sAssetBrowserSettings;

    SSelectObjectDialogSettings selectObjectDialog;

    // For Terrain Texture Generation Multiplier.
    float fBrMultiplier;

    AzToolsFramework::ConsoleColorTheme consoleBackgroundColorTheme;

    bool bShowTimeInConsole;

    // Enable the option do get detailed information about the loaded scene data in the Scene Settings window.
    bool enableSceneInspector;

    // Deep Selection Mode Settings
    SDeepSelectionSettings deepSelectionSettings;

    // Object Highlight Settings
    SObjectColors objectColorSettings;
    AZ_POP_DISABLE_DLL_EXPORT_MEMBER_WARNING

    SSmartOpenDialogSettings smartOpenSettings;

    bool bSettingsManagerMode;

    bool bAutoSaveTagPoints;

    bool bNavigationContinuousUpdate;
    bool bNavigationShowAreas;
    bool bNavigationDebugDisplay;
    bool bVisualizeNavigationAccessibility;
    int  navigationDebugAgentType;

    int backgroundUpdatePeriod;
    const char* g_TemporaryLevelName;

    SSliceSettings sliceSettings;

    SLevelSaveSettings levelSaveSettings;

    bool prefabSystem = true;                  ///< Toggle to enable/disable the Prefab system for level entities.

private:
    void SaveValue(const char* sSection, const char* sKey, int value);
    void SaveValue(const char* sSection, const char* sKey, const QColor& value);
    void SaveValue(const char* sSection, const char* sKey, float value);
    void SaveValue(const char* sSection, const char* sKey, const QString& value);

    void LoadValue(const char* sSection, const char* sKey, int& value);
    void LoadValue(const char* sSection, const char* sKey, QColor& value);
    void LoadValue(const char* sSection, const char* sKey, float& value);
    void LoadValue(const char* sSection, const char* sKey, bool& value);
    void LoadValue(const char* sSection, const char* sKey, QString& value);
    void LoadValue(const char* sSection, const char* sKey, ESystemConfigSpec& value);

    void SaveCloudSettings();

    void SaveSettingsRegistryFile();

    //! Set a boolean setting value from the registry.
    //! \param key[in] The key to set.
    //! \param value[in] The new value for the setting.
    //! \return Whether the value for the key was correctly set in the registry.
    bool SetSettingsRegistry_Bool(const char* key, bool value);

    //! Gets a boolean setting value from the registry.
    //! \param key[in] The key to query.
    //! \param value[out] The variable the queried value will be saved to, by reference.
    //! \return Whether a value for the key was found in the registry.
    bool GetSettingsRegistry_Bool(const char* key, bool& value);
};

//! Single instance of editor settings for fast access.
SANDBOX_API extern SEditorSettings gSettings;

#endif // CRYINCLUDE_EDITOR_SETTINGS_H
