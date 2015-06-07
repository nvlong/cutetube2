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
import com.nokia.symbian 1.1
import cuteTube 2.0
import ".."

MySheet {
    id: root

    function list(resourceId, title) {
        internal.resourceId = resourceId;
        internal.title = title;
        streamModel.list(resourceId);
    }

    acceptButtonText: streamModel.status == ResourcesRequest.Ready ? qsTr("Done") : ""
    rejectButtonText: qsTr("Cancel")
    content: Item {
        anchors.fill: parent

        MyFlickable {
            id: flicker

            anchors.fill: parent
            contentHeight: column.height + platformStyle.paddingLarge

            Column {
                id: column

                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }

                ValueSelector {
                    id: streamSelector

                    width: parent.width
                    title: qsTr("Video format")
                    focusItem: flicker
                    model: PluginStreamModel {
                        id: streamModel

                        service: Settings.currentService
                        onStatusChanged: {
                            switch (status) {
                            case ResourcesRequest.Loading: {
                                root.showProgressIndicator = true;
                                streamSelector.showProgressIndicator = true;
                                return;
                            }
                            case ResourcesRequest.Ready:
                                if (count > 0) {
                                    streamSelector.selectedIndex = Math.max(0, match("name",
                                                                            Settings.defaultDownloadFormat(service)));
                                }
                                else {
                                    infoBanner.showMessage(qsTr("No streams found"));
                                }

                                break;
                            case ResourcesRequest.Failed: {
                                infoBanner.showMessage(errorString);
                                break;
                            }
                            default:
                                break;
                            }

                            root.showProgressIndicator = false;
                            streamSelector.showProgressIndicator = false;
                        }
                    }
                    onAccepted: Settings.setDefaultDownloadFormat(streamModel.service,
                                                                  streamModel.data(selectedIndex, "name"))
                }
                
                ValueSelector {
                    id: categorySelector

                    width: parent.width
                    title: qsTr("Category")
                    focusItem: flicker
                    model: CategoryNameModel {
                        id: categoryModel
                    }
                    value: Settings.defaultCategory
                    onValueChanged: Settings.defaultCategory = value
                }
                
                MySwitch {
                    id: subtitleSwitch
            
                    text: qsTr("Download subtitles")
                    enabled: Plugins.resourceTypeIsSupported(subtitleModel.service, Resources.SUBTITLE);
                    onCheckedChanged: {
                        if (checked) {
                            if (subtitleModel.status != ResourcesRequest.Loading) {
                                subtitleModel.list(internal.resourceId);
                            }
                        }
                        else {
                            subtitleModel.cancel();
                        }
                    }
                }
                
                ValueSelector {
                    id: subtitleSelector

                    width: parent.width
                    title: qsTr("Video format")
                    focusItem: flicker
                    enabled: subtitleSwitch.checked
                    model: PluginSubtitleModel {
                        id: subtitleModel

                        service: Settings.currentService
                        onStatusChanged: {
                            switch (status) {
                            case ResourcesRequest.Loading: {
                                root.showProgressIndicator = true;
                                subtitleSelector.showProgressIndicator = true;
                                return;
                            }
                            case ResourcesRequest.Ready:
                                if (count > 0) {
                                    subtitleSelector.selectedIndex = Math.max(0, match("name",
                                                                                       Settings.subtitlesLanguage));
                                }
                                else {
                                    infoBanner.showMessage(qsTr("No subtitles found"));
                                }

                                break;
                            case ResourcesRequest.Failed: {
                                infoBanner.showMessage(errorString);
                                break;
                            }
                            default:
                                break;
                            }

                            root.showProgressIndicator = false;
                            streamSelector.showProgressIndicator = false;
                        }
                    }
                    onAccepted: Settings.setDefaultDownloadFormat(streamModel.service,
                                                                  streamModel.data(selectedIndex, "name"))
                }                
            }
        }

        MyScrollBar {
            flickableItem: flicker
        }
    }

    QtObject {
        id: internal

        property string resourceId
        property string title
    }

    onAccepted: Transfers.addDownloadTransfer(streamModel.service, internal.resourceId, streamSelector.value.id,
                                              internal.title, Settings.defaultCategory,
                                              subtitleSwitch.checked ?
                                              subtitleModel.data(subtitleSelector.selectedIndex, "name") : "")

    onStatusChanged: if ((status == DialogStatus.Closing)
                         && (streamModel.status == ResourcesRequest.Loading)) streamModel.cancel();
}
