import QtQuick 2.0
import QtTest 1.0
import Ubuntu.Components.ListItems 0.1 as ListItem

TestCase {
     name: "ListItemsThinDividerAPI"

     function test_divider() {
        verify((listItemThinDivider),"ThinDivider can be loaded")
     }

     ListItem.ThinDivider {
         id: listItemThinDivider
     }
}