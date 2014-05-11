#-------------------------------------------------
#
# Project created by QtCreator 2011-02-11T20:40:53
#
#-------------------------------------------------

QT += core gui webkit sql network

CONFIG += static

TARGET = SVAMP
TEMPLATE = app


QMAKE_CXXFLAGS_RELEASE += -fpermissive
QMAKE_CXXFLAGS_DEBUG += -fpermissive
QMAKE_LFLAGS_RELEASE -= -all_load

LIBS += -lfl -ly

win32{
QMAKE_LIBDIR += ../ncl_dist/lib
QMAKE_LFLAGS+= -static-libgcc
}

FLEXSOURCES = lexer.l
BISONSOURCES = parser.y

OTHER_FILES +=  \
    $$FLEXSOURCES \
    $$BISONSOURCES

ICON = images/SVAMP.icns

RESOURCES = svamp.qrc
SOURCES += main.cpp\
    mainwindow.cpp \
    track.cpp \
    trackcontainer.cpp \
    tracksequence.cpp \
    annotation.cpp \
    trackannotation.cpp \
    variation.cpp \
    trackvariation.cpp \
    searchresult.cpp \
    sourcesdialog.cpp \
    trackgraph.cpp \
    groupmanager.cpp \
    sequence.cpp \
    colors.cpp \
    save.cpp \
    feedback.cpp \
    log.cpp \
    basecoveragedialog.cpp \
    point.cpp \
    pcoadialog.cpp \
    pcoa.cpp \
    ap.cpp \
    alglibmisc.cpp \
    alglibinternal.cpp \
    linalg.cpp \
    mapdialog.cpp \
    gmap.cpp \
    treeoptionsdialog.cpp \
    phylip.c \
    fitch.c \
    dist.c \
    treeviewdialog.cpp \
    treecalculation.cpp \
    worker.cpp \
    mapalleledialog.cpp \
    mapdatahandler.cpp \
    mappage.cpp \
    dbquery.cpp \
    sgraphicsview.cpp \
    dojopiemap.cpp \
    jbrowsesettingdialog.cpp \
    snpselectiondialog.cpp \
    nexusutils.cpp

HEADERS  += mainwindow.h \
    track.h \
    trackcontainer.h \
    tracksequence.h \
    annotation.h \
    trackannotation.h \
    variation.h \
    trackvariation.h \
    searchresult.h \
    sourcesdialog.h \
    trackgraph.h \
    groupmanager.h \
    trackgraphgc.h \
    trackgraphvd.h \
    sequence.h \
    trackgraphfst.h \
    trackgraphtd.h \
    colors.h \
    save.h \
    feedback.h \
    log.h \
    basecoveragedialog.h \
    point.h \
    pcoadialog.h \
    pcoa.h \
    ap.h \
    alglibmisc.h \
    alglibinternal.h \
    linalg.h \
    mapdialog.h \
    gmap.h \
    treeoptionsdialog.h \
    phylip.h \
    phylip.h \
    dist.h \
    treeviewdialog.h \
    treecalculation.h \
    worker.h \
    mapalleledialog.h \
    mapdatahandler.h \
    mappage.h \
    dbquery.h \
    sgraphicsview.h \
    gridscene.h \
    dojopiemap.h \
    jbrowsesettingdialog.h \
    snpselectiondialog.h \
    nexusutils.h

FORMS    += mainwindow.ui \
    trackcontainer.ui \
    sourcesdialog.ui \
    groupmanager.ui \
    colors.ui \
    save.ui \
    feedback.ui \
    log.ui \
    basecoveragedialog.ui \
    pcoadialog.ui \
    treeoptionsdialog.ui \
    treeviewdialog.ui \
    mapalleledialog.ui \
    jbrowsesettingdialog.ui \
    snpselectiondialog.ui \
    mapdialog.ui

OTHER_FILES += \
    label.js


flexsource.input = FLEXSOURCES
flexsource.output = ${QMAKE_FILE_BASE}.cpp
flexsource.commands = flex --header-file=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
flexsource.variable_out = SOURCES
flexsource.name = Flex Sources ${QMAKE_FILE_IN}
flexsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += flexsource

flexheader.input = FLEXSOURCES
flexheader.output = ${QMAKE_FILE_BASE}.h
flexheader.commands = @true
flexheader.variable_out = HEADERS
flexheader.name = Flex Headers ${QMAKE_FILE_IN}
flexheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += flexheader

bisonsource.input = BISONSOURCES
bisonsource.output = ${QMAKE_FILE_BASE}.cpp
bisonsource.commands = bison -d --defines=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
bisonsource.variable_out = SOURCES
bisonsource.name = Bison Sources ${QMAKE_FILE_IN}
bisonsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += bisonsource

bisonheader.input = BISONSOURCES
bisonheader.output = ${QMAKE_FILE_BASE}.h
bisonheader.commands = @true
bisonheader.variable_out = HEADERS
bisonheader.name = Bison Headers ${QMAKE_FILE_IN}
bisonheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += bisonheader


release: DESTDIR = build/release
debug:   DESTDIR = build/debug

unix{
OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui
}

QMAKE_EXTRA_TARGETS += mac_dist mac_dmg linux_dist linux_zip windows_dist win_zip
mac_dist.depends = mac_dmg

mac_dmg.target = $$DESTDIR/${QMAKE_TARGET}.dmg
mac_dmg.commands = cd $$DESTDIR && macdeployqt ${QMAKE_TARGET}.app -dmg
mac_dmg.depends = $(TARGET)

linux_dist.depends = linux_zip

linux_zip.target = $$DESTDIR/SVAMP_linux_x86_64.zip
linux_zip.commands = cd $$DESTDIR && mkdir -p SVAMP_linux_x86_64 && cp ../../$(TARGET) SVAMP_linux_x86_64 &&  zip -r SVAMP_linux_x86_64.zip SVAMP_linux_x86_64
linux_zip.depends = $(TARGET)

windows_dist.depends = win_zip

win_zip.target = build/release/SVAMP_win.zip
win_zip.commands = cd build/release && mkdir -p SVAMP_win && cp -r /c/users/mohar0a/downloads/SVAMP_2.04_win/svamp2.04/* SVAMP_win && cp SVAMP.exe SVAMP_win
win_zip.depends = $(TARGET)


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../ncl_dist/lib/ncl -lncl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../ncl_dist/lib/ncl -lncl
else:symbian: LIBS += -lncl
else:unix: LIBS += -L$$PWD/../ncl_dist/lib/ncl/ -lncl

INCLUDEPATH += $$PWD/../ncl_dist/include
DEPENDPATH += $$PWD/../ncl_dist/include

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../ncl_dist/lib/ncl/libncl.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../ncl_dist/lib/ncl/libncl.as
else:unix:!symbian: PRE_TARGETDEPS += $$PWD/../ncl_dist/lib/ncl/libncl.a
