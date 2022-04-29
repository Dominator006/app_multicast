#ifndef APP_MULTICAST_H
#define APP_MULTICAST_H

#include <QMainWindow>
#include <QtNetwork>
#include <QFileDialog>
#include <QImage>
#include <QBuffer>
#include <QInputDialog>


#define  MULTICAST_ADDRESS "239.255.255.252"

#define PORT_GROUP  12345

QT_BEGIN_NAMESPACE
namespace Ui { class App_multicast; }
QT_END_NAMESPACE

class App_multicast : public QMainWindow
{
    Q_OBJECT

public:
     explicit App_multicast(QWidget *parent = nullptr);
    ~App_multicast();

public:
     void isConnected(const bool & connection);



private slots :

   void boutonEnvoyer_msg_clicked();
   void boutonEnvoyer_img_Clicked();
   void start_Sending();
   void processPendingDatagram();
   void erreurSocket(QAbstractSocket::SocketError erreur);



private:
    Ui::App_multicast *ui;
    QUdpSocket *socket_client, *socket_server;

private:

    QString image_file_data;
    QImage image;
    QString picture_data;
    int count_image;
    bool connection_;

private:
    QByteArray get_imagedata_from_imagefile(const QImage &image);
    QImage get_imagedata_from_byte(const QString &data);
};



#endif // APP_MULTICAST_H
