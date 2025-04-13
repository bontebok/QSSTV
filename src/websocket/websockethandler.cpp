#include "websockethandler.h"
#include <QThread>
#include <QWebSocket>
#include <QObject>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QByteArray>
#include <QBuffer>


WebSocketHandler::WebSocketHandler(QObject *parent) : QObject(parent) {

    // Create and start the worker thread
    workerThread = new QThread(this);

    // Move this object to the worker thread
    this->moveToThread(workerThread);

    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");

    webSocket = nullptr;

    // Connect signals to slots within the worker thread
    connect(workerThread, &QThread::started, this, [this]() {
        webSocket = new QWebSocket();
        webSocket->moveToThread(workerThread);

        connect(webSocket, &QWebSocket::connected, this, &WebSocketHandler::onConnected, Qt::QueuedConnection);
        connect(webSocket, &QWebSocket::disconnected, this, &WebSocketHandler::onDisconnected, Qt::QueuedConnection);
        connect(webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
                this, &WebSocketHandler::onError, Qt::QueuedConnection);

        // Queued connections for worker thread methods
        connect(this, &WebSocketHandler::requestOpen, this, &WebSocketHandler::handleOpen, Qt::QueuedConnection);
        connect(this, &WebSocketHandler::requestSendRXDetails, this, &WebSocketHandler::handleSendRXDetails, Qt::QueuedConnection);
        connect(this, &WebSocketHandler::requestSendHorizontalLine, this, &WebSocketHandler::handleSendHorizontalLine, Qt::QueuedConnection);
    });

    workerThread->start();
}

// Destructor
WebSocketHandler::~WebSocketHandler() {
    // No need to explicitly delete webSocket; it is handled by QObject parent-child relationship
    workerThread->quit();
    workerThread->wait();

    delete webSocket; // Explicit deletion since we moved it to a different thread
}


// Public methods to send requests to the worker thread
void WebSocketHandler::sendOpen(const QString &url) {
    emit requestOpen(url);
}

void WebSocketHandler::sendRXDetails(const QString &callsign, const QString &mode, unsigned int x, unsigned int y) {
    emit requestSendRXDetails(callsign, mode, x, y);
}

void WebSocketHandler::sendHorizontalLine(unsigned int pixelCount, unsigned int lineNumber, QRgb *pixelArray) {
    emit requestSendHorizontalLine(pixelCount, lineNumber, pixelArray);
}


void WebSocketHandler::handleOpen(const QString &url) {
    if (webSocket && !webSocket->isValid()) {
        webSocket->open(QUrl(url));
    }
}

void WebSocketHandler::handleSendRXDetails(const QString &callsign, const QString &mode, unsigned int x, unsigned int y) {
    if (webSocket && webSocket->isValid()) {
        QJsonObject json;
        json["type"] = "init";
        json["callsign"] = callsign;
        json["mode"] = mode;
        json["width"] = (int) x;
        json["height"] = (int) y;
        //json["height"] = (int) 1;

        QJsonDocument doc(json);
        webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }
}

void WebSocketHandler::handleSendHorizontalLine(unsigned int pixelCount, unsigned int lineNumber, QRgb *pixelArray) {
    if (webSocket && webSocket->isValid()) {
        QJsonArray lineColors;
        QJsonObject json;
        QByteArray byteArray;

        json["type"] = "line";
        json["y"] = (int) lineNumber;
        //json["y"] = (int) 0;

        byteArray.resize(pixelCount * 3);

        char* dataPtr = byteArray.data();

        for (unsigned int x = 0; x < pixelCount; ++x) {
            *dataPtr++ = static_cast<char>(qRed(pixelArray[x]));
            *dataPtr++ = static_cast<char>(qGreen(pixelArray[x]));
            *dataPtr++ = static_cast<char>(qBlue(pixelArray[x]));
        }

        QByteArray base64Encoded = byteArray.toBase64();

        json["colors"] = QString::fromUtf8(base64Encoded);

        // Convert to JSON and send
        QJsonDocument doc(json);
        webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }
}

void WebSocketHandler::onConnected() {
    qDebug() << "WebSocket connected";
    //sendImageDimensions();
}

void WebSocketHandler::onDisconnected() {
    qDebug() << "WebSocket disconnected";
}

void WebSocketHandler::onError(QAbstractSocket::SocketError error) {
    qDebug() << "WebSocket error:" << error;
}
