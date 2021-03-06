#include "stdafx.h"

#include "Core/GlobiSingleApplication.h"
#include "Core/ClipboardWatcher.h"
#include "Core/MimeDataEntry.h"
#include "Core/Settings.h"

#include "Core/Redis/RedisServer.h"

#include "QML/QMLPlugin.h"
#include "QML/QMLUi.h"
#include "QML/QMLClipboard.h"
#include "QML/QMLSettings.h"
#include "QML/QMLEnvironment.h"

#include "GUI/SystemTray.h"
#include "GUI/DynamicImageEngine.h"

#include "Network/NetworkAccessManager.h"
#include "Network/NetworkHTTPRequest.h"
#include "Network/NetworkHTTPReply.h"
#include "Network/LocalHTTPServer.h"

#include "Core/RemoteClipboard.h"

#if defined RELEASE_CONSOLE || defined _DEBUG
 #define QT_QML_DEBUG
#endif

int main(int argc, char *argv[])
{
    // Pre initialisation
    qRegisterMetaTypeStreamOperators<Shortcut>();

#if defined RELEASE_CONSOLE || defined _DEBUG
    QQmlDebuggingEnabler debuggingEnabler;
#endif
    QMLEnvironment::registerComponents <
        // QML Components
        QMLPlugin, QMLUi, QMLClipboard, QMLSettings,
        // Qt classes
        QQmlPropertyMap, QWidget, QLayout, QNetworkReply,
        // Exposed classes
        MimeDataEntry, LocalHTTPServer, NetworkHTTPRequest, NetworkHTTPReply
    >();

    // Single application management
    GlobiSingleApplication app(argc, argv, APPLICATION_NAME);
    app.setQuitOnLastWindowClosed(false);
    if(!app.isAlreadyRunning())
    {
        QObject::connect(&app, &GlobiSingleApplication::messageRceived, [](const QString & message) {
            SystemTray::instance().alert(message, APPLICATION_ALREADY_RUNNING_TITLE);       
        });
    
        // Component initialisation
        RedisServer::command(REDIS_FLUSHDB_COMMAND);
        SystemTray::instance().show();
        ClipboardWatcher::instance();

        // Load settings
        auto color = Settings::value<QColor>("mask_color", "Ui", Qt::black); // initial color
        DynamicImageEngine::setMaskColor(color);

        for(auto & pluginUrl : Settings::value<QStringList>("urls", "Plug-ins"))
            QMLEnvironment::addPlugin(QUrl(pluginUrl));

        app.exec();
    }
    else
        app.sendMessage(APPLICATION_ALREADY_RUNNING_NOTIFICATION);

    return 0;
}
