# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2012, 2013 Canonical Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

"""Tests for the Ubuntu UI Toolkit Gallery"""

import os
import shutil

from autopilot.matchers import Eventually
from testtools.matchers import Is, Not, Equals

from ubuntuuitoolkit import tests


class GalleryTestCase(tests.QMLFileAppTestCase):
    """Base class for gallery test cases."""

    local_desktop_file_path = None

    def setUp(self):
        if self._application_source_exists():
            self.test_source_path = self._get_path_to_gallery_source()
        else:
            self.test_source_path = self._get_path_to_installed_gallery()
        assert os.path.exists(self.test_source_path)
        self.test_qml_file_path = self._get_test_qml_file_path()
        self.desktop_file_path = self._get_desktop_file_path()
        super(GalleryTestCase, self).setUp()

    def _get_path_to_gallery_source(self):
        return os.path.join(
            tests.get_path_to_source_root(), 'examples',
            'ubuntu-ui-toolkit-gallery')

    def _application_source_exists(self):
        return 'UBUNTU_UI_TOOLKIT_AUTOPILOT_FROM_SOURCE' in os.environ

    def _get_test_qml_file_path(self):
        return os.path.join(
            self.test_source_path,
            'ubuntu-ui-toolkit-gallery.qml')

    def _get_path_to_installed_gallery(self):
        return '/usr/lib/ubuntu-ui-toolkit/examples/ubuntu-ui-toolkit-gallery'

    def _get_desktop_file_path(self):
        desktop_file_path = os.path.join(
            self.test_source_path,
            'ubuntu-ui-toolkit-gallery.desktop')
        if self._application_source_exists():
            local_desktop_file_dir = tests.get_local_desktop_file_directory()
            if not os.path.exists(local_desktop_file_dir):
                os.makedirs(local_desktop_file_dir)
            local_desktop_file_path = os.path.join(
                local_desktop_file_dir, 'ubuntu-ui-toolkit-gallery.desktop')
            shutil.copy(desktop_file_path, local_desktop_file_path)
            # We can't delete the desktop file before we close the application,
            # so we save it on an attribute to be deleted on tear down.
            self.local_desktop_file_path = local_desktop_file_path
            return local_desktop_file_path
        else:
            return desktop_file_path

    def tearDown(self):
        super(GalleryTestCase, self).tearDown()
        # We can't delete the desktop file before we close the application,
        # so we save it on an attribute to be deleted on tear down.
        if self.local_desktop_file_path is not None:
            os.remove(self.local_desktop_file_path)


class GenericTests(GalleryTestCase):
    """Generic tests for the Gallery"""

    def test_0_can_select_mainwindow(self):
        """Must be able to select the main window."""

        rootItem = self.main_view
        self.assertThat(rootItem, Not(Is(None)))
        self.assertThat(rootItem.visible, Eventually(Equals(True)))

    def test_navigation(self):
        item = "Navigation"
        self.loadItem(item)
        self.checkPageHeader(item)

    def test_toggles(self):
        item = "Toggles"
        self.checkListItem(item)
        self.loadItem(item)
        self.checkPageHeader(item)

        # check default states
        item_data = [
            ["checkbox_unchecked", False, True],
            ["checkbox_checked", True, True],
            ["checkbox_disabled_unchecked", False, False],
            ["checkbox_disabled_checked", True, False],
            ["switch_unchecked", False, True],
            ["switch_checked", True, True],
            ["switch_disabled_unchecked", False, False],
            ["switch_disabled_checked", True, False]
        ]

        for data in item_data:
            objName = data[0]
            objChecked = data[1]
            objEnabled = data[2]

            obj = self.getObject(objName)
            self.assertThat(obj.checked, Equals(objChecked))
            self.assertThat(obj.enabled, Equals(objEnabled))

            # try to interact with objects
            self.tap(objName)

            if (obj.enabled):
                self.assertThat(obj.checked, Not(Equals(objChecked)))
            else:
                self.assertThat(obj.checked, Equals(objChecked))

    def test_slider(self):
        item = "Slider"
        self.loadItem(item)
        self.checkPageHeader(item)

        item_data = [
            ["slider_standard"],
            ["slider_live"],
            ["slider_range"]
        ]

        for data in item_data:
            objName = data[0]
            self.getObject(objName)
            self.tap(objName)

            # TODO: move slider value

#     def test_textarea(self):
#         item = "Text Field"
#         self.loadItem(item)
#         self.checkPageHeader(item)

#         template_textinputs = self.getObject("textinputs")

#         item_data = [
#         ["textarea_default", True, -1, template_textinputs.longText, None ],
#             [ "textarea_expanding", True, -1, "", None],
#         [ "textarea_richtext", True, -1, template_textinputs.richText, None ]
#         ]

#         for data in item_data:
#             objName = data[0]
#             objEnabled = data[1]
#             objEchoMode = data[2]
#             objText = data[3]
#             objNumbersOnly = data[4]

#             obj = self.getObject(objName)
#             self.tap(objName)

#             self.assertThat(obj.enabled, Equals(objEnabled))
# #            self.assertThat(obj.focus, Equals(obj.enabled))
#             self.assertThat(obj.highlighted, Equals(obj.focus))
#             #self.assertThat(obj.hasClearButton, Equals(True))
#             self.assertThat(obj.text, Equals(objText))

            #TODO: There is no clear button or method to clear a textarea?
            #self.tap_clearButton(objName)

            # self.assertThat(obj.text,Equals(""))

            # self.type_string("Hello World!")

            # self.assertThat(obj.text,Equals("Hello World!"))

    def test_progress_and_activity(self):
        item = "Progress and activity"
        self.loadItem(item)
        self.checkPageHeader(item)

        item_data = [
            ["progressbar_standard"],
            ["progressbar_indeterminate"],
            ["activityindicator_standard"]
        ]

        for data in item_data:
            objName = data[0]
            self.getObject(objName)
            self.tap(objName)

            # TODO: check for properties

    def test_ubuntushape(self):
        item = "Ubuntu Shape"
        self.loadItem(item)
        self.checkPageHeader(item)

        item_data = [
            ["ubuntushape_color_hex"],
            ["ubuntushape_color_lightblue"],
            ["ubuntushape_color_darkgray"],
            ["ubuntushape_image"],
            ["ubuntushape_radius_small"],
            ["ubuntushape_radius_medium"],
            ["ubuntushape_sizes_15_6"],
            ["ubuntushape_sizes_10_14"]
        ]

        for data in item_data:
            objName = data[0]
            self.getObject(objName)


class ButtonsTestCase(GalleryTestCase):

    scenarios = [
        ('standard button', dict(
            button_name="button_text", is_enabled=True, color=None, icon=None,
            text="Call")),
        ('button with color', dict(
            button_name="button_color", is_enabled=True,
            color=[0, 0, 0, 255], icon=None, text="Call")),
        ('button with icon', dict(
            button_name="button_iconsource", is_enabled=True, color=None,
            icon="call.png", text=None)),
        ('button with icon on the right', dict(
            button_name="button_iconsource_right_text", is_enabled=True,
            color=None, icon="call.png", text="Call")),
        ('button with icon on the left', dict(
            button_name="button_iconsource_left_text", is_enabled=True,
            color=None, icon="call.png", text="Call")),
        ('disabled button', dict(
            button_name="button_text_disabled", is_enabled=False, color=None,
            icon=None, text="Call"))
    ]

    def test_buttons(self):
        item = "Buttons"
        self.loadItem(item)
        self.checkPageHeader(item)

        button = self.app.select_single(objectName=self.button_name)
        self.assertIsNot(button, None)
        self.assertThat(button.enabled, Equals(self.is_enabled))

        if self.color is not None:
            self.assertThat(button.color, Equals(self.color))

        if self.icon is not None:
            self.assertTrue(button.iconSource.endswith(self.icon))

        if self.text is not None:
            self.assertThat(button.text, Equals(self.text))

        # try to interact with objects
        self.pointing_device.move_to_object(button)
        self.pointing_device.press()

        if button.enabled:
            self.assertThat(button.pressed, Eventually(Equals(True)))
        else:
            self.assertFalse(button.pressed)

        self.pointing_device.release()

        self.assertThat(button.pressed, Eventually(Equals(False)))

    def test_all_pages_load(self):
        # TODO: Iterate items in widgetList
        for item in ['Navigation', 'Slider', 'Pickers', 'Icons']:
            self.loadItem(item)
            self.checkPageHeader(item)
            assert len(self.app.select_many('Template')) > 0, \
                ('%s can be loaded in gallery' % item)

