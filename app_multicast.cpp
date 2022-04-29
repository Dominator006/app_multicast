#include "app_multicast.h"
#include "ui_app_multicast.h"

#include <QtNetwork>
#include <QFileDialog>
#include <QDebug>


App_multicast::App_multicast(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::App_multicast),count_image(0),connection_(false)
{
    ui->setupUi(this);
    ui->pseudo->setText("Utilisateur");

  /* création du socket server/client UDP  */
    socket_client = new QUdpSocket(this);
    socket_server= new QUdpSocket(this);

    socket_client->bind(QHostAddress(MULTICAST_ADDRESS),socket_client->localPort());
    socket_client->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);


    /* socket du serveur */
    socket_server->bind(QHostAddress(MULTICAST_ADDRESS), PORT_GROUP , QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);


    socket_server->joinMulticastGroup((QHostAddress(MULTICAST_ADDRESS)));

    //if (!socket_server->bind(QHostAddress::AnyIPv6, PORT_GROUP, QUdpSocket::ShareAddress) || !socket_server->joinMulticastGroup((QHostAddress(MULTICAST_ADDRESS)))) {
        //exit(EXIT_FAILURE);
    //}


    connect(socket_server,&QUdpSocket::readyRead,this,&App_multicast::processPendingDatagram);


    connect(socket_server,SIGNAL(errorOccurred(QAbstractSocket::SocketError)),this,SLOT(erreurSocket(QAbstractSocket::SocketError)));

    connect(ui->send_msg, &QPushButton::pressed, this, &App_multicast::start_Sending);

    bool ok;

        QString text = QInputDialog::getText(this,
                                             tr("Client Connection"),
                                             tr("Give your pseudo"),
                                             QLineEdit::Normal,
                                             QDir::home().dirName(),
                                             &ok);
        if (ok and !text.isEmpty())
            ui->pseudo->setText(text);
        else
            ui->pseudo->setText("Utilisateur");

        ui->pseudo->setReadOnly(true);
  /* Prévenir les autres clients, qu'un client vient de se connecter */
  boutonEnvoyer_msg_clicked();
}


void App_multicast::isConnected(const bool& connection)
{
    this->connection_ = connection;
}

void App_multicast::boutonEnvoyer_msg_clicked(){
    if (connection_==true) {
        QByteArray message = ui->message->toPlainText().toUtf8();
        QByteArray datagram = ui->pseudo->text().toUtf8();
        datagram = datagram + " : " + message;
        socket_client->writeDatagram(datagram, QHostAddress(MULTICAST_ADDRESS), PORT_GROUP);
        ui->message->clear();
        ui->message->setFocus();
    } else {

        QByteArray datagram = ui->pseudo->text().toUtf8();
        datagram = datagram + ":arejoint#147258!@.";
        socket_client->writeDatagram(datagram, QHostAddress(MULTICAST_ADDRESS), PORT_GROUP);
        ui->message->clear();
        isConnected(true);
    }


}




/* SLOT reception message */
void App_multicast::processPendingDatagram(){

    QByteArray Buffer;
    QHostAddress sender;
    quint16 senderPort;

    while (socket_server->hasPendingDatagrams())
        {
            Buffer.resize(socket_server->pendingDatagramSize());
            socket_server->readDatagram(Buffer.data(), Buffer.size(), &sender, &senderPort);
            if (Buffer.size() < 2000)
            {
                QStringList msg = QString(Buffer.data()).split(":");
                            if (msg[0] == "Utilisateur " and msg[1] != "/image#147258!@.")
                            {
                                ui->display_msg->append("Utilisateur " + QString::number(senderPort) + " :" + msg[1]);
                            } else if (msg[0] == "Utilisateur" and msg[1] == "/image#147258!@.")
                            {
                                ui->display_msg->append("Utilisateur " + QString::number(senderPort) + " a envoyé une image.");

                            } else if (msg[0] != "Utilisateur" and msg[1] == "/image#147258!@.")
                            {
                                ui->display_msg->append(msg[0] + " a envoyé une image.");

                            } else if (msg[0] != "Utilisateur" and msg[1] == "arejoint#147258!@.")
                            {
                                ui->display_msg->append(msg[0] + " a rejoint la discussion !");
                            } else if (msg[0] == "Utilisateur" and msg[1] == "arejoint#147258!@.")
                            {
                                ui->display_msg->append("Utilisateur " + QString::number(senderPort) + " a rejoint la discussion !");
                            } else
                            {
                                ui->display_msg->append(Buffer.data());
                            }
            } else
            {
                QImage image_temp = get_imagedata_from_byte(Buffer.data());
                //image_temp.save(QString("./tmp/image%1.png").arg(count_image), "PNG");
                QImage* imgScaled = new QImage;
                *imgScaled = image_temp.scaled(400, 400, Qt::KeepAspectRatio);
                ui->image->setPixmap(QPixmap::fromImage(*imgScaled));
                count_image++;


            }
       }

    //qDebug()<<"Message from : " << senderPort ;
    //qDebug()<<"Message : " << Buffer ;
}




QByteArray  App_multicast::get_imagedata_from_imagefile(const QImage &image)
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    image.save(&buffer, "JPG");
    imageData = imageData.toBase64();
    return imageData;
}

QImage App_multicast::get_imagedata_from_byte(const QString &data)
{
    QByteArray imageData = QByteArray::fromBase64(data.toLatin1());
    QImage image;
    image.loadFromData(imageData);
    return image;
}



void App_multicast::boutonEnvoyer_img_Clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Image à envoyer", QDir::currentPath(), "Image Files(*.jpg *.png)");
    if (file_name == "")
        return;

    image.load(file_name);
    ui->image->setPixmap(QPixmap::fromImage(image).scaled(ui->image->size()));

    image_file_data = get_imagedata_from_imagefile(image);
    socket_client->writeDatagram(image_file_data.toLatin1(),image_file_data.size(),QHostAddress(MULTICAST_ADDRESS), PORT_GROUP);

    QByteArray datagram = ui->message->toPlainText().toUtf8();
    datagram += ":/image#147258!@.";
    socket_client->writeDatagram(datagram, QHostAddress(MULTICAST_ADDRESS), PORT_GROUP);
}





void App_multicast::erreurSocket(QAbstractSocket::SocketError erreur){
    switch(erreur) {

    case QAbstractSocket::HostNotFoundError:

           ui->display_msg->append(tr("<em>ERREUR : le serveur n'a pas pu être trouvé. Vérifiez l'IP et le port.</em>"));

          break;
    case QAbstractSocket::ConnectionRefusedError:
        ui->display_msg->append(tr("<em>ERREUR : le serveur a refusé la connexion. Vérifiez si le programme \"serveur\" a bien été lancé. Vérifiez aussi l'IP et le port.</em>"));
        break;

    case QAbstractSocket::RemoteHostClosedError:
            ui->display_msg->append(tr("<em>ERREUR : le serveur a coupé la connexion.</em>"));
          break;

    default:
       ui->display_msg->append(tr("<em>ERREUR : ") + socket_client->errorString() + tr("</em>"));

    }
}





void App_multicast::start_Sending()
{
    ui->send_msg->setEnabled(true);
}






App_multicast::~App_multicast(){

    /*Prevent all users that a client has been disconnected */

    QByteArray datagram = ui->pseudo->text().toUtf8();
    datagram = datagram + " s'est déconnecté !";
    socket_client->writeDatagram(datagram, QHostAddress(MULTICAST_ADDRESS), PORT_GROUP);

    /* Destruction of main window */

    delete ui;
}


