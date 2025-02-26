/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GemCatalog/GemCatalogScreen.h>
#include <ProjectManagerDefs.h>
#include <PythonBindingsInterface.h>
#include <ScreenHeaderWidget.h>
#include <ScreensCtrl.h>
#include <UpdateProjectCtrl.h>
#include <UpdateProjectSettingsScreen.h>
#include <ProjectUtils.h>

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QDir>

namespace O3DE::ProjectManager
{
    UpdateProjectCtrl::UpdateProjectCtrl(QWidget* parent)
        : ScreenWidget(parent)
    {
        QVBoxLayout* vLayout = new QVBoxLayout();
        vLayout->setContentsMargins(0, 0, 0, 0);

        m_header = new ScreenHeader(this);
        m_header->setTitle(tr(""));
        m_header->setSubTitle(tr("Edit Project Settings:"));
        connect(m_header->backButton(), &QPushButton::clicked, this, &UpdateProjectCtrl::HandleBackButton);
        vLayout->addWidget(m_header);

        m_updateSettingsScreen = new UpdateProjectSettingsScreen();
        m_gemCatalogScreen = new GemCatalogScreen();

        m_stack = new QStackedWidget(this);
        m_stack->setObjectName("body");
        m_stack->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));
        vLayout->addWidget(m_stack);

        QFrame* topBarFrameWidget = new QFrame(this);
        topBarFrameWidget->setObjectName("projectSettingsTopFrame");
        QHBoxLayout* topBarHLayout = new QHBoxLayout();
        topBarHLayout->setContentsMargins(0, 0, 0, 0);
        topBarFrameWidget->setLayout(topBarHLayout);

        QTabWidget* tabWidget = new QTabWidget();
        tabWidget->setObjectName("projectSettingsTab");
        tabWidget->tabBar()->setObjectName("projectSettingsTabBar");
        tabWidget->tabBar()->setFocusPolicy(Qt::TabFocus);
        tabWidget->addTab(m_updateSettingsScreen, tr("General"));

        QPushButton* gemsButton = new QPushButton(tr("Configure Gems"), this);
        topBarHLayout->addWidget(gemsButton);
        tabWidget->setCornerWidget(gemsButton);

        topBarHLayout->addWidget(tabWidget);

        m_stack->addWidget(topBarFrameWidget);
        m_stack->addWidget(m_gemCatalogScreen);

        QDialogButtonBox* backNextButtons = new QDialogButtonBox();
        backNextButtons->setObjectName("footer");
        vLayout->addWidget(backNextButtons);

        m_backButton = backNextButtons->addButton(tr("Back"), QDialogButtonBox::RejectRole);
        m_backButton->setProperty("secondary", true);
        m_nextButton = backNextButtons->addButton(tr("Next"), QDialogButtonBox::ApplyRole);

        connect(gemsButton, &QPushButton::clicked, this, &UpdateProjectCtrl::HandleGemsButton);
        connect(m_backButton, &QPushButton::clicked, this, &UpdateProjectCtrl::HandleBackButton);
        connect(m_nextButton, &QPushButton::clicked, this, &UpdateProjectCtrl::HandleNextButton);
        connect(reinterpret_cast<ScreensCtrl*>(parent), &ScreensCtrl::NotifyCurrentProject, this, &UpdateProjectCtrl::UpdateCurrentProject);

        Update();
        setLayout(vLayout);
    }

    ProjectManagerScreen UpdateProjectCtrl::GetScreenEnum()
    {
        return ProjectManagerScreen::UpdateProject;
    }

    // Called when pressing "Edit Project Settings..."
    void UpdateProjectCtrl::NotifyCurrentScreen()
    {
        m_stack->setCurrentIndex(ScreenOrder::Settings);
        Update();

        // Gather the available gems that will be shown in the gem catalog.
        m_gemCatalogScreen->ReinitForProject(m_projectInfo.m_path);
    }

    void UpdateProjectCtrl::HandleGemsButton()
    {
        if (UpdateProjectSettings(true))
        {
            m_stack->setCurrentWidget(m_gemCatalogScreen);
            Update();
        }
    }

    void UpdateProjectCtrl::HandleBackButton()
    {
        if (m_stack->currentIndex() > 0)
        {
            m_stack->setCurrentIndex(m_stack->currentIndex() - 1);
            Update();
        }
        else
        {
            if (UpdateProjectSettings(true))
            {
                emit GotoPreviousScreenRequest();
            }
        }
    }

    void UpdateProjectCtrl::HandleNextButton()
    {
        bool shouldRebuild = false;

        if (m_stack->currentIndex() == ScreenOrder::Settings && m_updateSettingsScreen)
        {
            if (!UpdateProjectSettings())
            {
                return;
            }
        }
        else if (m_stack->currentIndex() == ScreenOrder::Gems && m_gemCatalogScreen)
        {
            // Enable or disable the gems that got adjusted in the gem catalog and apply them to the given project.
            if (!m_gemCatalogScreen->EnableDisableGemsForProject(m_projectInfo.m_path))
            {
                QMessageBox::critical(this, tr("Failed to configure gems"), tr("Failed to configure gems for project."));
                return;
            }

            shouldRebuild = true;
        }

        if (shouldRebuild)
        {
            emit NotifyBuildProject(m_projectInfo);
        }

        emit ChangeScreenRequest(ProjectManagerScreen::Projects);
    }

    void UpdateProjectCtrl::UpdateCurrentProject(const QString& projectPath)
    {
        auto projectResult = PythonBindingsInterface::Get()->GetProject(projectPath);
        if (projectResult.IsSuccess())
        {
            m_projectInfo = projectResult.GetValue();
        }

        Update();
        UpdateSettingsScreen();
    }

    void UpdateProjectCtrl::Update()
    {
        if (m_stack->currentIndex() == ScreenOrder::Gems)
        {

            m_header->setTitle(QString(tr("Edit Project Settings: \"%1\"")).arg(m_projectInfo.GetProjectDisplayName()));
            m_header->setSubTitle(QString(tr("Configure Gems")));
            m_nextButton->setText(tr("Save"));
        }
        else
        {
            m_header->setTitle("");
            m_header->setSubTitle(QString(tr("Edit Project Settings: \"%1\"")).arg(m_projectInfo.GetProjectDisplayName()));
            m_nextButton->setText(tr("Save"));
        }
    }

    void UpdateProjectCtrl::UpdateSettingsScreen()
    {
        m_updateSettingsScreen->SetProjectInfo(m_projectInfo);
    }

    bool UpdateProjectCtrl::UpdateProjectSettings(bool shouldConfirm)
    {
        AZ_Assert(m_updateSettingsScreen, "Update settings screen is nullptr.")

        ProjectInfo newProjectSettings = m_updateSettingsScreen->GetProjectInfo();

        if (m_projectInfo != newProjectSettings)
        {
            if (shouldConfirm)
            {
                QMessageBox::StandardButton warningResult = QMessageBox::warning(
                    this,
                    QObject::tr("Unsaved Changes!"),
                    QObject::tr("Would you like to save your changes to project settings?"),
                    QMessageBox::No | QMessageBox::Yes
                );

                if (warningResult == QMessageBox::No)
                {
                    return true;
                }
            }

            if (!m_updateSettingsScreen->Validate())
            {
                QMessageBox::critical(this, tr("Invalid project settings"), tr("Invalid project settings"));
                return false;
            }

            // Check if project path has changed and move it
            // Move project first to avoid trying to update settings at the new location before it has been moved there
            if (newProjectSettings.m_path != m_projectInfo.m_path)
            {
                if (!ProjectUtils::MoveProject(m_projectInfo.m_path, newProjectSettings.m_path, this))
                {
                    QMessageBox::critical(this, tr("Project move failed"), tr("Failed to move project."));
                    return false;
                }

                emit NotifyBuildProject(newProjectSettings);
            }

            // Update project if settings changed
            {
                auto result = PythonBindingsInterface::Get()->UpdateProject(newProjectSettings);
                if (!result.IsSuccess())
                {
                    QMessageBox::critical(this, tr("Project update failed"), tr(result.GetError().c_str()));
                    return false;
                }
            }

            if (!newProjectSettings.m_newPreviewImagePath.isEmpty())
            {
                if (!ProjectUtils::ReplaceProjectFile(
                        QDir(newProjectSettings.m_path).filePath(newProjectSettings.m_iconPath), newProjectSettings.m_newPreviewImagePath))
                {
                    QMessageBox::critical(this, tr("File replace failed"), tr("Failed to replace project preview image."));
                    return false;
                }
                m_updateSettingsScreen->ResetProjectPreviewPath();
            }

            m_projectInfo = newProjectSettings;
        }

        return true;
    }

} // namespace O3DE::ProjectManager
