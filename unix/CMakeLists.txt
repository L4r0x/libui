# 3 june 2016

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

list(APPEND _LIBUI_SOURCES
	unix/alloc.c
	unix/box.c
	unix/button.c
	unix/checkbox.c
	unix/colorbutton.c
	unix/combobox.c
	unix/control.c
	unix/datetimepicker.c
	unix/debug.c
	unix/editablecombo.c
	unix/entry.c
	unix/future.c
	unix/grid.c
	unix/group.c
	unix/image.c
	unix/label.c
	unix/main.c
	unix/menu.c
	unix/multilineentry.c
	unix/progressbar.c
	unix/radiobuttons.c
	unix/separator.c
	unix/slider.c
	unix/spinbox.c
	unix/stddialogs.c
	unix/tab.c
	unix/table.c
	unix/tablebutton.c
	unix/tablemodel.c
	unix/text.c
	unix/util.c
	unix/window.c
)
set(_LIBUI_SOURCES ${_LIBUI_SOURCES} PARENT_SCOPE)

list(APPEND _LIBUI_INCLUDEDIRS
	unix
)
set(_LIBUI_INCLUDEDIRS _LIBUI_INCLUDEDIRS PARENT_SCOPE)

# TODO the other variables don't work?
set(_LIBUI_CFLAGS
	${GTK_CFLAGS}
PARENT_SCOPE)

set(_LIBUI_LIBS
	${GTK_LDFLAGS} m ${CMAKE_DL_LIBS}
PARENT_SCOPE)
