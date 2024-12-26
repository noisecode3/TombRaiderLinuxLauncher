#ifndef SRC_DEBUG_HPP_
#define SRC_DEBUG_HPP_

#include <QDebug>
#include <QString>
#include <QTextStream>

// Debug variable
inline bool DEBUG_ENABLED = true;  // Toggle this to enable/disable debugging

// Debug log function
inline void debugLog(const QString &message) {
    if (DEBUG_ENABLED) {
        qDebug() << message;
    }
}

// Stop function for debugging
inline void debugStop(
        const QString& var1 = "",
        const QString& var2 = "",
        const QString& var3 = "",
        const QString& var4 = "") {
    if (DEBUG_ENABLED) {
        QTextStream in(stdin);
        if (var1 != "") {
            qDebug() << var1;
        }
        if (var2 != "") {
            qDebug() << var2;
        }
        if (var3 != "") {
            qDebug() << var3;
        }
        if (var4 != "") {
            qDebug() << var4;
        }
        in.readLine();
    }
}

#endif  // SRC_DEBUG_HPP_

