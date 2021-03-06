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

#ifndef VIMEOVIDEOMODEL_H
#define VIMEOVIDEOMODEL_H

#include "vimeovideo.h"
#include <QAbstractListModel>

class VimeoPlaylist;

class VimeoVideoModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(bool canFetchMore READ canFetchMore NOTIFY statusChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(QVimeo::ResourcesRequest::Status status READ status NOTIFY statusChanged)
    
public:
    enum Roles {
        DateRole = Qt::UserRole + 1,
        DescriptionRole,
        DownloadableRole,
        DurationRole,
        FavouriteRole,
        IdRole,
        LargeThumbnailUrlRole,
        StreamUrlRole,
        SubtitlesRole,
        ThumbnailUrlRole,
        TitleRole,
        UrlRole,
        UserIdRole,
        UsernameRole,
        ViewCountRole
    };
    
    explicit VimeoVideoModel(QObject *parent = 0);
    
    QString errorString() const;
    
    QVimeo::ResourcesRequest::Status status() const;
    
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    
    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent = QModelIndex());
    
    QVariant data(const QModelIndex &index, int role) const;
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
    Q_INVOKABLE QVariantMap itemData(int row) const;
    
    Q_INVOKABLE VimeoVideo* get(int row) const;
    
    Q_INVOKABLE void list(const QString &resourcePath, const QVariantMap &filters = QVariantMap());

public Q_SLOTS:
    void clear();
    void cancel();
    void reload();
    
private:
    void append(VimeoVideo *video);
    void insert(int row, VimeoVideo *video);
    void remove(int row);
    
private Q_SLOTS:
    void onRequestFinished();
    void onVideoAddedToPlaylist(VimeoVideo *video, VimeoPlaylist *playlist);
    void onVideoRemovedFromPlaylist(VimeoVideo *video, VimeoPlaylist *playlist);
    void onVideoFavourited(VimeoVideo *video);
    void onVideoUnfavourited(VimeoVideo *video);
    void onVideoWatchLater(VimeoVideo *video);
    
Q_SIGNALS:
    void countChanged(int count);
    void statusChanged(QVimeo::ResourcesRequest::Status s);
    
private:
    QVimeo::ResourcesRequest *m_request;
    
    QString m_resourcePath;
    QVariantMap m_filters;
    bool m_hasMore;
        
    QList<VimeoVideo*> m_items;
    
    QHash<int, QByteArray> m_roles;
};
    
#endif // VIMEOVIDEOMODEL_H
