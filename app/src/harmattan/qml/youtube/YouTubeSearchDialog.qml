/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 1.1
import com.nokia.meego 1.0
import cuteTube 2.0
import ".."
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MySheet {
    id: root

    acceptButtonText: searchField.text ? qsTr("Done") : ""
    rejectButtonText: qsTr("Cancel")
    content: Item {
        anchors.fill: parent

        Column {
            id: column

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                topMargin: UI.PADDING_DOUBLE
            }

            MyTextField {
                id: searchField

                x: UI.PADDING_DOUBLE
                width: parent.width - UI.PADDING_DOUBLE * 2
                clearButtonEnabled: true
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: searchModel.setFilterFixedString(text)
                onAccepted: platformCloseSoftwareInputPanel()
            }

            Item {
                width: parent.width
                height: UI.PADDING_DOUBLE
            }

            ValueSelector {
                id: typeSelector

                width: parent.width
                title: qsTr("Search for")
                model: YouTubeSearchTypeModel {}
                value: Settings.defaultSearchType(Resources.YOUTUBE)
                onValueChanged: Settings.setDefaultSearchType(Resources.YOUTUBE, value)
            }

            ValueSelector {
                id: orderSelector

                width: parent.width
                title: qsTr("Order by")
                model: YouTubeSearchOrderModel {}
                value: Settings.defaultSearchOrder(Resources.YOUTUBE)
                onValueChanged: Settings.setDefaultSearchOrder(Resources.YOUTUBE, value)
            }
        }

        ListView {
            id: view

            anchors {
                left: parent.left
                right: parent.right
                top: column.bottom
                topMargin: UI.PADDING_DOUBLE
                bottom: parent.bottom
            }
            clip: true
            model: SearchHistoryModel {
                id: searchModel
            }
            header: SeparatorLabel {
                text: qsTr("Search history")
                x: UI.PADDING_DOUBLE
                width: view.width - UI.PADDING_DOUBLE
            }
            delegate: LabelDelegate {
                onClicked: searchField.text = display
                onPressAndHold: {
                    view.currentIndex = -1;
                    view.currentIndex = index;
                    contextMenu.open();
                }
            }

            ScrollDecorator {
                flickableItem: view
            }

            ContextMenu {
                id: contextMenu

                MenuLayout {

                    MenuItem {
                        text: qsTr("Remove")
                        onClicked: searchModel.removeSearch(view.currentIndex)
                    }

                    MenuItem {
                        text: qsTr("Clear")
                        onClicked: searchModel.clear()
                    }
                }
            }
        }

        states: State {
            name: "landscape"
            when: !appWindow.inPortrait

            AnchorChanges {
                target: column
                anchors.right: parent.horizontalCenter
            }

            AnchorChanges {
                target: view
                anchors {
                    left: column.right
                    top: parent.top
                }
            }
        }
    }

    onAccepted: {
        if (!mainPage.showResourceFromUrl(searchField.text)) {
            Settings.addSearch(searchField.text);
            mainPage.search(Resources.YOUTUBE, searchField.text, typeSelector.value, orderSelector.value);
        }
    }
    onStatusChanged: if (status == DialogStatus.Opening) searchField.text = "";
}