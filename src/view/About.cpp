#include "About.hpp"

UiAbout::UiAbout(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("About");

    layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    frameAbout = new FrameAbout(this);

    layout->addWidget(frameAbout, 0, 0);
}

FrameAbout::FrameAbout(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::NoFrame);
    setObjectName("frameAbout");

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(0);

    textBrowserAbout = new QTextBrowser(this);
    textBrowserAbout->setObjectName("textBrowserAbout");

    // Size policy
    textBrowserAbout->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // Font
    QFont font;
    font.setFamily("Sans");
    font.setPointSize(12);
    textBrowserAbout->setFont(font);

    // HTML content
    textBrowserAbout->setHtml(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\""
        "\"http://www.w3.org/TR/REC-html40/strict.dtd\">"
        "<html><head><meta name=\"qrichtext\" content=\"1\" />"
        "<meta charset=\"utf-8\" /><style type=\"text/css\">"
        "p, li { white-space: pre-wrap; }"
        "hr { height: 1px; border-width: 0; }"
        "li.unchecked::marker { content: \"\\2610\"; }"
        "li.checked::marker { content: \"\\2612\"; }"
        "</style></head><body style=\""
            "font-family:'Sans Serif'; "
            "font-size:12pt; "
            "font-weight:400; "
            "font-style:normal;"
        "\">"
        "<h3 style=\" "
            "margin-top:14px; "
            "margin-bottom:12px; "
            "margin-left:0px; "
            "margin-right:0px; "
            "-qt-block-indent:0; "
            "text-indent:0px;"
            "\">"
        "<span style=\" "
            "font-size:large; "
            "font-weight:600;"
        "\">About</span></h3>"
        "<h4 style=\" "
            "margin-top:12px; "
            "margin-bottom:12px; "
            "margin-left:0px; "
            "margin-right:0px; "
            "-qt-block-indent:0; "
            "text-indent:0px;"
        "\">"
        "<span style=\" "
            "font-size:medium; "
            "font-weight:600;"
        "\">Tomb Raider Linux Launcher</span></h4>"
        "<p style=\" "
            "margin-top:12px; "
            "margin-bottom:12px; "
            "margin-left:0px; "
            "margin-right:0px; "
            "-qt-block-indent:0; "
            "text-indent:0px;"
        "\">"
            "Welcome to the <span style=\" font-weight:600;\">"
            "Tomb Raider Linux Launcher</span>! "
            "This application is designed to provide a streamlined and "
            "convenient way to launch your favorite Tomb Raider games on "
            "Linux. Currently, the launcher supports Tomb Raider 1-6 "
            "everything from Core Design, trle.net and TEN at the moment."
        "</p>"
        "<p style=\" "
            "margin-top:12px; "
            "margin-bottom:12px; "
            "margin-left:0px; "
            "margin-right:0px; "
            "-qt-block-indent:0; "
            "text-indent:0px;"
        "\">"
        "Our aim is to enhance your gaming experience by offering a "
        "user-friendly interface that simplifies game management and "
        "launching. The launcher is still a work in progress, and we are "
        "actively working on adding support for more games and improving "
        "functionality."
        "</p>"
        "<p style=\" "
            "margin-top:12px; "
            "margin-bottom:12px; "
            "margin-left:0px; "
            "margin-right:0px; "
            "-qt-block-indent:0; "
            "text-indent:0px;"
        "\">"
        "Stay tuned for updates and new features as we continue to develop and "
        "refine the Tomb Raider Linux Launcher. Your feedback and suggestions "
        "are always welcome to help us create the best possible tool for Tomb "
        "Raider fans."
        "</p>"
        "<p style=\" "
            "margin-top:12px; "
            "margin-bottom:12px; "
            "margin-left:0px; "
            "margin-right:0px; "
            "-qt-block-indent:0; "
            "text-indent:0px;"
        "\">"
        "Thank you for using the Tomb Raider Linux Launcher!"
        "</p>"
        "<p align=\"center\" style=\" "
            "margin-top:0px; "
            "margin-bottom:0px; "
            "margin-left:0px; "
            "margin-right:0px; "
            "-qt-block-indent:0; "
            "text-indent:0px;"
        "\">"
        "<img src=\":/pictures/Lara3.jpg\" width=\"400\" height=\"350\" />"
        "</p>"
        "</body></html>"
    );

    layout->addWidget(textBrowserAbout);
}

