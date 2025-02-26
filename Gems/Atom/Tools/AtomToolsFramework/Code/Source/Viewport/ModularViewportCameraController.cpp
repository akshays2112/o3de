/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <AtomToolsFramework/Viewport/ModularViewportCameraController.h>
#include <AzCore/Console/IConsole.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Math/Color.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzFramework/Viewport/ScreenGeometry.h>
#include <AzFramework/Viewport/ViewportScreen.h>
#include <AzFramework/Windowing/WindowBus.h>
#include <AzToolsFramework/Viewport/ViewportMessages.h>

namespace AtomToolsFramework
{
    AZ_CVAR(
        AZ::Color,
        ed_cameraSystemOrbitPointColor,
        AZ::Color::CreateFromRgba(255, 255, 255, 255),
        nullptr,
        AZ::ConsoleFunctorFlags::Null,
        "");
    AZ_CVAR(float, ed_cameraSystemOrbitPointSize, 0.1f, nullptr, AZ::ConsoleFunctorFlags::Null, "");

    // debug
    void DrawPreviewAxis(AzFramework::DebugDisplayRequests& display, const AZ::Transform& transform, const float axisLength)
    {
        display.SetColor(AZ::Colors::Red);
        display.DrawLine(transform.GetTranslation(), transform.GetTranslation() + transform.GetBasisX().GetNormalizedSafe() * axisLength);
        display.SetColor(AZ::Colors::Green);
        display.DrawLine(transform.GetTranslation(), transform.GetTranslation() + transform.GetBasisY().GetNormalizedSafe() * axisLength);
        display.SetColor(AZ::Colors::Blue);
        display.DrawLine(transform.GetTranslation(), transform.GetTranslation() + transform.GetBasisZ().GetNormalizedSafe() * axisLength);
    }

    // convenience function to access the ViewportContext for the given ViewportId.
    static AZ::RPI::ViewportContextPtr RetrieveViewportContext(const AzFramework::ViewportId viewportId)
    {
        auto viewportContextManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
        if (!viewportContextManager)
        {
            return nullptr;
        }

        auto viewportContext = viewportContextManager->GetViewportContextById(viewportId);
        if (!viewportContext)
        {
            return nullptr;
        }

        return viewportContext;
    }

    ModularCameraViewportContextImpl::ModularCameraViewportContextImpl(const AzFramework::ViewportId viewportId)
        : m_viewportId(viewportId)
    {
    }

    AZ::Transform ModularCameraViewportContextImpl::GetCameraTransform() const
    {
        if (auto viewportContext = RetrieveViewportContext(m_viewportId))
        {
            return viewportContext->GetCameraTransform();
        }

        return AZ::Transform::CreateIdentity();
    }

    void ModularCameraViewportContextImpl::SetCameraTransform(const AZ::Transform& transform)
    {
        if (auto viewportContext = RetrieveViewportContext(m_viewportId))
        {
            viewportContext->SetCameraTransform(transform);
        }
    }

    void ModularCameraViewportContextImpl::ConnectViewMatrixChangedHandler(AZ::RPI::ViewportContext::MatrixChangedEvent::Handler& handler)
    {
        if (auto viewportContext = RetrieveViewportContext(m_viewportId))
        {
            viewportContext->ConnectViewMatrixChangedHandler(handler);
        }
    }

    void ModularViewportCameraController::SetCameraListBuilderCallback(const CameraListBuilder& builder)
    {
        m_cameraListBuilder = builder;
    }

    void ModularViewportCameraController::SetCameraPropsBuilderCallback(const CameraPropsBuilder& builder)
    {
        m_cameraPropsBuilder = builder;
    }

    void ModularViewportCameraController::SetCameraPriorityBuilderCallback(const CameraPriorityBuilder& builder)
    {
        m_cameraControllerPriorityBuilder = builder;
    }

    void ModularViewportCameraController::SetCameraViewportContextBuilderCallback(const CameraViewportContextBuilder& builder)
    {
        m_cameraViewportContextBuilder = builder;
    }

    void ModularViewportCameraController::SetupCameras(AzFramework::Cameras& cameras)
    {
        if (m_cameraListBuilder)
        {
            m_cameraListBuilder(cameras);
        }
    }

    void ModularViewportCameraController::SetupCameraProperties(AzFramework::CameraProps& cameraProps)
    {
        if (m_cameraPropsBuilder)
        {
            m_cameraPropsBuilder(cameraProps);
        }
    }

    void ModularViewportCameraController::SetupCameraControllerPriority(CameraControllerPriorityFn& cameraPriorityFn)
    {
        if (m_cameraControllerPriorityBuilder)
        {
            m_cameraControllerPriorityBuilder(cameraPriorityFn);
        }
    }

    void ModularViewportCameraController::SetupCameraControllerViewportContext(
        AZStd::unique_ptr<ModularCameraViewportContext>& cameraViewportContext)
    {
        if (m_cameraViewportContextBuilder)
        {
            m_cameraViewportContextBuilder(cameraViewportContext);
        }
    }

    // what priority should the camera system respond to
    AzFramework::ViewportControllerPriority DefaultCameraControllerPriority(const AzFramework::CameraSystem& cameraSystem)
    {
        // ModernViewportCameraControllerInstance receives events at all priorities, when it is in 'exclusive' mode
        // or it is actively handling events (essentially when the camera system is 'active' and responding to inputs)
        // it should only respond to the highest priority
        if (cameraSystem.m_cameras.Exclusive() || cameraSystem.HandlingEvents())
        {
            return AzFramework::ViewportControllerPriority::Highest;
        }

        // otherwise it should only respond to normal priority events
        return AzFramework::ViewportControllerPriority::Normal;
    }

    ModularViewportCameraControllerInstance::ModularViewportCameraControllerInstance(
        const AzFramework::ViewportId viewportId, ModularViewportCameraController* controller)
        : MultiViewportControllerInstanceInterface<ModularViewportCameraController>(viewportId, controller)
    {
        controller->SetupCameras(m_cameraSystem.m_cameras);
        controller->SetupCameraProperties(m_cameraProps);
        controller->SetupCameraControllerPriority(m_priorityFn);
        controller->SetupCameraControllerViewportContext(m_modularCameraViewportContext);

        auto handleCameraChange = [this](const AZ::Matrix4x4&)
        {
            // ignore these updates if the camera is being updated internally
            if (!m_updatingTransformInternally)
            {
                UpdateCameraFromTransform(m_targetCamera, m_modularCameraViewportContext->GetCameraTransform());
                m_camera = m_targetCamera;
            }
        };

        m_cameraViewMatrixChangeHandler = AZ::RPI::ViewportContext::MatrixChangedEvent::Handler(handleCameraChange);
        m_modularCameraViewportContext->ConnectViewMatrixChangedHandler(m_cameraViewMatrixChangeHandler);

        AzFramework::ViewportDebugDisplayEventBus::Handler::BusConnect(AzToolsFramework::GetEntityContextId());
        ModularViewportCameraControllerRequestBus::Handler::BusConnect(viewportId);
    }

    ModularViewportCameraControllerInstance::~ModularViewportCameraControllerInstance()
    {
        ModularViewportCameraControllerRequestBus::Handler::BusDisconnect();
        AzFramework::ViewportDebugDisplayEventBus::Handler::BusDisconnect();
    }

    bool ModularViewportCameraControllerInstance::HandleInputChannelEvent(const AzFramework::ViewportControllerInputEvent& event)
    {
        if (event.m_priority == m_priorityFn(m_cameraSystem))
        {
            AzFramework::WindowSize windowSize;
            AzFramework::WindowRequestBus::EventResult(
                windowSize, event.m_windowHandle, &AzFramework::WindowRequestBus::Events::GetClientAreaSize);

            return m_cameraSystem.HandleEvents(AzFramework::BuildInputEvent(event.m_inputChannel, windowSize));
        }

        return false;
    }

    void ModularViewportCameraControllerInstance::UpdateViewport(const AzFramework::ViewportControllerUpdateEvent& event)
    {
        // only update for a single priority (normal is the default)
        if (event.m_priority != AzFramework::ViewportControllerPriority::Normal)
        {
            return;
        }

        m_updatingTransformInternally = true;

        if (m_cameraMode == CameraMode::Control)
        {
            m_targetCamera = m_cameraSystem.StepCamera(m_targetCamera, event.m_deltaTime.count());
            m_camera = AzFramework::SmoothCamera(m_camera, m_targetCamera, m_cameraProps, event.m_deltaTime.count());

            // if there has been an interpolation, only clear the look at point if it is no longer
            // centered in the view (the camera has looked away from it)
            if (m_lookAtAfterInterpolation.has_value())
            {
                if (const float lookDirection =
                        (*m_lookAtAfterInterpolation - m_camera.Translation()).GetNormalized().Dot(m_camera.Transform().GetBasisY());
                    !AZ::IsCloseMag(lookDirection, 1.0f, 0.001f))
                {
                    m_lookAtAfterInterpolation = {};
                }
            }

            m_modularCameraViewportContext->SetCameraTransform(m_camera.Transform());
        }
        else if (m_cameraMode == CameraMode::Animation)
        {
            const auto smootherStepFn = [](const float t)
            {
                return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
            };

            const auto& [transformStart, transformEnd, animationTime] = m_cameraAnimation;

            const float transitionTime = smootherStepFn(animationTime);
            const AZ::Transform current = AZ::Transform::CreateFromQuaternionAndTranslation(
                transformStart.GetRotation().Slerp(transformEnd.GetRotation(), transitionTime),
                transformStart.GetTranslation().Lerp(transformEnd.GetTranslation(), transitionTime));

            const AZ::Vector3 eulerAngles = AzFramework::EulerAngles(AZ::Matrix3x3::CreateFromTransform(current));
            m_camera.m_pitch = eulerAngles.GetX();
            m_camera.m_yaw = eulerAngles.GetZ();
            m_camera.m_lookAt = current.GetTranslation();
            m_targetCamera = m_camera;

            if (animationTime >= 1.0f)
            {
                m_cameraMode = CameraMode::Control;
            }

            m_cameraAnimation.m_time = AZ::GetClamp(animationTime + event.m_deltaTime.count(), 0.0f, 1.0f);

            m_modularCameraViewportContext->SetCameraTransform(current);
        }

        m_updatingTransformInternally = false;
    }

    void ModularViewportCameraControllerInstance::DisplayViewport(
        [[maybe_unused]] const AzFramework::ViewportInfo& viewportInfo, AzFramework::DebugDisplayRequests& debugDisplay)
    {
        if (const float alpha = AZStd::min(-m_camera.m_lookDist / 5.0f, 1.0f); alpha > AZ::Constants::FloatEpsilon)
        {
            const AZ::Color orbitPointColor = ed_cameraSystemOrbitPointColor;
            debugDisplay.SetColor(orbitPointColor.GetR(), orbitPointColor.GetG(), orbitPointColor.GetB(), alpha);
            debugDisplay.DrawWireSphere(m_camera.m_lookAt, ed_cameraSystemOrbitPointSize);
        }
    }

    void ModularViewportCameraControllerInstance::InterpolateToTransform(const AZ::Transform& worldFromLocal, const float lookAtDistance)
    {
        m_cameraMode = CameraMode::Animation;
        m_cameraAnimation = CameraAnimation{ m_camera.Transform(), worldFromLocal, 0.0f };
        m_lookAtAfterInterpolation = worldFromLocal.GetTranslation() + worldFromLocal.GetBasisY() * lookAtDistance;
    }

    AZStd::optional<AZ::Vector3> ModularViewportCameraControllerInstance::LookAtAfterInterpolation() const
    {
        return m_lookAtAfterInterpolation;
    }
} // namespace AtomToolsFramework
