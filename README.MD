# GDS-Render

This software is a rendering programm for GDS2 layout files.
The GDS2 format is mainly used in integrated circuit development.
This program allows the conversion of a GDS file to a vector graphics file.

## Output Formats
* Export GDS Layout to LaTeX (using TikZ).
* Export to PDF (Cairographics).

# Features
Note: Due to various size limitations of both TikZ and the PDF export, the layout might not render correctly. In this case adjust the scale value. A higher scale value scales down your design.

* Configurable layer stack-up.
* Layer colors configurable as ARGB color values.
* Command line interface.
* ~~Awesome~~ Somehow usable GUI.

# License and Other Stuff
* Free software (GPLv2 _only_)
* Coded in plain C using GTK+3.0, Glib2, and Cairographics
