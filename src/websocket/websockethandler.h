#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H

#include <QObject>
#include <QWebSocket>
#include <QImage>

class WebSocketHandler : public QObject {
    Q_OBJECT

public:
    //explicit WebSocketHandler(const QString &url, QImage *image, QObject *parent = nullptr);

    explicit WebSocketHandler(QObject *parent = nullptr);
    ~WebSocketHandler();

    void sendOpen(const QString &url);
    void sendRXDetails(const QString &callsign, const QString &mode, unsigned int x, unsigned int y);
    void sendHorizontalLine(unsigned int pixelCount, unsigned int lineNumber, QRgb *pixelArray);

signals:
    void requestOpen(const QString &url);
    void requestSendRXDetails(const QString &callsign, const QString &mode, unsigned int x, unsigned int y);
    void requestSendHorizontalLine(unsigned int pixelCount, unsigned int lineNumber, QRgb *pixelArray);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

    void handleOpen(const QString &url);
    void handleSendRXDetails(const QString &callsign, const QString &mode, unsigned int x, unsigned int y);
    void handleSendHorizontalLine(unsigned int pixelCount, unsigned int lineNumber, QRgb *pixelArray);
    //void handleSendHorizontalLine(unsigned int pixelCount, unsigned int lineNumber, QRgb *pixelArray);

private:
    QWebSocket *webSocket;
    QThread *workerThread;
};

#endif // WEBSOCKETHANDLER_H
