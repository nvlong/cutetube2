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

#ifndef DAILYMOTIONPLAYBACKDIALOG_H
#define DAILYMOTIONPLAYBACKDIALOG_H

#include "dialog.h"
#include "dailymotionstreammodel.h"

class ValueSelector;
class QDialogButtonBox;
class QHBoxLayout;

class DailymotionPlaybackDialog : public Dialog
{
    Q_OBJECT

    Q_PROPERTY(QString streamUrl READ streamUrl)
    
public:
    explicit DailymotionPlaybackDialog(QWidget *parent = 0);

    QString streamUrl() const;
    
public Q_SLOTS:
    virtual void accept();
    
    void list(const QString &videoId);
    
private Q_SLOTS:
    void onModelStatusChanged(QDailymotion::StreamsRequest::Status status);;
    
private:
    DailymotionStreamModel *m_model;
    
    ValueSelector *m_streamSelector;
    QDialogButtonBox *m_buttonBox;
    QHBoxLayout *m_layout;
};

#endif // DAILYMOTIONPLAYBACKDIALOG_H
