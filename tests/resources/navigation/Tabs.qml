/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
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
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

MainView {
    width: 800
    height: 600
    Tabs {
        id: tabs
        selectedTabIndex: 0
        Item {
            // does this mess up stuff? nope.
        }

        Tab {
            title: i18n.tr("Simple page")
            page: Page {
                title: "This title is not visible"
                Row {
                    anchors.centerIn: parent
                    spacing: units.gu(1)
                    width: childrenRect.width
                    height: childrenRect.height
                    Button {
                        text: "tab bar on"
                        enabled: !tabs.tabBar.selectionMode
                        onClicked: tabs.tabBar.selectionMode = true;
                    }
                    Button {
                        text: "tab bar off"
                        enabled: tabs.tabBar.selectionMode
                        onClicked: tabs.tabBar.selectionMode = false;
                    }
                }
                tools: ToolbarItems {
                    ToolbarButton {
                        text: "action"
                        iconSource: "call_icon.png"
                        onTriggered: print("action triggered")
                    }
                }
            }
        }
        Repeater {
            model: 3
            Tab {
                title: "Extra " + index
                page: Page {
                    Column {
                        anchors.centerIn: parent
                        width: units.gu(40)
                        Label {
                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                            text: "Extra tab number "+index
                        }
                        Button {
                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                            text: "Previous"
                            onClicked: if (tabs.selectedTabIndex > 0) tabs.selectedTabIndex--
                        }
                    }
                }
            }
        }
        Tab {
            id: externalTab
            title: i18n.tr("External")
            page: Loader {
                parent: externalTab
                anchors.fill: parent
                source: (tabs.selectedTab === externalTab) ? Qt.resolvedUrl("MyCustomPage.qml") : ""
            }
        }
        Tab {
            title: i18n.tr("List view")
            page: Page {
                ListView {
                    clip: true
                    anchors.fill: parent
                    model: 20
                    delegate: ListItem.Standard {
                        icon: Qt.resolvedUrl("call_icon.png")
                        text: "Item "+modelData
                    }
                }
            }
        }
    }
}