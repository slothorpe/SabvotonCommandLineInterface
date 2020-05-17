# SabvotonCommandLineInterface
simple command-line interface for MQ Sabvoton BLDC controller using ModBus-Master library

This is a sipmle arduino sketch providing a command line interface to the MQ Sabvoton BLDC controller using the ModBus-Master library.
Also there is a screenshot showing the usage and a spreadsheet with the known registers of the Sabvoton.

Please be carefull with writing in unknown registers, you might kill your controller !

Last update:
- minor bugfixes
- added support for small OLED displays with U8GLIB to show the values of all status registers all 500ms
