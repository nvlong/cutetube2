/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "youtubedownloaddialog.h"
#include "categorynamemodel.h"
#include "resources.h"
#include "settings.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

YouTubeDownloadDialog::YouTubeDownloadDialog(QWidget *parent) :
    QDialog(parent),
    m_streamModel(new YouTubeStreamModel(this)),
    m_subtitleModel(new YouTubeSubtitleModel(this)),
    m_categoryModel(new CategoryNameModel(this)),
    m_streamSelector(new QComboBox(this)),
    m_subtitleSelector(new QComboBox(this)),
    m_categorySelector(new QComboBox(this)),
    m_subtitleCheckBox(new QCheckBox(tr("Download &subtitles"), this)),
    m_commandCheckBox(new QCheckBox(tr("&Override global custom command"), this)),
    m_commandEdit(new QLineEdit(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Download video"));

    m_streamSelector->setModel(m_streamModel);

    m_subtitleSelector->setModel(m_subtitleModel);

    m_categorySelector->setModel(m_categoryModel);
    
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    m_layout->addRow(tr("Video &format:"), m_streamSelector);
    m_layout->addRow(m_subtitleCheckBox);
    m_layout->addRow(tr("Subtitles &language:"), m_subtitleSelector);
    m_layout->addRow(tr("&Category:"), m_categorySelector);
    m_layout->addRow(tr("&Custom command:"), m_commandEdit);
    m_layout->addRow(m_commandCheckBox);
    m_layout->addRow(m_buttonBox);

    connect(m_streamModel, SIGNAL(statusChanged(QYouTube::StreamsRequest::Status)),
            this, SLOT(onStreamModelStatusChanged(QYouTube::StreamsRequest::Status)));
    connect(m_subtitleModel, SIGNAL(statusChanged(QYouTube::SubtitlesRequest::Status)),
            this, SLOT(onSubtitleModelStatusChanged(QYouTube::SubtitlesRequest::Status)));
    connect(m_subtitleCheckBox, SIGNAL(toggled(bool)), this, SLOT(setSubtitlesEnabled(bool)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QString YouTubeDownloadDialog::videoId() const {
    return m_videoId;
}

QString YouTubeDownloadDialog::streamId() const {
    return m_streamSelector->itemData(m_streamSelector->currentIndex()).toMap().value("id").toString();
}

QString YouTubeDownloadDialog::subtitlesLanguage() const {
    return m_subtitleCheckBox->isChecked() ? m_subtitleSelector->currentText() : QString();
}

QString YouTubeDownloadDialog::category() const {
    return m_categorySelector->currentText();
}

QString YouTubeDownloadDialog::customCommand() const {
    return m_commandEdit->text();
}

bool YouTubeDownloadDialog::customCommandOverrideEnabled() const {
    return m_commandCheckBox->isChecked();
}

void YouTubeDownloadDialog::accept() {
    Settings::setDefaultDownloadFormat(Resources::YOUTUBE, m_streamSelector->currentText());
    Settings::setSubtitlesEnabled(m_subtitleCheckBox->isChecked());
    Settings::setSubtitlesLanguage(subtitlesLanguage());
    Settings::setDefaultCategory(category());
    QDialog::accept();
}

void YouTubeDownloadDialog::list(const QString &videoId) {
    m_videoId = videoId;
    m_streamModel->list(videoId);
    
    if (m_subtitleCheckBox->isChecked()) {
        m_subtitleModel->list(videoId);
    }
    else {
        m_subtitleCheckBox->setChecked(Settings::subtitlesEnabled());
    }
}

void YouTubeDownloadDialog::setSubtitlesEnabled(bool enabled) {
    if (enabled) {
        m_subtitleModel->list(m_videoId);
    }
    else {
        m_subtitleModel->cancel();
    }
}

void YouTubeDownloadDialog::onStreamModelStatusChanged(QYouTube::StreamsRequest::Status status) {
    switch (status) {
    case QYouTube::StreamsRequest::Loading:
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    case QYouTube::StreamsRequest::Ready:
        m_streamSelector->setCurrentIndex(qMax(0, m_streamModel->match("name",
                                          Settings::defaultDownloadFormat(Resources::YOUTUBE))));
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_subtitleModel->status()
                                                              != QYouTube::SubtitlesRequest::Loading);
        break;
    case QYouTube::StreamsRequest::Failed:
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        QMessageBox::critical(this, tr("Error"), m_streamModel->errorString());
        break;
    default:
        break;
    }
}

void YouTubeDownloadDialog::onSubtitleModelStatusChanged(QYouTube::SubtitlesRequest::Status status) {
    switch (status) {    
    case QYouTube::SubtitlesRequest::Loading:
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    case QYouTube::SubtitlesRequest::Ready:
        m_subtitleSelector->setCurrentIndex(qMax(0, m_streamModel->match("name", Settings::subtitlesLanguage())));
        break;
    case QYouTube::SubtitlesRequest::Failed:
        QMessageBox::critical(this, tr("Error"), m_subtitleModel->errorString());
        break;
    default:
        break;
    }

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_streamModel->status()
                                                          == QYouTube::StreamsRequest::Ready);
}
