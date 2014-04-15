QT += network qml quick webkitwidgets script scripttools declarative

include(../plugin.pri)

#####################################
# required for Ubuntu Device Notifier
CONFIG += link_pkgconfig

PKGCONFIG += libudev
#####################################

#####################################
## Project files

FORMS += \
    ubuntudeviceswidget.ui \
    ubuntupackagingwidget.ui \
    ubuntusettingswidget.ui \
    ubuntusecuritypolicypickerdialog.ui \
    ubuntusettingsdeviceconnectivitywidget.ui \
    ubuntusettingsclickwidget.ui \
    ubuntuclickdialog.ui \
    ubuntucreatenewchrootdialog.ui \
    ubuntudeviceconfigurationwidget.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    UbuntuProject.mimetypes.xml \
    manifest.json.template \
    myapp.json.template \
    manifestlib.js

SOURCES += \
    ubuntuplugin.cpp \
    ubuntuwelcomemode.cpp \
    ubuntuprojectapplicationwizard.cpp \
    ubuntumenu.cpp \
    ubuntuprojectmanager.cpp \
    ubuntuproject.cpp \
    ubuntuprojectfile.cpp \
    ubuntuprojectnode.cpp \
    ubuntuprojectapp.cpp \
    ubuntuversion.cpp \
    ubuntufeatureprovider.cpp \
    ubuntuversionmanager.cpp \
    ubuntuircmode.cpp \
    ubuntuapimode.cpp \
    ubuntucoreappsmode.cpp \
    ubuntuwikimode.cpp \
    ubuntupackagingmode.cpp \
    ubuntupackagingwidget.cpp \
    ubuntubzr.cpp \
    ubuntuclickmanifest.cpp \
    ubuntuwebmode.cpp \
    ubuntupastebinmode.cpp \
    ubuntudeviceswidget.cpp \
    ubuntudevicemode.cpp \
    ubuntuprocess.cpp \
    ubuntudevicenotifier.cpp \
    ubuntusettingspage.cpp \
    ubuntusettingswidget.cpp \
    ubuntusecuritypolicypickerdialog.cpp \
    ubuntupolicygroupmodel.cpp \
    ubuntupolicygroupinfo.cpp \
    ubuntusettingsdeviceconnectivitypage.cpp \
    ubuntusettingsdeviceconnectivitywidget.cpp \
    ubuntusettingsclickpage.cpp \
    ubuntusettingsclickwidget.cpp \
    ubuntuclicktool.cpp \
    ubuntucreatenewchrootdialog.cpp \
    ubuntuclickdialog.cpp \
    ubuntuvalidationresultmodel.cpp \
    clicktoolchain.cpp \
    ubuntukitmanager.cpp \
    ubuntucmaketool.cpp \
    ubuntucmakebuildconfiguration.cpp \
    ubuntudevicefactory.cpp \
    ubuntudevice.cpp \
    ubuntudeviceconfigurationwidget.cpp \
    ubunturemoterunconfiguration.cpp \
    ubuntucmakemakestep.cpp \
    ubuntuemulatornotifier.cpp \
    localportsmanager.cpp \
    ubuntuprojectguesser.cpp \
    ubuntulocaldeployconfiguration.cpp \
    ubunturemotedeployconfiguration.cpp \
    ubuntulocalrunconfigurationfactory.cpp \
    ubunturemoteruncontrolfactory.cpp \
    ubuntulocalrunconfiguration.cpp

HEADERS += \
    ubuntuplugin.h \
    ubuntu_global.h \
    ubuntuconstants.h \
    ubuntuwelcomemode.h \
    ubuntuprojectapplicationwizard.h \
    ubuntumenu.h \
    ubuntushared.h \
    ubuntuprojectmanager.h \
    ubuntuproject.h \
    ubuntuprojectfile.h \
    ubuntuprojectnode.h \
    ubuntuprojectapp.h \
    ubuntuversion.h \
    ubuntufeatureprovider.h \
    ubuntuversionmanager.h \
    ubuntuircmode.h \
    ubuntuapimode.h \
    ubuntucoreappsmode.h \
    ubuntuwikimode.h \
    ubuntupackagingmode.h \
    ubuntupackagingwidget.h \
    ubuntubzr.h \
    ubuntuclickmanifest.h \
    ubuntuwebmode.h \
    ubuntupastebinmode.h \
    ubuntudevicemode.h \
    ubuntudeviceswidget.h \
    ubuntuprocess.h \
    ubuntudevicenotifier.h \
    ubuntusettingspage.h \
    ubuntusettingswidget.h \
    ubuntusecuritypolicypickerdialog.h \
    ubuntupolicygroupmodel.h \
    ubuntupolicygroupinfo.h \
    ubuntusettingsdeviceconnectivitypage.h \
    ubuntusettingsdeviceconnectivitywidget.h \
    ubuntusettingsclickpage.h \
    ubuntusettingsclickwidget.h \
    ubuntuclicktool.h \
    ubuntucreatenewchrootdialog.h \
    ubuntuclickdialog.h \
    ubuntuvalidationresultmodel.h \
    clicktoolchain.h \
    ubuntukitmanager.h \
    ubuntucmaketool.h \
    ubuntucmakebuildconfiguration.h \
    ubuntudevicefactory.h \
    ubuntudevice.h \
    ubuntudeviceconfigurationwidget.h \
    ubunturemoterunconfiguration.h \
    ubuntucmakemakestep.h \
    ubuntuemulatornotifier.h \
    localportsmanager.h \
    ubuntuprojectguesser.h \
    ubuntulocaldeployconfiguration.h \
    ubunturemotedeployconfiguration.h \
    ubuntulocalrunconfigurationfactory.h \
    ubunturemoteruncontrolfactory.h \
    ubuntulocalrunconfiguration.h
