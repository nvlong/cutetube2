/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import cuteTube 2.0
import QVimeo 1.0 as QVimeo
import ".."

Tab {
    id: root

    property alias model: userModel

    title: qsTr("Users")
    showProgressIndicator: userModel.status == QVimeo.ResourcesRequest.Loading
    tools: ToolBarLayout {

        BackToolButton {}

        MyToolButton {
            iconSource: "toolbar-refresh"
            toolTipText: qsTr("Reload")
            enabled: userModel.status != QVimeo.ResourcesRequest.Loading
            onClicked: userModel.reload()
        }
    }

    MyListView {
        id: view

        anchors.fill: parent
        cacheBuffer: 400
        model: VimeoUserModel {
            id: userModel

            onStatusChanged: if (status == QVimeo.ResourcesRequest.Failed) infoBanner.showMessage(errorString);
        }
        delegate: UserDelegate {
            onClicked: appWindow.pageStack.push(Qt.resolvedUrl("VimeoUserPage.qml")).load(userModel.get(index))
        }
    }

    MyScrollBar {
        flickableItem: view
    }

    Label {
        anchors {
            fill: parent
            margins: platformStyle.paddingLarge
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        font.pixelSize: 40
        color: platformStyle.colorNormalMid
        text: qsTr("No users found")
        visible: (userModel.status >= QVimeo.ResourcesRequest.Ready) && (userModel.count == 0)
    }
}
