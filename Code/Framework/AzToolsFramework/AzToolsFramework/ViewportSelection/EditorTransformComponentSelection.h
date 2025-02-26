/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Console/IConsole.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/optional.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzFramework/Viewport/ClickDetector.h>
#include <AzFramework/Viewport/CursorState.h>
#include <AzToolsFramework/API/EditorCameraBus.h>
#include <AzToolsFramework/Commands/EntityManipulatorCommand.h>
#include <AzToolsFramework/ComponentMode/EditorComponentModeBus.h>
#include <AzToolsFramework/Editor/EditorContextMenuBus.h>
#include <AzToolsFramework/Manipulators/BaseManipulator.h>
#include <AzToolsFramework/ToolsComponents/EditorLockComponentBus.h>
#include <AzToolsFramework/ToolsComponents/EditorVisibilityBus.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <AzToolsFramework/Viewport/EditorContextMenu.h>
#include <AzToolsFramework/Viewport/ViewportMessages.h>
#include <AzToolsFramework/ViewportSelection/EditorBoxSelect.h>
#include <AzToolsFramework/ViewportSelection/EditorHelpers.h>
#include <AzToolsFramework/ViewportSelection/EditorTransformComponentSelectionRequestBus.h>
#include <AzToolsFramework/ViewportUi/ViewportUiRequestBus.h>

namespace AzToolsFramework
{
    AZ_CVAR_EXTERNED(bool, ed_viewportStickySelect);

    class EditorVisibleEntityDataCache;

    using EntityIdSet = AZStd::unordered_set<AZ::EntityId>; //!< Alias for unordered_set of EntityIds.

    //! Entity related data required by manipulators during action.
    struct EntityIdManipulatorLookup
    {
        AZ::Transform m_initial; //!< Transform of Entity at mouse down on manipulator.
    };

    //! Alias for a mapping between EntityIds and Entity related data required by manipulators.
    using EntityIdManipulatorLookups = AZStd::unordered_map<AZ::EntityId, EntityIdManipulatorLookup>;

    //! Generic wrapper to handle specific manipulators controlling 1-* entities.
    struct EntityIdManipulators
    {
        EntityIdManipulatorLookups m_lookups; //!< Mapping between the EntityId and the transform of the Entity at
                                              //!< the point a manipulator started adjusting it.
        AZStd::unique_ptr<Manipulators> m_manipulators; //!< The aggregate manipulator currently in use.
    };

    //! Store translation and orientation only (no scale).
    struct Frame
    {
        AZ::Vector3 m_translation = AZ::Vector3::CreateZero(); //!< Position of frame.
        AZ::Quaternion m_orientation = AZ::Quaternion::CreateIdentity(); //!< Orientation of frame.
    };

    //! Temporary manipulator frame used during selection.
    struct OptionalFrame
    {
        //! What part of the transform did we pick (when using ditto on
        //! the manipulator). This will depend on the transform mode we're in.
        struct PickType
        {
            enum : AZ::u8
            {
                None = 0,
                Orientation = 1 << 0,
                Translation = 1 << 1
            };
        };

        bool HasTranslationOverride() const;
        bool HasOrientationOverride() const;
        bool HasTransformOverride() const;
        bool HasEntityOverride() const;
        bool PickedTranslation() const;
        bool PickedOrientation() const;

        //! Clear all state associated with the frame.
        void Reset();
        //! Clear only picked translation state.
        void ResetPickedTranslation();
        //! Clear only picked orientation state.
        void ResetPickedOrientation();

        AZ::EntityId m_pickedEntityIdOverride; //!< 'Picked' Entity - frame and parent space relative to this if active.
        AZStd::optional<AZ::Vector3> m_translationOverride; //!< Translation override, if set, reset when selection is empty.
        AZStd::optional<AZ::Quaternion> m_orientationOverride; //!< Orientation override, if set, reset when selection is empty.
        AZ::u8 m_pickTypes = PickType::None; //!< What mode(s) were we in when picking an EntityId override.
    };

    //! What frame/space is the manipulator currently operating in.
    enum class ReferenceFrame
    {
        Local, //!< The local space of the individual entity.
        Parent, //!< The parent space of the individual entity (world space if no parent exists).
        World, //!< World space (space aligned to world axes - identity).
    };

    //! Grouping of viewport ui related state for controlling the current reference space of the Editor.
    struct SpaceCluster
    {
        SpaceCluster() = default;
        // disable copying and moving (implicit)
        SpaceCluster(const SpaceCluster&) = delete;
        SpaceCluster& operator=(const SpaceCluster&) = delete;

        ViewportUi::ClusterId m_clusterId; //!< The id identifying the reference space cluster.
        ViewportUi::ButtonId m_localButtonId; //!< Local reference space button id.
        ViewportUi::ButtonId m_parentButtonId; //!< Parent reference space button id.
        ViewportUi::ButtonId m_worldButtonId; //!< World reference space button id.
        AZStd::optional<ReferenceFrame> m_spaceLock; //!< Locked reference frame to use if set.
        AZ::Event<ViewportUi::ButtonId>::Handler m_spaceHandler; //!< Callback for when a space cluster button is pressed.
    };

    //! Grouping of viewport ui related state for aligning transforms to a grid.
    struct SnappingCluster
    {
        SnappingCluster() = default;
        // disable copying and moving (implicit)
        SnappingCluster(const SnappingCluster&) = delete;
        SnappingCluster& operator=(const SnappingCluster&) = delete;

        //! Attempt to show the snapping cluster (will only succeed if snapping is enabled).
        void TrySetVisible(bool visible);

        ViewportUi::ClusterId m_clusterId; //!< The cluster id for all snapping buttons.
        ViewportUi::ButtonId m_snapToWorldButtonId; //!< The button id for snapping all axes to the world.
        AZ::Event<ViewportUi::ButtonId>::Handler m_snappingHandler; //!< Callback for when a snapping cluster button is pressed.
    };

    //! Entity selection/interaction handling.
    //! Provide a suite of functionality for manipulating entities, primarily through their TransformComponent.
    class EditorTransformComponentSelection
        : public ViewportInteraction::ViewportSelectionRequests
        , public EditorContextMenuBus::Handler
        , private EditorEventsBus::Handler
        , private EditorTransformComponentSelectionRequestBus::Handler
        , private ToolsApplicationNotificationBus::Handler
        , private Camera::EditorCameraNotificationBus::Handler
        , private ComponentModeFramework::EditorComponentModeNotificationBus::Handler
        , private EditorEntityContextNotificationBus::Handler
        , private EditorEntityVisibilityNotificationBus::Router
        , private EditorEntityLockComponentNotificationBus::Router
        , private EditorManipulatorCommandUndoRedoRequestBus::Handler
        , private AZ::TransformNotificationBus::MultiHandler
        , private ViewportInteraction::ViewportSettingsNotificationBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL

        EditorTransformComponentSelection() = default;
        explicit EditorTransformComponentSelection(const EditorVisibleEntityDataCache* entityDataCache);
        EditorTransformComponentSelection(const EditorTransformComponentSelection&) = delete;
        EditorTransformComponentSelection& operator=(const EditorTransformComponentSelection&) = delete;
        virtual ~EditorTransformComponentSelection();

        //! Register entity manipulators with the ManipulatorManager.
        //! After being registered, the entity manipulators will draw and check for input.
        void RegisterManipulator();
        //! Unregister entity manipulators with the ManipulatorManager.
        //! No longer draw or respond to input.
        void UnregisterManipulator();

        //! ViewportInteraction::ViewportSelectionRequests
        //! Intercept all viewport mouse events and respond to inputs.
        bool HandleMouseInteraction(const ViewportInteraction::MouseInteractionEvent& mouseInteraction) override;
        void DisplayViewportSelection(
            const AzFramework::ViewportInfo& viewportInfo, AzFramework::DebugDisplayRequests& debugDisplay) override;
        void DisplayViewportSelection2d(
            const AzFramework::ViewportInfo& viewportInfo, AzFramework::DebugDisplayRequests& debugDisplay) override;

        //! Add an entity to the current selection
        void AddEntityToSelection(AZ::EntityId entityId);
        //! Remove an entity from the current selection
        void RemoveEntityFromSelection(AZ::EntityId entityId);

    private:
        void CreateEntityIdManipulators();
        void CreateTranslationManipulators();
        void CreateRotationManipulators();
        void CreateScaleManipulators();
        void RegenerateManipulators();

        void CreateTransformModeSelectionCluster();
        void CreateSpaceSelectionCluster();
        void CreateSnappingCluster();

        void ClearManipulatorTranslationOverride();
        void ClearManipulatorOrientationOverride();
        //! Handle an event triggered by the user to clear any manipulator overrides.
        //! Delegate to either translation or orientation reset/clear depending on the state we're in.
        void DelegateClearManipulatorOverride();

        void ToggleCenterPivotSelection();

        bool IsEntitySelected(AZ::EntityId entityId) const;
        void SetSelectedEntities(const EntityIdList& entityIds);
        void DeselectEntities();
        bool SelectDeselect(AZ::EntityId entityId);

        void RefreshSelectedEntityIds();
        void RefreshSelectedEntityIds(const EntityIdList& selectedEntityIds);
        void InitializeManipulators(Manipulators& manipulators);

        void SetupBoxSelect();

        void RegisterActions();
        void UnregisterActions();

        void BeginRecordManipulatorCommand();
        void EndRecordManipulatorCommand();

        // If entity state has changed, make sure to refresh the selection and update
        // the active manipulators accordingly
        void CheckDirtyEntityIds();
        void RefreshSelectedEntityIdsAndRegenerateManipulators();

        // Builds an EntityManipulatorCommand::State based on the current state of the
        // EditorTransformComponentSelection (manipulators, frame overrides and picked entities).
        // Note: Precondition of calling CreateManipulatorCommandStateFromSelf is
        // m_entityIdManipulators.m_manipulators must be valid.
        EntityManipulatorCommand::State CreateManipulatorCommandStateFromSelf() const;

        // Helper to record the manipulator before and after a deselect so it
        // can be returned to its previous state after an undo/redo operation
        void CreateEntityManipulatorDeselectCommand(ScopedUndoBatch& undoBatch);

        // EditorTransformComponentSelectionRequestBus ...
        Mode GetTransformMode() override;
        void SetTransformMode(Mode mode) override;
        void RefreshManipulators(RefreshType refreshType) override;
        AZStd::optional<AZ::Transform> GetManipulatorTransform() override;
        void OverrideManipulatorOrientation(const AZ::Quaternion& orientation) override;
        void OverrideManipulatorTranslation(const AZ::Vector3& translation) override;
        void CopyTranslationToSelectedEntitiesIndividual(const AZ::Vector3& translation) override;
        void CopyTranslationToSelectedEntitiesGroup(const AZ::Vector3& translation) override;
        void ResetTranslationForSelectedEntitiesLocal() override;
        void CopyOrientationToSelectedEntitiesIndividual(const AZ::Quaternion& orientation) override;
        void CopyOrientationToSelectedEntitiesGroup(const AZ::Quaternion& orientation) override;
        void ResetOrientationForSelectedEntitiesLocal() override;
        void CopyScaleToSelectedEntitiesIndividualLocal(float scale) override;
        void CopyScaleToSelectedEntitiesIndividualWorld(float scale) override;
        void SnapSelectedEntitiesToWorldGrid(float gridSize) override;

        // EditorManipulatorCommandUndoRedoRequestBus ...
        void UndoRedoEntityManipulatorCommand(AZ::u8 pivotOverride, const AZ::Transform& transform, AZ::EntityId entityId) override;

        // EditorContextMenuBus...
        void PopulateEditorGlobalContextMenu(QMenu* menu, const AZ::Vector2& point, int flags) override;
        int GetMenuPosition() const override;
        AZStd::string GetMenuIdentifier() const override;

        // EditorEventsBus ...
        void OnEscape() override;

        // ToolsApplicationNotificationBus ...
        void BeforeEntitySelectionChanged() override;
        void AfterEntitySelectionChanged(const EntityIdList& newlySelectedEntities, const EntityIdList& newlyDeselectedEntities) override;

        // TransformNotificationBus ...
        void OnTransformChanged(const AZ::Transform& localTM, const AZ::Transform& worldTM) override;

        // Camera::EditorCameraNotificationBus ...
        void OnViewportViewEntityChanged(const AZ::EntityId& newViewId) override;

        // EditorContextVisibilityNotificationBus ...
        void OnEntityVisibilityChanged(bool visibility) override;

        // EditorContextLockComponentNotificationBus ...
        void OnEntityLockChanged(bool locked) override;

        // EditorComponentModeNotificationBus ...
        void EnteredComponentMode(const AZStd::vector<AZ::Uuid>& componentModeTypes) override;
        void LeftComponentMode(const AZStd::vector<AZ::Uuid>& componentModeTypes) override;

        // EditorEntityContextNotificationBus overrides ...
        void OnStartPlayInEditor() override;
        void OnStopPlayInEditor() override;

        // ViewportSettingsNotificationBus overrides ...
        void OnGridSnappingChanged(bool enabled) override;

        // Helpers to safely interact with the TransformBus (requests).
        void SetEntityWorldTranslation(AZ::EntityId entityId, const AZ::Vector3& worldTranslation);
        void SetEntityLocalTranslation(AZ::EntityId entityId, const AZ::Vector3& localTranslation);
        void SetEntityWorldTransform(AZ::EntityId entityId, const AZ::Transform& worldTransform);
        void SetEntityLocalScale(AZ::EntityId entityId, float localScale);
        void SetEntityLocalRotation(AZ::EntityId entityId, const AZ::Vector3& localRotation);
        void SetEntityLocalRotation(AZ::EntityId entityId, const AZ::Quaternion& localRotation);

        //! Responsible for keeping the space cluster in sync with the current reference frame.
        void UpdateSpaceCluster(ReferenceFrame referenceFrame);

        //! Hides/Shows all viewportUi toolbars.
        void SetAllViewportUiVisible(bool visible);

        AZ::EntityId m_hoveredEntityId; //!< What EntityId is the mouse currently hovering over (if any).
        AZ::EntityId m_cachedEntityIdUnderCursor; //!< Store the EntityId on each mouse move for use in Display.
        AZ::EntityId m_editorCameraComponentEntityId; //!< The EditorCameraComponent EntityId if it is set.
        EntityIdSet m_selectedEntityIds; //!< Represents the current entities in the selection.

        const EditorVisibleEntityDataCache* m_entityDataCache = nullptr; //!< A cache of packed EntityData that can be
                                                                         //!< iterated over efficiently without the need
                                                                         //!< to make individual EBus calls.
        AZStd::unique_ptr<EditorHelpers> m_editorHelpers; //!< Editor visualization of entities (icons, shapes, debug visuals etc).
        EntityIdManipulators m_entityIdManipulators; //!< Mapping from a Manipulator to potentially many EntityIds.

        EditorBoxSelect m_boxSelect; //!< Type responsible for handling box select.
        //! Track adjustments to manipulator translation and orientation (during mouse press/move).
        AZStd::unique_ptr<EntityManipulatorCommand> m_manipulatorMoveCommand;
        AZStd::vector<AZStd::unique_ptr<QAction>> m_actions; //!< What actions are tied to this handler.
        ViewportInteraction::KeyboardModifiers m_previousModifiers; //!< What modifiers were held last frame.
        EditorContextMenu m_contextMenu; //!< Viewport right click context menu.
        OptionalFrame m_pivotOverrideFrame; //!< Has a pivot override been set.
        Mode m_mode = Mode::Translation; //!< Manipulator mode - default to translation.
        Pivot m_pivotMode = Pivot::Object; //!< Entity pivot mode - default to object (authored root).
        ReferenceFrame m_referenceFrame = ReferenceFrame::Parent; //!< What reference frame is the Manipulator currently operating in.
        Frame m_axisPreview; //!< Axes of entity at the time of mouse down to indicate delta of translation.
        bool m_triedToRefresh = false; //!< Did a refresh event occur to recalculate the current Manipulator transform.
        //! Was EditorTransformComponentSelection responsible for the most recent entity selection change.
        bool m_didSetSelectedEntities = false;
        //! Do the active manipulators need to recalculated after a modification (lock/visibility etc).
        bool m_selectedEntityIdsAndManipulatorsDirty = false;
        bool m_transformChangedInternally = false; //!< Was an OnTransformChanged event triggered internally or not.
        ViewportUi::ClusterId m_transformModeClusterId; //!< Id of the Viewport UI cluster for changing transform mode.
        ViewportUi::ButtonId m_translateButtonId; //!< Id of the Viewport UI button for translate mode.
        ViewportUi::ButtonId m_rotateButtonId; //!< Id of the Viewport UI button for rotate mode.
        ViewportUi::ButtonId m_scaleButtonId; //!< Id of the Viewport UI button for scale mode.
        AZ::Event<ViewportUi::ButtonId>::Handler m_transformModeSelectionHandler; //!< Event handler for the Viewport UI cluster.
        AzFramework::ClickDetector m_clickDetector; //!< Detect different types of mouse click.
        AzFramework::CursorState m_cursorState; //!< Track the mouse position and delta movement each frame.
        SpaceCluster m_spaceCluster; //!< Related viewport ui state for controlling the current reference space.
        SnappingCluster m_snappingCluster; //!< Related viewport ui state for aligning positions to a grid or reference frame.
        bool m_viewportUiVisible = true; //!< Used to hide/show the viewport ui elements.
    };

    //! The ETCS (EntityTransformComponentSelection) namespace contains functions and data used exclusively by
    //! the EditorTransformComponentSelection type. Functions in this namespace are exposed to facilitate testing
    //! and should not be used outside of EditorTransformComponentSelection or EditorTransformComponentSelectionTests.
    namespace ETCS
    {
        //! The result from calculating the entity (transform component) orientation.
        //! Does the entity have a parent or not, and what orientation should the manipulator have when
        //! displayed at the object pivot (determined by the entity hierarchy and what modifiers are held).
        struct PivotOrientationResult
        {
            AZ::Quaternion m_worldOrientation;
            AZ::EntityId m_parentId;
        };

        //! Calculate the orientation for an individual entity based on the incoming reference frame.
        //! Note: If the entity is in a hierarchy the Parent reference frame will return the orientation of the parent.
        PivotOrientationResult CalculatePivotOrientation(AZ::EntityId entityId, ReferenceFrame referenceFrame);

        //! Calculate the orientation for a group of entities based on the incoming reference frame.
        template<typename EntityIdMap>
        PivotOrientationResult CalculatePivotOrientationForEntityIds(const EntityIdMap& entityIdMap, const ReferenceFrame referenceFrame);

        //! Calculate the orientation for a group of entities based on the incoming
        //! reference frame with possible pivot override.
        template<typename EntityIdMap>
        PivotOrientationResult CalculateSelectionPivotOrientation(
            const EntityIdMap& entityIdMap, const OptionalFrame& pivotOverrideFrame, const ReferenceFrame referenceFrame);

        void SetEntityWorldTranslation(AZ::EntityId entityId, const AZ::Vector3& worldTranslation, bool& internal);
        void SetEntityLocalTranslation(AZ::EntityId entityId, const AZ::Vector3& localTranslation, bool& internal);
        void SetEntityWorldTransform(AZ::EntityId entityId, const AZ::Transform& worldTransform, bool& internal);
        void SetEntityLocalScale(AZ::EntityId entityId, float localScale, bool& internal);
        void SetEntityLocalRotation(AZ::EntityId entityId, const AZ::Vector3& localRotation, bool& internal);
        void SetEntityLocalRotation(AZ::EntityId entityId, const AZ::Quaternion& localRotation, bool& internal);
    } // namespace ETCS
} // namespace AzToolsFramework
