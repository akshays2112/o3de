/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <TagWidget.h>
#include <QVBoxLayout>

namespace O3DE::ProjectManager
{
    TagWidget::TagWidget(const QString& text, QWidget* parent)
        : QLabel(text, parent)
    {
        setFixedHeight(35);
        setMargin(5);
        setStyleSheet("font-size: 12pt; background-color: #333333; border-radius: 4px;");
    }

    TagContainerWidget::TagContainerWidget(QWidget* parent)
        : QWidget(parent)
    {
        m_layout = new QVBoxLayout();
        m_layout->setAlignment(Qt::AlignTop);
        m_layout->setMargin(0);
        setLayout(m_layout);
    }

    void TagContainerWidget::Update(const QStringList& tags)
    {
        QWidget* parentWidget = qobject_cast<QWidget*>(parent());
        int width = 250;
        if (parentWidget)
        {
            width = parentWidget->width();
        }

        if (m_widget)
        {
            // Hide the old widget and request deletion.
            m_widget->hide();
            m_widget->deleteLater();
        }

        QVBoxLayout* vLayout = new QVBoxLayout();
        m_widget = new QWidget(this);
        m_widget->setLayout(vLayout);
        m_layout->addWidget(m_widget);

        vLayout->setAlignment(Qt::AlignTop);
        vLayout->setMargin(0);

        QHBoxLayout* hLayout = nullptr;
        int usedSpaceInRow = 0;
        const int numTags = tags.count();

        for (int i = 0; i < numTags; ++i)
        {
            // Create the new tag widget.
            TagWidget* tagWidget = new TagWidget(tags[i]);
            const int tagWidgetWidth = tagWidget->minimumSizeHint().width();

            // Calculate the width we're currently using in the current row. Does the new tag still fit in the current row?
            const bool isRowFull = width - usedSpaceInRow - tagWidgetWidth < 0;
            if (isRowFull || i == 0)
            {
                // Add a spacer widget after the last tag widget in a row to push the tag widgets to the left.
                if (i > 0)
                {
                    QWidget* spacerWidget = new QWidget();
                    spacerWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
                    hLayout->addWidget(spacerWidget);
                }

                // Add a new row for the current tag widget.
                hLayout = new QHBoxLayout();
                hLayout->setAlignment(Qt::AlignLeft);
                hLayout->setMargin(0);
                vLayout->addLayout(hLayout);

                // Reset the used space in the row.
                usedSpaceInRow = 0;
            }

            // Calculate the width of the tag widgets including the spacing between them of the current row.
            usedSpaceInRow += tagWidgetWidth + hLayout->spacing();

            // Add the tag widget to the current row.
            hLayout->addWidget(tagWidget);
        }
    }
} // namespace O3DE::ProjectManager