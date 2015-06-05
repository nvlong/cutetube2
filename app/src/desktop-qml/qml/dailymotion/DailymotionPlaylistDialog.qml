/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import cuteTube 2.0
import QDailymotion 1.0 as QDailymotion
import ".."

MyDialog {
    id: root
        
    function load(videoOrId) {
        video.loadVideo(videoOrId);
    }
        
    minimumWidth: 400
    minimumHeight: 400
    title: qsTr("Add to playlist")
    content: TabView {
        id: tabView
        
        anchors.fill: parent
        enabled: playlist.status != QDailymotion.ResourcesRequest.Loading
        
        Tab {
            id: playlistsTab
            
            title: qsTr("Playlists")
            
            ItemView {
                id: playlistsView
                
                model: DailymotionPlaylistModel {
                    id: playlistModel

                    onStatusChanged: if (status == QDailymotion.ResourcesRequest.Failed) messageBox.showError(errorString);
                }
                delegate: PlaylistDelegate {
                    onClicked: {
                        playlist.loadPlaylist(playlistModel.get(index));
                        playlist.addVideo(video);
                    }
                }
            }
        }
        
        Tab {
            id: newPlaylistTab
            
            title: qsTr("New playlist")
            
            GridLayout {
                id: grid
                
                property alias title: titleField.text
                property alias description: descriptionField.text
                
                function clear() {
                    titleField.text = "";
                    descriptionField.text = "";
                }
                
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: 10
                }
                columns: 2
                columnSpacing: 10
                rowSpacing: 10
                
                Label {
                    text: qsTr("Title")
                }
                
                TextField {
                    id: titleField
                    
                    Layout.fillWidth: true
                }
                
                Label {
                    text: qsTr("Description")
                }
                
                TextField {
                    id: descriptionField
                    
                    Layout.fillWidth: true
                }
            }
        }       
    }
    buttons: [
        Button {
            text: qsTr("&Cancel")
            iconName: "dialog-cancel"
            onClicked: root.reject()
        },
        
        Button {
            text: qsTr("&Ok")
            iconName: "dialog-ok"
            isDefault: true
            enabled: (tabView.currentIndex == 1) && (newPlaylistTab.item.title)
            onClicked: {
                var p = {};
                p["name"] = titleField.text;

                if (descriptionField.text) {
                    p["description"] = descriptionField.text;
                }

                playlist.addVideo(p, video);
            }
        }
    ]
    
    DailymotionVideo {
        id: video
    }

    DailymotionPlaylist {
        id: playlist

        onStatusChanged: {
            switch (status) {
            case QDailymotion.ResourcesRequest.Ready:
                root.accept();
                break;
            case QDailymotion.ResourcesRequest.Failed:
                messageBox.showError(errorString);
                break;
            default:
                break;
            }
        }
    }
    
    onOpened: playlistsTab.item.model.list("/me/playlists", {limit: MAX_RESULTS})
    onClosed: {
        tabView.currentIndex = 0;
        
        if (newPlaylistTab.item) {
            newPlaylistTab.item.clear();
        }
    }
}
