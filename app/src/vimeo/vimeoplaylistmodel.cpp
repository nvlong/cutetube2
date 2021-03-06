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

#include "vimeoplaylistmodel.h"
#include "logger.h"
#include "vimeo.h"

VimeoPlaylistModel::VimeoPlaylistModel(QObject *parent) :
    QAbstractListModel(parent),
    m_request(new QVimeo::ResourcesRequest(this)),
    m_hasMore(false)
{
    m_roles[DateRole] = "date";
    m_roles[DescriptionRole] = "description";
    m_roles[IdRole] = "id";
    m_roles[LargeThumbnailUrlRole] = "largeThumbnailUrl";
    m_roles[PrivacyRole] = "privacy";
    m_roles[ThumbnailUrlRole] = "thumbnailUrl";
    m_roles[TitleRole] = "title";
    m_roles[UserIdRole] = "userId";
    m_roles[UsernameRole] = "username";
    m_roles[VideoCountRole] = "videoCount";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
    m_request->setClientId(Vimeo::clientId());
    m_request->setClientSecret(Vimeo::clientSecret());
    m_request->setAccessToken(Vimeo::accessToken());
    
    connect(m_request, SIGNAL(accessTokenChanged(QString)), Vimeo::instance(), SLOT(setAccessToken(QString)));
    connect(m_request, SIGNAL(finished()), this, SLOT(onRequestFinished()));
}

QString VimeoPlaylistModel::errorString() const {
    return Vimeo::getErrorString(m_request->result().toMap());
}

QVimeo::ResourcesRequest::Status VimeoPlaylistModel::status() const {
    return m_request->status();
}

#if QT_VERSION >=0x050000
QHash<int, QByteArray> VimeoPlaylistModel::roleNames() const {
    return m_roles;
}
#endif

int VimeoPlaylistModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

bool VimeoPlaylistModel::canFetchMore(const QModelIndex &) const {
    return (status() != QVimeo::ResourcesRequest::Loading) && (m_hasMore);
}

void VimeoPlaylistModel::fetchMore(const QModelIndex &) {
    if (!canFetchMore()) {
        return;
    }
    
    const int page = m_filters.value("page").toInt();
    m_filters["page"] = (page > 0 ? page + 1 : 2);
    m_request->list(m_resourcePath, m_filters);
    emit statusChanged(status());
}

QVariant VimeoPlaylistModel::data(const QModelIndex &index, int role) const {
    if (const VimeoPlaylist *playlist = get(index.row())) {
        return playlist->property(m_roles[role]);
    }
    
    return QVariant();
}

QMap<int, QVariant> VimeoPlaylistModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    
    if (const VimeoPlaylist *playlist = get(index.row())) {
        QHashIterator<int, QByteArray> iterator(m_roles);
        
        while (iterator.hasNext()) {
            iterator.next();
            map[iterator.key()] = playlist->property(iterator.value());
        }
    }
    
    return map;
}

QVariant VimeoPlaylistModel::data(int row, const QByteArray &role) const {
    if (const VimeoPlaylist *playlist = get(row)) {
        return playlist->property(role);
    }
    
    return QVariant();
}

QVariantMap VimeoPlaylistModel::itemData(int row) const {
    QVariantMap map;
    
    if (const VimeoPlaylist *playlist = get(row)) {
        foreach (const QByteArray &role, m_roles.values()) {
            map[role] = playlist->property(role);
        }
    }
    
    return map;
}

VimeoPlaylist* VimeoPlaylistModel::get(int row) const {
    if ((row >= 0) && (row < m_items.size())) {
        return m_items.at(row);
    }
    
    return 0;
}

void VimeoPlaylistModel::list(const QString &resourcePath, const QVariantMap &filters) {
    if (status() == QVimeo::ResourcesRequest::Loading) {
        return;
    }
    
    Logger::log("VimeoPlaylistModel::list(). Resource path: " + resourcePath, Logger::HighVerbosity);
    clear();
    m_resourcePath = resourcePath;
    m_filters = filters;
    m_request->list(resourcePath, filters);
    emit statusChanged(status());
    
    disconnect(Vimeo::instance(), 0, this, 0);
    
    if (resourcePath == "/me/playlists") {
        connect(Vimeo::instance(), SIGNAL(playlistCreated(VimeoPlaylist*)),
                this, SLOT(onPlaylistCreated(VimeoPlaylist*)));
        connect(Vimeo::instance(), SIGNAL(playlistDeleted(VimeoPlaylist*)),
                this, SLOT(onPlaylistDeleted(VimeoPlaylist*)));
    }
}

void VimeoPlaylistModel::clear() {
    if (!m_items.isEmpty()) {
        beginResetModel();
        qDeleteAll(m_items);
        m_items.clear();
        m_hasMore = false;
        endResetModel();
        emit countChanged(rowCount());
    }
}

void VimeoPlaylistModel::cancel() {
    m_request->cancel();
}

void VimeoPlaylistModel::reload() {
    if (status() == QVimeo::ResourcesRequest::Loading) {
        return;
    }
    
    Logger::log("VimeoPlaylistModel::reload(). Resource path: " + m_resourcePath, Logger::HighVerbosity);
    clear();
    m_request->list(m_resourcePath, m_filters);
    emit statusChanged(status());
}

void VimeoPlaylistModel::append(VimeoPlaylist *playlist) {
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items << playlist;
    endInsertRows();
}

void VimeoPlaylistModel::insert(int row, VimeoPlaylist *playlist) {
    if ((row >= 0) && (row < m_items.size())) {
        beginInsertRows(QModelIndex(), row, row);
        m_items.insert(row, playlist);
        endInsertRows();
    }
    else {
        append(playlist);
    }
}

void VimeoPlaylistModel::remove(int row) {
    if ((row >= 0) && (row < m_items.size())) {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.takeAt(row)->deleteLater();
        endRemoveRows();
    }
}

void VimeoPlaylistModel::onRequestFinished() {
    if (m_request->status() == QVimeo::ResourcesRequest::Ready) {
        const QVariantMap result = m_request->result().toMap();
        
        if (!result.isEmpty()) {
            m_hasMore = !result.value("paging").toMap().value("next").isNull();
            const QVariantList list = result.value("data").toList();

            beginInsertRows(QModelIndex(), m_items.size(), m_items.size() + list.size() - 1);
    
            foreach (const QVariant &item, list) {
                m_items << new VimeoPlaylist(item.toMap(), this);
            }

            endInsertRows();
            emit countChanged(rowCount());
        }
    }
    else {
        Logger::log("VimeoPlaylistModel::onRequestFinished(). Error: " + errorString());
    }
    
    emit statusChanged(status());
}

void VimeoPlaylistModel::onPlaylistCreated(VimeoPlaylist *playlist) {
    insert(0, new VimeoPlaylist(playlist, this));
}

void VimeoPlaylistModel::onPlaylistDeleted(VimeoPlaylist *playlist) {
    const QModelIndexList list = match(index(0), IdRole, playlist->id(), 1, Qt::MatchExactly);
    
    if (!list.isEmpty()) {
        remove(list.first().row());
    }
}
